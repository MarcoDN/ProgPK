/*
 * asl.c
 *
 *  Created on: 20/gen/2012
 *  Author: Marco Di Nicola
 *
 *  Abstract: Questo modulo contiene le funzioni atte a operare su liste di oggetti di tipo semd_t: i descrittori dei semafori
 *  utilizzati per gestire la concorrenza dei processi nel sistema operativo pkaya2012.
 *  Questi sararanno organizzati in due liste: semdFree, contenente quelli non utilizzati; ASL (Active Semaphore List),
 *  contenente quelli su cui uno o più processi sono bloccati, in attesa di accedere alle risorse del sistema.
 *
 */

#include "types11.h"

HIDDEN semd_t semd_table[MAXPROC]; /* Array contenente tutti i semd. */

/* Sentinelle che definiscono rispettivamente le liste semdFree e ASL. */
HIDDEN struct list_head semdFree_h , semd_h;

/* 14:  Estraggo il puntatore ad un semd con determinata key, dalla ASL. */

semd_t* getSemd(int	key) {

	if (key>=0 && key<MAXPROC) {  /* Controlla che la key sia nel range compreso tra 0 e MAXPROC. */

		semd_t *semd = &semd_table[key];

		if (!list_empty(&semd->s_procQ)) /* Se la coda di processi bloccati su questo semd non è vuota, si trova nella ASL e viene restituito. */
			return semd;

	}

	return NULL; /* semd non trovato o valore di key invalido, restituisco NULL. */

}

/* 15: Inserisco un pcb nella coda di un semd. */

int	insertBlocked(int key,pcb_t	*p) {

	semd_t* current;  /* Puntatore per il semd da modificare. */

	if (key>=0 && key<MAXPROC) { /* Controlla che la key sia nel range compreso tra 0 e MAXPROC. */

		if ((current = getSemd(key))==NULL) { /* Se non è in già in uso il semd con la key fornita, nella ASL: */

			current = &semd_table[key];

			list_del(&current->s_next);

			list_add(&current->s_next,&semd_h); /* Concatena il semd con key data alla ASL,per l'utilizzo. */

		}

		p->p_semkey = key; /* Inizializzo la key del pcb a quella data. */

		list_add_tail(&p->p_next,&current->s_procQ); /* Aggiungo il processo alla coda del semd (già presente nella ASL
	o appena estratto dalla semdFree). */

		return FALSE;

	}
	else return TRUE;

}

/* 16: Restituisco, rimuovendolo, il pcb in testa alla coda di questo semd */

pcb_t*	removeBlocked(int key)	{

	semd_t* current;  /* Puntatore per il semd scelto. */
	pcb_t* result = NULL; /* Puntatore al primo pcb in coda. Inizializzato a NULL. */
	struct list_head* entry; /* Puntatore per trasferimento elementi tra liste. */

	/* Controllo che un semd con la key fornita sia nella ASL */
	if ((current = getSemd(key))!=NULL) {

		entry = list_next(&current->s_procQ); /* Prendo il primo list_head dalla coda dei processi del semd. */

		list_del(entry); /* Rimuovo il primo list_head (con il pcb di riferimento) dalla coda. */

		result = container_of(entry,pcb_t,p_next); /* Estraggo il pcb relativo al list_head ottenuto. */

		result->p_semkey = -1;

		if (list_empty(&current->s_procQ)) {  /* Se la coda dei pcb di questo semd è ormai vuota. */

			list_del(&current->s_next); /* Rimuovo riferimenti all'elemento selezionato dalla semdFree. */

			list_add(&current->s_next,&semdFree_h); /* Restituisco l'elemento alla semdFree list. */

		}

	}

	return result;  /* Restituisce al chiamante il puntatore trovato o NULL, qualora il semd non fosse presente nella ASL. */

}

/* 17: Rimuovo il pcb puntato da p dalla coda dei processi all'interno del semd corrispondente. */

pcb_t* outBlocked(pcb_t	*p) {

	semd_t* current;

	/* Cerca il semd con la key del pcb dato, nella ASL. */
	if ((current = getSemd(p->p_semkey))!=NULL && !list_empty(&p->p_next)) {   /* Se il semd relativo a questo pcb è presente nella ASL */

		list_del(&p->p_next);  /* Lo rimuove dalla coda. */

		p->p_semkey = -1;

		return p;  /* Lo restituisce al chiamante. */

	}

	/* Se la computazione arriva a questo punto: Nessun semd con questa key nella ASL o pcb non presente nella
		coda del suo semd. */
	return NULL;

}

/* 18: Restituisco il pcb in testa alla coda di questo semd. */

pcb_t* headBlocked(int key) {

	semd_t* current;  /* Puntatore per il semd scelto. */

	if ((current = getSemd(key))!=NULL && !list_empty(&current->s_procQ))  /* Se è in uso un semd con la key fornita nella ASL e
																				la coda dei pcb di questo semd non è vuota: */
		return container_of(list_next(&current->s_procQ),pcb_t,p_next); /* Restituisco il primo pcb bloccato su essa. */

	else return NULL;  /* semd non presente nella ASL. */

}

/* 19: Elimina ricorsivamente tutti i pcb discendenti di quello dato,dalle code dei loro semd. */

void  outChildBlocked(pcb_t	*p) {

	pcb_t* child;

	outBlocked(p);  /* Elimino il pcb dato dalla coda del suo semd,se presente. */

	/* Itero sulla lista dei figli, definita da p_child, e applico ricorsivamente la funzione outChildBlocked su ognuno di essi. */
	list_for_each_entry(child,&p->p_child,p_sib)
	outChildBlocked(child);

}

/* 20:  Inizializzo la semdFree in modo da contenere tutti i semd disponibili. */

void initASL() {

	int i;

	INIT_LIST_HEAD(&semdFree_h); /* Inizializzo l'elemento sentinella della semdFree. */

	INIT_LIST_HEAD(&semd_h);  /* Inizializzo l'elemento sentinella della ASL. */

	for (i=0;i<MAXPROC;i++) { /* Ciclo sull'array di semd allocati, concatenandone i puntatori nella ASL. */
		list_add(&semd_table[i].s_next,&semdFree_h);
		/* Ne inizializzo le key e le code di processi bloccati. */
		semd_table[i].s_key = i;
		INIT_LIST_HEAD(&semd_table[i].s_procQ);
	}

}

