
/*
 ����:			signals.h
 ��������:		��������� ��������
 ������: 		1
 �����������:	O�O ��� "�����"

 �������:
 ������ 1, 01.02.2019:
	- ��������� ������
 */

#ifndef __signals_h
#define __signals_h

#include "mcu_types.h"

/*
 �������� �� � � ��.

 ���������:
	s			�����, �

 ���������:
	�����, ��
*/
#define S2MS(s)					((u16)((s) * 1000))

/* ���������� ����� ������ */
#define C1						5

/* ������ ������� ����� � � ������� ����������� ����� ������ */
#define C2						{0x0002U, 0x0004U, 0x0008U, 0x0040U, 0x0080U}

/* � ���������� CRC �������������� ��������������� ������ */
#define C3						3

/* � ���������� CRC �������������� �������, ����������� 1-�� ���������� */
#define C4						5

/* � ���������� CRC �������������� �������, ����������� 2-�� ���������� */
#define C5						7

/* � ���������� CRC �������������� ������, ������������� � ����� */
#define C6						11

/* ����� ���������� �������� */
#define C7						54

/* ��������� �������� CRC */
#define C8						{ 363, 3789, 3343, 3261, 3247, 1005, 923, 909, 477, 463, 381, 1049, 1243,\
								1265, 3387, 3581, 3603, 3833, 5607, 5585, 4109, 5667, 5883, 4145,\
								5703, 5897, 14923, 17903, 17821, 17807, 15037, 15023, 14941, 17947,\
								18141, 18163, 1667, 5093, 4565, 4551, 2227, 2213, 1685, 5137, 6889,\
								6911, 16227, 19125, 19111, 16245, 483, 3909, 3463, 1125 }

/* ����� ���������� �������� */
#define C9						(u8)(0xFF)

/* ����� �� */
#define C93						3.1415926



/* �������� ������� �������� ���. */
/* LR-D.7.1 */
typedef struct s_signals_in
{
	boolean	X1;					// ����� ������ �������� 1
	boolean	X2;					// ����� ������ �������� 2
	boolean	X3;					// ����� ������ �������� 3
	boolean	X6;					// ����� ������ �������� 4
	boolean	X7;					// ����� ������ �������� 5

	boolean	LC_FAULT;			// ������ ������ ������ �� LC
	boolean	LC_API_FAULT;		// ������ ������ ������� ��� LC
	boolean	API_FAULT;			// ������ ������ ������� ���
	boolean	QNH_ERROR[2];		// ������ ������� QNH (X414)
	boolean	QFE_ERROR[2];		// ������ ������� QFE (X418)
} t_signals_in;

/* �������� �������� �������� ���. */
/* LR-D.167.1 */
typedef struct s_signals_out
{
	u8		Y1;					// ����� ��������

} t_signals_out;

/* �������� ���������� ��������� ���. */
/* LR-D.251.1 */
typedef struct s_signals_state
{
	boolean	Z1;					// ���������� ����������

} t_signals_state;

/* �������� �������� ���. */
/* LR-D.369.1 */
typedef struct s_signals_timer
{
	u16		T1;					// ����� ��������� �����������
	u16		T2;					// ������������� ������ ���������
	u32		T3;					// ������ �����������
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
