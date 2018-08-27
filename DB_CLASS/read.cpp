/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:read.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
extern class Terminal *term;
 
#define DEV_BSIZE 4096
 
int CX_BASE::open_FD(struct fd *ffd)
{ 
	int rdo=0; 
	char name[NAMESIZE]; 
	struct stat buf; 
 
 
	if(ffd->Fd>0)
		return(rdonly); 
	if(ffd->n==-2)
		full(__name,STORAGE,name); 
	else if(ffd->n<0)
		full(__name,ROOT,name); 
	else
	{ 
		char str[256]; 
 
		sprintf(str,"%s%d",BANK,ffd->n);
		full(__name,str,name); 
	} 
	if(!if_read(name)) 
		return(-1); 
	if(!if_write(name)) 
		rdo=2; 
	if((ffd->Fd=open(name,O_RDWR|O_BINARY))<0)
	{ 
		if((ffd->Fd=open(name,O_RDONLY|O_BINARY))<0)
			return(-1); 
		rdo=2; 
	} 
	if(ffd->n==-2)
	{
		char buf[4];
		if(read(ffd->Fd,buf,sizeof buf)>0)
			PAGESIZE=int_conv(buf,4);
	}
	if(!fstat(ffd->Fd,&buf) && (buf.st_size<PAGESIZE))
		ffd->size=(buf.st_size/PAGESIZE+1)*PAGESIZE;
	else 
		ffd->size=PAGESIZE;
	ffd->fsize=buf.st_size;
	if(in_memory)
	{
		lseek(ffd->Fd,0,SEEK_SET);
		ffd->buf=(char *)realloc(ffd->buf,ffd->size=buf.st_size+65536);
		ffd->fsize=read(ffd->Fd,ffd->buf,ffd->fsize);
		ffd->in_memory=in_memory;
		ffd->seek=0;
	}
	else
	{
		ffd->buf=(char *)realloc(ffd->buf,ffd->size);
		ffd->seek=-1;
	}
	return(rdo); 
} 

char *CX_BASE::Get_Buf(struct fd *ffd, off_t seek_abs, int len) 
{
	char *ch; 
	if( seek_abs < 0 || len < 0  || ffd->Fd<0) 
		return(NULL); 
	if(!ffd->Fd) 
		if(open_FD(ffd)<0) 
			return(NULL);

#ifndef WIN32 
	if(ts!=NULL) 
	{ 
		ch=ts->Read(ffd,seek_abs,len); 
		if(ch!=NULL) 
			return(ch); 
	} 
#endif

	if(var_point!=NULL && (ch=var_point->Get(ffd->n,seek_abs,len))!=NULL) 
		return(ch); 
	if(ffd->in_memory)
	{
		if(seek_abs+len>ffd->fsize || seek_abs+len>ffd->size)
			return(NULL);
		return(ffd->buf+seek_abs);
	}

	if(seek_abs<ffd->seek || seek_abs+len>ffd->seek+ffd->size || ffd->seek<0 || ts!=NULL)
	{ 
		int rsize=0;

		if(!ffd->in_memory)
			ffd->seek=(seek_abs/DEV_BSIZE)*DEV_BSIZE;

		if(ts!=NULL) 
			ffd->seek=seek_abs; 
		if(seek_abs-ffd->seek+len > ffd->size)
		{ 
			ffd->size=((seek_abs-ffd->seek+len)/DEV_BSIZE+1)*DEV_BSIZE;
			if(ffd->buf!=NULL)
				free(ffd->buf);
			if((ffd->buf=(char *)malloc(ffd->size))==NULL)
			{
				ffd->seek=-1;
				return(NULL);
			}
		} 
		if(ffd->seek<0) 
			ffd->seek=0;
		if(lseek(ffd->Fd,(off_t)ffd->seek,SEEK_SET)!=ffd->seek || (rsize=read(ffd->Fd,ffd->buf,ffd->size))<=0)
		{ 
			if(ffd->buf!=NULL) 
				bzero(ffd->buf,ffd->size);
			ffd->seek=-1;
			return(NULL); 
		} 
#ifndef WIN32
		if(seek_abs+len-ffd->seek>rsize) 
			return(NULL); 
#endif
		if(no_superblock==2) 
			no_superblock=0; 
	}
	return(ffd->buf+seek_abs-ffd->seek); 
} 
 
struct node CX_BASE::Get_Node(long record) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
//        sla->n=1; /// было 0.
	return(Get_Node(record,sla)); 
} 
 
struct node CX_BASE::Get_Node(long record,int f) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla->n=f; 
	return(Get_Node(record,sla)); 
} 
 
void CX_BASE::conv_39node(int fd,struct key *KEY) 
{ 
	if(key39==0) 
	{ 
		unsigned long buf[128]; 
		int size,i,p; 
 
		key39=1; 
		lseek(fd,0,SEEK_SET); 
		read(fd,&p,4); 
		lseek(fd,8,SEEK_SET); 
		for(;;) 
		{ 
			if((size=read(fd,buf,sizeof buf)/sizeof (long))<=0) 
				return; 
			for(i=0;i<size;i+=2) 
			{ 
				if(buf[i]==buf[i+1] && (p^buf[i])>0 && (p^buf[i])<1000) 
				{ 
					key39=buf[i]; 
					goto OK; 
				} 
			} 
		} 
	} 
OK: 
	if(key39!=1) 
	{ 
		KEY->l^=key39; 
		KEY->r^=key39; 
	} 
} 
 
struct node CX_BASE::Get_Node(long record,struct sla *sla) 
{ 
	struct node node; 
	struct key  KEY; 
	char *ch; 
	struct field *field=NULL; 
 
	node.x[0]=node.x[1]=-1; 
	node.s=1; 
	node.b=0; 
 
	if(record<0)
		return(node);
	if(sla->n==-1)
	{
		bzero(&node,sizeof node);
		TTree dt(this);
		long page=dt.Find_Page(0,"",record);
		if(page<0)
			return(node);
		TPage *t=(TPage *)Get_Buf(fd+2,page*PAGESIZE,PAGESIZE);
		KEY=t->find("",record);
		if(KEY.r>=0)
			node.s=1;
		return(node);
	}
	if(sla->n<0 || sla->n>context->ptm) 
		return(node); 
	field=Get_Field_Descr(sla); 
	if(context->pswd!=CXKEY3 && (field->k || (ss.field->k && sla->n==0)))
	{ 
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
					return(node);
			}
			if((ch=Get_Buf(fd+2,4+i*sizeof KEY,sizeof KEY))==NULL)
			{       // no root page. create it
				char *buf=(char *)calloc(PAGESIZE,1); 
				int_to_buf(buf,4,PAGESIZE);
				Put_Buf(fd+2,(off_t)0,PAGESIZE,buf);
				free(buf); 
				node.x[0]=0; 
				node.x[1]=-1; 
				return(node); 
			} 
		} 
		else
		{
			if((ch=Get_Buf(fd+2,record*PAGESIZE,sizeof KEY))==NULL)
				return(node);
		}
		bincopy(ch,&KEY,sizeof KEY);
	} 
	else if(sla[0].n<2 && sla[1].n==0) 
	{ 
		if(record>max_record) 
		{ 
			update(); 
			if(record>max_record) 
			{ 
				node.s=2; 
				return(node); 
			} 
		} 
		if(record==0) 
			ch=Get_Buf(fd,(off_t)0,sizeof KEY); 
		else 
			ch=Get_Buf(fd,(off_t)(root_size+(off_t)(record-1)*ss.size),sizeof KEY);
		if(ch==NULL) 
			goto EMPTY; 
		bincopy(ch,&KEY,sizeof KEY);
#ifdef SPARC 
		conv((char *)&KEY.l,4); 
		conv((char *)&KEY.r,4); 
#endif 
	} 
	else 
	{ 
		if(open_Tree(sla)<0) 
			return(node); 
 
		field=Get_Field_Descr(sla); 
 
		lseek(field->atr.attr->tree_fd,(off_t)(sizeof KEY)*(record),SEEK_SET); 
		if(read(field->atr.attr->tree_fd,&KEY,sizeof KEY)!=sizeof KEY) 
		{ 
EMPTY: 
			if(record==0) // it is empty tree. Create
			{ 
				node.x[1]=node.x[0]=0; 
				node.s=0; 
				Put_Node(0,sla,node); 
				bzero(&KEY,sizeof KEY); 
			} 
			else 
			{ 
				node.s=2; 
				return(node); 
			} 
		} 
#ifdef SPARC 
		conv((char *)&KEY.l,4); 
		conv((char *)&KEY.r,4); 
#endif 
		if(context->v==39) 
			conv_39node(field->atr.attr->tree_fd,&KEY); 
	} 
	node.x[0]=KEY.l; 
	node.x[1]=KEY.r; 
	node.s=0; 
	if(KEY.l<0 && KEY.r<0) 
		node.s=1; 
	else    if(KEY.l<0) 
	{ 
		node.x[0]=-KEY.l; 
		node.b=-1; 
	} 
	else    if(KEY.r<0) 
	{ 
		node.x[1]=-KEY.r; 
		node.b=1; 
	} 
	return(node); 
} 
 
int CX_BASE::is_index(int f) 
{ 
	return(open_Tree(f)>0); 
} 
 
int CX_BASE::is_index(struct sla *sla) 
{ 
	return(open_Tree(sla)>0); 
} 
 
int CX_BASE::open_Tree(int f) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla->n=f; 
	return(open_Tree(sla)); 
} 
 
int CX_BASE::open_Tree(struct sla *sla) 
{ 
	char *new_tree_name; 
	struct field *field; 
 
	if(sla->n<=0 || sla->n>context->ptm) 
		return(0); 
 
	field=Get_Field_Descr(sla->n); 
 
	if(field->atr.attr==NULL || field->m) 
		return(0); 
	if(field->atr.attr->tree_fd>0 || ((field->k || field->b) && context->ptm!=CXKEY3))
		return(1); 
	if(field->atr.attr->tree_fd<0) 
		return(-1); 
	if(sla->n==1) 
	{ 
		struct node node; 
 
		node=Get_Node(0); 
		if(node.b || node.s)      /* the root B-tree is disable */ 
		{ 
			field->atr.attr->tree_fd=-1; 
			return(-2); 
		} 
		field->atr.attr->tree_fd=1; 
		return(1); 
	} 
 
	char str[64]; 
	sla_to_str(sla,str); 
	new_tree_name=(char *)malloc(strlen(__name)+strlen(str)+32); 
 
// CX5 can't create b-tree for subfield yet. 
//        sprintf(new_tree_name,"%s/%s.%s",__name,TREE,str+1); 
 
	sprintf(new_tree_name,"%s/%s.%d",__name,TREE,sla->n); 
	if(access(new_tree_name,0)) 
	{ 
		free(new_tree_name); 
		field->atr.attr->tree_fd=-2; 
		return(-3); 
	} 
	if((field->atr.attr->tree_fd=open(new_tree_name,O_RDWR))<0) 
	{ 
		if((field->atr.attr->tree_fd=open(new_tree_name,O_RDONLY))<0) 
		{ 
			field->atr.attr->tree_fd=-3; 
			free(new_tree_name); 
			return(-4); 
		} 
	} 
	free(new_tree_name); 
	return(1); 
} 
