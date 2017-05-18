
// header file

#include "inc/hw_types.h"                                       //  
#include "driverlib/debug.h"                                    // ** for screen display
#include "driverlib/sysctl.h"                                   //     
#include "drivers/rit128x96x4.h"                                //  
                                                                //
//Include hardware memory map, GPIO driver, and PWM driver      //
#include "inc/hw_memmap.h"                                      //  ** for sound
#include "driverlib/gpio.h"                                     //  
#include "driverlib/pwm.h"                                      //

#include <stdlib.h>             //
#include <stdio.h>              //   ** for random number generator
#include <limits.h>             //

#include "inc/hw_ints.h"          //
#include "driverlib/interrupt.h"  //  **for UART
#include "driverlib/uart.h"       //

//--------for pushbutton----------------//
#include "driverlib/systick.h"

//--------for timer---------------//
#include "driverlib/timer.h"

//----------------------------------//


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void __error__(char *pcFilename, unsigned long ulLine){}
#endif


enum myBool {FALSE = 0, TRUE = 1};
typedef enum myBool bool;


// prototypes

//calling dummy functions ahead
int randomInteger(int low, int high); // random number generator
void startup_function(void);
void IntTimer0(void);
void UARTIntHandler(void);
void IntGPIOe(void);
void FrequencyIntGPIOa(void);

void trainCom(void* data);
void schedule(void* data);
void switchControl(void* data);
void currentTrain(void* data);
void serialCom(void* data);
void buildString(char*, char[]);
void frequencyCounter(void);

// defines TCBs
struct MyStruct {
  void (*myTask)(void*);
  void* taskDataPtr;
  struct MyStruct* next;
  struct MyStruct* prev;
};
typedef struct MyStruct TCB;

// defines the structs, not yet created
typedef struct {
  unsigned int* numTrainsPresentP;
  unsigned int* train1SizeP;
  unsigned int* train2SizeP;
  unsigned int* globalCountP;
  unsigned char* State0Ptr;
  unsigned char* State1Ptr;
  unsigned char* State2Ptr;
  unsigned char* State3Ptr;  
  unsigned int* train1TraversalTimeP;
  unsigned int* train2TraversalTimeP;   
  unsigned int* train1DirectionP;
  unsigned int* train2DirectionP;
  char* train1DirectionMessageP;
  char* train1FromP;
  char* train1SizeMessageP;
  char* train2DirectionMessageP;
  char* train2FromP;
  char* train2SizeMessageP;
  char* train1LoadMessageP;
  char* train2LoadMessageP;  
  unsigned int* previousGlobalCountP;
}trainComData;		// struct for trainComData

typedef struct {
  bool* northP;
  bool* eastP;
  bool* westP;
  bool* southP;
  unsigned int* numTrainsPresentP;  
  unsigned int* train1TraversalTimeP;
  unsigned int* train2TraversalTimeP;
  unsigned int* train1DirectionP;
  unsigned int* train2DirectionP;
  unsigned int* train1SizeP;
  unsigned int* train2SizeP;
  unsigned int* globalCountP;
  char* train1DirectionMessageP;
  char* train1FromP;
  char* train1SizeMessageP;
  char* train1LoadMessageP;
  char* train2DirectionMessageP;
  char* train2FromP;
  char* train2SizeMessageP;
  char* train2LoadMessageP;
  char* waitTimeMessageP;
}switchControlData;     // switchControl

typedef struct {
  unsigned int* numTrainsPresentP; 
  unsigned int* globalCountP;
  unsigned char* timerState;
  char* counterStringP;
  unsigned int* previousGlobalCountP;
}scheduleData;          // scheduleData

typedef struct {
  char* train1DirectionMessageP;
  char* train1FromP;
  char* train1SizeMessageP;
  char* train1LoadMessageP;
  char* train2DirectionMessageP;
  char* train2FromP;
  char* train2SizeMessageP;
  char* train2LoadMessageP;
  char* counterStringP;
  char* waitTimeMessageP;
}serialComData;          // serialComData

//edited by young
typedef struct {
  unsigned int* train1DirectionP;
  unsigned int* numTrainsPresentP;
  unsigned int* train1TraversalTimeP;
  char* train1DirectionMessageP;
} currentTrainData;     // currentTrainData

//****************************************************************//