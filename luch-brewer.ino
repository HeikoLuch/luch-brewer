#include <UIPEthernet.h> // Used for Ethernet
//#include <avr/pgmspace.h>
#include "NTP-Clock.h"
#include "html_forms.h"
#include "bm_global.h"
#include "bm_hmi.h"
#include "bm_temp.h"
#include "bm_relais.h"
#include "bm_buzzer.h"
#include "bm_estorage.h"
#include "bm_diag.h"



// ====================== CONST & GLOBAL =====================================

const boolean DEBUG = true;

char state_main_current = 0;
char state_main_requested = 0;

byte substate_main_current = 0;
byte substate_main_requested = 0;
bool SIMULATION = true;


// ----- Devices ----------
BM_Relais relais;
BM_Buzzer buzzer;
EStorage estorage;
BM_HMI hmi;

// Temp values
float tempInnerValue;
float tempSudValue;
float tempLautValue;


// Parameter: Siehe auch hier: http://www.besser-bier-brauen.de/selber-bier-brauen/brauanleitungen/detailwissen/maischen

brewType rasten[] = {
  //Name, temp, time, raise
  //{"Vorl&ouml;sen / Einmaischen", 22, 20, 50, true},    //Maischen
  {"Vorloesen / Einmaischen", 22, 20, 50, true},    //Maischen
  {"Glukanaserast (Gummirast)", 24, 20, 10, false},    //Roggenrast
  {"Weizenrasten (Ferularasten)", 26, 15, 10, false},     //Weizenrast
  {"Proteaserast (Eiwei&szlig;rast)", 26, 20, 10, false} ,    
  {"Maltoserast", 26, 30, 10, false},    
  {"Verzuckerung", 28, 30, 10, true},   
  {"Abmaischen", 30, 20, 10, true},    
};

/*
brewType rasten[] = {
  //Name, temp, time, raise
  {"Vorl&ouml;sen / Einmaischen", 25, 20, 50, true},    //Maischen
  {"Glukanaserast (Gummirast)", 39, 20, 10, false},    //Roggenrast
  {"Weizenrasten (Ferularasten)", 46, 15, 10, false},     //Weizenrast
  {"Proteaserast (Eiwei&szlig;rast)", 52, 20, 10, false} ,    
  {"Maltoserast", 63, 30, 10, false},    
  {"Verzuckerung", 72, 30, 10, true},   
  {"Abmaischen", 78, 20, 10, true},    
};*/


const float hysterese_sud = 0.3;    //used for temp regulation

//contains the time when rast was reached
unsigned long timer_rast_start;

//contains the calculated target time
unsigned long timer_rast_target;

//If pause is requested, the time is stored here when it was pressed  to calcualte a new time offset when resuming
unsigned long timer_pause_begin;

//true if the prgrammed rast halt was executed
bool rast_halt_executed;

byte rast_num_current = 255;  //the current tast-numer starting with 0

protocolType protocol;


// --------------------- Debug --------------------------------
BM_Diag diag;

//#define DEBUG_RAST
#ifdef DEBUG_RAST
void debugRast(String s) {
  Serial.print ("------ Rast #"); Serial.print(rast_num_current); Serial.println(" ------");
  Serial.print ("Event: "); Serial.println(s);
  Serial.print ("Millis(): "); Serial.println( millis());
  Serial.print ("Timer Target: "); Serial.println( timer_rast_target);
  Serial.print ("Protocol Heating Time: "); Serial.println( protocol.timeHeatingStart[rast_num_current]);
  Serial.print ("Protocol Rast Time: "); Serial.println( protocol.timeHeatingStart[rast_num_current]);
}
#endif


//#define DEBUG_STATE
#ifdef DEBUG_STATE
void debugState (String loc) {
  String s = loc;
  s += F(" - ");
  s += F("Main State (soll/ist) ");
  s += String ((int) state_main_current);
  s += F(" / ");
  s += String ((int) state_main_requested);
  s += F("; SUB State (soll/ist) ");
  s += String ((int) substate_main_current);
  s += F(" / ");
  s += String ((int) substate_main_requested);
  Serial.println (s);
}
#endif
//------------------------- Steuerung -------------------------
/**
 * Switch to next rast step.
 * - increment rast counter
 * - Initialize timer 
 * If next rast isn't available -> request finish
 */
void inc_rast() {
  if (++rast_num_current < MAX_RASTEN1) {
    diag.debug (F("inc_rast"), "Starting Rast Heating for #" + String(rast_num_current));
    protocol.timeHeatingStart[rast_num_current] = now();
    //Starte rast by calculation the timer
      
    //debug ("inc_rast", "Start " + String( timer_rast_start));
    //debug ("inc_rast", "Rast " + String( rasten[rast_num_current].time));
    timer_rast_target = 0;  //as long as heating is not finished   
    rast_halt_executed = false;
    
    #ifdef DEBUG_RAST
    debugRast(F("inc_rast() started"));
    #endif
  }
  else {
    state_main_requested = STATE_FINISHED;
  }
}



void heating() {

  const __FlashStringHelper* strLoc = F("heating");
  
  if (rast_num_current < MAX_RASTEN1) {

    if (substate_main_current == SUBSTATE_RUNNING_PAUSE)
    {
      relais.switchOff_SUD();
      relais.switchOff_LAUT();
      relais.switchOff_PUMP();
    }
    else if ((substate_main_current == SUBSTATE_RUNNING_HEATING) 
            || (substate_main_current == SUBSTATE_RUNNING_RASTING) 
            || (substate_main_current == SUBSTATE_RUNNING_HALT))
    {
      //Temperature regulation 
      // if temp  is greater than/ equal target temp -> temp reached -> switch off
      // Still heating plus 1 degree
      if ( tempSudValue  > ( rasten[rast_num_current].temp + hysterese_sud)) {
        
          //rast reached!
         relais.switchOff_SUD();
        
         //Is the target temp reached for the first time? -> set timer target
         if (protocol.timeRastStart[rast_num_current] == 0) 
         {
            substate_main_requested = SUBSTATE_RUNNING_RASTING;
            #ifdef DEBUG_RAST
            debugRast("Heating finished! Starting rast");
            #endif
            
            buzzer.rast_start();
            timer_rast_start = millis();   //store time when rast was started
            if (SIMULATION)
              timer_rast_target = ((unsigned long) rasten[rast_num_current].time * 500) + timer_rast_start;  //time == 1 seconds!
            else
              timer_rast_target = ((unsigned long) rasten[rast_num_current].time * 1000 * 60) + timer_rast_start;

            protocol.timeRastStart[rast_num_current] = now();    //remeber the time when heating switched off for the first time -> rasten starts!
         }
         //debug ("heating()", "Switching Sud OFF");
      }
  
      //if it is to cold -> switchOn
      else if ( tempSudValue  < (rasten[rast_num_current].temp - hysterese_sud)) {
          //may be the heating is switched on but the rast was already achieved
         relais.switchOn_SUD();
         //debug ("heating()", "Switching Sud ON");
      }

      //if timer is set AND timer elapsed -> rast is finished; switch to next rast
      //if the system is in Haltmode, timer is ignored until button is pressed
      
      //debug ("heating()", "Millis(): " + String(millis()));
      //debug ("heating()", "timer_rast_target=" + String(timer_rast_target));
      if ((substate_main_current != SUBSTATE_RUNNING_HALT)
          && (timer_rast_target > 0) 
          && (millis() > timer_rast_target)) {
            #ifdef DEBUG_RAST
              debugRast("Timer elapsed!");
            #endif
          buzzer.rast_finished();

          //if rast was finished AND rast HALT is programed -> stop and request state change AND we are not in HALT state AND the Rast Halt was not executed yet
        if (rasten[rast_num_current].wait4User 
            && !rast_halt_executed)
        {
          substate_main_requested = SUBSTATE_RUNNING_HALT;
          diag.debug (strLoc, F("Requesting configured HALT"));
          rast_halt_executed = true;
        }
        else
          inc_rast();
      }

    }
  }
  else {
    state_main_requested = STATE_FINISHED;    //end achieved
  }
}


// =============== Setup============================================================

/**
 * Switch off all relais except mains
 */
void setup_relais() {
  relais.switchOff_SUD();
  relais.switchOff_LAUT();
  relais.switchOff_PUMP();  
}

void initProtocol() {
  protocol.timeStart = 0;
  protocol.timeStop = 0;
  for (byte num=0; num < MAX_RASTEN1; num++) {
    protocol.timeHeatingStart[num] = 0;
    protocol.timeRastStart[num] = 0;
  }
}

// ================================ Switch System state ==============================================
/**
 * React on status change requests.
 * 
 * Diefferent routines may ask for a state chane by defining state_main_request.
 * If the request is valid / allowed, the transiion is forced and the necessary things are done (like swithcing relais).
 */
void trigger_states()
{
  const __FlashStringHelper* strLoc = F("trigger_states()");
    //There is no way out possible from failure state
  if (state_main_current == STATE_FAILURE)
    return;

  boolean allowed = false;

  //Is there a valid change requested?
  if (state_main_requested != state_main_current) {

    // Want to start a program ...
    if (state_main_requested == STATE_RUNNING)
    {
      // ... from Config page
      if (state_main_current == STATE_SETUP) {
          rast_num_current = 255; //next is 0
          initProtocol();
          inc_rast();
          protocol.timeStart = now();
//Serial.print("START:"); Serial.println(protocol.timeStart );          
          substate_main_current = SUBSTATE_RUNNING_INITIAL;
          substate_main_requested  = SUBSTATE_RUNNING_HEATING;
          allowed = true;
      }
    }

    // Automatically after program run.
    else if (state_main_requested == STATE_FINISHED) {
        rast_num_current = 255;
        setup_relais();   //all off except mains
        protocol.timeStop = now();
        buzzer.program_finished();
        allowed = true;
//Serial.print("START:"); Serial.println(protocol.timeStart );
//Serial.print("STOP:"); Serial.println(protocol.timeStop );
    }

     // Want to stop a running ...
    else if (state_main_requested == STATE_CANCELED)
    {
      if (state_main_current == STATE_RUNNING) {
        rast_num_current = 255;
        setup_relais();   //all off except mains
        buzzer.program_canceled();
        allowed = true;
        protocol.timeStop = now();
      }
    }
    // Want to configure a program ...
    if (state_main_requested == STATE_SETUP)
    {
      if (state_main_current == STATE_FINISHED)  {
        
        allowed = true;
      }
      if(state_main_current == STATE_CANCELED) {
        
        allowed = true;
      }
    }

    String s = F("MAIN STATE change from: ");
    s += String((int) state_main_current);
    s += F(" to ");
    s += String((int) state_main_requested);
      
    if (allowed) {
      s += F(" - DONE");
      diag.info (strLoc, s);
      state_main_current = state_main_requested;
      buzzer.ok();
    }
    else {
      s += F(" - IGNORED");
      diag.warn (strLoc, s);
      state_main_requested = state_main_current;
    }
  }
}

/**
 * React on SUBstatus change requests.
 * 
 */
void trigger_substates()
{
  //diag.debug (F("   Requested2"), String (substate_main_requested));
  
  const __FlashStringHelper* strLocation = F("trigger_substates()");
    //There is no way out possible from failure state
  if (state_main_current == STATE_FAILURE)
    return;

  boolean allowed = false;
  
  //diag.debug (F("   Substate"), String (substate_main_current));
  //diag.debug (F("   Requested"), String (substate_main_requested));
  
  //Is there a valid change requested?
  if (substate_main_requested != substate_main_current) {
    //diag.debug (strLocation, F(" change detected "));
    // Program is running and I want to stop/halt the program
    if (state_main_current == STATE_RUNNING)
    {
        //diag.debug (strLocation, F("Change detected when running"));
      
        if (substate_main_requested == SUBSTATE_RUNNING_PAUSE)
        {
          //want to pause a running program
          if ((substate_main_current == SUBSTATE_RUNNING_HEATING) 
              || (substate_main_current == SUBSTATE_RUNNING_RASTING))
          {
            timer_pause_begin = millis();
            allowed = true;
          }
        }
        else if (substate_main_requested == SUBSTATE_RUNNING_HALT)
        {
           allowed = true;
        }

        //Switch to heating || rasting
        else if ((substate_main_requested == SUBSTATE_RUNNING_HEATING)
                || (substate_main_requested == SUBSTATE_RUNNING_RASTING))
        {
          //want to proceed with  running program
          if (substate_main_current == SUBSTATE_RUNNING_PAUSE) {
            timer_rast_target += timer_pause_begin;
            allowed = true;
          }
          //want to proceed with  running program
          else if ((substate_main_current == SUBSTATE_RUNNING_HALT) 
              || (substate_main_current == SUBSTATE_RUNNING_INITIAL))
          {
            allowed = true;
          }
          //from heatin to rasting
          else if ((substate_main_current == SUBSTATE_RUNNING_HEATING) 
                  && (substate_main_requested == SUBSTATE_RUNNING_RASTING))
          {
            allowed = true;
          }
        }  
    }
  

      String s = F("SUBSTATE change from: ");
      s +=  String((int) substate_main_current);
      s += (" to ");
      s += String((int) substate_main_requested);
    if (allowed) {
      s +=  F(" -  DONE");
      diag.info (strLocation, s);
      substate_main_current = substate_main_requested;
      buzzer.ok();
    }
    else {
       s +=  F(" -  IGNORED");
       diag.warn (strLocation, s);
       substate_main_requested = substate_main_current;
    }
    
  }
}

// ================================ SETUP & LOOP ==============================================
void setup() {
  hmi.init();    //LCD
  hmi.start_msg(F("Relais"));
  setup_relais();
  Serial.begin(9600);
  hmi.start_msg(F("Temp. Sensor"));
  bm_temp_init();   //Initialize 3 sensors
  hmi.start_msg(F("Ethernet"));
  init_ethernet();  //Start Web Server
  hmi.start_msg(F("NTP Server"));
  //ntp_init();       //NTP synced clock
  
  state_main_current = STATE_SETUP;    // init state
  state_main_requested = STATE_SETUP;    // init state
  
  hmi.start_msg(F("Memory"));
  estorage.readGlobalRasten();

  hmi.start_msg(F("Done"));
}




void loop() {
  //unsigned long loop_time = millis();     //Zeitmessung
  //17 ms ohne Temp, 650 mit Temp Messung
  
  #ifdef DEBUG_STATE
  debugState ("1");
  #endif
  
  trigger_states();   //check if another state is requested ..
  trigger_substates();
  
  // ---- LCD & Rotary ------
  hmi.showStatus();    //show the current state, check rotary 
  hmi.updateRotaryStatus();
  byte btnBuffer = hmi.readButtonBuffer();
  
  
  //kostet ca 600 ms, wenn der Dallas angesprochen wird (Innentemp)
  updateTempSensorValues (tempInnerValue, tempSudValue, tempLautValue);   //measure and store temperature
  //Serial.print(F("Temperatur innen, sud, läuter: ")); Serial.print(tempInnerValue);Serial.print(", ");Serial.print(tempSudValue);Serial.print(", ");Serial.println(tempLautValue);
  form_input()    ;//show the current state
  
  switch (state_main_current) {
    case STATE_SETUP:
      {
        setup_relais();
         if (btnBuffer == BUTTON_LONG_PUSHED) {
            state_main_requested = STATE_RUNNING;     //proceed with program
         }
      }
      break;
    case STATE_RUNNING:
      {
        heating();

        //Handle key press

        //In Pause - proceed when key was pressed
        if (substate_main_current == SUBSTATE_RUNNING_PAUSE) {
          if (btnBuffer == BUTTON_SHORT_PUSHED) 
            substate_main_requested = SUBSTATE_RUNNING_HEATING;     //proceed with program
        }

         //In Pause - proceed with next Rast when key was pressed
        else  if (substate_main_current == SUBSTATE_RUNNING_HALT) {
          if (btnBuffer == BUTTON_SHORT_PUSHED) 
            substate_main_requested = SUBSTATE_RUNNING_HEATING;     //proceed with program
        }

         //When running - LongPress -> Cancel. ShortPress -> Pause
        else if ((substate_main_current == SUBSTATE_RUNNING_HEATING) 
             ||  (substate_main_current == SUBSTATE_RUNNING_RASTING))
        {
          if (btnBuffer == BUTTON_SHORT_PUSHED) 
            substate_main_requested = SUBSTATE_RUNNING_PAUSE;     //proceed with program
          else if (btnBuffer == BUTTON_LONG_PUSHED) 
            state_main_requested = STATE_CANCELED;     //proceed with program  
        }
      }
      break;
    case STATE_FINISHED:
      {
        setup_relais();
        if (btnBuffer == BUTTON_SHORT_PUSHED)
            state_main_requested = STATE_SETUP;     //proceed with program
      } 
      break;
    case STATE_CANCELED:
      {
        setup_relais();
        if (btnBuffer == BUTTON_SHORT_PUSHED) 
            state_main_requested = STATE_SETUP;     //proceed with program
      }
      break;
    case STATE_FAILURE:
      {
        setup_relais();
      }
      break;
  }


  //Messe Zeit
  //Serial.print (F("loop duration = ")); Serial.print (millis() - loop_time ); Serial.println (F("ms"));
}
