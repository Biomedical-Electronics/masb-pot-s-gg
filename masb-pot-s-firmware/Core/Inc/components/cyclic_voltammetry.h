/*
 * cyclic_voltammetry.h
 *
 *  Created on: May 6, 2021
 *      Author: riza_
 */

#ifndef INC_COMPONENTS_CYCLIC_VOLTAMMETRY_H_
#define INC_COMPONENTS_CYCLIC_VOLTAMMETRY_H_

struct CV_Configuration_S { // estructura que contiene los parámetros de configuración de una voltametría cíclica

 	double eBegin;
 	double eVertex1;
 	double eVertex2;
 	uint8_t cycles;
 	double scanRate;
 	double eStep;

};

#endif /* INC_COMPONENTS_CYCLIC_VOLTAMMETRY_H_ */
