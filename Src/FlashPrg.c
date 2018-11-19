/*********************************************************************
*               (c) SEGGER Microcontroller GmbH & Co. KG             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode template
--------  END-OF-HEADER  ---------------------------------------------
*/
#include "FlashOS.H"
#include "stm32h7_regs.h"
#include "qspi.h"
#include "gpio.h"

void clock_setup(void);
void qspi_init(void);

/*********************************************************************
*
*       Defines (configurable)
*
**********************************************************************
*/
#define PAGE_SIZE_SHIFT          (3)      // The smallest program unit (one page) is 8 byte in size
//
// Some flash types require a native verify function as the memory is not memory mapped available (e.g. eMMC flashes).
// If the verify function is implemented in the algorithm, it will be used by the J-Link DLL during compare / verify
// independent of what verify type is configured in the J-Link DLL.
// Please note, that SEGGER does not recommend to use this function if the flash can be memory mapped read
// as this may can slow-down the compare / verify step.
//
#define SUPPORT_NATIVE_VERIFY    (0)
#define SUPPORT_NATIVE_READ_BACK (0)
#define SUPPORT_BLANK_CHECK      (0)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// We use this dummy variable to make sure that the PrgData
// section is present in the output elf-file as this section
// is mandatory in current versions of the J-Link DLL 
//
static volatile int _Dummy;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _FeedWatchdog
*
*  Function description
*    Feeds the watchdog. Needs to be called during RAMCode execution
*    in case of an watchdog is active.
*/
static void _FeedWatchdog(void) {
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int Init(U32 Addr, U32 Freq, U32 Func) {
  (void)Addr;
  (void)Freq;
  (void)Func;
  //
  // Init code
  //
  clock_setup();

  gpio_set_qspi(GPIOA_BASE,'B',2,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'B',6,GPIOx_PUPDR_NOPULL, 0xA);
  gpio_set_qspi(GPIOA_BASE,'D',11,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'D',12,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'D',13,GPIOx_PUPDR_NOPULL, 0x9);
  gpio_set_qspi(GPIOA_BASE,'E',2,GPIOx_PUPDR_NOPULL, 0x9);

  quadspi_init(0, (void *)QUADSPI_BASE);

  if(Func != 1 )
    quadspi_mmap();

  return 0;
}

/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*
*  Parameters
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int UnInit(U32 Func) {
  (void)Func;
  //
  // Uninit code
  //
  quadspi_init(0, (void *)QUADSPI_BASE);

  quadspi_mmap();

  return 0;
}

/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    Addr: Address of the sector to be erased
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int EraseSector(U32 SectorAddr) {

  SectorAddr -= 0x90000000;
  quadspi_erase_sector(SectorAddr);
  //_FeedWatchdog();
  return 0;
}

/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr: Destination address
*    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
*    pSrcBuff: Point to the source buffer
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {
  

  DestAddr -= 0x90000000;
  quadspi_write(DestAddr,pSrcBuff,NumBytes);
  return 0;
}

/*********************************************************************
*
*       Verify
*
*  Function description
*    Compares a specified number of bytes of a provided data
*    buffer with the content of the device
*
*  Parameters
*    Addr: Start address in memory which should be compared
*    NumBytes: Number of bytes to be compared
*    pBuff: Pointer to the data to be compared
*
*  Return value 
*    == (Addr + NumBytes): O.K.
*    != (Addr + NumBytes): *not* O.K.
*
*/
#if SUPPORT_NATIVE_VERIFY
U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff) {
  unsigned char *pFlash;
  unsigned long r;

  pFlash = (unsigned char *)Addr;
  r = Addr + NumBytes;
  do {
      if (*pFlash != *pBuff) {
        r = (unsigned long)pFlash;
        break;
      }
      pFlash++;
      pBuff++;
  } while (--NumBytes);
  return r;
}
#endif

/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Checks if a memory region is blank
*
*  Parameters
*    Addr: Blank check start address
*    NumBytes: Number of bytes to be checked
*    BlankData: Pointer to the destination data
*
*  Return value 
*    0 O.K., blank
*    1 O.K., *not* blank
*    <  0 Error
*
*/
#if SUPPORT_BLANK_CHECK
int BlankCheck(U32 Addr, U32 NumBytes, U8 BlankData) {
  U8* pData;
  
  pData = (U8 *)Addr;
  do {
    if (*pData++ != BlankData) {
      return 1;
    }
  } while (--NumBytes);
  return 0;
}
#endif

/*********************************************************************
*
*       SEGGER_OPEN_Read
*
*  Function description
*    Reads a specified number of bytes into the provided buffer
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value 
*    >= 0 O.K., NumBytes read
*    <  0 Error
*
*/
#if SUPPORT_NATIVE_READ_BACK
int SEGGER_OPEN_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff) {
  //
  // Read function
  // Add your code here...
  //
  return NumBytes;
}
#endif
