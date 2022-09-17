#

/*
 * Mail -- a program for sending and receiving mail.
 *
 * Network name modification routines.
 */

#include "rcv.h"

/*
 * Map a name into the correct network "view" of the
 * name.  This is done by prepending the name with the
 * network address of the sender, then optimizing away
 * nonsense.
 */

char	*metanet = "!^:@";

char *
netmap(name, from)
	char name[], from[];
{
	char nbuf[BUFSIZ], ret[BUFSIZ];
	register char *cp;

	strcpy(nbuf, from);
	if (strlen(nbuf) == 0)
		return(name);
	cp = &nbuf[strlen(nbuf) - 1];
	while (!any(*cp, metanet) && cp > nbuf)
		cp--;
	if (cp == nbuf)
		return(name);
	*++cp = 0;
	strcat(nbuf, name);
	optim(nbuf, ret);
	if (!equal(name, ret))
		return((char *) savestr(ret));
	return(name);
}

/*
 * Turn a network machine name into a unique character
 * + give connection-to status.  BN -- connected to Bell Net.
 * AN -- connected to ARPA net, SN -- connected to Schmidt net.
 * CN -- connected to COCANET.
 */

#define	AN	1			/* Connected to ARPA net */
#define	BN	2			/* Connected to BTL net */
#define	CN	4			/* Connected to COCANET */
#define	SN	8			/* Connected to Schmidt net */

struct netmach {
	char	*nt_machine;
	char	nt_mid;
	short	nt_type;
} netmach[] = {
	"a",		'a',		SN,
	"b",		'b',		SN,
	"c",		'c',		SN,
	"d",		'd',		SN,
	"e",		'e',		SN,
	"f",		'f',		SN,
	"ccvax",	'f',		SN,
	"ingres",	'i',		AN|SN,
	"ing70",	'i',		AN|SN,
	"ingvax",	'j',		SN,
	"image",	'm',		SN,
	"optvax",	'o',		SN,
	"sesm",		'o',		SN,
	"q",		'q',		SN,
	"research",	'r',		BN,
	"src",		's',		SN,
	"vax",		'v',		BN|SN,
	"csvax",	'v',		BN|SN,
	"ucbvax",	'v',		BN|SN,
	"vax135",	'x',		BN,
	"cory",		'y',		SN,
	"eecs40",	'z',		SN,
	0,		0,		0
};

netlook(machine, attnet)
	char machine[];
{
	register struct netmach *np;
	register char *cp, *cp2;
	char nbuf[20];

	/*
	 * Make into lower case.
	 */

	for (cp = machine, cp2 = nbuf; *cp; *cp2++ = little(*cp++))
		;
	*cp2 = 0;

	/*
	 * If a single letter machine, look through those first.
	 */

	if (strlen(nbuf) == 1)
		for (np = netmach; np->nt_mid != 0; np++)
			if (np->nt_mid == nbuf[0])
				return(nbuf[0]);

	/*
	 * Look for usual name
	 */

	for (np = netmach; np->nt_mid != 0; np++)
		if (strcmp(np->nt_machine, nbuf) == 0)
			return(np->nt_mid);

	/*
	 * Look in side hash table.
	 */

	return(mstash(nbuf, attnet));
}

/*
 * Make a little character.
 */

little(c)
	register int c;
{

	if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	return(c);
}

/*
 * Turn a network unique character identifier into a network name.
 */

char *
netname(mid)
{
	register struct netmach *np;
	char *mlook();

	if (mid & 0200)
		return(mlook(mid));
	for (np = netmach; np->nt_mid != 0; np++)
		if (np->nt_mid == mid)
			return(np->nt_machine);
	return(NOSTR);
}

/*
 * Take a network machine descriptor and find the types of connected
 * nets and return it.
 */

nettype(mid)
{
	register struct netmach *np;

	if (mid & 0200)
		return(mtype(mid));
	for (np = netmach; np->nt_mid != 0; np++)
		if (np->nt_mid == mid)
			return(np->nt_type);
	return(0);
}

/*
 * Hashing routines to salt away machines seen scanning
 * networks paths that we don't know about.
 */

#define	XHSIZE		19		/* Size of extra hash table */
#define	NXMID		(XHSIZE*3/4)	/* Max extra machines */

struct xtrahash {
	char	*xh_name;		/* Name of machine */
	short	xh_mid;			/* Machine ID */
	short	xh_attnet;		/* Attached networks */
} xtrahash[XHSIZE];

struct xtrahash	*xtab[XHSIZE];		/* F: mid-->machine name */

short	midfree;			/* Next free machine id */

/*
 * Initialize the extra host hash table.
 * Called by sreset.
 */

minit()
{
	register struct xtrahash *xp, **tp;
	register int i;

	midfree = 0;
	tp = &xtab[0];
	for (xp = &xtrahash[0]; xp < &xtrahash[XHSIZE]; xp++) {
		xp->xh_name = NOSTR;
		xp->xh_mid = 0;
		xp->xh_attnet = 0;
		*tp++ = (struct xtrahash *) 0;
	}
}

/*
 * Stash a net name in the extra host hash table.
 * If a new entry is put in the hash table, deduce what
 * net the machine is attached to from the net character.
 *
 * If the machine is already known, add the given attached
 * net to those already known.
 */

mstash(name, attnet)
	char name[];
{
	register struct xtrahash *xp;
	struct xtrahash *xlocate();

	xp = xlocate(name);
	if (xp == (struct xtrahash *) 0) {
		printf("Ran out of machine id spots\n");
		return(0);
	}
	if (xp->xh_name == NOSTR) {
		if (midfree >= XHSIZE) {
			printf("Out of machine ids\n");
			return(0);
		}
		xtab[midfree] = xp;
		xp->xh_name = savestr(name);
		xp->xh_mid = 0200 + midfree++;
	}
	switch (attnet) {
	case '!':
	case '^':
		xp->xh_attnet |= BN;
		break;

	default:
	case ':':
		xp->xh_attnet |= SN;
		break;

	case '@':
		xp->xh_attnet |= AN;
		break;
	}
	return(xp->xh_mid);
}

/*
 * Search for the given name in the hash table
 * and return the pointer to it if found, or to the first
 * empty slot if not found.
 *
 * If no free slots can be found, return 0.
 */

struct xtrahash *
xlocate(name)
	char name[];
{
	register int h, q, i;
	register char *cp;
	register struct xtrahash *xp;

	for (h = 0, cp = name; *cp; h = (h << 2) + *cp++)
		;
	if (h < 0 && (h = -h) < 0)
		h = 0;
	h = h % XHSIZE;
	cp = name;
	for (i = 0, q = 0; q < XHSIZE; i++, q = i * i) {
		xp = &xtrahash[(h + q) % XHSIZE];
		if (xp->xh_name == NOSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
		if (h - q < 0)
			q += XHSIZE;
		xp = &xtrahash[(h - q) % XHSIZE];
		if (xp->xh_name == NOSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
	}
	return((struct xtrahash *) 0);
}

/*
 * Return the name from the extra host hash table corresponding
 * to the passed machine id.
 */

char *
mlook(mid)
{
	register int m;

	if ((mid & 0200) == 0)
		return(NOSTR);
	m = mid & 0177;
	if (m >= midfree) {
		printf("Use made of undefined machine id\n");
		return(NOSTR);
	}
	return(xtab[m]->xh_name);
}

/*
 * Return the bit mask of net's that the given extra host machine
 * id has so far.
 */

mtype(mid)
{
	register int m;

	if ((mid & 0200) == 0)
		return(0);
	m = mid & 0177;
	if (m >= midfree) {
		printf("Use made of undefined machine id\n");
		return(0);
	}
	return(xtab[m]->xh_attnet);
}

/*
 * Take a network name and optimize it.  This gloriously messy
 * opertions takes place as follows:  the name with machine names
 * in it is tokenized by mapping each machine name into a single
 * character machine id (netlook).  The separator characters (network
 * metacharacters) are left intact.  The last component of the network
 * name is stripped off and assumed to be the destination user name --
 * it does not participate in the optimization.  As an example, the
 * name "research!vax135!research!ucbvax!bill" becomes, tokenized,
 * "r!x!r!v!" and "bill"  A low level routine, optim1, fixes up the
 * network part (eg, "r!x!r!v!"), then we convert back to network
 * machine names and tack the user name on the end.
 *
 * The result of this is copied into the parameter "name"
 */

optim(net, name)
	char net[], name[];
{
	char netcomp[BUFSIZ], netstr[40], xfstr[40];
	register char *cp, *cp2;
	register int c;

	strcpy(netstr, "");
	cp = net;
	for (;;) {
		/*
		 * Rip off next path component into netcomp
		 */
		cp2 = netcomp;
		while (*cp && !any(*cp, metanet))
			*cp2++ = *cp++;
		*cp2 = 0;
		/*
		 * If we hit null byte, then we just scanned
		 * the destination user name.  Go off and optimize
		 * if its so.
		 */
		if (*cp == 0)
			break;
		if ((c = netlook(netcomp, *cp)) == 0) {
			printf("No host named \"%s\"\n", netcomp);
err:
			strcpy(name, net);
			return;
		}
		stradd(netstr, c);
		stradd(netstr, *cp++);
		/*
		 * If multiple network separators given,
		 * throw away the extras.
		 */
		while (any(*cp, metanet))
			cp++;
	}
	if (strlen(netcomp) == 0) {
		printf("net name syntax\n");
		goto err;
	}
	optim1(netstr, xfstr);

	/*
	 * Convert back to machine names.
	 */

	cp = xfstr;
	strcpy(name, "");
	while (*cp) {
		if ((cp2 = netname(*cp++)) == NOSTR) {
			printf("Made up bad net name\n");
			goto err;
		}
		strcat(name, cp2);
		stradd(name, *cp++);
	}
	strcat(name, netcomp);
}

/*
 * Take a string of network machine id's and separators and
 * optimize them.  We process these by pulling off maximal
 * leading strings of the same type, ppassing these to the appropriate
 * optimizer and concatenating the results.
 */

#define	IMPLICIT	1
#define	EXPLICIT	2

optim1(netstr, name)
	char netstr[], name[];
{
	char path[40], rpath[40];
	register char *cp, *cp2;
	register int tp, nc;

	cp = netstr;
	prefer(cp);
	strcpy(name, "");
	while (*cp != 0) {
		strcpy(path, "");
		tp = ntype(cp[1]);
		nc = cp[1];
		while (*cp && tp == ntype(cp[1])) {
			stradd(path, *cp++);
			cp++;
		}
		switch (tp) {
		default:
			strcpy(rpath, path);
			break;

		case IMPLICIT:
			optimimp(path, rpath);
			break;

		case EXPLICIT:
			optimex(path, rpath);
			break;
		}
		for (cp2 = rpath; *cp2 != 0; cp2++) {
			stradd(name, *cp2);
			stradd(name, nc);
		}
	}
	optiboth(name);
	prefer(name);
}

/*
 * Return the type of the separator --
 *	IMPLICIT -- if for a smart, implicitly routed network
 *	EXPLICIT -- if for a dumb, explicitly routed neetwork.
 *	0 if we don't know.
 */

ntype(nc)
	register int nc;
{

	switch (nc) {
	case '^':
	case '!':
		return(EXPLICIT);

	case ':':
		return(IMPLICIT);

	default:
		return(0);
	}
	/* NOTREACHED */
}
		
/*
 * Do name optimization for an explicitly routed network (eg BTL network).
 */

optimex(net, name)
	char net[], name[];
{
	register char *cp, *rp;
	register int m;
	char *rindex();

	strcpy(name, net);
	cp = name;
	if (strlen(cp) == 0)
		return(-1);
	if (cp[strlen(cp)-1] == LOCAL) {
		name[0] = 0;
		return(0);
	}
	for (cp = name; *cp; cp++) {
		m = *cp;
		rp = rindex(cp+1, m);
		if (rp != NOSTR)
			strcpy(cp, rp);
	}
	return(0);
}

/*
 * Do name optimization for implicitly routed network (eg, arpanet,
 * Berkeley network)
 */

optimimp(net, name)
	char net[], name[];
{
	register char *cp;
	register int m;

	cp = net;
	if (strlen(cp) == 0)
		return(-1);
	m = cp[strlen(cp) - 1];
	if (m == LOCAL) {
		strcpy(name, "");
		return(0);
	}
	name[0] = m;
	name[1] = 0;
	return(0);
}

/*

 * Perform global optimization on the given network path.
 * The trick here is to look ahead to see if there are any loops
 * in the path and remove them.  The interpretation of loops is
 * more strict here than in optimex since both the machine and net
 * type must match.
 */

optiboth(net)
	char net[];
{
	register char *cp, *cp2;
	char *rpair();

	cp = net;
	if (strlen(cp) == 0)
		return;
	if ((strlen(cp) % 2) != 0) {
		printf("Strange arg to optiboth\n");
		return;
	}
	while (*cp) {
		cp2 = rpair(cp+2, *cp, ntype(cp[1]));
		if (cp2 != NOSTR)
			strcpy(cp, cp2);
		cp += 2;
	}
}

/*
 * Find the rightmost instance of the given (machine, type) pair.
 */

char *
rpair(str, mach, t)
	char str[];
{
	register char *cp, *last;

	last = NOSTR;
	while (*cp) {
		if (*cp == mach /* && ntype(cp[1]) == t */ )
			last = cp;
		cp += 2;
	}
	return(last);
}

/*
 * Change the network separators in the given network path
 * to the preferred network transmission means.
 */

prefer(name)
	char name[];
{
	register char *cp;
	register int state, n;

	state = LOCAL;
	for (cp = name; *cp; cp += 2) {
		n = best(state, *cp);
		if (n)
			cp[1] = n;
		state = *cp;
	}
}

/*
 * Return the best network separator for the given machine pair.
 */

struct netorder {
	short	no_stat;
	char	no_char;
} netorder[] = {
	CN,	':',
	AN,	'@',
	SN,	':',
	BN,	'!',
	-1,	0
};

best(src, dest)
{
	register int dtype, stype;
	register struct netorder *np;

	stype = nettype(src);
	dtype = nettype(dest);
	if (stype == 0 || dtype == 0) {
		printf("ERROR:  unknown internal machine id\n");
		return(0);
	}
	if ((stype & dtype) == 0) {
		printf("No way to get from \"%s\" to \"%s\"\n", 
		    netname(src), netname(dest));
		return(0);
	}
	np = &netorder[0];
	while ((np->no_stat & stype & dtype) == 0)
		np++;
	return(np->no_char);
}

/*
 * Add a single character onto a string.
 */

stradd(str, c)
	register char *str;
	register int c;
{

	str += strlen(str);
	*str++ = c;
	*str = 0;
}
