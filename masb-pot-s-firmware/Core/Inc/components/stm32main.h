/*
 * stm32main.h
 *
 *  Created on: Apr 15, 2021
 *      Author: macbook
 */

#ifndef INC_COMPONENTS_STM32MAIN_H_
#define INC_COMPONENTS_STM32MAIN_H_

#include "stm32f4xx_hal.h" // Include the microcontroller header files.
#include "cobs.h" // Include to access the cobs.c defined functions.
#include "components/masb_comm_s.h"

struct Handles_S {
	UART_HandleTypeDef *huart; //Here we add the handleTypeDef that we want to add
	TIM_HandleTypeDef *htim;
	I2C_HandleTypeDef *hi2c;
	ADC_HandleTypeDef *hadc;

};

// Available functions when including this header file.
void setup(struct Handles_S *handles); //Parameters accepted by the void setup are modified
//Pointer defined with the parameters needed
void loop(void);


#endif /* INC_COMPONENTS_STM32MAIN_H_ */
