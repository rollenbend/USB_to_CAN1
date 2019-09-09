/*
 * can.c
 *
 *  Created on: 30 θών. 2017 γ.
 *      Author: DA.Tsekh
 */

#include "config.h"
#include "drv_can.h"

#define ENABLE_CAN1
#define ENABLE_CAN2



void CAN_to_USB_send(MDR_CAN_TypeDef* CANx, CAN_RxMsgTypeDef* RxMessage)
{
	USB_TO_CAN_FRAME frame;

	frame.idtype = RxMessage->Rx_Header.IDE;
	if (frame.idtype == CAN_ID_STD) frame.ID = CAN_EXTID_TO_STDID(RxMessage->Rx_Header.ID);
	else frame.ID = RxMessage->Rx_Header.ID;
	frame.bts_cnt = RxMessage->Rx_Header.DLC;
	frame.candata.data[0] = RxMessage->Data[0];
	frame.candata.data[1] = RxMessage->Data[1];

	if (CANx == MDR_CAN1) frame.Ncan = 1; else frame.Ncan = 2;
	USB_CDC_SendData((uint8_t*) &frame, sizeof(frame));
}

int k = 0;
void CAN1_IRQHandler(void)
{
#ifdef ENABLE_CAN1

	CAN_RxMsgTypeDef RxMessage;
	CAN_TxMsgTypeDef TxMsg;

//	if (MDR_CAN1->STATUS & CAN_STATUS_BIT_STUFF_ERR)	/*STUFF*/
//	{
//		MDR_CAN1->STATUS &= (~CAN_STATUS_BIT_STUFF_ERR);
//	}
//	if (MDR_CAN1->STATUS & 0x100)	/*arbitr (id lower)*/
//	{
//		MDR_CAN1->STATUS &= (~0x100);
//	}

	for (int i = 0; i < MAX_CAN1_RX_BUFFERS; i++)
	{
		if (CAN_GetRxITStatus( MDR_CAN1, i) == SET)
		{
			CAN_GetRawReceivedData(MDR_CAN1, i, &RxMessage);
			CAN_ITClearRxTxPendingBit(MDR_CAN1, i, CAN_STATUS_RX_READY);
			k++;

//			TxMsg.IDE = RxMessage.Rx_Header.IDE;
//			TxMsg.ID = RxMessage.Rx_Header.ID;
//			TxMsg.DLC = RxMessage.Rx_Header.DLC;
//			TxMsg.PRIOR_0 = DISABLE;
//			TxMsg.Data[0] = RxMessage.Data[0];
//			TxMsg.Data[1] = RxMessage.Data[1];
//
//			CAN_Transmit(MDR_CAN1, free_tx_buf(MDR_CAN1), &TxMsg);
//			break;
		}
	}


//	CAN_to_USB_send(MDR_CAN1, &RxMessage);


#endif
}


void CAN2_IRQHandler(void)
{
#ifdef ENABLE_CAN2
	CAN_RxMsgTypeDef RxMessage;

	for (int i = 0; i < MAX_CAN2_RX_BUFFERS; i++)
	{
		if (CAN_GetRxITStatus( MDR_CAN2, i) == SET)
		{
			CAN_GetRawReceivedData(MDR_CAN2, i, &RxMessage);
			CAN_ITClearRxTxPendingBit(MDR_CAN2, i, CAN_STATUS_RX_READY);
			break;
		}
	}


//	CAN_to_USB_send(MDR_CAN2, &RxMessage);

#endif
}


void init_CAN(MDR_CAN_TypeDef* CANx, uint16_t CAN_BaudRate ){
	CAN_InitTypeDef  sCAN;

	CAN_Cmd(CANx, DISABLE);
	/* Periph clocks enable */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_RST_CLK, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_CAN1, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_CAN2, ENABLE);

	if (CANx == MDR_CAN1) initCAN1_pins();
	if (CANx == MDR_CAN2) initCAN2_pins();

	/* Set the HCLK division factor = 1 for CAN1*/
	CAN_BRGInit(CANx, CAN_HCLKdiv1);

	/* CAN register init */
	CAN_DeInit(CANx);

	/* CAN cell init */
	CAN_StructInit (&sCAN);

	sCAN.CAN_ROP  = DISABLE;
	sCAN.CAN_SAP  = DISABLE;
	sCAN.CAN_STM  = DISABLE;
	sCAN.CAN_ROM  = DISABLE;
	sCAN.CAN_PSEG = CAN_PSEG_Mul_3TQ;
	sCAN.CAN_SEG1 = CAN_SEG1_Mul_8TQ;
	sCAN.CAN_SEG2 = CAN_SEG2_Mul_4TQ;
	sCAN.CAN_SJW  = CAN_SJW_Mul_1TQ;
	sCAN.CAN_SB   = CAN_SB_1_SAMPLE;
	sCAN.CAN_BRP  = CAN_BaudRate;
	sCAN.CAN_OVER_ERROR_MAX = 255;

	if (CAN_BaudRate == CAN_BaudRate_1mb)
	{
		sCAN.CAN_PSEG = CAN_PSEG_Mul_2TQ;
		sCAN.CAN_SEG1 = CAN_SEG1_Mul_3TQ;
		sCAN.CAN_SEG2 = CAN_SEG2_Mul_2TQ;
		sCAN.CAN_BRP  = 9;
	}
	CAN_Init (CANx, &sCAN);

	CAN_Cmd(CANx, ENABLE);


	for(int i = 0; i < MAX_CAN2_RX_BUFFERS; i++){
		/* Enable CAN2 interrupt from receive buffer */
		CAN_RxITConfig( CANx, ((u32)(0x01) << i), ENABLE);
		/* receive buffer enable */
		CAN_Receive(CANx, i, ENABLE);
	}

	for(int i = MAX_CAN2_RX_BUFFERS; i < (MAX_CAN2_RX_BUFFERS + MAX_CAN2_TX_BUFFERS); i++){
		/* Enable CAN1 interrupt from transmit buffer */
		CAN_TxITConfig( CANx, ((u32)(0x01) << i), ENABLE);
		CANx->BUF_CON[i] |= CAN_BUF_CON_EN;
	}

//	/* Enable CAN1 GLB_INT and RX_INT interrupts */
//	CAN_ITConfig( CANx, CAN_IT_GLBINTEN | CAN_IT_RXINTEN, ENABLE);
//
//	/* Enable CAN2 interrupt */
//	if (CANx == MDR_CAN1) NVIC_EnableIRQ(CAN1_IRQn);
//	if (CANx == MDR_CAN2) NVIC_EnableIRQ(CAN2_IRQn);
}

void initCAN1_pins( void ){
	PORT_InitTypeDef PORT_InitStructure;

	/* Enables the RTCHSE clock on PORTC */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTE, ENABLE);

	/* Configure PORTA pin 9 for output */
	PORT_InitStructure.PORT_Pin = (PORT_Pin_0);
	PORT_InitStructure.PORT_OE = PORT_OE_IN;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
	PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
	PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	PORT_Init(MDR_PORTE, &PORT_InitStructure); //CAN1 Tx pin

	/* Configure PORTA pin 9 for input */
	PORT_InitStructure.PORT_Pin = (PORT_Pin_1);
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
	PORT_Init(MDR_PORTE, &PORT_InitStructure); //CAN1 Rx pin

//	/* Enables the RTCHSE clock on PORTC */
//		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA, ENABLE);
//
//		/* Configure PORTA pin 9 for output */
//		PORT_InitStructure.PORT_Pin = (PORT_Pin_7);
//		PORT_InitStructure.PORT_OE = PORT_OE_IN;
//		PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
//		PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
//		PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
//		PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
//		PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
//		PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
//		PORT_Init(MDR_PORTA, &PORT_InitStructure); //CAN1 Tx pin
//
//		/* Configure PORTA pin 9 for input */
//		PORT_InitStructure.PORT_Pin = (PORT_Pin_6);
//		PORT_InitStructure.PORT_OE = PORT_OE_OUT;
//		PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
//		PORT_Init(MDR_PORTA, &PORT_InitStructure); //CAN1 Rx pin
}

void initCAN2_pins( void ){
	PORT_InitTypeDef PORT_InitStructure;

	/* Enables the RTCHSE clock on PORTC */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTE, ENABLE);

	/* Configure PORTf pin 2 for input */
	PORT_InitStructure.PORT_Pin = PORT_Pin_7;
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTE, &PORT_InitStructure);

	/* Configure PORTf pin 3 for output */
	PORT_InitStructure.PORT_Pin = PORT_Pin_6;
	PORT_InitStructure.PORT_OE = PORT_OE_IN;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTE, &PORT_InitStructure);
//	/* Enables the RTCHSE clock on PORTC */
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);
//
//	/* Configure PORTf pin 2 for input */
//	PORT_InitStructure.PORT_Pin = PORT_Pin_3;
//	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
//	PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
//	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
//	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
//	PORT_Init(MDR_PORTF, &PORT_InitStructure);
//
//	/* Configure PORTf pin 3 for output */
//	PORT_InitStructure.PORT_Pin = PORT_Pin_2;
//	PORT_InitStructure.PORT_OE = PORT_OE_IN;
//	PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
//	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
//	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
//	PORT_Init(MDR_PORTF, &PORT_InitStructure);
}




//void sendcan2(void)
//{
//	u8 stat,stat2;
//	stat2 = CAN_GetBufferStatus(MDR_CAN2, MAX_CAN2_RX_BUFFERS+1);
//	stat = CAN_GetTxITStatus( MDR_CAN2, MAX_CAN2_RX_BUFFERS+1);
//	CAN_TxMsgTypeDef TxMsg;
//	TxMsg.IDE = CAN_ID_STD;
//	TxMsg.DLC = 4;
//	TxMsg.PRIOR_0 = DISABLE;
//	TxMsg.ID = CAN_STDID_TO_EXTID(1);
//	TxMsg.Data[1] = 0x00;
//	TxMsg.Data[0] = 0x412fc230;
//	CAN_Transmit(MDR_CAN2, MAX_CAN2_RX_BUFFERS+1, &TxMsg);
//	stat2 = CAN_GetBufferStatus(MDR_CAN2, MAX_CAN2_RX_BUFFERS+1);
//	stat = CAN_GetTxITStatus( MDR_CAN2, MAX_CAN2_RX_BUFFERS+1);
//}
//
//
//
//void init_CAN1(  uint16_t CAN_BaudRate  ){
//	CAN_InitTypeDef  sCAN;
//
//	/* Periph clocks enable */
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_RST_CLK, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_CAN1, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA,	ENABLE);
//
//	initCAN1_pins();
//
//	/* Set the HCLK division factor = 1 for CAN1*/
//	CAN_BRGInit(MDR_CAN1, CAN_HCLKdiv1);
//
//	/* CAN register init */
//	CAN_DeInit(MDR_CAN1);
//
//	/* CAN cell init */
//	CAN_StructInit (&sCAN);
//
//	sCAN.CAN_ROP  = DISABLE;
//	sCAN.CAN_SAP  = DISABLE;
//	sCAN.CAN_STM  = DISABLE;
//	sCAN.CAN_ROM  = DISABLE;
//	sCAN.CAN_PSEG = CAN_PSEG_Mul_3TQ;
//	sCAN.CAN_SEG1 = CAN_SEG1_Mul_8TQ;
//	sCAN.CAN_SEG2 = CAN_SEG2_Mul_4TQ;
//	sCAN.CAN_SJW  = CAN_SJW_Mul_1TQ;
//	sCAN.CAN_SB   = CAN_SB_1_SAMPLE;
//	sCAN.CAN_BRP  = CAN_BaudRate;
//	sCAN.CAN_OVER_ERROR_MAX = 255;
//	CAN_Init (MDR_CAN1, &sCAN);
//
//	CAN_Cmd(MDR_CAN1, ENABLE);
//
//	for(int i = 0; i < MAX_CAN1_RX_BUFFERS; i++){
//		/* Enable CAN1 interrupt from receive buffer */
//		CAN_RxITConfig( MDR_CAN1, ((u32)(0x01) << i), ENABLE);
//		/* receive buffer enable */
//		CAN_Receive(MDR_CAN1, i, ENABLE);
//	}
//
//	for(int i = MAX_CAN1_RX_BUFFERS; i < (MAX_CAN1_TX_BUFFERS + MAX_CAN1_RX_BUFFERS); i++){
//		/* Enable CAN1 interrupt from transmit buffer */
//		CAN_TxITConfig( MDR_CAN1, ((u32)(0x01) << i), ENABLE);
//		MDR_CAN1->BUF_CON[i] |= CAN_BUF_CON_EN;
//	}
//
//	/* Enable CAN1 GLB_INT and RX_INT interrupts */
//	CAN_ITConfig( MDR_CAN1, CAN_IT_GLBINTEN | CAN_IT_RXINTEN, ENABLE);
//
//	/* Enable CAN1 interrupt */
//	NVIC_EnableIRQ(CAN1_IRQn);
//
//}
//
//void receivecan(void)
//{
//	CAN_RxMsgTypeDef RxMessage;
////	for (int i = 0; i < MAX_CAN1_RX_BUFFERS - 1; i++)
////	{
////		if ((MDR_CAN1->RX  & (((uint32_t)0x1) << i)) == SET)
////		{
////			CAN_GetRawReceivedData(MDR_CAN1, i, &RxMessage);
////			CAN_ITClearRxTxPendingBit(MDR_CAN1, i, CAN_STATUS_RX_READY);
////		}
////	}
//
//	u8 buf_num = 0;
//	while ((MDR_CAN1->RX & (((uint32_t) 0x1) << buf_num)) == 0)
//	{
//		buf_num++;
//		buf_num %= 16;
//	}
//	CAN_GetRawReceivedData(MDR_CAN1, buf_num, &RxMessage);
//	MDR_CAN1->BUF_CON[buf_num] &= ~CAN_STATUS_RX_FULL;
//
//	if (CAN_EXTID_TO_STDID(RxMessage.Rx_Header.ID) == 222)
//	{
//		int k = 0;
//	}
//}
//
//
//void init_CAN2( uint16_t CAN_BaudRate ){
//	CAN_InitTypeDef  sCAN;
//
//	CAN_Cmd(MDR_CAN2, DISABLE);
//	/* Periph clocks enable */
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_RST_CLK, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_CAN2, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF,	ENABLE);
//
//	initCAN2_pins();
//
//	/* Set the HCLK division factor = 1 for CAN1*/
//	CAN_BRGInit(MDR_CAN2, CAN_HCLKdiv1);
//
//	/* CAN register init */
//	CAN_DeInit(MDR_CAN2);
//
//	/* CAN cell init */
//	CAN_StructInit (&sCAN);
//
//	sCAN.CAN_ROP  = DISABLE;
//	sCAN.CAN_SAP  = DISABLE;
//	sCAN.CAN_STM  = DISABLE;
//	sCAN.CAN_ROM  = DISABLE;
//	sCAN.CAN_PSEG = CAN_PSEG_Mul_3TQ;
//	sCAN.CAN_SEG1 = CAN_SEG1_Mul_8TQ;
//	sCAN.CAN_SEG2 = CAN_SEG2_Mul_4TQ;
//	sCAN.CAN_SJW  = CAN_SJW_Mul_1TQ;
//	sCAN.CAN_SB   = CAN_SB_1_SAMPLE;
//	sCAN.CAN_BRP  = CAN_BaudRate; // 4=1Mb/s, 9=500 kb/s, 19=250kb/s, 39=125kb/s, 499=10kb/s
//	sCAN.CAN_OVER_ERROR_MAX = 255;
//
//	CAN_Init (MDR_CAN2, &sCAN);
//
//	CAN_Cmd(MDR_CAN2, ENABLE);
//
//
//	for(int i = 0; i < MAX_CAN2_RX_BUFFERS; i++){
//		/* Enable CAN2 interrupt from receive buffer */
//		CAN_RxITConfig( MDR_CAN2, ((u32)(0x01) << i), ENABLE);
//		/* receive buffer enable */
//		CAN_Receive(MDR_CAN2, i, ENABLE);
//	}
//
//	for(int i = MAX_CAN2_RX_BUFFERS; i < (MAX_CAN2_RX_BUFFERS + MAX_CAN2_TX_BUFFERS); i++){
//		/* Enable CAN1 interrupt from transmit buffer */
//		CAN_TxITConfig( MDR_CAN2, ((u32)(0x01) << i), ENABLE);
//		MDR_CAN2->BUF_CON[i] |= CAN_BUF_CON_EN;
//	}
//
//	/* Enable CAN1 GLB_INT and RX_INT interrupts */
//	CAN_ITConfig( MDR_CAN2, CAN_IT_GLBINTEN | CAN_IT_RXINTEN , ENABLE);
//
//	/* Enable CAN2 interrupt */
//	NVIC_EnableIRQ(CAN2_IRQn);
//}
//
//void sendcan(void)
//{
//	u8 stat,stat2;
//	stat2 = CAN_GetBufferStatus(MDR_CAN1, MAX_CAN1_TX_BUFFERS+1);
//	stat = CAN_GetTxITStatus( MDR_CAN1, MAX_CAN1_TX_BUFFERS+1);
//	CAN_TxMsgTypeDef TxMsg;
//	TxMsg.IDE = CAN_ID_STD;
//	TxMsg.DLC = 8;
//	TxMsg.PRIOR_0 = DISABLE;
//	TxMsg.ID = CAN_STDID_TO_EXTID(111);
//	TxMsg.Data[1] = 0x000;
//	TxMsg.Data[0] = 1111;
//	CAN_Transmit(MDR_CAN1, MAX_CAN1_TX_BUFFERS+1, &TxMsg);
//	stat2 = CAN_GetBufferStatus(MDR_CAN1, MAX_CAN1_TX_BUFFERS+1);
//	stat = CAN_GetTxITStatus( MDR_CAN1, MAX_CAN1_TX_BUFFERS+1);
//}



/*CAN_TO_USB_FRAME canframe[1];
CAN_DATA_UNION can;
int c = 0;
can.data[0] = RxMessage->Data[0];
can.data[1] = RxMessage->Data[1];
for (int i = 0; i < 8; i++)
	canframe[c].frame.data[i] = can.d[i];

canframe[c].frame.bts_cnt = RxMessage->Rx_Header.DLC;
canframe[c].isbuffull = 1;

if (RxMessage->Rx_Header.IDE == CAN_ID_STD)
{
	canframe[c].frame.idstd = CAN_EXTID_TO_STDID(RxMessage->Rx_Header.ID);
	USB_CDC_SendData((uint8_t*) &canframe[c].frame.idstd, sizeof(canframe[c].frame) - 2);
}
else
{
	*((uint32_t*) &canframe[c].frame.idex) = RxMessage->Rx_Header.ID;
	USB_CDC_SendData((uint8_t*) &canframe[c].frame.idex, sizeof(canframe[c].frame));
}*/
//	while (canframe[c].isbuffull != 0 && c < 3)
//	{
//		c++;
//	}
//	canframe[c].frame.end_str = '\n';
//canframe[c].frame.data[0] = (u8) RxMessage.Data[0];
//	canframe[c].frame.data[1] = (u8) (RxMessage.Data[0] >> 8);
//	canframe[c].frame.data[2] = (u8) (RxMessage.Data[0] >> 16);
//	canframe[c].frame.data[3] = (u8) (RxMessage.Data[0] >> 24);
//
//	canframe[c].frame.data[4] = (u8) RxMessage.Data[1];
//	canframe[c].frame.data[5] = (u8) (RxMessage.Data[1] >> 8);
//	canframe[c].frame.data[6] = (u8) (RxMessage.Data[1] >> 16);
//	canframe[c].frame.data[7] = (u8) (RxMessage.Data[1] >> 24); /*BIG ENDIAN*/
//
//	if (dlc > 0)
//		for (int i = dlc - 1; i >= 0; i--)
//			canframe[c].frame.data[i + (8 - dlc)] = canframe[c].frame.data[i];
//
//	for (int i = 0; i < (8 - dlc); i++)
//		canframe[c].frame.data[i] = 0;

	/* This is reverse bytes section from big to little */
//	u8 bc;
//	for (int i = 0; i < 4; i++)
//	{
//		bc = canframe[c].frame.data[i];
//		canframe[c].frame.data[i] = canframe[c].frame.data[7 - i]; /*LITTLE ENDIAN*/
//		canframe[c].frame.data[7 - i] = bc;
//	}


/*if (dlc == 1)
	{
		canframe[c].frame.data[0] = (u8) (RxMessage.Data[0] >> 0);
		canframe[c].frame.data[1] = 0;
		canframe[c].frame.data[2] = 0;
		canframe[c].frame.data[3] = 0;
	}
	if (dlc == 2)
	{
		canframe[c].frame.data[0] = (u8) (RxMessage.Data[0] >> 8);
		canframe[c].frame.data[1] = (u8) (RxMessage.Data[0] >> 0);
		canframe[c].frame.data[2] = 0;
		canframe[c].frame.data[3] = 0;
	}
	if (dlc == 3)
	{
		canframe[c].frame.data[0] = (u8) (RxMessage.Data[0] >> 16);
		canframe[c].frame.data[1] = (u8) (RxMessage.Data[0] >> 8);
		canframe[c].frame.data[2] = (u8) (RxMessage.Data[0] >> 0);
		canframe[c].frame.data[3] = 0;
	}
	if (dlc>=4)
	{
		canframe[c].frame.data[0] = (u8)(RxMessage.Data[0]>>24);
		canframe[c].frame.data[1] = (u8)(RxMessage.Data[0]>>16);
		canframe[c].frame.data[2] = (u8)(RxMessage.Data[0]>>8);
		canframe[c].frame.data[3] = (u8)(RxMessage.Data[0]>>0);
	}

	if (dlc>=4)
	{
	canframe[c].frame.data[4] = (u8)(RxMessage.Data[1]>>24);    /*LITTLE ENDIAN*/
	/*canframe[c].frame.data[5] = (u8)(RxMessage.Data[1]>>16);
	canframe[c].frame.data[6] = (u8)(RxMessage.Data[1]>>8);
	canframe[c].frame.data[7] = (u8)(RxMessage.Data[1]>>0);
	}*/
//	while (canframe[c].isbuffull != 0 && c<10)
	 //	{
	 //		c++;
	 //	}
	 //	for (int i=7; i>=0; i--)
	 //	{
	 //		if (RxMessage.Rx_Header.DLC > i)
	 //			if (i < 4) canframe[c].frame.data[i] = RxMessage.Data[0] >> i*8;
	 //			else canframe[c].frame.data[i] = RxMessage.Data[1] >> (i-4)*8;
	 //		else canframe[c].frame.data[i] = 0;
	 //	}
//if (canframe[c].frame.id == 1) flag=3;
//else if (canframe[c].frame.id == 0x43) flag=2;
//else flag = 1;
//	USB_Result result;
//	result = USB_CDC_SendData((uint8_t*)&frame, sizeof(frame));

//	if (CAN_EXTID_TO_STDID(RxMessage.Rx_Header.ID) == 111)
//	{
//		int k=0;
//	}
//	for (int i = 0; i < MAX_CAN2_TX_BUFFERS - 1; i++)
//	{
//		if (CAN_GetTxITStatus( MDR_CAN2, i) == SET)
//		{
//			CAN_ITClearRxTxPendingBit(MDR_CAN2, MAX_CAN2_RX_BUFFERS + i, CAN_STATUS_TX_READY);
//		}
//	}
