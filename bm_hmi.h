#include "Arduino.h"

#ifndef bm_lcd_h
#define bm_lcd_h

	//retunvalue for button state
const byte BUTTON_NOT_PUSHED = 0;
const byte BUTTON_LONG_PUSHED = 1;
const byte BUTTON_SHORT_PUSHED = 2;

class BM_HMI {
	
private:
	void show_braumaschine();
	void show(String s, float v);
	void show(String s1, String s2);
	void showSudTemp();
	void showDauer();
	void showTemp(byte x, byte y, float t);

	// ===================== Rotary ======================
	
	//Variables
	int counter = 0;		//rotary Value 
    int aState;				//
    int aLastState;  		//last STate at rotary encoder
	bool aHasNewValue;
	
    bool bState;
    bool bLastState;		//last STate at push button
	//bool bHasNewValue;
	byte bBuffer;
	unsigned long hl_time;	//ms when H->L was detected
	//unsigned long lh_time	//ms when L->H was detected
	
	//it must take some time before a new value is accepted 
    unsigned long lastRotationUpdate;
	
	// the know was turned -> a new Value exists 
	boolean hasNewRotationValue();
	
	// the current value from -120 to +120
	int getRotationValue();
	
	void setRotationValue(int v);
	
	/*
	// the knob was pushed / releases
	bool hasButtonChanged();
	
	//TRUE is pushed, FALSE is released
	bool getButtonValue();
*/
	
	
public:


	BM_HMI();
	
	/**
	 * Initializing LCD
	 */
	void init();

	/**
	 * Used for showing boot messsages
	 */	
	void start_msg(String msg);
	
	/**
	 * Initializing LCD
	 */
	void print(int x, int y, String s);
	
	/**
	 * Show all values of the current state.
	 * Called in loop
	 */
	void showStatus();
	
	//check PIN's of rotary
	bool updateRotaryStatus(); 
	
	/**
	* Returns BUTTON_NOT_PUSHED, ... and clears buffer
	*/
	byte readButtonBuffer();
	
};
#endif