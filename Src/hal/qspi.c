#include <stdint.h>
#include "stm32h7_regs.h"
#include "qspi.h"


void quadspi_busy_wait(void *base)
{
	while (QUADSPI_SR & QUADSPI_SR_BUSY);
}

void quadspi_wait_flag(void *base, uint32_t flag)
{
	while (!(QUADSPI_SR & flag));
	QUADSPI_FCR = flag;
}

void quadspi_write_enable(void *base)
{
	quadspi_busy_wait(base);

	QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		WRITE_ENABLE_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_TCF);

	quadspi_busy_wait(base);

	QUADSPI_PSMAR = N25Q512A_SR_WREN;
	QUADSPI_PSMKR = N25Q512A_SR_WREN;
	QUADSPI_PIR = 0x10;

	QUADSPI_CR |= QUADSPI_CR_AMPS;
	QUADSPI_DLR = 0;
	QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_SMF);
}

void quadspi_memory_ready(void *base)
{
	quadspi_busy_wait(base);

	QUADSPI_PSMAR = 0;
	QUADSPI_PSMKR = N25Q512A_SR_WIP;
	QUADSPI_PIR = 0x10;

	QUADSPI_CR |= QUADSPI_CR_AMPS;
	QUADSPI_DLR = 0;
	QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_SMF);
}

void quadspi_erase_sector(uint32_t sector)
{
//    quadspi_busy_wait((void*)QUADSPI_BASE);
//    *QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR;
        quadspi_busy_wait((void*)QUADSPI_BASE);

        quadspi_write_enable((void*)QUADSPI_BASE);

        quadspi_busy_wait((void*)QUADSPI_BASE);

	QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR |
                QUADSPI_CCR_ADSIZE_24BITS |
		QUADSPI_CCR_ADMOD_1_LINE | 
                QUADSPI_CCR_IDMOD_1_LINE | 0x20;
        QUADSPI_AR = sector;

        quadspi_wait_flag((void*)QUADSPI_BASE, QUADSPI_SR_TCF);

        quadspi_memory_ready((void*)QUADSPI_BASE);
}

void quadspi_write(uint32_t address,uint8_t *data,int len)
{
  int txCount;
  volatile uint32_t *data_reg = &QUADSPI_DR;
  while(len > 0){
    quadspi_write_enable((void*)QUADSPI_BASE);

    quadspi_busy_wait((void*)QUADSPI_BASE);

    QUADSPI_DLR = 255;
    QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | 
		QUADSPI_CCR_DCYC(0) | QUADSPI_CCR_ADSIZE_24BITS | QUADSPI_CCR_DMODE_4_LINES |
		QUADSPI_CCR_ADMOD_1_LINE | QUADSPI_CCR_IDMOD_1_LINE | 0x32;
    QUADSPI_AR = address;

    txCount = 256;
    while(txCount-- > 0){
      quadspi_wait_flag((void*)QUADSPI_BASE, QUADSPI_SR_FTF);
       *(volatile uint8_t *)data_reg = *data ++;
    }

    quadspi_wait_flag((void*)QUADSPI_BASE, QUADSPI_SR_TCF);

    len -= 256;
    address += 256;

    quadspi_memory_ready(0);
  }
}

void quadspi_reset_memory(void *base)
{
	/* Reset memory */
	quadspi_busy_wait(base);

	QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_ENABLE_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_TCF);

	QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		RESET_MEMORY_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_TCF);
}

void quadspi_write_sr2()
{
    quadspi_write_enable(0);
    QUADSPI_DLR = 1;
    QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
		0x31;

    while (!(QUADSPI_SR & QUADSPI_SR_FTF));

    QUADSPI_DR = 0x02;

    quadspi_wait_flag(0, QUADSPI_SR_TCF);
}

void quadspi_mmap(void)
{
	QUADSPI_CCR = QUADSPI_CCR_FMODE_MEMMAP | QUADSPI_CCR_DMODE_4_LINES |
		QUADSPI_CCR_DCYC(8) | QUADSPI_CCR_ADSIZE_24BITS |
		QUADSPI_CCR_ADMOD_1_LINE | QUADSPI_CCR_IDMOD_1_LINE |
		QUAD_OUTPUT_FAST_READ_CMD;

	quadspi_busy_wait(0);
}

void quadspi_init(struct qspi_params *params, void *base)
{
	uint32_t reg;

	QUADSPI_CR = QUADSPI_CR_FTHRES(0);

	quadspi_busy_wait(base);

    QUADSPI_CR |= QUADSPI_CR_PRESCALER(1) | QUADSPI_CR_SSHIFT;
    QUADSPI_DCR = QUADSPI_DCR_FSIZE_16MB | QUADSPI_DCR_CSHT(1);

    QUADSPI_CR |= QUADSPI_CR_EN;

        quadspi_reset_memory(base);
#if 1

	quadspi_busy_wait(base);

	QUADSPI_PSMAR = 0;
	QUADSPI_PSMKR = N25Q512A_SR_WIP;
	QUADSPI_PIR = 0x10;

	QUADSPI_CR |= QUADSPI_CR_AMPS;
	QUADSPI_DLR = 0;
	QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;

	quadspi_wait_flag(base, QUADSPI_SR_SMF);

#endif
//        quadspi_write_sr2();
#if 0
//        quadspi_erase_sector(base,0);

	/* Enter 3-bytes address mode */
//	quadspi_write_enable(base);
//
//	quadspi_busy_wait(base);
//
//	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_IDMOD_1_LINE |
//		ENTER_4_BYTE_ADDR_MODE_CMD;
//
//	quadspi_wait_flag(base, QUADSPI_SR_TCF);
//
//	quadspi_busy_wait(base);

//	*QUADSPI_PSMAR = 0;
//	*QUADSPI_PSMKR = N25Q512A_SR_WIP;
//	*QUADSPI_PIR = 0x10;
//
//	*QUADSPI_CR |= QUADSPI_CR_AMPS;
//	*QUADSPI_DLR = 0;
//	*QUADSPI_CCR = QUADSPI_CCR_FMODE_AUTO_POLL | QUADSPI_CCR_DMODE_1_LINE |
//		QUADSPI_CCR_IDMOD_1_LINE | READ_STATUS_REG_CMD;
//
//	quadspi_wait_flag(base, QUADSPI_SR_SMF);

	/* Configure dummy cycles on memory side */

//	quadspi_busy_wait(base);
//
//	*QUADSPI_DLR = 0;
//	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_DMODE_1_LINE |
//		QUADSPI_CCR_IDMOD_1_LINE | READ_VOL_CFG_REG_CMD;
//
//	*QUADSPI_CCR |= QUADSPI_CCR_FMODE_IND_RD;
//
//	*QUADSPI_AR = *QUADSPI_AR; //Needed?
//
//	while (!(*QUADSPI_SR & (QUADSPI_SR_FTF | QUADSPI_SR_TCF)));
//
//	reg = *QUADSPI_DR;
//
//	quadspi_wait_flag(base, QUADSPI_SR_TCF);
//
//	reg = (reg & ~0xf0) | (params->dummy_cycle << 4);
//
//	quadspi_write_enable(base);
//
//	quadspi_busy_wait(base);
//
//	*QUADSPI_DLR = 0;
//	*QUADSPI_CCR = QUADSPI_CCR_FMODE_IND_WR | QUADSPI_CCR_DMODE_1_LINE |
//		QUADSPI_CCR_IDMOD_1_LINE | WRITE_VOL_CFG_REG_CMD;
//
//	while (!(*QUADSPI_SR & QUADSPI_SR_FTF));
//
//	*QUADSPI_DR = reg;
//
//	quadspi_wait_flag(base, QUADSPI_SR_TCF);
//
#endif
	quadspi_busy_wait(base);

	//quadspi_mmap();

}

