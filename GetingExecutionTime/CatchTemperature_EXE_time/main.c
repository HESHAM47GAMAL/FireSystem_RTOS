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

#include <util/delay.h>


unsigned long int  timeTaken = 0 ;

void TimeForFunction(void);
void convertInt2Str(unsigned long int Int2str);


int main(void)
{
    LCD_init();
	USART_Init();
    ADC_Init();
    Timer0_Init();
    Timer0_SetCallBack(TimeForFunction);

    Timer0_ProvideClock();


    /*	Read Current temperature sensor value*/
    unsigned char Current_Temp = NTC_GetTemperature();
    LCD_MoveCursor(0,10);
    convertInt2Str(Current_Temp);
    if(Current_Temp < 100)
    {
        LCD_DisplayString((uint8 *)" ");
    }
    USART_SendStringPolling((uint8 *)"ADC Failed update lcd\r");


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
