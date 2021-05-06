/*
 * stm32main.h
 *
 *  Created on: May 6, 2021
 *      Author: riza_
 */

#ifndef INC_COMPONENTS_STM32MAIN_H_
#define INC_COMPONENTS_STM32MAIN_H_

#include "stm32f4xx_hal.h"

struct Handles_S {
    UART_HandleTypeDef *huart;
     // Aqui iriamos anadiendo los diferentes XXX_HandleTypeDef que necesitaramos anadir.
};
// Prototypes.
void setup(struct Handles_S *handles);
void loop(void);

#endif /* INC_COMPONENTS_STM32MAIN_H_ */