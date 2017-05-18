/*
    FreeRTOS V7.0.1 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution and has been modified to 
    demonstrate three simple tasks running.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


/*
 * Creates all the application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */




/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Set the following option to 1 to include the WEB server in the build.  By
default the WEB server is excluded to keep the compiled code size under the 32K
limit imposed by the KickStart version of the IAR compiler.  The graphics
libraries take up a lot of ROM space, hence including the graphics libraries
and the TCP/IP stack together cannot be accommodated with the 32K size limit. */

//  set this value to non 0 to include the web server

#define mainINCLUDE_WEB_SERVER		1

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */

#include "hw_sysctl.h"
#include "grlib.h"
#include "osram128x64x4.h"
#include "formike128x128x16.h"

/* Demo app includes. */

#include "lcd_message.h"
#include "bitmap.h"

#include "header.h"
#include "constantDefs.h"

#include "sysctl.h"
#include "hw_memmap.h"
#include "adc.h"
#include "rom.h"

/*-----------------------------------------------------------*/

/* 
  The time between cycles of the 'check' functionality (defined within the
  tick hook. 
*/
#define mainCHECK_DELAY	( ( portTickType ) 5000 / portTICK_RATE_MS )

// Size of the stack allocated to the uIP task.
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 3 )

// The OLED task uses the sprintf function so requires a little more stack too.
#define mainOLED_TASK_STACK_SIZE	    ( configMINIMAL_STACK_SIZE + 50 )

//  Task priorities.
#define mainQUEUE_POLL_PRIORITY		    ( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY		    ( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY		    ( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY		    ( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY	    ( tskIDLE_PRIORITY )


//  The maximum number of messages that can be waiting for display at any one time.
  #define mainOLED_QUEUE_SIZE					( 3 )

// Dimensions the buffer into which the jitter time is written. 
  #define mainMAX_MSG_LEN						25

/* 
  The period of the system clock in nano seconds.  This is used to calculate
  the jitter time in nano seconds. 
*/

#define mainNS_PER_CLOCK ( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )


// Constants used when writing strings to the display.

#define mainCHARACTER_HEIGHT		    ( 9 )
#define mainMAX_ROWS_128		    ( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96			    ( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64			    ( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE			    ( 15 )
#define ulSSI_FREQUENCY			    ( 3500000UL )

/*-----------------------------------------------------------*/

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */

/*
 * Configure the hardware .
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
extern void vSetupHighFrequencyTimer( void );


/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook( void );

/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
unsigned char timerState = 0;
char counterString[COUNTER_STRING_SIZE] = INITIAL_COUNTER_STRING;

// direction states
unsigned char State0 = 0;
unsigned char State1 = 0;
unsigned char State2 = 0;
unsigned char State3 = 0;

//Variables for train and gridlock info
unsigned int numTrainsPresent = 0;
unsigned int train1Size = 0;
unsigned int train2Size = 0;
unsigned int train1TraversalTime = 0;
unsigned int train2TraversalTime = 0;

unsigned int train1Direction = -1;
unsigned int train2Direction = -1;

//Global counter variable
unsigned int globalCount = 0;
unsigned int previousGlobalCount = 0;

//Initialize the data struct for each task       
trainComData myTrainComData;   
switchControlData mySwitchControlData;
scheduleData myScheduleData;
currentTrainData myCurrentTrainData;
serialComData mySerialComData;

//Global strings for gridlock alarm and all train information
char train1DirectionMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train1SizeMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train1FromMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train1LoadMessage[MAX_STRING_SIZE] = EMPTY_STRING;

char train2DirectionMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train2SizeMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train2FromMessage[MAX_STRING_SIZE] = EMPTY_STRING;
char train2LoadMessage[MAX_STRING_SIZE] = EMPTY_STRING;

char waitTimeMessage[MAX_STRING_SIZE] = EMPTY_STRING;
unsigned int trainComFlag = 0;

xSemaphoreHandle trainComSemaphore = NULL;
xSemaphoreHandle serialComSemaphore= NULL;
xSemaphoreHandle switchControlSemaphore= NULL;
xSemaphoreHandle currentTrainSemaphore= NULL;
xSemaphoreHandle scheduleSemaphore = NULL;

unsigned int frequencyCount = 0;
unsigned int frequency = 0;
unsigned int payload = 0;
double slope = 0.3;

unsigned int suspend = 1;
unsigned int OLED_Disable = 0;

// this is for storing samples
unsigned long ulADCval[8]; //store the samples
int voltage;


int main( void )
{
    prvSetupHardware();

    //Initialize trainCom data struct's fields
    myTrainComData.train1DirectionP = &train1Direction;
    myTrainComData.train2DirectionP = &train2Direction;
    myTrainComData.globalCountP = &globalCount;
    myTrainComData.numTrainsPresentP = &numTrainsPresent;
    myTrainComData.train1SizeP = &train1Size;
    myTrainComData.train1DirectionMessageP = train1DirectionMessage;
    myTrainComData.train1SizeMessageP = train1SizeMessage;
    myTrainComData.train1FromP = train1FromMessage;
    myTrainComData.train1LoadMessageP = train1LoadMessage;
    myTrainComData.train2DirectionMessageP = train2DirectionMessage;
    myTrainComData.train2SizeMessageP = train2SizeMessage;
    myTrainComData.train2FromP = train2FromMessage;
    myTrainComData.train2LoadMessageP = train2LoadMessage;
    myTrainComData.train2SizeP = &train2Size;
    myTrainComData.State0Ptr = &State0;
    myTrainComData.State1Ptr = &State1;
    myTrainComData.State2Ptr = &State2;
    myTrainComData.State3Ptr = &State3;   
    myTrainComData.train1TraversalTimeP = &train1TraversalTime;
    myTrainComData.train2TraversalTimeP = &train2TraversalTime;
    myTrainComData.previousGlobalCountP = &previousGlobalCount;
    
    //Initialize switchControl struct's fields
    mySwitchControlData.train1DirectionP = &train1Direction;
    mySwitchControlData.train2DirectionP = &train2Direction;
    mySwitchControlData.globalCountP = &globalCount;
    mySwitchControlData.numTrainsPresentP = &numTrainsPresent;
    mySwitchControlData.train1SizeP = &train1Size;
    mySwitchControlData.train1TraversalTimeP = &train1TraversalTime;
    mySwitchControlData.train2TraversalTimeP = &train2TraversalTime;
    mySwitchControlData.train1SizeP = &train1Size;
    mySwitchControlData.train1DirectionMessageP = train1DirectionMessage;
    mySwitchControlData.train1SizeMessageP = train1SizeMessage;
    mySwitchControlData.train1LoadMessageP = train1LoadMessage;
    mySwitchControlData.train1FromP = train1FromMessage;
    mySwitchControlData.train2DirectionMessageP = train2DirectionMessage;
    mySwitchControlData.train2SizeMessageP = train2SizeMessage;
    mySwitchControlData.train2FromP = train2FromMessage;
    mySwitchControlData.train2LoadMessageP = train2LoadMessage;
    mySwitchControlData.train2SizeP = &train2Size;
    mySwitchControlData.waitTimeMessageP = &waitTimeMessage;
    
    //Initialize currentTrain struct fields
    myCurrentTrainData.train1DirectionP = &train1Direction;
    myCurrentTrainData.numTrainsPresentP = &numTrainsPresent;
    myCurrentTrainData.train1TraversalTimeP = &train1TraversalTime;
    myCurrentTrainData.train1DirectionMessageP = train1DirectionMessage;

    //Initialize Schedule struct's fields
    myScheduleData.globalCountP = &globalCount;
    myScheduleData.timerState = &timerState;
    myScheduleData.numTrainsPresentP = &numTrainsPresent;
    myScheduleData.counterStringP = counterString;
    myScheduleData.previousGlobalCountP = &previousGlobalCount;

    //Initialize serialCom structs fields
    mySerialComData.train1DirectionMessageP = train1DirectionMessage;
    mySerialComData.train1SizeMessageP = train1SizeMessage;
    mySerialComData.train1LoadMessageP = train1LoadMessage;
    mySerialComData.train1FromP = train1FromMessage;
    mySerialComData.train2DirectionMessageP = train2DirectionMessage;
    mySerialComData.train2SizeMessageP = train2SizeMessage;
    mySerialComData.train2FromP = train2FromMessage;
    mySerialComData.train2LoadMessageP = train2LoadMessage;
    mySerialComData.counterStringP = counterString;
    mySerialComData.waitTimeMessageP = &waitTimeMessage;
     
    
    //-----------------------------------------------------------------------------
    
    
    vSemaphoreCreateBinary(trainComSemaphore);
    vSemaphoreCreateBinary(serialComSemaphore);
    vSemaphoreCreateBinary(currentTrainSemaphore);
    vSemaphoreCreateBinary(switchControlSemaphore);
    vSemaphoreCreateBinary(scheduleSemaphore);
    
    xSemaphoreTake(trainComSemaphore, 20);
    xSemaphoreTake(serialComSemaphore, 20);
    xSemaphoreTake(currentTrainSemaphore, 20);
    xSemaphoreTake(switchControlSemaphore, 20);
    xSemaphoreTake(scheduleSemaphore, 20);
    
    RIT128x96x4StringDraw(TRAIN_TITLE_MESSAGE, TRAIN_1_X, TRAIN_1_Y, brightness);
    RIT128x96x4StringDraw(TO_MESSAGE, x_coor, TRAIN_TO_Y, brightness);
    RIT128x96x4StringDraw(FROM_MESSAGE, x_coor, TRAIN_FROM_Y, brightness);
    RIT128x96x4StringDraw(SIZE_MESSAGE, x_coor, TRAIN_SIZE_Y, brightness);
    RIT128x96x4StringDraw(LOAD_MESSAGE, x_coor, TRAIN_LOAD_Y, brightness);
    RIT128x96x4StringDraw(WAIT_MESSAGE, x_coor, WAIT_Y, brightness);
    
    /* 
        Exclude some tasks if using the kickstart version to ensure we stay within
        the 32K code size limit. 
    */
    
    #if mainINCLUDE_WEB_SERVER != 0
    {
      /* 
          Create the uIP task if running on a processor that includes a MAC and PHY. 
      */
      
      if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) )
      {
          xTaskCreate( vuIP_Task, ( signed portCHAR * ) "uIP", mainBASIC_WEB_STACK_SIZE, NULL, 4, NULL );
      }
    }
    #endif
     
    
    /* Start the tasks */
   // xTaskCreate( vOLEDTask, ( signed portCHAR * ) "OLED", mainOLED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
    
    xTaskCreate(trainCom, "Train Com", 1024, (void*)&myTrainComData, 1, NULL);
    xTaskCreate(switchControl, "Switch Control", 1024, (void*)&mySwitchControlData, 3, NULL);
    xTaskCreate(currentTrain,"Current Train", 1024, (void*)&myCurrentTrainData, 1, NULL);
    xTaskCreate(schedule,"Schedule", 1024, (void*)&myScheduleData, 5 , NULL);
    xTaskCreate(serialCom,"Serial Com", 1024, (void*)&mySerialComData,1, NULL);
    
    /* 
      Configure the high frequency interrupt used to measure the interrupt
      jitter time. 
    */
    vSetupHighFrequencyTimer();

    /* 
      Start the scheduler. 
    */
    
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle task. */
    
    return 0;
}

/* 
Payload calculator.
if frequency is lower than 1000, payload = 0
if frequency is higher than 2000, payload = 300
if frequency is range from 1000 to 2000, then payload will be between 0 and 300 
which calculated by the equation.
*/
void frequencyCounter(void){
  
  frequency = frequencyCount;
  
  if( frequency < 1000) { 
     payload = 0;
  } else if (frequency > 2000) {
    payload = 300;
  } else {
    payload = (int)(double)(frequency - 1000)*slope;
  }
  
  frequencyCount = 0;
  
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    ( void ) pxTask;
    ( void ) pcTaskName;
  
    while( 1 );
}

/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* 
      If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
      a workaround to allow the PLL to operate reliably. 
    */
      if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }	  
	
    // Set the clocking to run from the PLL at 50 MHz
    SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );
    /* 	
      Enable Port F for Ethernet LEDs
            LED0        Bit 3   Output
            LED1        Bit 2   Output 
    */  
    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
    GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
    GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );	
	
    // Initialize the OLED display.
    RIT128x96x4Init(OLED_FREQUENCY);
    
    //---------------------GPIO--Port E---------------------------------//
            
    //Clear the default ISR handler and install IntGPIOe as the handler:
    GPIOPortIntUnregister(GPIO_PORTE_BASE);

    //Enable GPIO port E, set pin 0 as an input
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);    
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
     
    //Activate the pull-up on GPIO port E
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //Configure GPIO port E as triggering on falling edges
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);

    //Enable interrupts for GPIO port E
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_2);
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_3);

    IntEnable(INT_GPIOE);     
  
      //---------------------Speaker---------------------------------//

    /*All of the code used in this function was taken from the peripheraldemo.c 
    file found at the following link: http://students.washington.edu/allane/ee472/ 
    */

    unsigned long ulPeriod;    

    //Set PWM Divide Ratio to 1
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    //Set Device: PWM0 Enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); 

    //Set GPIO Port: G Enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); 

    //Tell Port G, Pin 1, to take input from PWM 0
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);  

    //Set a SPEAKER_FREQUENCY Hz  as u1Period.
    ulPeriod = SysCtlClockGet() / SPEAKER_FREQUENCY;  

    //Configure PWM0 in up-down count mode, no sync to clock
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    //Set u1Period (SPEAKER_FREQUENCY) as the period of PWM0
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod);

    //Set PWM0, output 1 to a duty cycle of DUTY_CYCLE
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, ulPeriod / DUTY_CYCLE);

    //Activate PWM0
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);  
   
   
  //---------------------UART for Serial---------------------------------//
    
  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  // Set GPIO A0 and A1 as UART pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));

  // Enable the UART interrupt.
  IntEnable(INT_UART0);
  
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	 
  
  
//--------------------------For frequency measuring pin-------------///
  
  // Set GPIO as GPIO input pins
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
   //Set as input
   GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);
   
   //Enable the pull up so the pin isn't floating
   GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
   
   //Set as Falling edge interrupt
   GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
   
   //GPIODirModeSet( GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_DIR_MODE_IN );
   
   
   IntEnable(INT_GPIOB); 
   
   GPIOPinIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0); 
   
     //-------------------for - ADC--------------------------------//

  // Enable the clock to the ADC module
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);
  // set Port B Pin 1 as analog input
  GPIOPinTypeADC(ADC_BASE, GPIO_PIN_1);
   //Enable the pull up so the pin isn't floating
   GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
   

  // Configure the ADC to sample at 500KSps
  //SysCtlADCSpeedSet(SYSCTL_ADCSPEED_500KSPS );

  // Disable sample sequences 0
  ADCSequenceDisable(ADC_BASE, 3);

  // Configure sample sequence 1: timer trigger, priority = 0
  ADCSequenceConfigure(ADC_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

  // Configure sample sequence 1 steps 0, 1 and 2
  //ADCSequenceStepConfigure(ADC_BASE, 1, 0, ADC_CTL_CH0);
  ADCSequenceStepConfigure(ADC_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

  // Enable sample sequence 0.
  ADCSequenceEnable(ADC_BASE, 3);

  // Enable the interrupt for sample sequence 0
  //ADCIntEnable(ADC0_BASE, 0);
  //IntEnable(INT_ADC0);
  
  // Clear the interrupt status flag.
  ADCIntClear(ADC_BASE, 3);


   
   
   

    ADCProcessorTrigger(ADC_BASE, 3);
    while(!ADCIntStatus(ADC_BASE, 3, false))
     {
     }

    ADCIntClear(ADC_BASE, 3);
   
   //Wait until the sample sequence has completed.
   
     voltage = ADCSequenceDataGet(ADC_BASE, 3, &ulADCval);
        
       ADCProcessorTrigger(ADC_BASE, 3);
    while(!ADCIntStatus(ADC_BASE, 3, false))
     {
     }

    ADCIntClear(ADC_BASE, 3);
   
   //Wait until the sample sequence has completed.
   
     voltage = ADCSequenceDataGet(ADC_BASE, 3, &ulADCval);
}

    
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    static xOLEDMessage xMessage = { "PASS" };
    static unsigned portLONG ulTicksSinceLastDisplay = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* 
      Called from every tick interrupt.  Have enough ticks passed to make it
      time to perform our health status check again? 
    */
    
    ulTicksSinceLastDisplay++;
    if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
    {
       ulTicksSinceLastDisplay = 0;
            
    }
}

void buildString(char* arrayPointer, char newString[]){
  int j = 0;
  
  while (newString[j] != '\0'){
     *arrayPointer = newString[j];
     j++;
     arrayPointer++;
  } 
  *arrayPointer = '\0';
}


extern void IntGPIOe(void){
  //Clear the interrupt to avoid continuously looping here
  GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_0);
  GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
  GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_2);
  GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);
  
  //Set the Event State for GPIO pin 0
  State0=GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);       // pin 0 up button
  State1=GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1);       // pin 1 down button
  State2=GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2);       // pin 2 left buttonv
  State3=GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3);       // pin 3 right button  
  
  // State1,2,3,4 are things to be declared in main
  
  
  //Switches are normally-high, so flip the polarity of the results:
  State0=State0^GPIO_PIN_0;  
  State1=State1^GPIO_PIN_1;
  State2=State2^GPIO_PIN_2;
  State3=State3^GPIO_PIN_3;
  
  xSemaphoreGiveFromISR(trainComSemaphore, NULL);
}

void schedule(void* data)
{
  scheduleData* mysd = (scheduleData*) data;
  
  xOLEDMessage xMessage;
  
  //initialially the list is empty
  char* a = mysd->counterStringP;
  
  //Variables used for the counter
  int localTime; 
  int digit;
  
  //execution part
  while(1)
  {
    if(suspend){
      
      *mysd->globalCountP = *mysd->globalCountP + increment;
        digit  = 0;
        localTime = (*mysd -> globalCountP);
        
        while(localTime != 0 ) {
          a[5 - digit] = (localTime % 10) + ASCII_ZERO;
          digit =  digit + 1;
          localTime = localTime / 10;
        }
        
        //Print count to near the bottom of the screen
        RIT128x96x4StringDraw(a, x_coor, y_coorGC, 15);
        
        if (*mysd->numTrainsPresentP > 0) {
          //If gridlock is TRUE or if trainPresent is TRUE, then everytime the timer interrupt occurs (every half second)
          //we want to add switch control back to the list to manage the gridlock
          xSemaphoreGive(switchControlSemaphore);
        }
        
        
        if ( (*mysd->previousGlobalCountP + 2 == *mysd->globalCountP) || (*mysd->globalCountP < 2) ){
          trainComFlag = 1;
        }
        
        xSemaphoreGive(serialComSemaphore);
        //*mysd->timerState = 0;
      
        frequencyCounter(); 
			

       
    }
  
	// if OLED is disabled clear the screen
	if(OLED_Disable){
		RIT128x96x4Clear();
	}

  vTaskDelay(500 / portTICK_RATE_MS); //500mS 
  }
}


//*****************************************************************************
// Send a string to the UART.
//
// All code in this function was taken from the uart_echo.c IAR example, 
// except CharPutNonBlocking was changed to CharPut
//*****************************************************************************
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPut(UART0_BASE, *pucBuffer++);
    }
}

void UARTIntHandler(void)
{
  unsigned long ulStatus;
      
  // Get the interrrupt status.
  ulStatus = UARTIntStatus(UART0_BASE, true);

  // Clear the asserted interrupts.
  UARTIntClear(UART0_BASE, ulStatus);

  // Loop while there are characters in the receive FIFO.
  while(UARTCharsAvail(UART0_BASE))
  {
    // Read the next character from the UART and write it back to the UART.
    //Nonblocking????
    UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
  }
} 

//freqeucny interrupt GPIO
void FrequencyIntGPIOa(void){
  GPIOPinIntClear(GPIO_PORTB_BASE, GPIO_PIN_0);
  frequencyCount++;
}
