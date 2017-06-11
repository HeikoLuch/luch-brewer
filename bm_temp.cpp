
/**
 * Muss nach der Initialisierung von Ethernet aufgerufen werden!
 */
#include "Arduino.h"
#include "bm_global.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <bm_temp.h>


// -------------------------------
//#define DEBUG_SHOW_TIME
//#define DEBUG_SHOW_TEMP

#ifdef DEBUG_SHOW_TEMP
void showTemp(String s, float v) {
	Serial.print(s); Serial.println(v);
}
#endif


// -------------------------------

#define SIZE_MEDIAN 3		

#define TIME_INTERVALL 2500		//number of ms between reading from sensors (sensor read 600ms)

//Offset for temp measurement
#define CORR_SUD  -0.5f;
#define CORR_LAUT  0.0f;
#define CORR_INNER  0.0f;


//Address: 

DeviceAddress ProbeInner 	= { 0x28, 0x39, 0x1C, 0x5D, 0x05, 0x00, 0x00, 0x48 }; 
DeviceAddress ProbeSud1 	= { 0x28, 0xFF, 0x44, 0x33, 0xB5, 0x16, 0x05, 0x8E }; 
DeviceAddress ProbeSud2 	= { 0x28, 0xFF, 0xAA, 0x91, 0xC1, 0x16, 0x04, 0x77 }; 

//Messung
//float innerTempC;    //holds the temp
//MAX6675 MAX6675_SUD2 (MAX6675_SUD2_CLK, MAX6675_SUD2_CS, MAX6675_SUD2_SO);
//MAX6675 MAX6675_SUD1 (MAX6675_SUD1_CLK, MAX6675_SUD1_CS, MAX6675_SUD1_SO);


// ------------- Dallas DS18B20 ------------------
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature t1_sensor(&oneWire);

// arrays to hold device address
//DeviceAddress insideThermometer;


void bm_temp_init() {
/*	
	//MAX6675 Temp sensors
  	Serial.print(F("Sensor 1 T[C]=")); 
  	Serial.println(MAX6675_SUD1.readCelsius());
  	Serial.print(F("Sensor 2 T[C]=")); 
  	Serial.println(MAX6675_SUD2.readCelsius());
*/  
  	
  // Inner Temp Sensor
  t1_sensor.begin();
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  t1_sensor.setResolution(ProbeInner, 12);
  t1_sensor.setResolution(ProbeSud1, 12);
  t1_sensor.setResolution(ProbeSud2, 12);
  
  /*
  // search for devices on the bus and assign based on an index.
  if (!t1_sensor.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  // set the resolution to 12 bit (Each Dallas/Maxim device is capable of several different resolutions)
  t1_sensor.setResolution(insideThermometer, 12);
	t1_sensor.requestTemperatures(); // Send the command to get temperatures
  Serial.print(F("Sensor 3 [Resolution = "));
  Serial.print(t1_sensor.getResolution(insideThermometer), DEC);
  Serial.print(F(" Bit] T[C]="));
  Serial.println (t1_sensor.getTempC(insideThermometer));*/
}



/**
* Calcualte the average og temp medians
*/
float average(float median[SIZE_MEDIAN]) {
	float d = 0;
	for (byte i=0; i<SIZE_MEDIAN; i++)
		d += median[i];
	return d / SIZE_MEDIAN;
	
}


/**
 * Ask the sensor for temperature.
 * Store the value in global variable.
 * Do this all TIME_INTERVALL ms.
 */
void updateTempSensorValues (	float& tempInnerValue,
								float& tempSudValue,
								float& tempLautValue) {

		//Check if measurement shall be performed
		static unsigned long lastCall = 0;
		if ((millis() - lastCall) < TIME_INTERVALL) {
			//showTemp(F("waiting="), (millis() - lastCall));
			return;
		}
		
		lastCall = millis();
		//showTemp(F("measuring="), lastCall);
		
		
		static float median_innen[SIZE_MEDIAN];												
		static float median_sud [SIZE_MEDIAN];
		static float median_laut[SIZE_MEDIAN];	
	
		//shift valuesleft and free the last entry
		for (byte i=1; i<SIZE_MEDIAN; i++)
		{
			byte z = i-1;
			median_innen[z] = median_innen[i];												
			median_sud [z] = median_sud [i] ;
			median_laut[z] = median_laut [i] ;
		}											
	
	
		byte z = SIZE_MEDIAN - 1;	// the last entry in median chain
		
		#ifdef DEBUG_SHOW_TIME
		unsigned long t = millis();
		#endif
		
		t1_sensor.requestTemperatures(); // Send the command to get temperatures; takes approx. 600 ms
		
		#ifdef DEBUG_SHOW_TIME
		Serial.print (F("Temp Request duration [ms]: ")); Serial.println (millis() -t); 
		t = millis();
		#endif

		median_innen[z] = t1_sensor.getTempC(ProbeInner);
		
		#ifdef DEBUG_SHOW_TIME
		Serial.print (F("get inner temp in [ms]: ")); Serial.println (millis() -t);
		t = millis();
		#endif
		
		#ifdef DEBUG_SHOW_TEMP
		showTemp(F("Temp innen C: "),tempInnerValue);
		#endif
		
		median_sud[z] = t1_sensor.getTempC(ProbeSud1);
		#ifdef DEBUG_SHOW_TIME
		Serial.print (F("get SUD temp in [ms]: ")); Serial.println (millis() -t);
		t = millis();
		#endif
		
		#ifdef DEBUG_SHOW_TEMP
		showTemp(F("Temp Sud: "), median_sud[z]);
		#endif
		
		median_laut[z] = t1_sensor.getTempC(ProbeSud2);
		
		#ifdef DEBUG_SHOW_TIME
		Serial.print (F("get LAUT temp in [ms]: ")); Serial.println (millis() -t);
		t = millis();
		#endif
		
		#ifdef DEBUG_SHOW_TEMP
		showTemp (F("Temp Laut: "), median_laut[z]);  
		#endif

		tempInnerValue = average (median_innen) + CORR_INNER;
		tempSudValue = average (median_sud) + CORR_SUD;
		tempLautValue = average (median_laut) + CORR_LAUT;		

    	
}
