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

#include <libumps.e>

extern state_t new_old_areas[16][8];
/* Indirizzo base del device */
HIDDEN memaddr device_baseaddr;
/* Variabili per accedere ai campi status e command di device o terminali */
HIDDEN int *read_s, *trans_s;
HIDDEN int *read_cmd, *trans_cmd;
HIDDEN int recvStatByte, transmStatByte;

HIDDEN int recognizeDev(int bitMapDevice)
{
	if (bitMapDevice == (bitMapDevice | 0x1)) return 0;
	if (bitMapDevice == (bitMapDevice | 0x2)) return 1;
	if (bitMapDevice == (bitMapDevice | 0x4)) return 2;
	if (bitMapDevice == (bitMapDevice | 0x8)) return 3;
	if (bitMapDevice == (bitMapDevice | 0x10)) return 4;
	if (bitMapDevice == (bitMapDevice | 0x20)) return 5;
	if (bitMapDevice == (bitMapDevice | 0x40)) return 6;
	return 7;
}

//extern pcb_t *locksemaphore;
void intHandler() {

	int int_cause;
	int *bitMapDevice;
	int devNumb;

	int cpu = getPRID(),cause = getCAUSE();
	pcb_t* current = getRunningProcess(cpu);
	if (cpu > 0)
		copyState((&new_old_areas[cpu][1]),(&running[cpu]->p_s));
	else
		copyState(((state_t*)INT_OLDAREA),(&running[cpu]->p_s));

	enqueueProcess(running[cpu],cpu);

	/* Linea 2 Interval Timer Interrupt + Gestione PSEUDO CLOCK ****************************/
	if (CAUSE_IP_GET(int_cause, INT_TIMER)) {

		/* Aggiornamento pseudo clock */
		/*	pseudo_tick = pseudo_tick + (GET_TODLOW - start_pseudo_tick);
		start_pseudo_tick = GET_TODLOW;

		assignProcess(&readyQ, current_Process);
		getRunningProcess(cpu);
	}
	else{

		SET_IT(PSEUDO_CLOCK_INTERVAL - pseudo_tick)
	/*eseguo v su system call*/


		/* if v non sblocca salvo stato dispositivo
		 * &currentThread->t_state)
		 * else chiamo la sys8*/

	}
	else if (CAUSE_IP_GET(cause,INT_TERMINAL)) { /*terminal interrupt*/

		/* Cerco la bitmap della linea attuale */
		bitMapDevice =(int *) (PENDING_BITMAP_START + (WORD_SIZE * (INT_TERMINAL - INT_LOWEST)));
		/*device number select on bitmap*/
		devNumb = recognizeDev(*bitMapDevice);
		/* Salvo indirizzo del Device Register */
		device_baseaddr = (memaddr)(DEV_REGS_START + ((INT_TERMINAL - INT_LOWEST) * 0x80) + (devNumb * 0x10));


		/* Recupera il campo status del device (ricezione) */
		read_s    = (int *) (device_baseaddr + 0x0);
		/* Recupera il campo status del device (trasmissione) */
		trans_s   = (int *) (device_baseaddr + 0x8);
		/* Recupera il campo command del device (ricezione) */
		read_cmd    = (int *) (device_baseaddr + 0x4);
		/* Recupera il campo command del device (trasmissione) */
		trans_cmd  = (int *) (device_baseaddr + 0xC);
		/* Estrae il byte dello status per capire cosa è avvenuto */
		recvStatByte   = (*read_s) & 0xFF;
		transmStatByte = (*trans_s) & 0xFF;
		/* Se è un carattere trasmesso */
		if(transmStatByte == DEV_TTRS_S_CHARTRSM)
		{
			/* Compie una V sul semaforo associato al device che ha causato l'interrupt */
			v = V(terminalWrite , 1 ,current)

									/* ACK per il riconoscimento dell'interrupt pendente */
									(*tCommand) = DEV_C_ACK;
		}
		/* Se è un carattere ricevuto */
		else if(recvStatByte == DEV_TRCV_S_CHARRECV)
		{
			/* Compie una V sul semaforo associato al device che ha causato l'interrupt */
			v = V(terminalWrite , 2 ,current)

									/* ACK per il riconoscimento dell'interrupt pendente */
									(*trans:cmd) = DEV_C_ACK;
		}
	}

	restartScheduler();

	/* if v non sblocca salvo stato dispositivo
	 * else chiamo la sys8*/



}
