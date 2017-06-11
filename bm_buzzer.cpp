
/**
 * Relais Control
 */
#include "Arduino.h"
#include <bm_buzzer.h>




BM_Buzzer::BM_Buzzer()
{
  pinMode(buzzer_pin, OUTPUT);
  tone(buzzer_pin, 880, 40); 
}


void BM_Buzzer::ok() {
	tone(buzzer_pin,440, 50);
}

void BM_Buzzer::rast_start() {
	/*tone(buzzer_pin,NOTE_C4, 100);
	*/tone(buzzer_pin,NOTE_D4, 100);
	tone(buzzer_pin,NOTE_C2, 100);
}

void BM_Buzzer::rast_finished() {
//	tone(buzzer_pin,NOTE_C4, 200);
	tone(buzzer_pin,NOTE_C5, 200);
}


void BM_Buzzer::program_finished() {
/*	tone(buzzer_pin,NOTE_C4, 200);
	tone(buzzer_pin,NOTE_E4, 200);
	tone(buzzer_pin,NOTE_G4, 200);
*/	tone(buzzer_pin,NOTE_G3, 1000);
}

void BM_Buzzer::program_canceled() {
	/*tone(buzzer_pin,NOTE_DS3, 200);
	tone(buzzer_pin,NOTE_D3, 200);
	*///tone(buzzer_pin,NOTE_CS3, 200);
	tone(buzzer_pin,NOTE_C2, 1500);

}

