#include "stm32h7_regs.h"
#include "gpio.h"
#include "qspi.h"

#define RCC_CR  (*(volatile unsigned long *)(RCC_BASE_REG))
#define RCC_CFGR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x10))
#define RCC_D1CFGR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x18))
#define RCC_D2CFGR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x1c))
#define RCC_D3CFGR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x20))
#define RCC_PLLCKSELR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x28))
#define RCC_PLLCFGR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x2c))
#define RCC_PLL1DIVR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x30))
#define RCC_PLL1FRACR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x34))
#define RCC_PLL2DIVR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x38))
#define RCC_PLL2FRACR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x3c))
#define RCC_PLL3DIVR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x40))
#define RCC_PLL3FRACR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x44))
#define RCC_D1CCIPR  (*(volatile unsigned long *)(RCC_BASE_REG + 0x4c))
#define RCC_D2CCIP1R  (*(volatile unsigned long *)(RCC_BASE_REG + 0x50))
#define RCC_D2CCIP2R  (*(volatile unsigned long *)(RCC_BASE_REG + 0x54))
#define RCC_D1AHB1ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xd4))
#define RCC_D2AHB1ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xd8))
#define RCC_D2AHB2ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xdc))
#define RCC_D3AHB1ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xe0))
#define RCC_D1APB1ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xe4))
#define RCC_D2APB1LENR (*(volatile unsigned long *)(RCC_BASE_REG + 0xe8))
#define RCC_D2APB1HENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xec))
#define RCC_D2APB2ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xf0))
#define RCC_D3APB1ENR  (*(volatile unsigned long *)(RCC_BASE_REG + 0xf4))
#define RCC_AHB3RST  (*(volatile unsigned long *)(RCC_BASE_REG + 0x7c))
#define FLASH_FACR  (*(volatile unsigned long *)(FLASH_BASE))
#define PWR_D3CR  (*(volatile unsigned long *)(PWR_BASE + 0x18))
#define PWR_CR3  (*(volatile unsigned long *)(PWR_BASE + 0xc))

void clock_setup(void)
{

	uint32_t divm, divn, divp, divq, divr;

	/*  enable HSI */
	RCC_CR |= RCC_CR_HSION;

	/* Reset CFGR register */
	/* HSI by default as system clock*/
	RCC_CFGR = 0;

	/*  reset registers ... */
	RCC_D1CFGR = 0;
	RCC_D2CFGR = 0;
	RCC_D3CFGR = 0;
	RCC_PLLCKSELR = 0;
	RCC_PLLCFGR = 0;
	RCC_PLL1DIVR = 0;
	RCC_PLL1FRACR = 0;
	RCC_PLL2DIVR = 0;
	RCC_PLL2FRACR = 0;
	RCC_PLL3DIVR = 0;
	RCC_PLL3FRACR = 0;

	/* Activate all clock */
	RCC_D3AHB1ENR = 0x7FF;
	RCC_D2APB2ENR = 0;
	RCC_D1AHB1ENR = (1<<14);
	RCC_D2AHB1ENR = 0;
	RCC_D2AHB2ENR = 0;
	RCC_D1APB1ENR = 0;
	RCC_D2APB1LENR = 0;
	RCC_D2APB1HENR = 0;
	RCC_D3APB1ENR = 0;
	RCC_D1CCIPR = 0x00000000;
	RCC_D2CCIP1R = 0x00000000;
	RCC_D2CCIP2R = 0x00000000;

	/* Second level */
	PWR_D3CR |= 0xc000;
	PWR_CR3 &= ~(0x4);
	while (!(PWR_D3CR & (1 <<13))) {
	}
	/* disable HSE to configure it  */
//	RCC_CR &= ~(RCC_CR_HSEON);
//	while ((RCC_CR & RCC_CR_HSERDY)) {
//	}
	/* clear HSE bypass and set it ON */
//	RCC_CR &= (~RCC_CR_HSEBYP);
//	RCC_CR |=RCC_CR_HSEON;
//	while (!(RCC_CR & RCC_CR_HSERDY)) {
//	}

	/* setup pll */
	/*  disable pll1 */
	RCC_CR &= ~(RCC_CR_PLL1ON);
	while ((RCC_CR & RCC_CR_PLL1RDY)) {
	}
	/* Configure PLL1 as clock source:
	 * OSC_HSE = 64 MHz
	 * VCO = 640MHz
	 * pll1_p = 320MHz / pll1_q = 320MHz*/
	divm = 8;
	divn = 80;
	divp = 2;
	divq = 2;
	divr = 2;

	/*  PLL SRC = HSE */
	RCC_PLLCKSELR |= (divm << 4 ) | 0x0;
	RCC_PLL1DIVR  |= ((divr - 1) << (24)) | ((divq - 1) << (16)) | ( (divp - 1) << 9) | (divn - 1);

	/*  Enable divP1, divQ1, divR1, pll1fracn */
	RCC_PLLCFGR |= (2 << 2);
	RCC_PLLCFGR |= (1 << 18) | (1 << 17) | (1 << 16);

	/*  enable the main PLL */
	RCC_CR |= (1 << 24 );
	while (!(RCC_CR & RCC_CR_PLL1RDY)) {
	}

	/*  set flash latency */
	FLASH_FACR &=0xfffffff0;
	FLASH_FACR |= 0xa;

	/* set HPRE (/2) DI clk --> 160MHz */
	RCC_D1CFGR |= (4<<4)|8;
        RCC_D2CFGR |= (4<<4)|(4<<8);
        RCC_D3CFGR |= (4<<4);

	/*  select PLL1 as clcok source */
	RCC_CFGR |= 0x3;
	while ((((RCC_CFGR)&(0x3<<3)) != (0x3<<3))) {
	}
	/*  test for sdram: use pll1_q as fmc_k clk */
//	RCC_D1CCIPR = 1 | (3 << 4);

	/* togle reset QSPI */
//	RCC_AHB3RST |= (1 << 14);
//	RCC_AHB3RST &= ~(1 << 14);

}

void qspi_init(void)
{


  gpio_set_qspi(GPIOA_BASE,'B',2,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'B',6,GPIOx_PUPDR_NOPULL, 0xA);
  gpio_set_qspi(GPIOA_BASE,'D',11,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'D',12,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'D',13,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'E',2,GPIOx_PUPDR_NOPULL, 0x9);

  quadspi_init(0, (void *)QUADSPI_BASE);
}
