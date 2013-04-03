#ifndef TINY_FIFO_INCLUDE
#define TINY_FIFO_INCLUDE

#include "datatypes.h"

void tinyFifoPutc(u8 c);
s8   tinyFifoGetc(u8* c);
u8   tinyFifoLength(void);

u8   tinyFifoEmpty(void);


#endif

