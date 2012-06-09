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

all:	clean pcb asl p1test link
	umps2-elf2umps -k kernel

#compilazione e linking

pcb:
	${CC} ${CFLAGS} pcb.c

asl:
	${CC} ${CFLAGS} asl.c

p1test: 
	${CC} ${CFLAGS} p1test_pkaya_v1.1.c

link:
	${LD} ${LFLAGS} pcb.o asl.o p1test_pkaya_v1.1.o -o kernel 

clean:
	rm -f *.o kernel*
cleanall:
	make clean
	rm -f *.o

