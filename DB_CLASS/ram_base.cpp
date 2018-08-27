/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:ram_base.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#include "ram_base.h" 
#include "../CX_Browser.h" 
#ifndef WIN32 
#include <sys/resource.h> 
#endif 
#define MAXMIN -2147483647l 
#define ST_DEPTH  44    /* this is enough for 2^31 records */ 

extern class Terminal *term;
 
RAM_BASE::RAM_BASE(char *folder,int field,int len_field):CX_BASE(folder) 
{ 
	struct sla sla[SLA_DEEP]; 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	if(ss.field[field-1].a==X_TEXT) 
		len_rec=16; 
	else if((len_rec=ss.field[field-1].l)>8) 
		len_rec=len_field; 
	init(sla); 
} 
 
RAM_BASE::RAM_BASE(char *folder,struct sla *sla,int len_field):CX_BASE(folder) 
{ 
	if(ss.field[sla->n-1].a==X_TEXT) 
		len_rec=16; 
	else if((len_rec=ss.field[sla->n-1].l)>8) 
		len_rec=len_field; 
	init(sla); 
} 
 
void RAM_BASE::init(struct sla *sla) 
{ 
	if(sla->n<1 || sla->n>Num_Fields()) 
	{ 
		throw(1); 
		return; 
	} 
 
	ram_size=(max_record+1)*(len_rec+=sizeof (struct key));
	if((pos=calloc(ram_size,1))==NULL) 
	{ 
#ifndef WIN32 
		struct rlimit rl; 
		getrlimit(RLIMIT_DATA, &rl); 
		rl.rlim_cur=ram_size+1024*1024; 
		if(setrlimit(RLIMIT_DATA,&rl)==-1 || (pos=calloc(ram_size,1))==NULL) 
#endif 
		{ 
			throw(2); 
			return; 
		} 
	} 
	des_field=Get_Field_Descr(&ss,sla); 
	bcopy(sla,v_field,sizeof v_field); 
	if(Wlock(0)) 
	{ 
		free(pos); 
		throw(3); 
	} 
	delete_list=NULL; 
	del_num=0; 
} 
 
RAM_BASE::~RAM_BASE() 
{ 
	Unlock(0); 
	free(pos); 
	if(delete_list!=NULL) 
		free(delete_list); 
} 
 
int RAM_BASE::Put_Node(long record,struct sla *sla,struct node node) 
{ 
	struct key KEY; 
 
	if(record<0) 
		return(-1); 
	if(node.b==-1) 
		KEY.l=-node.x[0]; 
	else    KEY.l=node.x[0]; 
	if(node.b==1) 
		KEY.r=-node.x[1]; 
	else    KEY.r=node.x[1]; 
	if(record==0l && KEY.l==0) 
		KEY.r=0; 
#ifdef SPARC 
	conv((char *)&KEY.r,sizeof (long)); 
	conv((char *)&KEY.l,sizeof (long)); 
#endif 
	bcopy(&KEY,(char *)pos+record*len_rec,sizeof(KEY)); 
	return(0); 
 
} 
struct node RAM_BASE::Get_Node(long record,struct sla *sla) 
{ 
	struct node node; 
	struct key  KEY; 
 
 
	node.x[0]=node.x[1]=-1; 
	node.s=1; 
	node.b=0; 
	if(record<0) return(node); 
	if(record>max_record) 
	{ 
		update(); 
		if(record>max_record) 
		{ 
			node.s=2; 
			return(node); 
		} 
	} 
	bcopy((char *)pos+record*len_rec,&KEY,sizeof KEY); 
#ifdef SPARC 
	conv((char *)&KEY.l,4); 
	conv((char *)&KEY.r,4); 
#endif 
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
 
int RAM_BASE::Insert_Node(long record,int field) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Insert_Node(record,sla)); 
} 
 
int RAM_BASE::Insert_Node(long record,struct sla *sla) 
{ 
	int rv=0; 
	struct node_val vnode; 
 
	bzero(&vnode,sizeof vnode); 
	if((rv=fill_vnode(record,sla,&vnode))>=0) 
	{ 
		rv = InsNode(sla,&vnode); 
		if(rv==-3 && sla[0].n==1 && sla[1].n==0) 
		{ 
			/* восстановление кадра в базе, не имеющей дерева по 1-му полю */ 
			struct node node; 
 
			node.x[0]=node.x[1]=0; 
			node.b=node.s=0; 
			Put_Node(record,sla,node); 
			rv=0; 
		} 
	} 
	return(rv); 
} 
/* 
 *      InsNode - non-recursive procedure to insert element in a tree. 
 *      Returns : 
 *          on error [invalid tree root]                -3, 
 *          on error [invalid tree structure]           -2, 
 *          on error [ENOMEM]                           -1, 
 *          if depth of tree has been increased          1, 
 *          otherwise -                                  0. 
 */ 
 
int RAM_BASE::InsNode(struct sla *sla, struct node_val *inode ) 
{ 
	long record=0,q,q1=0,q2,t; 
	struct node pn,qn,qn1,qn2,tn,nad[ST_DEPTH],*na=nad; 
	int h,lvl=0,max,fromleft=0; 
 
	pn=Get_Node(0L,sla); 
	if(pn.s || pn.b<0) 
		return(-2); 
	if(pn.b) 
		return(-3);       // для 1-го поля - отсутствие дерева 
	if((max=pn.x[0])>ST_DEPTH) 
		return(-1); 
	for(q=pn.x[1];q>0;pn=qn) 
	{ 
		record=q; 
		qn=Get_Node(q,sla); 
		if(lvl>max || qn.s) 
			return(-2); 
		pn.s=!fromleft; 
		na[lvl++]=pn; 
		h=comp_vnode(q,sla,inode); 
		if(h<0) 
		{ 
			fromleft=1; 
			q=qn.x[0]; 
		} 
		else if(h>0) 
		{ 
			fromleft=0; 
			q=qn.x[1]; 
		} 
		else 
			return(0); // узел уже есть в дереве 
	} 
 
	q=inode->record; 
	qn.x[0]=qn.x[1]=0; 
	qn.b=qn.s=0; 
	if(fromleft) 
		pn.x[0]=q; 
	else 
		pn.x[1]=q; 
	qn1.s=1;               // неинициализированный узел 
 
	for(h=1;h && lvl--;fromleft=!pn.s,pn.s=0) 
	{ 
		qn2=qn1; 
		q2=q1; 
		qn1=qn; 
		q1=q; 
		qn=pn; 
		q=record; 
		pn=na[lvl]; 
		record=lvl?(na[lvl-1].s?na[lvl-1].x[1]:na[lvl-1].x[0]):0; 
		if(fromleft) 
		{ 
			switch(qn.b) 
			{ 
			case  1: 
				qn.b=0; 
				h=0; 
				break; 
			case  0: 
				qn.b=-1; 
				break; 
			case -1:   // Балансировка 
				if(qn1.b<0 ) 
				{  // LL 
					qn.x[0]=qn1.x[1]; 
					qn1.x[1]=q; 
					qn.b=0; 
					tn=qn1; 
					qn1=qn; 
					qn=tn; 
					t=q1; 
					q1=q; 
					q=t; 
				} 
				else 
				{  // LR 
					q2=qn1.x[1]; 
					qn1.x[1]=qn2.x[0]; 
					qn2.x[0]=q1; 
					qn.x[0]=qn2.x[1]; 
					qn2.x[1]=q; 
					qn.b=(1-qn2.b)>>1; 
					qn1.b=-((1+qn2.b)>>1); 
					tn=qn2; 
					qn2=qn; 
					qn=tn; 
					t=q2; 
					q2=q; 
					q=t; 
				} 
				qn.b=0; 
				h=0; 
				if(!pn.s) 
					pn.x[0]=q; 
				else 
					pn.x[1]=q; 
			} 
		} 
		else 
		{ 
			switch (qn.b) 
			{ 
			case -1: 
				qn.b=0; 
				h=0; 
				break; 
			case  0: 
				qn.b=1; 
				break; 
			case  1:   // Балансировка 
				if(qn1.b > 0) 
				{  // RR 
					qn.x[1]=qn1.x[0]; 
					qn1.x[0]=q; 
					qn.b=0; 
					tn=qn1; 
					qn1=qn; 
					qn=tn; 
					t=q1; 
					q1=q; 
					q=t; 
				} 
				else 
				{  // RL 
					q2=qn1.x[0]; 
					qn1.x[0]=qn2.x[1]; 
					qn2.x[1]=q1; 
					qn.x[1]=qn2.x[0]; 
					qn2.x[0]=q; 
					qn.b=-((1+qn2.b)>>1); 
					qn1.b=(1-qn2.b)>>1; 
					tn=qn2; 
					qn2=qn; 
					qn=tn; 
					t=q2; 
					q2=q; 
					q=t; 
				} 
				qn.b=0; 
				h=0; 
				if(!pn.s) 
					pn.x[0]=q; 
				else 
					pn.x[1]=q; 
			} 
		} 
		if(!qn2.s) 
			Put_Node(q2,sla,qn2); 
	} 
	if(!qn1.s) 
		Put_Node(q1,sla,qn1); 
	Put_Node(q,sla,qn); 
	if(h) 
		pn.x[0]++;  // высота дерева увеличилась 
	Put_Node(record,sla,pn); 
	return(h); 
} 
/* 
int RAM_BASE::balance(struct sla *sla,long *parent,struct node *pqn ,int npr) 
{ 
	long q1,q2; 
	struct node qn1,qn2; 
	int b1,b2,h=1,z; 
 
	z=npr?1:-1; 
	switch(pqn->b*z) 
	{ 
	case  1:  // балланс востановился 
		pqn->b=0; 
		break; 
	case  0:  // есть перекос, но не смертельный 
		pqn->b=-z; 
		h=0; 
		break; 
	case -1:  // надо балансировать 
		q1=pqn->x[!npr]; 
		qn1=Get_Node(q1,sla); 
		if(qn1.s) 
			return(-2); 
		if((b1=qn1.b)*z<=0) 
		{ // RR 
			pqn->x[!npr]=qn1.x[npr]; 
			qn1.x[npr]=*parent; 
			if(b1) 
			{ 
				pqn->b=0; 
				qn1.b=0; 
			} 
			else 
			{ 
				pqn->b=-z; 
				qn1.b=z; 
				h=0; 
			} 
			*parent=q1; 
		} 
		else 
		{          // RL 
			q2=qn1.x[npr]; 
			qn2=Get_Node(q2,sla); 
			if(qn2.s) 
				return(-2); 
			b2=qn2.b; 
			qn1.x[npr]=qn2.x[!npr]; 
			qn2.x[!npr]=q1; 
			pqn->x[!npr]=qn2.x[npr]; 
			qn2.x[npr]=*parent; 
			pqn->b=npr?((b2<0)?1:0):((b2>0)?-1:0); 
			qn1.b=npr?((b2>0)?-1:0):((b2<0)?1:0); 
			qn2.b=0; 
			*parent=q2; 
			Put_Node(q2,sla,qn2); 
		} 
		Put_Node(q1,sla,qn1); 
	} 
	return(h); 
} 
*/ 
int RAM_BASE::fill_vnode(long record,struct sla *sla,struct node_val *inode) 
{ 
	inode->record=record; 
	inode->len=len_rec-sizeof(struct key); 
	inode->ch=(char *)pos+record*len_rec+sizeof (struct key); 
	return(0); 
} 
int Compare(struct field *des_field,char *ch1,char *ch2,int len); 
int RAM_BASE::comp_vnode(long record,struct sla *sla,struct node_val *inode) 
{ 
	int  i,j; 
	char *ch; 
 
	ch=(char *)pos+record*len_rec+sizeof(struct key); 
	j=Compare(des_field,inode->ch,ch,inode->len);
	if((des_field->a==X_STRING || des_field->a==X_TEXT) && j==0 && ((int)strlen(inode->ch)>=inode->len || (int)strlen(ch)>=inode->len))
	{ 
		char *ch1=NULL,*ch2=NULL; 
		Get_Slot(inode->record,v_field,ch1); 
		Get_Slot(record,v_field,ch2); 
		j=strcmpw(ch1,ch2); 
		free(ch1); 
		free(ch2); 
	} 
	i=j>0?1:j<0?-1:0; 
	return(i?i:inode->record-record); 
} 
 
void RAM_BASE::Create_Tree() 
{ 
	char *ch=NULL; 
 
	Wlock(0); 
 
	for(int page=1;page<=max_record;page++) 
	{ 
		if(Check_Del(page)) 
		{ 
			delete_list=(long *)realloc(delete_list,++del_num*sizeof(long)); 
			delete_list[del_num-1]=page; 
			continue; 
		} 
		if(des_field->a==X_TEXT) 
			Get_Slot(page,v_field,ch); 
		else Read(page,v_field,ch); 
		if(ch!=NULL) 
			bcopy(ch,(char *)pos+page*len_rec+sizeof (struct key),len_rec-sizeof (struct key)); 
		if(des_field->a==X_TEXT || des_field->a==X_STRING) 
			*(char *)((char *)pos+page*len_rec+sizeof (struct key)+len_rec-1)=0; 
		Insert_Node(page,1); 
	} 
	free(ch); 
	if(v_field->n==1) 
	{ 
		int del,cur_del=0; 
 
		Put_Buf(fd,(off_t)0,sizeof (struct key),(char *)pos); 
		if(del_num) 
		{ 
			del=-delete_list[0]; 
			Put_Buf(fd,sizeof (struct key),sizeof (long),(char *)&del); 
		} 
		for(int page=1;page<=max_record;page++) 
		{ 
			if(cur_del<del_num && page==delete_list[cur_del]) 
			{ 
				struct key key; 
 
				if(cur_del) 
					key.l=-delete_list[cur_del-1]; 
				else    key.l=MAXMIN; 
				if(cur_del<del_num-1) 
					key.r=-delete_list[cur_del+1]; 
				else    key.r=MAXMIN; 
				bcopy(&key,(char *)pos+page*len_rec,sizeof key); 
				if((++cur_del)==del_num) 
				{ 
					del=-delete_list[cur_del-1]; 
					Put_Buf(fd,sizeof (struct key)+sizeof (long),sizeof (long),(char *)&del); 
				} 
			} 
			Put_Buf(fd,root_size+(page-1)*ss.size,sizeof (struct key),(char *)pos+page*len_rec); 
		} 
	} 
	else 
	{ 
		char name[LINESIZE]; 
 
		sprintf(name,"%s/Tree.%d",Name_Base(),v_field->n); 
		int fd=creat(name,0666); 
		write(fd,pos,sizeof(struct key)); 
		for(int page=1;page<=max_record;page++) 
			write(fd,(char *)pos+page*len_rec,sizeof(struct key)); 
		close(fd); 
	} 
	Unlock(0); 
} 
