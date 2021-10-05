/* 
 * File:   main.h
 * Author: Роман
 *
 * Created on 2 октября 2021 г., 3:55
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <xc.h>
#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include "utils.h"
#include "twi.h"
#include "lcd.h"
#include "encoder.h"
#include "adc.h"

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

