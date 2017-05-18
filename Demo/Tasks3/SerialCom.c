#include "header.h"
#include "constantDefs.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle serialComSemaphore;

void serialCom(void* data)
{
  // re-cast the pointer
  serialComData* mySerialData = (serialComData*) data;

  while(1){
    if(xSemaphoreTake(serialComSemaphore, portMAX_DELAY)){
       // Message to be sent
       UARTSend((unsigned char *)HYPERTERM_CLEAR_SCREEN_CODE,7);
       
       UARTSend((unsigned char *)"      ", 6);
       UARTSend((unsigned char *)TRAIN_TITLE_MESSAGE, 14);
       UARTSend((unsigned char *)NEW_LINE,4);   
       UARTSend((unsigned char *)NEW_LINE,4);
       
       UARTSend((unsigned char *)SIZE_MESSAGE, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train1SizeMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train2SizeMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)NEW_LINE,4);
       
       UARTSend((unsigned char *)FROM_MESSAGE, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train1FromP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train2FromP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)NEW_LINE,4);
       
       UARTSend((unsigned char *)TO_MESSAGE, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train1DirectionMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train2DirectionMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)NEW_LINE,4);
       
       UARTSend((unsigned char *)LOAD_MESSAGE, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train1LoadMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)mySerialData->train2LoadMessageP, MAX_STRING_SIZE);
       UARTSend((unsigned char *)NEW_LINE,4);
       UARTSend((unsigned char *)NEW_LINE,4);
       
       UARTSend((unsigned char *)mySerialData->counterStringP, COUNTER_STRING_SIZE);
    }
  }
}