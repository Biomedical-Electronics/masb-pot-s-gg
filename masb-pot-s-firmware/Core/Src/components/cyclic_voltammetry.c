/**
  ******************************************************************************
  * @file		cyclic_voltammetry.c
  * @brief		Gestión de la voltammetría cíclica.
  * @author		Albert Álvarez Carulla
  * @copyright	Copyright 2020 Albert Álvarez Carulla. All rights reserved.
  ******************************************************************************
  */

#include "components/cyclic_voltammetry.h"
#include "components/stm32main.h"
#include "components/masb_comm_s.h"


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	_Bool agafa_mesura1 = TRUE;
}
//encendre relé (1)
//prendre mesura
//apagar relé (0) - GPIO_Pin que configurem
