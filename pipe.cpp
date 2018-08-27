/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:pipe.cpp
*/
#include "stdafx.h" 
#include "CX_pipe.h" 
#include <sys/wait.h> 
#include <sys/file.h> 
 
#define CHECDIR  "_Chec/" 
#define SHOWDIR  "_Show/" 
 
#define BSD 
extern Terminal *term; 
 
static int error; 
CX3::CX3(CX_BROWSER *cx) 
{ 
	if(cx==NULL || cx->db==NULL) 
		throw 1; 
	if(cx->db->context->pswd!=CXKEY3 || cx->db->p_method>0) 
		throw 1; 
	cx5=cx; 
	len=0; 
	for(int i=1;i<=cx5->db->Num_Fields();i++) 
	{ 
		int j=cx5->db->Field_Descr(i)->l; 
		if(j<4) 
			j=4; 
		len+=j; 
	} 
	shmchain=NULL; 
	key_t uid=(key_t)getpid(); 
	length=sizeof (struct cx3share)+len+(term->l_x()+1)*(term->l_y()+1)*sizeof(struct pics); 
	if((shmid=shmget(uid,length, IPC_CREAT|SHM_R|SHM_W))<0) 
		throw 1; 
	if((shbuf=(struct cx3share *)shmat(shmid,0,0))==(void*)-1) 
		throw 1; 
	cadr=(char *)shbuf + sizeof (struct cx3share) + (cx5->db->key39?4:0); 
 
	use_check=Creat_pipe(uid,cx5->db->Name_Base(),1,len); 
	use_show =Creat_pipe(uid,cx5->db->Name_Base(),2,len); 
} 
 
CX3::~CX3() 
{ 
	struct shmid_ds shmds; 
	if(use_check) 
		del_proc(&use_check); 
	if(use_show) 
		del_proc(&use_show); 
	if(shbuf) 
	{ 
		munmap(shbuf,length); 
		shmdt((char *)shbuf); 
		shmctl(shmid,IPC_RMID,&shmds); 
	} 
	shbuf=NULL; 
} 
 
void CX3::copy_cadr(long record) 
{ 
	bzero(cadr,len); 
	int s1=0,s2=0; 
	for(int i=1;i<=cx5->db->Num_Fields();i++) 
	{ 
		int j=cx5->db->Field_Descr(i)->l; 
		if(record==cx5->db->cadr_record) 
			bcopy(cx5->db->cadr+s1,cadr+s2,j); 
		else 
		{ 
			char *ch=NULL; 
			cx5->db->Read(record,i,ch); 
			if(ch!=NULL) 
			{ 
				bcopy(ch,cadr+s2,j); 
				free(ch); 
			} 
		} 
		s1+=j; 
		if(j<4) 
			j=4; 
		s2+=j; 
	} 
 
} 
void CX3::restore_cadr(long record) 
{ 
	int s1=0,s2=0; 
	for(int i=1;i<=cx5->db->Num_Fields();i++) 
	{ 
		int j=cx5->db->Field_Descr(i)->l; 
		if(record==cx5->db->cadr_record) 
			bcopy(cadr+s2,cx5->db->cadr+s1,j); 
		s1+=j; 
		if(j<4) 
			j=4; 
		s2+=j; 
	} 
 
} 
 
int CX3::Check_Line(long record,struct tag *tag,char *str) 
{ 
	int i; 
	if(!use_check) 
		return(0); 
	memcpy(shbuf->mess,str,256-1-128); 
	shbuf->mess[256-128]=0; 
 
	shbuf->pt   =tag->des.sla->n; 
	shbuf->ret  =tag->des.sla->m; 
	shbuf->page =record; 
 
	copy_cadr(record); 
	i=Connect(&use_check,SIGUSR1); 
	restore_cadr(record); 
 
	strcpy(cx5->db->share->output,shbuf->mess); 
	if(tag->des.sla->n!=shbuf->pt) 
		cx5->db->share->slot.sla->n=shbuf->pt; 
	if(i) 
		return(0); 
 
	cx5->db->share->color.bg=(shbuf->ret>>4)&0xf; 
	cx5->db->share->color.fg=shbuf->ret&0xf; 
 
	return(shbuf->ret&=0xff); 
} 
 
int CX3::Check_Cadr(long record,struct tag *tag,int del) 
{ 
	int i; 
	if(!use_check) 
		return(0); 
	copy_cadr(record); 
	shbuf->page=record; 
	shbuf->pt  =tag->des.sla->n; 
 
	if(del>0) shbuf->page=-shbuf->page; 
	if(del<0) shbuf->pt=0; 
 
	i=Connect(&use_check,SIGUSR2); 
	if(tag->des.sla->n!=shbuf->pt) 
		cx5->db->share->slot.sla->n=shbuf->pt; 
	if(i) 
		return(0); 
 
	if(shbuf->ret&0200) 
		cx5->Refresh(1); 
 
	cx5->db->share->color.bg=(shbuf->ret>>4)&0xf; 
	cx5->db->share->color.fg=shbuf->ret&0xf; 
 
	return(shbuf->ret&=0xff); 
} 
 
int CX3::Get_Show(long record,struct tag *tag,char *&str) 
{ 
	unsigned int ret_std=shbuf->ret,color; 
	struct xy AXY; 
 
	if(!use_show) 
		return(0); 
	*shbuf->mess=0; 
	shbuf->pt   =tag->des.sla->n; 
	shbuf->ret  =tag->des.sla->m; 
	shbuf->page =record; 
 
	AXY.n=tag->des.sla[0].n; 
	AXY.m=tag->des.sla[1].n; 
	AXY.t=tag->des.sla[2].n; 
	AXY.e=tag->des.sla[0].m; 
	AXY.f=tag->des.sla[1].m; 
 
	AXY.x=tag->des.x; 
	AXY.y=tag->des.y; 
	AXY.a=tag->des.atr; 
	AXY.l=tag->des.l; 
 
	AXY.p=0; 
 
	if(cx5->db->key39) 
	{ 
		if(tag->des.sla->n==cx5->tags[cx5->act_field].des.sla->n) 
			sprintf(&(shbuf->mess[1]),"%hi",tag->des.sla->n); 
		else 
			sprintf(&(shbuf->mess[1]),"0"); 
	} 
	else    shbuf->mess[1]=tag->des.l; 
 
	memcpy(&(shbuf->mess[32]),&AXY,sizeof(AXY)); 
	copy_cadr(record); 
 
	if(Connect(&use_show,SIGUSR1)) 
	{ 
		use_show=0; 
		return(-1); 
	} 
	str=(char *)realloc(str,strlen(shbuf->mess)+1); 
	strcpy(str,shbuf->mess); 
	*shbuf->mess=0; 
	color=shbuf->ret>>8; 
	shbuf->ret=ret_std; 
	return(color); 
} 
 
static void mode(int i) 
{ 
	switch(i) 
	{ 
	case SIGUSR1: 
		error=0; break; 
	case SIGUSR2: 
		error=-1; break; 
	default: 
		error=i; break; 
	} 
} 
 
void CX3::del_proc(pid_t *process) 
{ 
	if(!*process) 
		return; 
	kill(*process,SIGTERM); 
	waitpid(*process,(int *)0,0); 
	*process=0; 
} 
 
 
int CX3::Connect(pid_t *pid,int sign) 
{ 
	struct sigaction s,os1; 
	sigset_t mask; 
 
	if(!*pid) 
		return(1); 
 
	bzero(&s,sizeof(struct sigaction)); 
	s.sa_flags=SA_RESTART; s.sa_handler=mode; 
	sigaction(SIGUSR1,&s,&os1); 
 
	sigemptyset(&mask); 
	sigaddset(&mask,SIGUSR1); 
	sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL); 
 
	kill(*pid,sign); 
 
	sigemptyset(&mask); 
	sigsuspend((sigset_t *)&mask); 
 
	sigaction(SIGUSR1,&os1,NULL); 
 
	sigaddset(&mask,SIGUSR1); 
	sigprocmask(SIG_UNBLOCK,(sigset_t *)&mask,NULL); 
	if(error) 
	{ 
		del_proc(pid); 
		error=0; 
		return(-1); 
	} 
	return(0); 
} 
 
static pid_t exe_pro(char *cmd,char *name,char *uid,char *len) 
{ 
	int i; 
	if( (i=fork()) == 0 ) 
	{ 
		signal(SIGINT,SIG_DFL); 
		signal(SIGTERM,SIG_DFL); 
		signal(SIGQUIT,SIG_DFL); 
		execlp(cmd,name,uid,len,(char *)0); 
		exit(0); 
	} 
	return(i); 
} 
 
pid_t CX3::Creat_pipe(key_t UID, char *folder,short N,int len) 
{ 
	sigset_t mask; 
	pid_t pid; 
	char uid[11]; 
	char length[11]; 
	char *prog,*name; 
	struct sigaction s,os; 
 
	name=(char *)malloc(strlen(folder)+64); 
	if(N==1) 
		full(folder,CHECDIR,name); 
	else 
		full(folder,SHOWDIR,name); 
	prog=Select_From_Dir(name,if_exec,message(19),0); 
	free(name); 
 
	if(prog == NULL)
		return(0); 
	if(!*prog || access(prog,01))
	{
		free(prog);
		return(0);
	}
 
	alarm(0);  error=0; 
	sprintf(uid,"%ld",UID); 
	sprintf(length,"%d",len); 
 
	bzero(&s,sizeof(struct sigaction)); 
	s.sa_flags=SA_RESTART; s.sa_handler=mode; 
	sigaction(SIGUSR1,&s,&os); 
 
	sigemptyset(&mask); 
	sigaddset(&mask,SIGUSR1); 
	sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL); 
 
	signal(SIGALRM,mode); 
	sigemptyset(&mask); 
	alarm(5); 
 
	pid=exe_pro(prog,folder,uid,length); 
 
	sigsuspend((sigset_t *)&mask); 
 
	alarm(0); 
	sigaction(SIGUSR1,&os,NULL); 
 
	sigemptyset(&mask); 
	sigaddset(&mask,SIGUSR1); 
	sigprocmask(SIG_UNBLOCK,(sigset_t *)&mask,NULL); 
	if(error) 
	{ 
		if(pid) 
		{ 
			kill(pid,SIGTERM); 
			waitpid(pid,(int *)0,0); 
		} 
		return(0); 
	} 
	return(pid); 
} 
 
shmdesc *CX3::shmlookup( int shmid ) 
{ 
	shmdesc *sdp; 
 
	for ( sdp = shmchain; sdp != NULL; sdp = sdp->next ) 
	{ 
		if ( sdp->shmid == shmid ) 
			return(sdp); 
	} 
	return(NULL); 
} 
 
int CX3::shmget( key_t key, int size, int shmflg ) 
{ 
	char tbuf[1000]; 
	int cpid=0, len, shmid; 
	shmdesc *sdp; 
 
	sprintf(tbuf,"/tmp/.x%ld.shm",key); 
	shmid = open(tbuf,O_RDWR,0); 
	if ( shmid == -1 ) 
	{ 
		char *clrbuf; 
		if ( (shmflg & IPC_CREAT) == 0 ) 
			return(-1); 
		shmid = open(tbuf,O_RDWR | O_CREAT, 0600); 
		if ( shmid < 0 ) 
			return(-1); 
		if ( flock( shmid, LOCK_EX | LOCK_NB ) != 0 ) 
		{ 
			close(shmid); 
			return(-1); 
		} 
		cpid = getpid(); 
		len = 0; 
		clrbuf = (char *)calloc(1024,1); 
		while ( len < size ) 
		{ 
			int alen, wlen; 
			wlen = size - len; 
			if ( wlen > 1024 ) 
				wlen = 1024; 
			alen = write(shmid,clrbuf,wlen); 
			if ( alen < 0 ) 
			{ 
				free(clrbuf); 
				return(-1); 
			} 
			else if ( alen == 0 ) 
				abort(); 
			 else 
				len += alen; 
		} 
		free(clrbuf); 
		flock( shmid, LOCK_SH ); 
	} 
	else if ( (shmflg & IPC_CREAT) && (shmflg & IPC_EXCL) ) 
	{ 
		cpid = -1; 
		return(-1); 
	} 
 
	sdp = (shmdesc *)calloc(sizeof(shmdesc),1); 
	sdp->next = shmchain; 
	sdp->shmid = shmid; 
	sdp->cpid = cpid; 
	sdp->key = key; 
	sdp->mode = shmflg & 0777; 
	sdp->fname = strdup(tbuf); 
	sdp->segsz = size; 
 
	shmchain = sdp; 
 
	return(shmid); 
} 
 
void *CX3::shmat( int shmid, void *shmaddr, int shmflg ) 
{ 
	shmdesc *sdp; 
	void *addr; 
 
	sdp = shmlookup( shmid ); 
	if ( sdp == NULL ) 
		return((void *)-1); 
	if ( shmaddr == 0 ) 
		addr = mmap( 0, sdp->segsz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, shmid, 0 ); 
	else 
		addr = mmap( shmaddr, sdp->segsz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED | MAP_FIXED, shmid, 0 ); 
	return(addr); 
} 
 
int CX3::shmctl(int shmid,int cmd,struct shmid_ds *buf) 
{ 
	shmdesc *sdp; 
	struct stat sbuf; 
 
	sdp = shmlookup( shmid ); 
	if ( sdp == NULL ) 
		return(-1); 
 
	switch (cmd) 
	{ 
	case IPC_RMID: 
		close(shmid); 
		unlink(sdp->fname); 
		free(sdp->fname); 
		free(sdp); 
		return(0); 
	case IPC_STAT: 
		if ( fstat(shmid,&sbuf) < 0 ) 
			return(-1); 
		buf->shm_perm.uid = sbuf.st_uid; 
		buf->shm_perm.gid = sbuf.st_gid; 
		buf->shm_perm.cuid = sbuf.st_uid; 
		buf->shm_perm.cgid = sbuf.st_gid; 
		buf->shm_perm.mode = sdp->mode; 
		buf->shm_segsz = sdp->segsz; 
		buf->shm_lpid = -1;     /* how do I compute this? */ 
		buf->shm_cpid = sdp->cpid; 
		buf->shm_nattch = 2;    /* how do I compute this? */ 
		buf->shm_atime = time(0); /* how do I compute this? */ 
		buf->shm_dtime = time(0); /* how do I compute this? */ 
		buf->shm_ctime = time(0); /* how do I compute this? */ 
		return(0); 
	default: 
		return(-1); 
	} 
 
} 
