#include <types11.h>
#include <const11.h>
#include <const.h>
#include <uMPStypes.h>

/*Macro che sostituisce la memcpy per gli stati*/
#define copyState(source, dest) ({\
		int i; for(i=0;i<29;i++)\
		(*dest).gpr[i]=(*source).gpr[i];\
		(*dest).entry_hi=(*source).entry_hi;\
		(*dest).cause=(*source).cause;\
		(*dest).status=(*source).status;\
		(*dest).pc_epc=(*source).pc_epc;\
		(*dest).hi=(*source).hi;\
		(*dest).lo=(*source).lo;\
})
