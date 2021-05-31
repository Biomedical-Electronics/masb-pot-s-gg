#include "components/stm32main.h"
#include "components/masb_comm_s.h"
#include "components/ad5280_driver.h"
#include "components/mcp4725_driver.h"
#include "components/i2c_lib.h"
#include <math.h>

struct CV_Configuration_S cvConfiguration;
struct CA_Configuration_S caConfiguration;
struct Data_S data;
int8_t	Estado; //variable para cambiar el estado de tipo de medida

double vCell = 0;
double cicles = 0;
float vdac = 0;
double rTIA = 1e5;
double vObjetivo = 0;
uint32_t i=0;
const float sense = 3.3/4095; // factor de conversion de ADC a tension
MCP4725_Handle_T hdac = NULL;
volatile _Bool agafa_mesura = FALSE;
volatile _Bool canvi_vObjetivo = FALSE;



// Funcion ejecutada antes del while loop (solo se ejecuta una vez)
// estructura que contenga todos punteros a las diferentes configuraciones de los diferentes periféricos
void setup(struct Handles_S *handles) {
	//GPIO_pinAlimentació - encendre alimentació (1)
	//GPIO_pinRelé - apagar relé (no mesura)
	 MASB_COMM_S_setUart(handles->huart);
	 MASB_COMM_S_waitForMessage(); // componente masb_comm_s espera el primer byte
	 I2C_Init(handles->hi2c);
	 //posar timer i adc
	 // Creamos el handle de la libreria.

	 hdac = MCP4725_Init();

	 // Configuramos la direccion I2C de esclavo, su tension de referencia (que es la
	 // de alimentacion) e indicamos que funcion queremos que se encargue de la
	 // escritura a traves del I2C. Utilizaremos la funcion I2C_Write de la libreria
	 // i2c_lib.
	 MCP4725_ConfigSlaveAddress(hdac, 0x66);
	 MCP4725_ConfigVoltageReference(hdac, 4.0f);
	 MCP4725_ConfigWriteFunction(hdac, I2C_Write);
}

// Funcion ejecutada en el while loop.
void loop(void) {
	if (MASB_COMM_S_dataReceived()) { // Si se ha recibido un mensaje...

	 		switch(MASB_COMM_S_command()) { // Miramos que comando hemos recibido
	 			case START_CA_MEAS:
	 				// Leemos la configuracion que se nos ha enviado en el mensaje y
	 				// la guardamos en la variable caConfiguration
	 				caConfiguration = MASB_COMM_S_getCaConfiguration();
	 				Estado=CA;
	 			break;

	 			case START_CV_MEAS: // Si hemos recibido START_CV_MEAS

	                 // Leemos la configuracion que se nos ha enviado en el mensaje y
	                 // la guardamos en la variable cvConfiguration
					cvConfiguration = MASB_COMM_S_getCvConfiguration();
					Estado=CV;

	 				/* Mensaje a enviar desde CoolTerm para hacer comprobacion
	 				 * eBegin = 0.25 V
	 				 * eVertex1 = 0.5 V
	 				 * eVertex2 = -0.5 V
	 				 * cycles = 2
	 				 * scanRate = 0.01 V/s
	 				 * eStep = 0.005 V
	 				 *
	 				 * Mensaje previo a la codificacion (lo que teneis que poder obtener en el microcontrolador):
	 				 * 01000000000000D03F000000000000E03F000000000000E0BF027B14AE47E17A843F7B14AE47E17A743F
	 				 *
	 				 * Mensaje codificado que enviamos desde CoolTerm (incluye ya el termchar):
	 				 * 0201010101010103D03F010101010103E03F010101010114E0BF027B14AE47E17A843F7B14AE47E17A743F00
	 				 */
	 				__NOP(); // Esta instruccion no hace nada y solo sirve para poder anadir un breakpoint

	 				// Aqui iria todo el codigo de gestion de la medicion que hareis en el proyecto
	                // si no quereis implementar el comando de stop.

	 				break;

				case STOP_MEAS: // Si hemos recibido STOP_MEAS

	 				/*
	 				 * Debemos de enviar esto desde CoolTerm:
	 				 * 020300
	 				 */
	 				__NOP(); // Esta instruccion no hace nada y solo sirve para poder anadir un breakpoint
	 				Estado=IDLE;
	 				// Aqui iria el codigo para tener la medicion si implementais el comando stop.

	 				break;

	 			default: // En el caso que se envia un comando que no exista

	                 // Este bloque de codigo solo lo tenemos para hacer el test de que podemos enviar los
	                 // datos correctamene. EN EL PROYECTO FINAL TENEMOS QUE ELIMINAR ESTE CODIGO y pondremos
	                 // lo que se nos indique en los requerimientos del proyecto. Repito, es solo para
	                 // comprobar que podemos enviar datos del microcontrolador al PC.

	 				data.point = 1;
	 				data.timeMs = 100;
	 				data.voltage = 0.23;
	 				data.current = 12.3e-6;

	 				/*
	 				 * Debemos de enviar esto desde CoolTerm (un comando inventado):
	 				 * 010100
	 				 *
	 				 * Datos en el microcontrolador antes de la codificacion:
	 				 * 0100000064000000713D0AD7A370CD3F7050B12083CBE93E
	 				 *
	 				 * Datos codificados que debes de recibir en CoolTerm:
	 				 * 020101010264010111713D0AD7A370CD3F7050B12083CBE93E00
	 				 */

	                // Enviamos los datos
	 				MASB_COMM_S_sendData(data);

	 				break;

	 		}

	       // Una vez procesado los comando, esperamos el siguiente
	 		MASB_COMM_S_waitForMessage();

	} else {
		switch (Estado) {
		case CV: //CV

			i=1;

			double vCell_teorica = cvConfiguration.eBegin;
			uint16_t ts = (uint16_t)(cvConfiguration.eStep/cvConfiguration.scanRate * 1000.0);
			vdac = 1.65-vCell_teorica/2;
			MCP4725_SetOutputVoltage(hdac, vdac);
			vObjetivo = cvConfiguration.eVertex1;
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //obre relé, comença mesura
			__HAL_TIM_SET_AUTORELOAD(&htim3, ts * 10); //definim un periode de mesura
			// definit sampling period afora?
			__HAL_TIM_SET_COUNTER(&htim3, 0); //establim un counter a 0
			HAL_TIM_Base_Start_IT(&htim3); //comencem el timer
			agafa_mesura = FALSE;
			cicles = 1;




			while (cicles <= cvConfiguration.cycles) {
				if (agafa_mesura) {

					double sign = ((vCell_teorica + (sign*0.0000001) - vObjetivo)<0)?  1 : -1;
					double eStep = ((vCell_teorica + (sign*0.0000001) - vObjetivo)<0)? cvConfiguration.eStep : -cvConfiguration.eStep;

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

					double vADC = (adcValue/sense)*(1023);
					double vCell = (1.65 - vADC)*2;

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					adcValue = HAL_ADC_GetValue(&hadc1);

					vADC = (adcValue/sense)*(1023);
					double iCell = vCell/rTIA;

					if (!canvi_vObjetivo) {
						dades.point = i;
						dades.timeMs = ts * i;
						//dades.voltage = vCell; // v real que recibe
						dades.voltage = vCell_teorica; // pruebas para ver la vteorica que le mandamos
						//dades.current = iCell;
						dades.current = vCell_teorica / 10e3; // pruebas para ver la corriente en función de una resistencia (una recta)
						i++;
						agafa_mesura = FALSE;
						MASB_COMM_S_sendData(dades);
					} else {
						canvi_vObjetivo = FALSE;
					}


					if (sign * (vCell_teorica - vObjetivo) >= 0) {
						canvi_vObjetivo = TRUE;
						if (fabs(vObjetivo - cvConfiguration.eVertex1) < 0.00000001) {
							vObjetivo = cvConfiguration.eVertex2;
						} else {
							if (fabs(vObjetivo - cvConfiguration.eVertex2) < 0.00000001) {
								vObjetivo = cvConfiguration.eBegin;
							} else {
								vObjetivo = cvConfiguration.eVertex1;
								cicles++;

							}
						}
					} else {
						if (vObjetivo != 0){
							if (fabs(vCell_teorica + eStep) > fabs(vObjetivo)) {
								vCell_teorica = vObjetivo; //fijar tensión
							} else {
								vCell_teorica = vCell_teorica + eStep;
							}
						}
						else {
							if (vCell_teorica + eStep > vObjetivo) {
								vCell_teorica = vObjetivo; //fijar tensión
							} else {
								vCell_teorica = vCell_teorica + eStep;
							}
						}
					}


				}

			}

			HAL_TIM_Base_Stop_IT(&htim3); //parem timer
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); //parem mesura
						Estado = IDLE;

			//Canviar estat a IDLE (Si és l'últim punt)*/
		break;

		case CA: //CA
			vCell = caConfiguration.eDc;
			vdac = 1.65-vCell/2;
			MCP4725_SetOutputVoltage(hdac, vdac);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //tanquem relé (1), iniciem mesura
			__HAL_TIM_SET_AUTORELOAD(&htim3, caConfiguration.samplingPeriodMs*10); //definim un periode de mesura
			// definit sampling period afora?
			__HAL_TIM_SET_COUNTER(&htim3, 0); //establim un counter a 0
			HAL_TIM_Base_Start_IT(&htim3); //comencem el timer
			agafa_mesura = FALSE;
			uint32_t point = 1;
			i = 0;
			uint32_t measurementTimeMs = caConfiguration.measurementTime * 1000;

			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 200);
			uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

			double vADC = (adcValue/sense)*(1023);
			double vCell = (1.65 - vADC)*2;

			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 200);
			adcValue = HAL_ADC_GetValue(&hadc1);


			double iCell = vCell/rTIA;

			dades.point = point;
			dades.timeMs = i;
			dades.voltage = vCell;
			dades.current = iCell;

			i = i + caConfiguration.samplingPeriodMs;
			point++;

			MASB_COMM_S_sendData(dades);
			agafa_mesura = FALSE;


			while (i <= measurementTimeMs) {
				if (agafa_mesura) {
					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

					vADC = (adcValue/sense)*(1023);
					vCell = (1.65 - vADC)*2;

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					adcValue = HAL_ADC_GetValue(&hadc1);


					iCell = vCell/rTIA;




					dades.point = point;
					dades.timeMs = i;
					dades.voltage = vCell;
					dades.current = iCell;

					i = i + caConfiguration.samplingPeriodMs;
					point++;

					MASB_COMM_S_sendData(dades);
					agafa_mesura = FALSE;

					//HAL_ADC_Stop_IT(&hadc1);



				}

			}
			HAL_TIM_Base_Stop_IT(&htim3); //parem timer
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); //parem mesura
			Estado = IDLE;

		break;

		case IDLE: //IDLE
		break;
		}


	}

	 	// Aqui es donde deberia de ir el codigo de control de las mediciones si se quiere implementar
	   // el comando de STOP.

	//Leer el estado en el que estamos (CV, CA ,IDLE)

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	agafa_mesura = TRUE;
}


