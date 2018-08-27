/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:query.cpp
*/
#include "StdAfx.h"
#include "CX_BASE.h"
#include "../SCREEN/screen.h"

#include "../CX_Browser.h"
#ifdef WIN32
#define SERVER
#endif

#ifndef SERVER
extern Terminal *term;
#endif

CX_BROWSER *current_browser;
static selection *sel_std;

Query::Query(CX_BASE *db)
{
	bzero(this,sizeof *this);
	this->db=db;
}

static long get_record(long index,CX_BROWSER *cx)
{
	if(sel_std==NULL || sel_std->num_index==0)
	{
		return(index);
	}
	return(sel_std->Index(index));
}

int Query::Compare(char *str)
{
	double b;
	int i=0;

	if(rq->emp)
		b=str!=NULL && *str;
	else if(str==NULL)
		b=0;
	else switch(rq->ttip)
	{
		case X_DATE:
			b=conv_date(str);
			break;
		case X_TIME:
			b=conv_time(str);
			break;
		default:
			b=db->atof(str);
	}
	switch(*rq->cmd)
	{
		case '>':
			i=b>rq->a;
			break;
		case '<':
			i=b<rq->a;
			break;
		case '=':
			i=b==rq->a;
			break;
		default:
			if(rq->emp)
				i=b==0;
			else
			{
				if(rq->ttip==X_VARIANT && rq->short_flex)
				{
					char *ch=strchr(str,':');
					if(ch!=NULL)
						*ch=0;
				}
				i=strdif((unsigned char *)rq->obr,(unsigned char *)str,LINESIZE)==0;
			}
			break;
	}
	return(i);
}


long Query::Select(struct sla *sla,char *query,selection *select, long (*record)(long, CX_BROWSER *))
{
	long page=0,max_index;
	long *rez_set=NULL;
	long num_rez=0;
	char *str=NULL,*ch;
	int tree=0;
	int btree=0;
	int f=-1,x,y;
	int n,array=0;
	struct rq Rq;
	int type_field=0;

	if(select==NULL)
		return(-1);
	rq=&Rq;
	bzero(rq,sizeof Rq);

	if(record==NULL)
	{
		sel_std=select;
		record=get_record;
	}
	if(select->num_index==0)
	{
		rq->no_sel=1;
		max_index=db->last_cadr();
	}
	else
	{
		max_index=select->num_index;
		rq->no_sel=0;
	}
	rez_set=NULL;
	rq->obr=query;

	CX_FIND find(db);

	if(!strcmp(query,"="))
	{
		Sorting(&sla,1,select,0);
#ifndef SERVER
		if(term!=NULL && current_browser!=NULL && (max_index>1000 || db->Field_Descr(sla)->a==0))
		{
			x=(term->l_x()-50)/2;
			y=(term->l_y()/2);
			f=term->get_box(x-2,y-2,50+6,5);
			term->BOX(x-1,y-1,52,3,' ',41,41,41,41),
			term->Set_Color(016,0);
			term->dpp(x,y);
			term->scrbufout();
		}
#endif
		char *ch1=NULL,*ch2=NULL;
		for(page=1,n=0;page>0 && page<=max_index;page++)
		{
			long rec;

			rec=select->index[page-1];
#ifndef SERVER
			if(term!=NULL && max_index>50 && current_browser!=NULL && f>=0 && (page%(max_index/50))==0 && n<50)
			{
				term->dpo(' ');
				n++;
				term->scrbufout();
			}
#endif
			if(db->Check_Del(rec))
				continue;
			db->Get_Slot(rec,sla,ch1);
			if(ch1==NULL)
				continue;
			if(ch2==NULL || strcmpw(ch1,ch2))
			{
				if(ch2)
					free(ch2);
				ch2=ch1;
				ch1=NULL;
				rez_set=(long *)realloc(rez_set,(++num_rez)*sizeof (long));
				rez_set[num_rez-1]=rec;
			}
		}
		if(ch1)
			free(ch1);
		if(ch2)
			free(ch2);
		goto END;
	}

	array=db->Field_Descr(sla)->m;
	if(!array)
	{
		for(array=0;array<SLA_DEEP && sla[array].n;array++);

		for(;;array--)
		{
			if(sla[array].m)
			{
				sla[array].m=0;
				if(db->Field_Descr(sla)->m)
					array=0;
				array++;
				break;
			}
			if(!array)
				break;
		}
	}

	if(*rq->obr=='[' && (ch=strrchr(rq->obr,']'))!=NULL)
	{

		long page1,page2;
		if((n=sscanf(rq->obr,"[%ld-%ld]",  &page1,&page2))  !=2 &&
		   (n=sscanf(rq->obr,"[#%ld-#%ld]",&page1,&page2))!=2)
			return(-1);
		if(page1<1)
			page1=1;
		if(page2<1)
			page2=1;
		if(page1>max_index)
			page1=max_index;
		if(page2>max_index)
			page2=max_index;
		if(page1>page2)
		{
			long page=page2;
			page2=page1;
			page1=page;
		}

		for(num_rez=0;page1<=page2;page1++)
		{
			long page=record(page1,current_browser);
			if(db->Check_Del(page))
				continue;
			rez_set=(long *)realloc(rez_set,(++num_rez)*sizeof (long));
			rez_set[num_rez-1]=page;
		}
		goto END;
	}
	if(*rq->obr=='!')
	{
		rq->no=1;
		rq->obr++;
	}
	rq->cmd=rq->obr;
	if(!strcmp(rq->obr,"-") || !strcmp(rq->obr,"NULL") || !strcmp(rq->obr,"null"))
		rq->emp=1;
	else    rq->emp=0;
	if(*rq->obr=='=' || *rq->obr=='>' || *rq->obr=='<')
		rq->obr++;

	type_field=db->Field_Descr(sla)->a;

	if (strchr(rq->obr,'*')!=NULL || strchr(rq->obr,'?')!=NULL)
		if (type_field!=X_STRING && type_field!=X_TEXT)
			goto NO_TREE;


	btree=db->Field_Descr(sla)->b && rq->no_sel && *rq->cmd!='<';

	if(!rq->emp && rq->no_sel && !rq->no && *rq->cmd!='<' && ((*rq->obr!='*' && db->open_Tree(sla)>0) || btree))
	{
		int p;

		if((p=db->is_pointer(sla->n)) && (*rq->cmd!='#' || sla[1].n))
		{
			char *name;
			if((name=db->Name_Subbase(sla->n))!=NULL && strchr(name,':')==NULL)
			{
				CX_BASE *subbase=NULL;
				if((subbase=db->open_db(name))!=NULL &&subbase->open_Tree(sla+1)>0)
				{
					select->num_index=0;
					int num=subbase->Select(sla+1,query,select);
					if(!num)
						return(0);
					long *sb=select->index;
					select->index=NULL;
					for(int i=0;i<num;i++)
					{
						char str[64];
						struct sla SLA[SLA_DEEP];

						bcopy(sla,SLA,sizeof SLA);
						SLA[p].n=0;
						sprintf(str,"#%ld",sb[i]);
						select->num_index=0;
						int n=Select(SLA,str,select,record);
						if(n>=0)
						{
							rez_set=(long *)realloc(rez_set,(num_rez+n)*sizeof (long));
							bcopy(select->index,rez_set+num_rez,n*sizeof (long));
							num_rez+=n;
						}
						if(select->index!=NULL)
							free(select->index);
						select->index=NULL;
					}
					if(sb!=NULL)
						free(sb);
					goto END;
				}
			}
			goto NO_TREE;
		}
		char *ch,*req;
		req=(char *)malloc(strlen(rq->obr)+1);
		strcpy(req,rq->obr);
	      //  if((ch=strchr(req,'"'))==NULL)
		{
			ch=strchr(req+1,'*');
			if(ch!=NULL)
				*ch=0;
		}
		/*else
		{
			if(strchr(ch+1,'"')!=NULL)
			{
				bcopy(ch+1,ch,strlen(ch));
				ch=strchr(ch,'"');
				bcopy(ch+1,ch,strlen(ch));
			}
			ch=NULL;
		} */
		if(!btree)
		{
			if((page=find.Find_First(sla,req,*rq->cmd=='>'?2:ch!=NULL?1:0))<0)
			{
				free(req);
				goto END;
			}
		}
		free(req);
		tree=1;
	}
	else
	{
NO_TREE:
		page=1;
#ifndef SERVER
		if(term!=NULL && current_browser!=NULL && (max_index>1000 || db->Field_Descr(sla)->a==0))
		{
			x=(term->l_x()-50)/2;
			y=(term->l_y()/2);
			f=term->get_box(x-2,y-2,50+6,5);
			term->BOX(x-1,y-1,52,3,' ',41,41,41,41),
			term->Set_Color(016,0);
			term->dpp(x,y);
			term->scrbufout();
		}
#endif
	}
	rq->ttip=db->Field_Descr(sla)->a;
	if(rq->ttip==X_VARIANT && *rq->obr=='#')
		rq->short_flex=strchr(rq->obr,':')==NULL;
	else    rq->short_flex=0;
	if(rq->emp)
		rq->a=0;
	else switch(rq->ttip)
	{
		case X_DATE:
			rq->a=conv_date(rq->obr);
			break;
		case X_TIME:
			rq->a=conv_time(rq->obr);
			break;
		default:
			rq->a=db->atof(rq->obr);
	}

	if(btree)
	{
		char *ch=NULL;
		if(db->String_To_Buf(rq->obr,*db->Field_Descr(sla),ch,0,0,1)<0)
			goto END;
//                num_rez=db->FindBP(sla->n,rq->obr,rez_set);

		num_rez=db->FindBP(sla->n,ch,rez_set,(*rq->cmd=='>')?1:0);

		if(ch!=NULL)
			free(ch);
		if(rq->no && num_rez)
		{
			int num=max_index-num_rez;
			long *rez=NULL;
			for(int i=0,j=0,k=0;i<max_index;i++)
			{
				if(i+1!=rez_set[j])
				{
					rez=(long *)realloc(rez,(k+1)*sizeof (long));
					rez[k++]=i+1;
					continue;
				}
				j++;

			}
			free(rez_set);
			rez_set=rez;
			num_rez=num;
		}
		goto END;
	}
	for(n=0;page>0 && page<=max_index;tree?(page=find.Next()):page++)
	{
		int i;
		long rec;

		rec=rq->no_sel?page:record(page,current_browser);
#ifndef SERVER
		if(term!=NULL && max_index>50 && current_browser!=NULL && f>=0 && (page%(max_index/50))==0 && n<50)
		{
			term->dpo(' ');
			n++;
			term->scrbufout();
		}
#endif
		if(db->Check_Del(rec))
			continue;

		if(array && sla->n<=db->ss.ptm)
		{
			int num=db->Num_Elem_Array(rec,sla);
			for(sla[array-1].m=1;sla[array-1].m<=num;sla[array-1].m++)
			{
				db->Get_Slot(rec,sla,str);
				i=Compare(str);

				if(i^rq->no)
				{
					if((rez_set=(long *)realloc(rez_set,++num_rez*sizeof(long)))==NULL)
					{
						num_rez=-1;
						goto END;
					}
					rez_set[num_rez-1]=rec;
					break;
				}
			}
		}
		else
		{
			db->Get_Slot(rec,sla,str);
			i=Compare(str);
			if(i^rq->no)
			{
				if((rez_set=(long *)realloc(rez_set,++num_rez*sizeof(long)))==NULL)
				{
					num_rez=-1;
					break;
				}
				rez_set[num_rez-1]=rec;
			}
			else if(tree)
			{
				double b;
				if(rq->emp)
					b=0;
				else switch(rq->ttip)
				{
					case X_DATE:
						b=conv_date(str);
						break;
					case X_TIME:
						b=conv_time(str);
						break;
					default:
						b=db->atof(str);
				}
				if(*rq->cmd=='>' && b==rq->a)
					continue;
				break;
			}
		}
	}
	if(str!=NULL)
		free(str);
END:
	select->num_index=num_rez;
	if(num_rez)
	{
		if(select->index!=NULL)
			free(select->index);
		select->index=rez_set;
	}
#ifndef SERVER
	if(f>=0 && term!=NULL)
	{
		term->restore_box(f);
		term->free_box(f);
	}
#endif
	return(num_rez);
}

int Query::Sorting(struct sla **sla,int num_fields,selection *select,int nap)
{
	if(select==NULL)
		return(-1);
	int ttree=0;
	if(select->num_index==0)
	{
		if(db->Field_Descr(*sla)->b && num_fields==1)
		{
			db->SortBP((*sla)->n,select,nap);
			return 1;
		}
		if(num_fields==1 && db->open_Tree(*sla)>0 && db->Field_Descr(*sla)->k)
			ttree=1;
		select->num_index=db->last_cadr();
	}
	if(select->num_index<2) // нечего сортировать
		return(0);
	ff=-1;
#ifndef SERVER
	if(term!=NULL && current_browser!=NULL && (select->num_index>1000 || db->Field_Descr(sla[0])->a==0))
	{
		xx=(term->l_x()-50)/2;
		yy=(term->l_y()/2);
		ff=term->get_box(xx-2,yy-2,50+6,5);
		term->BOX(xx-1,yy-1,52,3,' ',41,41,41,41),
		term->Set_Color(016,0);
		term->dpp(xx,yy);
		n=0;
		term->scrbufout();
		curr_page=1;
	}
#endif
	int i;
	num_records=select->num_index;
	if(ttree)
	{
		CX_FIND f(db,nap>0?1:-1);

		long record=f.Find_First(*sla,"",-1);
		if(record<=0)
			return(-1);
		select->index=(long *)realloc(select->index,num_records*sizeof (long));
		select->index[0]=record;
		for(int i=1;i<num_records;i++)
		{
			select->index[i]=f.Next();
#ifndef SERVER
			if(term!=NULL && num_records>50 && current_browser!=NULL && ff>=0 && (i%(num_records/50))==0 && n<50)
			{
				term->dpo(' ');
				n++;
				term->scrbufout();
			}
#endif
		}
#ifndef SERVER
		if(ff>=0 && term!=NULL)
		{
			term->restore_box(ff);
			term->free_box(ff);
			ff=-1;
		}
#endif
		return(1);
	}
	struct val *val=(struct val *)calloc(select->num_index,sizeof (struct val));
	for(i=0;i<select->num_index;i++)
	{

		if(current_browser!=NULL)
			val[i].record=db->Record_Index(i+1);
		else
			val[i].record=select->index[i];
	}

	SLA=sla;
	NUM=num_fields;
	tip=(int *)calloc(num_fields,sizeof (int));
	num=(int *)calloc(num_fields,sizeof (int));
	for(i=0;i<num_fields;i++)
	{
		tip[i]=db->Field_Descr(sla[i])->a;
		if(tip[i]==0)   // VIRTUAL
		{
			char *ch=NULL;
			db->Get_Slot(1,sla[i],ch);
			if(ch!=NULL)
			{
				tip[i]=X_DOUBLE;
				for(int j=0;j<(int)strlen(ch);j++)
				{
					if((ch[j]<'0' || ch[j]>'9') && ch[j]!='.' && ch[j]!='-' && ch[j]!='+' && ch[j]!=' ')
					{
						tip[i]=0;
						break;
					}
				}
				//fill проверяем даату если есть длина(иначе всегда будет дата)
				//и если формат ch числовой иначе считаем что это шоу поле непонятного
				//разлива
				if (tip[i]!=0&&strlen(ch)>0)
				{
					if (check_date(ch)==0)
					tip[i]=X_DATE;
				}
				else
				tip[i]=0;

				free(ch);
			}
		}
		num[i]=db->Field_Descr(sla[i])->n;
	}
	NAP=nap;
	qsort(val,select->num_index,sizeof (struct val));
	free(tip);
	free(num);

	select->index=(long *)realloc(select->index,select->num_index*sizeof (long));

	for(i=0;i<select->num_index;i++)
		select->index[i]=val[i].record;
	free(val);
#ifndef SERVER
	if(ff>=0 && term!=NULL)
	{
		term->restore_box(ff);
		term->free_box(ff);
		ff=-1;
	}
#endif
	return(1);
}

union value Query::get_value(int tip,char *str)
{
	union value value;
	bzero(&value, sizeof value);
	switch(tip)
	{
		case X_DOUBLE:
		case X_FLOAT:
		case X_INTEGER:
		case X_UNSIGNED:
			value.d=db->atof(str);
			break;
		case X_DATE:
			value.i=conv_date(str);
			break;
		case X_TIME:
			value.i=conv_time(str);
			break;
		case X_VARIANT:
		{
			long page=0,spage=0;
			char str[32];

			if(sscanf(str,"#%ld:%ld",&spage,&spage)!=2)
			{
				if(sscanf(str,"%ld:%ld",&spage,&spage)!=2)
				{
					value.i=0;
					break;
				}
			}
			value.i=(spage<<16)+page;
			break;
		}
		case X_POINTER:
			if(*str=='#')
				value.i=atoi(str+1);
			else
				value.i=atoi(str);
			break;
		default:
			if(str==NULL)
				bzero(value.str,sizeof (value.str));
			else
				bcopy(str,value.str,sizeof (value.str));
			break;
	}
	return(value);
}

int Query::different(struct val *v1,struct val *v2,int i)
{
	int rez=0;

	switch(tip[i])
	{
		case X_DATE:
		case X_TIME:
		case X_POINTER:
		case X_VARIANT:
			rez=v1->v.i>v2->v.i?1:v1->v.i<v2->v.i?-1:0;
			break;
		case X_DOUBLE:
		case X_FLOAT:
		case X_INTEGER:
		case X_UNSIGNED:
			rez=v1->v.d>v2->v.d?1:v1->v.d<v2->v.d?-1:0;
			break;
		default:
		{
		     //fill  не знаю почему при сортироке не учитывался файл весов
		     //int j=strncmp(v1->v.str,v2->v.str,sizeof (v1->v.str));
			int j=strcmpw(v1->v.str,v2->v.str,sizeof (v1->v.str));
			rez=j>0?1:j<0?-1:0;

			if(rez==0 && strlen(v1->v.str)>=sizeof(v1->v.str))
			{
				char *ch1=NULL,*ch2=NULL;

				db->Get_Slot(v1->record,SLA[i],ch1);
				db->Get_Slot(v2->record,SLA[i],ch2);
				j=strcmpw(ch1,ch2);
				rez=j>0?1:j<0?-1:0;
				if(ch1!=NULL)
					free(ch1);
				if(ch2!=NULL)
					free(ch2);
			}
			break;
		}
	}
#ifndef SERVER
	if(term!=NULL && num_records>50 && current_browser!=NULL && ff>=0 && (curr_page%(num_records/50))==0)
	{
		if(curr_page>=num_records)
		{
			term->Set_Color(0x209+curr_color++,0);
			if(curr_color==7)
				curr_color=0;
			term->dpp(xx,yy);
			n=0;
			curr_page=0;
		}
		if(n<50)
			term->dpo(' ');
		n++;
		term->scrbufout();
	}
#endif
	curr_page++;
	if(rez==0 && i<NUM-1)
	{
		struct val val1,val2;
		char *ch=NULL;

		val1.record=v1->record;
		val2.record=v2->record;
		i++;
		db->Get_Slot(val1.record,SLA[i],ch);
		val1.v=get_value(tip[i],ch);
		db->Get_Slot(val2.record,SLA[i],ch);
		val2.v=get_value(tip[i],ch);
		if(ch)
			free(ch);
		v1=&val1;
		v2=&val2;
		rez=different(&val1,&val2,i);
	}
	return(rez);
}

int Query::Compare(const void *a,const void *b)
{
	int rez=0;
	struct val *v1=(struct val *)a;
	struct val *v2=(struct val *)b;
	char *ch1=NULL, *ch2=NULL;


	if(!v1->r)
	{
		db->Get_Slot(v1->record,SLA[0],ch1);
		v1->v=get_value(tip[0],ch1);
		v1->r=1;
#ifndef SERVER
		if(term!=NULL && num_records>50 && current_browser!=NULL && ff>=0 && (curr_page%(num_records/50))==0)
		{
			if(n<50)
				term->dpo(' ');
			n++;
			term->scrbufout();
		}
#endif
		curr_page++;
	}
	if(!v2->r)
	{
		db->Get_Slot(v2->record,SLA[0],ch2);
		v2->v=get_value(tip[0],ch2);
		v2->r=1;
	}
	rez=different(v1,v2,0);
	if(ch1!=NULL)
		free(ch1);
	if(ch2!=NULL)
		free(ch2);
	if(rez==0)
	{
		rez=v1->record>v2->record?1:v1->record<v2->record?-1:0;
	}
	if(NAP)
		rez=-rez;
	return(rez);
}

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#ifndef min
#define min(a, b)       (a) < (b) ? a : b
#endif
#define swapcode(TYPE, parmi, parmj, n) {               \
	long i = (n) / sizeof (TYPE);                   \
	register TYPE *pi = (TYPE *) (parmi);           \
	register TYPE *pj = (TYPE *) (parmj);           \
	do {                                            \
		register TYPE   t = *pi;                \
		*pi++ = *pj;                            \
		*pj++ = t;                              \
	} while (--i > 0);                              \
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

void Query::swapfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)                                      \
	if (swaptype == 0) {                            \
		long t = *(long *)(a);                  \
		*(long *)(a) = *(long *)(b);            \
		*(long *)(b) = t;                       \
	} else                                          \
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n)  if ((n) > 0) swapfunc((char *)a, b, n, swaptype)

char *Query::med3(char *a, char *b, char *c)
{
	return Compare(a, b) < 0 ? (Compare(b, c) < 0 ? b : (Compare(a, c) < 0 ? c : a )):(Compare(b, c) > 0 ? b : (Compare(a, c) < 0 ? a : c ));
}

void Query::qsort(void *a, size_t n, size_t es)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int d, r, swaptype, swap_cnt;

loop:   SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + n*es; pm+=es)
			for (pl=pm; pl > (char *) a && Compare(pl - es, pl) > 0;
			     pl-=es)
				swap(pl, pl-es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7)
	{
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40)
		{
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d);
			pm = med3(pm-d, pm, pm+d);
			pn = med3(pn-2 * d, pn-d, pn);
		}
		pm = med3(pl, pm, pn);
	}
	swap((char *)a, pm);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;)
	{
		while (pb <= pc && (r = Compare(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = Compare(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0)
	{
		for (pm = (char *)a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && Compare(pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}

	pn = (char *)a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb-r, r);
	r = min((int)(pd-pc), (int)(pn-pd-es));
	vecswap(pb, pn-r, r);
	if((r=(int)(pb-pa))>(int)es)
		qsort(a, r / es, es);
	if ((r=(int)(pd-pc))>(int)es)
	{
		a = pn - r;
		n = r / es;
		goto loop;
	}
}
