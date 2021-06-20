#include "components/stm32main.h"
#include "components/masb_comm_s.h"
#include "components/ad5280_driver.h"
#include "components/mcp4725_driver.h"
#include "components/i2c_lib.h"
#include <math.h>
#include "main.h"
#include "components/formulas.h"

struct CV_Configuration_S cvConfiguration;
struct CA_Configuration_S caConfiguration;
struct Data_S data;
int8_t Estado; //variable para cambiar el estado de tipo de medida

double vCell = 0;
double cicles = 0;
float vdac = 0;
double rTIA = 1e5;
double vObjetivo = 0;
uint32_t i = 0;
const float sense = 3.3 / 4095; // factor de conversion de ADC a tension
MCP4725_Handle_T hdac = NULL;
volatile _Bool agafa_mesura = FALSE;
volatile _Bool canvi_vObjetivo = FALSE;

// Funcion ejecutada antes del while loop (solo se ejecuta una vez)
// estructura que contenga todos punteros a las diferentes configuraciones de los diferentes periféricos
void setup(struct Handles_S *handles) {
	//encendre alimentació (1)
	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
	HAL_Delay(500);

	//GPIO_pinRelé - apagar relé (no mesura)
	MASB_COMM_S_setUart(handles->huart);
	MASB_COMM_S_waitForMessage(); // componente masb_comm_s espera el primer byte
	I2C_init(handles->hi2c);
	// Creamos el handle de la libreria.

	AD5280_Handle_T hpot = NULL;

	hpot = AD5280_Init();

	// Configuramos su direccion I2C de esclavo, su resistencia total (hay
	// diferentes modelos; este tiene 50kohms) e indicamos que funcion queremos que
	// se encargue de la escritura a traves del I2C. Utilizaremos la funcion
	// I2C_Write de la libreria i2c_lib.
	AD5280_ConfigSlaveAddress(hpot, 0x2C);
	AD5280_ConfigNominalResistorValue(hpot, 50e3f);
	AD5280_ConfigWriteFunction(hpot, I2C_write);

	// Fijamos la resistencia de, por ejemplo, 12kohms.
	AD5280_SetWBResistance(hpot, 50e3f);

	hdac = MCP4725_Init();

	// Configuramos la direccion I2C de esclavo, su tension de referencia (que es la
	// de alimentacion) e indicamos que funcion queremos que se encargue de la
	// escritura a traves del I2C. Utilizaremos la funcion I2C_Write de la libreria
	// i2c_lib.
	MCP4725_ConfigSlaveAddress(hdac, 0x66);
	MCP4725_ConfigVoltageReference(hdac, 4.0f);
	MCP4725_ConfigWriteFunction(hdac, I2C_write);
}

// Funcion ejecutada en el while loop.
void loop(void) {
	if (MASB_COMM_S_dataReceived()) { // Si se ha recibido un mensaje...

		switch (MASB_COMM_S_command()) { // Miramos que comando hemos recibido
		case START_CA_MEAS:
			// Leemos la configuracion que se nos ha enviado en el mensaje y
			// la guardamos en la variable caConfiguration
			caConfiguration = MASB_COMM_S_getCaConfiguration();
			Estado = CA;
			break;

		case START_CV_MEAS: // Si hemos recibido START_CV_MEAS

			// Leemos la configuracion que se nos ha enviado en el mensaje y
			// la guardamos en la variable cvConfiguration
			cvConfiguration = MASB_COMM_S_getCvConfiguration();
			Estado = CV;

			__NOP(); // Esta instruccion no hace nada y solo sirve para poder anadir un breakpoint

			break;

		case STOP_MEAS: // Si hemos recibido STOP_MEAS


			__NOP(); // Esta instruccion no hace nada y solo sirve para poder anadir un breakpoint
			Estado = IDLE;

			break;

		default: // En el caso que se envia un comando que no exista
			data.point = 1;
			data.timeMs = 100;
			data.voltage = 0.23;
			data.current = 12.3e-6;

			// Enviamos los datos
			MASB_COMM_S_sendData(data);

			break;

		}

		// Una vez procesado los comando, esperamos el siguiente
		MASB_COMM_S_waitForMessage();

	} else {
		switch (Estado) {
		case CV: //CV

			i = 1;

			double vCell_teorica = cvConfiguration.eBegin;
			uint16_t ts = (uint16_t) (cvConfiguration.eStep
					/ cvConfiguration.scanRate * 1000.0);
			vdac = calculateDacOutputVoltage(vCell_teorica);
			MCP4725_SetOutputVoltage(hdac, vdac);
			vObjetivo = cvConfiguration.eVertex1;
			HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET); //cierra relé, empieza medición
			__HAL_TIM_SET_AUTORELOAD(&htim3, ts * 10); //definimos un periodo de medición
			__HAL_TIM_SET_COUNTER(&htim3, 0); //establecemos un counter a 0
			HAL_TIM_Base_Start_IT(&htim3); //empezamos el timer
			agafa_mesura = FALSE;
			cicles = 1;

			double sign = (vCell_teorica - vObjetivo < 0) ? 1 : -1;
			double eStep =
					(vCell_teorica - vObjetivo < 0) ?
							cvConfiguration.eStep : -cvConfiguration.eStep;

			while (cicles <= cvConfiguration.cycles) {
				if (agafa_mesura) {

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					uint16_t adcValue = HAL_ADC_GetValue(&hadc1);
					double vCell = calculateVrefVoltage(adcValue);

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					adcValue = HAL_ADC_GetValue(&hadc1);
					double iCell = calculateIcellCurrent(adcValue);

					dades.point = i;
					dades.timeMs = ts * i;
					dades.voltage = vCell; // v real que recibe
					//dades.voltage = vCell_teorica; // pruebas para ver la vteorica que le mandamos
					dades.current = iCell;
					//dades.current = vCell_teorica / 10e3; // pruebas para ver la corriente en función de una resistencia (una recta)
					i++;
					agafa_mesura = FALSE;
					MASB_COMM_S_sendData(dades);


					if (sign * (vCell_teorica - vObjetivo) >= 0) { // Si nos pasamos del objetivo...
						if (fabs(vObjetivo - cvConfiguration.eVertex1)
								< 0.00000001) {
							vObjetivo = cvConfiguration.eVertex2;
							sign = -sign;
							eStep = -eStep;
						} else if (fabs(vObjetivo - cvConfiguration.eVertex2)
								< 0.00000001) {
							vObjetivo = cvConfiguration.eBegin;
							sign = -sign;
							eStep = -eStep;
						} else {
							vObjetivo = cvConfiguration.eVertex1;
							cicles++;
						}
					} else {

						if (fabs(vObjetivo - cvConfiguration.eVertex1)
								< 0.00000001) {
							if (vCell_teorica + eStep > vObjetivo) {
								vCell_teorica = vObjetivo; //fijar tensión
							} else {
								vCell_teorica = vCell_teorica + eStep;
							}
						} else if (fabs(vObjetivo - cvConfiguration.eVertex2)
								< 0.00000001) {
							if (vCell_teorica + eStep < vObjetivo) {
								vCell_teorica = vObjetivo; //fijar tensión
							} else {
								vCell_teorica = vCell_teorica + eStep;
							}
						} else {
							if (vCell_teorica + eStep > vObjetivo) {
								vCell_teorica = vObjetivo; //fijar tensión
							} else {
								vCell_teorica = vCell_teorica + eStep;
							}
						}

					}

					vdac = calculateDacOutputVoltage(vCell_teorica);
					MCP4725_SetOutputVoltage(hdac, vdac);

				}

			}

			HAL_TIM_Base_Stop_IT(&htim3); //paramos timer
			HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET); //paramos medición
			Estado = IDLE; //Cambiar estado a IDLE (Si es el último punto)


			break;

		case CA: //CA
			vCell = caConfiguration.eDc;
			vdac = calculateDacOutputVoltage(vCell);
			MCP4725_SetOutputVoltage(hdac, vdac);
			HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET); //cerramos relé (1), iniciamos medición
			__HAL_TIM_SET_AUTORELOAD(&htim3,
					caConfiguration.samplingPeriodMs * 10); //definimos periodo de muestra
			__HAL_TIM_SET_COUNTER(&htim3, 0); //establecemos un counter a 0
			HAL_TIM_Base_Start_IT(&htim3); //empezamos el timer
			agafa_mesura = FALSE;
			uint32_t point = 1;
			i = 0;
			uint32_t measurementTimeMs = caConfiguration.measurementTime * 1000;

			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 200);
			uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

			double vCell = calculateVrefVoltage(adcValue);

			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 200);
			adcValue = HAL_ADC_GetValue(&hadc1);

			double iCell = calculateIcellCurrent(adcValue);

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

					vCell = calculateVrefVoltage(adcValue);

					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1, 200);
					adcValue = HAL_ADC_GetValue(&hadc1);

					iCell = calculateIcellCurrent(adcValue);

					dades.point = point;
					dades.timeMs = i;
					dades.voltage = vCell;
					dades.current = iCell;

					i = i + caConfiguration.samplingPeriodMs; //incrementamos el tiempo y el punto
					point++;

					MASB_COMM_S_sendData(dades); //enviamos datos a host
					agafa_mesura = FALSE;

				}

			}
			HAL_TIM_Base_Stop_IT(&htim3); //paramos el timer
			HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
			Estado = IDLE;

			break;

		case IDLE: //IDLE
			break; //no pasa nada
		}

	}

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	agafa_mesura = TRUE;
}

