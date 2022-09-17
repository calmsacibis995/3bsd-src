/*	param.h	2.1	1/5/80	*/

/*
 * tunable variables
 *
 * NB: NBUF is well known in locore.s
 */

#define	NBUF	48		/* size of buffer cache */
#define	NINODE	200		/* number of in core inodes */
#define	NFILE	175		/* number of in core file structures */
#define	NMOUNT	8		/* number of mountable file systems */
#define	MAXUPRC	25		/* max processes per user */
#define	SSIZE	4		/* initial stack size (*512 bytes) */
#define	SINCR	4		/* increment of stack (*512 bytes) */
#define	NOFILE	20		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	SMAPSIZ	(4*NPROC)	/* size of swap allocation area */
#define	NCALL	40		/* max simultaneous time callouts */
#define	NPROC	125		/* max number of processes */
#define	NTEXT	60		/* max number of pure texts */
#define	NCLIST	500		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */
#define	TIMEZONE (8*60)		/* Minutes westward from Greenwich */
#define	DSTFLAG	1		/* Daylight Saving Time applies in this locality */
#define	MSGBUFS	128		/* Characters saved from error messages */
#define	NCARGS	5120		/* # characters in exec arglist */
/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PRIUBA	24
#define	PZERO	25
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	40
#define	PUSER	50

#define	NZERO	20

/*
 * signals
 * dont change
 */

#define	NSIG	17
/*
 * No more than 16 signals (1-16) because they are
 * stored in bits in a word.
 */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQUIT	3	/* quit (FS) */
#define	SIGINS	4	/* illegal instruction */
#define	SIGTRC	5	/* trace or breakpoint */
#define	SIGIOT	6	/* iot */
#define	SIGEMT	7	/* emt */
#define	SIGFPT	8	/* floating exception */
#define	SIGKIL	9	/* kill, uncatchable termination */
#define	SIGBUS	10	/* bus error */
#define	SIGSEG	11	/* segmentation violation */
#define	SIGSYS	12	/* bad system call */
#define	SIGPIPE	13	/* end of pipe */
#define	SIGCLK	14	/* alarm clock */
#define	SIGTRM	15	/* Catchable termination */

/*
 * fundamental constants of the implementation--
 * cannot be changed easily.
 * note: UPAGES is well known in locore.s
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */

#define	UPAGES	6		/* pages of u-area */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	SUPERB	((daddr_t)1)	/* block number of the super block */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * Clustering of hardware pages on machines with ridiculously small
 * page sizes is done here.  The paging subsystem deals with units of
 * CLSIZE pte's describing NBPG (from vm.h) pages each... BSIZE must
 * be CLSIZE*NBPG in the current implementation, that is the paging subsystem
 * deals with the same size blocks that the file system uses.
 *
 * NOTE: SSIZE, SINCR and UPAGES must be multiples of CLSIZE
 */
#define	CLSIZE	2

/* give the base virtual address (first of CLSIZE) */
#define	clbase(i)	((i) &~ (CLSIZE-1))

/* round a number of clicks up to a whole cluster */
#define	clrnd(i)	(((i) + (CLSIZE-1)) &~ (CLSIZE-1))

#if CLSIZE==1
#define	BSIZE	512		/* size of secondary block (bytes) */
#define	INOPB	8		/* 8 inodes per block */
#define	BMASK	0777		/* BSIZE-1 */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	NICINOD	100		/* number of superblock inodes */
#define	NICFREE	50		/* number of superblock free blocks */

#endif

#if CLSIZE==2
#define	BSIZE	1024
#define	INOPB	16
#define	BMASK	01777
#define	BSHIFT	10
#define	NMASK	0377
#define	NSHIFT	8
#define	NICINOD	100
#define	NICFREE	178
#endif

#if CLSIZE==4
#define	BSIZE	2048
#define	INOPB	32
#define	BMASK	03777
#define	BSHIFT	11
#define	NMASK	0777
#define	NSHIFT	9
#define	NICINOD	100
#define	NICFREE	434
#endif

/* inumber to disk address and inumber to disk offset */
#define	itod(x)	((daddr_t)((((unsigned)(x)+2*INOPB-1)/INOPB)))
#define	itoo(x)	((int)(((x)+2*INOPB-1)%INOPB))

/* file system blocks to disk blocks and back */
#define	fsbtodb(b)	((b)*CLSIZE)
#define	dbtofsb(b)	((b)/CLSIZE)

/* BSLOP can be 0 unless you have a TIU/Spider */
#define	BSLOP	0		/* In case some device needs bigger buffers */
#define	NINDIR	(BSIZE/sizeof(daddr_t))

#define	CBSIZE	28		/* number of chars in a clist block */
#define	CROUND	0x1F		/* clist rounding; sizeof(int *) + CBSIZE -1*/
#define	CLKTICK	(1000000/(HZ))	/* microseconds in a clock tick */

/*
 * Some macros for units conversion
 */
/* Core clicks (512 bytes) to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks (512 bytes) to disk blocks */
#define	ctod(x)	(x)

/* clicks to bytes */
#define	ctob(x)	((x)<<9)

/* bytes to clicks */
#define	btoc(x)	((((unsigned)(x)+511)>>9))

/* major part of a device */
#define	major(x)	((int)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	minor(x)	((int)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))

typedef	struct { int r[1]; } *	physadr;
typedef	int		daddr_t;
typedef	char *		caddr_t;
typedef	unsigned short	ino_t;
typedef	int		swblk_t;
typedef	int		size_t;
typedef	int		time_t;
typedef	int		label_t[10];
typedef	short		dev_t;
typedef	int		off_t;

/*
 * Machine-dependent bits and macros
 */
#define	UMODE	PSL_CURMOD		/* usermode bits */
#define	USERMODE(ps)	(((ps) & UMODE) == UMODE)

#define	BASEPRI(ps)	(((ps) & PSL_IPL) != 0)

#ifdef KERNEL
#ifdef lint
int	__void__;
#define	VOID	__void__ = (int)
#else
#define	VOID
#endif
#endif
