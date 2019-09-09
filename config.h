/*
 * config.h
 *
 *  Created on: 28 џэт. 2019 у.
 *      Author: DA.Tsekh
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "signals.h"
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_eeprom.h"
#include "MDR32F9Qx_power.h"
#include "MDR32F9Qx_adc.h"
#include "MDR32F9Qx_dma.h"
#include "MDR32F9Qx_usb_CDC.h"
#include "MDR32F9Qx_can.h"
#include "MDR32F9Qx_timer.h"
#include "drv_can.h"
#include "usb.h"
#include "Timers.h"

#define USE_HSE				1

#define ALL_PORTS_CLK (RST_CLK_PCLK_PORTA | RST_CLK_PCLK_PORTB | \
                       RST_CLK_PCLK_PORTC | RST_CLK_PCLK_PORTD | \
                       RST_CLK_PCLK_PORTE | RST_CLK_PCLK_PORTF)

#define 	CAN_BUS_OFF					((uint32_t)0x400)

#define 	CAN_BUS_OFF					((uint32_t)0x400)

#define 	HSE_8Mhz					8
#define 	HSE_16Mhz					16
void clock_configure(uint8_t Freq_in);
void LEDs_ini(void);

#endif /* INC_CONFIG_H_ */
