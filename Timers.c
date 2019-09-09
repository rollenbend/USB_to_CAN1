/*
 * Файл:		Timers.c
 * Описание:	
 * Версия:		1
 * Организация:	ПАО НПО "НАУКА"
 * 
 * История:
 * Версия 1, 14.08.2019:
 * 		- Начальная версия
 */

#include "Timers.h"

uint32_t rx1, rx2;
uint32_t timer;

void Make_mess(MDR_CAN_TypeDef* CANx, CAN_RxMsgTypeDef* RxMessage, USB_TO_CAN_FRAME* frame)
{
	frame->idtype = RxMessage->Rx_Header.IDE;
	if (frame->idtype == CAN_ID_STD) frame->ID = CAN_EXTID_TO_STDID(RxMessage->Rx_Header.ID);
	else frame->ID = RxMessage->Rx_Header.ID;
	frame->bts_cnt = RxMessage->Rx_Header.DLC;
	frame->candata.data[0] = RxMessage->Data[0];
	frame->candata.data[1] = RxMessage->Data[1];

	if (CANx == MDR_CAN1) frame->Ncan = 1; else frame->Ncan = 2;
}


void init_Timer1(void)
{
	TIMER_CntInitTypeDef Timer1_CntInitStructure;
	timer = 0;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE);
	TIMER_DeInit(MDR_TIMER1);

	/* Configure Timer1 for 1ms ticks */
	TIMER_CntStructInit( &Timer1_CntInitStructure );
	Timer1_CntInitStructure.TIMER_IniCounter = 1;
	Timer1_CntInitStructure.TIMER_Prescaler  = 0; /* 80 000 000 Hz */
	Timer1_CntInitStructure.TIMER_Period     = 8000-1; /* 100 us */
	Timer1_CntInitStructure.TIMER_CounterMode      = TIMER_CntMode_ClkFixedDir;
	Timer1_CntInitStructure.TIMER_CounterDirection = TIMER_CntDir_Up;
	Timer1_CntInitStructure.TIMER_EventSource      = TIMER_EvSrc_None;
	Timer1_CntInitStructure.TIMER_FilterSampling   = TIMER_FDTS_TIMER_CLK_div_1;
	Timer1_CntInitStructure.TIMER_ARR_UpdateMode   = TIMER_ARR_Update_Immediately;
	Timer1_CntInitStructure.TIMER_ETR_FilterConf   = TIMER_Filter_1FF_at_TIMER_CLK;
	Timer1_CntInitStructure.TIMER_ETR_Prescaler    = TIMER_ETR_Prescaler_None;
	Timer1_CntInitStructure.TIMER_ETR_Polarity     = TIMER_ETRPolarity_NonInverted;
	Timer1_CntInitStructure.TIMER_BRK_Polarity     = TIMER_BRKPolarity_NonInverted;

	TIMER_ITConfig( MDR_TIMER1, TIMER_STATUS_CNT_ZERO, ENABLE );

	TIMER_CntInit( MDR_TIMER1, &Timer1_CntInitStructure );

	/* Enable TIMER1 clock */
	TIMER_BRGInit(MDR_TIMER1,TIMER_HCLKdiv1);

	TIMER_Cmd( MDR_TIMER1, ENABLE );

	NVIC_EnableIRQ(TIMER1_IRQn);
}


void Timer1_IRQHandler(void)
{
	uint32_t buf = 0;
	uint32_t size = 0;
	CAN_RxMsgTypeDef RxMessage;
	USB_TO_CAN_FRAME frame[2];

	MDR_TIMER1->STATUS = 0;
	timer++;
	if (timer % 300 == 0)
	{
		PORT_ResetBits (MDR_PORTF, PORT_Pin_3);
		PORT_ResetBits (MDR_PORTF, PORT_Pin_2);
		PORT_ResetBits (MDR_PORTF, PORT_Pin_1);
		PORT_ResetBits (MDR_PORTF, PORT_Pin_0);
	}

	if (MDR_CAN1->STATUS & CAN_STATUS_ACK_ERR)			/*ACK*/
	{
		MDR_CAN1->STATUS &= (~CAN_STATUS_ACK_ERR);
	}

	if (MDR_CAN1->STATUS & CAN_STATUS_FRAME_ERR)		/*FRAME*/
	{
		MDR_CAN1->STATUS &= (~CAN_STATUS_FRAME_ERR);
	}
	if (MDR_CAN1->STATUS & CAN_STATUS_CRC_ERR)			/*CRC*/
	{
		MDR_CAN1->STATUS &= (~CAN_STATUS_CRC_ERR);
	}

	if (MDR_CAN1->STATUS & CAN_STATUS_BIT_STUFF_ERR)	/*STUFF*/
	{
		MDR_CAN1->STATUS &= (~CAN_STATUS_BIT_STUFF_ERR);
	}

	if (MDR_CAN1->STATUS & CAN_STATUS_BIT_ERR)			/*BIT*/
	{
		MDR_CAN1->STATUS &= (~CAN_STATUS_BIT_ERR);
	}
	if (MDR_CAN1->STATUS & CAN_BUS_OFF)		/*BUS OFF*/
	{
		// send to usb bus off message
		CAN_Cmd(MDR_CAN1, DISABLE); // deinitialization CAN
		CAN_DeInit(MDR_CAN1);
	}



	for (int i = 0; i < MAX_CAN1_RX_BUFFERS; i++)
	{
		buf = ((uint32_t) 0x1) << i;
		if ((MDR_CAN1->RX & buf) == buf)
		{
			if (size == 30) break;

			PORT_SetBits (MDR_PORTF, PORT_Pin_1);
//			if (rx1++ % 2 == 0) PORT_ResetBits (MDR_PORTF, PORT_Pin_1);
//			else PORT_SetBits (MDR_PORTF, PORT_Pin_1);

			CAN_GetRawReceivedData(MDR_CAN1, i, &RxMessage);
			MDR_CAN1->BUF_CON[i] &= ~CAN_STATUS_RX_FULL;
			if (size == 0) Make_mess(MDR_CAN1, &RxMessage, &frame[0]);
				else Make_mess(MDR_CAN1, &RxMessage, &frame[1]);
			size += sizeof(frame[0]);
		}
	}

	for (int i = 0; i < MAX_CAN2_RX_BUFFERS; i++)
	{
		buf = ((uint32_t) 0x1) << i;
		if ((MDR_CAN2->RX & buf) == buf)
		{
			if (size == 30) break;

			PORT_SetBits (MDR_PORTF, PORT_Pin_3);
//			if (rx2++ % 2 == 0) PORT_ResetBits (MDR_PORTF, PORT_Pin_3);
//			else PORT_SetBits (MDR_PORTF, PORT_Pin_3);

			CAN_GetRawReceivedData(MDR_CAN2, i, &RxMessage);
			MDR_CAN2->BUF_CON[i] &= ~CAN_STATUS_RX_FULL;
			if (size == 0) Make_mess(MDR_CAN2, &RxMessage, &frame[0]);
			else Make_mess(MDR_CAN2, &RxMessage, &frame[1]);
			size += sizeof(frame[0]);
		}
	}

	if (size > 0)
	{
		USB_CDC_SendData((uint8_t*) frame, size);
		size = 0;
	}

}
