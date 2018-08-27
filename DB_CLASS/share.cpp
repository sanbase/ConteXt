/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:share.cpp
*/
#include "StdAfx.h"
#include "CX_BASE.h" 

#ifndef WIN32 
#include <sys/mman.h> 
#include <sys/ipc.h> 
#ifdef SYSV 
#include <sys/shm.h> 
#endif 
#include <sys/wait.h> 
#include <sys/time.h> 
#include <sys/file.h> 
#include <signal.h> 

static int error;

static void mode(int i) 
{ 
	switch(i) 
	{ 
	case SIGUSR1: 
		error=0; 
		break; 
	case SIGCHLD: 
		error=10; 
		break; 
	default: 
		error=1; 
		break; 
	} 
} 

static void del_proc(pid_t *process) 
{ 
	int i; 
	if(!*process) 
		return; 
	signal(SIGCHLD,mode); 
	kill(*process,SIGTERM); 
	waitpid(*process,&i,0); 
	error=0; 
	*process=0; 
} 

static int HandShake(pid_t *pid, int *io) 
{ 
	char ch=1; 

	if(*pid<=0) 
	{ 
		return(1); 
	} 
	error=0; 
	kill(*pid,SIGUSR1); 
	char i=read(io[0],&ch,1); 
	if(i<0 || error) 
	{ 
		if(error!=10) 
			del_proc(pid); 
		error=0; 
		*pid=0; 
		return(-1); 
	} 
	return(0); 
} 

static pid_t exe_pro(char *cmd,char *name,char *sh_name,int *io) 
{ 
	int i; 

	if( (i=vfork()) == 0 ) 
	{ 
		signal(SIGINT,SIG_IGN); 
		signal(SIGTERM,SIG_DFL); 
		signal(SIGQUIT,SIG_DFL); 
		if(io!=NULL) 
		{ 
			dup2(io[1],1); 

			close(io[0]); 
			close(io[1]); 
		} 
		execlp(cmd,name,sh_name,(char *)0); 
		exit(0); 
	} 
	return(i); 
} 

void CX_BASE::Run_Method() 
{ 
	if(p_method>0) 
		return; 
	char *name=(char *)malloc(strlen(__name)+strlen(MODULE)+2); 
	sprintf(name,"%s/%s",__name,MODULE); 
	if(!access(name,X_OK)) 
		p_method=Run(name,method_io); 
	else    p_method=-1; 
	free(name); 
} 

/** создание канала */ 
int CX_BASE::Run(char *prog, int *io) 
{ 
	pid_t pid=0; 
	struct stat st; 

	if(!*prog || access(prog,X_OK) || shmid==0) 
	{ 
		return(-1); 
	} 
	stat(prog,&st); 
	if((st.st_mode&S_IFMT)==S_IFDIR)  /* Probably it's cx4 or cx3 */ 
	{ 
		return(-1); 
	} 
	alarm(0);  
	error=0; 
	if(context->v==39) 
		return(-1); 

	struct timeval timeout; 
	fd_set read_set; 
	if(pipe(io)) 
	{ 
		return(-1); 
	} 
	FD_ZERO(&read_set); 
	FD_SET(io[0], &read_set); 
	bzero(&timeout,sizeof timeout); 
	timeout.tv_sec=5; 

	pid=exe_pro(prog,__name,sh_name,io); 

	if(select(io[0]+1,&read_set,NULL,NULL,&timeout)==0) 
		goto ERROR; 

	char ch; 
	read(io[0],&ch,1); 
	close(io[1]); 
	if(ch=='0') 
		error=1; 
	if(error) 
	{ 
ERROR: 
		if(pid) 
			del_proc(&pid); 
		if(io[0]) 
			close(io[0]); 
		if(io[1]) 
			close(io[1]); 
		return(0); 
	} 
	return(pid); 
} 

#else
#include <process.h>
//#define SHMEM 
static void (* M_Virtual)   (Methods *,long,struct sla *);
static int  (* M_Action)    (Methods *,int);
static int  (* M_Write_Cadr)(Methods *);
static int  (* M_Write_Slot)(Methods *,char *);
static	HINSTANCE hinstLib;
void CX_BASE::Run_Method()
{
	if(p_method>0)
		return;
	char *name=(char *)malloc(strlen(__name)+strlen(MODULE)+2);
	sprintf(name,"%s/%s",__name,MODULE);
	if(!access(name,4))
	{
		hinstLib = LoadLibrary(name);
		if(hinstLib==NULL)
		{
			p_method=-1;
			return;
		}
		Methods *(*M_Create)(CX_BASE *);
		M_Create=(Methods * (*)(CX_BASE *))GetProcAddress(hinstLib,"Create");
		method=M_Create(this);
		if(method!=NULL)
		{
			p_method=1;
			M_Virtual=   (void(*) (Methods *,long,struct sla *))GetProcAddress(hinstLib,"Virtual");
			M_Action=    (int (*) (Methods *,int   ))GetProcAddress(hinstLib,"Action");
			M_Write_Cadr=(int (*) (Methods *       ))GetProcAddress(hinstLib,"Write_Cadr");
			M_Write_Slot=(int (*) (Methods *,char *))GetProcAddress(hinstLib,"Write_Slot");
			p_method=1;
		}
		else
		{
			p_method=-1;
		}
	}
	else    p_method=-1;
	free(name); 
}
int CX_BASE::Ask_Method()
{
	if(p_method<0)
		return(-1);
	flush_sb();
	cadr_record=share->cadr_record;

	int i=share->cmd;
	int act=0;
	struct sla sla[SLA_DEEP];
	share->cmd=0;
	char line[LINESIZE];
	switch(i)
	{
		case CHK_LINE:
			bcopy(share->output,line,LINESIZE);
			bzero(share->output,LINESIZE);
			if(!(share->ret=M_Write_Slot(method,line)))
				*share->output=0;
			else if(!strcmp(line,share->output))
				*share->output=0;
			break;
		case CHK_CADR:
			bzero(share->output,LINESIZE);
			share->ret=M_Write_Cadr(method);
			break;
		case CHK_ACT:
			bcopy(share->output,&act,sizeof act);
			bzero(share->output,LINESIZE);
			share->ret=M_Action(method,act);
			break;
		case VIRTUAL:
			bcopy(share->slot.sla,sla,sizeof sla);
			M_Virtual(method,share->record,sla);
			break;
		case NEWINDEX:
			Erase_Index(share->ret);
			Link_Index(share->output,share->ret);
			break;
	}
	return(0);
}

#endif 
#include <sys/times.h>

int  CX_BASE::Map() 
{ 
	int i=0;
#ifndef WIN32 
	if(!*sh_name) 
	{ 
		char *ch=strrchr(__name,'/');
		if(ch==NULL)
			ch=__name;
		else
		ch++;

		sprintf(sh_name,"%s/Mem:%s pid=%d.XXXXXX",TEMPDIR,ch,getpid());
		if((i=mkstemp(sh_name))<0) 
			return(i); 
		close(i); 
	}
#else
	if(!*sh_name)
	{
		sprintf(sh_name,"%s/mem=%d.XXXXXX",TEMPDIR,getpid());
		_mktemp(sh_name);
	}
#endif 
	i=Map(sh_name); 
	return(i); 
} 

int CX_BASE::Map(char *name) 
{ 
	int length=len_cadr+sizeof (struct share); 

	if(*sh_name && strcmp(name,sh_name))
	{
		UnMap();
	}

	if(name!=sh_name)
		strcpy(sh_name,name); 
	char *ch=sh_name; 
	ch=strrchr(sh_name,'=');
	if(ch!=NULL)
		ch++;

#ifndef WIN32 
	if(!(owner=(atoi(ch)==getpid())))
		owner=access(sh_name,R_OK);

	if((shmid=open(sh_name,O_RDWR|O_CREAT,0600))<0) 
	{
		goto LOCAL; 
	}
	if(owner) 
	{ 
#ifndef SPARC 
		if(flock(shmid,LOCK_EX|LOCK_NB)) 
#else 
			if(lockf(shmid,F_TLOCK,0)) 
#endif 
			{ 
				close(shmid); 
				if(owner) 
				{ 
					unlink(sh_name); 
					*sh_name=0; 
				} 
				goto LOCAL; 
			} 
		ftruncate(shmid,length); 
#ifndef SPARC 
		flock(shmid,LOCK_SH); 
#endif 
	} 

	share=(struct share *)mmap(0,length,PROT_READ|PROT_WRITE,MAP_SHARED,shmid,0); 
	if(share==(void *)-1) 
	{ 
		close(shmid); 
		if(owner) 
		{ 
			unlink(sh_name);
			*sh_name=0; 
		} 

		goto LOCAL; 
	} 
	cadr=(char *)(share+1); 
	return(0);
#else
#ifdef SHMEM
	shmid=NULL;
 	owner=(atoi(ch)==_getpid());
	// create anonymous file mapping
	shmid=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,length,sh_name);
	if(shmid==NULL)
		goto LOCAL;
	// map buffer to local process space
	share=(struct share *)MapViewOfFile(shmid,FILE_MAP_WRITE,0,0,length);
	if(share==NULL)
		goto LOCAL;
	if(share==(void *)-1)
	{
		if(owner)
		{
			*sh_name=0;
		}
		goto LOCAL;
	}
	cadr=(char *)(share+1);
	return(0);
#endif
#endif

LOCAL:
	share=(struct share *)calloc(length,1);
	cadr=(char *)(share+1);
	*sh_name=0;
#ifndef WIN32
	if(shmid>0)
		close(shmid);
	shmid=0;
#endif
	owner=0; 
	return(-1); 
} 

void CX_BASE::UnMap() 
{
#ifdef WIN32
	if(share)
		free(share);
#else
#ifdef SHMEM
	if(owner>0)
		UnmapViewOfFile(share); // unmap file buffer
	else
		if(share)
			free(share);
#endif
	if(shmid) 
	{ 
		if(p_method>0 && owner>0) 
		{ 
			error=0; 
			close(method_io[0]); 
			close(method_io[1]); 
			del_proc(&p_method); 
		} 
		p_method=0; 
		if(owner>0) 
		{ 
			if(!access(sh_name,F_OK))
			{ 
				munmap((char *)share,len_cadr+sizeof (struct share)); 
				unlink(sh_name); 
			} 
#ifdef SYSV 
			else 
			{ 
				shmdt(share); 
				shmctl(shmid,IPC_RMID,NULL); 
			} 
#endif 
			*sh_name=0; 
		} 
		close(shmid); 
		shmid=0; 
	} 
	else
	{ 
		if(share) 
			free(share); 
	}
#endif 
	share=NULL; 
	cadr=NULL; 
} 

void CX_BASE::Inherit(CX_BASE *std) 
{ 
	if(strcmp(__name,std->__name)) 
		return; 
	p_method=std->p_method; 
	bcopy(std->method_io,method_io,sizeof method_io); 
	share=std->share; 
	owner=-1; 
	cadr_record=std->cadr_record; 
	if(share!=NULL && std->share!=NULL) 
		bcopy(std->share,share,sizeof (struct share)); 
} 

int CX_BASE::Get_Virtual(long t_page,int field,char *&str) 
{ 
	struct sla sla[SLA_DEEP]; 

	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Get_Virtual(t_page,sla,str)); 
} 

int CX_BASE::Get_Virtual(long record,struct sla *sla,char *&slot) 
{ 
	int len=0; 
	char std[(sizeof share->output)]; 
	struct tag_descriptor slot_std; 

	if(p_method<0) 
		return(-1); 
	if(share==NULL)
		Map();
	bcopy(share->output,std,sizeof std); 
	bcopy(&share->slot,&slot_std,sizeof slot_std); 
	if(p_method==0) 
	{ 
		Run_Method(); 
		if(p_method<=0) 
		{ 
			p_method=-1; 
			return(-1); 
		} 
	} 
	share->record=record; 
	bzero(share->slot.sla,sizeof(share->slot.sla)); 
	bcopy(sla,share->slot.sla,sizeof share->slot.sla); 

	bzero(share->output,sizeof (share->output)); 
	share->cadr_record=cadr_record; 
	share->cmd=VIRTUAL;
#ifndef WIN32
	if(HandShake(&p_method,method_io)<0)
#else
	if(Ask_Method()<0)
#endif
		p_method=-1; 
	if(slot!=NULL) 
		free(slot); 
	slot=(char *)malloc(len=strlen(share->output)+1); 
	bcopy(share->output,slot,len); 
	bcopy(std,share->output,sizeof share->output); 
	bcopy(&slot_std,&share->slot,sizeof slot_std); 
	return(len); 
} 

void CX_BASE::Get_Check() 
{ 
	if(p_method<0) 
		goto ERR; 
	if(p_method==0) 
	{ 
		Run_Method(); 
		if(p_method<=0) 
			goto ERR; 
	} 
	share->cadr_record=cadr_record;
#ifndef WIN32
	if(HandShake(&p_method,method_io)<0)
#else
	if(Ask_Method()<0)
#endif
	{ 
ERR: 
		p_method=-1; 
		bzero(share->output,sizeof (share->output)); 
		share->ret=0; 
	} 
} 
