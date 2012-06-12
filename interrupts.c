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

#include "scheduler.h"
#include "traps.h"
#include "syscalls.h"
#include "interrupts.h"

#include "utilTest.h"

extern pcb_t *locksemaphore;
void intHandler() {

	int cause = getCAUSE();
	if(CAUSE_IP_GET(cause,INT_TIMER)){
		/*set interval timer*/
		SET_IT(5000);
		/*eseguo v su system call*/


				/* if v non sblocca salvo stato dispositivo
				 * &currentThread->t_state)
				 * else chiamo la sys8*/

	}else if(CAUSE_IP_GET(cause,INT_TERMINAL)){/*terminal interrupt*/
		/*set inteval timer*/
		SET_IT(5000);
		/*eseguo v su system call*/

		/* if v non sblocca salvo stato dispositivo
		 * else chiamo la sys8*/
	}
	else{
		return;
	}

}

