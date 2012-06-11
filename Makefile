#cartelle dove sono contenute le librerie mips
LIBDIR = /usr/local/lib/umps2/
SHAREDIR = /usr/local/share/umps2/

#cartella contenente i files di inclusioni (header)
INCLUDEDIR = include/ 

#compilatore
CC = mipsel-linux-gcc
#linker
LD = mipsel-linux-ld

#parametri del compilatore
CFLAGS = -c -I ${INCLUDEDIR}

#parametri del linker
LFLAGS = -T ${SHAREDIR}elf32ltsmip.h.umpscore.x ${LIBDIR}crtso.o ${LIBDIR}libumps.o

all:	clean pcb asl util scheduler traps syscalls interrupts main link
	umps2-elf2umps -k kernel

#compilazione

pcb:
	${CC} ${CFLAGS} pcb.c

asl:
	${CC} ${CFLAGS} asl.c
	
util:
	${CC} ${CFLAGS} util.c
	
scheduler:
	${CC} ${CFLAGS} scheduler.c
	
traps:
	${CC} ${CFLAGS} traps.c
	
syscalls:
	${CC} ${CFLAGS} syscalls.c
	
interrupts:
	${CC} ${CFLAGS} interrupts.c
	
main:
	${CC} ${CFLAGS} main.c

p1test: 
	${CC} ${CFLAGS} p1test_pkaya_v1.1.c
	
#linking

link:
	${LD} ${LFLAGS} pcb.o asl.o util.o scheduler.o traps.o syscalls.o interrupts.o main.o -o kernel 

clean:
	rm -f *.o kernel*
cleanall:
	make clean
	rm -f *.o
