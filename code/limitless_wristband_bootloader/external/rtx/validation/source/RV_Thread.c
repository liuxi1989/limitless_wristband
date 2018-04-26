/*-----------------------------------------------------------------------------
 *      Name:         TC_Thread.c 
 *      Purpose:      CMSIS RTOS Thread Management test
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

/* Definitions of shared variables */
uint32_t Var_Counter;
static void  Th_CountingThread  (void const *arg);
osThreadDef (Th_CountingThread, osPriorityNormal, 1, 0);

/* Definitions for TC_ThreadCreate */
uint32_t Var_ThreadExec;

void Th_Thr0 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr1 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr2 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr3 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr4 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr5 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr6 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }
void Th_Thr7 (void const *arg) { Var_ThreadExec += (arg == NULL) ? (1) : (2); }

void Th_Run  (void const *arg) { while(1) { osThreadYield(); } }
void Th_isr  (void const *arg) { while(1) {;}                  }

osThreadDef (Th_Thr0, osPriorityIdle,         1, 0);
osThreadDef (Th_Thr1, osPriorityLow,          1, 0);
osThreadDef (Th_Thr2, osPriorityBelowNormal,  1, 0);
osThreadDef (Th_Thr3, osPriorityNormal,       1, 0);
osThreadDef (Th_Thr4, osPriorityAboveNormal,  1, 0);
osThreadDef (Th_Thr5, osPriorityHigh,         1, 0);
osThreadDef (Th_Thr6, osPriorityRealtime,     1, 0);
osThreadDef (Th_Thr7, osPriorityError,        1, 0);

osThreadDef (Th_Run,  osPriorityIdle,         1, 0);
osThreadDef (Th_isr,  osPriorityIdle,         1, 0);


/* Definitions for TC_ThreadMultiInstance */
void Th_MultiInst (void const *arg);
osThreadDef (Th_MultiInst, osPriorityNormal,  5, 0);


/* Definitions for TC_ThreadGetId */
#define GETID_THR_CNT 7

void Th_GetId0 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId1 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId2 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId3 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId4 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId5 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }
void Th_GetId6 (void const *arg) { osThreadId *p = (osThreadId *)arg; *p = osThreadGetId(); }

osThreadDef (Th_GetId0, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId1, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId2, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId3, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId4, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId5, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_GetId6, osPriorityBelowNormal, 1, 0);


/* Definitions for TC_ThreadPriorityExec */
osThreadId ExecArr[7];

void Th_PrioExec (void const *arg);
osThreadDef (Th_PrioExec, osPriorityIdle, 7, 0);


/* Definitions for TC_ThreadChainedCreate */
void Th_Child_0 (void const *arg);
void Th_Child_1 (void const *arg);
void Th_Child_2 (void const *arg);
void Th_Child_3 (void const *arg);

osThreadDef (Th_Child_0, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_Child_1, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_Child_2, osPriorityBelowNormal, 1, 0);
osThreadDef (Th_Child_3, osPriorityBelowNormal, 1, 0);

typedef struct {
  uint32_t  Cnt;                        /* Counter value                      */
  uint32_t  Res;                        /* Result                             */
} YIELD_TEST;

void Th_YieldTest (void const *arg);
osThreadDef (Th_YieldTest, osPriorityNormal, 1, 0);


/* Definitions for thread management test in the ISR */

osThreadId ThId_Running;                /* Thread ID of the running thread    */
osThreadId ThId_Isr;                    /* Thread ID returned in the ISR      */
osPriority ThPr_Isr;                    /* Thread priority returned in the ISR*/
osStatus   ThSt_Isr;                    /* Status returned in the ISR         */


/*-----------------------------------------------------------------------------
 * Counting thread
 *----------------------------------------------------------------------------*/
static void Th_CountingThread  (void const *arg) {
  while(1) {
    Var_Counter++;
    osDelay (5);
  }
}


/*-----------------------------------------------------------------------------
 *      IRQ Handler
 *----------------------------------------------------------------------------*/
void Thread_IRQHandler (void) {
  
  switch (ISR_ExNum) {
    case 0: ThId_Isr = osThreadCreate (osThread (Th_isr), NULL);            break;
    case 1: ThId_Isr = osThreadGetId ();                                    break;
    case 2: ThPr_Isr = osThreadGetPriority (ThId_Running);                  break;
    case 3: ThSt_Isr = osThreadSetPriority (ThId_Running, osPriorityIdle ); break;
    case 4: ThSt_Isr = osThreadTerminate   (ThId_Running);                  break;
    case 5: ThSt_Isr = osThreadYield ();                                    break;
  }
}

/*-----------------------------------------------------------------------------
 * Auxiliary thread used to test creation of multiple instances
 *----------------------------------------------------------------------------*/
void Th_MultiInst (void const *arg) {
  uint8_t *var = (uint8_t *)arg;
  *var += 1;
}


/*-----------------------------------------------------------------------------
 * Auxiliary thread for testing execution by priority
 *----------------------------------------------------------------------------*/
void Th_PrioExec (void const *arg) {
  uint32_t i;
  for (i = 0; i < 7; i++) {
    if (ExecArr[i] == 0) {
      ExecArr[i] = *(osThreadId *)arg;
      break;
    }
  }
}

/*-----------------------------------------------------------------------------
 * Child threads for TC_ThreadChainedCreate
 *----------------------------------------------------------------------------*/
void Th_Child_0 (void const *arg) {
  uint32_t *arr = (uint32_t *)arg;
  arr[0] = 1;
  ASSERT_TRUE (osThreadCreate (osThread (Th_Child_1), &arr[1]) != NULL);
}
void Th_Child_1 (void const *arg) {
  uint32_t *arr = (uint32_t *)arg;
  arr[0] = 2;
  ASSERT_TRUE (osThreadCreate (osThread (Th_Child_2), &arr[1]) != NULL);
}
void Th_Child_2 (void const *arg) {
  uint32_t *arr = (uint32_t *)arg;
  arr[0] = 3;
  ASSERT_TRUE (osThreadCreate (osThread (Th_Child_3), &arr[1]) != NULL);
}
void Th_Child_3 (void const *arg) {
  uint32_t *arr = (uint32_t *)arg;
  arr[0] = 4;
}

/*-----------------------------------------------------------------------------
 * Yield test thread for TC_ThreadYield
 *----------------------------------------------------------------------------*/
void Th_YieldTest (void const *arg) {
  YIELD_TEST *cfg = (YIELD_TEST *)arg;
  uint32_t i;
  
  for (i = 0; i < cfg->Cnt; i++);
  ASSERT_TRUE (osThreadYield() == osOK); 
  
  cfg->Res = i;
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup thread_funcs Thread Functions
\brief Thread Functions Test Cases
\details
The test cases check the osThread* functions.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadCreate
\details
- Create threads with different priorities and pass NULL argument to them
- Create threads with different priorities and pass valid argument to them
*/
void TC_ThreadCreate (void) {
  uint32_t arg = 0xFF;
  
  Var_ThreadExec = 0;

  /* - Create threads with different priorities and pass NULL argument to them */
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr0), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr1), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr2), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr3), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr4), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr5), NULL) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr6), NULL) != NULL);
  
  /* Wait until all threads go into inactive state */
  osDelay (100);
  
  /* Check if they were all executed */
  ASSERT_TRUE (Var_ThreadExec == 7);
  Var_ThreadExec = 0;

  /* - Create threads with different priorities and pass valid argument to them */
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr0), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr1), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr2), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr3), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr4), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr5), &arg) != NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr6), &arg) != NULL);

  /* Wait until all threads go into inactive state */
  osDelay (100);

  /* Check if they were all executed */
  ASSERT_TRUE (Var_ThreadExec == 14);

  /* Creating thread with invalid priority should fail */
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr7), NULL) == NULL);
  ASSERT_TRUE (osThreadCreate (osThread(Th_Thr7), &arg) == NULL);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadMultiInstance
\details
- Synchronise on time slice
- Create all defined instances of a thread
- Verify each instance id
- Wait until all instances executed
- Verify execution variable
*/
void TC_ThreadMultiInstance (void) {
  uint8_t    cnt[5];
  uint32_t   i;
  osThreadId id[5];

  /* Init counter array */
  for (i = 0; i < 5; i++) {
    cnt[i] = 0;
  }

  /* - Synchronise on time slice */
  osDelay (1);

  /* - Create all defined instances of a thread */
  for (i = 0; i < 5; i++) {
    id[i] = osThreadCreate (osThread (Th_MultiInst), &cnt[i]);
    /* - Verify each instance id */
    ASSERT_TRUE (id[i] != NULL);
  }

  /* - Wait until all instances executed */
  osDelay (100);
  
  /* - Verify execution variable */
  for (i = 0; i < 5; i++) {
    ASSERT_TRUE (cnt[i] == 1);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadTerminate
\details
- Initialize global counter variable to zero
- Create and run a test thread which increments a global counter
- Terminate test thread and remember counter value
- Wait and verify that thread is terminated by checking global counter
- Restart a thread and repeat the procedure
- Counter must not increment after test thread is terminated
*/
void TC_ThreadTerminate (void) {
  uint32_t i, cnt;
  osThreadId id;
  
  for (i = 0; i < 2; i++) {
    /* - Initialize global counter variable to zero */
    Var_Counter = 0;

    /* - Create and run a test thread which increments a global counter */
    id = osThreadCreate (osThread (Th_CountingThread), NULL);
    ASSERT_TRUE (id != NULL);

    if (id != NULL) {
      /* Allow counting thread to run */
      ASSERT_TRUE (osDelay(5) == osEventTimeout);
      /* - Terminate test thread and remember counter value */
      ASSERT_TRUE (osThreadTerminate (id) == osOK);
      ASSERT_TRUE (Var_Counter != 0);
      cnt = Var_Counter;

      ASSERT_TRUE (osDelay(5) == osEventTimeout);
      /* - Wait and verify that thread is terminated by checking global counter */
      ASSERT_TRUE (cnt == Var_Counter);
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadRestart
\details
- Create a counting thread and verify that it runs
- Terminate counting thread
- Recreate a counting thread and verify that is runs again
*/
void TC_ThreadRestart (void) {
  osThreadId id;
  uint32_t   cnt, iter;
  
  /* Initalize global counter variable to zero */
  Var_Counter = 0;
  
  cnt  = 0;
  iter = 0;
  while (iter++ < 2) {
    /* - Create a counting thread and verify that it runs */
    id = osThreadCreate (osThread (Th_CountingThread), NULL);
    ASSERT_TRUE (id != NULL);
  
    if (id != NULL) {
      /* Allow counting thread to run */
      ASSERT_TRUE (osDelay(5) == osEventTimeout);
      ASSERT_TRUE (osThreadTerminate (id) == osOK);

      if (iter == 1) {
        ASSERT_TRUE (Var_Counter != 0);
        cnt = Var_Counter;
      }
      else {
        ASSERT_TRUE (Var_Counter > cnt);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadGetId
\details
- Create multiple threads
- Each thread must call osThreadGetId and return its thread ID to the main thread
- Check if each thread returned correct ID
- osThreadGetId must return same ID as obtained by osThreadCreate
*/
void TC_ThreadGetId (void) {
  osThreadId id[2][7];
  osThreadId id_main;
  uint32_t i;

  /* - Create multiple threads */
  id[0][0] = osThreadCreate (osThread (Th_GetId0), &id[1][0]);
  id[0][1] = osThreadCreate (osThread (Th_GetId1), &id[1][1]);
  id[0][2] = osThreadCreate (osThread (Th_GetId2), &id[1][2]);
  id[0][3] = osThreadCreate (osThread (Th_GetId3), &id[1][3]);
  id[0][4] = osThreadCreate (osThread (Th_GetId4), &id[1][4]);
  id[0][5] = osThreadCreate (osThread (Th_GetId5), &id[1][5]);
  id[0][6] = osThreadCreate (osThread (Th_GetId6), &id[1][6]);

  /* Check if all threads were created */
  for (i = 0; i < 7; i++) {
    ASSERT_TRUE (id[0][i] != NULL);
  }

  /* Lower priority of the main thread */
  id_main = osThreadGetId();
  ASSERT_TRUE (id_main != NULL);
  
  if (id_main != NULL) {
    ASSERT_TRUE (osThreadSetPriority (id_main, osPriorityLow) == osOK);
    /* - Check if each thread returned correct ID */
    for (i = 0; i < 7; i++) {
      ASSERT_TRUE (id[0][i] == id[1][i]);
    }
    /* Restore priority of the main thread back to normal */
    ASSERT_TRUE (osThreadSetPriority (id_main, osPriorityNormal) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadPriority
\details
- Change priority of the main thread in steps from Idle to Realtime
- At each step check if priority was changed
- Do the same for main child thread
*/
void TC_ThreadPriority (void) {
  osThreadId t_idc, t_idr;
  
  /* Get main thread ID */
  t_idc = osThreadGetId();
  ASSERT_TRUE (t_idc != NULL);
  
  if (t_idc) {
    /* - Change priority of the main thread in steps from Idle to Realtime  */
    /* - At each step check if priority was changed                         */
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityIdle        ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityIdle);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityLow         ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityLow);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityBelowNormal ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityBelowNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityNormal      ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityAboveNormal ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityAboveNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityHigh        ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityHigh);
    
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityRealtime    ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idc) == osPriorityRealtime);
    
    /* Setting invalid priority value should fail */
    ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityError) == osErrorValue);
  }


  /* Create a child thread */
  t_idr = osThreadCreate (osThread (Th_Run), NULL);
  ASSERT_TRUE (t_idr != NULL);
  
  if (t_idr) {
    /* - Change priority of the child thread in steps from Idle to Realtime  */
    /* - At each step check if priority was changed                          */
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityIdle);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityLow         ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityLow);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityBelowNormal ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityBelowNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityNormal      ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityAboveNormal ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityAboveNormal);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityHigh        ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityHigh);
    
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityRealtime    ) == osOK);
    ASSERT_TRUE (osThreadGetPriority (t_idr) == osPriorityRealtime);

    /* Setting invalid priority value should fail */
    ASSERT_TRUE (osThreadSetPriority (t_idr, osPriorityError) == osErrorValue);
    
    /* Terminate the thread */
    ASSERT_TRUE (osThreadTerminate (t_idr) == osOK);
  }

  /* Restore priority of the main thread back to normal */
  ASSERT_TRUE (osThreadSetPriority (t_idc, osPriorityNormal) == osOK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadPriorityExec
\details
- Raise main thread priority to osPriorityRealtime
- Create multiple instances of the test thread
- Set instance priorities from low to realtime
- Lower main thread priority to allow execution of test threads
- Check if threads were executed according to their priorities
*/
void TC_ThreadPriorityExec (void) {
  uint32_t i;
  osThreadId main_id, inst[7];
  
  main_id = osThreadGetId();
  ASSERT_TRUE (main_id != NULL);

  if (main_id != NULL) {
    /* - Raise main thread priority to osPriorityRealtime */
    ASSERT_TRUE (osThreadSetPriority (main_id, osPriorityRealtime) == osOK);

    /* - Create multiple instances of the test thread */
    inst[0] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[0]);
    inst[1] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[1]);
    inst[2] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[2]);
    inst[3] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[3]);
    inst[4] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[4]);
    inst[5] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[5]);
    inst[6] = osThreadCreate (osThread (Th_PrioExec), (void *)&inst[6]);

    for (i = 0; i < 7; i++) {
      ASSERT_TRUE (inst[i] != NULL);
      if (inst[i] == NULL) {
        /* Abort test if thread instance not created */
        return;
      }
    }
    
    /* Clear execution array */
    for (i = 0; i < 7; i++) {
      ExecArr[i] = 0;
    }

    /* - Set instance priorities from low to realtime */
    ASSERT_TRUE (osThreadSetPriority (inst[0], osPriorityIdle)        == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[1], osPriorityLow)         == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[2], osPriorityBelowNormal) == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[3], osPriorityNormal)      == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[4], osPriorityAboveNormal) == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[5], osPriorityHigh)        == osOK);
    ASSERT_TRUE (osThreadSetPriority (inst[6], osPriorityRealtime)    == osOK);
    
    /* - Lower main thread priority to allow execution of test threads */
    ASSERT_TRUE (osThreadSetPriority (main_id, osPriorityIdle)        == osOK);
    ASSERT_TRUE (osThreadYield () == osOK);
    
    /* - Check if threads were executed according to their priorities */
    for (i = 0; i < 7; i++) {
      ASSERT_TRUE (ExecArr[i] == inst[7 - 1 - i]);
    }
  }
  /* - Restore main thread priority to osPriorityNormal */
  ASSERT_TRUE (osThreadSetPriority (main_id, osPriorityNormal) == osOK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadChainedCreate
\details
- Create a first thread in a chain
- Wait until child threads are created
- Verify that all child threads were created
*/
void TC_ThreadChainedCreate (void) {
  uint32_t i, arr[4];
  osThreadId id;
  
  /* Clear array */
  for (i = 0; i < 4; i++) {
    arr[i] = 0;
  }
  
  /* - Create a first thread in a chain */
  id = osThreadCreate (osThread  (Th_Child_0), arr);
  ASSERT_TRUE (id != NULL);
  
  /* - Wait until child threads are created */
  osDelay(50);
  
  /* - Verify that all child threads were created */
  for (i = 0; i < 4; i++) {
    ASSERT_TRUE (arr[i] == (i + 1));
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadYield
\details
- Raise priority of the main thread
- Create child thread instances with normal priority
- Lower priority of the main thread to allow child threads to run
- When child threads terminate, check counter values
- Restore priority of the main thread
*/
void TC_ThreadYield (void) {
  YIELD_TEST cfg[2];
  osThreadId id_main, id[2];

  /* Get main thread id */
  id_main = osThreadGetId();
  ASSERT_TRUE(id_main != NULL);
  
  /* Raise priority of the main thread */
  ASSERT_TRUE (osThreadSetPriority (id_main, osPriorityHigh) == osOK);
  
  /* Init test counters */
  cfg[0].Cnt=100;
  cfg[1].Cnt=1000;
  cfg[0].Res=0;
  cfg[1].Res=0;
  
  /* - Create instances of the same thread */
  /* - Each instance has different counter values */
  id[0] = osThreadCreate (osThread (Th_YieldTest), &cfg[0]);
  id[1] = osThreadCreate (osThread (Th_YieldTest), &cfg[1]);
  ASSERT_TRUE (id[0] != NULL);
  ASSERT_TRUE (id[1] != NULL);

  /* Lower priority of the main thread to allow child threads to run */
  ASSERT_TRUE (osThreadSetPriority (id_main, osPriorityLow) == osOK);  
  
  /* Both child threads shall be terminated, check expected counter values */
  ASSERT_TRUE (cfg[0].Res == cfg[0].Cnt);  
  ASSERT_TRUE (cfg[1].Res == cfg[1].Cnt); 
  
  /* Restore priority of the main thread back to normal */
  ASSERT_TRUE (osThreadSetPriority (id_main, osPriorityNormal) == osOK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadParam
\details
- Test thread management functions with invalid parameters
*/
void TC_ThreadParam (void) {
  /* - Test invalid thread definition on osThreadCreate */
  ASSERT_TRUE (osThreadCreate (NULL, NULL) == NULL);
  
  /* - Test invalid thread id on osThreadTerminate */
  ASSERT_TRUE (osThreadTerminate (NULL) == osErrorParameter);
  
  /* - Test invalid thread id on osThreadSetPriority */
  ASSERT_TRUE (osThreadSetPriority (NULL, osPriorityNormal) == osErrorParameter);
  
  /* - Test invalid thread id on osThreadGetPriority */
  ASSERT_TRUE (osThreadGetPriority (NULL) == osPriorityError);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_ThreadInterrupts
\details
- Call all thread management functions from the ISR
*/
void TC_ThreadInterrupts (void) {
  
  TST_IRQHandler = Thread_IRQHandler;
  
  ThId_Running = osThreadGetId ();
  ASSERT_TRUE (ThId_Running != NULL);
  
  if (ThId_Running != NULL) {

    NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);

    ISR_ExNum = 0; /* Test: osThreadCreate */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThId_Isr == NULL);
    
    ISR_ExNum = 1; /* Test: osThreadGetId */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThId_Isr == NULL);

    ISR_ExNum = 2; /* Test: osThreadGetPriority */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThPr_Isr == osPriorityError);
    
    ISR_ExNum = 3; /* Test: osThreadSetPriority */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThSt_Isr == osErrorISR);
    
    ISR_ExNum = 4; /* Test: osThreadTerminate */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThSt_Isr == osErrorISR);
    
    ISR_ExNum = 5; /* Test: osThreadYield */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (ThSt_Isr == osErrorISR);
    
    NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
  }
}

/**
@}
*/ 
// end of group thread_funcs

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
