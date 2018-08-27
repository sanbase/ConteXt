/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:storage.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
extern class Terminal *term;
 
int CX_BASE::Get_Storage(long record, int field, char *&slot) 
{ 
	if(field==0)
		return(0);

	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	char *ch=Get_Buf(fd,seek,4); 
 
	if(ch==NULL) 
		return(-1); 
	long page=int_conv(ch,4); 
	if(page<=0) 
		return(-1); 
	TPage *val=(TPage *)Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
	if(val==NULL || (ch=val->value(record))==NULL) 
		return(-1); 
	if(slot!=NULL) 
		free(slot); 
	int length=val->len(ch)-4;
	slot=(char *)calloc(length,1);
	if(val->hdr.z) 
	{ 
		while(val->hdr.z) 
		{ 
			bincopy(ch,slot,PAGESIZE-sizeof (struct page_hdr)-sizeof (struct key));
			slot+=PAGESIZE-sizeof (struct page_hdr)-sizeof (struct key);
			val=(TPage *)Get_Buf(fd+2,val->hdr.z*PAGESIZE,PAGESIZE);
		} 
		bincopy(ch,slot,PAGESIZE-val->hdr.space);
		return(length);
	} 
	else 
	{ 
		if(val->hdr.a==X_TEXT) 
		{ 
			length-=(val->hdr.n==0?4:val->hdr.n);
			bincopy(val->value(ch),slot,length);
			return(length);
		} 
		else 
		{ 
			bincopy(val->value(ch),slot,length);
			return(length);
		} 
	} 
} 
 
char *CX_BASE::Get_Storage(long record, int field) 
{ 
	if(field==0)
		return(NULL);
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	char *ch=Get_Buf(fd,seek,4); 
	if(ch==NULL) 
		return(NULL); 
	long page=int_conv(ch,4); 
	if(page==0) 
		return(NULL); 
	TPage *val=(TPage *)Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
	if(val==NULL) 
		return(NULL); 
	return(val->value(record)); 
} 

long CX_BASE::New_Storage_Page(int field) 
{ 
	long page; 
	struct field *des=Get_Field_Descr(field); 
 
	Wlock(0); 
	char *ch=Get_Buf(fd+2,PAGESIZE-4,4); 
	if(ch!=NULL) 
		page=-int_conv(ch,4); 
	else page=0; 
	if(page>0) 
		Restore_Record(page,field); 
	else 
	{ 
		if(fd[2].in_memory)
		{
			page=fd[2].fsize/PAGESIZE;
		}
		else
		{
			struct stat st;
			fstat(fd[2].Fd,&st);
			page=st.st_size/PAGESIZE;
		}
		if(page==0)
			page=1;
	} 
	off_t seek=page*PAGESIZE; 
	TPage *val=(TPage *)calloc(PAGESIZE,1);
	val->hdr.space=PAGESIZE-sizeof (struct page_hdr)-sizeof (struct key);
	val->hdr.a=des->a; 
	val->hdr.l=des->l; 
	val->hdr.n=des->n; 
	val->hdr.field=field;
	val->hdr.PAGESIZE=PAGESIZE;
	Put_Buf(fd+2,seek,PAGESIZE,(char *)val); 
	free(val); 
	Unlock(0); 
	return(page); 
} 
 
int CX_BASE::Put_Storage(long record,int field,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Put_Storage(record,sla,slot)); 
} 
 
int CX_BASE::Put_Storage(long record,struct sla *sla,char *slot) 
{ 
	Wlock(0); 
	struct field *fdes=Get_Field_Descr(sla->n); 
	char *ch=Get_Storage(record,sla->n); 
	if(ch!=NULL) 
	{ 
		int len=fdes->l;
		if(fdes->a==X_TEXT)
		{
			int l=fdes->n==0?4:fdes->n;
			if(int_conv(ch,l)==int_conv(slot,l))
			{
				if(!strcmp(ch+l,slot+l))
				{
					Unlock(0);
					return(0);      // ничего не изменилось
				}
			}
		}
		else if(!bcmp(ch,slot,len))
		{ 
			Unlock(0); 
			return(0);      // ничего не изменилось 
		} 
	} 
	Insert_Storage(record,sla,slot); 
	Unlock(0); 
	return(-1); 
} 
 
int CX_BASE::Delete_Storage(long record,int field) 
{ 
	long page=0; 
	char *ch; 
	TPage *val;
	if(field<=0)
	{
		TTree dt(this);
		if((page=dt.Find_Page(0,"",record))<=0)
		{
			return(0);
		}
	}
	else
	{
		int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
		off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
		Wlock(0);
		ch=Get_Buf(fd,seek,4);
		if(ch==NULL)
			goto ERR;
		page=int_conv(ch,4);
		if(page==0)
			goto ERR;
	}
	ch=Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
	if(ch==NULL)
	{
		goto ERR; 
	}
	val = (TPage *)malloc(PAGESIZE);
	bincopy(ch,val,PAGESIZE);

	if((ch=val->value(record))==NULL) 
	{
		goto ERR; 
	}
	val->del(ch-4);
	if(val->hdr.last==0) 
	{ 
		if(field==-1)
		{
			Delete_Node(page,0,1);
			Delete_Record(page,0);
		}
		else
		{
			Delete_Node(page,field,1);
			Delete_Record(page,field);
		}
	} 
	else 
		Put_Buf(fd+2,page*PAGESIZE,PAGESIZE,(char *)val); 
	Unlock(0); 
	free(val);
	return(0);
ERR: 
	Unlock(0); 
	return(-1); 
} 
 
int CX_BASE::Insert_Storage(long record,struct sla *sla,char *slot) 
{ 
	int rebild=0; 
	long page; 
 
	TTree dt(this);
	Delete_Storage(record,sla->n);
	page=dt.Find_Page(sla->n,slot,record); 
	if(page<0) 
	{ 
		page=New_Storage_Page(sla->n); 
		rebild=1; 
	} 
	int i=Insert_Storage(record,sla,slot,page,rebild); 
	return(i); 
} 

int CX_BASE::Insert_Storage(long record,struct sla *sla,char *slot, long page, int rebild) 
{ 
	TPage *val;
	int i; 
	char *ch; 
	ch=Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
	if(ch==NULL)
		return(-1);
	val = (TPage *)calloc(PAGESIZE*3,1);
	bincopy(ch,val,PAGESIZE);
	if((ch=val->value(record))!=NULL) 
		val->del(ch-4);
	TPage *tmp_r = (TPage *)((char *)val+PAGESIZE);
	TPage *tmp_l = (TPage *)((char *)val+2*PAGESIZE);
	bincopy(val,tmp_r,sizeof val->hdr);
	tmp_r->hdr.last=0;
	tmp_r->hdr.space=PAGESIZE-sizeof (struct page_hdr)-sizeof (struct key);
	bincopy(tmp_r,tmp_l,sizeof tmp_r->hdr);
	if(tmp_r->len(slot)>=tmp_r->hdr.space)    // field size > PAGESIZE
	{
		int size=tmp_r->len(slot);
		int len=0;
		int first=0;
		while(len<size)
		{
			bincopy(slot+len,tmp_r->buf,tmp_r->hdr.space);
			len+=tmp_r->hdr.space;
			tmp_r->hdr.space=0;
			page=New_Storage_Page(sla->n);
			tmp_r->hdr.z=page;
			Put_Buf(fd+2,page*PAGESIZE,PAGESIZE,(char *)tmp_r);
			if(!first)
			{
				Insert_Node(page,sla->n,1);
				first=page;
			}
		}
		free(val);
		return(first);
	}
	char str[64];
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	off_t seek;
	if(sla->n==0)
		seek=-1;
	else
		seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[sla->n-1].atr.attr->wshift;
INS:
	switch(val->insert(slot,record))
	{
		case -1:
			val->push_right(tmp_r);
			goto INS;
		case 1:
			val->push_left(tmp_l);
			goto INS;
		case -2:
			tmp_r->insert(slot,record);
			goto NEWPAGE;
		case 2:
			tmp_l->insert(slot,record);
			goto NEWPAGE;
	}
	Put_Buf(fd+2,page*PAGESIZE,PAGESIZE,(char *)val); 
	if(rebild) 
		Insert_Node(page,sla->n,1); 
 
	if(seek>0)
	{
		int_to_buf(str,4,(dlong)page);
		Put_Buf(fd,seek,4,str);
	}
NEWPAGE:
	if(tmp_r->hdr.last)
	{
		if(tmp_r->hdr.KEY.r==0)
		{
			page=New_Storage_Page(sla->n);
			tmp_r->hdr.KEY.r=0;
			tmp_r->hdr.KEY.l=0;
			Put_Buf(fd+2,page*PAGESIZE,PAGESIZE,(char *)tmp_r);
			Insert_Node(page,sla->n,1);
			int_to_buf(str,4,(dlong)page);

			for(i=0;i<tmp_r->hdr.last;i++)
			{
				if(sla->n)
				{
					seek=root_size+(off_t)(tmp_r->rec(i)-1)*ss.size+key_len+ss.field[sla->n-1].atr.attr->wshift;
					Put_Buf(fd,seek,4,str);
				}
			}
		}
		else
		{
			TTree dt(this);
			if((page=dt.Find_Left(sla->n,tmp_r->hdr.KEY.r<0?-tmp_r->hdr.KEY.r:tmp_r->hdr.KEY.r))>0)
			{
				for(i=0;i<tmp_r->hdr.last;i++)
				{
					Insert_Storage(tmp_r->rec(i),sla,tmp_r->buf+tmp_r->pos(i)+4,page,0);
				}
			}
		}
	}
	if(tmp_l->hdr.last)
	{ 
		if(tmp_l->hdr.KEY.l==0)
		{ 
			page=New_Storage_Page(sla->n); 
			tmp_l->hdr.KEY.r=0;
			tmp_l->hdr.KEY.l=0;
			Put_Buf(fd+2,page*PAGESIZE,PAGESIZE,(char *)tmp_l);
			Insert_Node(page,sla->n,1); 
			int_to_buf(str,4,(dlong)page);
			for(i=0;i<tmp_l->hdr.last;i++)
			{ 
				if(sla->n)
				{
					seek=root_size+(off_t)(tmp_l->rec(i)-1)*ss.size+key_len+ss.field[sla->n-1].atr.attr->wshift;
					Put_Buf(fd,seek,4,str);
				}
			} 
		} 
		else 
		{ 
			TTree dt(this);
			if((page=dt.Find_Right(sla->n,tmp_l->hdr.KEY.l<0?-tmp_l->hdr.KEY.l:tmp_l->hdr.KEY.l))>0)
			{
				for(i=0;i<tmp_l->hdr.last;i++)
				{
					Insert_Storage(tmp_l->rec(i),sla,tmp_l->buf+tmp_l->pos(i)+4,page,0);
				}
			}
		} 
	} 
	free(val);
	return(page); 
} 
