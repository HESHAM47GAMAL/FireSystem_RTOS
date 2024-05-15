 /******************************************************************************
 *
 * Module: NTC Temperature Sensor
 *
 * File Name: NTC_program.c
 *
 * Description: contain Implementation of NTC Driver
 *
 * Author: Hesham Shehata
 *
 *******************************************************************************/


/**************************                   INCLUDES                   **************************/
#include "NTC_interface.h"
#include "NTC_config.h"
#include "../../MCAL/ADC/ADC_interface.h"
#include <math.h>


uint8 NTC_GetTemperature(void)
{
    uint16 V_out = ADC_ReadChannelSingleConvertion(NTC_PIN);
    /*  Apply voltage divider to get Current resistance for NTC     */
    float32 R1 = R2 * ( (ADC_MAX_VALUE / (float32)V_out) - 1.0 ) ;
    float32 LogR1 = log(R1) ;
    float32 T = (1.0 / ( C1 + (C2 *LogR1) + (C3 * LogR1 * LogR1 * LogR1) ) ) ;
    T -= 273.15 ;
    return T ;
}

    //                         1
    // T = -------------------------------------------
    //     C1 + (C2 * log(R1))  + (C3 * ( log(R1) )^3)