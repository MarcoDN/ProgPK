/*
 * traps.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "types11.h"
#include "const.h"
#include "copy.h"
#include "sysvars.h"
#include "scheduler.h"
#include "syscalls.h"
#include "terminate.h"

/*Macro per accesso alla OldArea come stato del processore*/
#define progtrap_oldarea	((state_t *) PGMTRAP_OLDAREA)
#define sysbp_oldarea  		((state_t *) SYSBK_OLDAREA)
#define tbltrap_oldarea		((state_t *) TBL_OLDAREA)

extern state_t new_old_areas[MAX_CPU][8];

/*Gestore delle eccezione statenate da ProgramTrap*/
void progTrapHandler(){
	
	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(progtrap_oldarea,(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;;

	/* Se non è definita la proTrapmanager il processo viene ucciso */
	if(current->def_areas[3] == 0){
		terminateprocess(current, cpu);
		current = NULL;
	}
	
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente*/
	else
		copyState(progtrap_oldarea, current->def_areas[3]);
	
	/*Passo il controllo allo scheduler*/
	restartScheduler();
}

/*Gestore delle eccezione statenate da ProgramTrap*/
void tblExceptionHandler(){
	int cause_exCode, KoUMode;

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(tbltrap_oldarea,(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/* Se non è definita la proTrapmanager il processo viene ucciso */
	if(current-> def_areas[1] == 0){
		terminateprocess(current, cpu);
		current = NULL;
	}
	
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente*/
	else
		copyState(tbltrap_oldarea, current->def_areas[1]);
	
	/*Passo il controllo allo scheduler*/
	restartScheduler();
}

/*Gestore delle SysCall e BreackPoint*/
void trapsHandler() {
	int cause_exCode;

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Recupero il codice del tipo di eccezione scatetanata*/
	cause_exCode = CAUSE_EXCCODE_GET(current->p_s.cause);
	
	if ((cause_exCode > 0) && (cause_exCode < 4))
		tblExceptionHandler();
	else if (((cause_exCode > 3) && (cause_exCode < 8)) || ((cause_exCode > 9) && (cause_exCode < 15)))
			progTrapHandler();
	
}
