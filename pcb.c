
/*****************************************************************************
 * Marco Di Nicola, Michele Lambertini,                						 *
                        Elena Fabbri, Lorenzo Vinci                          *
 *****************************************************************************/

#include "types11.h" /* typedef struct di pcb_t e sem_t */


/* array di max processi consentiti (20) */
pcb_t pcbFree_table[MAXPROC];

/* Definizione elemento sentinella della PcbFree */
struct list_head pcbFree_h;


/* variabile di controllo per non aggiungere più di 20 pcb_t in esecuzione nel sistema,
come da specifica*/
HIDDEN unsigned int available;


/* 1. void initPcbs(); */
/* Estrae tutti i pcb_t dall'array dei processi massimi consentiti e li inserisce nalla coda di quelli liberi, non ancora utilizzati. */
void initPcbs() {

	int j;

	INIT_LIST_HEAD(&pcbFree_h); /* inizializzo la testa della pcbFree */
	available = MAXPROC;

	/* uno alla volta estraggo i pcbFree dall'array e ne inizializzo i list_head p_child, per uso futuro negli alberi di pcb */
	for(j=0;j<MAXPROC;j++) {

		list_add_tail(&(pcbFree_table[j].p_next), &pcbFree_h);

		INIT_LIST_HEAD(&(pcbFree_table[j].p_child));

	}

}




/* 2. void freePcb(pcb_t * p) */
/* Inserisce il pcb puntato da p nella coda di quelli liberi, la pcbFree */

void freePcb(pcb_t *p) {

	if (available < MAXPROC) {

		list_add_tail(&p->p_next, &pcbFree_h);

		available++; /* mi ricordo che ora ho un processo in più tra quelli liberi 
									e quindi incremento available */

	}
}








/* 3. pcb_t* allocPcb() */
/* Estrae il pcb in testa alla coda della pcbFree, inizializzandone i campi. Se tale coda è vuota, viene restituito NULL*/
pcb_t* allocPcb() {

	int i;
	struct pcb_t * elem;

	if (list_empty(&pcbFree_h))
		return NULL;
	else {

		elem = container_of(pcbFree_h.next, pcb_t, p_next); 
		/* copio indirizzo del primo elemento puntato dalla sentinella estraendolo dal suo list_head */

		list_del(pcbFree_h.next);

		INIT_LIST_HEAD(&(elem->p_child));
		INIT_LIST_HEAD(&(elem->p_sib));
		INIT_LIST_HEAD(&(elem->p_next));

		/* Inizializzo i campi interi del pcb. Tutti i list head escluso il puntatore a list_head p_parent sono
		stati inizializzati dalla funzione prcedente quindi non devo occuparmene */
		elem -> p_parent = NULL;
		elem -> p_s.entry_hi = 0;
		elem -> p_s.cause = 0;
		elem -> p_s.status = 0;
		elem -> p_s.pc_epc = 0;
		for (i = 0; i < 29; i++)
			elem -> p_s.gpr[i] = 0;
		elem -> p_s.hi = 0;
		elem -> p_s.lo = 0;
		elem -> priority = 0;
		elem -> p_semkey = -1;

		available--; /* Avendo estratto l'elemento in testa alla coda pcbFree, 
										ora ne ho disponibiile uno in meno */
		return elem;

	}

}


/* 4. pcb_t* mkEmptyProcQ(struct list_head* head) */
/* Inizializza una coda di pcb puntata da head, ossia inizializzandone il list_head sentinella */
void mkEmptyProcQ(struct list_head * head) {

	INIT_LIST_HEAD(head);

	return;

}

/* 5. int emptyProcQ(struct list_head* head) */
/* Restituisce TRUE se la cosa puntata da head è vuota, FALSE altrimenti */
int emptyProcQ(struct list_head* head)  {

	if (list_empty(head))
		return TRUE;
	else
		return FALSE;

}

/* 6 void insertProcQ(struct list_head* head, pcb_t *p) */
/* Inserisce il pcb puntato da p nella coda puntata da head. L'inserimento avviene secondo la priorità dei pcb. La coda è ordinata in ordine decrescente di priorità dei processi.  */

void insertProcQ(struct list_head* head, pcb_t *p) {

	pcb_t *tmp; /* Puntatore a pcb_t per estrarre pcb. */
	struct list_head *pos; /* Puntatore a list_head per ciclare su coda. */
	int currentPriority = p->priority;  /* Memorizzo il valore di priorità del pcb_t passato alla funzione. */

	/* Ciclo su coda, verificando per ogni pcb già presente la priopria priorità.
	Ad ogni ciclo viene confrontata la priorità del pcb corrente con quella del pcb passato alla funzione,
	e disposto di conseguenza dentro alla coda in modo da ottenerla ordinata. */
	list_for_each(pos,head) { 

		tmp = container_of(pos,pcb_t,p_next);

		if ( tmp->priority < currentPriority ) { /* Se priorità è più bassa di quella dell'elemento passato
																								 alla funzione */

			__list_add(&p->p_next,pos->prev,pos);

			return;

		}

	}

	list_add_tail(&p->p_next,head); /* nel caso la lista in cui voglio aggiungere il pc sia vuota, non mi devo
																	   preoccupare dell'ordine */

}


/* 7. pcb_t headProcQ(struct list_head* head) */
/* Restituisce il puntatore al pcb in testa alla coda puntata da head, SENZA ESTRARLO.
Qualora la coda fosse vuota, restitusce NULL */
pcb_t* headProcQ(struct list_head * head) {

	if (emptyProcQ(head))
		return NULL;
	else
		return container_of(list_next(head), pcb_t , p_next);

}


/* 8. pcb_t* removeProc(struct list_head* head) */
/* Rimuove il primo pcb, quello in testa, nella coda puntata da head restituendone il puntatore ad esso.
Se tale coda è vuota viene restituito NULL */
pcb_t* removeProcQ(struct list_head* head) {

	pcb_t* res; /* puntatore al pcb da estrarre richiesto */

	if (emptyProcQ(head))
		return NULL;
	else {

		res = container_of(head->next, pcb_t, p_next); /* N.B.:
																											il primo elemento è il puntato dalla sentinella 
																											head (dal campo next, NON prev)*/

		list_del(&(res->p_next));

		return res;

	}
}

/* 9. pcb_t* outProcQ(struct list_head* head, pcb_t *p) */
/* Estrae il pcb puntato da p nella coda puntata da head. 
   Il pcb puntato da p può trovarsi in posizione QUALSIASI all'interno della coda puntata da head.
   Viene restituito il puntatore al pcb rimosso.
   Se nella non c'è il pcb puntato da p, è restituito NULL. */

pcb_t* outProcQ(struct list_head* head, pcb_t *p) {

	pcb_t *tmp,*result = NULL; /* result è il puntatore che restituiamo, per ora inizializzato a NULL.*/


	/* Considerando ogni elemento della coda puntata da head, lo estraggo mettendolo nel pcb* temp e 			confrontando se tale puntatore è uguale a quello passato alla funzione. Questo fino a trovare, se esiste, il pcb richiesto.*/


	list_for_each_entry(tmp,head,p_next) {

		if ( tmp==p ) {

			list_del(&(tmp->p_next));

			result = tmp;

		}

	}

	return result;

}


/* 10. int emptyChild(pcb_t *p) */
/* Restituisce TRUE se il pcb puntato da p non ha figli, FALSE altrimenti */
int emptyChild(pcb_t *p) {

	if (list_empty(&(p->p_child)))
		return TRUE;
	else
		return FALSE;

}




/* 11. void insertChild(pcb_t *prnt, pcb_t *p) */
/* Aggiunge il pcb puntato da p come figlio del pc puntato da prnt. 
Questo significa due cose: se il pcb puntato da prnt non ha ancora figli, il pcb puntato da p è inserito
come figlio unico diretto. In caso contrario il pcb puntato da p è inserito come fratello dei figli già presenti del pcb puntato da prnt. Questo si risolve semplicemente aggiungendo in coda il pcb puntato da p, che sia figlio unico o un ulteriore fratello.*/
void insertChild(pcb_t *prnt, pcb_t *p) {

	list_add_tail (&p->p_sib , &prnt->p_child);

	p->p_parent = prnt;

	return;
}





/* 12. pcb_t* removeChild(pcb_t *p) */
/* Rimuove il PRIMO figlio dei figli del pcb puntato da p. 
Se il pcb puntato da p non ha figli, è restituito il valore NULL */
pcb_t* removeChild(pcb_t* p) {


	if (!emptyChild(p)) {

		struct pcb_t *children;

		/*Caso 2: p ha sicuramente almeno un figlio*/

		children = container_of(p->p_child.next, pcb_t, p_sib);

		/*Elimino il legame del figlio nella lista fratelli*/
		list_del (&(children->p_sib));

		/*Elimino il legame del figlio dal padre*/
		children->p_parent=NULL;

		return children;

	}

	/*Caso 1: p non ha figli. Restituire NULL*/
	return NULL;

}







/* 13. pcb_t * outChild(pcb_t * p) */
/* Rimuove il pcb puntato da p dalla lista dei figli di suo padre.
Se pcb* p non ha un padre, è restituito il valore NULL.
A differenza della removeChild p può essere un figlio qualasiasi di suo padre,
o il primo diretto oppure uno qualsiasi tra i fratelli. */

pcb_t * outChild(pcb_t * p) {

	pcb_t *tmp;

	/* Dal pcb puntato da p risalgo a ritroso fino a suo padre, e da qui alla lista di tutti i figli */
	struct list_head *head = &( p->p_parent->p_child );

	list_for_each_entry(tmp,head,p_sib) {

		if ( tmp==p ) { /* Ad ogni ciclo estraggo un figlio del padre del pcb puntato da p
											 e verifico se tale figlio corrisponde a quello passato alla funzione */

			list_del(&(tmp->p_sib)); /* Se trovo il figlio corrispondente, rompo il legame col padre,
																	o coi fratelli e ne restituisco il puntatore ad esso. */

			return tmp;

		}

	}

	/* Se il pcb puntato da p non ha un padre. */
	return NULL;

}
