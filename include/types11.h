#ifndef _TYPES11_H
#define _TYPES11_H

#include "uMPStypes.h"
#include "listx.h"

// Process Control Block (PCB) data structure
typedef struct pcb_t {
	/*process queue fields */
	struct list_head	p_next;

	/*process tree fields */
	struct pcb_t		*p_parent;
	struct list_head	p_child,
	p_sib;


	/* processor state, etc */
	state_t       		p_s;     

	/* process priority */
	int					priority;

	/* key of the semaphore on which the process is eventually blocked */
	int					p_semkey;

	/* field indicating whether this process was terminated or not. */
	int					killed;

	/* field containing the process' total execution time (in Microseconds) */
	unsigned long		cpu_time;

	/* indicates whether this pcb has to be removed or not

	/* pointers to eventual old/new areas the process might have redefined.
	 * to promote an easier manipulation they are defined as an array , containing in order:
	 *
	 * TLB Exceptions old/new area
	 * PgmTrap Exceptions old/new area
	 * Syscalls/BP Exceptions old/new area
	 *
	 *  */
	state_t*			def_areas[6];

} pcb_t;



// Semaphore Descriptor (SEMD) data structure
typedef struct semd_t {
	struct list_head	s_next;

	// Semaphore value
	int					s_value;

	// Semaphore key
	int					s_key;

	// Queue of PCBs blocked on the semaphore
	struct list_head	s_procQ;
} semd_t;



#endif
