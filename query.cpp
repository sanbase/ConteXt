/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:query.cpp
*/
#include "stdafx.h" 
#include "CX_Browser.h" 
extern Terminal *term;
 
extern CX_BROWSER *current_browser; 
 
static long Get_Record(long page,CX_BROWSER *browser) 
{ 
	return(browser->Record(page)); 
} 
 
int CX_BROWSER::Query(struct query *query) 
{ 
	long page=Record(index); 
	int num_rez=0; 
 
	selection sel; 
	if(db->cx_cond&SORT) 
	{ 
		sel.num_index=db->max_index; 
	} 
	current_browser=this; 
	num_rez=db->Select(query->sla,query->str,&sel,Get_Record); 
 
	if(Arr_To_Index(sel.index,num_rez)>0) 
		Add_Stack(query,num_rez);
 
	if(db->share!=NULL) 
	{ 
		if(num_rez<0) 
		{ 
			strcpy(db->share->output,message(10)); 
			num_rez=0; 
		} 
		else if(num_rez) 
			sprintf(db->share->output,message(8),num_rez); 
		else 
			strcpy(db->share->output,message(9)); 
	} 
	index=find_page(page); 
	return(num_rez); 
} 
