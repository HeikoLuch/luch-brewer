#ifndef bm_global_h
#define bm_global_h
#include "bm_relais.h"
//#include "max6675.h"
#include "bm_estorage.h"
#include "BM_Diag.h"


// ------------- System STATE ------------------

const char STATE_SETUP    = 1;
const char STATE_RUNNING  = 2;
const char STATE_FINISHED = 3;
const char STATE_CANCELED = 4;
const char STATE_FAILURE  = 10;

extern char state_main_current;
extern char state_main_requested;

// --- Subs States for RUNNING ---
const byte SUBSTATE_RUNNING_INITIAL	= 1;			//Before heating starts
const byte SUBSTATE_RUNNING_HEATING = 2;			//Heating phase -> temp not reached yet
const byte SUBSTATE_RUNNING_RASTING	= 3;			//Temp was reached at leats once -> rast started
const byte SUBSTATE_RUNNING_PAUSE 	= 4;			//Program is pausing, Relais OFF
const byte SUBSTATE_RUNNING_HALT 	= 5;			//Halt at end of Rast


extern byte substate_main_current;
extern byte substate_main_requested;
//true if the prgrammed rast halt was executed
extern bool rast_halt_executed;

// ------------- other ------------------

#define MAX_RASTEN1  7    //max Number of Rasten

//simulate program run
extern bool SIMULATION; 
extern BM_Diag diag;

// ----------------- Program Parameter -----------------------

struct brewType {
  const char *name;       //Name prefix of HTML input elements
  unsigned char temp;     //Temperature limit
  unsigned char time;     //max time for "rast"
  unsigned char raise;    //raise of temperature increase
  bool wait4User;         //Shall it continue automatically or wait for user interaction
};

extern brewType rasten[];
extern byte rast_num_current;  //the current rast-number starting with 0. Is 255 if no rast is active
extern unsigned long timer_rast_target;  //contains the calculated target time

// ----------------- Protocol -----------------------


struct protocolType {
	//Unix Time
  unsigned long timeStart; 						// in sec. using now()
  unsigned long timeStop;						// in sec. using now()
  
  unsigned long timeHeatingStart[MAX_RASTEN1];	//When the rast step  was started by heating 
  unsigned long timeRastStart[MAX_RASTEN1];		//When the rast step reached the target temp and the rast pahse started (first time)
};

extern protocolType protocol;


// ------------- Dallas MAX6675 ------------------
// 2 temperature sensors 

//#define MAX6675_SUD1_SO  43   //black
//#define MAX6675_SUD1_CS  47   //green
//#define MAX6675_SUD1_CLK 45   //white

//#define MAX6675_SUD2_SO  42   //black
//#define MAX6675_SUD2_CS  46   //green
//#define MAX6675_SUD2_CLK 44   //white

//extern MAX6675 MAX6675_SUD2;
//extern MAX6675 MAX6675_SUD1;

// Temp values
extern float tempInnerValue;
extern float tempSudValue;
extern float tempLautValue;


// -------- Relais ----------

extern BM_Relais relais;

// -------- EPROM ----------

extern EStorage estorage;

#endif