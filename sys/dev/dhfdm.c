/*	dhfdm.c	2.1	2/14/80	*/

/*
 *	DM-BB fake driver
 */
#include "../h/param.h"
#include "../h/tty.h"
#include "../h/conf.h"

struct	tty	dh11[];

dmopen(dev)
{
	register struct tty *tp;

	tp = &dh11[minor(dev)];
	tp->t_state |= CARR_ON;
}

dmclose(dev)
{
}
dmctl(dev)
{
}
