/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:transaction.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifndef WIN32 
Transaction::Transaction(CX_BASE *base) 
{ 
	bzero(this,sizeof (Transaction)); 
	db=base; 
	db->ts=this; 
} 
Transaction::~Transaction() 
{ 
	db->ts=NULL; 
	int i,j; 
 
// все изменения физически записываются в базу 
	for(i=0;i<num_elem;i++) 
		for(j=0;j<db->fdnum;j++) 
			if(db->fd[j].n==elem[i].bank) 
				db->Put_Buf(db->fd+j,elem[i].seek_abs,elem[i].new_buf.len,elem[i].new_buf.buf); 
 
// потом снимаются блокировки 
	for(i=0;i<num_elem;i++) 
	{ 
 
		struct flock arg; 
 
		arg.l_start=elem[i].seek_abs; 
		arg.l_len=elem[i].new_buf.len; 
		arg.l_type=F_UNLCK; 
		for(j=0;j<db->fdnum;j++) 
		{ 
			if(db->fd[j].n==elem[i].bank) 
			{ 
				fcntl(db->fd[j].Fd,F_SETLK,&arg); 
				break; 
			} 
		} 
		free(elem[i].new_buf.buf); 
		free(elem[i].old_buf.buf); 
	} 
} 
void Transaction::Roll_Back() 
{ 
// в этом случае просто освобождаются буфера 
	for(int i=0;i<num_elem;i++) 
	{ 
		for(int j=0;j<db->fdnum;j++) 
		{ 
			if(db->fd[j].n==elem[i].bank) 
			{ 
				free(elem[i].new_buf.buf); 
				free(elem[i].old_buf.buf); 
			} 
		} 
	} 
	num_elem=0; 
} 
char *Transaction::Read(struct fd *ffd, off_t seek_abs, int len) 
{ 
	for(int i=0;i<num_elem;i++) // если это было изменено в транзакции, вернем новое значание 
	{ 
		if(ffd->n==elem[i].bank) 
		{ 
			if(seek_abs+len<elem[i].seek_abs || seek_abs>elem[i].seek_abs+elem[i].new_buf.len) 
				continue; 
 
			if(elem[i].seek_abs<=seek_abs && elem[i].seek_abs+elem[i].new_buf.len>=seek_abs+len) 
				return(elem[i].new_buf.buf+(seek_abs-elem[i].seek_abs)); 
 
			char *ch=db->Get_Buf(ffd,seek_abs,len); 
 
			if(seek_abs<=elem[i].seek_abs && seek_abs+len>=elem[i].seek_abs+elem[i].new_buf.len) 
				bcopy(elem[i].new_buf.buf,ch+elem[i].seek_abs-seek_abs,elem[i].new_buf.len); 
 
			if(seek_abs<=elem[i].seek_abs && seek_abs+len<elem[i].seek_abs+elem[i].new_buf.len) 
				bcopy(elem[i].new_buf.buf,ch+(elem[i].seek_abs-seek_abs),(seek_abs-elem[i].seek_abs)+(len-elem[i].new_buf.len)); 
 
			if(seek_abs>elem[i].seek_abs) 
				bcopy(elem[i].new_buf.buf+seek_abs-elem[i].seek_abs,ch,elem[i].seek_abs+elem[i].new_buf.len-seek_abs); 
 
			return(ch); 
		} 
	} 
	// а если нет, то читаем из базы 
	return(db->Get_Buf(ffd,seek_abs,len)); 
} 
int  Transaction::Write(struct fd *fd, off_t seek_abs,int len, char *buf) 
{ 
	for(int i=0;i<num_elem;i++) // проверим, не входило ли это в транзакцию раньше 
	{ 
		if(fd->n==elem[i].bank && elem[i].seek_abs>=seek_abs && (elem[i].seek_abs+elem[i].new_buf.len)<=(seek_abs+len)) 
		{ 
			bcopy(elem[i].new_buf.buf+seek_abs-elem[i].seek_abs,buf,len); 
			return(0); 
		} 
	} 
	// если нет, то подпишем новое значение в журнал 
	char *ch=db->Get_Buf(fd,seek_abs,len); 
	struct flock arg; 
 
	arg.l_start=seek_abs; 
	arg.l_len=len; 
	arg.l_type=F_WRLCK; 
 
	fcntl(fd->Fd,F_SETLKW,&arg); // все что пишется - блокируется 
 
	elem=(struct elem *)realloc(elem,sizeof (struct elem)*(++num_elem)); 
	elem[num_elem-1].seek_abs=seek_abs; 
	elem[num_elem-1].new_buf.len=len; 
	elem[num_elem-1].bank=fd->n; 
	elem[num_elem-1].new_buf.buf=(char *)malloc(len); 
	elem[num_elem-1].old_buf.buf=(char *)malloc(len); 
	if(ch!=NULL) 
	{ 
		bcopy(ch,elem[num_elem-1].old_buf.buf,len); 
		bcopy(ch,elem[num_elem-1].new_buf.buf,len); 
	} 
	bcopy(buf,elem[num_elem-1].new_buf.buf,len); 
	return(0); 
} 
#endif 
