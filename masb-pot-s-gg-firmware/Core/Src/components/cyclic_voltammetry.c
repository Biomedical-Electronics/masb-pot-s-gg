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

uint32_t i=0;
double rTIA = 1e5;
const float sense = 3.3/4095; // factor de conversion de ADC a tension
volatile struct Data_S dades;
uint16_t adcValue = 0;
const float samplingPeriod = 0.5;
double cycles = 5;
double measurementTime = 60;


//uint8_t txBuffer[32] = { 0 }; // buffer para transmitir
//uint8_t txBufferSize = 0; // tamano a enviar


HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	HAL_ADC_Start_IT(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 200);
	uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

	double vADC = (adcValue/sense)*(1023);
	double vCell = (1.65 - vADC)*2;
	double iCell = vCell/rTIA;

	dades.point = 1;
	dades.timeMs = samplingPeriod * i;
	dades.voltage = vCell;
	dades.current = iCell;

	MASB_COMM_S_sendData(dades);
	HAL_ADC_Stop_IT(&hadc1);

	i++;

	if(i == measurementTime) {
		HAL_TIM_Base_Stop_IT(&htim3);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
}
