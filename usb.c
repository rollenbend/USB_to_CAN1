/*
 * usb.c
 *
 *  Created on: 19 èþë. 2018 ã.
 *      Author: DA.Tsekh
 */


#include "MDR32F9Qx_usb_handlers.h"
#include "MDR32F9Qx_rst_clk.h"
#include "usb.h"
#include "drv_can.h"

#define BUFFER_LENGTH                        256
USB_Clock_TypeDef USB_Clock_InitStruct;
USB_DeviceBUSParam_TypeDef USB_DeviceBUSParam;
static uint8_t USB_RX_Buffer[BUFFER_LENGTH];
#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
#endif /* USB_CDC_LINE_CODING_SUPPORTED */
#ifdef USB_VCOM_SYNC
volatile uint32_t PendingDataLength = 0;
#endif /* USB_VCOM_SYNC */

uint32_t c1, c2;

void usb_to_can_sent(uint8_t* Buffer, uint32_t Length)
{
	if (Length == 15)
	{
		CAN_TxMsgTypeDef TxMsg;
		CAN_DATA_UNION can;

		TxMsg.IDE = Buffer[1];
		if (TxMsg.IDE == CAN_ID_STD) TxMsg.ID = CAN_STDID_TO_EXTID((*(uint32_t*)(Buffer + 2)) % (0x7FF + 1));
		else TxMsg.ID = (*(uint32_t*)(Buffer + 2)) % (0x1FFFFFFF + 1);
		TxMsg.DLC = Buffer[6] % 9;
		TxMsg.PRIOR_0 = DISABLE;
		for (int i = 0; i < 8; i++)
			can.d[i] = Buffer[i + 7];
		TxMsg.Data[0] = can.data[0];
		TxMsg.Data[1] = can.data[1];
		if (Buffer[0] == 1)
		{
//			if (c1++ % 2 == 0) PORT_ResetBits (MDR_PORTF, PORT_Pin_0);
//			else PORT_SetBits (MDR_PORTF, PORT_Pin_0);
			PORT_SetBits (MDR_PORTF, PORT_Pin_0);
			CAN_Transmit(MDR_CAN1, free_tx_buf(MDR_CAN1), &TxMsg);
		}
		else
		{
			PORT_SetBits (MDR_PORTF, PORT_Pin_2);
//			if (c2++ % 2 == 0) PORT_ResetBits (MDR_PORTF, PORT_Pin_2);
//			else PORT_SetBits (MDR_PORTF, PORT_Pin_2);
			CAN_Transmit(MDR_CAN2, free_tx_buf(MDR_CAN2), &TxMsg);
		}
	}

	if (Length == 3 && Buffer[1] == 'S')
	{
		NVIC_DisableIRQ(TIMER1_IRQn);
		if (Buffer[0] == 1) ini_connect_CAN(MDR_CAN1, Buffer[2]);
		if (Buffer[0] == 2) ini_connect_CAN(MDR_CAN2, Buffer[2]);
		uint8_t response[2] = { Buffer[0], 0x79 };
		USB_CDC_SendData(response, 2);
		NVIC_EnableIRQ(TIMER1_IRQn);
	}
}

uint32_t free_tx_buf(MDR_CAN_TypeDef* CANx)
{
	uint32_t buf_num = MAX_CAN2_RX_BUFFERS;
	for ( ; buf_num < (MAX_CAN2_RX_BUFFERS + MAX_CAN2_TX_BUFFERS); buf_num++)
		if (CANx->TX & (((uint32_t) 0x1) << buf_num)) return buf_num;
	return buf_num;
}
void ini_connect_CAN(MDR_CAN_TypeDef* CANx, uint8_t Speed)
{
	switch (Speed)
	{
	case 0:
		init_CAN( CANx, CAN_BaudRate_83kb);
		break; // 83.3kbit
	case 1:
		init_CAN( CANx, CAN_BaudRate_125kb);
		break;  // 125kbit
	case 2:
		init_CAN( CANx, CAN_BaudRate_250kb);
		break;  // 250kbit
	case 3:
		init_CAN( CANx, CAN_BaudRate_500kb);
		break;  // 500kbit
	case 4:
		init_CAN( CANx, CAN_BaudRate_1mb);
		break;   // 1Mbit
	case 5:
		CAN_Cmd(CANx, DISABLE); // deinitialization CAN
		CAN_DeInit(CANx);
		break;   // turn off CANx
	}
}
/* USB_CDC_HANDLE_DATA_RECEIVE implementation - data echoing */
USB_Result USB_CDC_RecieveData ( uint8_t* Buffer, uint32_t Length )
{
	USB_Result result;

#ifdef USB_DEBUG_PROTO
	ReceivedByteCount += Length;
#endif /* USB_DEBUG_PROTO */


	usb_to_can_sent(Buffer, Length); // receive data and go it in CAN

	/* Send back received data portion */
//	result = USB_CDC_SendData(Buffer, Length);
	uint8_t pxChar;
	uint32_t i=0;
	while( i < Length ){
		pxChar = Buffer[i];
		result = USB_SUCCESS;
		i++;
	}


#ifdef USB_DEBUG_PROTO
	if (result == USB_SUCCESS) {
		SentByteCount += Length;
	}
#ifndef USB_VCOM_SYNC
	else
	{
		SkippedByteCount += Length;
	}
#endif /* !USB_VCOM_SYNC */
#endif /* USB_DEBUG_PROTO */

#ifdef USB_VCOM_SYNC
	if (result != USB_SUCCESS) {
		/* If data cannot be sent now, it will await nearest possibility
		 * (see USB_CDC_DataSent) */
		PendingDataLength = Length;
	}
	return result;
#else
	return USB_SUCCESS;
#endif /* USB_VCOM_SYNC */
}



/* USB protocol debugging */
#ifdef USB_DEBUG_PROTO

#define USB_DEBUG_NUM_PACKETS   100

typedef struct {
	USB_SetupPacket_TypeDef packet;
	uint32_t address;
} TDebugInfo;

static TDebugInfo SetupPackets[USB_DEBUG_NUM_PACKETS];
static uint32_t SPIndex;
static uint32_t ReceivedByteCount, SentByteCount, SkippedByteCount;

#endif /* USB_DEBUG_PROTO */


/* USB Device layer setup and powering on */
void Setup_USB ( uint8_t HSE_in )
{
	/* Enables the CPU_CLK clock on USB */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_USB, ENABLE);

	/* Device layer initialization */
	if (HSE_in == HSE_8Mhz)
	{
		USB_Clock_InitStruct.USB_USBC1_Source = USB_C1HSEdiv1;
	}
	if (HSE_in == HSE_16Mhz)
	{
		USB_Clock_InitStruct.USB_USBC1_Source = USB_C1HSEdiv2;
	}
	USB_Clock_InitStruct.USB_PLLUSBMUL = USB_PLLUSBMUL6;

	USB_DeviceBUSParam.MODE = USB_SC_SCFSP_Full;
	USB_DeviceBUSParam.SPEED = USB_SC_SCFSR_12Mb;
	USB_DeviceBUSParam.PULL = USB_HSCR_DP_PULLUP_Set;

	USB_DeviceInit(&USB_Clock_InitStruct, &USB_DeviceBUSParam);

	/* Enable all USB interrupts */
	USB_SetSIM(USB_SIS_Msk);

	USB_DevicePowerOn();

	/* Enable interrupt on USB */
#ifdef USB_INT_HANDLE_REQUIRED
	NVIC_EnableIRQ(USB_IRQn);
#endif /* USB_INT_HANDLE_REQUIRED */

	USB_DEVICE_HANDLE_RESET;
}


/* Example-relating data initialization */
void VCom_Configuration ( void )
{
#ifdef USB_CDC_LINE_CODING_SUPPORTED
	LineCoding.dwDTERate = 115200;
	LineCoding.bCharFormat = 0;
	LineCoding.bParityType = 0;
	LineCoding.bDataBits = 8;
#endif /* USB_CDC_LINE_CODING_SUPPORTED */
}

void init_USB_CDC( uint8_t HSE_in ){
	VCom_Configuration();
	/* CDC layer initialization */
	USB_CDC_Init(USB_RX_Buffer, 1, SET);
	Setup_USB(HSE_in);
}



#ifdef USB_VCOM_SYNC

/* USB_CDC_HANDLE_DATA_SENT implementation - sending of pending data */
USB_Result USB_CDC_DataSent ( void )
{
	USB_Result result = USB_SUCCESS;

	if (PendingDataLength) {
		result = USB_CDC_SendData(USB_RX_Buffer, PendingDataLength);
#ifdef USB_DEBUG_PROTO
		if (result == USB_SUCCESS) {
			SentByteCount += PendingDataLength;
		}
		else {
			SkippedByteCount += PendingDataLength;
		}
#endif /* USB_DEBUG_PROTO */
		PendingDataLength = 0;
		USB_CDC_ReceiveStart();
	}
	return USB_SUCCESS;
}

#endif /* USB_VCOM_SYNC */

#ifdef USB_CDC_LINE_CODING_SUPPORTED

/* USB_CDC_HANDLE_GET_LINE_CODING implementation example */
USB_Result USB_CDC_GetLineCoding ( uint16_t wINDEX,
		USB_CDC_LineCoding_TypeDef* DATA )
{
	assert_param(DATA);
	if (wINDEX != 0) {
		/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just store received settings */
	*DATA = LineCoding;
	return USB_SUCCESS;
}

/* USB_CDC_HANDLE_SET_LINE_CODING implementation example */
USB_Result USB_CDC_SetLineCoding ( uint16_t wINDEX,
		const USB_CDC_LineCoding_TypeDef* DATA )
{
	assert_param(DATA);
	if (wINDEX != 0) {
		/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just send back settings stored earlier */
	LineCoding = *DATA;
	return USB_SUCCESS;
}

#endif /* USB_CDC_LINE_CODING_SUPPORTED */

#ifdef USB_DEBUG_PROTO

/* Overwritten USB_DEVICE_HANDLE_SETUP default handler - to dump received setup packets */
USB_Result USB_DeviceSetupPacket_Debug ( USB_EP_TypeDef EPx,
		const USB_SetupPacket_TypeDef* USB_SetupPacket )
{

#ifdef USB_DEBUG_PROTO
	SetupPackets[SPIndex].packet = *USB_SetupPacket;
	SetupPackets[SPIndex].address = USB_GetSA();
	SPIndex = (SPIndex < USB_DEBUG_NUM_PACKETS ? SPIndex + 1 : 0);
#endif /* USB_DEBUG_PROTO */

	return USB_DeviceSetupPacket(EPx, USB_SetupPacket);
}

#endif /* USB_DEBUG_PROTO */


/*if (Length == 11) // std id
{
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = Buffer[2] % 9;
	TxMsg.PRIOR_0 = DISABLE;
	TxMsg.ID = CAN_STDID_TO_EXTID((*(uint16_t*)Buffer) % (0x7FF + 1)); // 11 bit maximum

	for (int i = 0; i < 8; i++)
		can.d[i] = Buffer[i + 3];
	TxMsg.Data[0] = can.data[0];
	TxMsg.Data[1] = can.data[1];
}

if (Length == 13) // ext id
{
	TxMsg.IDE = CAN_ID_EXT;
	TxMsg.DLC = Buffer[4] % 9;
	TxMsg.PRIOR_0 = DISABLE;
	TxMsg.ID = (*(uint32_t*)Buffer) % (0x1FFFFFFF + 1); // 29 bit maximum

	for (int i=0; i<8; i++)
		can.d[i]=Buffer[i+5];
	TxMsg.Data[0] = can.data[0];
	TxMsg.Data[1] = can.data[1];
}
CAN_Transmit(MDR_CAN2, free_tx_buf(MDR_CAN2), &TxMsg);*/

//void ReverseBytes(uint8_t* d)
//{
//	uint8_t b;
//	b = d[0]; d[0] = d[3]; d[3] = b;
//	b = d[1]; d[1] = d[2]; d[2] = b;
//}
//		TxMsg.Data[0] = *(uint32_t*)(Buffer+7);
//		TxMsg.Data[1] = *(uint32_t*)(Buffer+3);
//		TxMsg.Data[0] = (u32)((u8)(TxMsg.Data[0])<<24) + (u32)((u8)(TxMsg.Data[0]>>8)<<16) + \
//				  (u32)((u8)(TxMsg.Data[0]>>16)<<8) + (u32)(u8)(TxMsg.Data[0]>>24);
//
//		TxMsg.Data[1] = (u32)((u8)(TxMsg.Data[1])<<24) + (u32)((u8)(TxMsg.Data[1]>>8)<<16) + \
//						  (u32)((u8)(TxMsg.Data[1]>>16)<<8) + (u32)(u8)(TxMsg.Data[1]>>24);
//		if (TxMsg.DLC == 1) TxMsg.Data[0] = Buffer[10];
//		if (TxMsg.DLC == 2) TxMsg.Data[0] = Buffer[9] + (Buffer[10] << 8);
//		if (TxMsg.DLC == 3) TxMsg.Data[0] = Buffer[8] + (Buffer[9] << 8) + (Buffer[10] << 16);
//		if (TxMsg.DLC == 4) TxMsg.Data[0] = Buffer[7] + (Buffer[8] << 8) + (Buffer[9] << 16) + (Buffer[10] << 24);
//
//		if (TxMsg.DLC == 5) {TxMsg.Data[1] = Buffer[10]; TxMsg.Data[0] = Buffer[6] + (Buffer[7] << 8) + (Buffer[8] << 16) + (Buffer[9] << 24);}
//		if (TxMsg.DLC == 6) {TxMsg.Data[1] = Buffer[9] + (Buffer[10] << 8); TxMsg.Data[0] = Buffer[5] + (Buffer[6] << 8) + (Buffer[7] << 16) + (Buffer[8] << 24);}
//		if (TxMsg.DLC == 7) {TxMsg.Data[1] = Buffer[8] + (Buffer[9] << 8) + (Buffer[10] << 16); TxMsg.Data[0] = Buffer[4] + (Buffer[5] << 8) + (Buffer[6] << 16) + (Buffer[7] << 24);}
//		if (TxMsg.DLC == 8) {TxMsg.Data[1] = Buffer[7] + (Buffer[8] << 8) + (Buffer[9] << 16) + (Buffer[10] << 24); TxMsg.Data[0] = Buffer[3] + (Buffer[4] << 8) + (Buffer[5] << 16) + (Buffer[6] << 24);}


		//uint32_t Data0, Data1;
//		uint8_t can[11];
//		for (int i = 0; i < 11; i++)
//			can[i] = Buffer[i];

//		TxMsg.Data[1] = (Buffer[3] << 24)+(Buffer[4] << 16)+(Buffer[5] << 8)+Buffer[6];
//		TxMsg.Data[0] = (Buffer[7] << 24)+(Buffer[8] << 16)+(Buffer[9] << 8)+Buffer[10];

//		if (TxMsg.DLC == 1) TxMsg.Data[0] = Buffer[7];
//		if (TxMsg.DLC == 2) TxMsg.Data[0] = Buffer[8] + (Buffer[7] << 8);
//		if (TxMsg.DLC == 3) TxMsg.Data[0] = Buffer[9] + (Buffer[8] << 8) + (Buffer[7] << 16);
//		if (TxMsg.DLC >= 4) TxMsg.Data[0] = Buffer[10] + (Buffer[9] << 8) + (Buffer[8] << 16) + (Buffer[7] << 24);
//
//		if (TxMsg.DLC == 5) TxMsg.Data[1] = Buffer[3];
//		if (TxMsg.DLC == 6) TxMsg.Data[1] = Buffer[3] + (Buffer[4] << 8);
//		if (TxMsg.DLC == 7) TxMsg.Data[1] = Buffer[3] + (Buffer[4] << 8) + (Buffer[5] << 16);
//		if (TxMsg.DLC == 8) TxMsg.Data[1] = Buffer[3] + (Buffer[4] << 8) + (Buffer[5] << 16) + (Buffer[6] << 24);

//		TxMsg.Data[0] = *(uint32_t*)(Buffer+7);
//		TxMsg.Data[1] = *(uint32_t*)(Buffer+3);
