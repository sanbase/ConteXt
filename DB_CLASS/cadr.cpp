/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:cadr.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term; 
 
void CX_BASE::Cadr_Read(long record,char *buf) 
{ 
	char *ch=NULL; 
	int len,size=0; 
	bzero(buf,len_cadr); 
/* 
	if(buf==cadr) 
		cadr_record=record; 
*/ 
	if(record==0) 
		return; 
	for(int i=1;i<=context->ptm;i++) 
	{ 
		register int n; 
 
		n=i-1; 
		if((len=ss.field[n].l)>0) 
		{ 
			if((ss.field[n].k || ss.field[n].b)&& context->pswd!=CXKEY3)
			{ 
				char *b=NULL; 
				int l;
				if(ss.field[n].k)
					l=Get_Storage(record,i,b);
				else    l=Get_Tree(record,i,b);
				if(ss.field[n].a==X_TEXT) 
					len=256; 
				if(b!=NULL) 
				{ 
					bcopy(b,buf+ss.field[n].atr.attr->cshift,l>len?len:l); 
					free(b); 
				} 
			} 
			else 
			{ 
				if(ss.field[n].m) 
				{
					if(context->pswd==CXKEY6)
						len=8;
					else
						len=4;
				}
				if(size+len<=len_cadr && (ch=Get_Main(record,i))!=NULL) 
					bcopy(ch,buf+ss.field[n].atr.attr->cshift,len); 
			} 
			size+=len; 
		} 
	} 
} 
 
int CX_BASE::Cadr_Read(long record) 
{ 
	if(cadr==NULL) 
		return(-1); 
	Cadr_Read(record,cadr); 
	cadr_record=record; 
	if(hist!=NULL) 
		bcopy(cadr,hist,len_cadr);
	if(share!=NULL)
		share->edit_flag=0;

	return(0); 
} 
 
int CX_BASE::Cadr_Write() 
{ 
	return(Cadr_Write(cadr_record,cadr)); 
} 
 
int CX_BASE::Cadr_Write(char *cadr_buf) 
{ 
	return(Cadr_Write(cadr_record,cadr_buf)); 
} 
 
int CX_BASE::Cadr_Write(long record, char *cadr_buf) 
{ 
	char *ch=NULL;
	int flag=0; 
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
 
	if(cadr_buf==NULL) 
		return(-1); 
	if(record==0) 
	{ 
		record=New_Record(); 
		flag=1; 
	} 
	else 
	{ 
		if(Wlock(record)) 
		{ 
			return(-1); 
		} 
	} 
	if(record==cadr_record) 
		cadr_record=0; 
	if(share!=NULL)
		share->edit_flag=0;
	for(int i=1;i<=context->ptm;i++) 
	{ 
		register int n; 
 
		n=i-1; 
 
		if(ss.field[n].a==0) 
			continue; 
		if(variable(&ss,n)) 
		{ 
			struct fd *ffd; 
			off_t seek0; 
			char *buf=NULL;
			if(ss.field[n].d && !(context->pswd==CXKEY3)) 
			{ 
				if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
					ss.field[n].atr.attr->sfd=fd+(int)ss.field[n].atr.attr->sfd; 
				ffd=ss.field[n].atr.attr->sfd; 
				seek0=(record-1)*ss.field[n].l; 
			} 
			else 
			{ 
				ffd=fd; /* Main */ 
				seek0=root_size+(off_t)(record-1)*ss.size+key_len;
			} 
			int len=ss.field[n].l; 
			if(ss.field[n].m) 
			{
				if(context->pswd==CXKEY6)
					len=8;
				else
					len=4;
			}
			if(var_point==NULL) 
			{ 
				buf=Get_Buf(ffd,seek0+ss.field[n].atr.attr->wshift,len); 
				dlong seek1=int_conv(buf,len); 
 
				if(buf==NULL || seek1!=int_conv(cadr_buf+ss.field[n].atr.attr->cshift,len)) 
				{ 
					if(seek1 && hist==NULL) 
						Free_Space(seek1-1,len); 
					Put_Buf(ffd,seek0+ss.field[n].atr.attr->wshift,len,cadr_buf+ss.field[n].atr.attr->cshift); 
				} 
			} 
		} 
		else if((ss.field[n].k || ss.field[n].b) && context->pswd!=CXKEY3)
		{ 
			int j; 
			if(flag) 
				j=1; 
			else 
			{ 
				char *ch=NULL; 
				if(ss.field[n].k)
					Get_Storage(record,i,ch);
				else
					Get_Tree(record,i,ch);
				if(ch!=NULL) 
				{ 
					if(ss.field[n].a==X_TEXT) 
						j=strncmp(ch,cadr_buf+ss.field[n].atr.attr->cshift,256); 
					else 
						j=memcmp(ch,cadr_buf+ss.field[n].atr.attr->cshift,ss.field[n].l); 
					free(ch); 
				} 
				else    j=1; 
			} 
			if(j!=0) 
			{ 
				if(ss.field[n].k && context->pswd!=CXKEY3 && ss.field[n].a==X_TEXT) 
				{ 
					char *ch=NULL; 
					if((String_To_Buf(cadr_buf+ss.field[n].atr.attr->cshift,ss.field[n],ch,0,0,record))>0 && ch!=NULL) 
						Write(record,n+1,ch); 
					if(ch!=NULL) 
						free(ch); 
				} 
				else Write(record,n+1,cadr_buf+ss.field[n].atr.attr->cshift); 
			} 
 
		} 
		else if(flag || (ch=Get_Main(record,i))==NULL || memcmp(ch,cadr_buf+ss.field[n].atr.attr->cshift,ss.field[n].m?(context->pswd==CXKEY6?8:4):ss.field[n].l))
		{
			Write(record,n+1,cadr_buf+ss.field[n].atr.attr->cshift); 
		}
	} 
	if(cadr_record==0) 
		cadr_record=record; 
	Unlock(record); 
	if(hist!=NULL) 
	{ 
		char str[32]; 
		char *name=(char *)malloc(strlen(__name)+strlen(LOGDB)+3); 
		CX_BASE *h=NULL; 
 
		sprintf(name,"%s/%s",__name,LOGDB); 
		try 
		{ 
			h = new CX_BASE(name); 
		} 
		catch(...) 
		{ 
			free(name); 
			return(record); 
		} 
		free(name); 
 
		CX_FIND f(h); 
		long page; 
 
		sprintf(str,"%d",(int)record); 
		if(f.Find_First(1,str,0)<=0) 
		{ 
			page=h->New_Record(); 
			sprintf(str,"%d",(int)record); 
			h->Put_Slot(page,1,str); 
			get_date(get_act_date(),str); 
			h->Put_Slot(page,2,str); 
			get_time(get_act_time(),str); 
			h->Put_Slot(page,3,str); 
			h->Put_Slot(page,4,"Initial"); 
			h->Put_Slot(page,5,hist,len_cadr);
			h->Unlock(page); 
		} 
		page=h->New_Record(); 
		sprintf(str,"%d",(int)record); 
		h->Put_Slot(page,1,str); 
		get_date(get_act_date(),str); 
		h->Put_Slot(page,2,str); 
		get_time(get_act_time(),str); 
		h->Put_Slot(page,3,str); 
		h->Put_Slot(page,4,GetLogin()); 
		for(int i=0;i<len_cadr;i++)
			 hist[i]=cadr_buf[i]-hist[i];
		h->Put_Slot(page,5,hist,len_cadr);
		h->Unlock(page); 
 
		delete h; 
	} 
	return(record); 
} 
 
void CX_BASE::Roll_Back() 
{ 
	if(cadr==NULL || cadr_record==0) 
		return; 
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	for(int i=1;i<=context->ptm;i++) 
	{ 
		register int n; 
 
		n=i-1; 
 
		if(ss.field[n].a==0) 
			continue; 
		if(variable(&ss,n)) 
		{ 
			struct fd *ffd; 
			off_t seek0=0; 
			char *buf=NULL;
 
			if(ss.field[n].d) 
			{ 
				if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
				{ 
					ss.field[n].atr.attr->sfd=fd+(int)ss.field[n].atr.attr->sfd; 
				} 
				ffd=ss.field[n].atr.attr->sfd; 
				seek0=(cadr_record-(context->pswd==CXKEY3?0:1))*ss.field[n].l; 
			} 
			else 
			{ 
				ffd=fd; /* Main */ 
				seek0=root_size+(off_t)(cadr_record-1)*ss.size+key_len;
				seek0+=ss.field[n].atr.attr->wshift; 
			} 
			int len=ss.field[n].l; 
			if(ss.field[n].m) 
			{
				if(context->pswd==CXKEY6)
					len=8;
				else
					len=4;
			}
			buf=Get_Buf(ffd,seek0,len); 
			if(buf!=NULL && int_conv(buf,len)!=(seek0=int_conv(cadr+ss.field[n].atr.attr->cshift,len))) 
			{ 
				if(seek0>0 && hist==NULL) 
					Free_Space(seek0-1,len); 
			} 
 
		} 
	} 
} 
 
long CX_BASE::current_record() 
{ 
	if(this==NULL) 
		return(0); 
	return(cadr_record); 
} 
 
int CX_BASE::Cadr_Change() 
{ 
	char *cadr_old=NULL;
 
	if(cadr==NULL) 
		return(1); 
	if(cadr_record==0) 
	{ 
		for(int i=0;i<len_cadr;i++) 
			if(cadr[i]) 
				return(1); 
		return(0); 
	} 
	if((cadr_old=(char *)malloc(len_cadr))==NULL) 
		return(-1); 
	Cadr_Read(cadr_record,cadr_old); 
	int i=memcmp(cadr,cadr_old,len_cadr); 
	free(cadr_old); 
	return(i); 
} 
 
 
int CX_BASE::Field_Change(int field) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	if(field<1 || field>ss.ptm) 
		return(0); 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Field_Change(sla)); 
} 
 
int CX_BASE::Field_Change(struct sla *sla) 
{ 
	char *ch1=NULL; 
	char *ch2=NULL; 
	int i=0; 
	long record_std=cadr_record; 
	struct get_field_result res1,res2; 
 
	if(sla->n<1 || sla->n>ss.ptm) 
		return(0); 
	cadr_record=0; 
	res1=Read(record_std,sla,ch1); 
	if(res1.len<0) 
	{ 
		cadr_record=record_std; 
		goto END; 
	} 
	cadr_record=record_std; 
	res2=Read(record_std,sla,ch2); 
	if(res2.len<0) 
		goto END; 
	if(res1.len!=res2.len || ch1==NULL || ch2==NULL) 
		i=1; 
	else 
		i=memcmp(ch1,ch2,res2.field.l)!=0; 
END: 
	if(ch1!=NULL) 
		free(ch1); 
	if(ch2!=NULL) 
		free(ch2); 
	return(i); 
} 
