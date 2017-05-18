#include "header.h"
#include "constantDefs.h"
#include "FreeRTOS.h"
#include "semphr.h"


extern xSemaphoreHandle trainComSemaphore;
extern xSemaphoreHandle serialComSemaphore;
extern xSemaphoreHandle switchControlSemaphore;
extern unsigned int payload;
extern unsigned int trainComFlag;
extern unsigned int OLED_Disable;

void trainCom(void* data)
{
  // re-cast the void pointer 
  trainComData* mytcd = (trainComData*) data;
  
  // declares two variables, one for direction and one for train size
  int  direction = 0;
  int TrainSize = 0;
  
  while (1)
  {
    if (xSemaphoreTake(trainComSemaphore,portMAX_DELAY)){
      
      if(trainComFlag) {
        *mytcd ->previousGlobalCountP = *mytcd ->globalCountP;
        
        if (*mytcd -> numTrainsPresentP < 2)
        {
          if (*mytcd -> numTrainsPresentP == 0) 
          { //Then there are no trains in line, create train 1

            // North button 
            if (*mytcd->State0Ptr == 1){
              buildString(mytcd->train1FromP, NORTH_MESSAGE);
              *mytcd->State0Ptr == 0;
              
            // South button
            } else if (*mytcd->State1Ptr == 2) { 
              buildString(mytcd->train1FromP, SOUTH_MESSAGE);
              *mytcd->State1Ptr == 0;  
             
            // West button 
            } else if (*mytcd->State2Ptr == 4) {    
              buildString(mytcd->train1FromP, WEST_MESSAGE);
              *mytcd->State2Ptr == 0;
            
            // East button  
            } else if (*mytcd->State3Ptr == 8) {      
              buildString(mytcd->train1FromP, EAST_MESSAGE);
              *mytcd->State3Ptr == 0;
            }    
            
            // Generate a random number for train size
            TrainSize = randomInteger(2,9);  
            *mytcd -> train1SizeP = TrainSize;
            *mytcd -> train1TraversalTimeP = TrainSize * 12;
            
            // Build the train size string
            // Add the train size to the end of the train size string
            mytcd->train1SizeMessageP[0] = TrainSize+ASCII_ZERO;
            
            // generate a random number for direction, WEST == 0, NORTH == 1, EAST ==2, SOUTH ==3
            direction = randomInteger(0,3);  	
            
            if (direction == WEST){
              *mytcd -> train1DirectionP = WEST; 
              buildString(mytcd->train1DirectionMessageP, WEST_MESSAGE);
            }    
            if (direction == NORTH){
              *mytcd -> train1DirectionP = NORTH;
              buildString(mytcd->train1DirectionMessageP, NORTH_MESSAGE);
            }
            if (direction == EAST){
              *mytcd -> train1DirectionP = EAST;
              buildString(mytcd->train1DirectionMessageP, EAST_MESSAGE);
            } 
            if (direction == SOUTH){
              *mytcd -> train1DirectionP = SOUTH;
              buildString(mytcd->train1DirectionMessageP, SOUTH_MESSAGE);      
            } 
          
            //Build the payload message for train 1
            mytcd->train1LoadMessageP[0] = ((payload/ 100) % 10)+ASCII_ZERO;
            mytcd->train1LoadMessageP[1] = ((payload/ 10) % 10)+ASCII_ZERO;
            mytcd->train1LoadMessageP[2] = ((payload/ 1) % 10)+ASCII_ZERO;
            
          } else if (*mytcd -> numTrainsPresentP == 1){
            //Then there is only 1 train waiting, create train 2
            // North button 
            if (*mytcd->State0Ptr == 1){
              buildString(mytcd->train2FromP, NORTH_MESSAGE);
              *mytcd->State0Ptr == 0;
              
            // South button
            } else if (*mytcd->State1Ptr == 2) { 
              buildString(mytcd->train2FromP, SOUTH_MESSAGE);
              *mytcd->State1Ptr == 0;  
             
            // West button 
            } else if (*mytcd->State2Ptr == 4) {    
              buildString(mytcd->train2FromP, WEST_MESSAGE);
              *mytcd->State2Ptr == 0;
            
            // East button  
            } else if (*mytcd->State3Ptr == 8) {      
              buildString(mytcd->train2FromP, EAST_MESSAGE);
              *mytcd->State3Ptr == 0;
            }    
            
            // Generate a random number for train size
            TrainSize = randomInteger(2,9);  
            *mytcd -> train2SizeP = TrainSize;
            *mytcd -> train2TraversalTimeP = TrainSize * 12;      
            
            // Build the train size string
            // Add the train size to the end of the train size string
            mytcd->train2SizeMessageP[0] = TrainSize+ASCII_ZERO;
            
            // generate a random number for direction, WEST == 0, NORTH == 1, EAST ==2, SOUTH ==3
            direction = randomInteger(0,3);  	
            
            if (direction == WEST){
              *mytcd -> train2DirectionP = WEST;  
              buildString(mytcd->train2DirectionMessageP, WEST_MESSAGE);
            }    
            if (direction == NORTH){
              *mytcd -> train2DirectionP = NORTH;
              buildString(mytcd->train2DirectionMessageP, NORTH_MESSAGE);
            }
            if (direction == EAST){
              *mytcd -> train2DirectionP = EAST;
              buildString(mytcd->train2DirectionMessageP, EAST_MESSAGE);
            } 
            if (direction == SOUTH){
              *mytcd -> train2DirectionP = SOUTH;
              buildString(mytcd->train2DirectionMessageP, SOUTH_MESSAGE);      
            }    
            
            //Build the payload message for train 2
            mytcd->train2LoadMessageP[0] = ((payload/ 100) % 10)+ASCII_ZERO;
            mytcd->train2LoadMessageP[1] = ((payload/ 10) % 10)+ASCII_ZERO;
            mytcd->train2LoadMessageP[2] = ((payload/ 1) % 10)+ASCII_ZERO;    
          }  
            
          //  buildString(mytcd->payloadMessageP, PASSENGER_NUMBER_MESSAGE);
          //  mytcd->payloadMessageP[12] = ((payload/ 100) % 10)+ASCII_ZERO;
         //   mytcd->payloadMessageP[13] = ((payload/ 10) % 10)+ASCII_ZERO;
         //   mytcd->payloadMessageP[14] = ((payload/ 1) % 10)+ASCII_ZERO;
            
            RIT128x96x4StringDraw(mytcd->train1DirectionMessageP, TRAIN_1_X, TRAIN_TO_Y, brightness);
            RIT128x96x4StringDraw(mytcd->train1SizeMessageP, TRAIN_1_X, TRAIN_SIZE_Y, brightness);    
            RIT128x96x4StringDraw(mytcd->train1FromP, TRAIN_1_X, TRAIN_FROM_Y, brightness);
            RIT128x96x4StringDraw(mytcd->train1LoadMessageP, TRAIN_1_X, TRAIN_LOAD_Y, brightness);
            
            RIT128x96x4StringDraw(mytcd->train2DirectionMessageP, TRAIN_2_X, TRAIN_TO_Y, brightness);
            RIT128x96x4StringDraw(mytcd->train2SizeMessageP, TRAIN_2_X, TRAIN_SIZE_Y, brightness);    
            RIT128x96x4StringDraw(mytcd->train2FromP, TRAIN_2_X, TRAIN_FROM_Y, brightness);
            RIT128x96x4StringDraw(mytcd->train2LoadMessageP, TRAIN_2_X, TRAIN_LOAD_Y, brightness);

          //  RIT128x96x4StringDraw(mytcd->payloadMessageP, x_coor, trainFrom_y_coor+10, brightness);

           
            *mytcd -> numTrainsPresentP = *mytcd -> numTrainsPresentP + 1;
            // Add serialCom back to list so that the new train information is printed
            xSemaphoreGive(serialComSemaphore);
            xSemaphoreGive(switchControlSemaphore);
            //vTaskDelay(1000 / portTICK_RATE_MS); //500mS
            trainComFlag = 0;
        }
      }
    
      // if OLED is disabled clear the screen
	  if(OLED_Disable){
		RIT128x96x4Clear();
	  }
	}
  }
}