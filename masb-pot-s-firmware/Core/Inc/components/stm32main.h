/*
 * stm32main.h
 *
 *  Created on: May 6, 2021
 *      Author: riza_
 */

#ifndef INC_COMPONENTS_STM32MAIN_H_
#define INC_COMPONENTS_STM32MAIN_H_

#include "stm32f4xx_hal.h"
#include "masb_comm_s.h"
#include "mcp4725_driver.h"

struct Handles_S {
	UART_HandleTypeDef *huart;
	I2C_HandleTypeDef * hi2c;
	ADC_HandleTypeDef * hadc1;
	TIM_HandleTypeDef * htim3;
// Aqui iriamos anadiendo los diferentes XXX_HandleTypeDef que necesitaramos anadir.
};


volatile struct Data_S dades;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

// Prototypes.
void setup(struct Handles_S *handles);
void loop(void);

#endif /* INC_COMPONENTS_STM32MAIN_H_ */
