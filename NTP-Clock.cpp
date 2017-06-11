/**
 * Muss nach der Initialisierung von Ethernet aufgerufen werden!
 */
#include <Arduino.h>
#include <UIPUdp.h>
#include "NTP-Clock.h"

//#define DEBUG_NTP

#define NTP_SERVER 132, 163, 4, 101 // time-a.timefreq.bldrdoc.gov

// local port to listen for UDP packets
#define LOCAL_PORT 8888  

//const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)
#define TIME_ZONE 1

EthernetUDP ntp_udp;

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

bool isSynced = false;

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  ntp_udp.beginPacket(address, 123); //NTP requests are to port 123
  ntp_udp.write(packetBuffer, NTP_PACKET_SIZE);
  ntp_udp.endPacket();
}

/**
* Convert INT to String and fill leading '0' if necessary
*/
String convert2String(int i) {
	String s = "";
	s += i;
	if (s.length() == 1)
		return "0" + s;
	return s;
}

/**
* Returns hh:mm:ss
*/
String time2TimeString(time_t t) {
	// digital clock display of the time
 	
  String s = F("");
  s += convert2String (hour(t));
  s += F(":");
  s += convert2String (minute(t));
  s += F(":");
  s += convert2String (second(t));
  return s; 
}


/**
* Returns dd.mm.yyyy
*/
String time2DateString(time_t t) {
	// digital clock display of the time
 	
  String s = F("");
  s += convert2String (day(t));
  s += F(".");
  s += convert2String (month(t));
  s += F(".");
  s += year(t); 
  return s; 
}

/**
* Returns "hh:mm:ss  dd.mm.yyyy"
*/
String time2String(time_t t) {
	String s = F("");
	s += time2TimeString(t);
	s += F("  ");
	s += time2DateString (t);
	return s; 
}


/**
* Syncs the internal clock with NTP.
* All further requests are handled by the internal clock.
 
*/
time_t getNtpTime()
{
	IPAddress timeServer(NTP_SERVER);
  while (ntp_udp.parsePacket() > 0) ; // discard any previously received packets
  
  #ifdef DEBUG_NTP
  Serial.print(F("Transmit NTP Request to TimeServer: "));
  Serial.println(timeServer);
  #endif
  
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = ntp_udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
	  
	  #ifdef DEBUG_NTP
      Serial.println(F("Receive NTP Response"));
	  #endif
	  
      ntp_udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
	  isSynced = true;
      return secsSince1900 - 2208988800UL + TIME_ZONE * SECS_PER_HOUR;
    }
  }
  Serial.println(F("No NTP Response :-("));
  isSynced = false;
  return 0; // return 0 if unable to get the time
}


void ntp_init() {
	#ifdef DEBUG_NTP
	Serial.println (F("Initializing NTP-Clock"));
  	Serial.println (F("	Starting UDP ..."));
	#endif
	
  	ntp_udp.begin(LOCAL_PORT);
	
	#ifdef DEBUG_NTP
  	Serial.println (F("	Requesting time ..."));
	#endif
	
  	setSyncProvider(getNtpTime);
	setSyncInterval(10);	//resync all x seconds
}



String ntp_time_string() {
	
	if (!isSynced) {
		ntp_init();
	}
	
	
  // digital clock display of the time
  
  String s = F("");
  s += convert2String (hour());
  s += F(":");
  s += convert2String (minute());
  s += F(":");
  s += convert2String (second());
  s += F("  ");
  s += convert2String (day());
  s += F(".");
  s += convert2String (month());
  s += F(".");
  s += year(); 
  return s; 
}


boolean isSyncedNtp() {
	return isSynced;
}



//========================================================
 
