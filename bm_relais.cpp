
/**
 * Relais Control
 */
#include <Arduino.h>
#include "bm_relais.h"


//const byte MAX = 4;		//max num of relais

byte OK = 1;				//common return value
byte NOK = 0;				//common return value

byte rstatus [MAX_RELAIS] =  {0,0,0,0};	//last status of the relais



BM_Relais::BM_Relais()
{
	
	for (byte i=0; i < MAX_RELAIS; i++)
		pinMode(rpin[i], OUTPUT);
 	switchOffAll();
}


 	byte BM_Relais::switchOn_SUD()
 	{
 		return switchOn (0);
 	}
 	
 	byte BM_Relais::switchOff_SUD()
 	{
 		return switchOff (0);
 	}
 	
 	byte BM_Relais::get_SUD()
 	{
 		return rstatus [0];
	} 	
 	
 	
 	
 	byte BM_Relais::switchOn_LAUT()
 	{
 		return switchOn (1);
 	}
 	
 	byte BM_Relais::switchOff_LAUT()
 	{
 		return switchOff (1);
 	}
 	
 	byte BM_Relais::get_LAUT()
 	{
 		return rstatus [1];
	}
 	
 	
 	byte BM_Relais::switchOn_MAIN()
 	{
 		return switchOn (2);
 	}
 	
 	byte BM_Relais::switchOff_MAIN()
 	{
 		return switchOff (2);
 	}

 	byte BM_Relais::get_MAIN()
 	{
 		return rstatus [2];
	}

 	
 	
 	byte BM_Relais::switchOn_PUMP()
 	{
 		return switchOn (3);
 	}
 	
 	byte BM_Relais::switchOff_PUMP()
 	{
 		return switchOff (3);
 	}
 	
  	byte BM_Relais::get_PUMP()
 	{
 		return rstatus [3];
	}

 	

// ---- private ---------

byte BM_Relais::checkRelaisNum(byte relais) {
	if ((relais < MAX_RELAIS)&& (relais >= 0)) 
		return OK;
	return NOK;
}


byte BM_Relais::switchOn (byte relais) {
	if (OK == checkRelaisNum(relais)) {
		digitalWrite( rpin[relais], HIGH);
		rstatus [relais] = 1;
		return OK;  		
	}
	return NOK;
}


byte BM_Relais::switchOff (byte relais) {
	if (OK == checkRelaisNum(relais)) {
		digitalWrite( rpin[relais], LOW);
		rstatus [relais] = 0;
  		return OK;
	}
	return NOK;
}

byte BM_Relais::switchOffAll() {
	byte status = OK;
	for (byte c=0; c < MAX_RELAIS && (status == OK); c++){
		status = switchOff(c);
	}
	return status;
}





