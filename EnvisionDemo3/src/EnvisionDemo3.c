#include <stdint.h>
#include <stdbool.h>
#include <iodefine.h>
#include <stdlib.h>
#include "lcd_driver.h"

void system_clock_config(void);
void push_button_ir_config(void);
char *itoa(int value, char *str, int base);
void delay_ms(uint16_t ms);
void delay_us(uint32_t us);

int main(void)
{
	char num_buf[10];
	int i = 0;

	system_clock_config();
	push_button_ir_config();		// initialize push button and an interrupt when it's pushed
	lcd_init_1_2();					// initialize both lcd buffers

	// clear gr1 and gr2
	lcd_filled_rectangle_1(0, 0, 480, 272, false);
	lcd_filled_rectangle_2(0, 0, 480, 272, RED);

    while (true)
    {
    	i++;
    	if (i > 250)
    	{
    		i = 0;
    	}

    	// plot and scroll text on gr1
    	lcd_string_1(0, 261, "Test text");
    	itoa(i, num_buf, 10);
    	lcd_string_1(60, 261, num_buf);
    	lcd_scroll_display_up_1(10);
    	delay_ms(500);

    	// simple moving graphic on gr2
    	lcd_filled_rectangle_2(0, 0, 480, 272, RED);
    	lcd_filled_rectangle_2(i, i, 20, 20, BLUE);
    	delay_ms(500);
    }

    return 0;
}

/**
 * Configure IRQ13 on push button
 */
void push_button_ir_config(void)
{
    /* enable writing to MPC pin function control registers */
	SYSTEM.PRCR.WORD = 0xA50BU;
    MPC.PWPR.BIT.B0WI = 0U;
    MPC.PWPR.BIT.PFSWE = 1U;

    /* set P05 to IRQ13 */
    MPC.P05PFS.BYTE = 0x40U;

    /* disable writing to MPC pin function control registers */
    MPC.PWPR.BIT.PFSWE = 0U;
    MPC.PWPR.BIT.B0WI = 1U;

    /* disable IRQ13 interrupt */
	ICU.IER[9].BIT.IEN5 = 0U;

	/* set up push button gpio input */
	PORT0.PMR.BIT.B5 = 0U;	/* mode to gpio */
	PORT0.PDR.BIT.B5 = 0U;	/* input */
	PORT0.PCR.BIT.B5 = 0U;  /* pull-up disable */

    /* disable IRQ13 digital filter */
    ICU.IRQFLTE1.BIT.FLTEN13 = 0;

    /* trigger interrupt on both edges */
	ICU.IRQCR[13].BIT.IRQMD = 3U;

	/* clear any outstanding IRQ13 interrupt */
    IR(ICU, IRQ13) = 0U;

    /* set IRQ13 priority level */
    IPR(ICU, IRQ13) = 2U;

    /* enable IRQ13 interrupt */
    IEN(ICU,IRQ13) = 1U;

    /* enable IRQ13 interrupt */
	ICU.IER[9].BIT.IEN5 = 1U;
}

/**
 * Configure the system and peripheral clocks
 */
void system_clock_config(void)
{
	/* enable all protect register bits */
	SYSTEM.PRCR.WORD = 0xA50B;

	/* forcibly oscillate the main clock */
	SYSTEM.MOFCR.BIT.MOFXIN = 0U;

	/* select resonator as main clock source */
	SYSTEM.MOFCR.BIT.MOSEL = 0;

	/* stop the HOCO, not using it */
	SYSTEM.HOCOCR.BIT.HCSTP = 1U;

	/* disable the sub-clock as no crystal input on hw */
	RTC.RCR3.BIT.RTCEN = 0;

	/* not using USB clock */
	SYSTEM.SCKCR2.WORD = 0x0001U;

	/* set drive level for 12MHz input */
	SYSTEM.MOFCR.BIT.MODRV2 = 2U;

	/* set wait time for starting main clock oscillator */
	SYSTEM.MOSCWTCR.BYTE = 0x53;

	/* start main clock oscillator */
	SYSTEM.MOSCCR.BIT.MOSTP = 0U;

	/* set rom wait state appropriate to main clock speed */
	SYSTEM.ROMWT.BIT.ROMWT = 2U;

	/* set PLL division to 1 */
	SYSTEM.PLLCR.BIT.PLIDIV = 0U;

	/* set PLL multiplication to 20 */
	SYSTEM.PLLCR.BIT.STC = 39U;

	/* start the PLL */
	SYSTEM.PLLCR2.BIT.PLLEN = 0U;

	/* wait for clock stabilization */
	while (0 == SYSTEM.OSCOVFSR.BIT.PLOVF)
	{
		__asm("NOP");
	}

	/* ICLK to 120MHz */
	SYSTEM.SCKCR.BIT.ICK = 1U;

	/* FCLK to 60 MHz */
	SYSTEM.SCKCR.BIT.FCK = 2U;

	/* PCLKA to 120MHz */
	SYSTEM.SCKCR.BIT.PCKA = 1U;

	/* PCLKB to 60MHz */
	SYSTEM.SCKCR.BIT.PCKB = 2U;

	/* PCLKC to 60MHz */
	SYSTEM.SCKCR.BIT.PCKC = 2U;

	/* PCLKD to 60MHz */
	SYSTEM.SCKCR.BIT.PCKD = 2U;

	/* SDCLK not used */
	SYSTEM.SCKCR.BIT.PSTOP0 = 1U;

	/* BCLK not used */
	SYSTEM.SCKCR.BIT.PSTOP1 = 1U;

	/* switch main clock source to PLL */
	SYSTEM.SCKCR3.BIT.CKSEL = 4U;

	/* stop LOCO as finished with it now */
	SYSTEM.LOCOCR.BYTE = 1U;

	/* disable all protect register bits */
	SYSTEM.PRCR.WORD = 0xA500;
}

void delay_ms(uint16_t ms)
{
	volatile uint16_t i;

	for (i = 0U; i < ms; i++)
	{
		delay_us(950U);
	}
}

void delay_us(uint32_t us)
{
	volatile uint32_t i;

	for (i = 0U; i < 13327U; i++)
	{
	}
}
