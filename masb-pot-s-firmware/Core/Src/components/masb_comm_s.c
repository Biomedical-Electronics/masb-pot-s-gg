/*
 * masb_comm_s.c
 *
 *  Created on: Apr 14, 2021
 *      Author: riza_
 */

#include "components/masb_comm_s.h"


 // variables
 static UART_HandleTypeDef *huart; // creamos variable de tipo puntero donde almacenaremos la dirección de memoria
 static _Bool dataReceived = FALSE;
 uint8_t rxIndex = 0;
 uint8_t rxBuffer[UART_BUFF_SIZE] = { 0 };
 uint8_t rxBufferDecoded[UART_BUFF_SIZE] = { 0 };
 uint8_t txBuffer[UART_BUFF_SIZE] = { 0 };
 uint8_t txBufferDecoded[UART_BUFF_SIZE] = { 0 };

 static double saveByteArrayAsDoubleFromBuffer(uint8_t *buffer, uint8_t index); // función para recibir los datos (guardar de byte a variables)
 static long saveByteArrayAsLongFromBuffer(uint8_t *buffer, uint8_t index); // de byte a long
 static void saveLongAsByteArrayIntoBuffer(uint8_t *buffer, uint8_t index, uint32_t longVal); // funciones para enviar los datos
 static void saveDoubleAsByteArrayIntoBuffer(uint8_t *buffer, uint8_t index, double doubleVal); // (conversiones de variables decimales a vectores de bytes)

 /*
  *
  */

 //funciones
void MASB_COMM_S_setUart(UART_HandleTypeDef *newHuart) { // obtener el puntero a la configuración de la UART
	huart = newHuart; // lo guardamos en la variable huart
}

void MASB_COMM_S_waitForMessage(void) { // esperemos un nuevo mensaje

	dataReceived = FALSE;
	rxIndex = 0; // índice de conteo de los bytes recibidos
 	HAL_UART_Receive_IT(huart, &rxBuffer[rxIndex], 1); // habilitamos las interrupciones de recepción de bytes
}

_Bool MASB_COMM_S_dataReceived(void) { // nos indica si se ha recibido un mensaje

 	if (dataReceived) { // si se ha recibido un mensaje, lo decodificamos

 		COBS_decode(rxBuffer, rxIndex, rxBufferDecoded);
 	}
 	return dataReceived;

}

 uint8_t MASB_COMM_S_command(void) { // nos devuelve el primer byte que recibimos

 	return rxBufferDecoded[0];

}

struct CV_Configuration_S MASB_COMM_S_getCvConfiguration(void) { // coger los bytes del mensaje y unirlos para obtener los diferentes parámetros de la voltametría cíclica

	struct CV_Configuration_S cvConfiguration;

	  	cvConfiguration.eBegin = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 1);
	  	cvConfiguration.eVertex1 = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 9);
	  	cvConfiguration.eVertex2 = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 17);
	  	cvConfiguration.cycles = rxBufferDecoded[25];
	  	cvConfiguration.scanRate = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 26);
	  	cvConfiguration.eStep = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 34);

	return cvConfiguration;
 }

static double saveByteArrayAsDoubleFromBuffer(uint8_t *buffer, uint8_t index) {

	union Double_Converter {
		double d;
		uint8_t b[8];
} doubleConverter;

for (uint8_t i = 0; i < 8; i++) {

	doubleConverter.b[i] = buffer[i + index];
}

return doubleConverter.d;
 }


void MASB_COMM_S_sendData(struct Data_S data) { // enviar los datos

	   	saveLongAsByteArrayIntoBuffer(txBufferDecoded, 0, data.point);
	   	saveLongAsByteArrayIntoBuffer(txBufferDecoded, 4, data.timeMs);
	   	saveDoubleAsByteArrayIntoBuffer(txBufferDecoded, 8, data.voltage);
	   	saveDoubleAsByteArrayIntoBuffer(txBufferDecoded, 16, data.current);

	   	uint32_t txBufferLenght = COBS_encode(txBufferDecoded, 24, txBuffer);

	   	txBuffer[txBufferLenght] = UART_TERM_CHAR;
	   	txBufferLenght++;

	   	HAL_UART_Transmit_IT(huart, txBuffer, txBufferLenght);

 }
	   static void saveLongAsByteArrayIntoBuffer(uint8_t *buffer, uint8_t index, uint32_t longVal) {
		   // convertir de variables long - entero largo - a vectores de bytes
	      union Long_Converter {
	         long l;
	         uint8_t b[4];
	      } longConverter;

	   	longConverter.l = longVal;

	   	for (uint8_t i = 0; i < 4; i++) {

	   		buffer[i + index] = longConverter.b[i];

	   	}

   }
	   static void saveDoubleAsByteArrayIntoBuffer(uint8_t *buffer, uint8_t index, double doubleVal) {
		   // convertir de variables double - decimal doble - a vectores de bytes
	     union Double_Converter {
	         double d;
	         uint8_t b[8];
	    } doubleConverter;

	   	doubleConverter.d = doubleVal;

	   	for (uint8_t i = 0; i < 8; i++) {

	   		buffer[i + index] = doubleConverter.b[i];

	   	}

 }

void HAL_UART_RxCpltCallback (UART_HandleTypeDef *huart) { // callback de la interrupción de recepción de un byte

	if (rxBuffer[rxIndex] == UART_TERM_CHAR) {
		dataReceived = TRUE;
	} else {
		rxIndex++;
		HAL_UART_Receive_IT(huart, &rxBuffer[rxIndex], 1);

	}
}

struct CA_Configuration_S MASB_COMM_S_getCaConfiguration(void) { // cronoamperometría

	struct CA_Configuration_S caConfiguration;

		caConfiguration.eDc = saveByteArrayAsDoubleFromBuffer(rxBufferDecoded, 1);
		caConfiguration.samplingPeriodMs = saveByteArrayAsLongFromBuffer(rxBufferDecoded, 9);
		caConfiguration.measurementTime = saveByteArrayAsLongFromBuffer(rxBufferDecoded, 13);

		return caConfiguration;
 }

static long saveByteArrayAsLongFromBuffer(uint8_t *buffer, uint8_t index) {

	    union Long_Converter {
	    	long l;
	    	uint8_t b[4];
	    } longConverter;

	  	for (uint8_t i = 0; i < 4; i++) {

	  		longConverter.b[i] = buffer[i + index];
	   	}

	  	return longConverter.l;
}

