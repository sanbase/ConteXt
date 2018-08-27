/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:find.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term;
 
CX_FIND::CX_FIND(CX_BASE *a,int dir) 
{ 
	bzero(this,sizeof (CX_FIND)); 
	db=a; 
	direction=dir; 
	if(direction==0)
		direction=1;
	t_record=0; 
	page=0; 
	position=0; 
	ttree=0;
	bptree=0;
} 
 
CX_FIND::~CX_FIND() 
{ 
	if(f_steck!=NULL) 
		free(f_steck); 
	if(find_str!=NULL) 
		free(find_str); 
} 
 
void CX_FIND::Set_Direction(int i) 
{ 
	direction=i>=0?1:-1; 
} 
 
long CX_FIND::Find_First(char *des,char *str1,int atr) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	str_to_sla(des,sla); 
	return(Find_First(sla,str1,atr)); 
} 
 
long CX_FIND::Find_Left(struct sla *sla)
{
	direction=-1;
	return(Find_First(sla,"",-1));
}

long CX_FIND::Find_Left(int field)
{
	direction=-1;
	return(Find_First(field,"",-1));
}

long CX_FIND::Find_Left(char *des)
{
	direction=-1;
	return(Find_First(des,"",-1));
}

long CX_FIND::Find_Right(int field)
{
	direction=1;
	return(Find_First(field,"",-1));
}

long CX_FIND::Find_Right(struct sla *sla)
{
	direction=1;
	return(Find_First(sla,"",-1));
}

long CX_FIND::Find_Right(char *des)
{
	direction=1;
	return(Find_First(des,"",-1));
}

long CX_FIND::Find_First(int field,char *str1,int atr) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Find_First(sla,str1,atr)); 
} 
 
/* 
	atr==0  find ==
	atr==1  find str*
	atr==2  find >=
	atr==-1 find minimum or maximum
*/ 
 
long CX_FIND::Find_First(struct sla *SLA,char *str1,int atr) 
{ 
	struct node node; 
//	char *ch; 
	long record=0,last_rec=0; 
	int i,v,max,len; 
	struct node_val vnode; 
	TTree dt(db);
	TPage *val=NULL;
 
	t_record=-1; 
	if(db==NULL || db->Name_Base()==NULL) 
		return(-1); 
	bzero(sla,sizeof sla); 
	for(i=0;i<SLA_DEEP && SLA[i].n;i++) 
	{ 
		sla[i]=SLA[i]; 
		struct field *field=db->Field_Descr(sla); 
		if(field->a==X_POINTER || field->a==X_VARIANT) 
		{ 
			break; 
		} 
	} 
	struct field *des_field=db->Get_Field_Descr(sla);
 
	if(des_field->k && db->context->pswd!=CXKEY3)
		ttree=1;
/*
	if(des_field->b)
		bptree=1;
	if(!bptree && db->open_Tree(SLA)<=0)
*/
	if(db->open_Tree(SLA)<=0)
		return(0); 
	char *str=(char *)malloc(strlen(str1)+1); 
	strcpy(str,str1); 
 
	find_str=(char *)realloc(find_str,strlen(str1)+1); 
	bzero(&vnode,sizeof vnode); 
 
	strcpy(find_str,str); 
	find_atr=atr; 
 
	db->Rlock(0); /* ждем когда освободят дерево */ 
/*
	if(bptree && atr==-1)
	{

		BTree *bt=
	}
*/
	node=db->Get_Node(0,sla); 
	if(node.x[1]==0) 
	{ 
		db->Unlock(0); 
		return(-1); 
	} 
	if(atr==-1)
	{
		record=page=node.x[1];
		if(ttree)
		{
			TTree dt(db);
			if(direction<0)
				page=dt.Find_Left(sla->n,node.x[1]);
			else
				page=dt.Find_Right(sla->n,node.x[1]);
			TPage *t = (TPage *)db->Get_Buf(db->fd+2,page*db->PAGESIZE,db->PAGESIZE);
			db->Unlock(0);
			if(direction<0)
			{
				position=0;
				return(t_record=t->rec(t->MIN()));
			}
			else
			{
				position=t->hdr.last-1;
				return(t_record=t->rec(t->MAX()));
			}
		}
		while(record!=0)
		{
			node=db->Get_Node(record,sla);
			if(direction<0)
			{
				if(node.x[0]==0)
				{
					db->Unlock(0);
					return(t_record=record);
				}
				record=node.x[0];
			}
			else
			{
				if(node.x[1]==0)
				{
					db->Unlock(0);
					return(t_record=record);
				}
				record=node.x[1];
			}
		}
		db->Unlock(0);
		return(-1);
	}
	if(atr==1)
		len=strlen(str); 
	else if(des_field->a==X_VARIANT && strchr(str,':')==NULL) 
		len=des_field->l; 
	else    len=0; 
 
	if(des_field->a!=X_TEXT || ttree)
	{ 
		if(db->String_To_Buf(str,*des_field,vnode.ch,0,0,record)<0) 
		{ 
			t_record=-1; 
			goto END; 
		} 
	} 
	else 
	{ 
		vnode.ch=(char *)malloc(strlen(str)+1); 
		strcpy(vnode.ch,str); 
	} 
	if(f_steck!=NULL) 
		free(f_steck); 
	f_steck=(struct find_steck*)calloc((node.x[0]+3),(sizeof(struct find_steck))); 
	record=node.x[1]; 
	max=node.x[0]+1; 
 
	for(v=0;;) 
	{ 
		for(;;) 
		{ 
			struct key key; 
			if(record<0) 
				goto NO; 
			if(ttree)
			{ 
				val=(TPage *)db->Get_Buf(db->fd+2,record*db->PAGESIZE,db->PAGESIZE);
				key=dt.Pos_in_Page(record,vnode.ch,0,val,len);
				i=key.l; 
			} 
			else
				i=direction*db->comp_vnode(des_field,record,sla,&vnode,len,1); 

			node=db->Get_Node(record,sla); 
			if(i>0)
			{ 
				if(atr==2 && v>0) 
				{ 
					record=f_steck[v-1].num; 
				} 
				break;  /* проскочили */ 
			} 
			if(!i) 
			{ 
				if(ttree)
				{ 
					if(key.r<0 && atr==0) 
						goto NO; 
					char *ch=val->buf+val->pos(key.r); 
					if(last_rec==0) 
						last_rec=val->rec(ch); 
					if(val->rec(ch)<=last_rec) 
					{ 
						page=record; 
						position=key.r; 
						last_rec=t_record=val->rec(ch); 
					} 
					i=-1; 
				} 
				else 
					t_record=record; 
			} 
			if(node.s) 
				goto NO; 
			f_steck[v].num=record; 
			f_steck[v].node=node; 
			if(v++ > max)   /* нет такого */ 
			{ 
NO: 
				free(f_steck); 
				f_steck=NULL; 
				t_record=-2; 
				goto END; 
			} 
			if((direction<0?node.x[1]:node.x[0])==0l) 
				break; 
			record=direction<0?node.x[1]:node.x[0]; 
		} 
		if(v > max) 
			goto NO; 
		if(!i || (direction<0?node.x[0]:node.x[1])==0) 
			break; 
		record=direction<0?node.x[0]:node.x[1]; 
	} 
	if(t_record<0l && atr==2) 
	{ 
		if(direction*db->comp_vnode(des_field,record,sla,&vnode,len,1)<0) 
		{ 
			if(v>1 && (direction<0?f_steck[v-2].node.x[0]:f_steck[v-2].node.x[1])==record) 
			{ 
				if(atr==2 && (direction*db->comp_vnode(des_field,f_steck[v-2].num,sla,&vnode,len,1))<0) 
				{ 
/* если предыдущий кадр в стеке еще ближе к искомому (проскочили) */ 
					t_record=f_steck[v-2].num; 
					goto END; 
				} 
			} 
			t_record=record; 
			goto END; 
		} 
	} 
END: 
	db->Unlock(0); 
	if(vnode.ch!=NULL) 
		free(vnode.ch); 
	free(str); 
	return(t_record); 
} 
 
long CX_FIND::Next() 
{ 
	struct node node; 
	int v; 
	long i=0l; 
	if(t_record<0 || sla->n<=0) 
		return(-1); 
 
	db->Rlock(0); 
	if(ttree)
	{ 
		TPage *val=(TPage *)db->Get_Buf(db->fd+2,page*db->PAGESIZE,db->PAGESIZE);
		if(direction>0)
		{
			if(++position<val->hdr.last)
			{
				t_record=val->rec(val->buf+val->pos(position));

				db->Unlock(0);
				return(t_record);
			}
		}
		else
		{
			if(--position>=0)
			{
				t_record=val->rec(val->buf+val->pos(position));

				db->Unlock(0);
				return(t_record);
			}
		}
		TTree dt(db);
		long r_page;
		if(direction>0)
			r_page=val->hdr.KEY.r<0?-val->hdr.KEY.r:val->hdr.KEY.r;
		else
			r_page=val->hdr.KEY.l<0?-val->hdr.KEY.l:val->hdr.KEY.l;
		if(r_page==0) 
		{ 
			long parent=val->hdr.parent;
			for(;page!=0;parent=val->hdr.parent)
			{ 
				val=(TPage *)db->Get_Buf(db->fd+2,parent*db->PAGESIZE,sizeof (struct page_hdr));
				if(direction>0)
					r_page=val->hdr.KEY.r<0?-val->hdr.KEY.r:val->hdr.KEY.r;
				else
					r_page=val->hdr.KEY.l<0?-val->hdr.KEY.l:val->hdr.KEY.l;

				if(r_page!=page) 
				{
					page=parent;
					break; 
				}
				page=parent;
			} 
			if(page==0) 
			{ 
				db->Unlock(0); 
				return(0); 
			} 
		} 
		else 
		{
			if(direction>0)
			{
				if((page=dt.Find_Left(sla->n,r_page))==0)
					return(0);
			}
			else
			{
				if((page=dt.Find_Right(sla->n,r_page))==0)
					return(0);
			}
		}
		val=(TPage *)db->Get_Buf(db->fd+2,page*db->PAGESIZE,db->PAGESIZE);
		if(direction>0)
		{
			t_record=val->rec(val->MIN());
			position=0;
		}
		else
		{
			t_record=val->rec(val->MAX());
			position=val->hdr.last-1;
		}
		db->Unlock(0); 
		return(t_record); 
	} 
	node=db->Get_Node(0,sla); 
	for(v=0;v<node.x[0];v++) 
	{ 
		if(f_steck[v].num==t_record) 
			break;         /* нашли узел */ 
	} 
	if(v==node.x[0]) 
	{ 
		db->Unlock(0); 
		return(t_record=-1); 
	} 
	if((direction<0?f_steck[v].node.x[0]:f_steck[v].node.x[1])==0) 
	{ 
		if(!v) 
		{ 
			db->Unlock(0); 
			return(t_record=-1); 
		} 
		db->Unlock(0); 
		return(t_record=f_steck[v-1].num); 
	} 
	i=direction<0?f_steck[v].node.x[0]:f_steck[v].node.x[1]; 
	for(;v<node.x[0];v++) 
	{ 
		f_steck[v].node=db->Get_Node(i,sla); 
		f_steck[v].num=i; 
		if((direction<0?f_steck[v].node.x[1]:f_steck[v].node.x[0])==0l) 
			break; 
		i=direction<0?f_steck[v].node.x[1]:f_steck[v].node.x[0]; 
	} 
	db->Unlock(0); 
	return(t_record=i); 
} 
 
long CX_FIND::Find_Next() 
{ 
	char *str=NULL; 
 
	if(t_record<0) 
		return(-1); 
	t_record=Next(); 
	if(t_record<0 || db->Get_Slot(t_record,sla,str)<0) 
		t_record=-1; 
	else if(strcmpw(find_str,str)) 
		t_record=-1; 
	if(str!=NULL) 
		free(str); 
	return(t_record); 
} 
