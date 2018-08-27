#include "../CX_Browser.h"

struct
{
	int total;
	int group;
	int current;
	int cur_group;
} l[10];

extern int l_x;

int draw_struct(CX_BASE *db, struct sla *sla1, int level, int num, int n, int atr)
{
	struct sla sla[16];
	int nn=0;

	bcopy(sla1,sla,sizeof sla);
	if(atr)
		l[level].cur_group++;
	for(sla[level].n=1;sla[level].n<=num;sla[level].n++)
	{
		struct field *f=db->Field_Descr(sla);
		if(atr)
		{

			if(l[level].total+l[level].group<l_x)
			{
				dpp((l_x-l[level].total-l[level].group)/2+l[level].cur_group+l[level].current++,level*4);
				if(f->a==STRUCTURE)
				{
					Set_Color(016,8);
					dpo('S');
				}
				else
				{
					Set_Color(012,8);
					dpo('F');
				}
//                        dps(f->name);
			}
			else return(-1);
		}
		else l[level].total++;
		if(f->a==STRUCTURE)
		{
			if(!atr)
				l[level+1].group++;
			draw_struct(db,sla,level+1,f->st.st->ptm,++nn,atr);
		}
	}
	return(0);
}

main(int argc, char **argv)
{
	CX_BASE *db;
	try
	{
		db=new CX_BASE(argv[1]);
	}
	catch(int i)
	{
		exit(1);
	}
	dpbeg("");

	int num=db->Num_Fields();
	struct sla sla[16];
	bzero(sla,sizeof sla);
	int nn=0;
	bzero(l,sizeof l);
	l[0].group=1;

	for(sla[0].n=1;sla[0].n<=num;sla[0].n++)
	{
		struct field *f=db->Field_Descr(sla);
		l[0].total++;
		if(f->a==STRUCTURE)
		{
			l[1].group++;
			draw_struct(db,sla,1,f->st.st->ptm,++nn,0);
		}
	}

	nn=0;
	for(sla[0].n=1;sla[0].n<=num;sla[0].n++)
	{
		struct field *f=db->Field_Descr(sla);
		Set_Color(016,8);
		dpp((l_x-l[0].total)/2+l[0].current++,0);
		if(f->a==STRUCTURE)
		{
			Set_Color(016,8);
			dpo('S');
		}
		else
		{
			Set_Color(012,8);
			dpo('F');
		}
		if(f->a==STRUCTURE)
		{
			if(draw_struct(db,sla,1,f->st.st->ptm,++nn,1)<0)
				break;
		}
	}
	int i;
	i=dpi();
	if(i>'0' && i<'9')
	{
			bzero(l,sizeof l);
			bzero(sla,sizeof sla);

			sla[0].n=i-'0';
			struct field *f=db->Field_Descr(sla);
			if(f->a==STRUCTURE)
			{
				l[1].group++;
				draw_struct(db,sla,1,f->st.st->ptm,1,0);
			}
			f=db->Field_Descr(sla);
			dpp(0,0); Set_Color(010,010); dpo(es);
			draw_struct(db,sla,1,f->st.st->ptm,1,1);
			dpi();
	}
	dpend();
	exit(0);
}
