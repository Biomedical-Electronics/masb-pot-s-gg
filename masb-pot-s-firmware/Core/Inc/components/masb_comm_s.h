/*
 * masb_comm_s.h
 *
 *  Created on: Apr 14, 2021
 *      Author: riza_
 */

#ifndef INC_COMPONENTS_MASB_COMM_S_H_
#define INC_COMPONENTS_MASB_COMM_S_H_

// Macros que usaremos definidas
 #define UART_BUFF_SIZE		 50
 #define UART_TERM_CHAR		 0x00
 #define TRUE					1
 #define FALSE					0
 #define CV						2
 #define CA						3
 #define IDLE					4

 // Commandos definidos en el documento (set de instrucciones)
 #define START_CV_MEAS	0x01
 #define START_CA_MEAS	0x02
 #define STOP_MEAS		0x03

// Incluir los archivos que utilizaremos
 #include "stm32f4xx_hal.h"
 #include "components/cobs.h"
 #include "components/cyclic_voltammetry.h"
 #include "components/chrono_amperometry.h"

// Structures
// Incluir estructura data_s, que contiene los datos que el microcontrolador enviará al PC
struct Data_S {

 	uint32_t point;
 	uint32_t timeMs;
 	double voltage;
 	double current;

};

 // Prototypes.
 void MASB_COMM_S_setUart(UART_HandleTypeDef *newHuart); // puntero a la estructura de configuración de la UART para tener acceso a ella desde aquí
 void MASB_COMM_S_waitForMessage(void); // activa la interrupción para la recepción de mensajes
 _Bool MASB_COMM_S_dataReceived(void); // indica si se ha recibido un mensaje
 uint8_t MASB_COMM_S_command(void); // devuelve el comando recibido

 struct CV_Configuration_S MASB_COMM_S_getCvConfiguration(void); // extrae del mensaje recibido los parámetros de configuración de una voltametría cíclica
 struct CA_Configuration_S MASB_COMM_S_getCaConfiguration(void); // extrae del mensaje recibido los parámetros de configuración de una amperometría
 void MASB_COMM_S_sendData(struct Data_S data); // envía los datos al PC

 #endif /* INC_COMPONENTS_MASB_COMM_S_H_ */
