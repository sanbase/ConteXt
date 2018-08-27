/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:tree.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
extern class Terminal *term;
 
#define ST_DEPTH  44    // it is enough for 2^31 records
 
int rebuild_tree(char *name,long record,struct sla *sla); 
extern int debug; 
int Compare(struct field *des_field,char *ch1,char *ch2,int len) 
{ 
	dlong j; 
 
	switch(des_field->a) 
	{ 
		case X_DATE: 
			j=date_from_rec(ch1)-date_from_rec(ch2); 
			break; 
		case X_UNSIGNED: 
			j=unsigned_conv(ch1,des_field->l)-unsigned_conv(ch2,des_field->l); 
			break; 
		case X_VARIANT: 
		{ 
			long page1, page2; 
			page1=int_conv(ch1,des_field->n); 
			page2=int_conv(ch2,des_field->n); 
			j=page1-page2; 
			if(j==0 && len==0) 
			{ 
				page1=int_conv(ch1+des_field->n,des_field->l-des_field->n); 
				page2=int_conv(ch2+des_field->n,des_field->l-des_field->n); 
				j=page1-page2; 
			} 
			break; 
		} 
		case X_POINTER:
		case X_TIME:
		case X_INTEGER:
			j=int_conv(ch1,des_field->l)-int_conv(ch2,des_field->l); 
			break; 
		case X_DOUBLE: 
		{ 
			double a=*(double *)(ch1)-*(double *)ch2; 
			j=a>0?1:a==0?0:-1; 
			break; 
		} 
		case X_FLOAT: 
		{ 
			float a=*(float *)(ch1)-*(float *)ch2; 
			j=a>0?1:a==0?0:-1; 
			break; 
		} 
		case X_TEXT: 
			j=strcmpw(ch1,ch2); 
			break; 
		default: 
			j=strcmpw(ch1,ch2,len==0?des_field->l:len); 
			break; 
	} 
	return(j>0?1:j<0?-1:0); 
} 
 
 
int CX_BASE::Insert_Node(long record,int field, int dtree) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	sla[0].n=field; 
	sla[0].m=0; 
	sla[1].n=0; 
	sla[1].m=0; 
	return(Insert_Node(record,sla,dtree)); 
} 
 
int CX_BASE::Insert_Node(long record,struct sla *sla, int dtree) 
{ 
	int rv=0; 
	int count=0; 
	struct node_val vnode; 

	if(!dtree && open_Tree(sla)<0)
	{
		if(sla->n!=1)
			return (0);
	}
	update(); 
	if(Wlock(0)) 
		return(-1); 
	bzero(&vnode,sizeof vnode); 
BEG: 
	bzero(&vnode,sizeof vnode); 
	vnode.Dtree=dtree; 
	struct field *des_field=Get_Field_Descr(&ss,sla); 
	if((rv=fill_vnode(record,sla,&vnode))>=0) 
	{ 
		rv = InsNode(des_field,sla,&vnode); 
		if(rv) 
		{ 
			struct node node; 
 
			node.x[0]=node.x[1]=0; 
			node.b=node.s=0; 
			Put_Node(record,sla,node); 
			/* восстановление кадра в базе, не имеющей дерева по 1-му полю */ 
			if(rv==-3 && sla[0].n==1 && sla[1].n==0) 
				rv=0; 

			if(rv<0 && !count) 
			{ 
				if(vnode.ch!=NULL) 
					free(vnode.ch); 
				if(rebuild_tree(__name,record,sla)<0) 
					return(-1); 
				count++; 
				goto BEG; 
			} 
		} 
	} 
	if(vnode.ch!=NULL) 
		free(vnode.ch); 
	Unlock(0); 
	return(rv); 
} 
 
int CX_BASE::Delete_Node(long record,int field,int dtree) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	sla[0].n=field; 
	sla[0].m=0; 
	sla[1].n=0; 
	sla[1].m=0; 
	return(Delete_Node(record,sla,dtree)); 
 
} 
 
int CX_BASE::Delete_Node(long record,struct sla *sla, int dtree) 
{ 
	int rv=0; 
	int count=0; 
	struct node_val vnode; 
 
	if(!dtree && open_Tree(sla)<0) 
		return(0); 
	update(); 
	if(Wlock(0)) 
		return(-1); 
	bzero(&vnode,sizeof vnode); 
	if(Get_Node(record,sla).s) 
	{ 
		Unlock(0); 
		return(0);      // record is deleted 
	} 
 
BEG: 
	bzero(&vnode,sizeof vnode); 
	vnode.Dtree=dtree; 
	struct field *des_field=Get_Field_Descr(&ss,sla); 
	if((rv=fill_vnode(record,sla,&vnode))>= 0) 
	{ 
		rv=DelNode(des_field,sla,&vnode); 
		if(rv==-3 && sla[0].n==1 && sla[1].n==0) 
		{ 
			/* удаление кадра в базе, не имеющей дерева по 1-му полю */ 
			struct node node; 
 
			node.s=0; 
			Put_Node(record,sla,node); 
			rv=0; 
		} 
		if(rv<0 && !count) 
		{ 
			if(vnode.ch!=NULL) 
				free(vnode.ch); 
			if(rebuild_tree(__name,record,sla)<0) 
				return(-1); 
			count++; 
			goto BEG; 
		} 
 
	} 
	if(vnode.ch!=NULL) 
		free(vnode.ch); 
	Unlock(0); 
	return(rv); 
} 
 
/* 
 *      InsNode - non-recursive procedure to insert element in a tree. 
 *	Returns : 
 *          on error [invalid tree root]                -3, 
 *          on error [invalid tree structure]           -2, 
 *          on error [ENOMEM]                           -1, 
 *          if depth of tree has been increased          1, 
 *          otherwise -                                  0. 
 */ 
int CX_BASE::InsNode(struct field *des_field,struct sla *sla, struct node_val *inode ) 
{ 
	long record=0,q,q1=0,q2,t; 
	struct node pn,qn,qn1,qn2,tn,nad[ST_DEPTH],*na=nad; 
	int h=0,lvl=0,max,fromleft=0; 
 
 
	pn=Get_Node(0,sla); 
	if(pn.s || pn.b<0) 
		return(-2); 
	if(pn.b) 
		return(-3);       /* для 1-го поля - отсутствие дерева */ 
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
		h=comp_vnode(des_field,q,sla,inode); 
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
			return(0); /* узел уже есть в дереве */ 
	} 
 
	q=inode->record; 
	qn.x[0]=qn.x[1]=0; 
	qn.b=qn.s=0; 
	if(fromleft) 
		pn.x[0]=q; 
	else 
		pn.x[1]=q; 
	qn1.s=1;               /* неинициализированный узел */ 
 
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
			case -1:   /* Балансировка */ 
				if(qn1.b<0 ) 
				{  /* LL */ 
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
				{  /* LR */ 
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
			case  1:   /* Балансировка */ 
				if(qn1.b > 0) 
				{  /* RR */ 
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
				{  /* RL */ 
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
		pn.x[0]++;  /* высота дерева увеличилась */ 
	Put_Node(record,sla,pn); 
	return(h); 
} 
 
int CX_BASE::balance(struct sla *sla,long *parent,struct node *pqn ,int npr) 
{ 
	long q1,q2; 
	struct node qn1,qn2; 
	int b1,b2,h=1,z; 
 
	z=npr?1:-1; 
	switch(pqn->b*z) 
	{ 
	case  1:  /* балланс востановился */ 
		pqn->b=0; 
		break; 
	case  0:  /* есть перекос, но не смертельный */ 
		pqn->b=-z; 
		h=0; 
		break; 
	case -1:  /* надо балансировать */ 
		q1=pqn->x[!npr]; 
		qn1=Get_Node(q1,sla); 
		if(qn1.s) 
			return(-2); 
		if((b1=qn1.b)*z<=0) 
		{ /* RR */ 
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
		{          /* RL */ 
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
 
int CX_BASE::_del(struct sla *sla,long *parent,struct node qn,struct node *na,int max,int npr ) 
{ 
	short h=1,lvl=0; 
	long q=*parent,q1=qn.x[npr],*pln; 
	struct node qn1=Get_Node(q1,sla); 
 
	while(!qn1.s && qn1.x[!npr]>0) 
	{ 
		if(lvl>=max) 
			return(-2); 
		na[lvl++]=qn1; 
		q1=qn1.x[!npr]; 
		qn1=Get_Node(q1,sla); 
	} 
	if(qn1.s) 
		return(-2); 
	if(lvl) 
	{ 
		na[lvl-1].x[!npr]=qn1.x[npr]; 
		qn1.x[npr]=qn.x[npr]; 
	} 
	qn1.x[!npr]=qn.x[!npr]; 
	qn1.b=qn.b; 
	*parent=q1; 
 
	while(h>0 && lvl--) 
	{ 
		pln=lvl?&na[lvl-1].x[!npr]:&qn1.x[npr]; 
		q=*pln; 
		h=balance(sla,pln,&na[lvl],!npr); 
		Put_Node(q,sla,na[lvl]); 
	} 
	if(h>0) 
		h=balance(sla,parent,&qn1,npr); 
	else if(lvl--) 
		Put_Node(lvl?na[lvl-1].x[!npr]:qn1.x[npr],sla,na[lvl]); 
	Put_Node(q1,sla,qn1); 
	return(h); 
} 
 
/* 
 *      DelNode - non-recursive procedure to delete element from a tree. 
 *	Returns : 
 *          on error [ENOMEM]                           -4, 
 *          on error [invalid tree root]                -3, 
 *          on error [invalid tree structure]           -2, 
 *          on error [no such node]                     -1, 
 *          if depth of tree has been decreased          1, 
 *          otherwise -                                  0. 
 */ 
int CX_BASE::DelNode(struct field *des_field,struct sla *sla,struct node_val *inode) 
{ 
	long record=0,q; 
	struct node pn,qn,nad[ST_DEPTH],*na=nad; 
	int h=0,lvl=0,i,max,fromleft=0; 
 
//        fd->seek=-1; 
	update(); 
	pn=Get_Node(0L,sla); 
	if(pn.s || pn.b<0) 
		return(-2); 
	if(pn.b) 
		return(-3);       /* для 1-го поля - отсутствие дерева */ 
	if((max=pn.x[0])>ST_DEPTH) 
		return(-4); 
	for(q=pn.x[1];q>0L;pn=qn) 
	{ 
		qn=Get_Node(q,sla); 
		if(lvl>=max || qn.s) 
		{ 
			h=-2; 
			break; 
		} 
		i=comp_vnode(des_field,q,sla,inode); 
		if(i) 
		{ 
			record=q; 
			pn.s=!fromleft; 
			na[lvl++]=pn; 
			if(i<0) 
			{ 
				fromleft=1; 
				q=qn.x[0]; 
			} 
			else 
			{ 
				fromleft=0; 
				q=qn.x[1]; 
			} 
		} 
		else 
		{       /* удаляемый узел найден */ 
			if(!qn.x[1]) 
			{ 
				h=1; 
				if(fromleft) 
					pn.x[0]=qn.x[0]; 
				else    pn.x[1]=qn.x[0]; 
			} 
			else if(!qn.x[0]) 
			{ 
				h=1; 
				if(fromleft) pn.x[0]=qn.x[1]; 
				else pn.x[1]=qn.x[1]; 
			} 
			else   /* заменим удаляемый узел на узел из более длинной ветви */ 
			{ 
				h=_del(sla,fromleft?&pn.x[0]:&pn.x[1],qn,na+lvl,max-lvl,qn.b<0?0:1); 
			} 
//                        qn.s=1; 
//                        Put_Node(q,sla,qn);   /* пометим узел удал. */ 
 
			for(;h>0 && lvl--;fromleft=!pn.s,pn.s=0) 
			{ 
				qn=pn; 
				q=record; 
				pn=na[lvl]; 
				record=lvl?(na[lvl-1].s?na[lvl-1].x[1]:na[lvl-1].x[0]):0; 
				h=balance(sla,pn.s?&pn.x[1]:&pn.x[0],&qn,fromleft?0:1); 
				Put_Node(q,sla,qn); 
			} 
			if(h>0) 
				pn.x[0]--; /* высота дерева уменьшилась */ 
			Put_Node(record,sla,pn); 
			break; 
		} 
	} 
	return(h); 
} 
 
int CX_BASE::fill_vnode(long record,struct sla *sla,struct node_val *inode) 
{ 
	inode->record=record; 
	if(inode->Dtree)      // D-tree 
	{ 
		TPage *val=(TPage *)Get_Buf(fd+2,record*PAGESIZE,PAGESIZE);
		if(val!=NULL) 
		{ 
			int l=val->len(val->buf+4);
			inode->ch=(char *)malloc(l);
			bcopy(val->buf,inode->ch,l);
		} 
		else 
			inode->ch=(char *)calloc(8,1); 
	} 
	else if(Get_Field_Descr(&ss,sla)->a==X_TEXT) 
		Get_Slot(record,sla,inode->ch); 
	else 
		Read(record,sla,inode->ch); 
	if(inode->ch==NULL) 
		inode->ch=(char *)calloc(1,1); 
	return(0); 
} 
 
/* сравнение полей */ 
int CX_BASE::comp_vnode(struct field *des_field,long record,struct sla *sla,struct node_val *inode,int len,int arg) 
{ 
	int i; 
	dlong j; 
	char *ch=NULL; 
 
	if(inode->Dtree)   // D-tree 
	{ 
		TPage *val=(TPage *)Get_Buf(fd+2,record*PAGESIZE,PAGESIZE);
		if(val==NULL) 
			return(1); 
		if(val->hdr.a==X_TEXT) 
		{ 
			int len1=val->len_field(inode->ch+4);
			char *a1=(char *)calloc(len1+1,1); 
			bcopy(val->value(inode->ch+4),a1,len1);
			int len2=val->len_field(val->buf+4);
			char *a2=(char *)calloc(len2+1,1); 
			bcopy(val->value(val->buf+4),a2,len2);
			j=strcmpw(a1,a2); 
			free(a1); 
			free(a2); 
		} 
		else j=Compare(des_field,inode->ch+4,val->buf+4,len);
		if(j==0) 
			j=val->rec(inode->ch)-val->rec(val->buf); 
		return(j>0?1:j<0?-1:0); 
	} 
	else 
	{ 
		if(des_field->a==X_TEXT || des_field->a==X_STRING)
			Get_Slot(record,sla,ch); 
		else
			Read(record,sla,ch);
		if(ch==NULL) 
			ch=(char *)calloc(4,1); 
		j=Compare(des_field,inode->ch,ch,len); 

		if(ch!=NULL)
			free(ch); 
	} 
	i=j>0?1:j<0?-1:0; 
	return(i?i:arg?0:inode->record-record); 
} 
 
void CX_BASE::Create_Tree() 
{ 
	int field; 
	char *name=(char *)malloc(strlen(__name)+16); 
 
	if(Wlock(0))
		free(name);
		return; 
	for(field=1;field<=Num_Fields();field++) 
	{ 
		if(!is_index(field)) 
			continue; 
		sprintf(name,"%s/Tree.%d",Name_Base(),field); 
#ifndef WIN32 
		truncate(name,0); 
#endif 
	} 
	struct key KEY; 
	bzero(&KEY,sizeof KEY); 
	if(Put_Buf(fd,(off_t)0,sizeof KEY,(char *)&KEY)<0) 
		return; 
	free(name); 
	for(int page=1;page<=max_record;page++) 
	{ 
//                if(!(page%1000))
//                        printf("%d\n",page);
		for(field=1;field<=Num_Fields();field++) 
		{ 
			if(!is_index(field)) 
				continue; 
			Insert_Node(page,field); 
		} 
	} 
	Unlock(0); 
} 
