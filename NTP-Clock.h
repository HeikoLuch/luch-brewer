#include "Arduino.h"
#include <TimeLib.h>


#ifndef NTP_CLOCK_h
#define NTP_CLOCK_h

//time_t getNtpTime();
//void ntp_init();


/**
 * Returns String like  "12:13:59 25.06."
 *
 */
String ntp_time_string();

String time2TimeString(time_t t);

/**
 * Is the clock successfully synchonized with WWW?
 * may be used to check if there is still an internet connection.
 *
 */
boolean isSyncedNtp();

/**
* Returns dd.mm.yyyy
*/
//String time2DateString(time_t t) ;

/**
* Returns "hh:mm:ss  dd.mm.yyyy"
*/
//String time2String(time_t t);

#endif