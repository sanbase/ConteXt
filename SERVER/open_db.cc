#include "CX_BASE.h"

CX_BASE **open_base=NULL;
int num_open_bases=0;

void clean_cx()
{
	for(int i=0;i<num_open_bases;i++)
		delete open_base[i];
	if(open_base!=NULL)
	       free(open_base);
	open_base=NULL;
	num_open_bases=0;
}

CX_BASE *open_db(char *name)
{
	for(int i=0;i<num_open_bases;i++)
	{
		if(!strcmp(open_base[i]->Name_Base(),name))
		{
			CX_BASE *tmp=open_base[i];
			bcopy(open_base+i+1,open_base+i,(num_open_bases-i-1)*(sizeof *open_base));
			bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base));
			open_base[0]=tmp;
			return(open_base[0]);
		}
	}
	CX_BASE *subbase;
	try
	{
		subbase = new CX_BASE(name);
	}
	catch(int i)
	{
		return(NULL);
	}
	if(num_open_bases==MAX_OPEN_BASE)
	{
		delete open_base[--num_open_bases];
	}
	open_base=(CX_BASE **)realloc(open_base,(++num_open_bases)*sizeof (CX_BASE *));
	bcopy(open_base,open_base+1,(num_open_bases-1)*(sizeof *open_base));
	return(open_base[0]=subbase);
}
