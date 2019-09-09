/*
 * drv_can.h
 *
 *  Created on: 12 февр. 2019 г.
 *      Author: IA.Osnovin
 */

#ifndef INC_DRV_CAN_H_
#define INC_DRV_CAN_H_

#include "config.h"
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
#include "usb.h"

#define MAX_CAN1_RX_BUFFERS 16
#define MAX_CAN1_TX_BUFFERS 16
#define MAX_CAN2_RX_BUFFERS 16
#define MAX_CAN2_TX_BUFFERS 16

#define BL_CMD		(u32)0xBE

#define CAN_BaudRate_1mb			4
#define CAN_BaudRate_500kb			9
#define CAN_BaudRate_250kb			19
#define CAN_BaudRate_125kb			39
#define CAN_BaudRate_83kb			59
#define CAN_BaudRate_10kb			499

#pragma pack(push,1)
typedef struct {
	USB_TO_CAN_FRAME frame;
	uint8_t isbuffull;
} CAN_TO_USB_FRAME;
#pragma pack(pop)

void CAN_to_USB_send(MDR_CAN_TypeDef* CANx, CAN_RxMsgTypeDef* RxMessage);
void init_CAN(MDR_CAN_TypeDef* CANx, uint16_t CAN_BaudRate );
void initCAN1_pins( void );
void initCAN2_pins( void );


void init_CAN1(  uint16_t CAN_BaudRate );
void init_CAN2( uint16_t CAN_BaudRate );

#endif /* INC_DRV_CAN_H_ */
