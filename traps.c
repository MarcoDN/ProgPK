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

/*Macro per accesso alla OldArea come stato del processore*/
#define progtrap_oldarea	((state_t *) PGMTRAP_OLDAREA)
#define sysbp_oldarea  		((state_t *) SYSBK_OLDAREA)
#define tbltrap_oldarea		((state_t *) TBL_OLDAREA)

extern state_t new_old_areas[MAX_CPU][8];

/*Gestore delle SysCall e BreackPoint*/
void trapsHandler() {
	int cause_exCode, KoUMode;

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(((state_t*)PGMTRAP_OLDAREA),(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/*Recupero il bit di modalità della sysBp Old Area (controllo se Kernel o User mode)*/
	KoUMode = ((sysbp_oldarea->status) & STATUS_KUp) >> 3;

	/*Recupero il codice del tipo di eccezione scatetanata*/
	cause_exCode = CAUSE_EXCCODE_GET(sysbp_oldarea->cause);

	/*Se il codice dell'eccezione è una syscall o bp*/
	if (cause_exCode == EXC_SYSCALL){

		/*Se il bit di lavoro è in user mode*/
		if (KoUMode == 0){

			/*Caso 1: chiamata una syscall da 1 a 11 ma non in modalità kernel --> ProgramTrap*/
			if((sysbp_oldarea->reg_a0 > 0) && (sysbp_oldarea->reg_a0 < 12)){
			      
				/* Imposto lo stato del processore con codice RI --> Riserved Instruction Exception */
				sysbp_oldarea->cause = CAUSE_EXCCODE_SET(sysbp_oldarea->cause, EXC_RESERVEDINSTR);
				
				/*Salva lo stato della SysBP Old Area nella pgmTrap Old Area */
				copyState(sysbp_oldarea, progtrap_oldarea);
				
				/*Invoco il gestore delle ProgramTrap*/
				pgmTrapHandler();
			  
			}
			else{
				/*Se non è stata ancora avviata la SYSCALL11 termino il processo*/
				/*TODO Non capisco se manca il vettore per la gestione dei trapmanager o se sono io che lo devo creare da boot*/
				/*if(nome_vettore_trap -> == 0){
				      
				      KILLALL (current);
				      current = NULL;
				      restartScheduler();
				  }
				  Altrimenti il PGMTRAPMANAGER è stato specificato e si deve passare il controllo a lui
				  else{
				  }
				*/
			}
			
		}
		/*Altrimenti vanno gestite le SYSCALL direttamente in kernel mode --> richiamo il syscallhandler contenuto in syscalls.c*/
		else
			sysHandler();
		}
	/*Chiamata syscallbp inesistente*/
	else PANIC();
}

/*Gestore delle eccezione statenate da ProgramTrap*/
void progTrapHandler(){
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);
	int ris;

	/* Se il processo è in esecuzione, viene direttamente caricato il gestore*/
	if(current != NULL)
		copyState(progtrap_oldarea, &(current->p_s));

	/* Se non è definita la proTrapmanager il processo viene ucciso
	if(current-> nome di quel cazzo di vettore per le trap == 0){
		KILLALL(current);
		current = NULL;
		restartScheduler();
	}
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente
	else
	{
		copyState(pgmTrap_old, currentProcess->pgmtrapState_old);
	}*/
}
