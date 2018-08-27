/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:delete.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term;
 
/* удалить кадр из базы и дописать новое значение удаленного кадра */ 
int CX_BASE::Delete(long record) 
{ 
	int i; 

	if(record<=0l || context->b0&NO_DELETE) 
		return(-1); 
	if(record>max_record) 
	{ 
		update(); 
		if(record>max_record) 
			return(-1); 
	} 
	if(Check_Del(record))
		return(0); 
	if(Wlock(0)) 
	{
		Unlock(record); 
		return(-1); 
	} 
	if(Wlock(record))
	{
		Unlock(0);
		return(-1);
	}
	for(i=1;i<=context->ptm;i++) 
	{ 
		if(Get_Field_Descr(i)->k && context->pswd!=CXKEY3)
			Delete_Storage(record,i);
		else if(Get_Field_Descr(i)->b)
			Delete_Tree(record,i);
		else if(open_Tree(i)>0)
			Delete_Node(record,i);
		if(variable(&ss,i-1))
			Write(record,i,NULL);   // удалим поля переменного размера
	} 

	Delete_Record(record,ss.field->k && context->pswd!=CXKEY3?-1:0);
	Unlock(record);
	Unlock(0);
	write_change(record,-1);
	return(0); 
} 
/* востановить удаленный кадр */ 
int CX_BASE::Restore(long record ) 
{ 
	int i; 
 
	if(record<0) 
		return(-1); 
	if(record>max_record) 
	{ 
		update(); 
		if(record>max_record) 
			return(-1); 
	} 
	if( Wlock(record)<0) 
		return(-1); 
	if(!Get_Node(record,0).s || Wlock(0)) 
	{ 
		Unlock(record); 
		return(-1); 
	} 
	Restore_Record(record,ss.field->k && context->pswd!=CXKEY3?-1:0);
	for(i=1;i<=context->ptm;i++) 
	{ 
		if(Get_Field_Descr(i)->k && context->pswd!=CXKEY3) 
			continue; 
		if(open_Tree(i)>0 || i==1)
			Insert_Node(record,i); 
	} 
	Unlock(0); 
	Unlock(record); 
	return(0); 
} 
 
int CX_BASE::Check_Del(long record) 
{ 
	if(!record) 
		return(0); 
	if(ss.field->k && context->pswd!=CXKEY3)
	{
		char *ch=Get_Buf(fd,(root_size+(off_t)(record-1)*ss.size),2);
		int del=int_conv(ch,2);
		return(del&0x80);
	}
	return(Get_Node(record).s); 
} 
 
int CX_BASE::Remove_Element(long record,struct sla *sla) 
{ 
	char *ch=NULL; 
	struct sla SLA[SLA_DEEP]; 
	int shift=4;
	int j=-1,size,len; 
 
	bzero(SLA,sizeof SLA); 
	for(int i=0;sla[i].n;i++) 
	{ 
		SLA[i].n=sla[i].n; 
		SLA[i].m=sla[i].m; 
		if(SLA[i].m) 
			j=i; 
	} 
	if(j<0) 
		return(-1);     // number of element is absent 
	SLA[j+1].n=0; 
	if(!Field_Descr(SLA)->m) 
		return(-1);     // it's not an array 
	SLA[j].m=-1; 
	if( Wlock(record)<0) 
		return(-1); 
	Rlock(0); 
	int num=Read(record,SLA,ch).len; 
	if(sla[j].m>num) 
	{ 
		Unlock(0); 
		Unlock(record); 
		return(-1); 
	} 
	SLA[j].m=0; 
	if(num==1) 
	{ 
		ch=(char *)realloc(ch,sizeof (long)+4);
		bzero(ch,sizeof (long)+4);
		goto END; 
	} 
	len=Read(record,SLA,ch).len; 
	if(ch==NULL) 
	{ 
		Unlock(0); 
		Unlock(record); 
		return(-1); 
	} 
	size=len/num; 

	bcopy(ch+(sla[j].m)*size,ch+(sla[j].m-1)*size,(num-sla[j].m)*size); 

	shift=4+(context->pswd==CXKEY6?4:0);
	ch=(char *)realloc(ch,len+shift);
	bcopy(ch,ch+shift,len);
	num--; 
	len=num*size+shift;
	bcopy(&len,ch,sizeof (long)); 
	if(context->pswd==CXKEY6)
		bcopy(&record,ch+4,4);
#ifdef SPARC 
	conv(ch,sizeof (long)); 
#endif 
END: 
	size=Write(record,SLA,ch);
	Unlock(0); 
	Unlock(record); 
	free(ch); 
	return(size); 
} 
