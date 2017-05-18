// for time measuring
#include "inc/lm3s8962.h"

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
void trainCom(void* data);
void schedule(void* data);
void switchControl(void* data);
void currentTrain(void* data);
void serialCom(void* data);
void buildString(char*, char[]);

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
  bool* northP;
  bool* westP;
  bool* eastP;
  bool* southP;
  bool* trainPresentP;
  bool* gridLockP;
  unsigned int* trainSizeP;
  unsigned int* globalCountP;
  unsigned int* trainComFlagP;
  unsigned int* serialComFlagP;
  unsigned char* State0Ptr;
  unsigned char* State1Ptr;
  unsigned char* State2Ptr;
  unsigned char* State3Ptr;  
  char* trainDirectionMessageP;
  char* trainSizeMessageP;
  char* trainFromP;
}trainComData;		// struct for trainComData

typedef struct {
  bool* northP;
  bool* eastP;
  bool* westP;
  bool* southP;
  bool* gridLockP;
  bool* trainPresentP;  
  unsigned int* traversalTimeP;
  unsigned int* trainSizeP;
  unsigned int* globalCountP;
  unsigned int* gridLockDelayTimeP;
  unsigned int* switchControlFlagP;
  unsigned int* serialComFlagP;
  char* gridlockMessageP;
  char* trainDirectionMessageP;
  char* trainFromP;
  char* trainSizeMessageP;
}switchControlData;     // switchControl

typedef struct {
  bool* gridLockP;
  bool* trainPresentP; 
  unsigned int* globalCountP;
  unsigned int* currentTrainFlagP;
  unsigned int* switchControlFlagP;
  unsigned int* serialComFlagP;
  unsigned int* trainComFlagP;
  unsigned char* timerState;
  struct MyStruct* trainComPtr;
  struct MyStruct* switchControlPtr;
  struct MyStruct* currentTrainPtr;
  struct MyStruct* serialComPtr;
  char* counterStringP;
}scheduleData;          // scheduleData

typedef struct {
  char* gridlockMessageP;
  char* trainDirectionMessageP;
  char* trainSizeMessageP;
  char* trainFromP;
  unsigned int* serialComFlagP;
  char* counterStringP;
}serialComData;          // serialComData

typedef struct {
  bool* eastP;
  bool* northP;
  bool* westP;
  bool* southP;
  bool* trainPresentP;
  unsigned int* traversalTimeP; 
  unsigned int* currentTrainFlagP;
  char* trainDirectionMessageP;
  char* trainSizeMessageP;
  char* trainFromP;
} currentTrainData;     // currentTrainData

//****************************************************************//