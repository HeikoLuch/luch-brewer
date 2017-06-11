#include <Arduino.h>
#include <UIPEthernet.h> 
#include <avr/pgmspace.h>
#include "bm_estorage.h"
#include "bm_global.h"
#include "NTP-Clock.h"


//#define HTML_FORMS_DEBUG

// ---------------- Definitions ---------------

#define RAST_TEMPERATURE "_t"
#define RAST_TIME "_z"
//#define RAST_RAISE "_r"
#define RAST_HOLD "_h"

#define CFG_SIMU F("cfg_simu")

#define MaxHeaderLength 255    //maximum length of http header received
#define MAX_ETH_BUF_SIZE 1300		//maximum buffer size before sending the string to eth

#define IP_ADDRESS 192, 168, 178, 222
const byte mac[] PROGMEM  = { 0x74, 0x24, 0x11, 0x01, 0x40, 0x31 };
EthernetServer server(80);
#define MY_URL F("http://192.168.178.222")

const byte MAX_MENU_ENTRIES = 4;		//may number of menu entries to be shown

//Das scheint die optimalste Übergabe eines Strings zu sein
String HTML_TABLE_HEADER_RASTEN() {
	//return String (F("<table style='border:1px solid;'><tr><th>Rast</th><th>Temperatur<br>[&deg;C]</th><th>Zeit<br>[min]</th><th>Temp Steigung<br>[&deg;C / 10min]</th><th>Anhalten</th></tr>"));	
	return String (F("<table style='border:1px solid;'><tr><th>Rast</th><th>Temperatur<br>[&deg;C]</th><th>Zeit<br>[min]</th><th>Anhalten</th></tr>"));	
}
          

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int) __brkval); 
}					                      

// --------------------------

void init_ethernet(){
	// start the Ethernet connection and the server:
  IPAddress ip (IP_ADDRESS);
  Ethernet.begin(mac, ip);
  server.begin();
  String s = (F("Starting\n\tIP Address: "));
	s += Ethernet.localIP();
  diag.info(F("init_ethernet"), s);

}

#ifdef HTML_FORMS_DEBUG
void showString (String name, String s) {
 	Serial.print (F("HTML_FORMS.cpp - ")); Serial.print (name);  Serial.print(F(": '")); Serial.print(s);   Serial.println(F("'"));
}
#endif

// --------------- Send Buffer ------------------
/**
 *	Send to Client
 * Growing String until reaching a limit.
 * If limit is exceeded -> send it to the client!
 * Dont forget to force sending at the end.
 * This reducsd the number of calls of "client.print()" -> increasing speed
 */
void s2C(String s, byte force){
	static String buf = "";
	buf += s;	
	if ((buf.length() > MAX_ETH_BUF_SIZE) || (force!=0)) {
		EthernetClient client = server.available();
		client.print(buf);
		buf = "";
		#ifdef HTML_FORMS_DEBUG
			showString (F("s2C Sending"), String(freeRam()));
		#endif
	}
}

void s2C(String s){
	s2C (s, 0);
}


/**
* Create a Link toggling the HTML Element's "display" property.
* Requires, that 
* a) you define the same ID for the HTML element
* b) and (optional) it's initial style as "display : none" or "display : block"
* i.e <span id="test1" style="display: none">Text Nummer zwei kommt hier ...</span>
*/
String createJsToggle(String id, String linkText) {
	String s =  F("<a href=\"javascript:toggle('");
	s += id;
	s += F("')\">");
	s += linkText;
	s += F("</a>");
	return s;
}


// ---------------- Table Rows ------------------------


  
/* 
	Show the parameters for 'Rastsen' and make it editable.
*/
String form_input_make_table_row( byte num) {
  //long startTime = millis();
  
  const __FlashStringHelper* INPUT_1 = F("<td align='center'><input type='text'  style='text-align: right;width: 48px;' name='");
  const __FlashStringHelper* INPUT_3 = F("' value= '");
  const __FlashStringHelper* INPUT_2 = F("' style='width: 50px;' align='right'>");

  //Column name
  String s = F("<tr><td>");
  s += (num + 1);
  s += F(": ");
  s += rasten[num].name;
  s += F("</td>");
 
  //Column temperature
  s += INPUT_1;
  s += num;
  s += F(RAST_TEMPERATURE); // "_temp"
  s += INPUT_3;
  s += String(rasten[num].temp);
  s += INPUT_2;
  s += F("</td>");

    //Column time
  s += INPUT_1;
  s += num;
  s += F(RAST_TIME);
  s += INPUT_3;
  s += String(rasten[num].time);
  s += INPUT_2;
  s += F("</td>");
/**
  //Column raise
  s += INPUT_1;
  s += num;
  s += F(RAST_RAISE);
  s += INPUT_3;
  s += String(rasten[num].raise);
  s += INPUT_2;
  s += F("</td>");
*/

  //Column hold
  s += F("<td><input type='checkbox' name='");
  s += num;
  s += F(RAST_HOLD);
  if (rasten[num].wait4User)
    s += F("' checked >") ;
  else
   s += F("'>") ;
  s += F("</td></tr>");
  
 return s;

  //Serial.print ("Laufzeit form_input_make_table_row():");
  //Serial.println(millis() - startTime);
}




/**
* Send HTML String containing table showing the values of parameters
*/
void send_protocol_table (byte expanded ) {
  //long startTime = millis();
   s2C( createJsToggle( F("tblProto"), F("Brau-Protokoll")));
   s2C( F("<div id='tblProto' style='display: "));
    if (expanded == 0)
    	s2C(  F("none'>"));
    else 
    	s2C( F("block'>"));
	s2C( F("<p>Programmstart: "));
	s2C( time2TimeString(protocol.timeStart));

	s2C( F(", Laufzeit: "));
	s2C( time2TimeString(protocol.timeStop - protocol.timeStart));
	s2C( F("<br>"));
	s2C( F("<table><tr><th>Brauschritt</th><th>Startzeit</th><th>Rastbeginn</th>"));
	for ( byte num = 0; num < MAX_RASTEN1; num++) {
		s2C( F("<tr><td>"));
		s2C( rasten[num].name);
		s2C( F("</td><td>"));
		s2C( time2TimeString(protocol.timeHeatingStart[num]));
		s2C( F("</td><td>"));
		s2C( time2TimeString(protocol.timeRastStart[num]));
		s2C( F("</td></tr>"));
	}	
  s2C(  F("</table></p></div>"));
 // return s;
}




/**
* Calculate HTML String containing table showing the values of parameters
*/
void send_parameter_table () {
  //long startTime = millis();

	//calculate Color	
	String bgColorDone = F(" bgcolor=#32CD32");
	String bgColorInWork = F(" bgcolor=#FF8C00");
	String bgColor = "";
	
	s2C( HTML_TABLE_HEADER_RASTEN());
	//loop over parameters
	for ( byte num = 0; num < MAX_RASTEN1; num++) {
		
  		//Column name
  		bgColor = "";
  		if (num == rast_num_current)
  			bgColor = bgColorInWork;
  		else if (num < rast_num_current)
  			bgColor = bgColorDone;
  			
  		s2C( F("<tr><td"));
  		s2C( (bgColor + F(">")));
  		s2C( String (num + 1));
  		s2C( F(": "));
  		s2C( rasten[num].name);
  		s2C( F("</td><td"));
 
  		//Column temperature
  		bgColor = "";
  		if (num < rast_num_current)
  			bgColor = bgColorDone;
  		else if (num == rast_num_current) {
  			bgColor = bgColorInWork;
  			if (protocol.timeRastStart[num] > 0) //Heizende erreicht -> rast begonnen. 
  				bgColor = bgColorDone;
  		}
  			
  		s2C( bgColor + ">");	
  		s2C( String(rasten[num].temp));
  		s2C( F("</td><td"));


    	//Column time
    	bgColor = "";
  		if (num < rast_num_current) 
  			bgColor = bgColorDone;
  		else if (num == rast_num_current) {
  			bgColor = bgColorInWork;
  			if (protocol.timeRastStart[num] == 0) //Heizende noch nicht erreicht. 
  				bgColor = "";
  			else if(rast_halt_executed)
  				bgColor = bgColorDone;
  		}
  		
  		s2C( bgColor + ">");	

  		s2C( String(rasten[num].time));  	
  		s2C( F("</td><td"));

		/*
  		//Column raise
		s2C( F(">"));
  		s2C( String(rasten[num].raise));
  		s2C( F("</td><td"));
*/

  		//Column hold
  		bgColor = "";
  		if (num < rast_num_current)
  			bgColor = bgColorDone;
  		else if ((num == rast_num_current) && rast_halt_executed)
  			bgColor = bgColorInWork;
  			
  		s2C( bgColor + F(">"));	
  		if (rasten[num].wait4User)
    		s2C( F("JA") );
  		else
   		s2C( F("NEIN")) ;
  		s2C( F("</td>"));
  }
  s2C( F("</table>"));
}



/*
	I.e. for Relais: ON is RED and OFF is Blue
*/
String getOnOffString(byte status) {
	
	if (status == 0)
		return F("<FONT COLOR='#FF0000'>OFF</FONT>");
	if (status == 1)
		return F("<FONT COLOR='#0000FF'>ON</FONT>");
	return F("UNKNOWN");		
}


/**
* If condition is true, the text will be shown as link.
*/
String conditionalLinkedText(boolean condition, String txt, String link1, String link2){
 
	if (condition == 1)  {
		String s = String ( "<a href='" + link1  + link2 + "'>" + txt + "</a>");
		//showString ("conditionalLinkedText()", String (condition) );
		return s;
	}
	return txt; 
}




void sendRelaisState (byte expanded) { 
	 //Show Relais States - independent from "state_main"
    s2C (F("<p>"));
	 s2C ( createJsToggle( F("tblStates"), "Relais Status")); 
	 
    s2C (F("<div id='tblStates' style='display: "));
    if (expanded == 0)
    	s2C (F("none'>"));
    else 
    	s2C (F("block'>"));
    s2C (F("<table  style='border:1px solid;border-spacing:10px;'><tr><th>Relais</th><th>Aktueller Wert</th></tr>"));
          
    s2C (F("<tr><td>Hauptrelais</td><td align='center'><b>"));
    s2C (String(relais.get_MAIN()));
    s2C (F("</b></td></tr>"));
          
    s2C (F("<tr><td>SUD</td><td align='center'><b>"));
    s2C (String (relais.get_SUD()));
    s2C (F("</b></td></tr>"));

    s2C (F("<tr><td>L&aumlutern</td><td align='center'><b>"));
    s2C (String (relais.get_LAUT()));
    s2C (F("</b></td></tr>"));

    s2C (F("<tr><td>Pumpe</td><td align='center'><b>"));
    s2C (String (relais.get_PUMP()));
    s2C (F("</b></td></tr>"));
    s2C (F("</table></div>"));  
    //return s;        
}


/**
 * Create menu entries: Recommended usage of conditionalLinkedText().)
 * Remeber MAX_MENU_ENTRIES!
 */
String createMenu (String p[]) 
{
	//showString ("substate_main_current1 != SUBSTATE_RUNNING_HALT", String (substate_main_current1) ); 
	String s = F("<table cellpadding='10'><tr>");
	for (byte i=0; i < MAX_MENU_ENTRIES; i++) {
		if (p[i].length() > 1 ) {
			s += F("<td>");
			s += p[i];
			s += F("</td>");
		}
	}
	s += F("</tr></table>");	
	return s;
/*			
			s += conditionalLinkedText (substate_main_current1 != SUBSTATE_RUNNING_OK, F("FORTSETZEN"), MY_URL, "/run");
		s += F("</td><td>");

	s += conditionalLinkedText ((substate_main_current1 != SUBSTATE_RUNNING_PAUSE) && (substate_main_current1 != SUBSTATE_RUNNING_HALT), 
			F("PAUSE"), MY_URL, "/pause");
	s += F("</td></tr></table>");*/
}

// ---------------- Ethernet input handler ------------------------



/*
   Show formular.
   Depending on state_main, different content is shown
*/
void form_input() {

	const __FlashStringHelper* loc = F("form_input");
	
  //long startTime = millis();
  String HttpHeader = String("");

  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {

        char c = client.read();
        //read MaxHeaderLength number of characters in the HTTP header
        //discard the rest until \n
        if (HttpHeader.length() < MaxHeaderLength)
        {
          //store characters to string
          HttpHeader = HttpHeader + c;
        }
        //if HTTP request has ended
        if (c == '\n') {

          //This is the initial call when requesting from IP: "GET / HTTP/1.1"
          //Achtung! Stringlänge beträgt bereits 200 plus/minus, abh. von gesendeten parametern!
          //This is the call when submitting form: "GET /?0_t=22&0_z=20&0_r=50&0_h=on&1_t=24&1_z=20&1_r=10&2_t=26&2_z=15&2_r=10&3_t=26&3_z=20&3_r=10&4_t=26&4_z=30&4_r=10&5_t=28&5_z=30&5_r=10&5_h=on&6_t=30&6_z=20&6_r=10&6_h=on&cfg_simu=on HTTP/1.1"
		  #ifdef HTML_FORMS_DEBUG
			showString (F("HttpHeader: "), HttpHeader);
		#endif

          //Seems to be a submit of form?
          int index1 = HttpHeader.indexOf (F("GET /?"));
          int index2 = HttpHeader.indexOf (F(" HTTP/"));
          if (index1 > -1) {
				//new parameter were entered          	
				//... and this is the form submit:  GET /?A_temp=88&A_min=99&A_raise=&B_temp=11&B_min=&B_raise=&C_temp=&C_min=&C_raise=&D_temp=&D_min=&D_raise= HTTP/1.1
             
				if (index2 > index1) {
						
			
					HttpHeader = HttpHeader.substring (index1 + 6, index2); //'A_temp=88&A_min=99&A_raise=&B_temp=11&B_min=&B_raise=&C_temp=&C_min=&C_raise=&D_temp=&D_min=&D_raise='
					index2 = 0;   //search string from here
					#ifdef HTML_FORMS_DEBUG
					showString (F("Starting here"), HttpHeader);
					#endif
					//remember: not checked checkboxes are not sent! Therefore set it to false always
					SIMULATION = false;
					
					do //split parameter string by '&'
					{
						index1 = HttpHeader.indexOf (F("&"), index2);
						if (index1 < 0)
							index1 = HttpHeader.length();    //there is no '&' at the end

						String s = HttpHeader.substring(index2, index1);    	//Fragment i.e. 'A_temp=44'
						//showString (F("Fragment1"), s);
                  
						String valStr = s.substring (s.indexOf("=") + 1);		//Fragment i.e. '44'
						//showString (F("Value"), valStr);          

						//first: handle common configs
						
						
						if (s.startsWith(CFG_SIMU)) {
							
							if (0 == valStr.compareTo(F("on")))
								SIMULATION = true;
							
							//Serial.print(F("valStr: ")); 		
							//Serial.println(valStr); 									
						}
					
						else { //Rast Config with rows
					
							String column = s.substring (1, s.indexOf("="));		//Fragment i.e. 'A_temp'

							//valid values are >0!
							byte value = valStr.toInt();									//Value '44'
						 
							//determine row and colum
							unsigned char row =  s.charAt(0) - 48;   				//the most left character i.e. '0' (is row 0); ASCII '0' = 48
							//showString (F("Row 3"), String(row));
						 
							if ((row >= 0) && (row < MAX_RASTEN1)) {
								if (value > 0)  {
									if (0 == column.compareTo(RAST_TEMPERATURE)) {
										rasten[row].temp = value;
									}
									else if (0 == column.compareTo(RAST_TIME)) {
										rasten[row].time = value;
									}
									/*
									else if (0 == column.compareTo(RAST_RAISE)) {
										rasten[row].raise = value;
									}*/
									else
									diag.error (loc, F("ERROR 4"));
								}

								//checkboxes are set in a different way: if unchecked, they are NOT transmitted
								//therefore set it always to FALSE until a) they are transmitted AND value is 'on'
								rasten[row].wait4User = false;
								if ((0 == column.compareTo(RAST_HOLD)) && (0 == valStr.compareTo(F("on"))))
									rasten[row].wait4User = true;
							}
							else
								diag.error(loc, F("ERROR 2"));
						}
						/*
						Serial.println(index1);
						Serial.println(index2);*/
						index2 = index1  + 1;

					} while (index1 < HttpHeader.length());
              
			  		//Serial.print(F("SIMULATION: ")); 		             
					//Serial.println(SIMULATION);

					estorage.storeGlobalRasten();
				}
				else
					diag.error (loc, F("ERROR 1"));
			}
          else {		//simple GETS like "'GET /pause HTTP/1.1"
          	//Pause a running program -> relais off
          	if ((state_main_current == STATE_RUNNING) && (HttpHeader.indexOf(F("/pause")) > -1)) {
          		substate_main_requested = SUBSTATE_RUNNING_PAUSE;
          		//showString ("PAUSE", "==========");
          	}
          		
          	//HALT a running program -> no next Rast
          	else if ((state_main_current == STATE_RUNNING) && (HttpHeader.indexOf(F("/halt")) > -1))
          		substate_main_requested = SUBSTATE_RUNNING_HALT;
          	//Pause a running program
          	else if ((state_main_current == STATE_RUNNING) && (HttpHeader.indexOf(F("/run")) > -1))
          		substate_main_requested = SUBSTATE_RUNNING_HEATING;
          	else if ((state_main_current == STATE_RUNNING) && (HttpHeader.indexOf(F("/stop")) > -1))
          		state_main_requested = STATE_CANCELED;
          		//Pause a running program
          	else if ((state_main_current != STATE_RUNNING) && (HttpHeader.indexOf(F("/config")) > -1))
          		state_main_requested = STATE_SETUP;
          	
          	else if ((HttpHeader.indexOf(F("/run")) > -1) 
          			&& (state_main_current == STATE_SETUP))
          	{
          		state_main_requested = STATE_RUNNING;
          	}
          }


          // ========== start of web page - common part, starting body =======

			 //if (state_main == STATE_RUNNING)
          //	s +=F("\nRefresh: 5");  // refresh the page automatically every X sec
          client.println(F("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close"));	// the connection will be closed after completion of the response	
          client.println();	//DO NOT DELETE THIS! For any reason this println() is elementary ...
          s2C( F("<!DOCTYPE HTML>"));
          
          s2C( F("<html><head>"));
		
			 //Refresh only if running 
			 if (state_main_current == STATE_RUNNING) {        
          	s2C( F("<meta http-equiv='refresh' content='15; URL="));
          	s2C( MY_URL);
          	s2C( F("'/>"));
				}
          s2C( F("<TITLE>LUCH-Brauer</TITLE>"));

				//JavaScript zum auf und zuklappen: http://alice-grafixx.de/JavaScript-Tutorial/Auf/Zu-klappen-159
          s2C(F("<script type='text/javascript'>function toggle(control){var elem = document.getElementById(control);if(elem.style.display == 'none'){elem.style.display = 'block';}else{elem.style.display = 'none';}}</script>"));
          
          s2C(F("</head><body>\n"));
          s2C( F("<h3><a href='"));
          s2C( MY_URL);
          s2C( F("'>LUCH-Brau-Maschine</a></h3>")); 
          
//Serial.print(F("form_input() SRAM 1: "));
//Serial.println(freeRam());
        
      	 // 1. HEAD Line with color
      	 if (state_main_current == STATE_CANCELED) {
					s2C( F("<p><i><div style='background-color:lightblue;'>Prozess abgebrochen.<br>"));
					s2C(ntp_time_string());
					s2C( F("</div></i></p>"));
			 } 
			 else if (state_main_current == STATE_FINISHED) {
					s2C( F("<p><i><div style='background-color:lightgreen;'>Brauprogramm erfolgreich beendet.<br>"));
					s2C( ntp_time_string());
					s2C( F("</div></i></p>"));
			 }		
			 else if (state_main_current == STATE_SETUP) {
          	s2C( F("<p><div style='background-color:lightgray;'><i>Hinweis: Rast eingeben oder Felder leer lassen<br><small>"));
          	s2C( ntp_time_string());
          	s2C( F("</small></div></i></p>"));	
          }
          else if ((state_main_current == STATE_RUNNING) 
          		|| ((state_main_current == STATE_SETUP) && (state_main_requested == STATE_RUNNING)))
          {
					s2C(F("<br><i><div style='background-color:yellow;'>Brauen gestartet!<br>"));
					s2C( ntp_time_string());
					s2C( F("</div></i>"));//<br>Ablauf kann mit [Brauen abbrechen] unterbrochen werden ...</i></p>");	
				} 
			 else 
			 	s2C( F("<p>UNBEKANNTER STATUS ...</p>"));
			 	
			 	
      	 // 2. Menu
			if (state_main_current == STATE_SETUP){
				String sa[] = {
					conditionalLinkedText (true, 
								F("PROGRAMM STARTEN"), MY_URL, F("/run")),
							"","", ""
				};
				s2C( createMenu(sa)); 
			}
			else if ((state_main_current == STATE_FINISHED) || (state_main_current == STATE_CANCELED)){
				String sa[] = {
					conditionalLinkedText (true, 
								F("KONFIGURIEREN"), MY_URL, F("/config")),
							"","", ""
				};
				s2C( createMenu(sa)); 
			}
			else if ((state_main_current == STATE_RUNNING) 
          		|| ((state_main_current == STATE_SETUP) && (state_main_requested == STATE_RUNNING)))
         {
				String sa[] = {
						conditionalLinkedText ((substate_main_current != SUBSTATE_RUNNING_HEATING) && (substate_main_current != SUBSTATE_RUNNING_RASTING), 
							F("FORTSETZEN"), MY_URL, F("/run")),
						conditionalLinkedText ((substate_main_current != SUBSTATE_RUNNING_PAUSE) && (substate_main_current != SUBSTATE_RUNNING_HALT),
							F("PAUSE"), MY_URL, F("/pause")),
						conditionalLinkedText ((substate_main_current != SUBSTATE_RUNNING_PAUSE) && (substate_main_current != SUBSTATE_RUNNING_HALT),
							F("PROGRAMM ABBRECHEN"), MY_URL, F("/stop")),
						""
				};
				s2C( createMenu(sa));
			}
					       	 
      	 // 3. Protocol
      	
      	if ((state_main_current == STATE_RUNNING)  ||  (state_main_current == STATE_FINISHED) || (state_main_requested == STATE_CANCELED))
         {
				send_protocol_table(false);
			}
			
			  
      	 // 4. Konfig Table
//      	s = ""; 
			if (state_main_current == STATE_SETUP) {
          
          	s2C( F("<form method=get>"));
				s2C( HTML_TABLE_HEADER_RASTEN());
	
				//Because of memory issues the table each row is transmitted in one package				
				
				for ( byte num = 0; num < MAX_RASTEN1; num++)
            	 s2C(form_input_make_table_row ( num));	//show HTML-Inputs
            			 
            s2C( F("</table><p>Brauprozess simulieren?")); 
            s2C( F("<input type='checkbox' name='cfg_simu'"));
            if (SIMULATION)
            	s2C( F("checked"));
            s2C( F("></p><p>"));
            s2C( F("<input type='submit' value='Konfiguration speichern' align='right'></p></form>"));
			} 
			 
      	 // 5. Progress
			if (state_main_current == STATE_RUNNING){     	 
      		 send_parameter_table();	
      	}
      	 // 6. Measurement Points

//Serial.print(F("form_input() SRAM 2: "));
//Serial.println(freeRam());	      	 
				//s="";
          s2C( F("<p><table style='border:1px solid;border-spacing:10px;'><tr><th>Messpunkte</th><th>Aktueller Wert</th></tr>"));
          
          s2C( F("<tr><td>SUD 1 Temperatur</td><td align='right'><b>"));
          s2C( String(tempSudValue));
          s2C( F(" [&deg;C]</b></td></tr>"));

          s2C( F("<tr><td>SUD 2 Temperatur</td><td align='right'><b>"));
          s2C( String(tempLautValue));
          s2C( F(" [&deg;C]</b></td></tr>"));
          
          s2C( F("<tr><td>Innen Temperatur</td><td align='right'><b>"));
          s2C( String (tempInnerValue));
          s2C( F(" [&deg;C]</b></td></tr>"));

          s2C( F("</table>"));
			
      	 // 7. Relais
      	 
          sendRelaisState (state_main_current == STATE_RUNNING);
          
          
         //8. some debug messages 
			s2C(F("<p><h5>Free SRAM 3: "));
			s2C(String(freeRam()));	
			s2C(F("<h5></p>"));			
          
          s2C( F("</body></html>"), true);
          
          //client.print(s);
          
          //stopping client
          client.stop();
        }
      }
    }

    //Serial.print(F("form_input() SRAM finish: "));
	//Serial.println(freeRam());	
   //Serial.print ("Laufzeit form_input():");
  	//Serial.println(millis() - startTime);
  } //if client
  

}