/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:vector.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
Mem_Buf::Mem_Buf() 
{ 
	num=0; 
	e=NULL; 
} 
 
Mem_Buf::~Mem_Buf() 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(e[i].ch!=NULL) 
			free(e[i].ch); 
	} 
	if(e!=NULL) 
		free(e); 
} 
 
void Mem_Buf::Free(int bank, off_t seek0, int len) 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(bank==e[i].bank && seek0>=e[i].seek && seek0+len<=e[i].seek+e[i].len) 
		{ 
			if(e[i].ch!=NULL) 
				free(e[i].ch); 
			if((--num)!=i) 
				bcopy(e+i+1,e+i,(num-i)*sizeof (struct mem_buf)); 
		} 
	} 
} 
 
char *Mem_Buf::Get(int bank, off_t seek0, int len) 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(bank==e[i].bank && seek0>=e[i].seek && seek0+len<=e[i].seek+e[i].len) 
		{ 
			return(e[i].ch+(seek0-e[i].seek)); 
		} 
	} 
	return(NULL); 
} 
 
int Mem_Buf::Open(int bank, off_t seek0, int len, char *ch) 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(bank==e[i].bank && seek0>=e[i].seek && seek0+len<=e[i].seek+e[i].len) 
			return(i); 
	} 
	e=(struct mem_buf *)realloc(e,++num*sizeof (struct mem_buf)); 
	e[num-1].ch=(char *)malloc(len); 
	bcopy(ch,e[num-1].ch,len); 
	e[num-1].seek=seek0; 
	e[num-1].len=len; 
	e[num-1].bank=bank; 
	return(num-1); 
} 
 
int Mem_Buf::Put(int bank, off_t seek0, int len, char *ch) 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(bank==e[i].bank && seek0==e[i].seek) 
		{ 
			e[i].ch=(char *)realloc(e[i].ch,len); 
			bcopy(ch,e[i].ch,len); 
			e[i].len=len; 
			return(i); 
		} 
		if(bank==e[i].bank && seek0>=e[i].seek && seek0+len<=e[i].seek+e[i].len) 
		{ 
			bcopy(ch,e[i].ch+(seek0-e[i].seek),len); 
			return(i); 
		} 
	} 
	return(-1); 
} 
