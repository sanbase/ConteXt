/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:write.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
extern class Terminal *term;
 
int CX_BASE::Put_Buf(struct fd *ffd,off_t seek_abs,int len,char *buf) 
{ 
	int i=0; 
 
	if( seek_abs < 0 || len < 0 ) 
		return(-1); 
	if(!ffd->Fd) 
		open_FD(ffd); 
	if(var_point!=NULL && var_point->Put(ffd->n,seek_abs,len,buf)!=-1) 
	{
		return(len); 
	}
	if(ffd->in_memory)
	{
		int f_size=ffd->fsize;
		if(seek_abs+len > ffd->fsize)
			ffd->fsize=seek_abs+len;
		if(seek_abs+len>ffd->size)
		{
			int bsize=seek_abs+len+65536;
			char *b=ffd->buf;
			if((ffd->buf=(char *)realloc(ffd->buf,bsize))==NULL)
			{
				if(b!=NULL)
				{
					lseek(ffd->Fd,0,SEEK_SET);
					write(ffd->Fd,b,f_size);
				}
				close(ffd->Fd);
				ffd->Fd=0;
				ffd->seek=-1;
				free(b);
				ffd->in_memory=0;
				in_memory=0;
				open_FD(ffd);
			}
			else
			{
				bzero(ffd->buf+ffd->size,bsize-ffd->size);
				ffd->size=bsize;
			}
		}
	}
	if(ffd->in_memory<2 && (lseek(ffd->Fd,seek_abs,SEEK_SET)!=seek_abs))
	{
		return(-2);
	}
	if(buf!=NULL) 
	{ 
		if(ffd->in_memory)
		{
			bcopy(buf,ffd->buf+seek_abs,len);
			if(ffd->in_memory==1)
				len=write(ffd->Fd,buf,len);
			return(len);
		}
		if(!(seek_abs<ffd->seek || seek_abs+len>ffd->seek+ffd->size || ffd->seek<0 || ts!=NULL))
		{
			if(Get_Buf(ffd,seek_abs,len)!=NULL)
				bcopy(buf,ffd->buf+seek_abs-ffd->seek,len);
		}
		i=write(ffd->Fd,buf,len); 
		return(i); 
	} 
	if(ffd->in_memory)
	{
		bzero(ffd->buf+seek_abs,len);
		return(len);
	}
	if(!(seek_abs<ffd->seek || seek_abs+len>ffd->seek+ffd->size || ffd->seek<0 || ts!=NULL))
	{
		if(Get_Buf(ffd,seek_abs,len)!=NULL)
			bzero(ffd->buf+seek_abs-ffd->seek,len);
	}
	buf=(char *)calloc(len,1);
	i=write(ffd->Fd,buf,len);
	free(buf);
	return(i); 
} 
 
int CX_BASE::Put_Node(long record,struct node node) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	return(Put_Node(record,sla,node)); 
} 
 
int CX_BASE::Put_Node(long record,int f,struct node node) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla->n=f; 
	return(Put_Node(record,sla,node)); 
} 
 
int CX_BASE::Put_Node(long record,struct sla *sla,struct node node) 
{ 
	struct key KEY; 
	struct field *field; 
 
	if(record<0 || sla->n<0 || sla->n>context->ptm ) 
	{ 
		return(-1); 
	} 
	field=Get_Field_Descr(sla); 
	if(node.s==1) 
	{ 
//                if(field->k==0 || sla[0].n!=1) 
			KEY.l=KEY.r=-1; 
//                else    return(Delete_Record(record,sla->n)); 
	} 
	else if(node.b==-1) 
		KEY.l=-node.x[0]; 
	else    KEY.l=node.x[0]; 
	if(node.b==1) 
		KEY.r=-node.x[1]; 
	else    KEY.r=node.x[1]; 
	if(record==0l && KEY.l==0) 
		KEY.r=0; 
	if(context->pswd!=CXKEY3 && (field->k || (ss.field->k && sla->n==0)))
	{ 
#ifdef SPARC 
		conv((char *)&KEY.r,sizeof (long)); 
		conv((char *)&KEY.l,sizeof (long)); 
#endif 
		if(record==0) 
		{ 
			int i; 
			if(ss.field->k && sla->n==0)
				i=num_keys;
			else
			{
				for(i=0;i<num_keys;i++)
					if(key_field[i]==sla->n)
						break;
				if(i==num_keys)
					return(-1);
			}
			if(Put_Buf(fd+2,4+i*sizeof KEY,sizeof KEY,(char *)&KEY)<0)
				return(-1); 
		} 
		else 
		{ 
			if(Put_Buf(fd+2,record*PAGESIZE,sizeof KEY,(char *)&KEY)<0) 
				return(-1); 
		} 
		if(node.x[0]>0 && record!=0) 
			Put_Buf(fd+2,node.x[0]*PAGESIZE+sizeof KEY,4,(char *)&record); 
		if(node.x[1]>0) 
			Put_Buf(fd+2,node.x[1]*PAGESIZE+sizeof KEY,4,(char *)&record); 
		return(0); 
	} 
 
	if(sla[0].n<=1 && sla[1].n==0) 
	{ 
		if(!node.s && (KEY.r>=0 || KEY.l>=0) && record) 
			Restore_Record(record);
#ifdef SPARC 
		conv((char *)&KEY.r,sizeof (long)); 
		conv((char *)&KEY.l,sizeof (long));
#endif 
		if(record==0) 
		{ 
			if(Put_Buf(fd,(off_t)0,sizeof KEY,(char *)&KEY)<0) 
				return(-1); 
		} 
		else 
		{ 
			if(Put_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),sizeof KEY,(char *)&KEY)<0)
				return(-1); 
		} 
	} 
	else 
	{ 
		if(open_Tree(sla)<0) 
			return(-1); 
		if(context->v==39) 
			conv_39node(field->atr.attr->tree_fd,&KEY); 
#ifdef SPARC 
		conv((char *)&KEY.r,sizeof (long)); 
		conv((char *)&KEY.l,sizeof (long)); 
#endif 
		lseek(field->atr.attr->tree_fd,(off_t)(sizeof KEY)*(record),SEEK_SET); 
		if(write(field->atr.attr->tree_fd,&KEY,sizeof KEY)<0) 
			return(-1); 
	} 
	return(0); 
} 
 
#define MAXMIN  -2147483647l 
/* Вставить узел в цепочку удаленных (удалить кадр) */ 
int CX_BASE::Delete_Record(long record,int field) 
{ 
	struct node node; 
	struct key  KEY; 
	long recordl; 
	char *ch; 
 
	if(Wlock(0)) 
		return(-1); 
	if(field<=0)
	{
		if(ss.field->k && context->pswd!=CXKEY3 && field==-1)
		{
			char *ch=Get_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),2);
			int del=int_conv(ch,2);
			if(!(del&0x80))
			{
				del|=0x80;
				char str[8];
				int_to_buf(str,2,(dlong)del);
				Put_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),2,str);
			}

			TTree dt(this);
			long page=dt.Find_Page(0,"",record);
			if(page>0)
			{
				TPage *t=(TPage *)Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
				KEY=t->find("",record);
				if(KEY.r>=0)
					return(0);      // this record is deleted yet
			}
			Put_Storage(record,0,"");

			Unlock(0);
			return(0);
		}
		ch=Get_Buf(fd,(off_t)sizeof KEY,sizeof KEY); 
	}
	else    ch=Get_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY);
	if(ch==NULL) 
	{ 
		Unlock(0); 
		return(-1); /* нельзя удалить что-то из пустой базы */ 
	} 
	bcopy(ch,&KEY,sizeof KEY); 
#ifdef SPARC 
	conv((char *)&KEY.r,sizeof (long)); 
	conv((char *)&KEY.l,sizeof (long)); 
#endif 
	node=Get_Node(record,field); 
	if(node.s) /* зачем удалять удаленное? Но проверим. */ 
	{ 
		for(node.x[1]=KEY.l;node.x[1]<0 && node.x[1]!=-record && node.x[1]!=MAXMIN;) 
			node=Get_Node(-node.x[1],field); 
		if(node.x[1]==-record) 
		{ 
			Unlock(0); 
			return(0);      /* он и так в цепочке */ 
		} 
	} 
	recordl=-KEY.r; 
	if(recordl>0 && KEY.l!=MAXMIN) 
	{ 
		node=Get_Node(recordl,field); 
		node.x[1]=-record; 
		node.s=node.b=0; 
		Put_Node(recordl,field,node); 
		node.x[0]=-recordl; 
	} 
	else 
	{ 
		node=Get_Node(record,field); 
		KEY.l=-record; 
		node.x[0]=MAXMIN; 
	} 
	node.x[1]=MAXMIN; 
	node.s=node.b=0; 
	Put_Node(record,field,node); 
	KEY.r=-record; 
 
#ifdef SPARC 
	conv((char *)&KEY.r,sizeof (long)); 
	conv((char *)&KEY.l,sizeof (long)); 
#endif 
	if(field==0 && !(ss.field->k && context->pswd!=CXKEY3))
		recordl=Put_Buf(fd,(off_t)sizeof KEY,sizeof KEY,(char *)&KEY); 
	else    recordl=Put_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY,(char *)&KEY);
	Unlock(0); 
	return(recordl); 
 
} 
 
/* Удалить узел из цепочки удаленных (восстановить кадр) */ 
int CX_BASE::Restore_Record(long record,int field) 
{ 
	struct node node; 
	struct key  KEY; 
	long recordl,recordr; 
	char *ch; 
 
	if(Wlock(0)) 
		return(-1); 
	if(field<=0)
	{
		if(ss.field->k && context->pswd!=CXKEY3)
		{
			if(field==-1)
			{
				Delete_Storage(record,0);
				char *ch=Get_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),2);
				int del=int_conv(ch,2);
				if(del&0x80)
				{
					del&=~0x80;
					char str[8];
					int_to_buf(str,2,(dlong)del);
					Put_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),2,str);
				}
				return(0);
			}
			ch=Get_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY);
		}
		else
			ch=Get_Buf(fd,(off_t)sizeof KEY,sizeof KEY);
	}
	else
		ch=Get_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY);
	if(ch==NULL) 
	{ 
		Unlock(0); 
		return(0); /* нельзя восстановить кадр пустой базы */ 
	} 
	bcopy(ch,&KEY,sizeof KEY); 
#ifdef SPARC 
	conv((char *)&KEY.r,sizeof (long)); 
	conv((char *)&KEY.l,sizeof (long)); 
#endif 
	node=Get_Node(record,field); 
	if(!node.s)        /* если он не удален - восстанавливать нечего */ 
	{ 
		Unlock(0); 
		return(-1); 
	} 
	recordl=node.x[0]; 
	recordr=node.x[1]; 
	if(recordl==MAXMIN) 
	{       /* первый в цепочке */ 
		if(recordr!=MAXMIN) 
			KEY.l=recordr; 
		else KEY.r=KEY.l=0;     /* нет удаленных */ 
#ifdef SPARC 
		conv((char *)&KEY.r,sizeof (long)); 
		conv((char *)&KEY.l,sizeof (long)); 
#endif 
		if(field==0 && !(ss.field->k && context->pswd!=CXKEY3))
			Put_Buf(fd,(off_t)sizeof KEY,sizeof KEY,(char *)&KEY); 
		else    Put_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY,(char *)&KEY);
 
		if(recordr!=MAXMIN) 
			goto NEXT; 
		Unlock(0); 
		return(0); 
	} 
	node=Get_Node(-recordl,field); 
	node.x[1]=recordr; 
	node.s=node.b=0; 
	Put_Node(-recordl,field,node); 
	if(recordr==MAXMIN) 
	{       /* последний в цепочке */ 
		KEY.r=recordl; 
		if(field==0 && !(ss.field->k && context->pswd!=CXKEY3))
			Put_Buf(fd,(off_t)sizeof KEY,sizeof KEY,(char *)&KEY); 
		else    Put_Buf(fd+2,PAGESIZE-sizeof KEY,sizeof KEY,(char *)&KEY);
 
		Unlock(0); 
		return(0); 
	} 
NEXT: 
	node=Get_Node(-recordr,field); 
	node.x[0]=recordl; 
	node.s=node.b=0; 
	Put_Node(-recordr,field,node); 
	Unlock(0); 
	return(0); 
} 

#include <time.h>

int CX_BASE::write_change(long record,int n)
{
//добавил reord<=0 так как а зачем сохранять если номера записи нет fill
	if(change_db==NULL||record<=0)
		return 0;
	int ret;
	int field=n+2;
	char *t=local_time();


       /* а нахрена это делать с основной базой-то! fill
       if(Wlock(0)<0)
		return(-1);*/
	if((ret=change_db->Check_Del(record)))
	{
		if((ret=change_db->Write_Empty_Record(record))<0)
			return ret;
	}
	if((ret=change_db->Put_Slot(record,1,t))>0 && n>=0)
		ret=change_db->Put_Slot(record,field,t);
	 /*
	 добавил Unlock fill
	 */
	 change_db->Unlock(record);
       // Unlock(0);
	return ret;
}
