
/*
 Файл:			signals.h
 Описание:		Обработка сигналов
 Версия: 		1
 Организация:	OАO НПО "НАУКА"

 История:
 Версия 1, 01.02.2019:
	- Начальная версия
 */

#ifndef __signals_h
#define __signals_h

#include "mcu_types.h"

/*
 Пересчет из с в мс.

 Аргументы:
	s			время, с

 Результат:
	время, мс
*/
#define S2MS(s)					((u16)((s) * 1000))

/* Количество линий адреса */
#define C1						5

/* Адреса выводов порта Е к которым прикреплены линии адреса */
#define C2						{0x0002U, 0x0004U, 0x0008U, 0x0040U, 0x0080U}

/* В многочлене CRC соотвтетствует неподключенному выводу */
#define C3						3

/* В многочлене CRC соотвтетствует выводам, соединенным 1-ой перемычкой */
#define C4						5

/* В многочлене CRC соотвтетствует выводам, соединенным 2-ой перемычкой */
#define C5						7

/* В многочлене CRC соотвтетствует выводу, подключенному к земле */
#define C6						11

/* Общее количество приводов */
#define C7						54

/* Известные значения CRC */
#define C8						{ 363, 3789, 3343, 3261, 3247, 1005, 923, 909, 477, 463, 381, 1049, 1243,\
								1265, 3387, 3581, 3603, 3833, 5607, 5585, 4109, 5667, 5883, 4145,\
								5703, 5897, 14923, 17903, 17821, 17807, 15037, 15023, 14941, 17947,\
								18141, 18163, 1667, 5093, 4565, 4551, 2227, 2213, 1685, 5137, 6889,\
								6911, 16227, 19125, 19111, 16245, 483, 3909, 3463, 1125 }

/* Общее количество приводов */
#define C9						(u8)(0xFF)

/* Число ПИ */
#define C93						3.1415926



/* Описание входных сигналов СПО. */
/* LR-D.7.1 */
typedef struct s_signals_in
{
	boolean	X1;					// Линия адреса задвижки 1
	boolean	X2;					// Линия адреса задвижки 2
	boolean	X3;					// Линия адреса задвижки 3
	boolean	X6;					// Линия адреса задвижки 4
	boolean	X7;					// Линия адреса задвижки 5

	boolean	LC_FAULT;			// ошибка приема данных от LC
	boolean	LC_API_FAULT;		// ошибка вызова функции АПО LC
	boolean	API_FAULT;			// ошибка вызова функции АПО
	boolean	QNH_ERROR[2];		// ошибка формата QNH (X414)
	boolean	QFE_ERROR[2];		// ошибка формата QFE (X418)
} t_signals_in;

/* Описание выходных сигналов СПО. */
/* LR-D.167.1 */
typedef struct s_signals_out
{
	u8		Y1;					// Адрес задвижки

} t_signals_out;

/* Описание внутренних состояний СПО. */
/* LR-D.251.1 */
typedef struct s_signals_state
{
	boolean	Z1;					// Блокировка управления

} t_signals_state;

/* Описание таймеров СПО. */
/* LR-D.369.1 */
typedef struct s_signals_timer
{
	u16		T1;					// Выбор состояния контроллера
	u16		T2;					// Подтверждение выбора состояния
	u32		T3;					// Работа контроллера
} t_signals_timer;

void signals_init(void);

void signals_z2_save(void);

void signals_apo_in(void);

void signals_apo_out(void);

void signals_fault_write(void);

void signals_pack(void);

void bios_api_fault(void);

extern t_signals_in si;

extern t_signals_out so;

extern t_signals_state ss;

extern t_signals_timer st;

extern u8 sp[];

#endif
