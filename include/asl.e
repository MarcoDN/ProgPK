#ifndef ASL_E
#define ASL_E

#include "const.h"
#include "types11.h"
#include "listx.h"

/* ASL handling functions */
semd_t* getSemd(int key);
void initASL();

int insertBlocked(int key,pcb_t* p);
pcb_t* removeBlocked(int key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int key);
void outChildBlocked(pcb_t *p);

#endif
