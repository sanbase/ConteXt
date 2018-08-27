/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:init.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifndef WIN32 
#include <sys/param.h> 
#include <sys/wait.h> 
#include <sys/mount.h>
#endif 
 
#ifdef SPARC 
void conv_header(char *header,int len,int atr) 
{ 
	long num; 
	char *ch; 
 
	conv(header,4); 
	if(atr) 
	{ 
		bcopy(header+4,&num,4); 
		conv(header+4,4); 
	} 
	else 
	{ 
		conv(header+4,4); 
		bcopy(header+4,&num,4); 
	} 
	ch=header+16; 
	for(int i=0;i<num;i++,ch+=16) 
	{ 
		struct field *fld=(struct field *)ch; 
		char ch1=ch[1]; 
		if(!atr) 
		{ 
			ch[1]=(ch1&0xf)<<4; 
			ch[1]&=0xf0; 
			for(int j=4;j<8;j++) 
			{ 
				if((ch1>>j)&1) 
					ch[1]|=(1<<(7-j)); 
			} 
		} 
		else 
		{ 
			ch[1]=ch1>>4; 
			ch[1]&=0xf; 
			ch1&=0xf; 
			for(int j=0;j<4;j++) 
			{ 
				if((ch1>>j)&1) 
					ch[1]|=(1<<(7-j)); 
			} 
		} 
		conv(ch+2,2); 
		conv(ch+4,4); 
		conv(ch+8,4); 
		conv(ch+12,4); 
		if(*ch==X_STRUCTURE) 
		{ 
			int snum=0; 
			bcopy(ch+4,&snum,4); 
			if(atr) 
				conv((char *)&snum,4); 
			num+=snum; 
		} 
	} 
} 
#endif 
 
void CHL_handler(int sig) 
{ 
	int status=sig; 
#ifndef WIN32 
	waitpid(-1, &status, WNOHANG); 
	signal(SIGCHLD,CHL_handler); 
#endif 
} 
 
int CX_BASE::open(char *path, int flags, int mode)
{
	char name[256];
	strncpy(name,path,255);
	decode(name);
	return ::open(path,flags,mode);
}
 
void CX_BASE::close_tree(struct st *s) 
{ 
	for(int i=0;i<(int)s->ptm;i++) 
	{ 
		if(s->field[i].atr.attr->tree_fd>1) 
			close(s->field[i].atr.attr->tree_fd); 
		s->field[i].atr.attr->tree_fd=0; 
		if(s->field[i].a==X_STRUCTURE && s->field[i].st.st!=NULL) 
			close_tree(s->field[i].st.st); 
	} 
} 
 
void CX_BASE::free_substruct(struct st *s) 
{ 
	int i; 
 
	for(i=0;i<(int)s->ptm;i++) 
	{ 
		if(s->field[i].atr.attr!=NULL) 
			free(s->field[i].atr.attr); 
		if(s->field[i].st.st!=NULL) 
		{ 
			free_substruct(s->field[i].st.st); 
			free(s->field[i].st.st); 
			s->field[i].st.st=NULL; 
		} 
	} 
} 

CX_BASE::~CX_BASE() 
{ 
	if(__name==NULL || *__name==0) 
		return; 
	if(change_db)
		delete change_db;

	for(int i=0;i<num_btree;i++)
		delete btree[i];
	if(btree!=NULL)
		free(btree);

	Erase_Index(index_level);
	if(*idx_name) 
		unlink(idx_name); 
	if(index_buf!=NULL) 
		free(index_buf); 
	if(hist!=NULL) 
	       free(hist); 
	if(var_point!=NULL) 
		delete var_point; 
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
	if(key_field!=NULL) 
		free(key_field); 
	bzero(this,sizeof (CX_BASE)); 

} 
 
void CX_BASE::close_FD() 
{ 
	int i; 
#ifndef SERVER 
	for(i=0;i<num_open_bases;i++)
	{ 
		if(open_base[i]) 
			delete open_base[i]; 
	}
	if(open_base!=NULL) 
	       free(open_base); 
	num_open_bases=0; 
	open_base=NULL; 
#endif 
	close_tree(&ss); 
	for(i=0;i<fdnum;i++) 
	{ 
		if(fd[i].in_memory && fd[i].buf!=NULL && fd[i].Fd>0)
		{
			lseek(fd[i].Fd,0,SEEK_SET);
			write(fd[i].Fd,fd[i].buf,fd[i].fsize);
		}
		if(fd[i].buf!=NULL) 
		{ 
			free(fd[i].buf); 
			fd[i].buf=0; 
		} 
		if(fd[i].Fd>0) 
		{ 
			close(fd[i].Fd); 
			fd[i].Fd=0; 
		} 
		fd[i].seek=-1; 
	} 

} 
#ifdef SPARC 
static int head4to5(char **,int,int); 
#else 
static int head4to5(char **,int); 
#endif 
static int head3to5(char **,int); 
 
CX_BASE::CX_BASE(char *orig_name,char *dir)
{
	char folder[MAXPATHLEN+1];
	long len_dir=0;
	if (dir!=NULL)
	len_dir=strlen(dir);
	if(dir!=NULL&&len_dir>0&&strncmp(orig_name,dir,len_dir)!=0)
	{
		if (dir[len_dir-1]!='/')
		sprintf(folder,"%s/%s",dir,orig_name);
		else
		sprintf(folder,"%s%s",dir,orig_name);
	}
	else
	{
		strcpy(folder,orig_name);
	}

	if(!if_base(0,folder)) 
	{ 
		throw(1); 
		return; 
	} 
#ifdef WIN32
	_fmode=_O_BINARY;
#endif
	bzero(this,sizeof *this); 
	char *header_name=(char *)malloc(2*strlen(folder)+2); 
	char *n=strrchr(folder,'/');
	if(n==NULL)
		n=folder;
	else    n++;
	full(folder,n,header_name);
 
	int df=0; 
	if(!if_read(header_name) || (df=open(header_name,O_RDONLY|O_BINARY))<0)
	{ 
		free(header_name); 
		throw(2); 
		return; 
	} 
	free(header_name); 
 
	int i; 
	struct stat st; 
 
	cadr_record=-1; 
	fstat(df,&st); 
	context = (struct header *)malloc(st.st_size); 
	read(df,context,st.st_size); 
	PAGESIZE=4096;
	close(df); 
	i=int_conv((char *)context,4); 
	if(i!=CXKEY6 && i!=CXKEY5 && i!=CXKEY4 && i!=CXKEY3 && i!=-(CXKEY3) && i!=5472566)
	{ 
		free(context); 
		throw(3); 
		return; 
	} 
	if(i==CXKEY4) 
#ifdef SPARC 
		st.st_size=head4to5((char **)&context,st.st_size,0); 
#else 
		st.st_size=head4to5((char **)&context,st.st_size); 
#endif 
	else if(i==CXKEY3 || i==-(CXKEY3) || i==5472566) 
		st.st_size=head3to5((char **)&context,st.st_size); 
#ifdef SPARC 
	else conv_header((char *)context,st.st_size,0); 
#endif 
	if(i==5472566) 
	    context->v=39;
	__name=(char *)malloc(strlen(folder)+1);
	strcpy(__name,folder);
	ss.field=(struct field *)(context+1); 
	ss.size=(ss.field->k && context->pswd!=CXKEY3)?2:sizeof (struct key);
	ss.ptm=context->ptm; 
	fd = (struct fd *)calloc(fdnum=3,sizeof (struct fd)); 
	fd[0].n=-1;     // Main 
	fd[0].seek=-1; 
	fd[1].n=0;      // Bank.0 
	fd[1].seek=-1; 
	fd[2].n=-2;     // Storage 
	fd[2].seek=-1; 
	rdonly=open_FD(fd); 
	len_cadr=0;
	create_struct(&ss,fd,(char *)context); 
	if(context->pswd==CXKEY5 || context->pswd==CXKEY6)
		root_size=sizeof (struct root); 
	else 
	{ 
		root_size=ss.size; 
		if((unsigned)root_size<sizeof (struct key)) 
			root_size=sizeof (struct key); 
	} 
	len_record=ss.size; 
	if(if_base(folder,LOGDB)==5) 
		hist=(char *)calloc(len_cadr+sizeof len_cadr,1); 
	else    hist=NULL; 

	if(if_base(folder,CHANGEDB)==5||if_base(folder,CHANGEDB)==6)
	{
		char name[MAXPATHLEN+1];
		full(__name,CHANGEDB,name);
		change_db=new CX_BASE(name);
	}
	else    change_db=NULL;

	if(dir!=NULL&&len_dir>0)
	{
		if (dir[len_dir-1]!='/')
		sprintf(root_dir,"%s/",dir);
		else
		sprintf(root_dir,"%s",dir);
	}
	else
	{
		root_dir[0]=0;
	}

	no_superblock=0; 
	if(context->pswd!=CXKEY3) 
	{ 
		for(i=0;i<context->ptm;i++) 
		{ 
			if(ss.field[i].k)       // polymorph field 
			{ 
				key_field=(int *)realloc(key_field,++num_keys*sizeof (int)); 
				key_field[num_keys-1]=i+1; 
			} 
			if(ss.field[i].b)       // B++ tree
			{
				btree=(BTree **)realloc(btree,++num_btree*sizeof (BTree *));

				char name[MAXPATHLEN+1];
				char str[256];
				sprintf(str,"BTree.%d",i+1);
				full(__name,str,name);
				btree[num_btree-1]=new BTree(this,name,ss.field[i],i+1);
			}
		} 
	} 
	update(); 
	Map();
#ifndef WIN32 
	signal(SIGCHLD,CHL_handler); 
#endif 

#ifndef WIN32
	int fd=open("/usr/local/etc/.int_delimiter",O_RDONLY);
#else
	int fd=open("C:\\Program Files\\UnixSpace\\etc\\.int_delimiter",O_RDONLY|O_BINARY);
#endif
	if(fd>=0)
	{
		if(read(fd,&int_delimiter,1)!=1)
			int_delimiter='.';
		close(fd);
	}
	else
		int_delimiter='.';
} 
 
struct field *CX_BASE::Get_Field_Descr(register struct st *st,register int field,long record)
{ 
	struct sla sla[SLA_DEEP]; 
	bzero(sla,sizeof sla);
 
	sla[0].n=field; 
	sla[1].n=0; 
	sla[0].m=0; 
	sla[1].m=0; 
	return(Get_Field_Descr(st,sla,record));
} 
 
struct field *CX_BASE::Get_Field_Descr(register int field,long record)
{ 
	return(Get_Field_Descr(&ss,field,record));
} 
 
struct field *CX_BASE::Get_Field_Descr(register struct sla *sla,long record)
{ 
	return(Get_Field_Descr(&ss,sla, record));
} 
 

struct field *CX_BASE::Get_Field_Descr(register struct st *st,register struct sla *sla,long record)
{ 
	register int tip; 
	static struct field field; 
	register int n=sla->n-1; 
	if(sla->n<=0 || sla->n>st->ptm) 
	{ 
		bzero(&field,sizeof field); 
		return(&field); 
	} 
	if((tip=st->field[n].a)==X_STRUCTURE && sla[1].n) 
	{
		struct field *subfield;
		int f=(sla+1)->n;
		subfield=Get_Field_Descr(st->field[n].st.st,f);
		if (subfield->a==X_POINTER||subfield->a==X_VARIANT)
		{
		    char *ch=NULL;
		    struct sla sla1[SLA_DEEP];
		    bzero(sla1,sizeof sla1);
		    bcopy(sla,sla1,sizeof (struct sla)*2);
		    if (record<=0)
			    record=1;
		    Read(record,sla1,ch);
		    if (ch!=NULL)
		    {
			 if ((record=int_conv(ch,subfield->a==X_VARIANT?2:4))<=0)
				 record=1;
			 free(ch);
		    }
		    return(Get_Field_Descr(st->field[n].st.st,sla+1,(-1)*record));

		}
		return(Get_Field_Descr(st->field[n].st.st,sla+1,record));
	}
	if((tip==X_POINTER || tip==X_VARIANT || tip==X_EXPRESSION || tip==X_COMPLEX) && sla[1].n)
	{ 
		struct field *subfield; 
		//fill
		if (tip!=X_VARIANT)
		{
			CX_BASE *subbase=get_subbase(st,sla);
			if(subbase==NULL || subbase==this || subbase->__name==0)
			{
				bzero(&field,sizeof field);
				return(&field);
			}
			if (record==0)
				record=1;
			if (tip==X_POINTER)
			{
				char *ch=NULL;
				if (record>0)
				{
					Read(record,sla->n,ch);
					if (ch!=NULL)
					{
					     if ((record=int_conv(ch,4))<=0)
						     record=1;
					     free(ch);
					}
				}
				else
				{
					record*=-1;
				}

			}
			subfield=subbase->Get_Field_Descr(&subbase->ss,sla+1,record);
			bcopy(subfield,&field,sizeof field);
			return(&field);
		}
		else
		{

			char *ch=NULL;
			CX_BASE *subbase=NULL;
			if (record==0)
				record=1;
			if (record>0)
			{
				Read(record,sla->n,ch);
				if (ch!=NULL)
				{
				     if ((record=int_conv(ch,st->field[n].n))<=0)
					record=1;
				     free(ch);
				     ch=NULL;
				}
			}
			else
			{
				record*=-1;
			}

			subbase=get_subbase(st,sla);
			if(subbase!=NULL)
			{
				ch=NULL;
				subbase->Read(record,1,ch);
				subbase=open_db(ch);
			}
			if(ch!=NULL)
				free(ch);
			ch=NULL;
			if(subbase==NULL || subbase==this || subbase->__name==0)
			{
				bzero(&field,sizeof field);
				return(&field);
			}
			subfield=subbase->Get_Field_Descr(&subbase->ss,sla+2,record);
			bcopy(subfield,&field,sizeof field);
			return(&field);

		}
	} 
	return(st->field+n); 
} 

/*
original text
struct field *CX_BASE::Get_Field_Descr(register struct st *st,register struct sla *sla,long record)
{
	register int tip;
	static struct field field;
	register int n=sla->n-1;

	if(sla->n<=0 || sla->n>st->ptm)
	{
		bzero(&field,sizeof field);
		return(&field);
	}
	if((tip=st->field[n].a)==X_STRUCTURE && sla[1].n)
		return(Get_Field_Descr(st->field[n].st.st,sla+1));
	if((tip==X_POINTER || tip==X_VARIANT || tip==X_EXPRESSION || tip==X_COMPLEX) && sla[1].n)
	{
		struct field *subfield;
 
		CX_BASE *subbase=get_subbase(st,sla);
		if(subbase==NULL || subbase==this || subbase->__name==0)
		{
			bzero(&field,sizeof field);
			return(&field);
		}
		subfield=subbase->Get_Field_Descr(&subbase->ss,sla+1);
		bcopy(subfield,&field,sizeof field);
		return(&field);
	}
	return(st->field+n);
}
*/

CX_BASE *CX_BASE::get_subbase(struct sla *sla) 
{ 
	return(get_subbase(&ss,sla)); 
} 
 
CX_BASE *CX_BASE::get_subbase(struct st *st,struct sla *sla,int *f)
{ 
	struct field *field; 
	int j=0; 
	char *name=NULL; 

	for(field=Get_Field_Descr(st,sla->n);field->a==X_STRUCTURE;)
	{
		if(st->field[sla[j].n-1].st.st==NULL)
		{
			j++;
			break;
		}
		field=Get_Field_Descr(st->field[sla[j].n-1].st.st,sla[j].n);
		j++;
	}

	if(f!=NULL)
		*f=j;


//        if(f!=NULL && field->a==X_STRUCTURE)
//                *f=*f+1;

	if(field->l==0 || (field->a!=X_POINTER && field->a!=X_EXPRESSION && field->a!=X_COMPLEX && field->a!=X_VARIANT))
		return(this); 
 
	switch(field->a) 
	{ 
		case X_EXPRESSION: 
			name=(char *)malloc(strlen(__name)+strlen(EXPRDB)+2); 
			sprintf(name,"%s/%s",__name,EXPRDB); 
			break; 
		case X_COMPLEX: 
			name=(char *)malloc(strlen(__name)+strlen(COMPLDB)+2); 
			sprintf(name,"%s/%s",__name,COMPLDB); 
			break; 
		case X_POINTER: 
		case X_VARIANT: 
			char *n,*ch; 
			if(field->name==NULL || !(*field->name))     // probably it is a property
			{ 
				char *name=(char *)malloc(strlen(__name)+1);
				strcpy(name,__name); 
				char *ch=strrchr(name,'/'); 
				if(ch!=NULL) 
					*ch=0; 
				field->name=name; 
			} 
			n=(char *)malloc(strlen(field->name)+1);
			strcpy(n,field->name);

			if((ch=strchr(n,'\n'))!=NULL) 
				*ch=0; 
			name=(char *)malloc(strlen(n)+1); 
			strcpy(name,n); 
			free(n); 
 
			if((*name!='/'&&*name!='\\') && strchr(name,':')==NULL)
			{ 
				n=(char *)malloc(strlen(__name)+strlen(name)+2); 
				strcpy(n,__name); 
				char *ch=strrchr(n,'/'); 
				if(ch!=NULL) 
				{ 
					*ch=0; 
					strcat(n,"/"); 
					strcat(n,name); 
					free(name); 
					name=n; 
				} 
				else 
					free(n); 
			} 
			break; 
	} 
	if(name!=NULL) 
	{ 
		CX_BASE *db=open_db(name); 
		free(name); 
		return(db); 
	} 
	return(this); 
} 
 
#ifndef SERVER 
CX_BASE *CX_BASE::open_db(char *name1)
{ 

	if (name1==NULL||strlen(name1)<=0)
	return NULL;
	long len_dir=strlen(root_dir);
	char *name=(char *)malloc(strlen(name1)+len_dir+2);
	if(len_dir>0&&strncmp(name1,root_dir,len_dir)!=0)
	{
		sprintf(name,"%s%s",root_dir,name1);
	}
	else
	{
		strcpy(name,name1);
	}

	for(int i=0;i<num_open_bases;i++)
	{ 
		if(open_base[i]->__name&&name&&!strcmp(open_base[i]->__name,name))
		{ 
			CX_BASE *tmp=open_base[i]; 
			if(num_open_bases-i>1) 
			{ 
				bcopy(open_base+i+1,open_base+i,(num_open_bases-i-1)*(sizeof *open_base)); 
			} 
			if(num_open_bases>1) 
			{ 
				bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base)); 
			} 
			open_base[0]=tmp; 
			free(name);
			return(open_base[0]); 
		} 
	}
	free(name);
	CX_BASE *subbase=NULL; 
	try 
	{ 
		subbase = new CX_BASE(name1,root_dir);
		if(in_memory)
			subbase->In_Memory(in_memory);
	} 
	catch(...) 
	{ 
		return(NULL); 
	} 
	if(num_open_bases==MAX_OPEN_BASE) 
	{ 
		printf("Too many open bases"); getchar();
		delete open_base[--num_open_bases]; 
	} 
	open_base=(CX_BASE **)realloc(open_base,(++num_open_bases)*sizeof (CX_BASE *)); 
	if(num_open_bases>1) 
		bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base)); 
	return(open_base[0]=subbase); 
} 
#endif 
 
int CX_BASE::create_struct(struct st *s,struct fd *sfd,char *hdr) 
{ 
	register int i; 
	int wshift=0; 
 
	for(i=0;i<s->ptm;i++) 
	{ 
		int st_ptm=s->field[i].atr.num_subfield; 
		int length; 
 
		s->field[i].atr.attr=(struct attr *)calloc(1,sizeof (struct attr)); 
		s->field[i].atr.attr->sfd=sfd; 
 
		if(hdr==(char *)context) 
		{ 
			if(s->field[i].d && !(context->pswd==CXKEY3 && variable(s,i))) 
				s->field[i].atr.attr->wshift=0; 
			else 
				s->field[i].atr.attr->wshift=wshift; 
			s->field[i].atr.attr->cshift=len_cadr; 
		} 

		s->field[i].name=hdr+(int)s->field[i].name;

		if((hdr==(char *)context)) 
		{ 
			if(s->field[i].d || s->field[i].b)
			{
				fd=(struct fd *)realloc(fd,(fdnum+1)*sizeof (struct fd));
				bzero(fd+fdnum,sizeof (struct fd));
				s->field[i].atr.attr->sfd=(struct fd *)fdnum;
				fd[fdnum].n=i+1;
				fdnum++;
			}
			else if(variable(s,i)) 
				s->field[i].atr.attr->sfd=fd+1; 
		} 
		if(s->field[i].a==X_STRUCTURE) 
		{ 
			int shift=s->field[i].st.struct_descr; 
 
			s->field[i].st.st=(struct st *)calloc(1,sizeof (struct st)); 
 
			s->field[i].st.st->field=(struct field *)(hdr+sizeof(struct field)*(1+shift)); 
			s->field[i].st.st->ptm=st_ptm; 
			s->field[i].st.st->size=0; 
			create_struct(s->field[i].st.st,s->field[i].atr.attr->sfd,hdr); 
		} 
		if((s->field[i].k || s->field[i].b) && context->pswd!=CXKEY3)
			length=4; 
		else if(s->field[i].m)
		{
			if(context->pswd==CXKEY6)
				length=8;
			else
				length=4;
		}
		else 
			length=s->field[i].l; 
		if(hdr==(char *)context) 
		{ 
			if((s->field[i].k || s->field[i].b) && context->pswd!=CXKEY3)
			{ 
				if(s->field[i].a==X_TEXT) 
					len_cadr+=256; 
				else    len_cadr+=s->field[i].l; 
			} 
			else if(s->field[i].a!=X_STRUCTURE) 
				len_cadr+=length; 
			else if(s->field[i].m) 
			{
				len_cadr-=(s->field[i].l-4);    // lenght of the array should be 4 bytes
				if(context->pswd==CXKEY6)
					len_cadr+=4;
			}
			wshift+=(s->field[i].d && !(context->pswd==CXKEY3 && variable(s,i))?0:length); 
		} 
		if(s->field[i].b)
			s->size+=4;
		else
			s->size+=(s->field[i].d  && !(context->pswd==CXKEY3 && variable(s,i))?0:length);
	} 
	return(s->size); 
} 
 
int CX_BASE::Len_Cadr()
{
	return(len_cadr);
}
 
int CX_BASE::Len_Record()
{ 
	return(len_record);
} 
 
int CX_BASE::version() 
{ 
	return(context->v); 
} 
 
long CX_BASE::Max_Index() 
{ 
	return(max_index); 
} 
 
long CX_BASE::Max_Record() 
{ 
	return(max_record); 
} 
 
struct st *CX_BASE::type() 
{ 
	return(&ss); 
} 
 
struct la 
{ 
	unsigned char  a:4; 
	unsigned char  s:1; 
	unsigned char  h:1; 
	unsigned char  m:1; 
	unsigned char  d:1; 
	unsigned char  n; 
 
 
 
	unsigned int  l:14; 
	unsigned char  x:2; 
}; 
 
static int head3to4(char **header,int size) 
{ 
	int i,len_size,shift; 
	struct header *context4; 
	char *names,*out,*x3; 
 
	union la4 
	{ 
		struct la la; 
		char s[sizeof (struct la)]; 
	} *la4; 
 
	struct old_h 
	{ 
		long pswd; 
		unsigned short ptm; 
		unsigned char b[10]; 
	} *context3; 
 
	struct ola 
	{ 
		unsigned char a; 
		unsigned char l; 
	} *la3; 
 
	x3=(char *)*header; 
	context3=(struct old_h *)x3; 
	context4=(struct header *)calloc(sizeof (struct header),1); 
	if(context3->pswd==-(CXKEY3)) 
		len_size=2; 
	else    len_size=3; 
 
#ifdef SPARC 
	conv(x3,4); 
	conv(x3+4,2); 
#endif 
	context4->pswd=CXKEY3; 
	context4->ptm=context3->ptm; 
 
	la3=(struct ola *)(context3+1); 
	la4=(union la4 *)calloc(context4->ptm,(sizeof (union la4))); 
 
	for(i=0;i<context3->ptm;i++) 
	{ 
 
		la4[i].la.a=la3[i].a&017; 
		la4[i].la.h=la3[i].a&040?1:0; 
		if(((la3[i].a&017)==01) && la3[i].a&0100) 
		{ 
			la4[i].la.h=1; 
			la4[i].la.m=0; 
		} 
		else 
			la4[i].la.m=la3[i].a&0100?1:0; 
		la4[i].la.d=la3[i].a&0200?1:0; 
		la4[i].la.l=la3[i].l; 
		if(la4[i].la.a==1) 
		{ 
			la4[i].la.l=4; 
			la4[i].la.n=0; 
		} 
		if(la4[i].la.a==3) 
		{ 
			if(context3->pswd==5472566) 
				la4[i].la.l=4; 
			else 
				la4[i].la.l=3; 
			la4[i].la.n=0; 
		} 
		if(la4[i].la.a==5) 
		{ 
			la4[i].la.l=2; 
			la4[i].la.n=0; 
		} 
		if(la4[i].la.a==4) 
		{ 
			la4[i].la.a=5; 
			la4[i].la.l=1; 
			la4[i].la.n=0; 
		} 
		if(la4[i].la.a==7) 
		{ 
			la4[i].la.a=5; 
			la4[i].la.l=4; 
			la4[i].la.n=0; 
		} 
		if(la4[i].la.a==8) 
		{ 
			la4[i].la.a=7; 
			la4[i].la.l=8; 
			la4[i].la.n=la3[i].l; 
		} 
		if(la4[i].la.a==14) 
			la4[i].la.a=8; 
		if(la4[i].la.a==6) 
		{ 
			la4[i].la.l=4; 
			la4[i].la.n=la3[i].l; 
		} 
		if(la4[i].la.a==1 || la4[i].la.a==10 || la4[i].la.a==11 || la4[i].la.m) 
			la4[i].la.x=len_size; 
		else 
			la4[i].la.x=0; 
		if(la4[i].la.a==9) la4[i].la.a=7; 
	} 
	names=x3+sizeof (struct old_h)+context3->ptm*sizeof (struct ola); 
	out=(char *)malloc(size+=(context3->ptm*(sizeof (struct la)-sizeof (struct ola)))); 
	memcpy(out,(char *)context4,sizeof (struct header)); 
	shift=sizeof (struct header); 
 
	for(i=0;i<context3->ptm;i++) 
	{ 
		memcpy(out+shift,la4[i].s,sizeof (struct la)); 
		shift+=sizeof (struct la); 
		memcpy(out+shift,names,strlen(names)+1); 
		shift+=strlen(names)+1; 
		names+=strlen(names)+1; 
	} 
	free(context4); 
	free(la4); 
	free(*header); 
	*header=out; 
	return(size); 
} 
 
static int head3to5(char **buf,int len) 
{ 
	len=head3to4(buf,len); 
#ifdef SPARC 
	return(head4to5(buf,len,1)); 
#else 
	return(head4to5(buf,len)); 
#endif 
} 
 
static int atr4to5(int atr) 
{ 
	switch(atr) 
	{ 
		case 0: 
			return(X_STRING); 
		case 1: 
			return(X_TEXT); 
		case 2: 
			return(X_POINTER); 
		case 3: 
			return(X_DATE); 
		case 4: 
			return(X_TIME); 
		case 5: 
			return(X_INTEGER); 
		case 6: 
			return(X_FLOAT); 
		case 7: 
			return(X_DOUBLE); 
		case 8: 
			return(X_EXPRESSION); 
		case 9: 
			return(X_STRUCTURE); 
		case 10: 
			return(X_BINARY); 
		case 11: 
			return(X_FILENAME); 
		case 12: 
			return(X_VARIANT); 
		case 13: 
			return(X_UNSIGNED); 
	} 
	return(0); 
} 
 
#ifdef SPARC 
static int head4to5(char **buf,int len,int atr) 
#else 
static int head4to5(char **buf,int len) 
#endif 
{ 
	struct header *header; 
	struct field *field; 
	struct la la; 
	char *names=NULL,*x4; 
	int shift=sizeof (struct header); 
	int names_len=0; 
	int i,last_field; 
 
	header=(struct header *)malloc(sizeof (struct header)); 
	x4=(char *)*buf; 
	header->pswd=int_conv(x4,4); 
	header->ptm =int_conv(x4+4,2); 
 
	bcopy(x4+6,(char *)(header)+8,sizeof (struct header)-8); 
	header->v=0; 
	field=(struct field *)calloc(last_field=header->ptm,sizeof (struct header)); 
	for(i=0;i<header->ptm;i++) 
	{ 
		bcopy(x4+shift,&la,sizeof (struct la)); 
 
#ifdef SPARC 
		if(!atr) 
		{ 
			char *ch=(char *)&la; 
			char ch1=ch[0]; 
			ch[0]=0; 
			ch[0]=(ch1&0x1f)<<4; 
			for(int j=4;j<8;j++) 
			{ 
				if((ch1>>j)&1) 
					ch[0]|=(1<<7-j); 
			} 
			field[i].l=ch[2]; 
		} 
		else 
			field[i].l=la.l; 
#else 
		field[i].l=la.l; 
#endif 
 
		field[i].a=atr4to5(la.a); 
		field[i].n=la.n; 
		field[i].m=la.m; 
		field[i].d=la.d; 
		field[i].k=la.h; 
		shift+=sizeof (struct la); 
		len=strlen(x4+shift)+1; 
		names=(char *)realloc(names,names_len+len); 
		bcopy(x4+shift,names+names_len,strlen(x4+shift)); 
		names[names_len+len-1]=0; 
		field[i].name=(char *)names_len; 
		names_len+=len; 
		shift+=len; 
		if(field[i].a==X_STRUCTURE) 
		{ 
			field[i].atr.num_subfield=la.n; 
			field[i].n=0; 
			field=(struct field *)realloc(field,(last_field+la.n)*sizeof (struct field)); 
			bzero(field+last_field,la.n*sizeof (struct field)); 
			for(int j=0;j<field[i].atr.num_subfield;j++,last_field++) 
			{ 
				bcopy(x4+shift,&la,sizeof (struct la)); 
#ifdef SPARC 
				if(!atr) 
				{ 
					char *ch=(char *)&la; 
					char ch1=ch[0]; 
					ch[0]=0; 
					ch[0]=(ch1&0x1f)<<4; 
					for(int k=4;k<8;k++) 
					{ 
						if((ch1>>k)&1) 
							ch[0]|=(1<<7-k); 
					} 
					field[last_field].l=ch[2]; 
				} 
				else 
					field[last_field].l=la.l; 
#else 
				field[last_field].l=la.l; 
#endif 
				field[last_field].a=atr4to5(la.a); 
				field[last_field].n=la.n; 
				field[last_field].m=la.m; 
				field[last_field].d=la.d; 
				field[last_field].k=la.h; 
				shift+=sizeof (struct la); 
				len=strlen(x4+shift)+1; 
				names=(char *)realloc(names,names_len+len); 
 
				bcopy(x4+shift,names+names_len,strlen(x4+shift)); 
				names[names_len+len-1]=0; 
 
				field[last_field].name=(char *)names_len; 
				names_len+=len; 
				shift+=len; 
			} 
			field[i].st.struct_descr=last_field-field[i].atr.num_subfield; 
		} 
	} 
	len=sizeof (struct header)+last_field*sizeof (struct field); 
	for(i=0;i<last_field;i++) 
		field[i].name+=len; 
	free(*buf); 
	*buf=(char *)calloc(sizeof (struct header)+last_field*sizeof (struct field)+names_len,1); 
	len=sizeof (struct header); 
	bcopy(header,*buf,len); 
	bcopy(field, (char *)*buf+len,last_field*sizeof (struct field)); 
	len+=last_field*sizeof (struct field); 
	bcopy(names, (char *)*buf+len,names_len); 
	len+=names_len; 
	free(field); 
	free(names); 
	free(header); 
	return(len); 
} 
