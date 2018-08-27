/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:create_db.cpp
*/

#include "StdAfx.h" 
#include "CX_BASE.h" 

#ifdef SPARC 
void conv_header(char *header,int len,int atr); 
#endif 

static void  create_directories(struct st *st, char *DBNAME, int serv) 
{ 
	char *name=NULL;
	int i; 
	struct root root; 
	char *dir[]= 
	    { 
		BLANKDIR, 
		INDEXDIR, 
		TMPDIR, 
		FILES, 
		LIMITDIR, 
		EXEDIR, 
		MACRO, 
		QUERY, 
		NULL 
	}; 
	name=(char *)malloc(strlen(DBNAME)+32); 
	for(i=0;dir[i]!=NULL;i++) 
	{ 
		sprintf(name,"%s/%s",DBNAME,dir[i]); 
#ifdef WIN32 
		_mkdir(name); 
#else 
		mkdir(name,0777); 
#endif 
		if(serv) 
			break; 
	} 
	bzero(&root,sizeof root); 
	sprintf(name,"%s/%s",DBNAME,ROOT); 
	if(st->field[0].b)      // the first field is a B++ tree
		root.Root.l=-1; // the binary tree is disabled
	if(access(name,F_OK))
	{ 
		i=creat(name,0664); 
		write(i,&root,sizeof root); 
		close(i); 
	}
	else
	{
		i=open(name,O_RDWR,0664);
		long tmpl=0;
		read(i,&tmpl,4);
		lseek(i,0,SEEK_SET);
		if (serv<0)
		{
			if (tmpl!=CXKEY5)
			{
				tmpl=CXKEY5;
				write(i,&tmpl,4);
			}
		}
		else
		{
			if (tmpl!=CXKEY6)
			{
				tmpl=CXKEY6;
				write(i,&tmpl,4);
			}
		}
		close(i);
	}
	sprintf(name,"%s/%s",DBNAME,STORAGE); 
	if(access(name,F_OK))
	{ 
		i=creat(name,0664); 
		/*
		char buf[PAGESIZE]; 
		bzero(buf,sizeof buf); 
		write(i,buf,PAGESIZE); 
*/
		close(i); 
	} 
	sprintf(name,"%s/%s0",DBNAME,BANK); 
	if(access(name,F_OK))
	{ 
		close(creat(name,0664)); 
	} 
	for(i=0;i<st->ptm;i++) 
	{ 
		if(st->field[i].d) // || st->field[i].b)
		{ 
			sprintf(name,"%s/%s%d",DBNAME,BANK,i+1); 
			if(access(name,F_OK))
				close(creat(name,0664)); 
		} 
	} 
#ifndef WIN32
	sprintf(name,"%s/Methods.cc",DBNAME);
	/*if(access(name,F_OK) && serv==0 && !access("/usr/local/etc/Methods.cc",R_OK))
	{ 
		fcopy("/usr/local/etc/Methods.cc",name);
	} */
#else
	if(serv==0 && !access("C:/Program Files/UnixSpace/etc/Methods.cpp",R_OK))
	{
		sprintf(name,"%s/Methods.cpp",DBNAME);
		fcopy("C:/Program Files/UnixSpace/etc/Methods.cpp",name);
	}
#endif
	free(name); 
} 


static int num_fields(struct st *st) 
{ 
	int i,num=0; 

	for(i=0;i<st->ptm;i++) 
	{ 
		num++; 
		if(st->field[i].a==X_STRUCTURE && st->field[i].st.st!=NULL) 
			num+=num_fields(st->field[i].st.st); 
	} 
	return(num); 
} 

static int variable(struct field field)
{
	return(field.m || field.a>=X_TEXT);
}

int len_struct(struct st *st)
{
	int len=0;
	int i;

	for(i=0;i<st->ptm;i++)
	{
		if(variable(st->field[i]))
			len+=8;
		else if(st->field[i].a==X_STRUCTURE && st->field[i].st.st!=NULL)
			len+=len_struct(st->field[i].st.st);
		else
			len+=st->field[i].l;
	}
	return(len);
}


static int write_fields_struct(struct st *st,char **names,char **buf,int *name_len,int *buf_len,int shift) 
{ 
	int l,i,f; 
	int len=0; 

	f=*buf_len; 
	for(i=0;i<st->ptm;i++) 
	{ 
		struct field line; 
		char *ch=NULL;

	       /* fill
	       if(variable(st->field[i]))
			st->field[i].l=8;*/

		if(st->field[i].a==X_STRUCTURE)
			st->field[i].l=len_struct(st->field[i].st.st);

		memcpy(&line,st->field+i,sizeof (struct field)); 
		l=*name_len; 
		if((ch=st->field[i].name)==NULL) 
			ch=""; 
		*names=(char *)realloc(*names,(*name_len)+=strlen(ch)+1); 
		(*names)[l]=0; 
		strcat((*names)+l,ch); 
		line.name=(char *)(l+shift); 
		l=*buf_len; 
		*buf=(char *)realloc(*buf,(*buf_len)+=sizeof (struct field)); 
		memcpy((*buf)+l,&line,sizeof line); 
	} 
	for(i=0;i<st->ptm;i++) 
	{ 
		if(st->field[i].a==X_STRUCTURE && st->field[i].st.st!=NULL) 
		{ 
			struct field *line; 

			line=(struct field *)((*buf)+f+i*sizeof (struct field)); 
			line->st.struct_descr=(*buf_len)/sizeof (struct field); 
			line->atr.num_subfield=st->field[i].st.st->ptm; 
			l=write_fields_struct(st->field[i].st.st,names,buf,name_len,buf_len,shift); 
			line=(struct field *)((*buf)+f+i*sizeof (struct field)); 
			len+=l; 

		} 
		else 
		{
		       /* fill
		       if(variable(st->field[i]))
				st->field[i].l=8;*/

			len+=st->field[i].l;
		}
	} 
	return(len); 
} 


void create_class(struct st *st,char *db, int serv)
{ 
	int fd,name_len=0,buf_len=0,shift; 
	union line { 
		char str[16]; 
		struct field field; 
		struct header hdr; 
	} 
	line; 
	char *names=NULL,*buf=NULL,*header_name; 
	char *class_name=NULL;

	header_name=(char *)malloc(2*strlen(db)+20); 
	class_name=db;

	bzero(&line,sizeof line); 
#ifdef WIN32 
	mkdir(class_name);
#else 
	mkdir(class_name,0777);
#endif 
	full(class_name,class_name,header_name);
	fd=creat(header_name,0644); 
	free(header_name); 

	if(serv<0)
		line.hdr.pswd=CXKEY5;
	else
		line.hdr.pswd=CXKEY6;
	line.hdr.ptm=st->ptm; 
	line.hdr.v=VER; 

	shift=num_fields(st)*sizeof (struct field) + sizeof (struct header); 
	write_fields_struct(st,&names,&buf,&name_len,&buf_len,shift); 

	buf=(char *)realloc(buf,buf_len+sizeof line); 
	bcopy(buf,buf+sizeof line,buf_len); 
	bcopy(&line,buf,sizeof line); 
	buf_len+=sizeof line; 
#ifdef SPARC 
	conv_header(buf,buf_len,1); 
#endif 
	write(fd,buf,buf_len); 
	write(fd,names,name_len); 
	close(fd); 
	create_directories(st,class_name,serv);
	if(names!=NULL)
		free(names);
	if(buf!=NULL)
		free(buf);
} 

void CX_BASE::make_changedb()
{
	struct st st;
	st.ptm=context->ptm+1;
	st.field=(struct field *)calloc(st.ptm,sizeof (struct field));
	for(int i=0;i<st.ptm;i++)
	{
		st.field[i].a=X_TIME;
		st.field[i].l=4;
	}
	char DBNAME[256];
	sprintf(DBNAME,"%s/%s",__name,CHANGEDB);
	if(!access(DBNAME,W_OK))
	{
		char name[256];
		sprintf(name,"%s/%s",__name,"CHANGEDB_TMP");
		create_class(&st,name,-1);
		CX_BASE *std=new CX_BASE(DBNAME);
		CX_BASE *dst=new CX_BASE(name);
		int max=std->Max_Record();
		char *str=NULL;
		for(int i=1;i<=max;i++)
		{
			long page=dst->New_Record();
			std->Get_Slot(i,1,str);
			for(int field=1;field<=context->ptm+1;field++)
				dst->Put_Slot(page,field,str);
			dst->Unlock(page);
		}
		delete std;
		delete dst;
		if(str!=NULL)
			free(str);

		char cmd[256];
		char tmp[256];

		sprintf(name,"%s/%s",__name,CHANGEDB);
		sprintf(cmd,"rm -r %s",name);
		system(cmd);

		sprintf(tmp,"%s/CHANGEDB_TMP",__name);
		rename(tmp,name);

		sprintf(name,"%s/%s/%s",__name,CHANGEDB,CHANGEDB);
		sprintf(tmp,"%s/%s/CHANGEDB_TMP",__name,CHANGEDB);
		rename(tmp,name);
	}
	else create_class(&st,DBNAME,-1);
	free(st.field);
}
