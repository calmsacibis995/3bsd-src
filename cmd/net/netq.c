/* netq - print the netq send queue */
/* netq [-a] [mach] */
/* must be setuid root */

# include "defs.h"
# define STSIZE 150
# define SCREEN 21

static FILE *df, *look;
static char jname[16], printlong;
static struct table {
	char name[16];
	long filesize;
	} stack[STSIZE], temp;
static int stptr = 0;
static char mach,visit[26];
static char netcmd1[] =	NETCMD1;
static int hisuid,sumj;
static long sumb;
static struct stat statbuf;
static struct direct dirbuf;

main(argc,argv)
  char **argv; {
	char outbuf[BUFSIZ];
	int i;
	setbuf(stdout,outbuf);
	hisuid = getuid();
	hisuid = uidmask(hisuid);
	if(stat(netcmd,&statbuf) >= 0)
		if((statbuf.st_mode & 07) == 0){
			printf("Network is down\n");
			exit(1);
			}
	else if(stat(netcmd1,&statbuf) >= 0)
		if((statbuf.st_mode & 07) == 0){
			printf("Network is down\n");
			exit(1);
			}
	while(argc > 1){
		switch(argv[1][0]){
		case '-': printlong++; break;
		default: mach = lookup(argv[1]);
			if(mach > 0 && machtype[mach-'a'] == 0)mach = 0;
			break;
		}
		argc--, argv++;
		}
	if(mach){
		mach = gothru(local,mach); /* list to directly conn. machine */
		if(mach == 0){
			fprintf(stderr,"That machine not directly connected.\n");
			exit(1);
		}
		senddir[strlen(senddir)-1] = mach;
		if(chdir(senddir) < 0){
			perror(senddir);
			exit(1);
		}
		pdir(senddir);
	}
	else for(i = 'a'; i <= 'z'; i++)
		if((mach = gothru(local,i)) && !visit[mach - 'a']){
			visit[mach - 'a'] = 1;
			senddir[strlen(senddir)-1] = mach;
			if(chdir(senddir) < 0)continue;
			pdir(senddir);
			printf("---\n");
			}
	fflush(stdout);
	}
static pdir(str)
  char *str; {
	int i;
	char more = 0, *cp;
	int (*compar)();
	df = fopen(str,"r");
	if(df == NULL){
		perror(str);
		exit(1);
		}
	stptr = 0;
	while(fread(&dirbuf,1,sizeof dirbuf,df)==sizeof dirbuf){
		if(dirbuf.d_ino == 0 
		|| dirbuf.d_name[0] != 'c'
		|| dirbuf.d_name[1] != 'f'
		|| stat(dirbuf.d_name,&statbuf) < 0)
			continue;
		if(mach != dirbuf.d_name[2])continue;
		dirbuf.d_name[0] = 'd';
		if(stat(dirbuf.d_name,&statbuf) < 0)continue;

		if(!insert(dirbuf.d_name,getsize(&statbuf))){
			more++;
			break;
			}
		}
	if(stptr == 0){
		printf("Network queue to/thru %s is empty.\n",longname(mach));
		return;
		}
	cp = (char *)&(stack[0].name[0]);
	sort(cp,stptr,sizeof temp,compar);
	printf("Network queue to/thru %s:\n",longname(mach));
	printf(
	"From       To           Len  Code   Time          Command\n");
	for(i = 0; i < stptr; i++){ /* screen size */
		strcpy(jname,stack[i].name);
		jname[0] = 'd';
		if(stat(jname,&statbuf) < 0)
			continue;
		if(printlong || guid(statbuf.st_uid,statbuf.st_gid) == hisuid)
			process();
		else summarize(i);
		}
	printsum();
	if(more)printf("   ... more ...\n");
	}
summarize(i){
	sumb += stack[i].filesize;
	sumj++;
	}
printsum(){
	if(sumj != 0){
		printf("%d request%s, %ld bytes\n",
			sumj,(sumj > 1 ? "s" : ""),sumb);
		sumj = 0;
		sumb = 0L;
		}
	}
process(){
	int code, tm, fm;
	char login[NS], passwd[FNS], infile[FNS], ttystr[20];
	char outfile[FNS], resp[FNS];
	char localname[NS], cmd[BUFSIZ];
	char realcmd[BUFSIZ];
	char b1[10], b2[10];
	static char parmlist[PARMLIST];
	char *cp;
	int c, i;
	char *s;
	printsum();
	look = fopen(jname,"r");
	if(look == NULL)
		return;
	code = tm = login[0] = passwd[0] = infile[0] = 0;
	outfile[0] = resp[0] = localname[0] = 0;
	cmd[0] = 0;
	code = ngetc();
	if(code == 0)return;
	tm = ngetc();
	fm = ngetc();	/* from machine */
	ngetc();
	ngetc();
	ngets(login,NS);
	ngets(passwd,FNS);
	ngets(infile,FNS);
	ngets(outfile,FNS);
	ngets(resp,FNS);
	ngets(localname,NS);
	if(localname[0] == 0)strcpy(localname,"Internal");
	for(i=0;i<20;i++)ttystr[i] = 0;
	ngets(ttystr,20);
	expandcc(ttystr);
	ngetc();		/* cflag */
	ngets(b1,10);
	ngets(parmlist,PARMLIST);		/* jobno */
	parseparmlist(parmlist);
	ngets(b2,10);		/* unused */
	s = cmd;
	while((c = getc(look)) != EOF && c != '\n'){
		if(c == '\\')c = getc(look);
		*s++ = c;
		}
	*s = 0;
	s = realcmd;
	while((c = getc(look)) != EOF && c != '\n'){
		if(c == '\\')c = getc(look);
		*s++ = c;
		}
	*s = 0;
	if(realcmd[0] == 0)strcpy(realcmd,cmd);
	fclose(look);
/*	
	i = strlen(login);
	login[i] = ')';
	login[i+1] = 0;
*/
	cp = ctime(&statbuf.st_mtime);
	cp[strlen(cp)-9] = 0;
	jname[3] = jname[2];
	printf("%c:%-8s %c:%-9s %5ld %s %s  %-.27s\n",
		fm,localname,tm,login,getsize(&statbuf),
		jname+3,cp+4,realcmd);
	}
ngetc(){
	char b[3];
	if(feof(look))return(0);
	if(fread(b,1,3,look) != 3) return(0);
	return(b[0]);
	}
/* read a string s of max length maxlen out of queue file */
ngets(s,maxlen)
  int maxlen;
  char *s; {
	int i;
	if(feof(look))return;
	for(;;){
		i = getc(look);
		if(i == EOF){
			*s = 0;
			return;
			}
		*s = i;
		if(*s == '\\')*s = getc(look);
		if(*s == ' ')break;
		if(maxlen-- > 0)s++;
		}
	*s = 0;
	getc(look);
	}
insert(f,t)
  char *f;
  long t; {
	strcpy(stack[stptr].name,f);
	stack[stptr++].filesize = t;
	return(stptr <= STSIZE);
	}
compar(a,b)
  register struct table *a,*b; {
	if(a->filesize < b->filesize)return(-1);
	if(a->filesize > b->filesize)return(1);
	return(0);
	}
sort(){		/* use this cause qsort doesn't work */
	register int i,j;
	for(i=0; i< stptr-1; i++)
		for(j=i+1;j<stptr;j++)
			if(compar(&stack[i],&stack[j]) > 0)
				swap(&stack[i],&stack[j]);
	}
swap(a,b)
  register struct table *a, *b; {
	char str[16];
	long t;
	strcpy(str,a->name);
	t = a->filesize;
	strcpy(a->name,b->name);
	a->filesize = b->filesize;
	strcpy(b->name,str);
	b->filesize = t;
	}
expandcc(s)
  char *s; {
	char w[100],*p;
	strcpy(w,s);
	p = w;
	while(*p){
		if(!isprint(*p)){
			*s++ = '^';
			*s++ = *p++ + 0140;
			}
		else *s++ = *p++;
		}
	}
