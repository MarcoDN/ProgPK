/*
 * syscalls.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_


#include "include/asl.e"
#include "include/base.h"
#include "include/const.h"
#include "include/const11.h"
#include "include/copy.h"
#include "include/libumps.h"
#include "include/pcb.e"
#include "include/sysvars.h"
#include "include/types11.h"
#include "include/uMPStypes.h"
#include "include/scheduler.h"

#define FREE 1
#define BUSY 0

void sysHandler();

unsigned int sem_esclusion[MAXPROC]; //tutti inizializzati e free
//le chiamate ai semafori, P e V, devono essere eseguite senza essere interrompibili ed in mutua esclusione.
//quindi prima di iniziare un'operazione su semaforo, bisogna che nessun'altra cpu stia facendo quella operazione
//su quel semaforo. Ho quindi una variabile globale per ogni semaforo, che mi indica se sia libero o occupato

void P(semd_t *s, int key, pcb_t *caller) {
	while(!CAS(&sem_esclusion[key], FREE, BUSY)) ; //aspetto fino a quando si è liberato il semaforo

	s->s_value = (s->s_value)--;
	if (s->s_value <= 0) {

		//se il semaforo ha valore 0 o minore, il processo si blocca
		//devo estrarlo dalla coda ready e metterlo in quella del seamforo;
		caller->p_semkey = key;
		insertBlocked(key, caller);
		//devo inserire adesso questo semaforo nella asl
	}

	CAS(&sem_esclusion[key], BUSY, FREE);
	return;

}



void V(semd_t *s, int key) {
	while(!CAS(&sem_esclusion[key], FREE, BUSY)) ;
	s->s_value = (s->s_value)++;
	int cpu = getPRID();

	//se il semaforo ha dei processi bloccati in coda
	if(!(emptyProcQ(&s->s_procQ))) {
		pcb_t* wake_pcb = removeBlocked(key);
		enqueueProcess(wake_pcb, cpu);
	}
	else {/*togliere il semaforo dalla ASL*/}
	CAS(&sem_esclusion[key], BUSY, FREE);
}


#endif /* SYSCALLS_H_ */
