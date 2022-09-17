/*	conf.c	2.1	1/5/80	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/tty.h"
#include "../h/conf.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/acct.h"
#include "../h/mba.h"

int	nulldev();
int	nodev();

int	hpstrategy(), hpread(), hpwrite(), hpintr();
struct	buf	hptab;
 
int	htopen(), htclose(), htstrategy(), htread(), htwrite();
struct	buf	httab;

#ifdef ERNIE
int	uhpopen(), uhpstrategy(), uhpread(), uhpwrite();
struct	buf	uhptab;

int	urpopen(), urpstrategy(), urpread(), urpwrite();
struct	buf	urptab;
#endif

struct bdevsw	bdevsw[] =
{
/* 0 */	nulldev,	nulldev,	hpstrategy,	&hptab,
/* 1 */	htopen,		htclose,	htstrategy,	&httab,
#ifdef ERNIE
/* 2 */	uhpopen,	nulldev,	uhpstrategy,	&uhptab,
/* 3 */	urpopen,	nulldev,	urpstrategy,	&urptab,
#endif
	0,
};

int	cnopen(), cnclose(), cnread(), cnwrite(), cnioctl();

#ifdef ERNIE
int	klopen(), klclose(), klread(), klwrite(), klioctl();
struct	tty kl_tty[];
#endif

int	flopen(), flclose(), flread(), flwrite();

int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl(), dzstop();
struct	tty dz_tty[];

int	syopen(), syread(), sywrite(), syioctl();

int 	mmread(), mmwrite();

#ifdef ERNIE
int	vpopen(), vpclose(), vpwrite(), vpioctl();
#endif

int	mxopen(), mxclose(), mxread(), mxwrite(), mxioctl();
int	mcread();
char	*mcwrite();

struct cdevsw	cdevsw[] =
{
/* 0 */		cnopen, cnclose, cnread, cnwrite, cnioctl, nulldev, 0,
/* 1 */		dzopen, dzclose, dzread, dzwrite, dzioctl, dzstop, dz_tty,
/* 2 */		syopen, nulldev, syread, sywrite, syioctl, nulldev, 0,
/* 3 */		nulldev, nulldev, mmread, mmwrite, nodev, nulldev, 0,
/* 4 */		nulldev, nulldev, hpread, hpwrite, nodev, nodev, 0,
/* 5 */		htopen,  htclose, htread, htwrite, nodev, nodev, 0,
#ifdef ERNIE
/* 6 */		vpopen, vpclose, nodev, vpwrite, vpioctl, nulldev, 0,
/* 7 */		klopen, klclose, klread, klwrite, klioctl, nulldev, kl_tty,
#else
/* 6 */		nodev, nodev, nodev, nodev, nodev, nodev, 0,
/* 7 */		nodev, nodev, nodev, nodev, nodev, nodev, 0,
#endif
/* 8 */		flopen, flclose, flread, flwrite, nodev, nodev, 0,
/* 9 */		mxopen, mxclose, mxread, mxwrite, mxioctl, nulldev, 0,
#ifdef ERNIE
/* 10 */	uhpopen, nulldev, uhpread, uhpwrite, nodev, nodev, 0,
/* 11 */	urpopen, nulldev, urpread, urpwrite, nodev, nodev, 0,
#endif
		0,
};

int	ttyopen(),ttread();
char	*ttwrite();
int	ttyinput(), ttyrend();
 
struct	linesw linesw[] =
{
	ttyopen, nulldev, ttread, ttwrite, nodev,
	ttyinput, ttyrend, nulldev, nulldev, nulldev,	/* 0 */
	mxopen, mxclose, mcread, mcwrite, mxioctl,
	nulldev, nulldev, nulldev, nulldev, nulldev,	/* 1 */
	0
};
 
int	nldisp = 1;
dev_t	rootdev	= makedev(0, 0);
dev_t	swapdev	= makedev(0, 1);
dev_t	pipedev	= makedev(0, 0);
daddr_t swplo = 1;		/* (swplo-1) % CLSIZE must be 0 */
int	nswap = 33439;
 
struct	buf	buf[NBUF];
struct	file	file[NFILE];
struct	inode	inode[NINODE];
struct	text	text[NTEXT];
struct	proc	proc[NPROC];
struct	buf	bfreelist;
struct	buf	bswlist;	/* free list of swap headers */
struct	buf	*bclnlist;	/* header for list of cleaned pages */
struct	acct	acctbuf;
struct	inode	*acctp;
 
int	mem_no = 3; 	/* major device number of memory special file */


extern struct user u;

int mbanum[] = {	/* mba number of major device */
	0,		/* disk */
	1,		/* tape */
	9999999,	/* unused */
	9999999,	/* unused */
	0,		/* disk, raw */
	1,		/* tape, raw */
};

int *mbaloc[] = { 	/* virtual location of mba */
	(int *)MBA0,
	(int *)MBA1,
};
