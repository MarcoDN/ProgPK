#ifndef _TYPES_H
#define _TYPES_H
#include "const.h"
#include "base.h"

/************************** Device register types ***************************/

/* Device register type for disks, tapes and printers (dtp) */
typedef struct {
  U32 status;
	U32 command;
	U32 data0;
	U32 data1;
} dtpreg_t;

/* Device register type for terminals; fields have different meanings */
typedef struct {
  U32 recv_status;
	U32 recv_command;
	U32 transm_status;
	U32 transm_command;
} termreg_t;

/* With a pointer to devreg_t we can refer to a "generic" device register, and
   then use the appropriate union member to access the fields (not strictly 
	 necessary, but for convenience and clarity) */
typedef union {
  dtpreg_t dtp;
	termreg_t term;
} devreg_t;

/************************* CPU state type ***********************************/

typedef struct {
	U32 entry_hi;
	U32 cause;
	U32 status;
	U32 pc_epc;  /* pc in the new area, epc in the old area */
	U32 gpr[29];
	U32 hi;
	U32 lo;				
} state_t;

#define reg_at  gpr[0]  /* so we can use registers by name */
#define reg_v0  gpr[1]
#define reg_v1  gpr[2]
#define reg_a0  gpr[3]
#define reg_a1  gpr[4]
#define reg_a2  gpr[5]
#define reg_a3  gpr[6]
#define reg_t0  gpr[7]
#define reg_t1  gpr[8]
#define reg_t2  gpr[9]
#define reg_t3  gpr[10]
#define reg_t4  gpr[11]
#define reg_t5  gpr[12]
#define reg_t6  gpr[13]
#define reg_t7  gpr[14]
#define reg_s0  gpr[15]
#define reg_s1  gpr[16]
#define reg_s2  gpr[17]
#define reg_s3  gpr[18]
#define reg_s4  gpr[19]
#define reg_s5  gpr[20]
#define reg_s6  gpr[21]
#define reg_s7  gpr[22]
#define reg_t8  gpr[23]
#define reg_t9  gpr[24]
#define reg_gp  gpr[25]
#define reg_sp  gpr[26]
#define reg_fp  gpr[27]
#define reg_ra  gpr[28]
#define reg_HI  gpr[29]
#define reg_LO  gpr[30]


/********************** process-supplied trap vectors ***********************/
typedef struct {
  state_t *old_area;
	state_t *new_area;
} sys5_vect_t;

typedef struct {
  U32 max_cyl;
  U32 max_head;
  U32 max_sect;
} disk_data_t;

/******************* VM data structures *************************************/
typedef struct {
  U32 entry_hi;
  U32 entry_lo;
} pte_entry_t;

/* PTE for kUseg2 and kUseg3 */
typedef struct {
  U32 header;
  pte_entry_t pte[KUSEG_PAGES]; /* kUseg2 and kUseg3 PTEs have same length */
} uPTE_t;

/* PTE for ksegOS (has more entries) */
typedef struct {
  U32 header;
  pte_entry_t pte[KSEGOS_PAGES];
} osPTE_t;

/* segment table type */
typedef struct {
  osPTE_t *ksegOS_pte;
  uPTE_t *kUseg2_pte;
  uPTE_t *kUseg3_pte;
} segtable_t;

#endif
