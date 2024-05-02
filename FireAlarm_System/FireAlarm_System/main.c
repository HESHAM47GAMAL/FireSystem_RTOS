/*
 * FireAlarm_System.cpp
 *
 * Created: 4/25/2024 6:22:08 PM
 * Author : moham
 */ 

#include "MCAL/USART/USART_interface.h"
#include "MCAL/ADC/ADC_interface.h"

#include "HAL/LCD/LCD_interface.h"
#include "HAL/NTC_Temperature/NTC_interface.h"
#include "HAL/LED/LED_interface.h"

#define ExceedTemperature    (1 << 0)
#define SystemFireState		 (1 << 1)

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
// responsible for Initialize GPIO , ADC , UART , LCD and NTC temperature
void System_Init(void);

void LCD_MAINInit(void);

// 
// void T_T1(void* pvParam);
// void T_T2(void* pvParam);
void T_T3_Catch_Update_Temp(void * pvparam);
void T_T4_UART_listen(void * pvparam);
void T_T5_Update_LCD(void *pvparam);
void T_T6_SwitchBetHaz_Nor(void *pvparam);

void F_fireStateScreen(void);



unsigned char Current_Temp  ;
unsigned char thresold_Temp = 80;
boolean AlarmState = TRUE; 


SemaphoreHandle_t xMutexLCD;
// SemaphoreHandle_t XFiresystemState_Semph ;

QueueHandle_t	MessQUart_Thres_temp = NULL;
QueueHandle_t 	MessMailuart_fireState = NULL;

EventGroupHandle_t 	egEvents = NULL;
EventBits_t			ebValues = 0;

int main(void)
{

	System_Init();
	LCD_MAINInit();
	
	xMutexLCD = xSemaphoreCreateMutex();
	// XFiresystemState_Semph = xSemaphoreCreateBinary();
	MessQUart_Thres_temp = xQueueCreate(3,3 * sizeof(uint8) );	
	MessMailuart_fireState = xQueueCreate(3, sizeof(uint8));

	egEvents = xEventGroupCreate();



	xTaskCreate(T_T6_SwitchBetHaz_Nor, "Emergency_normal", 200, NULL, 4, NULL);
	xTaskCreate(T_T3_Catch_Update_Temp, "Update Current Temp", 100, &Current_Temp, 3, NULL);
	xTaskCreate(T_T5_Update_LCD, "update Threshold_state", 100, NULL, 2, NULL);
	xTaskCreate(T_T4_UART_listen, "Recieve UART Mess", 100, NULL, 1, NULL);

	xEventGroupSetBits(egEvents,SystemFireState); // As I by defualt make system Fire Enable

	vTaskStartScheduler();

	return 0 ;
}



void T_T3_Catch_Update_Temp(void * pvparam)
{
	
	while(1)
	{
		/*	Read Current temperature sensor value*/
		Current_Temp = NTC_GetTemperature();
		
		/*	Check if current temperature >= Threshold temperature*/
		if(Current_Temp >= thresold_Temp )
		{
			/*	set event of exceeding temperature	*/
			xEventGroupSetBits(egEvents,ExceedTemperature);
		}
		else
		{
			/*	Clear event of exceeding temperature	*/
			xEventGroupClearBits(egEvents,ExceedTemperature);

		}

		/*	take mutex to access LCD shared resource*/
		if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 100 ) == pdTRUE )
		{
			/*	Dispalay value of current Temperature in LCD */
			LCD_MoveCursor(0,10);
			LCD_intToString(Current_Temp);
			if(Current_Temp < 100)
			{
				LCD_DisplayString((uint8 *)" ");
			}
			// USART_SendStringPolling((uint8 *)"ADC update lcd\r");
			xSemaphoreGive( xMutexLCD );
		}
		/*	Failed to take mutex*/
		else
		{
			/*	tell that failled to update LCD	*/
			USART_SendStringPolling((uint8 *)"ADC Failed update lcd\r");
		}

		vTaskDelay(200);/*	As I want to take new Temperature Value every half second	*/
	}
}


void T_T4_UART_listen(void * pvparam)
{
	
	uint8 u8Ind = 0;
	uint8 txMsg_UART[4] ={};
	uint8 u8Data = 0;
	boolean update_thresould_temp = FALSE ;
	while(1){
		/*	Check if received any new data */
		if(UART_receiveByteUnblocking(&u8Data)){
			
			
			
			if(u8Data == 'G') /*	Will update flag of Threshold Temperature */
			{
				update_thresould_temp = TRUE ;
			}
			else if( (update_thresould_temp == TRUE) && ((u8Data >= '0') && (u8Data <= '9')) )
			{
				txMsg_UART[u8Ind] = u8Data;
				u8Ind++;
			}
			else if( (update_thresould_temp == TRUE) && (u8Data == 'K')) /*	Accept new threshold temperature value*/
			{
				txMsg_UART[u8Ind] = 0;
				xQueueSend(MessQUart_Thres_temp, txMsg_UART, portMAX_DELAY);
				update_thresould_temp = FALSE ;
				u8Ind = 0;	
			}
			else if( (update_thresould_temp == TRUE) && (u8Data == 'C')) /*	Cancel new threshold temperature value*/
			{
				update_thresould_temp = FALSE ;
				u8Ind = 0;
			}
			else if(u8Data == 'T') /*	Will toggle state of Alarm system*/
			{
				xQueueSend(MessMailuart_fireState, &u8Data, portMAX_DELAY);
			}
			else if(u8Data == 'D') /*	Will Disbale Alarm system*/
			{
				xQueueSend(MessMailuart_fireState, &u8Data, portMAX_DELAY);
			}
		}
	}
}


void T_T5_Update_LCD(void *pvparam)
{
	uint8 rxMsg_thresholdTemp[4] = {};
	uint8 rxmsg_fireState ;
	while(1)
	{
		/*	handle received thresould temperature value and display it */
		if(xQueueReceive(MessQUart_Thres_temp, rxMsg_thresholdTemp, 100))
		{
			thresold_Temp = rxMsg_thresholdTemp[0] - '0';
			thresold_Temp *= 10 ;
			thresold_Temp += rxMsg_thresholdTemp[1] - '0';

			/*	Update Event state if thresold_Temp <= Current_Temp */
			if(thresold_Temp <= Current_Temp)
			{
				/*	set event of exceeding temperature	*/
				xEventGroupSetBits(egEvents,ExceedTemperature);
				

			}
			/*	No need to update threshold value	*/
			else
			{
				/*	Clear event of exceeding temperature	*/
				xEventGroupClearBits(egEvents,ExceedTemperature);
			}

			if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 100 ) == pdTRUE )
			{
				LCD_DisplayStringRowCol(rxMsg_thresholdTemp,1,16);
				USART_SendStringPolling((uint8 *)"Thre update lcd\r");
				xSemaphoreGive( xMutexLCD );
			}
			else
			{
				USART_SendStringPolling((uint8 *)"thre Failed update lcd\r");
			}
		}
		else
		{

		}

		/*	handle state of firing system if it enabled or disabled and update lcd with this change	*/
		if(xQueueReceive(MessMailuart_fireState, &rxmsg_fireState, 100))
		{
			/*	Disable state of fire system */
			if(rxmsg_fireState == 'D')
			{
				AlarmState = FALSE ;
				xEventGroupClearBits(egEvents,SystemFireState);
				if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 400 ) == pdTRUE )
				{
					LCD_DisplayStringRowCol((uint8 *)"D",2,13);	
					xSemaphoreGive( xMutexLCD );
				}
				else 
				{

				}
			}
			/*	Toggle state of fire system */
			else if(rxmsg_fireState == 'T')
			{
				
				if(AlarmState == TRUE)
				{
					AlarmState = FALSE ;
					xEventGroupClearBits(egEvents,SystemFireState);
					if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 400 ) == pdTRUE )
					{
						LCD_DisplayStringRowCol((uint8 *)"D",2,13);	
						xSemaphoreGive( xMutexLCD );
					}
					else 
					{

					}
				}
				else if(AlarmState == FALSE)
				{
					AlarmState = TRUE ;
					xEventGroupSetBits(egEvents,SystemFireState);
					if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 400 ) == pdTRUE )
					{
						LCD_DisplayStringRowCol((uint8 *)"E",2,13);	
						xSemaphoreGive( xMutexLCD );
					}
					else 
					{

					}
				}

			}
		}
		

	}
}


void T_T6_SwitchBetHaz_Nor(void *pvparam)
{
	boolean Mutex_Taken_Already = FALSE ;
	while (1)
	{
		ebValues = xEventGroupWaitBits(egEvents,
										(SystemFireState | ExceedTemperature),
										0,
										1,//AND
										100);

		if( ( ebValues & (SystemFireState | ExceedTemperature) )== (SystemFireState | ExceedTemperature) )
		{
			if( xSemaphoreTake( xMutexLCD, ( TickType_t ) 1000 ) == pdTRUE )
			{
				Mutex_Taken_Already = TRUE ;
				LED_OnOffPositiveLogic(PORTC_ID,PIN0_ID,LED_ON);
				F_fireStateScreen();
			}
			
		}
		else
		{
			if(Mutex_Taken_Already == TRUE)
			{
				Mutex_Taken_Already = FALSE ;
				LED_OnOffPositiveLogic(PORTC_ID,PIN0_ID,LED_OFF);
				xSemaphoreGive( xMutexLCD );
				/*	Should be here to avoid continous update LCD without go from Fire state to normal state*/
				LCD_MAINInit();	
			}
		
		}
		vTaskDelay(100);
	}
	
}


void System_Init(void)
{
	LCD_init();
	USART_Init();
    ADC_Init();
    LED_Init(PORTC_ID,PIN0_ID);

}

void LCD_MAINInit(void)
{
	LCD_ClearScreen();
	LCD_DisplayStringRowCol((uint8 *)"Cur Temp:    C",0,0);	
	/*	Catch value for Temperature and Display it  */
	Current_Temp = NTC_GetTemperature();
	LCD_MoveCursor(0,10);
	LCD_intToString(Current_Temp);
	LCD_DisplayStringRowCol((uint8 *)"Threshold Temp:    C",1,0);	
	LCD_MoveCursor(1,16);
	LCD_intToString(thresold_Temp);
	LCD_DisplayStringRowCol((uint8 *)"Alarm State: ",2,0);	
	LCD_MoveCursor(2,13);
	if(AlarmState == TRUE)
	{
		LCD_DisplayCharacter('E');
	}
	else 
	{
		LCD_DisplayCharacter('D');
	}
	
}


void F_fireStateScreen(void)
{
	LCD_ClearScreen();
	LCD_DisplayStringRowCol((uint8 *)"!! ALARM !!", 1 , 4);
}


