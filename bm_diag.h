#include "Arduino.h"
#include <avr/pgmspace.h>

#ifndef bm_diag_h
#define bm_diag_h



class BM_Diag {

	
	unsigned long tStart;			//time when initialized
	
	void log(const __FlashStringHelper* prefix, const __FlashStringHelper* loc, String msg);
 
public:
	
 	BM_Diag();
 	void debug(const __FlashStringHelper* loc, String msg);
	void debug(const __FlashStringHelper* loc, const __FlashStringHelper* msg);
	
	void info(const __FlashStringHelper* loc, String msg);
	void warn(const __FlashStringHelper* loc, String msg);
	void error(const __FlashStringHelper* loc, String msg);
};


#endif