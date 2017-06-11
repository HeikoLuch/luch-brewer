
/**
 * Relais Control
 */
#include <Arduino.h>
#include "bm_diag.h"

void BM_Diag::log(const __FlashStringHelper*  prefix, const __FlashStringHelper* loc, String msg) {
	Serial.print (prefix);
	Serial.print (F(": "));
	Serial.print (loc);
	Serial.print (F("() - "));
	Serial.print (millis() - tStart);
	Serial.print (F(" "));
	Serial.println (msg);
}
 
BM_Diag::BM_Diag(){
	tStart = millis();
}

void BM_Diag::debug(const __FlashStringHelper* loc, String msg){
	log(F("DEBUG"), loc, msg);
}

void BM_Diag::debug(const __FlashStringHelper* loc, const __FlashStringHelper* msg){
	log(F("DEBUG"), loc, String(msg));
}

void BM_Diag::info(const __FlashStringHelper* loc, String msg){
	log(F("INFO"),loc, msg);
}
void BM_Diag::warn(const __FlashStringHelper* loc, String msg){
	log(F("WARN"),loc, msg);
}
void BM_Diag::error(const __FlashStringHelper* loc, String msg){
	log(F("ERROR"),loc, msg);
}



