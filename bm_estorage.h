#ifndef estorage_h
#define estorage_h

/**
 * Von hier: https://playground.arduino.cc/Code/EEPROMWriteAnything
 */
// -------------------------------

#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

// --- Rast storage ---
//we have 7 rast a 4 byte
#define START_RAST 0;		//Start here to store Rasts
#define SIZE_RAST 4;		//required size to store one Rast

// --- Config storage ---
#define START_CONFIG 29;		//Start here to store Rasts


class EStorage {

	private:
		
	
	public:
		EStorage();
		//void storeRast(byte rastNum, brewType bt);
		//void readRast (byte rastNum, brewType& bt);

		
		void storeGlobalRasten();		//Store value of global rast variable within eeprom
		void readGlobalRasten ();
};





#endif