#include "header.h"
#include "constantDefs.h"
#include "FreeRTOS.h"
#include "semphr.h"


extern xSemaphoreHandle currentTrainSemaphore;
extern xSemaphoreHandle serialComSemaphore;
extern xSemaphoreHandle switchControlSemaphore;
extern unsigned int OLED_Disable;

void switchControl(void* data)
{
  // re-cast pointer type
  switchControlData* myscd = (switchControlData*) data;
  int waitTime = 0;
  // sets global variable gridLockChecked to true
  
  int gridLockNum;
  while(1) {
    if(xSemaphoreTake(switchControlSemaphore,portMAX_DELAY)){      
      
      
      
      if (*myscd->train1TraversalTimeP > 0) 
      {        
        // decrements the remaining traversal time by 1 count
        *myscd->train1TraversalTimeP = *myscd ->train1TraversalTimeP - 1;
      
        
        if(*myscd->numTrainsPresentP == 2 ) {
          myscd->waitTimeMessageP[0] = ((waitTime/ 100) % 10)+ASCII_ZERO;
          myscd->waitTimeMessageP[1] = ((waitTime/ 10) % 10)+ASCII_ZERO;
          myscd->waitTimeMessageP[2] = ((waitTime/ 1) % 10)+ASCII_ZERO;
          RIT128x96x4StringDraw(myscd->waitTimeMessageP, TRAIN_1_X, WAIT_Y, brightness);
          
          if(waitTime > 71) {
            myscd->waitTimeMessageP[3] ='L';
            myscd->waitTimeMessageP[4] ='W';
            RIT128x96x4StringDraw(myscd->waitTimeMessageP, TRAIN_1_X, WAIT_Y, brightness);
        } 
         waitTime++;
        } 
        
        xSemaphoreGive(currentTrainSemaphore);
    
      } else {    
          // resets global variables
        
          *myscd->numTrainsPresentP = *myscd->numTrainsPresentP - 1;
          waitTime = 0;
          
          if (*myscd->numTrainsPresentP == 1){
            buildString(myscd->waitTimeMessageP,EMPTY_STRING);
            RIT128x96x4StringDraw(myscd->waitTimeMessageP, TRAIN_1_X, WAIT_Y, brightness);
  
            //Set train 1 size to train2's size
            *myscd->train1SizeP = *myscd->train2SizeP;
            //Set train 1 to direction to train2's to direction,
            *myscd->train1DirectionP = *myscd->train2DirectionP;
            //Set train1 traversal time to the correct train2 traversal time
            *myscd->train1TraversalTimeP = *myscd->train2TraversalTimeP;
            
            // and rebuild the "to direction" string
            if (*myscd->train1DirectionP == WEST){
              buildString(myscd->train1DirectionMessageP, WEST_MESSAGE);
            }    
            if (*myscd->train1DirectionP == NORTH){
              buildString(myscd->train1DirectionMessageP, NORTH_MESSAGE);
            }
            if (*myscd->train1DirectionP == EAST){
              buildString(myscd->train1DirectionMessageP, EAST_MESSAGE);
            } 
            if (*myscd->train1DirectionP == SOUTH){
              buildString(myscd->train1DirectionMessageP, SOUTH_MESSAGE);      
            } 
            
            //Reset the train1 from direction string to train2's from direction
            buildString(myscd->train1FromP, myscd->train2FromP);
            
            myscd->train1SizeMessageP[0] = (*myscd->train1SizeP) + ASCII_ZERO;
            
            //Set train1 payload to train2's payload
            buildString(myscd->train1LoadMessageP, myscd->train2LoadMessageP);           
            
            //Reset all of train2's fields 
            *myscd->train2SizeP = 0;
            *myscd->train2TraversalTimeP = 0;
            *myscd->train2DirectionP = -1;
            buildString(myscd->train2FromP, EMPTY_STRING);
            buildString(myscd->train2DirectionMessageP, EMPTY_STRING);
            buildString(myscd->train2SizeMessageP, EMPTY_STRING);
            buildString(myscd->train2LoadMessageP, EMPTY_STRING);
            
            //Reprint all new strings to the screen
            RIT128x96x4StringDraw(myscd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);
            RIT128x96x4StringDraw(myscd->train1SizeMessageP, TRAIN_1_X, TRAIN_SIZE_Y, brightness);    
            RIT128x96x4StringDraw(myscd->train1FromP, TRAIN_1_X, TRAIN_FROM_Y, brightness);
            RIT128x96x4StringDraw(myscd->train1LoadMessageP, TRAIN_1_X, TRAIN_LOAD_Y, brightness);
            
            RIT128x96x4StringDraw(myscd->train2DirectionMessageP, TRAIN_2_X, TRAIN_TO_Y, brightness);
            RIT128x96x4StringDraw(myscd->train2SizeMessageP, TRAIN_2_X, TRAIN_SIZE_Y, brightness);    
            RIT128x96x4StringDraw(myscd->train2FromP, TRAIN_2_X, TRAIN_FROM_Y, brightness);
            RIT128x96x4StringDraw(myscd->train2LoadMessageP, TRAIN_2_X, TRAIN_LOAD_Y, brightness);
            
          } else {
            //Then there is no second train waiting
            //Reset all of train2's fields 
            *myscd->train1SizeP = 0;
            *myscd->train1DirectionP = -1;
            buildString(myscd->train1FromP, EMPTY_STRING);
            buildString(myscd->train1DirectionMessageP, EMPTY_STRING);
            buildString(myscd->train1SizeMessageP, EMPTY_STRING);  
            buildString(myscd->train1LoadMessageP, EMPTY_STRING);
            
            //Reprint all new strings to the screen
            RIT128x96x4StringDraw(myscd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);
            RIT128x96x4StringDraw(myscd->train1SizeMessageP, TRAIN_1_X, TRAIN_SIZE_Y, brightness);    
            RIT128x96x4StringDraw(myscd->train1FromP, TRAIN_1_X, TRAIN_FROM_Y, brightness); 
            RIT128x96x4StringDraw(myscd->train1LoadMessageP, TRAIN_1_X, TRAIN_LOAD_Y, brightness);           
          }
          
          // turn off the sound
          PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);
          
          // Add serialCom back to list so that the new information is printed
          xSemaphoreGive(serialComSemaphore);
        }
      

	  	// if OLED is disabled clear the screen
		if(OLED_Disable){
			RIT128x96x4Clear();
		}
	  }
    }
  }
 
