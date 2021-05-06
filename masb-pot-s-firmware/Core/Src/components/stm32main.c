#include "components/stm32main.h"
#include "components/masb_comm_s.h"

struct CV_Configuration_S cvConfiguration;
struct CA_Configuration_S caConfiguration;
struct Data_S data;

// Funcion ejecutada antes del while loop (solo se ejecuta una vez)
// estructura que contenga todos punteros a las diferentes configuraciones de los diferentes periféricos
void setup(struct Handles_S *handles) {
	 MASB_COMM_S_setUart(handles->huart);
	 MASB_COMM_S_waitForMessage(); // componente masb_comm_s espera el primer byte
}

// Funcion ejecutada en el while loop.
void loop(void) {
	if (MASB_COMM_S_dataReceived()) { // Si se ha recibido un mensaje...

	 		switch(MASB_COMM_S_command()) { // Miramos que comando hemos recibido
	 			case START_CA_MEAS:
	 				// Leemos la configuracion que se nos ha enviado en el mensaje y
	 				// la guardamos en la variable caConfiguration
	 				caConfiguration = MASB_COMM_S_getCaConfiguration();
	 			break;

	 			case START_CV_MEAS: // Si hemos recibido START_CV_MEAS

	                 // Leemos la configuracion que se nos ha enviado en el mensaje y
	                 // la guardamos en la variable cvConfiguration
					cvConfiguration = MASB_COMM_S_getCvConfiguration();

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
	}

	 	// Aqui es donde deberia de ir el codigo de control de las mediciones si se quiere implementar
	   // el comando de STOP.
}

