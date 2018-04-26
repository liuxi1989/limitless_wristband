// Copyright (c) 2016-2017, ARM Limited or its affiliates. All rights reserved
//
// This file and the related binary are licensed under the ARM Object Code and
// Headers License; you may not use these files except in compliance with this
// license.
//
// You may obtain a copy of the License at <.../external/nrf_cc310/license.txt>
//
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef DX_LINUX_PLATFORM
#include <sys/mman.h>//for mmap
#include <unistd.h>//for close()
#include <sys/types.h>//open()
#include <sys/stat.h>//open()
#include <fcntl.h>//open()
#include <stdio.h>//for printf
#include <errno.h>//for errno
#include <pthread.h>
#endif
#include <stdint.h>
#include "ssi_pal_types.h"
#include "ssi_regs.h"
//#include "integration_test_ssi_defs.h"
#include "integration_test_plat_defs.h"
#include "crys_rnd.h"

#include "nrf52840.h"


void CRYPTOCELL_IRQHandler(void)
{
    INTEG_TEST_PRINT("Got Cryptocell interrupt\r\n");
}

/*Globals*/
uint32_t g_free_mem_addr;
uint32_t *g_test_stack_base_addr;
uint32_t g_rom_base_addr;
uint32_t g_env_rom_base_addr;


CRYS_RND_Context_t*   rndContext_ptr;
CRYS_RND_WorkBuff_t*  rndWorkBuff_ptr;

CRYS_RND_Context_t   rndContext;
CRYS_RND_WorkBuff_t  rndWorkBuff;

uint32_t* UserSpace = 0;
int fd_mem;

//The following implementation is for linux OS, using mmap
//This should be modified according to platform environment
SaSiError_t   mapEnvMemory()
{
#ifdef DX_LINUX_PLATFORM
	/*Open device*/
	if ((fd_mem=open("/dev/mem", O_RDWR|O_SYNC))<0)
	{
		INTEG_TEST_PRINT("Error: Can not open /dev/mem. errno=%u",errno);
		return -1;	
	}

   /*Perform mapping for HW base address*/
   g_rom_base_addr = (unsigned int)mmap(NULL, REG_AREA_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, REG_BASE_ADR);
   if (g_rom_base_addr == 0 || g_rom_base_addr == 0xFFFFFFFF ) {
      INTEG_TEST_PRINT("Error: mmap failed");
      return -1;
   }

   INTEG_TEST_PRINT("g_rom_base_addr is 0x%X, mapped to 0x%X\n",(uint32_t)g_rom_base_addr, REG_BASE_ADR);
    /*PErform mapping for env. base address*/
   g_env_rom_base_addr = (unsigned int)mmap(NULL, ENV_REG_AREA_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, ENV_REG_BASE_ADR);
   if (g_env_rom_base_addr == 0 || g_env_rom_base_addr == 0xFFFFFFFF ) {
      INTEG_TEST_PRINT("Error: mmap failed");
      return -1;
   }

   INTEG_TEST_PRINT("g_env_rom_base_addr is 0x%X, mapped to 0x%X\n",(uint32_t)g_env_rom_base_addr, ENV_REG_BASE_ADR);

    
	/*map base address*/
	g_free_mem_addr = (unsigned int)mmap((uint32_t *)FREE_MEM_BASE_ADR, CONTIG_FREE_MEM, 
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, 
		fd_mem, FREE_MEM_BASE_ADR);

	if (g_free_mem_addr == 0 || g_free_mem_addr == 0xFFFFFFFF) {
		INTEG_TEST_PRINT("Error: mmap failed");
		return -1;    
	}

	INTEG_TEST_PRINT("g_free_mem_addr is 0x%X, mapped to 0x%X\n",(uint32_t)g_free_mem_addr, FREE_MEM_BASE_ADR);

 	/*Map address for stack with fixed address*/
	g_test_stack_base_addr = (uint32_t *)mmap((uint32_t *)PTHREAD_STACK_BASE_ADR, PTHREAD_STACK_SIZE, 
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, 
		fd_mem, PTHREAD_STACK_BASE_ADR);

	if (g_test_stack_base_addr == (uint32_t*)NULL || g_test_stack_base_addr == (uint32_t*)0xFFFFFFFF) {
		INTEG_TEST_PRINT("Error:PTHREAD_STACK_BASE_ADR mmap failed");
		return -1;    
	}

	INTEG_TEST_PRINT("g_test_stack_base_addr is 0x%X, mapped to 0x%X size %x\n",(uint32_t)g_test_stack_base_addr, PTHREAD_STACK_BASE_ADR,PTHREAD_STACK_SIZE);
	INTEG_TEST_PRINT("\n end of mapping \n");
#else

#endif
	return 0;
}


void unmapMemory(void)
{
#ifdef DX_LINUX_PLATFORM
	close(fd_mem);
#endif
        INTEG_TEST_PRINT("==================== TEST END ====================\r\n");
}

//initializatoins that need to be done prior to running the integration tests.
SaSiError_t integration_tests_setup(void)
{
    SaSiError_t ret = 0;
    rndContext_ptr = &rndContext;
    rndWorkBuff_ptr = &rndWorkBuff;

    // Initialize Segger RTT logging

#if NRF_SDK_PRESENT

    #if NRF_MODULE_ENABLED(NRF_LOG)
    uint32_t err_code = NRF_LOG_INIT(NULL);
    if(err_code != 0)
    {
        return 1;
    }
    #endif
    
#else
    
    (void)SEGGER_RTT_Init();

#endif

    INTEG_TEST_PRINT("==================== TEST START ====================\r\n");
        
    NRF_CRYPTOCELL->ENABLE = 1;

    return ret;
}

void integration_tests_clear(void)
{
    INTEG_TEST_PRINT("==================== TEST END ====================\r\n");
    while(1);
}
