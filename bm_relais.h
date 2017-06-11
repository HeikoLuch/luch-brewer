#include "Arduino.h"

#ifndef bm_relais_h
#define bm_relais_h

#define MAX_RELAIS 4

class BM_Relais {
	
private:
	//Pins of Relais for SSR_SUD, SSR_LAUT, Mains-Relais, pump
 	byte rpin[MAX_RELAIS] = {22,24,26,28};
 	
	byte switchOff (byte relais);
 	byte switchOn (byte relais);
	byte checkRelaisNum(byte relais);
 
public:
	byte switchOffAll();
	
 	BM_Relais();
 	byte switchOn_SUD();
 	byte switchOff_SUD();
 	byte get_SUD();
 	
 	byte switchOn_LAUT();
 	byte switchOff_LAUT();
 	byte get_LAUT();
 	
 	byte switchOn_MAIN();
 	byte switchOff_MAIN();
 	byte get_MAIN();
 	
 	byte switchOn_PUMP();
 	byte switchOff_PUMP();
 	byte get_PUMP();
};


#endif