/*
 * Файл:		Timers.h
 * Описание:	
 * Версия:		1
 * Организация:	ПАО НПО "НАУКА"
 * 
 * История:
 * Версия 1, 14.08.2019:
 * 		- Начальная версия
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#include "config.h"

void init_Timer1(void);
void Make_mess(MDR_CAN_TypeDef* CANx, CAN_RxMsgTypeDef* RxMessage, USB_TO_CAN_FRAME* frame);

#endif /* INC_TIMERS_H_ */
