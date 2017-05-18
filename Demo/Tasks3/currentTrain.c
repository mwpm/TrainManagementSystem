#include "header.h"
#include "constantDefs.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle currentTrainSemaphore;
extern unsigned int OLED_Disable;

void currentTrain(void* data){
  
  currentTrainData* myctd = (currentTrainData*) data;
  unsigned int remainingTraversalTime;
  
  while(1){
    if(xSemaphoreTake(currentTrainSemaphore,portMAX_DELAY)){
      
      remainingTraversalTime = *myctd->train1TraversalTimeP;
      
      if(remainingTraversalTime > 0)
      {
        if (*myctd -> train1DirectionP == EAST){
            int eastSoundArray[] = {1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0};

            if (eastSoundArray[remainingTraversalTime % 26] == 1){
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true); // turns on the sound when the value hit in the array is 1
            } else {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);  // turns off the sound when the value hit in the array is 0
            }
            
            // this algorithm gives us a flashing rate of 2 secs ON 2 secs OFF
            if(remainingTraversalTime % 8 < 4) 
            {
                // prints train direction and train size
               RIT128x96x4StringDraw(myctd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);      
               
            } else { 
                RIT128x96x4StringDraw(EMPTY_STRING, TRAIN_1_X, TRAIN_TO_Y, brightness);
            }    
        } 
        else if (*myctd -> train1DirectionP == WEST){
           int westSoundArray[] = {1,1,1,1,0,0,1,1,0,0,1,1,0,0};

            if (westSoundArray[remainingTraversalTime % 14] == 1){
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);  // turns on the sound when the value hit in the array is 1
            } else {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);  // turns off the sound when the value hit in the array is 0
            }
            
            // this algorithm gives us a flashing rate of 1 sec ON 1 sec OFF
            if(remainingTraversalTime % 4 < 2)  
            {
                // prints train direction and train size
               RIT128x96x4StringDraw(myctd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);     
            } else {
                RIT128x96x4StringDraw(EMPTY_STRING, TRAIN_1_X, TRAIN_TO_Y, brightness); 
            }        
        } 
        else if (*myctd -> train1DirectionP == NORTH){
            int northSoundArray[] = {1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0};
            // creates an integer array for train size display

            if (northSoundArray[remainingTraversalTime % 20] == 1)
            {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);  // turns on the sound when the value hit in the array is 1
            } else {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);  // turns off the sound when the value hit in the array is 0
            }
              
            // this algorithm gives us a flashing rate of 1.5 secs ON 1.5 secs OFF
            if(remainingTraversalTime % 6 < 3)  
            { 
                // prints train direction and train size
                RIT128x96x4StringDraw(myctd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);
            } else {   
                RIT128x96x4StringDraw(EMPTY_STRING, TRAIN_1_X, TRAIN_TO_Y, brightness);
             }  
        } 
        else if (*myctd -> train1DirectionP == SOUTH){
            int southSoundArray[] = {1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0};
            // creates an integer array for train size display

            if (southSoundArray[remainingTraversalTime % 24] == 1)
            {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);  // turns on the sound when the value hit in the array is 1
            } else {
                PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);  // turns off the sound when the value hit in the array is 0
            }
              
            // this algorithm gives us a flashing rate of 1 sec ON 1 sec OFF
            if(remainingTraversalTime % 4 < 2)  
            {   
                // prints train direction and train size
                RIT128x96x4StringDraw(myctd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);
            
            } else {
                RIT128x96x4StringDraw(EMPTY_STRING, TRAIN_1_X, TRAIN_TO_Y, brightness);  
             } 
        }
      } 
      
	  // if OLED is disabled clear the screen
	  if(OLED_Disable){
		RIT128x96x4Clear();
	  }
	
	}
  }
}