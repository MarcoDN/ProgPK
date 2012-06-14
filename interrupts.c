/*
 * intHandler.c
 *
 *  Created on: Jun 12, 2012
 *      Author: michele
 */


#include "const.h"
#include "asl.e"
#include "pcb.e"

#include "libumps.h"
#include "types11.h"
#include "sysvars.h"

#include "scheduler.h"
#include "traps.h"
#include "syscalls.h"
#include "interrupts.h"

#include "utilTest.h"
#include "copy.h"

//extern state_t new_old_areas[16][8];

//extern pcb_t *locksemaphore;
void intHandler() {

	int cpu = getPRID(),cause = getCAUSE();

	copyState(((state_t*)PGMTRAP_OLDAREA),(&running[cpu]->p_s));
	running[cpu]->p_s.pc_epc += WORD_SIZE;

	insertProcQ(&readyQ[cpu],running[cpu]);
	running[cpu] = NULL;

	if(CAUSE_IP_GET(cause,INT_TIMER)){

		LDST(&scheduler);
		/*eseguo v su system call*/


		/* if v non sblocca salvo stato dispositivo
		 * &currentThread->t_state)
		 * else chiamo la sys8*/

	}else if(CAUSE_IP_GET(cause,INT_TERMINAL)){/*terminal interrupt*/
		/*set inteval timer*/
		SET_IT(5000);
		/*eseguo v su system call*/
		/*if(check_V()){//return state device on process v0
			currentThread->t_state.reg_v0=(state_t *)INT_OLDAREA->reg_v0;
		}
		else{
			copyState((state_t *)INT_OLDAREA, &currentThread->t_state);
			unsigned int * terminal;
			terminal=(unsigned int) TERMINAL0ADDR +0x0000000c;
			if(((unsigned int) (TERMINAL0ADDR+0x00000008)&0x0000ff00)==0x0a)/*check ack*/
		//	*terminal=0x00000000;/*reset terminal*/
		//	else	*terminal=0x00000001;
		//	terminal=(unsigned int)TERMINAL0ADDR+0x00000008;
		/*lori fa partire la sys8*/
		/*metterlo in attesa con funzione di marco*/
		//}
	}

	/* if v non sblocca salvo stato dispositivo
	 * else chiamo la sys8*/



}
