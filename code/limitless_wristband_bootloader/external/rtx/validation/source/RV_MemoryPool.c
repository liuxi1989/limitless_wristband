/*-----------------------------------------------------------------------------
 *      Name:         RV_MemoryPool.c 
 *      Purpose:      CMSIS RTOS validation tests implementation
 *-----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include <string.h>
#include "RV_Typedefs.h"
#include "cmsis_rv.h"
#include "cmsis_os.h"

/*-----------------------------------------------------------------------------
 *      Test implementation
 *----------------------------------------------------------------------------*/
#define MEMBL_CNT  8
#define MEMBL_SZ  32
typedef struct {
  uint8_t Buf[MEMBL_SZ];
} MEM_BLOCK;

osPoolDef (MemPool,     8, MEM_BLOCK);
osPoolDef (MemPool_Isr, 2, MEM_BLOCK);

osPoolId MemPool_Id;

/* Definitions for TC_MemPoolAllocAndFreeComb */
uint32_t LFSR_32Bit (void);

osPoolId MemPool_IdIsr;
osStatus MemPool_StIsr;

void *MemPtr_Isr[2];

/*-----------------------------------------------------------------------------
 *      Default IRQ Handler
 *----------------------------------------------------------------------------*/
void MemoryPool_IRQHandler (void) {
  
  switch (ISR_ExNum) {
    case 0: MemPool_IdIsr = osPoolCreate (osPool(MemPool_Isr));         break;
    case 1: MemPtr_Isr[0] = osPoolAlloc (MemPool_IdIsr);                break;
    case 2: MemPtr_Isr[1] = osPoolCAlloc(MemPool_IdIsr);                break;
    case 3: MemPool_StIsr = osPoolFree  (MemPool_IdIsr, MemPtr_Isr[0]); break;
    case 4: MemPool_StIsr = osPoolFree  (MemPool_IdIsr, MemPtr_Isr[1]); break;
  }
}


/*-----------------------------------------------------------------------------
 *      Initialization: create main memory pool
 *----------------------------------------------------------------------------*/
void CreateMemoryPool (void) {
  /* Create a memory pool */
  MemPool_Id = osPoolCreate (osPool (MemPool));
  ASSERT_TRUE (MemPool_Id != NULL);
}

/*-----------------------------------------------------------------------------
 * 32-bit LFSR with maximal period (x^32 + x^31 + x^29 + x + 1)
 *----------------------------------------------------------------------------*/
uint32_t LFSR_32Bit (void) {
  static uint32_t lfsr = 0xAA55AA55;
  lfsr = (lfsr >> 1) ^ (-(lfsr & 1U) & 0xD0000001U);
  return (lfsr);
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup mempool_func Memory Pool Functions
\brief Memory Pool Functions Test Cases
\details
The test cases check the osPool* functions.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MemPoolAllocAndFree
\details
- Allocate all memory blocks
- Verify that only available blocks can be allocated
- Return all memory blocks back to a memory pool
*/
void TC_MemPoolAllocAndFree (void) {
  uint32_t *mp, *addr[MEMBL_CNT];
  uint32_t  i;

  /* Clear pointer array */
  for (i = 0; i < MEMBL_CNT; i++) { addr[i] = NULL; }
  
  if (MemPool_Id != NULL) {
    /* - Allocate all memory blocks */
    for (i = 0; i < MEMBL_CNT; i++) {
      addr[i] = (uint32_t *)osPoolAlloc (MemPool_Id);
      ASSERT_TRUE (addr[i] != NULL);
    }
    /* - Verify that only available blocks can be allocated */
    mp = (uint32_t *)osPoolAlloc (MemPool_Id);
    ASSERT_TRUE (mp == NULL);

    /* - Return all blocks back to a memory pool */
    for (i = 0; i < MEMBL_CNT; i++) {
      if (addr[i] != NULL) {
        ASSERT_TRUE (osPoolFree (MemPool_Id, addr[i]) == osOK);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MemPoolAllocAndFreeComb
\details
- Allocate all memory blocks
- Verify that only available blocks can be allocated
- Return all memory blocks back to a memory pool
*/
void TC_MemPoolAllocAndFreeComb (void) {
  uint32_t *addr[MEMBL_CNT];
  uint32_t i, k, set, alloc;

  ASSERT_TRUE (MEMBL_CNT <= 32);

  if (MEMBL_CNT <= 32) {
    /* - Perform psevdorandom memory allocations of available memory blocks */
    for (i = 0; i < MEMBL_CNT; i++) {
      /* Clear pointer array */
      addr [i] = 0;
    }

    for (k = 0; k < 512; k++) {
      set = LFSR_32Bit ();
      
      for (i = 0; i < MEMBL_CNT; i++) {
        /* If bit is set, allocate memory block else free it */
        alloc = (set & (1 << i)) ? (1) : (0);
        
        if (alloc) {
          if (addr[i] == 0) {
            /* Memory block is not allocated, we will do that now */
            addr[i] = (uint32_t *)osPoolAlloc (MemPool_Id);
            ASSERT_TRUE (addr[i] != NULL);
          }
        }
        else {
          if (addr[i] != 0) {
            /* Memory block was allocated, we can free it */
            ASSERT_TRUE (osPoolFree (MemPool_Id, addr[i]) == osOK);
            addr [i] = 0;
          }
        }
      }
    }
    /* Free any memory block that remained allocated */
    for (i = 0; i < MEMBL_CNT; i++) {
      if (addr[i] != 0) {
        ASSERT_TRUE (osPoolFree (MemPool_Id, addr[i]) == osOK);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MemPoolZeroInit
\details
- Allocate all memory blocks and fill them with pattern
- Return all blocks to a memory pool
- Allocate all memory blocks with osPoolCAlloc
- Check if memory was zero initialized
- Return all blocks to a memory pool
*/
void TC_MemPoolZeroInit (void) {
  uint32_t  *addr[MEMBL_CNT];  
  uint32_t   i, k;
  MEM_BLOCK *bp;

  if (MemPool_Id != NULL) {
    /* Allocate all memory blocks and fill them with pattern */
    for (i = 0; i < MEMBL_CNT; i++) {
      /* Get address of the allocated memory block */
      addr[i] = osPoolAlloc (MemPool_Id);
      ASSERT_TRUE (addr[i] != NULL);

      if (addr[i] != NULL) {
        bp = (MEM_BLOCK *)addr[i];
        
        /* Fill memory with pattern */
        for (k = 0; k < MEMBL_SZ; k++) { bp->Buf[k] = 0xAA; }
      }
    }
    /* Return all blocks to a memory pool */
    for (i = 0; i < MEMBL_CNT; i++) {
      ASSERT_TRUE (osPoolFree (MemPool_Id, addr[i]) == osOK);
    }
    
    /* Check if memory initialized to zero */
    for (i = 0; i < MEMBL_CNT; i++) {
      /* Allocate zero initialized block */
      addr[i] = osPoolCAlloc (MemPool_Id);
      ASSERT_TRUE (addr != NULL);
      
      if (addr[i] != NULL) {
        bp = (MEM_BLOCK *)addr[i];
        for (k = 0; k < MEMBL_SZ; k++) {
          ASSERT_TRUE (bp->Buf[k] == 0);
        }
      }
    }
    /* Return all blocks to a memory pool */
    for (i = 0; i < MEMBL_CNT; i++) {
      ASSERT_TRUE (osPoolFree (MemPool_Id, addr[i]) == osOK);
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MemPoolParam
\details
- Test memory pool management functions with invalid parameters
*/
void TC_MemPoolParam (void) {
  uint32_t val = 0;
  
  ASSERT_TRUE (osPoolAlloc  (NULL) == NULL);
  ASSERT_TRUE (osPoolCAlloc (NULL) == NULL);
  ASSERT_TRUE (osPoolCreate (NULL) == NULL);
  
  ASSERT_TRUE (osPoolFree (NULL,       NULL) == osErrorParameter);
  ASSERT_TRUE (osPoolFree (NULL,       &val) == osErrorParameter);
  ASSERT_TRUE (osPoolFree (MemPool_Id, &val) == osErrorValue);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MemPoolInterrupts
\details
- Call all memory pool management functions from the ISR
*/
void TC_MemPoolInterrupts (void) {
  
  TST_IRQHandler = MemoryPool_IRQHandler;
  
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
  
  ISR_ExNum = 0; /* Test: osPoolCreate */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (MemPool_IdIsr == NULL);
  
  if (MemPool_Id != NULL) {
    MemPool_IdIsr = MemPool_Id;
    
    ISR_ExNum = 1;  /* Test: osPoolAlloc */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (MemPtr_Isr[0] != NULL);

    ISR_ExNum = 2;  /* Test: osPoolCAlloc */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (MemPtr_Isr[1] != NULL);
    
    ISR_ExNum = 3; /* Test: osPoolFree */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (MemPool_StIsr == osOK);

    ISR_ExNum = 4; /* Test: osPoolFree */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (MemPool_StIsr == osOK);
  }
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/**
@}
*/ 
// end of group mempool_func

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
