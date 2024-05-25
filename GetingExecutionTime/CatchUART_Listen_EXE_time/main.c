/*
 * CatchTemperature_EXE_time.c
 *
 * Created: 5/15/2024 2:43:30 PM
 * Author : Hesham Shehata
 */ 
#include "MCAL/USART/USART_interface.h"
#include "MCAL/ADC/ADC_interface.h"
#include "MCAL/TIMER0/TIMER0_interface.h"
#include "MCAL/TIMER0/TIMER0_private.h"

#include "HAL/LCD/LCD_interface.h"
#include "HAL/NTC_Temperature/NTC_interface.h"
#include "HAL/LED/LED_interface.h"

#include <util/delay.h>


unsigned long int  timeTaken = 0 ;

void TimeForFunction(void);
void convertInt2Str(unsigned long int Int2str);


int main(void)
{
    LCD_init();
	USART_Init();
    ADC_Init();
    LED_Init(PORTC_ID,PIN0_ID);
    LED_OnOffPositiveLogic(PORTC_ID,PIN0_ID,0);
    Timer0_Init();
    Timer0_SetCallBack(TimeForFunction);

    Timer0_ProvideClock();


    /*	catch uart data */
    uint8 u8Ind = 0;
	uint8 txMsg_UART[4] ={};
	uint8 u8Data = 0;
	boolean update_thresould_temp = FALSE ;
	
    /*	Check if received any new data */
    while(!UART_receiveByteUnblocking(&u8Data)){
        
        
        
        if(u8Data == 'G') /*	Will update flag of Threshold Temperature */
        {
            LED_OnOffPositiveLogic(PORTC_ID,PIN0_ID,1);
            break;
        }
   
    }
	


    Timer0_StopClock();
	
	_delay_ms(2000);
    _delay_ms(2000);
    _delay_ms(2000);
    
    LCD_ClearScreen();
       
    convertInt2Str(timeTaken + TCNT0);
    while (1) 
    {
      
    }
}


void TimeForFunction(void)
{
    timeTaken += 256 ;     
}

void convertInt2Str(unsigned long int Int2str)
{
    unsigned long int Num  = Int2str;
    unsigned long int reversed_Num = 0 ;
    unsigned char I2S[6] = {};
    unsigned char Count_Num = 0;
    unsigned char ten_multiple = 0 ;
    unsigned char firstZeros = 0 ;
    while(Num > 0)
    {
        reversed_Num *= 10 ;
        if(firstZeros == 0)
        {
            if( (Num %10 ) == 0)
            {
                ten_multiple++;
            }
            else
            {
                firstZeros = 1 ;
            }
        }
        reversed_Num += Num %10 ;
        Num /= 10;
    }
    while(reversed_Num > 0)
    {
        I2S[Count_Num] = (reversed_Num % 10) + '0';
        reversed_Num /= 10;
        Count_Num++;
    }
    while(ten_multiple)
    {
        I2S[Count_Num] = '0';
        Count_Num++;
        ten_multiple--;
    }
    I2S[Count_Num] = '\0';
    LCD_DisplayString( (unsigned char *)I2S);
    LCD_DisplayString( (unsigned char *)" Tick");
}
