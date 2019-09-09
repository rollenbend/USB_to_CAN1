/*
 * config.c
 *
 *  Created on: 28 янв. 2019 г.
 *      Author: DA.Tsekh
 */


#include "config.h"

/*******************************************************************************
* Function Name  : ClockConfigure
* Description    : Configures the CPU_PLL and RTCHSE clock.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void clock_configure(uint8_t Freq_in)
{
	 RST_CLK_DeInit();
#ifdef USE_HSI
	RST_CLK_HSIcmd(ENABLE);
	RST_CLK_HSIclkPrescaler(RST_CLK_HSIclkDIV1);
	RST_CLK_HSIadjust( 22 );
	/* Configures the CPU_PLL clock source */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSIdiv1, RST_CLK_CPU_PLLmul16);
#elif USE_HSE
	  /* Enable HSE (High Speed External) clock */
	  RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	  if (RST_CLK_HSEstatus() == ERROR) {
	    while (1);
	  }

	/* Configures the CPU_PLL clock source */
	if (Freq_in == HSE_8Mhz)
	{
		RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10); // 80 mhz (HSE=8 MHz, 8*10 = 80MHz - max.)
	}

	if (Freq_in == HSE_16Mhz)
	{
		RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv2, RST_CLK_CPU_PLLmul10); // 80 mhz (HSE=16 MHz, 16/2*10 = 80MHz - max.)
	}
#endif


	/* Enables the CPU_PLL */
	RST_CLK_CPU_PLLcmd(ENABLE);
	if (RST_CLK_CPU_PLLstatus() == ERROR) {
		while (1);
	}

	/* Enables the RST_CLK_PCLK_EEPROM */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
	/* Sets the code latency value */
	EEPROM_SetLatency(EEPROM_Latency_3);

	/* Select the CPU_PLL output as input for CPU_C3_SEL */
	RST_CLK_CPU_PLLuse(ENABLE);
	/* Set CPUClk Prescaler */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

	/* Select the CPU clock source */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

	/* Enables the RTCHSE clock on all ports */
	RST_CLK_PCLKcmd(ALL_PORTS_CLK, ENABLE);

	MDR_RST_CLK->PER_CLOCK |= 0x6BFFFDFF; // разрешение тактирования всей периферии

	POWER_DUccMode( POWER_DUcc_over_80MHz);
	SystemCoreClockUpdate();
}

void LEDs_ini(void)
{
	PORT_InitTypeDef PORT_InitStructure;

	/* Enables the RTCHSE clock on PORTC */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

	/* Configure PORTf pin 2 for input */
	PORT_InitStructure.PORT_Pin = PORT_Pin_0 | PORT_Pin_1 | PORT_Pin_2 | PORT_Pin_3;
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_Init(MDR_PORTF, &PORT_InitStructure);
}
