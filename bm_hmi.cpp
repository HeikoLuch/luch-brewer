/**
 * Muss nach der Initialisierung von Ethernet aufgerufen werden!
 */
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "bm_hmi.h"
#include "bm_global.h"
#include "NTP-Clock.h"


// ---------- Rotary ---------------

#define KEY_PIN_A 41
#define KEY_PIN_B 43
#define KEY_PIN_C 45

#define MIN -120
#define MAX 120
#define KEY_TIME_IGNORE 150 //burst may be recognized as ticks -> wait at least ms before accepting new ticks

#define BUTTON_DURATION_SHORT 800		// max duration of a short button press
#define BUTTON_DURATION_LONG 1500		// min duration of a long button press

// ------------- LCD ------------------
#define UPDATE_INTERVALL 500				//how often shall be the creen updated 
#define SCREEN_INTERVALL 3000				//how fast shall the screen toggle?

// select the pins used on the LCD panel
LiquidCrystal lcd(12, 11, 30, 32, 34, 36);

// ------------- INIT ------------------

BM_HMI::BM_HMI() {
	aHasNewValue = false;
}

void BM_HMI::init() {
 //LCD init
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0, 0);
  lcd.clear();
  
  //Rotary
  pinMode (KEY_PIN_A, INPUT);
  pinMode (KEY_PIN_B, INPUT);
  pinMode (KEY_PIN_C, INPUT);
       
  // Reads the initial state of the outputA
  aLastState =  digitalRead(KEY_PIN_A); 
  bLastState =  digitalRead(KEY_PIN_C);
}

// ========================= Rotary Encoder =========================

// the know was turned -> a new Value exists 
boolean BM_HMI::hasNewRotationValue() {
	return aHasNewValue;
}
	
// the current value from -120 to +120 (-128 to +127)
int BM_HMI::getRotationValue() {
	aHasNewValue = false;
	return counter;
}
	
void BM_HMI::setRotationValue(int v) {
	counter = v;
}

	
/**
 * Returns BUTTON_NOT_PUSHED, ... and clears buffer
 */	
byte BM_HMI::readButtonBuffer() {
	byte retValue = bBuffer;
	
	//if (hl_time == 0)	// is 0 if the button is HIGH again
		bBuffer = BUTTON_NOT_PUSHED;
	return retValue;
}	
	
	
bool BM_HMI::updateRotaryStatus () { 
	//check rotation
    aState = digitalRead(KEY_PIN_A); // Reads the "current" state of the outputA
	// If the previous and the current state of the outputA are different, that means a Pulse has occured
    if ((aState != aLastState) && ((millis()-lastRotationUpdate) > KEY_TIME_IGNORE))
	{
		aHasNewValue = true;
		
		// If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
        if (digitalRead(KEY_PIN_B) != aState) { 
           if (++counter > MAX) 
			   counter = MAX;				//set limit
        } 
		else {
           if (--counter < MIN)
			   counter = MIN;				//set limit
        }
        Serial.print(millis()-lastRotationUpdate);
        Serial.print(F(" - Position: "));
        Serial.println(counter);
		
        lastRotationUpdate = millis(); // store last update
		aLastState = aState; // Updates the previous state of the outputA with the current state
	} 
    

    //check Button
    bState =  digitalRead(KEY_PIN_C);
    if (bState != bLastState) 
    {
		Serial.println(F("Button state changed."));
		
		//Wenn man abrutscht -> wird dadurch ignoriert
		lastRotationUpdate = millis(); // store last update
		//bHasNewValue = true;
		
		if (bLastState == HIGH)	//H -> L edge
		{
			if (hl_time == 0)
				hl_time = millis();
		}
		else 	//L -> H edge
		{
			//unsigned long m = millis();
			if ((millis() - hl_time) < BUTTON_DURATION_SHORT)
				bBuffer = BUTTON_SHORT_PUSHED;
			
			hl_time = 0;
		}
		
		bLastState = bState;
		if (bState == HIGH)
			Serial.println(F("Button released!"));
		else
			Serial.println(F("Button pushed!"));
    }
	//check for long push
	else if ((bState == bLastState) 
		&& (bState == LOW)
		&& (hl_time>0)) {						//is button still pushed? And buffer not read yet?
			if ((millis() - hl_time) > BUTTON_DURATION_LONG) {	
				bBuffer = BUTTON_LONG_PUSHED;
				hl_time = 0;
				Serial.println(F("Long PUSH detected"));
			}
		}
	
}

	
// ========================= LCD =========================


void BM_HMI::print(int x, int y, String s){
	lcd.setCursor(x, y);           // move to the begining of the second line
	lcd.print (s);
}


void BM_HMI::show_braumaschine() {
	lcd.setCursor(0, 0);
	lcd.print (F("Luch-Brauer"));
}



void BM_HMI::start_msg(String msg) {
	lcd.clear();
	show_braumaschine();
	print(0, 1, F("Init:"));
	print(6, 1, msg);
}


void BM_HMI::showTemp(byte x, byte y, float t) {
	//show temp
	String s = String (t, 1);
	print ( x, y, s);
	char grad = 0xDF;
	print ( x +4, y, String(grad));
	print ( x + 5, y, F("C"));
}


void BM_HMI::showSudTemp() {
	//show temp
	print ( 0, 1, F("SUD"));
	showTemp ( 4, 1, tempSudValue);
	/*
	String s = String (tempSudValue, 1);
	print ( 4, 1, s);
	char grad = 0xDF;
	print ( 8, 1, String(grad));
	print ( 9, 1, F("C"));*/
}


void BM_HMI::showDauer() {
	print ( 0, 1, F ("Dauer"));
	
	
	String s = String((protocol.timeStop - protocol.timeStart) /60);
	if (protocol.timeStop == 0)		//for cancelation 
		s = String((now() - protocol.timeStart) /60) + 1;
	
	Serial.print(F("protocol.timeStart: ")); Serial.println (protocol.timeStart);
	Serial.print(F("protocol.timeStop: ")); Serial.println (protocol.timeStop);
		
	int i = s.length();
	print ( 6, 1, s);
		
	print ( 6 + i + 1, 1, F ("min"));
}

void BM_HMI::showStatus() {
		
	//number of screen to be shown
	static byte screen_cnt = 0; 	
	screen_cnt++;
	
	
	//avoid flickering by updating all x ms
	static unsigned long lastCall = 0;
	if ((millis() - lastCall) < UPDATE_INTERVALL)
		return;
	else
		lastCall = millis();
	
	String timeStr = ntp_time_string();
	lcd.clear();
	if (state_main_current == STATE_SETUP) {
		print ( 0, 0, timeStr);
		showSudTemp();
		
	}
	else if (state_main_current == STATE_RUNNING) {
		
		if (screen_cnt > 1) screen_cnt=0;
		
		//Name of rast or target temp
		if (screen_cnt == 1) {
			print ( 0, 0, F("SOLL"));
			showTemp(5, 0, rasten[rast_num_current].temp);
			//print ( 2, 0, rasten[rast_num_current].name);			
		} 
		else {
			print ( 0, 0, String (rast_num_current + 1));
			print ( 2, 0, rasten[rast_num_current].name);
		}
		
		//Sud temp		
		showSudTemp();
		
		String s = String(F("???"));
		if (substate_main_current == SUBSTATE_RUNNING_INITIAL) {
			s = F("Init");
		}
		else if (substate_main_current == SUBSTATE_RUNNING_HEATING) {
			s = F("Heizen");
		}
		else if (substate_main_current == SUBSTATE_RUNNING_RASTING) {
			s = String ((timer_rast_target - millis())/1000);	//and time being left
		}
		else if (substate_main_current == SUBSTATE_RUNNING_PAUSE) {
			s = F("Pause");	
		}
		else if (substate_main_current == SUBSTATE_RUNNING_HALT) {
			s = F("Halt");
		}
		print ( 11, 1, s);
	}
	else if (state_main_current == STATE_FINISHED) {
		print ( 0, 0, F ("Programm Ende"));
		showDauer();
	}
	else if (state_main_current == STATE_CANCELED) {
		print ( 0, 0, F ("Programm-Abbruch"));
		showDauer();
	}
	else {
		print ( 0, 0, F("Status unbekannt"));
		int z = state_main_current;
		print ( 0, 1, String(z));
	}
}


// ================= Utilities ====================
// for debugging
void BM_HMI::show(String s, float v) {
	Serial.print(s); Serial.println(v);
}

void BM_HMI::show(String s1, String s2) {
	Serial.print(millis()); Serial.print(F(": ")); Serial.print(s1); Serial.print(F("() - ")); Serial.println(s2);
}


/*
// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;

  

  return btnNONE;  // when all others fail, return this...
}
*/

/*
void handleLcd()
{
  lcd.setCursor(9, 1);           // move cursor to second line "1" and 9 spaces over
  lcd.print(millis() / 1000);    // display seconds elapsed since power-up


  lcd.setCursor(0, 1);           // move to the begining of the second line
  lcd_key = read_LCD_buttons();  // read the buttons

  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
      {
        lcd.print("RIGHT ");
        break;
      }
    case btnLEFT:
      {
        lcd.print("LEFT   ");
        break;
      }
    case btnUP:
      {
        lcd.print("UP    ");
        break;
      }
    case btnDOWN:
      {
        lcd.print("DOWN  ");
        break;
      }
    case btnSELECT:
      {
        lcd.print("SELECT");
        break;
      }
    case btnNONE:
      {
        lcd.print("NONE  ");
        break;
      }
  }
}
*/


