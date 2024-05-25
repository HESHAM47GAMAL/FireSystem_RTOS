 /******************************************************************************
 *
 * Module: NTC Temperature Sensor
 *
 * File Name: NTC_config.h
 *
 * Description: Header to configure NTC
 *
 * Author: Hesham Shehata
 *
 *******************************************************************************/
//Gard file

#ifndef NTC_CONFIG_H_
#define NTC_CONFIG_H_

/**************************                   INCLUDES                   **************************/

#include "../../MCAL/ADC/ADC_interface.h"
#include "../../SERVICE/STD_TYPES.h"


/*  Choose pin used to make analog read     */
#define NTC_PIN         ADC_Channel_1

#define R2              10000   //R2 Recommended to be 10K ohm




/*      Connection of hardware          */

//  ------------------ 5v (voltage reference)
//  |
//  |
//  |                                           -------------
// NTC  (R1)                                    |           |
//  |                                           |           |
//  ------------------- (ADC channel) (V_out) ->|   MCU     |   
//  |                                           |           |
// (R2)                                         |           |
//  |                                           -------------
//  |
//  -------- Ground



#endif