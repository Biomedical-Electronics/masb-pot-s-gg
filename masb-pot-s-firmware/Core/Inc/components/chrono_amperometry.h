/*
 * chrono_amperometry.h
 *
 *  Created on: May 6, 2021
 *      Author: riza_
 */

#ifndef INC_COMPONENTS_CHRONO_AMPEROMETRY_H_
#define INC_COMPONENTS_CHRONO_AMPEROMETRY_H_

struct CA_Configuration_S { // estructura que contiene los parámetros de configuración de la amperometría

 	double eBegin;
 	double eVertex1;
 	double eVertex2;
 	uint8_t cycles;
 	double scanRate;
 	double eStep;

};

#endif /* INC_COMPONENTS_CHRONO_AMPEROMETRY_H_ */
