/*
 * main.c
 *
 *  Created on: 17 џэт. 2020 у.
 *      Author: IA.Osnovin
 */

#include "config.h"
#include "stdlib.h"

int main (void)
{
//	CAN_TxMsgTypeDef TxMsg;
//
//	TxMsg.IDE = CAN_ID_EXT;
//	TxMsg.ID = 0x111FF;
//
//	TxMsg.DLC = 8;
//	TxMsg.PRIOR_0 = DISABLE;
//	TxMsg.Data[0] = 0x0;
//	TxMsg.Data[1] = 0x0000000;

	clock_configure(HSE_16Mhz);	/* Configure MCU clocks */
	LEDs_ini();
	init_USB_CDC(HSE_16Mhz);
	init_Timer1();

//	init_CAN( MDR_CAN1, CAN_BaudRate_125kb);

	PORT_ResetBits (MDR_PORTF, PORT_Pin_3);
	PORT_ResetBits (MDR_PORTF, PORT_Pin_2);
	PORT_ResetBits (MDR_PORTF, PORT_Pin_1);
	PORT_ResetBits (MDR_PORTF, PORT_Pin_0);
	while (1)
	{

//		CAN_Transmit(MDR_CAN1, free_tx_buf(MDR_CAN1), &TxMsg);
//		TxMsg.ID += rand()%100;  TxMsg.ID %= 0x20000000;
//		TxMsg.DLC++; TxMsg.DLC %= 9;
//		if (MDR_CAN1->STATUS & 0x100)	/*arbitr (id lower)*/
//		k += rand(); k %= 1000;

//		for (int i=0; i<200000; i++) {;}
//		PORT_ResetBits (MDR_PORTF, PORT_Pin_3);
//		for (int i=0; i<200000; i++) {;}
//		PORT_ResetBits (MDR_PORTF, PORT_Pin_2);
//		for (int i=0; i<200000; i++) {;}
//		PORT_ResetBits (MDR_PORTF, PORT_Pin_1);
//		for (int i=0; i<200000; i++) {;}
//		PORT_ResetBits (MDR_PORTF, PORT_Pin_0);
//
//		for (int i=0; i<200000; i++) {;}
//		PORT_SetBits (MDR_PORTF, PORT_Pin_3);
//		for (int i=0; i<200000; i++) {;}
//		PORT_SetBits (MDR_PORTF, PORT_Pin_2);
//		for (int i=0; i<200000; i++) {;}
//		PORT_SetBits (MDR_PORTF, PORT_Pin_1);
//		for (int i=0; i<200000; i++) {;}
//		PORT_SetBits (MDR_PORTF, PORT_Pin_0);
	}

	return 0;
}

