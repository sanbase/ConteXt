/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:block.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h"
#ifdef WIN32
#include <sys/locking.h>
#define F_WRLCK	_LK_NBLCK
#define F_RDLCK	_LK_RLCK
#define F_UNLCK	_LK_UNLCK
struct flock
{
	off_t	l_start;	/* starting offset */
	off_t	l_len;		/* len = 0 means until end of file */
	pid_t	l_pid;		/* lock owner */
	short	l_type;		/* lock type: read/write, etc. */
	short	l_whence;	/* type of l_start */
};
#endif
 
/* блокировка на запись */ 
int CX_BASE::Wlock(long record,int bank) 
{ 
	return(_Lock(record,F_WRLCK,bank)); 
} 
 
/* блокировка на чтение */ 
int CX_BASE::Rlock(long record,int bank) 
{ 
	if(in_memory)
		return(0);
	return(_Lock(record,F_RDLCK,bank)); 
} 
 
/* разблокировать кадр в базе */ 
int CX_BASE::Unlock(long record,int bank) 
{ 
	struct flock arg; 
	int num,i; 

#ifdef WIN32
	return(0);
#endif	
	if(in_memory)
		return(0);
	if(bank) 
	{ 
		if(!fd[1].Fd && open_FD(fd+1)<0) 
			return(-1); 
		record=0; 
	} 
	if((num=_Check_Lock(record,bank,&i))>1) 
	{ 
		if(lock.lock_str[i].num)
			lock.lock_str[i].num--;
		else 
			lock.lock_str[i].record=-1;
		return(0); 
	} 
	if(num<=0)       /* нет блокировок */ 
		return(0); 
	arg.l_whence=0; 
	arg.l_type=F_UNLCK; 
	if(record>0) 
	{ 
		arg.l_start=root_size+(off_t)(record-1)*ss.size;
		arg.l_len=ss.size; 
	} 
	else 
	{ 
		arg.l_start=0; 
		arg.l_len=root_size; 
	}
#ifdef WIN32
	lseek(bank?fd[1].Fd:fd->Fd,arg.l_start,SEEK_SET);
	_locking(bank?fd[1].Fd:fd->Fd,LK_UNLCK,arg.l_len);
#else
	fcntl(bank?fd[1].Fd:fd->Fd,F_SETLK,&arg);
#endif
	if(record==0) 
		no_superblock=1; 
	del_lock(record,bank); 
	return(0); 
} 
 
int CX_BASE::Check_Lock(long record,int bank) 
{ 
	if(in_memory)
		return(0);
#ifdef WIN32 
		return(0); 
#else 
	struct flock arg; 
 
	if(bank) 
	{ 
		record=0; 
		if(!fd[1].Fd && open_FD(fd+1)<0) 
			return(-1); 
	} 
	arg.l_whence=0; 
	arg.l_type=F_WRLCK; 
	if(record>0) 
	{ 
		arg.l_start=root_size+(off_t)(record-1)*ss.size;
		arg.l_len=ss.size; 
	} 
	else 
	{ 
		arg.l_start=0; 
		arg.l_len=root_size; 
	} 
	fcntl(bank?fd[1].Fd:fd->Fd,F_GETLK,&arg); 
	if(arg.l_type && arg.l_type !=F_UNLCK) 
	{ 
		if(arg.l_pid==getppid() || arg.l_pid==p_method)     /* блокирует родитель или Methods   */ 
			return(100);            /* будем считать все OK  */ 
	} 
	return(arg.l_type==F_WRLCK);  /* кто-то блокирует участок (не я) */ 
#endif 
} 
 
#ifndef WIN32 
#include <setjmp.h> 
static jmp_buf ret_buf; 
static void time_out(int i) 
{ 
	if(i==SIGALRM) 
		longjmp(ret_buf,1); 
} 
#endif 
 
/* заблокировать кадр в базе */ 
int CX_BASE::_Lock(long record,int type,int bank) 
{ 
	struct flock arg; 
	int j; 
	int i,num; 
	struct fd *ffd=NULL; 

#ifdef WIN32
	return(0);
#endif
	if(in_memory)
		return(0);
	flush_sb(); 
	if(bank) 
	{ 
		ffd=fd+1; 
		record=0; 
		if(!ffd->Fd && open_FD(ffd)<0) 
		{
			return(-1); 
		}
	} 
	else    ffd=fd; 
 
	if((num=_Check_Lock(record,bank,&i))>0) 
	{ 
		if(type!=F_WRLCK || lock.lock_str[i].type!=F_RDLCK)
		{ 
			if(lock.lock_str[i].type==F_RDLCK && type==F_WRLCK)
				lock.lock_str[i].type=F_WRLCK;
			lock.lock_str[i].num++;
			return(0); 
		} 
	} 
	if(num<0) 
	{
		return(-1); 
	}
	arg.l_whence=0; 
	arg.l_type=type; 
	if(record>0) 
	{ 
		arg.l_start=root_size+(off_t)(record-1)*ss.size;
		arg.l_len=ss.size; 
	} 
	else 
	{ 
		arg.l_start=0; 
		arg.l_len=root_size; 
	}
#ifndef WIN32
	j=fcntl(ffd->Fd,F_SETLK,&arg); 
	if(j==-1 && (arg.l_type && arg.l_type !=F_UNLCK)) 
	{ 
		if(fcntl(ffd->Fd,F_GETLK,&arg) || (arg.l_pid!=getppid() && arg.l_pid!=p_method)) 
		{ 
			if(record==0) /* нулевой кадр может блокироваться только временно, подождем */ 
			{ 
				void (* old_sig)(int); 
				if(setjmp(ret_buf)) 
					return(-1); 
				old_sig=signal(SIGALRM,time_out); 
				alarm(10);      // timeout 10 sec.
 
				j=fcntl(ffd->Fd,F_SETLKW,&arg);
 
				alarm(0); 
				signal(SIGALRM,old_sig); 
 
			} 
			else
				return(1); /* блокирует НЕ родитель и не Methods. */ 
		} 
	}
#else
	lseek(bank?fd[1].Fd:fd->Fd,arg.l_start,SEEK_SET);
	_locking(bank?fd[1].Fd:fd->Fd,LK_NBLCK,arg.l_len);
#endif
	if(record==0) 
		no_superblock=2; 
	add_lock(record,bank,type); 
	return(0); 
} 
 
/* сколько раз блокирован этот кадр */ 
int CX_BASE::_Check_Lock(long record,int bank,int *p) 
{ 
	int i; 

#ifdef WIN32
	return(0);
#endif
	if(in_memory)
		return(0);
	if(p!=NULL) 
		*p=-1; 
	if(bank) 
		record=0; 
	for(i=0;i<(int)lock.size;i++)
	{ 
		if(lock.lock_str[i].record==record && lock.lock_str[i].bank==bank)
		{ 
#ifdef SERVER 
			if(!pthread_equal(lock.lock_str[i].pid,pthread_self()))
				return(-1); 
#endif 
			if(p!=NULL) 
				*p=i; 
			return(lock.lock_str[i].num+1);
		} 
	} 
	return(0); 
} 
 
int CX_BASE:: add_lock(long record,int bank,int type) 
{ 
	int i; 
	if(bank) 
		record=0; 
BEG: 
	for(i=0;i<(int)lock.size;i++)
	{ 
		if(lock.lock_str[i].record == record && lock.lock_str[i].bank==bank)
		{ 
			if(lock.lock_str[i].type==F_RDLCK && type==F_WRLCK)
				lock.lock_str[i].type=F_WRLCK;
			lock.lock_str[i].num++;
#ifdef SERVER 
			lock.lock_str[i].pid=pthread_self();
#endif 
			return(0); 
		} 
	} 
	for(i=0;i<(int)lock.size;i++)
	{ 
		if(lock.lock_str[i].record==-1)
		{ 
				lock.lock_str[i].record=record;
				lock.lock_str[i].bank=bank;
				lock.lock_str[i].type=type;
				lock.lock_str[i].num=0;
#ifdef SERVER 
				lock.lock_str[i].pid=pthread_self();
#endif 
				return(0); 
		} 
	} 
	i=lock.size;
	lock.lock_str=(struct lock_record *)realloc(lock.lock_str,(++lock.size)*sizeof (struct lock_record));
	lock.lock_str[i].record=-1;
	goto BEG; 
} 
 
int CX_BASE:: del_lock(long record,int bank) 
{ 
	int i; 
	if(bank) 
		record=0; 
	for(i=0;i<(int)lock.size;i++)
	{ 
		if(lock.lock_str[i].record==record && lock.lock_str[i].bank==bank)
		{ 
			if(lock.lock_str[i].num)
				lock.lock_str[i].num--;
			else 
				lock.lock_str[i].record=-1;
			return(0); 
		} 
	} 
	return(0); 
} 

int CX_BASE::In_Memory(int arg)
{
	if(in_memory==0)
	{
#ifndef WIN32
		struct flock arg;
		int j;

		arg.l_start=0;
		arg.l_len=0;
		arg.l_whence=0;
		arg.l_type=F_RDLCK;

		void (* old_sig)(int);
		if(setjmp(ret_buf))
			return(-1);
		old_sig=signal(SIGALRM,time_out);
		alarm(10);      // timeout 10 sec.

		j=fcntl(fd->Fd,F_SETLKW,&arg);

		alarm(0);
		signal(SIGALRM,old_sig);
		if(j<0)
			return(-1);
#else
		lseek(fd->Fd,0,SEEK_SET);
		_locking(fd->Fd,LK_NBLCK,0);
#endif
	}
	in_memory=arg;
	for(int i=0;i<fdnum;i++)
	{
		if(fd[i].Fd>0)
		{
			if(fd[i].in_memory && fd[i].buf!=NULL && fd[i].Fd>0)
			{
				lseek(fd[i].Fd,0,SEEK_SET);
				write(fd[i].Fd,fd[i].buf,fd[i].fsize);
			}
			close(fd[i].Fd);
			fd[i].Fd=0;
			open_FD(fd+i);
		}
	}
	return(0);
}
