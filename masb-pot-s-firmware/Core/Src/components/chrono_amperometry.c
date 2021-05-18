/**
  ******************************************************************************
  * @file		chronoamperometry.c
  * @brief		Gestión de la cronoamperometría.
  * @author		Albert Álvarez Carulla
  * @copyright	Copyright 2020 Albert Álvarez Carulla. All rights reserved.
  ******************************************************************************
  */

#include "components/chrono_amperometry.h"
#include "components/stm32main.h"
#include "components/masb_comm_s.h"


//uint8_t txBuffer[32] = { 0 }; // buffer para transmitir
//uint8_t txBufferSize = 0; // tamano a enviar


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	_Bool agafa_mesura = TRUE;
}
	/*
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
	//enviar los datos
	/*void MASB_COMM_S_sendData(struct Data_S data) {

		   	saveLongAsByteArrayIntoBuffer(txBufferDecoded, 0, data.point);
		   	saveLongAsByteArrayIntoBuffer(txBufferDecoded, 4, data.timeMs);
		   	saveDoubleAsByteArrayIntoBuffer(txBufferDecoded, 8, data.voltage);
		   	saveDoubleAsByteArrayIntoBuffer(txBufferDecoded, 16, data.current);

		   	uint32_t txBufferLenght = COBS_encode(txBufferDecoded, 24, txBuffer);

		   	txBuffer[txBufferLenght] = UART_TERM_CHAR;
		   	txBufferLenght++;

		   	HAL_UART_Transmit_IT(huart, txBuffer, txBufferLenght);

	 }*/

