/*
 * usb.h
 *
 *  Created on: 19 èþë. 2018 ã.
 *      Author: DA.Tsekh
 */

#ifndef INC_USB_H_
#define INC_USB_H_

#pragma pack(push,1)
typedef union {
	uint32_t data[2];
	uint8_t d[8];
}CAN_DATA_UNION;

typedef struct {
	uint8_t Ncan;
	uint8_t idtype;
	uint32_t ID;
	uint8_t bts_cnt;
	CAN_DATA_UNION candata;
} USB_TO_CAN_FRAME;
#pragma pack(pop)



void ini_connect_CAN(MDR_CAN_TypeDef* CANx, uint8_t Speed);
void ReverseBytes(uint8_t* d);
uint32_t free_tx_buf(MDR_CAN_TypeDef* CANx);
void init_USB_CDC( uint8_t HSE_in );
void usb_to_can_sent(uint8_t* Buffer, uint32_t Length);
#endif /* INC_USB_H_ */
