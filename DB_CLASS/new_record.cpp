/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:new_record.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term;
 
// create a new Object (empty). Attention! The record still be locked. 
long CX_BASE::Write_Empty_Record(long record)
{

	if(Wlock(0)<0)
		return(-1);
	if(Wlock(record))
	{
		Unlock(0);
		return(-2);
	}

	char *ch;
	int ret;
	if(record>max_record)
	{
		ch=(char *)calloc(ss.size,1);
		ret=Put_Buf(fd,root_size+(off_t)(record-1)*ss.size,ss.size,ch);
		free(ch);
	}
	else
	{
		ch=(char *)calloc(len_cadr,1);
		ret=Cadr_Write(record,ch);
		free(ch);
	}
	update();
	Unlock(0);
	return ret;
}
long CX_BASE::New_Record(int arg) 
{ 
	long record=0;
	char *ch=NULL;
	struct root root; 
	if(Wlock(0)<0)
		return(-1); 
	update(); 
	if(max_record==0) 
	{ 
		if((ch=Get_Buf(fd,0,sizeof (struct root)))==NULL) 
			bzero(&root, sizeof (struct root)); 
		else    bcopy(ch,&root, sizeof (struct root)); 
		Put_Buf(fd,0,sizeof (struct root),(char *)&root); 
		if(root_size>(int)sizeof (struct root)) 
		{ 
			char *b=(char *)calloc(root_size-sizeof (struct root),1); 
			Put_Buf(fd,sizeof (struct root),root_size-sizeof (struct root),b); 
			free(b); 
		} 
		update(); 
	} 
BEG:
	if(ss.field->k && context->pswd!=CXKEY3)
	{
		struct node node=Get_Node(0,0);
		if(node.x[1]>0)
		{
			TPage *t=(TPage *)Get_Buf(fd+2,node.x[1]*PAGESIZE,PAGESIZE);
			if(t!=NULL)
			{
				record=t->rec(t->MIN());
			}
		}

	}
	else
	{
		if((ch=Get_Buf(fd,0,sizeof (struct root)))==NULL)
		{
			Unlock(0);
			return(-1);
		}
		bcopy(ch,&root,sizeof (struct root));
		record=-root.del_chain.r;
	}
	if(record<=0 || context->b0&KEEP_DELETE || hist!=NULL || record>max_record || !Check_Del(record))
	{ 
NEW_REC:
		update(); 
		record = max_record + 1; 
		if(Wlock(record)) 
		{ 
#ifndef WIN32 
			sleep(1); 
#endif 
			update(); 
			record = max_record + 1; 
			if(Wlock(record)) 
			{ 
				Unlock(0); 
				return(-2); 
			} 
		} 
	} 
	else 
	{ 
		if(record>max_record) 
			goto NEW_REC;
		if(Wlock(record)) 
		{ 
#ifndef WIN32 
			sleep(1); 
#endif 
			update(); 
			goto BEG; 
		} 
		Restore_Record(record,ss.field->k && context->pswd!=CXKEY3?-1:0);
	} 
	ch=(char *)calloc(ss.size,1); 
	Put_Buf(fd,root_size+(off_t)(record-1)*ss.size,ss.size,ch);
	free(ch); 
	update(); 
 
	if(arg==0) 
	{ 
		for(int i=1;i<=context->ptm;i++) 
		{ 
			if((ss.field[i-1].k || ss.field[i-1].b) && context->pswd!=CXKEY3)
				continue; 
			if(open_Tree(i)>0 && Get_Field_Descr(i)->k==0) 
			{ 
				Insert_Node(record,i); 
			} 
		} 
	} 
	Unlock(0); 
	if(change_db!=NULL)
	{
		/*запишем историю в changedb fill*/
		 write_change(record,-1);
	       /* long page=change_db->New_Record(1);
		change_db->Unlock(page);*/
	}
	return(record); 
} 
