/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:repair.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#include "ram_base.h" 
#include "../CX_Browser.h" 
#ifndef WIN32 
#include <sys/wait.h> 
#endif 
#define MAXMIN -2147483647l 
 
#ifndef WIN32 
static void suspend_init() 
{ 
	for(int i=1;i<SIGUSR1;i++) 
		signal(i,SIG_IGN); 
	return; 
} 
#endif 
 
void CX_REPAIR::Create_Tree() 
{ 
	char *ch=NULL; 
 
	Wlock(0); 
	for(int page=1;page<=max_record;page++) 
	{ 
		if(Check_Del(page)) 
		{ 
			delete_list=(long *)realloc(delete_list,++del_num*sizeof(long)); 
			delete_list[del_num-1]=page; 
			continue; 
		} 
		Read(page,v_field,ch); 
		if(ch!=NULL) 
			bcopy(ch,(char *)pos+page*len_rec+sizeof (struct key),len_rec-sizeof (struct key)); 
		Insert_Node(page,1); 
	} 
	free(ch); 
	if(v_field->n==1) 
	{ 
		int del,cur_del=0; 
 
		Put_Buf(fd,(off_t)0,sizeof (struct key),(char *)pos); 
		if(del_num) 
		{ 
			del=-delete_list[0]; 
			Put_Buf(fd,sizeof (struct key),sizeof (long),(char *)&del); 
		} 
		else 
		{ 
			long a[2]; 
			bzero(a,sizeof a); 
			Put_Buf(fd,sizeof (struct key),2*sizeof (long),(char *)a); 
		} 
		for(int page=1;page<=max_record;page++) 
		{ 
			if(cur_del<del_num && page==delete_list[cur_del]) 
			{ 
				struct key key; 
 
				if(cur_del) 
					key.l=-delete_list[cur_del-1]; 
				else    key.l=MAXMIN; 
				if(cur_del<del_num-1) 
					key.r=-delete_list[cur_del+1]; 
				else    key.r=MAXMIN; 
				bcopy(&key,(char *)pos+page*len_rec,sizeof key); 
				if((++cur_del)==del_num) 
				{ 
					del=-delete_list[cur_del-1]; 
					Put_Buf(fd,sizeof (struct key)+sizeof (long),sizeof (long),(char *)&del); 
				} 
			} 
			Put_Buf(fd,root_size+(page-1)*ss.size,sizeof (struct key),(char *)pos+page*len_rec); 
		} 
	} 
	else 
	{ 
		char name[LINESIZE]; 
 
		sprintf(name,"%s/Tree.%d",Name_Base(),v_field->n); 
		int fd=creat(name,0666); 
		write(fd,pos,sizeof(struct key)); 
		for(int page=1;page<=max_record;page++) 
		{ 
			write(fd,(char *)pos+page*len_rec,sizeof(struct key)); 
		} 
		close(fd); 
	} 
	Unlock(0); 
} 
 
int rebuild_tree(char *name,long record,struct sla *sla) 
{ 
	int pid,i; 
//        return(-1); // temporary
#ifndef WIN32 
 
	if((pid=vfork())==0) 
	{ 
		CX_REPAIR *ram; 
		char str[LINESIZE]; 
		suspend_init(); 
		sprintf(str,"%s/db.log",name); 
		int fd=open(str,O_RDWR|O_CREAT,0666); 
		lseek(fd,0,SEEK_END); 
		sprintf(str,"%s tree_error: page=%ld n=%d\n",local_time(),record,sla->n); 
		write(fd,str,strlen(str)); 
		close(fd); 
		try 
		{ 
			ram=new CX_REPAIR (name,sla->n); 
		} 
		catch(...) 
		{ 
			exit(1); 
		} 
		ram->Create_Tree(); 
		delete ram; 
		exit(0); 
	} 
	waitpid(pid,&i,0); 
	return(0); 
#endif 
} 
