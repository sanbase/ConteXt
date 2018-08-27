/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:remote.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifdef THREAD 
#include <pthread.h> 
#endif 
 
RCX_BASE **open_base=NULL; 
int num_open_bases=0; 

RCX_BASE::RCX_BASE(char *name):CX_BASE(name)
{
};
 
void clean_cx() 
{ 
	for(int i=0;i<num_open_bases;i++) 
		delete open_base[i]; 
	if(open_base!=NULL) 
	       free(open_base); 
	open_base=NULL; 
	num_open_bases=0; 
} 
 
RCX_BASE::~RCX_BASE() 
{ 
	if(__name==NULL || *__name==0) 
		return; 
	if(hist!=NULL) 
	       free(hist); 
	close_FD(); 
	if(fd!=NULL) 
		free(fd); 
	free_substruct(&ss); 
	if(share!=NULL) 
		UnMap(); 
	else    if(cadr!=NULL) 
			free(cadr); 
	if(lock.lock_str!=NULL) 
		free(lock.lock_str); 
	if(__name!=NULL) 
		free(__name); 
	if(context!=NULL) 
		free(context); 
	bzero(this,sizeof (RCX_BASE)); 
} 
 
RCX_BASE *open_db(char *name) 
{ 
	RCX_BASE *subbase=NULL; 
	for(int i=0;i<num_open_bases;i++) 
	{ 
		if(!strcmp(open_base[i]->Name_Base(),name)) 
		{ 
			RCX_BASE *tmp=open_base[i]; 
			bcopy(open_base+i+1,open_base+i,(num_open_bases-i-1)*(sizeof *open_base)); 
			bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base)); 
			open_base[0]=tmp; 
			return(open_base[0]); 
		} 
	} 
	try 
	{ 
		subbase = new RCX_BASE(name); 
	} 
	catch(...) 
	{ 
		return(NULL); 
	} 
	if(num_open_bases==MAX_OPEN_BASE) 
	{ 
		delete open_base[--num_open_bases]; 
	} 
	open_base=(RCX_BASE **)realloc(open_base,(++num_open_bases)*sizeof (RCX_BASE *)); 
	bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base)); 
	return(open_base[0]=subbase); 
} 
 
int RCX_BASE::_Check_Lock(long record,int bank,int *p) 
{ 
	int i; 
 
	if(p!=NULL) 
		*p=-1; 
	if(bank) 
		record=0; 
	for(i=0;i<(int)lock.size;i++) 
	{ 
		if(lock.lock_str[i].record==record && lock.lock_str[i].bank==bank) 
		{ 
#ifdef THREAD 
			if(!pthread_equal((pthread_t)lock.lock_str[i].pid,pthread_self())) 
#else 
			if(lock.lock_str[i].pid==getpid()) 
#endif 
				return(-1); 
			if(p!=NULL) 
				*p=i; 
			return(lock.lock_str[i].num+1); 
		} 
	} 
	return(0); 
} 
 
int RCX_BASE::add_lock(long record,int bank,int type) 
{ 
	int i; 
	if(bank) 
		record=0; 
BEG: 
	for(i=0;i<(int)lock.size;i++) 
	{ 
		if(lock.lock_str[i].record == record && lock.lock_str[i].bank==bank) 
		{ 
#ifndef WIN32 
			if(lock.lock_str[i].type==F_RDLCK && type==F_WRLCK) 
				lock.lock_str[i].type=F_WRLCK; 
#endif 
			lock.lock_str[i].num++; 
#ifdef THREAD 
			lock.lock_str[i].pid=pthread_self(); 
#else 
			lock.lock_str[i].pid=getpid(); 
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
#ifdef THREAD 
				lock.lock_str[i].pid=pthread_self(); 
#else 
				lock.lock_str[i].pid=getpid(); 
#endif 
				return(0); 
		} 
	} 
	i=lock.size; 
	lock.lock_str=(struct lock_record *)realloc(lock.lock_str,(++lock.size)*sizeof (struct lock_record)); 
	bzero(lock.lock_str+i,(lock.size-i)*sizeof (struct lock_record)); 
	lock.lock_str[i].record=-1; 
	goto BEG; 
} 
 
int RCX_BASE:: del_lock(long record,int bank) 
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
			{ 
				lock.lock_str[i].record=-1; 
				lock.lock_str[i].bank=0; 
				lock.lock_str[i].type=0; 
				lock.lock_str[i].pid=0; 
			} 
			return(0); 
		} 
	} 
	return(0); 
} 
