/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:put_slot.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
extern class Terminal *term;
int Compare(struct field *des_field,char *ch1,char *ch2,int len); 
static struct field *fld; 

extern int cmp_type(struct field *des_field,char *ch1,char *ch2); 
static int cmp(const void *ch1,const void *ch2) 
{ 
	return(cmp_type(fld,(char *)ch1,(char *)ch2)); 
} 
 
int CX_BASE::Put_Slot(int field,char *slot,int field_size) 
{ 
	return(Put_Slot(share->record,field,slot,field_size)); 
} 
int CX_BASE::Put_Slot(char *descr,char *slot,int field_size) 
{ 
	if(share==NULL) 
		return(-1); 
	return(Put_Slot(share->record,descr,slot,field_size)); 
} 
int CX_BASE::Put_Slot(struct sla *sla,char *slot,int field_size) 
{ 
	if(share==NULL) 
		return(-1); 
	return(Put_Slot(share->record,sla,slot,field_size)); 
} 
int CX_BASE::Write(char *descr,char *slot) 
{ 
	if(share==NULL) 
		return(-1); 
	return(Write(share->record,descr,slot)); 
} 
int CX_BASE::Write(int field,char *slot) 
{ 
	if(share==NULL) 
		return(-1); 
	return(Write(share->record,field,slot)); 
} 
int CX_BASE::Write(struct sla *sla,char *slot) 
{ 
	if(share==NULL) 
		return(-1); 
	return(Write(share->record,sla,slot)); 
} 
 
int CX_BASE::Put_Slot(long record,int field,char *slot, int field_size) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Put_Slot(record,sla,slot,field_size)); 
} 
 
int CX_BASE::Put_Slot(long record,char *descr,char *slot,int field_size) 
{ 
	struct sla sla[SLA_DEEP]; 
	if(Str_To_Sla(descr,sla)) 
		return(-1); 
	return(Put_Slot(record,sla,slot,field_size)); 
}
 
int CX_BASE::Put_Slot(long record,struct sla *sla,char *slot,int field_size) 
{ 
	char *ch,str[256];
	struct field *field; 

	int i; 
	ch=NULL; 
	if(sla->n>ss.ptm)
		return(0);
	if(record>max_record)
		update();
	if(record>max_record || (record>0 && Check_Del(record)))
		return(-1);
	if(slot==NULL) 
	{ 
		return(Write(record,sla,NULL)); 
	} 
	for(i=0;i<SLA_DEEP && sla[i].n;i++) 
		if(sla[i].n<0) 
			return(0); 
	field=Get_Field_Descr(&ss,sla); 
	if(field->a==X_BINARY && (strstr(__name,LOGDB)!=NULL || strstr(__name,"_HistDB")))
		i=0;
	else
	i=1;

       /* if((field->a==X_BINARY && strstr(__name,LOGDB)!=NULL) || field->a==X_IMAGE)
		i=-1; 
	else    i=context->b0&NO_COMPRESS;*/

	if((field->a==X_POINTER || is_pointer(sla)) && *slot && *slot!='#')
	{ 
		int shift=0;
		CX_BASE *subbase=get_subbase(&ss,sla,&shift);

		shift+=1;
		if(subbase==NULL || subbase->__name==0) 
			return(-2); 

		CX_FIND f(subbase); 
		subbase->Wlock(0); 
		long subrecord=f.Find_First(sla+shift,slot,0);

		if(subrecord<=0) 
		{ 
			int rez=0;
			if((subrecord=subbase->New_Record(1))>0) 
			{ 
				rez=subbase->Put_Slot(subrecord,sla+shift,slot);
				subbase->Unlock(subrecord); 
			} 
		} 
		subbase->Unlock(0); 
		sprintf(str,"#%ld",subrecord);
		slot=str; 
	} 
	if(*slot=='#') 
	{ 
		struct sla SLA[SLA_DEEP]; 
		bzero(SLA,sizeof SLA); 
		for(int j=0;sla[j].n;j++) 
		{ 
			SLA[j]=sla[j]; 
			if((field=Get_Field_Descr(&ss,SLA))->a==X_POINTER)
				break; 
		} 

		if((String_To_Buf(slot,*field,ch,i,field_size,record))<0 || ch==NULL) 
			return(-40);
		i=Write(record,SLA,ch); 
	} 
	else 
	{ 
		if(field->m && sla->m==0) 
			return(-16); 
		if(((i=String_To_Buf(slot,*field,ch,i,field_size,record)))<0 || ch==NULL) 
		{ 
			if(i==-2) 
				return(-5); 
			return(-44);
		} 

		i=Write(record,sla,ch); 
	} 
	if(ch!=NULL && ch!=slot) 
		free(ch); 
	return(i); 
} 
 
int CX_BASE::Write(long record,char *descr,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	if(Str_To_Sla(descr,sla)) 
		return(-5); 
	return(Write(record,sla,slot)); 
} 
 
int CX_BASE::Write(long record,int field,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla[0].n=field; 
 
	return(Write(record,sla,slot)); 
} 
 
int CX_BASE::Write(long record,struct sla *sla,char *slot) 
{ 
	if(sla->n>ss.ptm) 
		return(0); 
	if(record>max_record) 
	{ 
		update(); 
		if(record>max_record) 
			return(-6); 
	} 
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);

	return(Write(record,sla,(root_size+(off_t)(record-1)*ss.size+key_len),fd,&ss,slot,NULL,0));
} 
 
int CX_BASE::Write(long record,struct sla *sla,off_t seek,struct fd *ffd,struct st *s,char *slot,char *out_buf,int out_seek) 
{ 
	char *ch; 
	int locked=0; 
	int tree=0; 
	off_t seek0=0; 
	int rlock=record; 
	register int n=sla->n-1; 

	if(n<0)
		n=0;

	if(change_db)
		write_change(record,n);

	int dtree=((s->field[n].k || s->field[n].b)&& context->pswd!=CXKEY3);
 
	int len=s->field[n].l; 
 
	if(s==&ss && dtree==0) 
		tree=open_Tree(sla); 
 
	if(n<0 || sla->n > s->ptm || record<0)    // Virtual field 
		return(-1); 
	if(s->field[n].m || (dtree && (cadr==NULL || record!=cadr_record)))
	{
		if(context->pswd==CXKEY6 && s->field[n].m)
			len=8;          // this is the size of the pointer to Bank.0
		else
			len=4;
	}
	if(s->field[n].d && out_buf==NULL) 
	{ 
		if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
			ss.field[n].atr.attr->sfd=fd+(int)ss.field[n].atr.attr->sfd; 

		ffd=s->field[n].atr.attr->sfd; 
		seek=(record-(context->pswd==CXKEY3?0:1))*s->field[n].l; 
	} 
	else 
		seek+=s->field[n].atr.attr->wshift; 
	out_seek+=s->field[n].atr.attr->wshift; 
	if(cadr!=NULL && record==cadr_record && ffd!=fd+1) 
	{ 
		ch=cadr+s->field[n].atr.attr->cshift; 
	} 
	else 
	{ 
		if(rlock!=0) 
		{ 
			if(Wlock(rlock))
				return(-7); 
			locked=1; 
		} 
		ch=Get_Buf(ffd,seek,len); 
	} 
	if(variable(s,n)) 
	{ 
		int arr=s->field[n].m; 
		int cx5=context->pswd==CXKEY5 || context->pswd==CXKEY6;
		int num=0;
		struct fd *fdb; 
		off_t seek_new=0,seek_std=0;
		int size=0;
		char *buf=NULL; 
		int keep_space=0; 
 

		if(Wlock(0,1)<0) 
		{ 
			if(locked) 
				Unlock(rlock); 
			return(0); 
		} 
		int bank_size=s->field[n].n; 
		if(bank_size==0 || arr) 
		{
			if(context->pswd==CXKEY6)
				bank_size=8;
			else
				bank_size=4;
		}
		if(ch!=NULL) 
			seek_std=int_conv(ch,bank_size);  // it is not correct but it will work
		else    seek_std=0; 
 
		if(context->pswd==CXKEY3 && ss.field[n].d) 
		{ 
			if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
				ss.field[n].atr.attr->sfd=fd+(int)ss.field[n].atr.attr->sfd; 

			fdb=s->field[n].atr.attr->sfd; 
 
			if(fdb!=fd+1)   // прямая ссылка. Пока не поддерживается. 
			{ 
				if(locked) 
					Unlock(rlock); 
				return(0); 
			} 
		} 
		else 
			fdb=fd+1;       // Bank.0 
		if((seek0=seek_std)>0) 
		{ 
			if((ch=Get_Buf(fdb,seek0-1,bank_size+4))==NULL)  // read value from Bank.0
			{ 
				size=0; 
				num=0; 
				seek0=0; 
			} 
			else 
			{ 
				size=int_conv(ch,4);
				num= int_conv(ch+4,4);
				if(!cx5) 
					num=num*s->field[n].l+bank_size; 
			} 
		} 
		if(arr && sla->m) 
		{ 
			if(seek0==0 || sla->m>(num-bank_size)/s->field[n].l)
			{               // we should create new element of array
NEWSEG: 
				off_t size;
				if((buf=(char *)calloc(size=(sla->m)*s->field[n].l+bank_size,1))==NULL)
				{ 
					Unlock(0,1); 
					if(locked) 
						Unlock(rlock); 
					return(-8); 
				} 
				if(!cx5) 
					size=sla->m; 
				bcopy(&size,buf,4);

				if(context->pswd==CXKEY6)
					bcopy(&record,buf+4,4);

				if(seek0>0 && num>(int)sizeof (long)) 
				{ 
					if((ch=Get_Buf(fdb,seek0-1+4+bank_size,num-bank_size))!=NULL)
						bcopy(ch,buf+bank_size,num-bank_size); 
				} 
			} 
			else 
			{       // the current hole has enough space for the new information 
				if((buf=(char *)malloc(num))==NULL) 
				{ 
					Unlock(0,1); 
					if(locked) 
						Unlock(rlock); 
					return(-9); 
				} 
				if((ch=Get_Buf(fdb,seek0-1+4,num))==NULL)
				{ 
					free(buf); 
					goto NEWSEG; 
				} 
				bcopy(ch,buf,num); 

 
				if((seek0!=seek_std && hist==NULL) || cadr==NULL)
					keep_space=1; 
			} 
			int out_s=(sla->m-1)*s->field[n].l+bank_size;

			if(s->field[n].a==X_STRUCTURE && sla[1].n) 
				Write(record,sla+1,seek0-1+4+out_s,fdb,s->field[n].st.st,slot,buf,out_s);
			else if(slot!=NULL) 
			{ 
				if(arr==SET) 
				{ 
					int num_el=(*(long *)buf-bank_size-(context->pswd==CXKEY6)?4:0)/s->field[n].l;
					char *ch1=buf+bank_size; 
 
					fld=s->field+n; 
					for(int i=0;i<num_el;i++,ch1+=fld->l) 
					{ 
						if(!Compare((struct field *)fld,(char *)slot,(char *)ch1,(int)fld->l)) 
						{ 
							Unlock(0,1); 
							return(len); 
						} 
					} 
				} 
				bcopy(slot,buf+out_s,s->field[n].l); 
			} 
			else 
			{ 
				bzero(buf+out_s,s->field[n].l); 
			} 
			if(arr==LIST) 
			{ 
				fld=s->field+n; 
				int bs=bank_size; //+(context->pswd==CXKEY6)?4:0;
				qsort(buf+bs,(*(long *)buf-bs)/s->field[n].l,s->field[n].l,cmp);
			} 
 
			slot=buf; 
		} 
		if(arr && sla->m==0 && sla[1].n) 
		{ 
			Unlock(0,1); 
			if(locked) 
				Unlock(rlock); 
			return(-10); 
		} 

		int size_new=int_conv(slot,4);
		if(!cx5 && slot==buf) 
			size_new=size_new*s->field[n].l+bank_size; 
		if(size_new==0) 
			seek_new=0; 
		else if(keep_space) 
		{ 
			char *buf=(char *)calloc(size_new+bank_size,1); 
			if(cx5 || arr) 
				bcopy(&size_new,buf,4);
#ifdef SPARC 
			conv(buf,bank_size);
#endif 

			if(slot!=NULL) 
			{
//                                bcopy((cx5 || slot==buf)?slot:slot+bank_size,(cx5||slot==buf)?buf+bank_size:buf,size_new);

				bcopy((cx5 || slot==buf)?slot:slot+4,(cx5||slot==buf)?buf+4:buf,size_new);
			}
			else 
				bzero(buf+bank_size,size_new); 
			char *ch=Get_Buf(fdb,seek0-1,size_new+bank_size); 

			if(ch==NULL || bcmp(ch,buf,size_new+bank_size)) 
				Put_Buf(fdb,seek0-1,size_new+bank_size,buf); 
			free(buf); 
			seek_new=seek0; 
		} 
		else
		{
			if((seek_new=Write_Text((cx5||slot==buf)?slot:slot+4,size_new,4))<0)
			{
				if(buf!=NULL)
					free(buf);
				Unlock(0,1);
				if(locked)
					Unlock(rlock);
				return(seek_new);
			}
		}
		if(buf!=NULL) 
			free(buf); 
		if(!keep_space && seek0) 
		{ 
			if(cadr==NULL || record!=cadr_record)
			{
				Free_Space(seek0-1,bank_size); 
			}
			else 
			{ 
				char *ch1=Get_Buf(ffd,seek,bank_size); 
				if(ch1!=NULL) 
					seek_std=int_conv(ch1,bank_size);
				else    seek_std=0; 
				if(seek0!=seek_std)
					Free_Space(seek0-1,bank_size); 
			} 
		} 
		seek0=seek_new; 
		slot=(char *)&seek0; 
#ifdef SPARC 
		conv(slot,sizeof (long)); 
#endif 
		Unlock(0,1); 
	} 
	else if(s->field[n].a==X_STRUCTURE && sla[1].n) 
	{ 
		len=Write(record,sla+1,seek,ffd,s->field[n].st.st,slot,out_buf,out_seek);
		if(locked) 
			Unlock(rlock); 
		return(len); 
	} 
	else if(((s->field[n].a==X_POINTER || s->field[n].a==X_VARIANT || s->field[n].a==X_EXPRESSION) && sla[1].n))
	{ 
		CX_BASE *subbase=get_subbase(s,sla); 
		if(subbase==NULL || subbase==this || subbase->__name==NULL) 
		{ 
			if(locked) 
				Unlock(rlock); 
			return(-13); 
		} 
		if(s->field[n].a==X_VARIANT) 
		{ 
			if(sla[1].n==1) 
				goto END; 
			int subrecord=int_conv(ch,s->field[n].n); 
			if((record=int_conv(ch+s->field[n].n,len-s->field[n].n))==0) 
			{ 
				if(locked) 
					Unlock(rlock); 
				return(0); 
			} 
			char *name=NULL; 
			subbase->Get_Slot(subrecord,1,name); 
 
			if((subbase=open_db(name))==NULL) 
			{ 
				if(name!=NULL) 
					free(name); 
				if(locked) 
					Unlock(rlock); 
				return(-14); 
			} 
			if(name!=NULL) 
				free(name); 
			sla++; 
		} 
		else 
		{ 
			record=int_conv(ch,len); 
			if(record==0) 
			{ 
				if(locked) 
					Unlock(rlock); 
				return(0); 
			} 
		} 
 
		if(subbase==NULL || subbase==this || subbase->__name==0) 
		{ 
			if(locked) 
				Unlock(rlock); 
			return(-15); 
		} 
		int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
		len=subbase->Write(record,sla+1,(subbase->root_size+(off_t)(record-1)*subbase->ss.size+key_len),subbase->fd,&subbase->ss,slot,out_buf,out_seek);
		if(locked) 
			Unlock(rlock); 
		return(len); 
	} 
END: 
	if(out_buf!=NULL) 
	{ 
		if(slot!=NULL) 
			bcopy(slot,out_buf+out_seek,len); 
		else 
			bzero(out_buf+out_seek,len); 
		if(locked) 
			Unlock(rlock); 
		return(len); 
	} 
	if(cadr!=NULL && record==cadr_record && ffd!=fd+1) 
	{ 
		if(dtree && s->field[n].a==X_TEXT) 
		{ 
			bzero(cadr+s->field[n].atr.attr->cshift,255); 
			if(slot!=NULL) 
			{ 
				int l=s->field[n].n==0?4:s->field[n].n; 
				len=int_conv(slot,l)-l; 
				slot+=l; 
				if(len>250) 
					len=250; 
			} 
		} 
		if(slot!=NULL) 
			bcopy(slot,cadr+s->field[n].atr.attr->cshift,len); 
		else 
			bzero(cadr+s->field[n].atr.attr->cshift,len); 
	} 
	else 
	{ 
		if(dtree) 
		{ 
			if(s->field[n].k)
				Put_Storage(record,sla,slot);
			else
				Put_Tree(record,sla,slot);
			if(locked) 
				Unlock(rlock); 
			return(len); 
		} 
		ch=Get_Buf(ffd,seek,len); 
		if(ch==NULL || bcmp(ch,slot,len))
		{ 
			int deleted_tree=0; 
			Wlock(0); 
			if(tree>0 && (cadr==NULL || record!=cadr_record))
			{ 
				deleted_tree=1; 
				if(ch!=NULL) 
					Delete_Node(record,sla); 
 
			} 
			Put_Buf(ffd,seek,len,slot); 
			if(deleted_tree) 
				Insert_Node(record,sla); 

			Unlock(0); 

		} 
	} 
	if(locked) 
		Unlock(rlock); 
	if(cadr!=NULL && record==cadr_record && share!=NULL)
		share->edit_flag=1;
	return(len); 
} 
 
off_t CX_BASE::Write_Text(char *slot,int len,int l)
{ 
	off_t seek; 
	int len0; 
	char *buf; 

	len0=len; 

	seek=Find_Space(&len0); 
 
	if((buf=(char *)calloc(len+4,1))==NULL)
		return(-14); 
	bcopy(&len0,buf,4);
#ifdef SPARC 
	conv(buf,4);
#endif 
	if(slot!=NULL) 
		bcopy(slot,buf+l,len);
	else 
		bzero(buf+l,len);
	Put_Buf(fd+1,seek,len+l,buf);
	free(buf); 
	return(seek+1); 
} 
 
off_t CX_BASE::Find_Space(int *len) 
{ 
	off_t seek; 
	struct stat st; 
	char *str; 
	long record; 
 
	str=(char *)malloc(strlen(__name)+strlen(SPACEDB)+2); 
	sprintf(str,"%s/%s",__name,SPACEDB);
 
	CX_BASE *Space=NULL; 
	try 
	{ 
		Space=open_db(str); 
	} 
	catch(...) 
	{ 
		Space=NULL; 
		goto SEEK_TO_END; 
	} 
	if(Space==NULL)
		goto SEEK_TO_END;
	if(Space->Wlock(0)<0) 
	{ 
		Space=NULL; 
		goto SEEK_TO_END; 
	} 
 
	if(!fd[1].Fd) 
		open_FD(fd+1); 
	if(Space->Access()<1) 
	{ 
SEEK_TO_END: 
		fstat(fd[1].Fd,&st); 
		fd[1].fsize=st.st_size; 
		free(str); 
		if(Space!=NULL) 
			Space->Unlock(0); 
		return(fd[1].fsize); 
	} 
 
	sprintf(str,"%d",*len);
 
FIND_SPACE: 
	CX_FIND *Sp = new CX_FIND(Space); 
	record=Sp->Find_First(1,str,2); 
	delete(Sp); 
 
	if(record<=0) 
		goto SEEK_TO_END; 

	int l=0;
	Space->Get_Slot(record,1,str); 
	l=atoi(str);
 
	seek=0; 
	Space->Get_Slot(record,2,str); 
	seek=atoll(str); 
 
	Space->Delete(record); 
	if(seek<0) 
		goto SEEK_TO_END; 
	if(seek>fd[1].fsize) 
	{ 
		fstat(fd[1].Fd,&st); 
		fd[1].fsize=st.st_size; 
		if(seek>fd[1].fsize) 
			goto SEEK_TO_END; 
	} 
	char *buf=Get_Buf(fd+1,seek,sizeof (long)); 
	if(buf==NULL) 
		goto FIND_SPACE; 
	int size=int_conv(buf,sizeof(long)); 

	if(-size!=l)
		goto SEEK_TO_END;
	*len=l;
	Space->Unlock(0); 
	free(str); 
	return(seek); 
} 
 
int CX_BASE::Free_Space(off_t seek,int bank_size)
{ 
	char *buf; 
	struct stat st; 
	int len=0; 
	long record; 
 
	if(seek<0 || Wlock(0)) 
		return(-15); 
	if(!fd[1].Fd) 
		open_FD(fd+1); 
	if(seek>fd[1].fsize) 
	{ 
		fstat(fd[1].Fd,&st);
		fd[1].fsize=st.st_size; 
	} 
	if(seek>fd[1].fsize) 
	{ 
		Unlock(0);
		return(-16); 
	}
	buf=Get_Buf(fd+1,seek,4);
	len=(long)int_conv(buf,4);
	if(len==0) 
	{ 
		Unlock(0); 
		return(0); 
	} 
	len=-len; 
#ifdef SPARC 
	conv((char *)&len,4);
#endif 
 
	Put_Buf(fd+1,seek,4,(char *)&len);
 
	char *str; 
 
	str=(char *)malloc(strlen(__name)+strlen(SPACEDB)+16); 
	sprintf(str,"%s/%s",__name,SPACEDB);
 
	CX_BASE *Space; 
 
	try 
	{ 
		Space = open_db(str); 
	} 
	catch(...) 
	{ 
		Unlock(0); 
		return(-17); 
	} 
	if(Space==NULL)
	{
		Unlock(0);
		return(-17);
	}
	len=-len; 
 
	if(Space->Wlock(0)) 
	{ 
		Unlock(0); 
		return(-20); 
	} 
	record=Space->New_Record(1); 
	sprintf(str,"%d",len); 
	Space->Put_Slot(record,1,str); 
	sprintf(str,"%lld",seek); 
	Space->Put_Slot(record,2,str); 
	free(str); 
	Space->Unlock(record); 
	Space->Unlock(0); 
 
	Unlock(0); 
	return(0); 
} 
 
int CX_BASE::Insert_Element(long record,int field,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Insert_Element(record,sla,slot)); 
} 
 
int CX_BASE::Insert_Element(long record,char *descr,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
	if(Str_To_Sla(descr,sla)) 
		return(-18); 
	return(Insert_Element(record,sla,slot)); 
} 
 
int CX_BASE::Insert_Element(long record,struct sla *SLA,char *slot) 
{ 
	struct sla sla[SLA_DEEP]; 
	int m=0; 
 
	bzero(sla,sizeof sla);
	bcopy(SLA,sla,sizeof sla); 
	int array; 
	for(array=0;array<SLA_DEEP && sla[array].n;array++); // looking for end of sla
	if(!array) 
		return(-1); 

	for(--array;;array--) 
	{ 
		sla[array].m=0; 
		if(Field_Descr(sla)->m) 
			break; 
		if(!array) 
			return(-2); 
		sla[array].n=0; 
	} 
	m=Num_Elem_Array(record,sla)+1; 
	bcopy(SLA,sla,sizeof sla); 
	sla[array].m=m; 
	return((array=Put_Slot(record,sla,slot))<0?array:m); 
} 
 

int CX_BASE::Put_Property(long record, char *name, char *value,int attr)
{
	char *str=NULL;
	CX_BASE *property,*flex;
	if(name==NULL || !*name)
		return(-1);
	try
	{
		str=(char *)malloc(strlen(__name)+32);
		sprintf(str,"%s/%s",__name,PROPERTY);
		property=open_db(str);
		strcat(str,"/");
		strcat(str,FLEXDB);
		flex=open_db(str);
	}
	catch(...)
	{
		return(-1);
	}
	str=(char *)realloc(str,strlen(__name)+strlen(name)+32);
	sprintf(str,"%s/%s",PROPERTY,name);
	char *item_db=(char *)malloc(strlen(str)+32);
	sprintf(item_db,"%s/%s",__name,str);
 
	CX_FIND f=CX_FIND(flex);
	long page=f.Find_First(1,str,0);
	if(page<=0)     // no such record in property class.
	{
		if(!if_base("",item_db))      // database doesn't exist
		{
			struct st st;
 
			st.ptm=1;
			st.field=(struct field *)calloc(1,sizeof (struct field));
			st.field->a=X_STRING;
			st.field->l=128;
			create_class(&st,item_db,1);
			free(st.field);
		}
		page=flex->New_Record(1);
		flex->Put_Slot(page,1,str);
		flex->Unlock(page);
	}
 
	CX_BASE *item=open_db(item_db);
	free(item_db);
	f=CX_FIND(item);
	long spage=0;
	if (*value!='#')
	{
		spage=f.Find_First(1,value,0);
		if (spage<=0)
		{
			spage=item->New_Record(1);
			item->Put_Slot(spage,1,value);
			item->Unlock(spage);
		}
	}
	else
		spage=atoi(value+1);
 
	long ppage=0;
	if (attr)
	{
		selection sel;
		char *ch=NULL;
		sprintf(str,"#%ld",record);
		property->Select(1,str,&sel);
		for(int i=1;i<=sel.length();i++)
		{
			property->Read(sel.Index(i),2,ch);
			if (ch!=NULL&&int_conv(ch,2)==page)
			{
			     ppage=sel.Index(i);
			     break;
			}
		}
		if (ch!=NULL)
		free(ch);

	}
	if (ppage<=0)
	{
		ppage=property->New_Record(1);
		sprintf(str,"#%ld",record);
		property->Put_Slot(ppage,1,str);
	}

	sprintf(str,"#%ld:%ld",page,spage);
	property->Put_Slot(ppage,2,str);
	property->Unlock(ppage);
	free(str);
 
	return(0);
}
