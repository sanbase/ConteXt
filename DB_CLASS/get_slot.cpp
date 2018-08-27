/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:get_slot.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term;
 
int CX_BASE::Read(char * &cadr) 
{ 
	return(Read(share->record,cadr)); 
} 
 
int CX_BASE::Get_Slot(int field,char * &slot)			   
{ 
	return(Get_Slot(share->record,field,slot)); 
} 
 
int CX_BASE::Get_Slot(char *descr,char * &slot) 
{ 
	return(Get_Slot(share->record,descr,slot)); 
} 
 
int CX_BASE::Get_Slot(struct sla *sla,char * &slot) 
{ 
	return(Get_Slot(share->record,sla,slot)); 
} 
 
struct get_field_result CX_BASE::Read(int field,char * &slot,int atr) 
{ 
	return(Read(share->record,field,slot,atr)); 
} 
 
struct get_field_result CX_BASE::Read(char *descr,char * &slot,int atr) 
{ 
	return(Read(share->record,descr,slot,atr)); 
} 
 
struct get_field_result CX_BASE::Read(struct sla *sla,char * &slot,int atr) 
{ 
	return(Read(share->record,sla,slot,atr)); 
} 
 
int CX_BASE::Read(long record,char * &cadr) 
{ 
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	if((cadr=(char *)realloc(cadr,ss.size-key_len))==NULL)
		return(-1); 
	char *ch=Get_Buf(fd,root_size+(off_t)(record-1)*ss.size+key_len,ss.size-key_len);
	if(ch==NULL) 
		return(-1); 
	bcopy(ch,cadr,ss.size-key_len);
	return(0); 
} 
 
long CX_BASE::Back_Pointer(long record,int field)
{
	struct sla sla[SLA_DEEP];
	bzero(sla,sizeof sla);
	sla[0].n=field;
 
	return(Back_Pointer(record,sla));
}
 
long CX_BASE::Back_Pointer(long record,char *descr)
{
	struct sla sla[SLA_DEEP];
	if(Str_To_Sla(descr,sla))
	{
		return(0);
	}
	return(Back_Pointer(record,sla));
}
 
long CX_BASE::Back_Pointer(long record,struct sla *sla)
{
	if(!ss.field[sla->n-1].m || context->pswd!=CXKEY6)
		return 0;
	char *ch=NULL;

	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[sla->n-1].atr.attr->wshift;

	if((ch=Get_Buf(fd,seek,ss.field[sla->n-1].l))==NULL)
	{
		return 0;
	}
	seek=int_conv(ch,ss.field[sla->n-1].l)-1;
	if((ch=Get_Buf(fd+1,seek+8,4))==NULL)
		return 0;
	return int_conv(ch,4);
}


int CX_BASE::Get_Slot(long record,int field,char * &slot)
{ 
	struct sla sla[SLA_DEEP]; 
	bzero(sla,sizeof sla);
	sla[0].n=field; 
 
	return(Get_Slot(record,sla,slot)); 
} 
 
int CX_BASE::Get_Slot(long record,char *descr,char * &slot) 
{ 
	struct sla sla[SLA_DEEP]; 
	if(Str_To_Sla(descr,sla)) 
	{ 
		return(0); 
	} 
	return(Get_Slot(record,sla,slot)); 
} 
 
int CX_BASE::Get_Slot(long record,struct sla *sla,char * &slot) 
{
	char *ch=NULL; 
	struct get_field_result res; 
 
	if(slot!=NULL) 
		free(slot); 
	slot=NULL; 
#if !defined(SERVER) & defined(DEBUG) 
	char sl[64]; 
	char *cmd=(char *)malloc(strlen(__name)+64); 
	sla_to_str(sla,sl); 
	sprintf(cmd,"%s;%d;%s",__name,(int)record,sl);
	Sock_Message *s; 
	try 
	{ 
		s = new Sock_Message("127.1",5000);
	} 
	catch(...) 
	{ 
		free(cmd); 
		return(-1); 
	} 
	s->WriteMsg(cmd); 
	int len=s->ReadMsg(slot); 
	free(cmd); 
	delete s; 
	return(len); 
#else 
	if(sla->n>ss.ptm) 
		return(Get_Virtual(record,sla,slot));
	for(int i=0;i<SLA_DEEP && sla[i].n;i++) 
	{ 
		if(sla[i].n<0 || sla[i].m<0) 
		{ 
			double a=Total(record,sla); 
			slot=(char *)realloc(slot,32); 
			sprintf(slot,"%f",a); 
			ch=strchr(slot,'.'); 
			if(ch!=NULL) 
			{ 
				i=strlen(slot)-1; 
				while(i>1 && slot[i]=='0') 
					slot[i--]=0; 
			} 
			return(strlen(slot)); 
		} 
	} 
	try 
	{ 
		res=Read(record,sla,ch); 
	} 
	catch(...) 
	{ 
		goto ERR; 
	} 
	if(res.len<=0) 
	{ 
ERR: 
		if(ch!=NULL) 
			free(ch); 
		slot=(char *)calloc(1,1);  
		return(res.len); 
	} 
	if(ss.field[sla->n-1].k && context->pswd!=CXKEY3 && res.field.a==X_TEXT) 
		res.field.a=0; 
 
	if(res.field.a) 
	{ 
		try 
		{ 
			if(res.field.a==X_STRUCTURE || res.field.m) 
			{ 
				int i,j; 
				struct sla SLA[SLA_DEEP]; 
 
				for(i=0;sla[i].n;i++); 
				for(j=0;res.sla[j].n;j++); 
 
				bcopy(sla,SLA,sizeof SLA); 
				i-=j; 
 
				slot=(char *)calloc(2,1); 
				if(res.field.m && SLA[i].m==0) 
				{ 
					res.sla->m=-1; 
					int num=Read(record,res.sla,ch).len; 
 
					for(j=1;j<=num;j++) 
					{ 
						SLA[i].m=j; 
						int rez=Get_Slot(record,SLA,ch)>0;
						slot=(char *)realloc(slot,strlen(slot)+(rez?strlen(ch)+3:3));
						strcat(slot,"[");
						if (rez)
							strcat(slot,ch);
						strcat(slot,"]");
					} 
					if(ch!=NULL) 
						free(ch); 
					return(strlen(slot)); 
				} 
				if(res.field.a==X_STRUCTURE && SLA[i+1].n==0) 
				{ 
					*slot='{'; 
					for(j=1;j<=res.field.st.st->ptm;j++) 
					{ 
						SLA[i+1].n=j; 
						int rez=Get_Slot(record,SLA,ch)>0;
						slot=(char *)realloc(slot,strlen(slot)+(rez?strlen(ch)+3:3));
						if(j>1)
						   strcat(slot,";");
						if (rez)
							strcat(slot,ch); 
					} 
					strcat(slot,"}"); 
					if(ch!=NULL) 
						free(ch); 
					return(strlen(slot)); 
				} 
			} 
			if(res.field.k && context->pswd!=CXKEY3)
			{
				if(sla[1].n)
					res.field.k=0;
				if(res.field.a==X_TEXT)
				{
					res.field.a=X_STRING;
					if(ch!=NULL)
					{
						res.len=strlen(ch);
						res.field.l=res.len;
					}
					else    res.len=0;
				}
			}
			res.len=Buf_To_String(ch,&res,slot);
		} 
		catch(...) 
		{ 
			goto ERR; 
		} 
		if(ch!=NULL) 
		{ 
			free(ch); 
		} 
	} 
	else if(ch!=NULL)       // это VIRTUAL поле
		slot=ch; 
	return(res.len); 
#endif 
} 

 
struct get_field_result CX_BASE::Read(long record,int field,char * &slot,int atr) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	sla[0].n=field; 
	sla[0].m=0; 
	sla[1].n=0; 
	sla[1].m=0; 
	return(Read(record,sla,slot,atr)); 
} 
 
struct get_field_result CX_BASE::Read(long record,char *descr,char * &slot,int atr) 
{ 
	struct sla sla[SLA_DEEP]; 
	static struct get_field_result res; 
 
	if(Str_To_Sla(descr,sla)) 
	{ 
		bzero(&res, sizeof res); 
		return(res); 
	} 
	return(Read(record,sla,slot,atr)); 
} 
 
struct get_field_result CX_BASE::Read(long record,struct sla *sla,char * &slot,int atr) 
{ 
	static struct get_field_result res; 
 
	if(slot!=NULL) 
	{ 
		if(slot!=NULL) 
			free(slot); 
		slot=(char *)malloc(4); 
		*slot=0; 
	} 
	if(sla->n>ss.ptm) 
	{ 
		bzero(&res,sizeof res); 
		res.len=Get_Virtual(record,sla,slot);
//                res.field.s=1; 
		return(res); 
	} 
	if(record>max_record) 
	{ 
		update(); 
		if(record>max_record) 
		{ 
			bzero(&res,sizeof res); 
			res.len=-1; 
			return(res); 
		} 
	} 
	int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	return(Read(record,sla,root_size+(off_t)(record-1)*ss.size+key_len,fd,&ss,slot,atr));
} 
char  *CX_BASE::Get_Main(long record,int field) 
{ 
	struct fd *ffd=fd; 
	off_t seek; 
 
	if(field<1 || field>context->ptm || record<=0) 
		return(NULL); 
 
	field-=1; 
 
	int len=ss.field[field].l; 

	if(ss.field[field].m) 
	{
		if(context->pswd==CXKEY6)
			len=8;
		else
			len=4;
	}
	if(ss.field[field].k && context->pswd!=CXKEY3)
	{ 
		return(Get_Storage(record,field)); 
	} 
	else if(ss.field[field].b)
		return(Get_Tree(record,field));
	else if(ss.field[field].d && !(context->pswd==CXKEY3 && variable(&ss,field))) 
	{ 
		if((unsigned int)ss.field[field].atr.attr->sfd<=(unsigned int)fdnum) 
		{ 
			ss.field[field].atr.attr->sfd=fd+(int)ss.field[field].atr.attr->sfd; 
		} 
		ffd=ss.field[field].atr.attr->sfd; 
		seek=(record-(context->pswd==CXKEY3?0:1))*ss.field[field].l; 
	} 
	else 
	{
		int key_len=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
		seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field].atr.attr->wshift;
	}
	return(Get_Buf(ffd,seek,len)); 
} 
 
struct get_field_result CX_BASE::Read(long record,struct sla *isla,off_t seek,struct fd *ffd,struct st *s,char * &slot, int atr) 
{ 
	struct get_field_result res; 
	register int n; 
	int len=0; 
	char *ch=NULL; 
	struct sla *sla; 
 
	bzero(&res,sizeof res); 
	sla=res.sla; 
	for(int i=0;i<SLA_DEEP && isla[i].n;i++) 
		sla[i]=isla[i]; 
	if(sla->n<1) 
		sla->n=1; 
 
	n=sla->n-1; 
	bcopy(context,&res.context,sizeof res.context); 
	if(sla->n > s->ptm)
	{
		return(res); 
	}
	if(s==&ss && sla->n>ss.ptm) 
	{ 
		res.len=Get_Virtual(record,sla,slot); 
//                res.field.s=1; 
		return(res); 
	} 
	res.field=s->field[n]; 
	if(s->field[n].m || (s->field[n].k && context->pswd!=CXKEY3)) 
	{
		if(context->pswd==CXKEY6) // && s->field[n].m)
			len=8;
		else
			len=4;
	}
	else 
		len=s->field[n].l; 
	if(s->field[n].d && !(context->pswd==CXKEY3 && variable(s,n))) 
	{ 
		if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
		{ 
			ss.field[n].atr.attr->sfd=fd+(int)ss.field[n].atr.attr->sfd; 
		} 
		ffd=s->field[n].atr.attr->sfd; 
		seek=(record-(context->pswd==CXKEY3?0:1))*s->field[n].l; 
	} 
	else 
		seek+=s->field[n].atr.attr->wshift; 
 
	if((ss.field[sla->n-1].k || ss.field[sla->n-1].b) && context->pswd!=CXKEY3)
	{ 
		res.field=ss.field[sla->n-1]; 
		if(cadr!=NULL && record==cadr_record) 
		{ 
			char *ch1=cadr+ss.field[sla->n-1].atr.attr->cshift; 
			if(ss.field[sla->n-1].a==X_TEXT) 
				res.len=256; 
			else    res.len=ss.field[sla->n-1].l; 
			if(slot!=NULL) 
				free(slot); 
			slot=(char *)malloc(res.len); 
			bcopy(ch1,slot,res.len); 
		} 
		else 
		{
			if(ss.field[sla->n-1].k)
				res.len=Get_Storage(record,sla->n,slot);
			else
			res.len=Get_Tree(record,sla->n,slot);
		}
		ch=slot; 
	} 
	else 
	{ 
		if(cadr!=NULL && record==cadr_record && ffd!=fd+1 && (s->field[n].k==0 || context->pswd==CXKEY3)) 
			ch=cadr+s->field[n].atr.attr->cshift; 
		else if((ch=Get_Buf(ffd,seek,len))==NULL)
			goto ERR;
	} 
	if(s->field[n].a==X_COMPLEX) 
	{ 
		int len_pointer=s->field[n].n; 
		long subfield=int_conv(ch,len_pointer); 
 
		CX_BASE *subbase=get_subbase(s,sla); 
 
		if(subbase==NULL)
			goto ERR; 
		s=&subbase->ss; 
 
		sla->n=(short)subfield; 
 
		if(s->field[n].m) 
		{
			if(context->pswd==CXKEY6)
				len=8;
			else
				len=4;
		}
		else 
			len=s->field[n].l; 
 
		if(len>(int)sizeof (long)) 
		{ 
			ffd=fd+1; 
			ch=Get_Buf(ffd,int_conv(ch,len_pointer),sizeof (long)); 
		} 
		else 
			ch+=len_pointer; 
	} 
	if(variable(s,n)) 
	{ 
		int arr=s->field[n].m; 
		int cx5=context->pswd==CXKEY5 || context->pswd==CXKEY6;
		seek=int_conv(ch,len); 
		if(!seek) 
			return(res); 
		seek--; 
 
		if(context->pswd==CXKEY3 && ss.field[n].d) 
		{ 
			if((unsigned int)ss.field[n].atr.attr->sfd<=(unsigned int)fdnum) 
				ss.field[n].atr.attr->sfd=fd+(unsigned int)ss.field[n].atr.attr->sfd; 
			ffd=s->field[n].atr.attr->sfd; 
		} 
		else 
			ffd=fd+1;       // Bank.0 
 
		int bank_size=s->field[n].n; 
		if(bank_size==0 || arr) 
		{
			if(context->pswd==CXKEY6 && arr)
				bank_size=8;
			else
				bank_size=4;
		}
		if((ch=Get_Buf(ffd,seek,4+bank_size))==NULL)
			return(res); 

		struct var var; 
 
		var.length=int_conv(ch,4);
		var.size  =int_conv(ch+4,4);
		len=s->field[n].l; 
		int num=cx5?(var.size-bank_size)/len:var.size; 

		if(arr) 
		{ 
			if(sla->m<0)      // return num_elem 
			{ 
				res.len=num; 
				return(res); 
			} 
			if(cx5 && var.length<var.size)
				goto ERR; // data is corrupted
			if(sla->m==0) 
			{ 
				if((ch=Get_Buf(ffd,seek+bank_size+4,cx5?(len=var.size-bank_size):(len=num*s->field[n].l)))==NULL)
					goto ERR;
				goto END; 
			} 
			if(sla->m>num) 
				return(res); 
			seek+=bank_size+4+(sla->m-1)*s->field[n].l;
		} 
		else 
		{ 
			seek+=(cx5?4+bank_size:4);
			if(cx5) 
				len=var.size-bank_size; 
			else 
				len=var.length; 
		} 
		if((ch=Get_Buf(ffd,seek,len))==NULL)
			goto ERR;
	} 
	if(s->field[n].a==X_STRUCTURE && sla[1].n) 
		return(Read(record,sla+1,seek,ffd,s->field[n].st.st,slot)); 

	if((s->field[n].a==X_POINTER || s->field[n].a==X_VARIANT || s->field[n].a==X_EXPRESSION)
	&& (sla[1].n || context->pswd==CXKEY3))
	{ 
		long subrecord; 
		CX_BASE *subbase=NULL; 
		if(s->field[n].a==X_VARIANT) 
		{ 
			char *name=NULL;
			if((subrecord=int_conv(ch,s->field[n].n))<=0) 
				return(res); 
			if(sla[1].n==1) 
			{ 
				record=subrecord; 
				goto GET_SUB; 
			} 
			if((record=int_conv(ch+s->field[n].n,len-s->field[n].n))==0) 
				return(res); 
			if((subbase=get_subbase(s,sla))==NULL) 
				return(res); 
			subbase->Get_Slot(subrecord,1,name);
			subrecord=record; 
			if(name!=NULL && strchr(name,':')!=NULL) 
			{ 
//#ifndef WIN32 
			       res.len=Get_Remote_Slot(name,sla+1,subrecord,slot); 
			       free(name); 
//#endif 
			       return(res); 
			} 
			try 
			{ 
				if(name==NULL || (subbase=open_db(name))==NULL) 
				{ 
					if(name!=NULL) 
						free(name); 
					return(res); 
				} 
			} 
			catch(...) 
			{ 
				if(name!=NULL) 
					free(name); 
				return(res); 
			} 
			free(name); 
			sla++; 
		} 
		else 
		{ 
			len=s->field[n].l; 
			if((subrecord=int_conv(ch,len))==0) 
				return(res); 

			char *name=NULL;
			if((name=Name_Subbase(sla->n,record))!=NULL)
			{ 
//#ifndef WIN32
				if(strrchr(name,':')!=NULL)
				{ 
					res.len=Get_Remote_Slot(name,sla+1,subrecord,slot); 
					free(name);
					return(res); 
				} 
//#endif
			} 
			if(name!=NULL)
				free(name);

		} 
GET_SUB: 
		if(s->field[n].a==X_EXPRESSION) 
		{ 
			if((subbase=get_subbase(s,sla))==NULL) 
				return(res); 
			subbase->Read(subrecord,1,slot); 
			double a=Expression(record,slot); 
			slot=(char *)realloc(slot,32); 
			sprintf(slot,"%f",a); 
			res.len=len; 
			return(res); 
		} 
		if(subbase==NULL && (subbase=get_subbase(s,sla))==NULL) 
			return(res); 
		if(atr==0 || subbase->is_pointer(sla+1)) 
			return(subbase->Read(subrecord,sla+1,slot,atr)); 
	} 
END: 
	if(slot!=ch) 
	{ 
		if(slot!=NULL) 
			free(slot); 
		slot=(char *)calloc(len+1,1); 
		if(ch!=NULL) 
		{
			int num;
			if(s->field[n].a==X_TEXT && (num=sla->m)!=0 || (num=sla[1].n)!=0)
			{
				int i;
				char *beg=ch;
				for(i=0;i<len && ch[i];i++)
				{
					if(ch[i]=='\n')
					{
						if(--num>0)
							beg=ch+i+1;
						else    break;
					}
				}
				if(num==0)
				{
					len=ch+i-beg;
					bcopy(beg,slot,len);
				}

			}
			else
				bcopy(ch,slot,len);
		}
	} 
	res.len=len; 
	return(res); 
ERR:
	res.len=-1; 
	return(res); 
} 
 
//#ifndef WIN32 
int CX_BASE::Get_Remote_Slot(char *name,struct sla *sla,long record,char * &slot) 
{ 
	int len=0; 
 
	Sock_Message *s; 
	ClientIPsocket *c; 
	int fd; 
 
	char *str=(char *)malloc(len=strlen(name)+1); 
	strcpy(str,name); 
	char *ch=strchr(str,':'); 
	*ch=0; 
	ch++; 
	try 
	{ 
		c = new ClientIPsocket(str,80); 
		s = new Sock_Message(fd=c->fd()); 
	} 
	catch(int i)
	{ 
		free(str);
		return(-1); 
	} 
	if(!s->isValid()) 
	{ 
		free(str); 
		delete s; 
		return(-1); 
	} 
 
	char sl[256];
	sla_to_str(sla,sl); 
 
	char *line=(char *)malloc(len+strlen(sl)+10); 
	sprintf(line,"SLOT %s;%d;%s\n\n",ch,(int)record,sl); 
	free(str); 
 
	if(write(fd,line,strlen(line))>0) 
		len=s->ReadMsg(slot); 
	free(line); 
	delete s; 
	return(len); 
} 
//#endif 
 
double CX_BASE::Total(long record,struct sla *sla) 
{ 
	struct sla tmp[SLA_DEEP]; 
	int i; 
	double a=0; 
	struct field *f; 
 
	bzero(tmp,sizeof tmp); 
	for(i=0;i<SLA_DEEP;i++) 
	{ 
		struct sla TMP[SLA_DEEP]; 
 
		tmp[i].n=sla[i].n; 
		tmp[i].m=sla[i].m; 
		f=Field_Descr(tmp); 
		if(f->m) 
		{ 
			if(sla[i].m<0) 
			{ 
				bcopy(sla,TMP,sizeof TMP); 
				int num=Num_Elem_Array(record,tmp); 
				for(int j=1;j<=num;j++) 
				{ 
					TMP[i].m=j; 
					a+=Total(record,TMP); 
				} 
				break; 
			} 
		} 
		if(f->a==X_STRUCTURE) 
		{ 
			if(sla[i+1].n<=0) 
			{ 
				bcopy(sla,TMP,sizeof TMP); 
				for(int j=1;j<=f->st.st->ptm;j++) 
				{ 
					TMP[i+1].n=j; 
					a+=Total(record,TMP); 
				} 
				break; 
			} 
		} 
		else 
		{ 
			if(f->a==X_INTEGER || f->a==X_UNSIGNED || f->a==X_FLOAT || f->a==X_DOUBLE
			|| f->a==X_POINTER || f->a==X_VARIANT)
			{ 
				char *ch=NULL; 
				struct get_field_result res; 
 
				res=Read(record,tmp,ch); 
 
				if(ch!=NULL) 
				{ 
					switch(res.field.a) 
					{ 
						case X_INTEGER: 
						case X_UNSIGNED: 
						{ 
							dlong i=0; 
 
							if(res.field.a==X_UNSIGNED) 
								bcopy(ch,&i,res.field.l); 
							else 
								i=int_conv(ch,res.field.l); 
							free(ch); 
							if(res.context.pswd==CXKEY4) 
							{ 
								if(i==0) 
									break; 
								if(i>0) 
									i--; 
							} 
							a=(double)i; 
							while(res.field.n>0) 
							{ 
								a/=10.0; 
								res.field.n--; 
							} 
							break; 
						} 
						case X_DOUBLE: 
							a=*(double *)ch; 
							free(ch); 
							break; 
						case X_FLOAT: 
							a=(double)(*(float *)ch); 
							free(ch); 
							break; 
						case X_POINTER: 
						case X_VARIANT: 
						{ 
							long subrecord=0;
							if (res.field.a==X_POINTER)
								subrecord=int_conv(ch,res.field.l);
							else
							{
								subrecord=int_conv(ch+res.field.n,res.field.l-res.field.n);
								i++;
							}
							free(ch); 
							if(subrecord<=0) 
								break; 
							try 
							{ 
								CX_BASE *subbase; 
								if((subbase=open_db(Name_Subbase(tmp,record)))==NULL) 
									break; 
								a=subbase->Total(subrecord,sla+i+1); 
							} 
							catch(...) 
							{ 
								break; 
							} 
						} 
					} 
				} 
			} 
			break; 
		} 
	} 
	return(a); 
} 
 
// for compatability with previous version. 
int CX_BASE::Get_Slot(long record,int field,char **slot) 
{ 
	return(Get_Slot(record,field,*slot)); 
} 
int CX_BASE::Get_Slot(long record,char *descr,char **slot) 
{ 
	return(Get_Slot(record,descr,*slot)); 
} 
int CX_BASE::Get_Slot(long record,struct sla *sla,char **slot) 
{ 
	return(Get_Slot(record,sla,*slot)); 
} 
struct get_field_result CX_BASE::Read(long record,int field,char **slot) 
{ 
	return(Read(record,field,*slot)); 
} 
struct get_field_result CX_BASE::Read(long record,char *descr,char **slot) 
{ 
	return(Read(record,descr,*slot)); 
} 
struct get_field_result CX_BASE::Read(long record,struct sla *sla,char **slot) 
{ 
	return(Read(record,sla,*slot)); 
} 
long CX_BASE::Get_Property_Value(long record,char *attr)
{
	char *tmp=NULL;
	long rez=0;
	if (Get_Property_Slot(record,attr,tmp,1)>0)
	rez=atol(tmp+1);
	free(tmp);
	return rez;
}
int CX_BASE::Get_Property_Slot(long record,char *attr,char * &slot,int flag_point)
{
	char *str=NULL;
	CX_BASE *property=NULL,*flex=NULL;
	if(slot!=NULL)
		free(slot);
	slot=(char *)malloc(2);
	slot[0]=0;


	if(attr==NULL || !*attr||record<=0)
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
	if (flex==NULL||property==NULL)
		return 0;

	str=(char *)realloc(str,strlen(__name)+strlen(attr)+32);
	sprintf(str,"%s/%s",PROPERTY,attr);
 
	CX_FIND f=CX_FIND(flex);
	long page=f.Find_First(1,str,0);
	if(page<=0)     // such attribute is not exist
		return(0);
 
	selection psel;
	sprintf(str,"#%ld",record);
	psel.clean();
	if (property->Select(1,str,&psel)<=0)
	return 0;
	char *ch=NULL;
	long tmpl=0;
	for(int i=1;i<=psel.length();i++)
	{
		property->Read(psel.Index(i),2,ch);
		if(ch!=NULL && int_conv(ch,2)==page)
		{
			tmpl=int_conv(ch+2,4);
			break;
		}
	}
	if(ch!=NULL)
		free(ch);

	if (tmpl<=0)
		return 0;
	if (flag_point)
	{
		slot=(char *)realloc(slot,32);
		sprintf(slot,"#%li",tmpl);
		return strlen(slot);
	}

	sprintf(str,"%s/%s/%s",__name,PROPERTY,attr);
	flex=open_db(str);
	if (flex==NULL)
		return 0;
	if (str!=NULL)
		free(str);
	return (flex->Get_Slot(tmpl,1,slot)) ;
}
 
// independent methods working through CX_httpd
#ifndef WIN32 
 
int Get_Slot(char *folder,long record,int field,char *&slot) 
{ 
	char descr[32]; 
	sprintf(descr,"^%d",field); 
	return(Get_Slot(folder,record,descr,slot)); 
} 
int Get_Slot(char *folder,long record,struct sla *sla,char *&slot) 
{ 
	char descr[32]; 
	sla_to_str(sla,descr); 
	return(Get_Slot(folder,record,descr,slot)); 
} 
 
int Get_Slot(char *folder,long record,char *descr,char *&slot) 
{ 
	Sock_Message *s=NULL; 
	ClientIPsocket *c=NULL; 
	char *buf,*cmd,*ch; 
 
	if(folder==NULL) 
		return(-1); 
	buf=(char *)malloc(strlen(folder)+1); 
	strcpy(buf,folder); 
	try 
	{ 
		if((ch=strchr(buf,':'))==NULL) 
		{ 
			c = new ClientIPsocket("127.1",80); 
			ch=buf; 
		} 
		else 
		{ 
			*ch=0; 
			c = new ClientIPsocket(buf,80); 
			ch++; 
		} 
	} 
	catch(int i)
	{ 
		return(-2); 
	} 
	try 
	{ 
		s = new Sock_Message(c->fd()); 
	} 
	catch(...) 
	{ 
		delete c; 
		return(-1); 
	} 
 
	cmd=(char *)malloc(strlen(ch)+strlen(descr)+32); 
	sprintf(cmd,"SLOT %s;%d;%s\n\n",ch,(int)record,descr); 
	free(buf); 
 
	if(write(c->fd(),cmd,strlen(cmd))<0) 
	{ 
		free(cmd); 
		return(-1); 
	} 
	free(cmd); 
	int len=s->ReadMsg(5,slot); 
 
	delete s; 
	delete c; 
	return(len); 
} 
#endif 
