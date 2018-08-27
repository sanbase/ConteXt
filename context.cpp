/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:context.cpp
*/
#include "stdafx.h"
#include "CX_Browser.h"

static char info[80];
static char image_name[NAMESIZE];
static int image_x=0, image_y=0;

extern Terminal *term;
CX_BROWSER::CX_BROWSER(CX_BROWSER *STD):
BROWSER(NULL,0,0,0,0)
{
	zero();

	inherit=1;
	db          = STD->db;
	stack       = STD->stack;
	num_stack_sel=STD->num_stack_sel;
	prot_fd     = STD->prot_fd;
	keep_str    = STD->keep_str;
	read_only   = STD->read_only;
	shmid       = STD->shmid;

	cx_cond     = STD->cx_cond;
	record      = STD->record;
	cx3         = STD->cx3;
	if (STD->personal_selection!=NULL&&*STD->personal_selection)
	{
		personal_selection = (char *)malloc(strlen(STD->personal_selection)+1);
		strcpy(personal_selection,STD->personal_selection);
	}else
	personal_selection = NULL;

	if (STD->restrict!=NULL && *STD->restrict)
	{
		restrict    = (char *)malloc(strlen(STD->restrict)+1);
		strcpy(restrict,STD->restrict);
	}
	else
		restrict    = NULL;

	Load_Env(STD);
	read_only=1;
	ed=NULL;
	Create_Map(STD->tags[STD->act_field].des.sla,STD->tags[STD->act_field].des.l);
}

CX_BROWSER::CX_BROWSER(CX_BROWSER *STD,long REC,struct sla *sla,int max_len):
BROWSER(NULL,0,0,0,0)
{
	zero();

	record=REC;
	inherit=1;
	db          = STD->db;
	stack       = STD->stack;
	num_stack_sel=STD->num_stack_sel;
	prot_fd     = STD->prot_fd;
	keep_str    = STD->keep_str;
	read_only   = STD->read_only;
	shmid       = STD->shmid;

	cx_cond     = STD->cx_cond;
	record      = STD->record;
	cx3         = STD->cx3;
	if (STD->personal_selection!=NULL&&*STD->personal_selection)
	{
		personal_selection = (char *)malloc(strlen(STD->personal_selection)+1);
		strcpy(personal_selection,STD->personal_selection);
	}
	else
		personal_selection = NULL;

	if (STD->restrict!=NULL && *STD->restrict)
	{
		restrict    = (char *)malloc(strlen(STD->restrict)+1);
		strcpy(restrict,STD->restrict);
	}
	else
		restrict    = NULL;

	Load_Env(STD);
/*
	if(STD->restrict!=NULL)
	{
		set_prim_selection(STD->restrict);
		Read_Index(restrict);
	}
*/
	read_only=1;
	ed=NULL;
	Create_Map(sla,max_len);
}

CX_BROWSER::CX_BROWSER(CX_BROWSER *STD,long REC,int num,struct tag *des):
BROWSER(num,des,0,0,0,0)
{
	zero();

	record=REC;
	inherit=1;
	db          = STD->db;
	stack       = STD->stack;
	num_stack_sel=STD->num_stack_sel;
	prot_fd     = STD->prot_fd;
	keep_str    = STD->keep_str;
	read_only   = STD->read_only;
	shmid       = STD->shmid;

	cx3         = STD->cx3;
	if (STD->personal_selection!=NULL&&*STD->personal_selection)
	{
		personal_selection = (char *)malloc(strlen(STD->personal_selection)+1);
		strcpy(personal_selection,STD->personal_selection);
	}
	else
		personal_selection = NULL;

	if (STD->restrict!=NULL && *STD->restrict)
	{
		restrict    = (char *)malloc(strlen(STD->restrict)+1);
		strcpy(restrict,STD->restrict);
	}
	else
		restrict    = NULL;
	Load_Env(STD);
	cx_cond=STD->cx_cond;  // было cx_cond=0;
	form_cond|=TABLE;
	if(STD->form_cond&BW)
		form_cond|=BW;
	create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
	Set_Index();
	record=STD->record;
	bzero(obr,sizeof obr);
	index=find_page(record);
	New_Index();
	if(db->share!=NULL)
		Ind_Update();
	Form_Restruct();
	db->Cadr_Read(Record(index));
	form_update();
	load_menu();
}

CX_BROWSER::CX_BROWSER(CX_BROWSER *STD,int x, int y,char *name_blank):
BROWSER(NULL,x,y,0,0)
{

	zero();

	inherit=1;
	db          = STD->db;
	stack       = STD->stack;
	num_stack_sel=STD->num_stack_sel;
	prot_fd     = STD->prot_fd;
	keep_str    = STD->keep_str;
	read_only   = STD->read_only;
	shmid       = STD->shmid;

	cx_cond     = STD->cx_cond;
	record      = STD->record;
	cx3         = STD->cx3;

      //  personal_selection = STD->personal_selection;
	if (STD->personal_selection!=NULL&&*STD->personal_selection)
	{
		personal_selection = (char *)malloc(strlen(STD->personal_selection)+1);
		strcpy(personal_selection,STD->personal_selection);
	}
	else
		personal_selection = NULL;


	if (STD->restrict!=NULL && *STD->restrict)
	{
		restrict    = (char *)malloc(strlen(STD->restrict)+1);
		strcpy(restrict,STD->restrict);
	}
	else
		restrict    = NULL;


	term->MultiColor(x0,y0,l,h);


	Load_Env(STD);
       // read_only=1;
	ed=NULL;
	hidden=1;

	act_field=0;
	Del_Label();
	Free_Form();
	title=NULL;
	create_Form(db->Name_Base(),name_blank);
	create_geom(x,y,-term->l_x()+level-1,-term->l_y()+level);
//        create_geom(x,y,term->l_x()-level+1,term->l_y()-level);
	restore_bg();
	bzero(db->share->output,sizeof (db->share->output));
	Form_Restruct();

	New_Index();
	Go_To_Index(index);
	clean_menu();


//        f_mark_field=&mark_field;
//        f_num_mark_fields=&num_mark_fields;
}


CX_BROWSER::CX_BROWSER(char *name_base,long REC,struct sla *sla,int max_len,char *ch):
BROWSER(NULL,0,0,0,0)
{
	zero();

	form_cond=0;
	cx_cond=0;
	record=REC;

	try
	{
		db = new CX_BASE(name_base);
	}
	catch(...)
	{
		throw 9;
	}
	if(ch!=NULL && db->share!=NULL)
		strcpy(db->share->io,ch);
	personal_selection=NULL;
	if(REC==0 || REC>db->last_cadr())
		record=db->last_cadr();
	index=find_page(record);
	New_Index();
	read_only=1;
	ed=NULL;
	if(db->share!=NULL)
	{
		Ind_Update();
	}
	Create_Map(sla,max_len);
}


CX_BROWSER::CX_BROWSER(char *name_base,long REC,struct x_form *f,int x,int y,int w,int h):
BROWSER(name_base,f,x,y,w,h)
{

	level=0;
	initialize(name_base,REC,0,f);
}

CX_BROWSER::CX_BROWSER(char *name_base,long REC,struct x_form *x,int recurs,char *ch):
BROWSER(name_base,x,recurs,recurs,term->l_x()-recurs+1,term->l_y()-recurs)
{
	level=recurs;
	initialize(name_base,REC,0,x,ch);
}

CX_BROWSER::CX_BROWSER(CX_BROWSER *STD,long REC,struct x_form *x,int recurs):
BROWSER(STD->db->Name_Base(),x,recurs,recurs,term->l_x()-recurs+1,term->l_y()-recurs)
{
	level=recurs;
	initialize(STD,REC);
}

CX_BROWSER::CX_BROWSER(char *name_base,long REC,struct x_form *x,char *ch):
BROWSER(name_base,x,0,0,term->l_x()+1,term->l_y())
{
	level=0;
	initialize(name_base,REC,0,x,ch);
}

CX_BROWSER::CX_BROWSER(char *name_base,long REC,int blank,char *ch):
BROWSER(name_base,term->l_x()+1,term->l_y(),0,0)
{
	struct x_form x;
	x.form=blank;
	*x.blank=0;
	level=0;
	initialize(name_base,REC,0,&x,ch);
}

CX_BROWSER::CX_BROWSER(char *name_base,long REC,char *blank,char *ch):
BROWSER(name_base,term->l_x()+1,term->l_y(),0,0)
{
	struct x_form x;
	x.form=0;
	strcpy(x.blank,blank);;
	level=0;
	initialize(name_base,REC,0,&x,ch);
}


CX_BROWSER::~CX_BROWSER()
{
	Write();
	message(-1);
#ifndef WIN32
	if(!inherit && cx3!=NULL)
		delete cx3;
#endif
	if(!inherit && prot_fd>0)
		close(prot_fd);
	if(mark_field!=NULL)
	{
		for(int i=0;i<num_mark_fields;i++)
			if(mark_field[i].name!=NULL)
				free(mark_field[i].name);
		free(mark_field);
	}

	if(restrict!=NULL)
	{
		free(restrict);
		restrict=NULL;
	}
	if(!inherit && ed!=NULL)
	{
		delete ed;
		ed=NULL;
	}
	if(mark!=NULL)
	{
		free(mark);
		mark=NULL;
	}
	if(!inherit && db!=NULL)
	{
		delete db;
		db=NULL;
	}
	if(!inherit && keep_str!=NULL)
	{
		delete keep_str;
		keep_str=NULL;
	}
	if(*image_name)
	{
		if(!access(image_name,W_OK))
			unlink(image_name);
		term->Del_Image(image_x,image_y);
		*image_name=0;
		image_x=-1;
		image_y=-1;
	}
	if(personal_selection!=NULL)
	{
		free(personal_selection);
		personal_selection=NULL;
	}
/*
	if(!inherit)
		clean_menu();
*/
/*
	DIR *fd;
	struct dirent *dp;
	char str[64];
	sprintf(str,".%d.",getpid());
	if((fd=opendir("/var/www/docs/Files"))!=NULL)
	{
		while((dp=readdir(fd))!=NULL)
		{
			if(!strncmp(dp->d_name,str,strlen(str)))
				unlink(dp->d_name);
		}
		closedir(fd);
	}
	if((fd=opendir("/var/www/docs/Images/Tmp"))!=NULL)
	{
		while((dp=readdir(fd))!=NULL)
		{
			if(!strncmp(dp->d_name,str,strlen(str)))
				unlink(dp->d_name);
		}
		closedir(fd);
	}
*/
}

void CX_BROWSER::Create_Map(struct sla *sla,int max_len)
{
	int columns,lines;
	long *ind_array;
	struct tag *tgs;
	int nnum_fields;
BEGIN:
	if((nnum_fields=db->max_index)==0)
	{
		return;
	}
	if((columns=(term->l_x()-4)/(max_len+1))<=0)
		columns=1;
	lines=columns*(term->l_y()-6);
	if(nnum_fields>lines)
		nnum_fields=lines;
	for(;;)
	{
		if((columns=get_columns(nnum_fields,(term->l_x()-4)/(max_len+1)))==0)
			columns=1;
		lines = (nnum_fields+columns-1)/columns;
		if(lines<term->l_y()-6)
			break;
		if(nnum_fields>1000)
			nnum_fields/=2;
		else
			nnum_fields--;
	}
	ind_array=NULL;
	if(nnum_fields==0)
		nnum_fields=1;

	nnum_fields=Sub_Index(index,nnum_fields,ind_array);
	if(*obr)
	{
		char *ch=NULL;
		int i;

		for(i=0;i<nnum_fields;i++)
			if(ind_array[i]==record)
				break;
		if(i==nnum_fields)
			i=0;
		db->Get_Slot(Record(ind_array[i]),sla,ch);
		if(ch!=NULL)
		{
			char *ch1=ch;
			while(*ch1==' ')
				ch1++;
			if(strcmpw(obr,ch1,strlen(obr)) || db->Check_Del(ind_array[i]))
			{
				if(nnum_fields<2)
				{
					free(ch);
					obr[strlen(obr)-1]=0;
					if(ind_array!=NULL)
						free(ind_array);
					goto BEGIN;
				}
				bcopy(ind_array+1+i,ind_array+i,(--nnum_fields-i)*sizeof (long));
			}
			db->Get_Slot(Record(ind_array[nnum_fields-1]),sla,ch);
			if(ch!=NULL)
			{
				ch1=ch;
				while(*ch1==' ')
					ch1++;
				if(strcmpw(obr,ch1,strlen(obr)))
				{
					nnum_fields--;
				}
				free(ch);
			}
			else
				nnum_fields--;
		}
		if(nnum_fields<=0)
		{
			obr[strlen(obr)-1]=0;
			if(ind_array!=NULL)
				free(ind_array);
			goto BEGIN;
		}
	}
	if((columns=get_columns(nnum_fields,(term->l_x()-4)/(max_len+1)))==0)
		columns=1;
	lines = (nnum_fields+columns-1)/columns;

	tgs=(struct tag *)calloc((nnum_fields+1),sizeof (struct tag));
	for(int i=0;i<nnum_fields;i++)
	{
		tgs[i].des.x=(i/lines)*(max_len+1);
		tgs[i].des.y=i%lines;
		tgs[i].des.l=max_len;
		tgs[i].des.atr=0;
		tgs[i].index=ind_array[i];
		bcopy(sla,tgs[i].des.sla,SLA_DEEP*sizeof (struct sla));
	}
	free(ind_array);

	int xx=(term->l_x()-columns*(max_len+1))/2;
	int yy=(term->l_y()-lines)/2;

	create_geom(xx,yy,columns*(max_len+1),lines+2);
	restore_bg();
	create_Manual(nnum_fields,tgs,0,NULL);
	Form_Restruct();
	act_field=Find_Active();
	Cadr_Read();
	form_cond|=MAP;
	f_mark_field=&mark_field;
	f_num_mark_fields=&num_mark_fields;
	form_update();
}


int comp_d(const void *a1,const void *a2)
{
	struct tag *a=(struct tag *)a1;
	struct tag *b=(struct tag *)a2;

	if(a==NULL && b==NULL)
		return(0);
	if(a==NULL)
		return(1);
	if(b==NULL)
		return(-1);
	return((a->des.y>b->des.y)?1:(a->des.y<b->des.y)?-1:(a->des.x>b->des.x)?1:(a->des.x<b->des.x)?-1:0);
}


void CX_BROWSER::Form_Restruct()
{
	int y1,i,j,y,len=0;
	int begin,s,first=-1,last;
	long *ind_array=NULL;
	int h_size=h;
	int text=0;

	f_mark_field=&mark_field;
	f_num_mark_fields=&num_mark_fields;

	if(num_fields<1)
		return;

	for(j=0;j<num_fields;j++)
	{
		if(tags[j].str!=NULL)
			free(tags[j].str);
		tags[j].str=NULL;

		if(form_cond&MANUAL)
		{
			if(num_panel>0 && tags[j].des.atr&TABL && panel[num_panel-1].y!=tags[j].des.y)
			{
				bcopy(tags+j+1,tags+j,(num_fields-j-1)*sizeof (struct tag));
				j--;
				num_fields--;
			}
		}

	}

	if(record<=0 || record>db->last_cadr())
		record=db->last_cadr();

	if((index=find_page(record))<0)
	{
		index=1;
		record=Record(1);
	}

	if((db->max_record=db->last_cadr())==0 || form_cond&NEW)
	{
		index=0;
		record=0;
	}

	first_line=0;
	Find_Place();
	set_form();
	for(y1=-1,j=0;j<num_panel;j++)
	{
		if(panel[j].atr & TABL)
		{
			y1=panel[j].y;
			break;
		}
	}

	form_cond&=~TABLE;
	form_cond&=~ARRAY;

	if(y1<0)
	{
		goto END;
	}
	for(begin=0,y=0;background[begin] && y!=y1;begin++)
	{
		if(background[begin]=='\n')
			y++;
	}

	if(!(y==0 && y1!=0))
	{
		for(len=0;background[begin+len] && background[begin+len]!='\n';len++);
		if(background[begin+len]!=0)
			len++;

		for(j=begin+len;background[j];j++)
		{
			if(background[j]=='\n')
			{
				y++;
			}
		}
		if(j>0 && background[j-1]!='\n')
		{
			y++;
		}
	}

	for(j=0;j<num_fields;j++)
	{
		if(tags[j].des.y+tags[j].des.h>y)
		{
			y=tags[j].des.y+tags[j].des.h;
		}
	}
	for(j=0;j<num_panel;j++)
		if(panel[j].y+panel[j].h>y)
			y=panel[j].y+panel[j].h;

	s=size;

	last=0;
	for(j=0,num_colon=0;j<num_fields;j++)
	{
		if(tags[j].des.l && tags[j].des.y==y1)
		{
			tags[j].des.atr|=TABL;
			num_colon++;
			last=j;
			if(first<0)
				first=j;
		}
	}
	num_lines=h_size-y-2;

	while(num_lines<=0)
		num_lines+=h_size;      // ???

	for(i=0,j=first;j<=last;j++)
	{
		if(tags[j].des.sla->n>db->Num_Fields())
		{
			i++;
			continue;
		}
		if(tags[j].des.sla->m || db->Field_Descr(tags[j].des.sla->n)->m==0)
		{
			break;
		}
	}
	if(first!=last && j==last && j!=i)     // check for virtual
	{
		for(j=first;j<=last;j++)
		{
			if(tags[j].des.sla->n>db->Num_Fields())
			{
				if(tags[j].des.sla->m || db->Num_Elem_Array(Record(index),tags[j].des.sla)==0)
				{
					break;
				}
			}
		}
	}

	if(i==j && j!=0 && tags[first].des.sla->n>db->Num_Fields())
	{
		struct sla sla[SLA_DEEP];

		bcopy(tags[first].des.sla,sla,sizeof sla);
		sla->m=-1;

		if(db->Num_Elem_Array(Record(index),sla)>0)
		{
			if((j=Sub_Index(index,num_lines,ind_array))<=0)
			{
				return; //A.L.
			}
		}
	}
	if((i!=j && j>last) || (text=(first==last && db->Field_Descr(tags[first].des.sla->n)->a==X_TEXT)))
	{
		form_cond|=ARRAY;
		for(j=first;l<=last;j++)
			tags[j].des.atr&=~TABL;
		struct sla sla[SLA_DEEP];

		bcopy(tags[first].des.sla,sla,sizeof sla);
		sla->m=-1;
		j=(read_only==0 && text==0)+db->Num_Elem_Array(Record(index),sla);
		if(j<1)
			j=1;
		if(j>num_lines)
			j=num_lines;
	}
	else
	{
		if((j=Sub_Index(index,num_lines,ind_array))<=0)
		{
			return; //A.L.
		}
	}
	h-=(num_lines-j);
	h_size-=(num_lines-j);
	num_lines=j;
	lines=h_size-2;

	if(!(form_cond&ARRAY))
		form_cond|=TABLE;
	else
		form_cond&=~TABLE;
	if(len>0)
	{
		background=(char *)realloc(background,size+=(num_lines-1)*len+1);
		background[size-1]=0;

		if(s-begin>len)
			bcopy(background+begin+len,background+begin+len+(num_lines-1)*len,s-begin-len);

		for(j=1;j<h_size-y-2;j++)
			bcopy(background+begin,background+begin+(j*len),len);
	}
	for(j=0;j<num_panel;j++)
	{
		if(panel[j].atr&TABL)
			continue;
		if(panel[j].y>y1)
			panel[j].y+=num_lines-1;
		if(panel[j].y+panel[j].h>=y1 && panel[j].y<=y1)
		{
			panel[j].h+=h_size-y-3;
		}
	}

	for(j=0;j<num_fields;j++)
		if(tags[j].des.y>y1)
			tags[j].des.y+=num_lines-1;
	last=num_fields;
	tags=(struct tag *)realloc(tags,(num_fields+=(num_lines-1)*num_colon)*sizeof (struct tag));
	if(num_fields>last)
		bzero(tags+last,(num_fields-last)*sizeof (struct tag));

	for(j=0;j<num_lines-1;j++)
		bcopy(tags+first,tags+last+j*num_colon,num_colon*sizeof (struct tag));

	for(j=0;j<num_lines-1;j++)
	{
		for(i=0;i<num_colon;i++)
		{
			tags[last+j*num_colon+i].des.y+=1+j;
			tags[last+j*num_colon+i].str=NULL;
		}
	}
	qsort(tags,num_fields,sizeof(struct tag),comp_d);

	for(first_line=0;first_line<num_fields && ((tags[first_line].des.atr&TABL)==0);first_line++);
	for(j=0;j<num_lines;j++)
	{
		for(i=0;i<num_colon;i++)
		{
			if(form_cond&ARRAY)
			{
				tags[first_line+j*num_colon+i].des.sla->m=j+1;
				tags[first_line+j*num_colon+i].index=index;
			}
			else
			{
				tags[first_line+j*num_colon+i].index=ind_array[j];
			}
		}
	}
	if(ind_array!=NULL)
		free(ind_array);
END:
	if(form_cond&TABLE)
	{
		act_field=find_pos(0);
		act_field=Find_Active();
		while(!possible(act_field) && act_field<num_fields)
			act_field++;
		while(!possible(act_field) && act_field>0)
			act_field--;
	}
}

extern int refresh_flag;

int CX_BROWSER::Go(int kod)
{
	int i;

	switch(kod)
	{
		case CU:
		case CD:
			if(form_cond&EDIT && form_cond&TABLE)
			{
				if(Write())
					break;
			}
		case '\t':
		case CR:
		case CL:
			protocol(kod);
			if(!num_fields)
				break;
			i=find_pos(kod);
			if(form_cond&ARRAY && tags[act_field].des.atr&TABL && (i<0 || !(tags[i].des.atr&TABL)) && kod!='\t')
			{
				if(Scroll_Array(kod))
					break;
			}
			if(i==-1)
			{
				act_field=Scroll(CD);
				break;
			}
			if(i==-2)
			{
				act_field=Scroll(CU);
				break;
			}
		 /*       if(kod==CD && i==act_field && i<num_fields-1)
				i++;

			if(kod==CU && i==act_field && i>0)
				i--;*/
			act_field=i;
/*
			if(tags[act_field].des.y<line || tags[act_field].des.y>h+line-3)
			{
				line=(h-2)*(tags[act_field].des.y/(h-2));
			}
			if(tags[act_field].des.x<colon || ((tags[act_field].des.x+tags[act_field].des.l/2)>l+colon-2))
			{
				if((colon=tags[act_field].des.x-1)<5)
					colon=0;
			}
*/
//                        act_field=find_pos(0);
			index=tags[act_field].index;
			break;
		case EN:
			protocol(kod);
			if(form_cond&ARRAY && tags[act_field].des.atr&TABL)
			{
				if(Scroll_Array(kod))
					break;
			}
			if(form_cond&TABLE || form_cond&MAP)
			{
				Go_To_Index(db->max_index);
			}
			else
			{
				if(lines<line+h-1)
					break;
				line+=h-1;
				act_field=find_pos(-1);
			}
			break;
		case HM:
			protocol(kod);
			if(form_cond&ARRAY && tags[act_field].des.atr&TABL)
			{
				if(Scroll_Array(kod))
					break;
			}
			if(form_cond&TABLE || form_cond&MAP)
			{
				Go_To_Index(1);
			}
			else
			{
				if(line>=h-1)
					line-=h-1;
				else    line=0;
				act_field=find_pos(1);
			}
			break;
		default:
			return(-1);
	}
	return(0);
}

int CX_BROWSER::Move(int kod)
{
	int i=0;
	int act_field_std;

	for(CX_Show();;CX_Show())
	{
MOVE:
		if(kod==0)
		{
			refresh_flag=1;
			kod=Xmouse(term->dpi());
			refresh_flag=0;
		}


A1:
		act_field_std=act_field;

//передача кодов

		if(kod==CU || kod==CD || kod==CL || kod==CR || kod=='\t')
		{
			char std[sizeof db->share->output];
			if(db->share!=NULL)
			{
				bcopy(db->share->output,std,sizeof db->share->output);

			if((i=Event(kod))!=0)
			{
				if(db->share!=NULL && !(*db->share->output))
					bcopy(std,db->share->output,sizeof db->share->output);
//                                Go_To_Field(db->share->slot,tags[act_field].index);
				Cmd_Exe(i);
				return 0;
			}
			else
			{
				bzero(db->share->output,sizeof db->share->output);
				db->share->cmd=0;
				bzero(&db->share->slot, sizeof (db->share->slot));
			}
			}
		}

//конец передачи
		switch(kod)
		{
		case 0:
		case -3:
			if((i=get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))!=0)
			{
				kod=1000+i;
				goto END;
			}
			if(term->ev().b!=1)
			{
				kod=0;
				goto END;
			}
			if(term->ev().y==y0+h-1)
			{
				if((char)term->get_ch(term->ev().x,term->ev().y)=='<')
				{
					term->flush_mouse();
					if(form_cond&TABLE)
					{
						kod=CU;
						goto MOVE;
					}
					kod=HM;
					goto END;
				}
				if((char)term->get_ch(term->ev().x,term->ev().y)=='>')
				{
					term->flush_mouse();
					if(form_cond&TABLE)
					{
						kod=CD;
						goto MOVE;
					}
					kod=EN;
					goto END;
				}
				i=0;
				while(i<l && (char)term->get_ch(x0+i,term->ev().y)!='>') i++;
				i-=2;

				Go_To_Index(1+((db->max_index*(term->ev().x-x0-1))/i));
				term->flush_mouse();

				break;
			}
			for(i=0;i<num_fields;i++)
			{
				if(term->ev().y==y0+tags[i].des.y-line+1 && term->ev().x>x0+tags[i].des.x-colon && term->ev().x<=x0+tags[i].des.x-colon+tags[i].des.l && pos_able(i))
				{
					if(form_cond&EDIT && form_cond&TABLE && tags[i].des.y!=tags[act_field].des.y)
					{
						int x=term->ev().x;
						int y=term->ev().y;
						if(Write())
							break;
#ifndef WIN32
				       term->evSetX(x);
						term->evSetY(y);
#endif
						goto A1;
					}
					if(i==act_field)
					{
						kod='\r';
						goto A1;
					}

					act_field=i;
					break;
				}
			}
			break;
		default:
			if(Go(kod)<0)
			{
				goto END;
			}
		if(kod==CU || kod==CD || kod==CL || kod==CR || kod=='\t')
			Event(-kod);


			break;
		}
		if(form_cond&TABLE && (kod==CU || kod==CD || kod==0))
		{
			index=tags[act_field].index;
			record=Record(index);
			for(i=0;i<num_fields;i++)
			{
				if(!(tags[i].des.atr&TABL))
				{
					tags[i].index=index;
					form_update(i+1);
				}
			}
		}
		kod=0;
	}
	return(0);
END:
	if(image_x!=-1)
	{
		if(!access(image_name,W_OK))
			unlink(image_name);
		term->Del_Image(image_x,image_y);
		*image_name=0;
		image_x=-1;
		image_y=-1;
	}
	return(kod);
}

void CX_BROWSER::CX_Show()
{
	char str[265];
	static int act_field_std;

	if(cx_cond&HIST)
		frame_color=016;
	else if(form_cond&MARK)
		frame_color=013;
	else    frame_color=017;

	if(db->share!=NULL && db->share->slot.sla->n)
	{
		if(slacmp(db->share->slot.sla,tags[act_field].des.sla))
		{
			Go_To_Field(db->share->slot,tags[act_field].index);
		}
		bzero(db->share->slot.sla,sizeof db->share->slot.sla);
	}

	if(tags[act_field].des.y<line || tags[act_field].des.y>h+line-3)
	{
		if(h<=2)
			line=0;
		else
			line=(h-2)*(tags[act_field].des.y/(h-2));
	}
	if(tags[act_field].des.x<colon || ((tags[act_field].des.x+tags[act_field].des.l/2)>l+colon-2))
	{
		if((colon=tags[act_field].des.x-1)<5)
			colon=0;
	}

	Show();

	if(!(form_atr&NOMENU))
	{
		if(form_cond&MARK)
			help_line(3);
		else
			help_line(1);
	}
	else    help_line(0);
	if(db->share!=NULL && *db->share->output)
	{
		term->BlackWhite(0,0,term->l_x(),term->l_y());
		dial(db->share->output,4,&db->share->color,db->share->font.fnt==-1?-1:((db->share->font.fnt<<4)&0x7)+(db->share->font.fnt&0x3));
		term->BlackWhite(0,0,term->l_x(),term->l_y());
		term->MultiColor(x0,y0,l,h);

		db->share->color.bg=0;
		db->share->color.fg=0;
		db->share->font.fnt=-1;
		db->share->font.atr=0;
		bzero(db->share->output,sizeof (db->share->output));
	}
	if(form_cond&MAP)
	{
		int i=tags[act_field].des.l+1;
		term->Set_Color(0,frame_color);
		while(i<l)
		{
			if(i-colon>0)
				term->vert_s(x0+i-colon,y0,h-1);
			i+=tags[act_field].des.l+1;
		}
	}

	term->Set_Color(010,017);
	term->Set_Font(1,3);

	sprintf(str," %s: %s ",db->Short_Name(),name_form.blank);
	term->dpp(x0+2,y0);
	if((int)strlen(str)<l)
	{
		term->dps(str);
	}
	if(*obr)
	{
		sprintf(str,"[%s]",obr);
		term->Set_Color(010,014);
		term->dps(str);
		term->Set_Color(010,017);
	}
	if(record>0 && l>30+(int)strlen(str))
	{
		char from[LINESIZE],*ch;
		if((ch=message(57))!=NULL)
			strcpy(from,ch);
		else    strcpy(from,"from");
		if(cx_cond&HIST)
			sprintf(str,"History[%d] %s",(int)hist_record,info);
		else if(db->cx_cond&SORT)
			sprintf(str," %s: %d->%d %s: %d(%d) ",message(56),(int)tags[act_field].index,(int)Record(tags[act_field].index),from,(int)db->max_index,db->insert);
		else
			sprintf(str," %s: %d %s: %d ",message(56),(int)(act_field<num_fields?tags[act_field].index:Record(index)),from,(int)db->max_index);
		term->dpp(x0+l-strlen(str)-1,y0);
		term->dps(str);
	}
	term->Set_Font(0,0);
	if(num_mark)
	{
		for(int i=0;i<num_fields;i++)
		{
			if(if_mark(Record(tags[i].index)))
			{
				if(Draw_Slot(i,05+0x200,tags[i].color.fg,tags[i].font))
				{
					if(form_cond&TABLE && tags[i].des.y==tags[act_field].des.y)
						Draw_Slot(i,0x11e,tags[i].color.fg,tags[i].font);
					if(i==act_field)
						Draw_Slot(i,0x10b,15,tags[i].font);
				}
			}
		}
	}
	if(act_field<num_fields && db->Check_Del(Record(tags[act_field].index)))
	{
		if(form_cond&TABLE)
		{
			for(int i=0;i<num_fields;i++)
			{
				if(tags[i].index==tags[act_field].index)
					Draw_Slot(i,24,7,tags[i].font);

			}
		}
		else for(int y=1;y<h-1;y++)
		{
			for(int x=1;x<l-1;x++)
			{
				term->put_fg(x+x0,y+y0,7);
				term->put_bg(x+x0,y+y0,24);
			}
		}
	}
	*str=0;
	term->Set_Font(1,3);
	if(lines>h-2)
	{
		int i;

		term->dpp(x0+l-1,y0+1+(((h-2)*(tags[act_field].des.y))/lines));
		term->Set_Color(0,7);
		term->dpo('■');
		term->Set_Color(010,017);
		sprintf(str," Page: %d(%d)",1+line/(h-2),1+lines/(h-2));
		if((i=x0+l-2-strlen(str))>0 && (int)strlen(str)<l-2)
		{
			term->dpp(i,y0+h-1);
			term->dps(str);
		}
		else *str=0;
	}
	term->Set_Font(0,0);
	if(db->max_index>1 && !(form_cond&MAP) && !(form_cond&NEW))
	{
		int i;

		term->dpp(x0+1,y0+h-1);
		term->Set_Color(0x10f,0);
		term->dpo('<');
		i=l-3-strlen(str);
		term->Set_Color(0,15);
		term->dpn(i-2,'─');
		term->Set_Color(0x10f,0);
		term->dpo('>');

		i=((i-3)*(tags[act_field].index-1))/(db->max_index-1);
		term->dpp(x0+2+i,y0+h-1);
		term->Set_Color(0,7);
		term->dpo('█');
	}
	if(keep_str!=NULL)
	{
		term->Set_Color(0,15);
		term->dpp(x0,y0+1);
		term->dpo('*');
	}
	if(act_field>=num_fields || tags[act_field].des.x>=l || tags[act_field].des.y-line>=h-2 || tags[act_field].des.y<line)
		return;
	if(tags[act_field].des.x<colon || tags[act_field].des.x-colon>=l || tags[act_field].des.y-line>=h-2 || tags[act_field].des.y<line)
		return;
	if(tags[act_field].des.atr&IS_IMAGE && !access(tags[act_field].str,R_OK))
	{
		char *n,sl[64];
		if(x0+tags[act_field].des.x-colon+1==image_x && y0+tags[act_field].des.y-line+1==image_y)
			goto END;
		if(image_x!=-1)
			term->Del_Image(image_x,image_y);
		sla_to_str(tags[act_field].des.sla,sl);
		if((n=strrchr(db->Name_Base(),'/'))==NULL)
			n=db->Name_Base();
		else    n++;
		if(*image_name && !access(image_name,W_OK))
			unlink(image_name);
		sprintf(image_name,"/var/www/docs/Images/Tmp/%s.%ld.%s",n,Record(index),sl);
		fcopy(image_name,tags[act_field].str);
		term->Show_Image(image_x=x0+tags[act_field].des.x-colon+1,image_y=y0+tags[act_field].des.y-line+1,image_name+14,NULL);
		act_field_std=act_field;
	}
	else if(image_x!=-1 && tags[act_field_std].des.atr&IS_IMAGE)
	{
		if(!access(image_name,W_OK))
			unlink(image_name);
		term->Del_Image(image_x,image_y);
		*image_name=0;
		image_x=-1;
		image_y=-1;
	}
END:
	term->dpp(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1);
}
int CX_BROWSER::Scroll(int kod)
{
	int i,first=-1,last=-1;

	if(form_cond&ARRAY)
		return(act_field);
	for(i=0;i<num_fields;i++)
		if(tags[i].des.atr&TABL)
		{
			if(first<0)
				first=i;
			last=i;
		}
	if(first<0 || last<0)
		return(act_field);

	if(kod==CD)
	{
		int ind;

		if(act_field>=first+num_colon*num_lines || tags[act_field].index==0)
		{
			return(act_field);
		}

		ind=Next_Index(tags[act_field].index);
		if(ind==tags[act_field].index)
			return(act_field);
		for(i=0;i<num_colon;i++)
		{
			int j=first+i;
			if(j>=num_fields)
				break;
			if(tags[j].str!=NULL)
				free(tags[j].str);
			tags[j].str=NULL;
		}
		for(i=0;i<num_colon*(num_lines-1);i++)
		{
			int j=first+i;
			if(j+num_colon>=num_fields)
				break;
			tags[j].index=tags[j+num_colon].index;
			tags[j].str  =tags[j+num_colon].str;
			tags[j].color=tags[j+num_colon].color;
		}
		term->Scroll_Up(tags[first].des.x-colon+1,y0+tags[first].des.y-line+1,tags[last].des.x-tags[first].des.x+tags[last].des.l,tags[last].des.y-tags[first].des.y+1,1);
		for(i=0;i<num_colon;i++)
		{
			int j=first+i+num_colon*(num_lines-1);
			if(j>=num_fields)
				break;
			tags[j].index=ind;
			tags[j].str=NULL;
			form_update(j+1);
		}
	}
	if(kod==CU)
	{
		int ind;

		if(tags[first].index==1 || tags[act_field].index==0)
			return(act_field);

		ind=Prev_Index(tags[act_field].index);
		if(ind==tags[act_field].index)
			return(act_field);

		for(i=0;i<num_colon;i++)
		{
			int j=first+i+num_colon*(num_lines-1);
			if(j>=num_fields)
				break;
			if(tags[j].str!=NULL)
				free(tags[j].str);
			tags[j].str=NULL;
		}
		for(i=num_colon*(num_lines-1);i>=0;i--)
		{
			int j=first+i;
			if(j+num_colon>=num_fields)
				continue;
			tags[j+num_colon].index=tags[j].index;
			tags[j+num_colon].str  =tags[j].str;
			tags[j+num_colon].color=tags[j].color;
		}
		term->Scroll_Down(tags[first].des.x-colon+1,y0+tags[first].des.y-line+1,tags[last].des.x-tags[first].des.x+tags[last].des.l,tags[last].des.y-tags[first].des.y+1,1);
		for(i=0;i<num_colon;i++)
		{
			int j=first+i;
			if(j>=num_fields)
				break;
			tags[j].index=ind;
			tags[j].str=NULL;
			form_update(j+1);
		}

	}
	index=tags[act_field].index;
	record=Record(index);
	return(act_field);
}

int CX_BROWSER::Scroll_Array(int kod,int dp)
{
	int i,first=-1,last=-1;

	for(i=0;i<num_fields;i++)
		if(tags[i].des.atr&TABL)
		{
			if(first<0)
				first=i;
			last=i;
		}
	if(first<0 || last<0)
		return(0);
	int num=tags[last].des.sla->m-tags[first].des.sla->m;

	struct sla sla[SLA_DEEP];
	bcopy(tags[first].des.sla,sla,sizeof sla);
	sla->m=-1;

	if(kod==HM)
	{
		if(tags[first].des.sla->m==1)
			return(0);
		if(tags[first].des.sla->m-num<1)
			num=tags[first].des.sla->m-1;
		for(i=first;i<=last;i++)
			tags[i].des.sla->m-=num;
		form_update();
		return(1);
	}
	if(kod==EN)
	{
		if(tags[last].des.sla->m>=db->Num_Elem_Array(Record(index),sla))
			return(0);
		for(i=0;i<num;i++)
			Scroll_Array(CD,0);
		return(1);
	}
	if(kod==CU)
	{
		if(tags[act_field].des.sla->m==1)
			return(0);
		for(i=first;i<=last;i++)
			tags[i].des.sla->m--;
		if(dp)
			term->Scroll_Down(tags[first].des.x-colon+1,y0+tags[first].des.y-line+1,tags[last].des.x-tags[first].des.x+tags[last].des.l,tags[last].des.y-tags[first].des.y,1);

		form_update();
		return(1);
	}
	if(kod==CD)
	{
		i=(read_only==0)+db->Num_Elem_Array(Record(index),sla);
		if(tags[act_field].des.sla->m==i)
			return(0);
		for(i=first;i<=last;i++)
			tags[i].des.sla->m++;
		if(dp)
			term->Scroll_Up(tags[first].des.x-colon+1,y0+tags[first].des.y-line+1,tags[last].des.x-tags[first].des.x+tags[last].des.l,tags[last].des.y-tags[first].des.y,1);
		form_update();
		return(1);
	}
	return(0);
}

void CX_BROWSER::form_update(int field)
{
	if(field<1 || field>num_fields)
		return;
	if(tags[field-1].des.sla->n==-2)
		db->Str_To_Sla(tags[field-1].name,tags[field-1].des.sla);
	if(tags[field-1].des.sla->n<=0)
	{
		char str[32],fmt[32];

		sprintf(fmt,"%%%dd",(int)tags[field-1].des.l);
		sprintf(str,fmt,tags[field-1].index);
		tags[field-1].str=(char *)realloc(tags[field-1].str,strlen(str)+1);
		strcpy(tags[field-1].str,str);
		tags[field-1].des.atr|=NO_POS;
	}
	else
	{
		if(db->share!=NULL)
			db->share->font.fnt=-1;
		Get_Slot(Record(tags[field-1].index),tags+field-1,tags[field-1].str);

		if(db->Field_Descr(tags[field-1].des.sla->n)->a==X_TEXT && tags[field-1].des.sla[1].n)
		{
			char *ch=tags[field-1].str;
			char *buf=ch;

			for(int i=0;i<tags[field-1].des.sla[1].n;i++)
			{
				if(i)
					buf=ch+1;
				if((ch=strchr(buf,'\n'))==NULL)
					break;
			}
			if(ch!=NULL)
			{
				int len=ch-buf;
				if(len>term->l_x())
					len=term->l_x();
				char *tmp=(char *)malloc(len+2);
				bcopy(buf,tmp,len+1);
				tmp[len+1]=0;
				free(tags[field-1].str);
				tags[field-1].str=tmp;
			}
		}
		if(db->Field_Descr(tags[field-1].des.sla->n)->a==X_VARIANT && tags[field-1].des.sla[1].n==1)
		{
			char *ch=strrchr(tags[field-1].str,'/');
			if(ch!=NULL)
				bcopy(ch+1,tags[field-1].str,strlen(ch));
		}
		if(tags[field-1].des.sla->n>db->Num_Fields() && db->share!=NULL)
		{
			if(db->share->font.fnt!=-1)
			{
				tags[field-1].font=db->share->font;
			}
			if(db->share->color.bg || db->share->color.fg)
			{
				tags[field-1].color.bg=db->share->color.bg;
				tags[field-1].color.fg=db->share->color.fg;
				if(form_cond&BW)
				{
					tags[field-1].color.fg=0;
					if(tags[field-1].color.bg!=010)
						tags[field-1].color.bg=017;
					else
						tags[field-1].color.bg=7;
				}
			}
		}
	}
}

void CX_BROWSER::total(long max)
{
	int f=-1,x,y,n=0;

	db->update();
	refresh_flag=1;
	if(!(form_cond&EDIT))
		Cadr_Read();
	if(max==0)
		max=db->max_index;
	if(max>50)
	{
		x=(term->l_x()-50)/2;
		y=(term->l_y()/2);
		f=term->get_box(x-2,y-2,50+6,5);
		term->BOX(x-1,y-1,52,3,' ',6,0xf,6,0xf);
		term->Set_Color(03,0);
		term->dpp(x,y);
		term->flush();
	}
	for(int i=0;i<num_fields;i++)
	{
		if(tags[i].str!=NULL)
		{
			free(tags[i].str);
			tags[i].str=NULL;
		}
	}
	for(long ind=1;ind<=max;ind++)
	{
		if(f>=0 && max>50 && (ind%(max/50))==0 && n<50)
		{
			term->dpo(' ');
			n++;
			term->flush();
		}
		if(!(form_cond&SHOWDEL) && db->Check_Del(Record(ind)))
			continue;
		for(int i=0;i<num_fields;i++)
		{
			char str[64],*ch=NULL;
			int pr;
			char arg[SLA_DEEP];

			Get_Slot(Record(ind),tags+i,ch);
			if(ch==NULL)
				continue;
			if(string_digit(ch,db->int_delimiter))
			{
				double a=db->atof(ch),b=0;
				char *prec=strrchr(ch,'.');
				if(prec!=NULL)
					pr=strlen(ch)-(prec-ch);
				else pr=0;
				sprintf(arg,"%%.%df",pr);
				free(ch);
				if(tags[i].str!=NULL)
					b=db->atof(tags[i].str);
				sprintf(str,arg,a+b);
				tags[i].str=(char *)realloc(tags[i].str,strlen(str)+1);
				strcpy(tags[i].str,str);
			}
			else
			{
				tags[i].str=(char *)realloc(tags[i].str,strlen(ch)+1);
				strcpy(tags[i].str,ch);
				free(ch);
			}
		}
	}
	if(f>=0)
	{
		term->restore_box(f);
		term->free_box(f);
		f=-1;
	}
}

void CX_BROWSER::form_update()
{
	db->update();
	if(num_fields<1)
		return;
	if(!(form_cond&EDIT))
		Cadr_Read();
	for(int i=1;i<=num_fields;i++)
		form_update(i);
	if(!(form_cond&TABLE) && db->Field_Descr(tags[act_field].des.sla)->a==X_IMAGE)
	{
		char *ch=NULL;
		int len=Get_Slot(Record(index),tags+act_field,ch);
		if(ch!=NULL && len!=0)
		{
			char *n,sl[64];

			sla_to_str(tags[act_field].des.sla,sl);
			if((n=strrchr(db->Name_Base(),'/'))==NULL)
				n=db->Name_Base();
			else    n++;
			if(*image_name && !access(image_name,W_OK))
				unlink(image_name);
			sprintf(image_name,"/var/www/docs/Images/Tmp/%s.%ld.%s",n,Record(index),sl);
			int fd=creat(image_name,0600);
			write(fd,ch,len);
			close(fd);
			free(ch);
			term->Show_Image(image_x=x0+tags[act_field].des.x-colon+1,image_y=y0+tags[act_field].des.y-line+1,image_name+14,NULL);
		}
	}
	else if(*image_name)
	{
		if(!access(image_name,W_OK))
			unlink(image_name);
		term->Del_Image(image_x,image_y);
		*image_name=0;
		image_x=-1;
		image_y=-1;
	}
}


void CX_BROWSER::set_form()
{
	act_field=find_pos(0);
	if(!num_fields)
	{
		num_fields=db->Num_Fields();
		if(tags!=NULL)
			free(tags);
		tags=(struct tag *)calloc(num_fields,sizeof (struct tag));
		for(int i=0;i<num_fields;i++)
		{
			tags[i].color=Type_Color(db,db->Field_Descr(i+1)->a);
			if(form_cond&BW)
			{
				tags[i].color.fg=0;
				if(tags[i].color.bg!=010)
					tags[i].color.bg=017;
				else
					tags[i].color.bg=7;
			}
			tags[i].des.x=2;
			tags[i].des.x+=(tags[i].des.x%20);
			tags[i].des.y=i;
			tags[i].des.l=term->l_x()-tags[i].des.x;
			tags[i].des.sla[0].n=i+1;
			tags[i].index=index;
		}
		if(background!=NULL)
			free(background);
		background=(char *)calloc(1,1);
		return;
	}
	for(int i=0;i<num_fields;i++)
	{
		int j=db->Field_Descr(tags[i].des.sla)->a;
		if(db->is_pointer(tags[i].des.sla))
			j=X_POINTER;

		tags[i].color=Type_Color(db,j);
		if(form_cond&BW)
		{
			tags[i].color.fg=0;
			if(tags[i].color.bg&0xf!=010)
				tags[i].color.bg=017;
			else
				tags[i].color.bg=7;
		}
	}
}

void CX_BROWSER::New_Index()
{
	db->Flush_Index_Buf(level);
	if(form_cond&MAP)
		return;
	for(int i=0;i<num_fields;i++)
	{
		if(tags[i].des.atr&TABL)
			continue;
		if(tags[i].des.atr&REL)
			tags[i].index=index+tags[i].shift;
		else
			tags[i].index=index;
	}
}

void hist_log(long record,char *name);

int CX_BROWSER::Write()
{
	char str[128];
	int i;

BEGIN:
	if(db==NULL)
		return(0);
	int cur_rec=db->current_record();

	if(cx_cond&HIST)
		return(0);
	if(!(form_cond&EDIT))
	{
		if(form_cond&NEW)
		{
			form_cond&=~NEW;
			Go_To_Index(db->max_index);
			goto END;
		}
		return(0);
	}
	db->update();
	if(!db->Cadr_Change() && db->share->edit_flag==0)
	{
		if(form_cond&NEW)
		{
			form_cond&=~NEW;
			Go_To_Index(db->max_index);
		}
		goto END;
	}
	for(i=0;i<num_fields;i++)
	{
		if(tags[i].index!=tags[act_field].index)
			continue;
		term->Set_Color(014|0x100,016);
		if(db->Field_Change(tags[i].des.sla))
			Draw_Slot(i,014,016,tags[i].font);
	}
	if(form_cond&NEW && parent_record>0 && !strcmp(db->Name_Base(),PROPERTY))
	{
		char str[64];
		sprintf(str,"#%ld",parent_record);
		db->Put_Slot(record,1,str);
	}
	if(!(form_atr&NOASK))
	{
		term->Set_Color(0,15);
		term->dpp(0,term->l_y());
		term->clean_line();
		if(cur_rec==0)
			sprintf(str,"%s",message(32));
		else
			sprintf(str,"%s%d?",message(33),cur_rec);
		term->dps(str);
		if(!yes(1))
		{
			db->Roll_Back();
			db->Unlock(record);
			db->Cadr_Read(cur_rec);
			form_update();
			CX_Show();
			form_cond&=~NEW;
			form_cond&=~EDIT;
			return(0);
		}
	}
	i=Check_Cadr();
	if(i)
	{
		form_update();
		CX_Show();
		if(*db->share->output)
			dial(db->share->output,4,&db->share->color,db->share->font.fnt==-1?-1:((db->share->font.fnt<<4)&0x7)+(db->share->font.fnt&0x3));
		return(i);

		goto BEGIN;
	}
	form_cond&=~EDIT;
	record=db->Cadr_Write();

	char name[256];
	sprintf(name,"%s/_HistDB",db->Name_Base());
	if(!access(name,W_OK))
		hist_log(record,name);

	if(ed!=NULL)
		ed->Write(record);
	db->max_record=db->last_cadr();
	if(cur_rec==0)
	{
		if(db->cx_cond&SORT || personal_selection!=NULL)
		{
			if(personal_selection!=NULL && !(db->cx_cond&SORT))
				put_Record(1,record);
			else
				put_Record(db->max_index+1,record);
		}
		db->Flush_Index_Buf(level);
		index=find_page(record);
		if(index<0)
		{
			form_cond|=NEW;
			index=0;
		}
		else for(i=0;i<num_fields;i++)
		{
			if(tags[i].des.atr&TABL)
				continue;
			tags[i].index=index;
		}
		Find_Active();
		Cadr_Read();
		Go_To_Index(index);
	}
	else
	{
		form_cond&=~NEW;
	}
END:
	db->Unlock(record);
	form_cond&=~EDIT;
	if(ed!=NULL)
		ed->Reset();
	db->update();
//        Table_Normalize();
	form_update();
	CX_Show();
	return(0);
}

int CX_BROWSER::Change_Form()
{
	if(bg_orig!=NULL)
	{
		free(bg_orig);
		bg_orig=NULL;
	}
	if(*db->share->output)
	{
		index=tags[act_field].index;
		record=Record(index);
		act_field=0;
		Del_Label();
		Free_Form();

		create_Form(db->Name_Base(),db->share->output);
		create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);

		restore_bg();
		bzero(db->share->output,sizeof (db->share->output));
		Form_Restruct();

		New_Index();
		Go_To_Index(index);
		f_mark_field=&mark_field;
		f_num_mark_fields=&num_mark_fields;

		bg_orig=(char *)malloc(bg_size=size);
		memcpy(bg_orig,background,size);

		return(0);

	}
	struct x_form form;
	bzero(&form,sizeof form);
	if(Select_Form(db->Name_Base(),&form)<0)
		return(0);
	index=tags[act_field].index;
	record=Record(index);
	act_field=0;
	Del_Label();
	Free_Form();
	create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
	restore_bg();
	Get_Form(db->Name_Base(),&form);
	Form_Restruct();

	bg_orig=(char *)malloc(bg_size=size);
	memcpy(bg_orig,background,size);

	New_Index();
	Go_To_Index(index);
	return(0);
}


long CX_BROWSER::Next_Index(long ind)
{
	long index_std=ind;

	if(ind>=db->max_index)
		return(ind);
	while(++ind<=db->max_index)
	{
		if(!(form_cond&SHOWDEL) && db->Check_Del(Record(ind)))
		{
			if(db->cx_cond&SORT)
				del_Record(ind);
			continue;
		}
		if(form_cond&MAP && *obr)
		{
			char *ch=NULL;
			Get_Slot(Record(ind),tags+act_field,ch);
			if(ch!=NULL)
			{
				char *ch1=ch;
				while(*ch1==' ')
					ch1++;
				if(strcmpw(ch1,obr,strlen(obr)))
				{
					free(ch);
					continue;
				}
				free(ch);
			}
		}
		break;
	}
	if(ind>db->max_index)
	{
		return(index_std);
	}
	return(ind);
}

long CX_BROWSER::Prev_Index(long ind)
{
	long index_std=ind;

	if(ind<=1)
		return(ind);
	while(--ind>0)
	{
		if(!(form_cond&SHOWDEL) && db->Check_Del(Record(ind)))
		{
			if(db->cx_cond&SORT)
				del_Record(ind);
			continue;
		}
		if(form_cond&MAP && *obr)
		{
			char *ch=NULL;

			Get_Slot(Record(ind),tags+act_field,ch);
			if(strcmpw(ch,obr,strlen(obr)))
			{
				free(ch);
				continue;
			}
			free(ch);
		}
		break;
	}
	if(ind<=0)
		return(index_std);
	return(ind);
}

void CX_BROWSER::Find_Place()
{
	long ind=index;

	if((form_cond&SHOWDEL) || !db->Check_Del(Record(index)))
		return;
	index=Next_Index(index);
	if(ind==index)
	{
		index=Prev_Index(index);
	}
	record=Record(index);
}

long CX_BROWSER::Sub_Index(int ind,int num_records,long *&ind_array)
{
	long i,m=0;

	if(num_records<1)
	{
		return(0);
	}
	if(form_cond&NEW || ind>db->max_index)
		ind=db->max_index;
	ind_array=(long *)calloc(num_records,sizeof (long));

	if(cx_cond&PUTD)
	{
		for(i=0;i<num_records;i++)
			ind_array[i]=ind+i;
		return(i);
	}
	ind_array[i=num_records/2]=ind;
	for(;i>0;i--)
	{
		long j=Prev_Index(ind_array[i]);
		if(j==ind_array[i])
			break;
		ind_array[i-1]=j;
	}
	if(!i)
		m=1;
	bcopy(ind_array+i,ind_array,(num_records/2)*sizeof (long));
	for(i=(num_records/2-i)+1;i<num_records;i++)
	{
		long j=Next_Index(ind_array[i-1]);
		if(j==ind_array[i-1])
			break;
		ind_array[i]=j;
	}
	if(num_records>i)
		bzero(ind_array+i,num_records-i);

	if(m && i<num_records)
	{
		long j;

		m=num_records-i;
		bcopy(ind_array,ind_array+m,i*sizeof (long));
		for(j=m;j>0;j--)
		{
			ind_array[j-1]=Prev_Index(ind_array[j]);
			if(ind_array[j-1]==ind_array[j])
				break;
		}
		bcopy(ind_array+j,ind_array,(num_records-j)*sizeof (long));
		i=num_records-j;
	}

	if(form_cond&NEW)
	{
		if(i==num_records || db->last_cadr()==0)
		{
			bcopy(ind_array+1,ind_array,(num_records-1)*sizeof (long));
			ind_array[num_records-1]=0;
		}
		else
		{
			ind_array=(long *)realloc(ind_array,(++i)*sizeof (long));
			ind_array[i-1]=0;
		}
	}
	if(!(form_cond&SHOWDEL))
	{
		for(int j=0;j<i;j++)
		{
			if(db->Check_Del(Record(ind_array[j])))
			{
				bcopy(ind_array+j+1,ind_array+j,(i-j)*sizeof (long));
				i--;
			}
		}
		if(db->Check_Del(Record(ind_array[i-1])))
		{
			i--;
			ind_array[i]=0;
		}
	}
	return(i);
}
int CX_BROWSER::Find_Active()
{
	if(form_cond&TABLE)
	{
		if(tags[act_field].index==index && tags[act_field].des.atr&TABL)
			return(act_field);
		for(int i=0;i<num_fields;i++)
			if(tags[i].index==index && tags[i].des.atr&TABL)
				return(i);
	}
	else
	{
		if(tags[act_field].index==index)
			return(act_field);
		for(int i=0;i<num_fields;i++)
			if(tags[i].index==index)
				return(i);
	}
	return(act_field);
}

long CX_BROWSER::Act_Record()
{
	return(Record(tags[act_field].index));
}

void CX_BROWSER::Table_Normalize()
{
	if(!(form_cond&TABLE))
		return;

	long *ind_array;
	int shift;
	if(num_colon)
		shift=(act_field-first_line)%num_colon;
	else    shift=0;
	int j=Sub_Index(index,num_lines,ind_array);

	for(j=0;j<num_lines;j++)
		for(int i=0;i<num_colon;i++)
			tags[first_line+j*num_colon+i].index=ind_array[j];
	free(ind_array);
	if(act_field!=Find_Active())
		act_field=Find_Active()+shift;
}

void CX_BROWSER::Go_To_Index(long ind)
{
	if(ind<0 || ind>db->max_index)
		ind=db->max_index;
	if(form_cond&MAP)
	{
		struct sla sla[SLA_DEEP],*sl;
		int len=tags[0].des.l;

		bcopy(tags[0].des.sla,sla,sizeof sla);
		sl=sla;
		index=ind;
		record=Record(index);
		Create_Map(sl,len);
		return;
	}
	struct tag_descriptor td;
	bzero(&td,sizeof td);
	if(act_field>=0 && act_field<num_fields)
		bcopy(&tags[act_field].des,&td,sizeof td);
	if(Write())
		return;
	index=ind;
	record=Record(index);
	Form_Refresh(&td);
	New_Index();
	Cadr_Read();
	form_update();
}

void CX_BROWSER::zero()
{
	inherit=0;
	record=0;
	index=0;
	shmid=0;
	bzero(obr,sizeof obr);
	mark=NULL;
	num_mark=0;
	level=0;
	stack=NULL;
	num_stack_sel=0;
	hyperform=0;
	prot_fd=0;
	read_only=0;
	hist_record=0;
	ed=NULL;
	mark_field=NULL;
	num_mark_fields=0;
	keep_str=NULL;
	restrict=NULL;
	cx3=NULL;
	parent_record=0;
}

void CX_BROWSER::initialize(CX_BROWSER *STD,long REC)
{
	zero();

	inherit=1;
	db          = STD->db;
	stack       = STD->stack;
	num_stack_sel=STD->num_stack_sel;
	prot_fd     = STD->prot_fd;
	keep_str    = STD->keep_str;
	read_only   = STD->read_only;
	ed          = STD->ed;
	shmid       = STD->shmid;

	cx_cond     = STD->cx_cond;
	record      = REC;
	bg_orig     = STD->bg_orig;

	if (STD->restrict!=NULL && *STD->restrict)
	{
		restrict    = (char *)malloc(strlen(STD->restrict)+1);
		strcpy(restrict,STD->restrict);
	}
	else
		restrict    = NULL;

	cx3         = STD->cx3;

	if (STD->personal_selection!=NULL&&*STD->personal_selection)
	{
		personal_selection = (char *)malloc(strlen(STD->personal_selection)+1);
		strcpy(personal_selection,STD->personal_selection);
	}
	else
		personal_selection = NULL;

	index=find_page(record);

	Form_Restruct();
	db->Cadr_Read(Record(index));
	Go_To_Index(index);

	form_update();
	load_menu();
}

void CX_BROWSER::initialize(char *name_base,long REC,int atr,struct x_form *f,char *ch)
{
	if(name_base!=NULL)
	{
		try {
			db=new CX_BASE(name_base);
		}
		catch(...) {
			throw 10;
		}
	}
	if(ch!=NULL && db->share!=NULL)
		strcpy(db->share->io,ch);
	inherit=0;
	cx_cond=0;
	form_cond=0;
	ed=NULL;
	mark=NULL;
	num_mark=0;
	mark_field=NULL;
	num_mark_fields=0;
	stack=NULL;
	num_stack_sel=0;
	personal_selection=NULL;
	bzero(obr,sizeof obr);
	prot_fd=0;
	keep_str=NULL;
	restrict=NULL;
	read_only=db->writable()!=0;
	hyperform=0;
	parent_record=0;
	flag_cxerror = 0;
	int ret;
	char output[LINESIZE];
	char *name;

	if(f!=NULL && db->share!=NULL)
	{
		bcopy(f,&db->share->form,sizeof db->share->form);
	}

	if((ret=Event(0))==c_Exit)
	{
		if(*db->share->output)
			dial(db->share->output,4,&db->share->color,db->share->font.fnt==-1?-1:((db->share->font.fnt<<4)&0x7)+(db->share->font.fnt&0x3));
		delete db;
		flag_cxerror = 77;
		return ;
	}
	if(ret==c_GetSel)
	{
		strcpy(output,db->share->output);
	}
	if(ret==c_GetLimitSel)
	{
		if(!access(db->share->output,R_OK))
		{
			personal_selection=(char *)realloc(personal_selection,strlen(db->share->output)+1);
			strcpy(personal_selection,db->share->output);
			Read_Index(db->share->output);
			*db->share->output=0;
		}
		else
			goto LIM;
	}
	else
	{
LIM:
		name=(char *)malloc(strlen(db->Name_Base())+strlen(LIMITDIR)+strlen(GetLogin())+4);
		sprintf(name,"%s/%s/%s",db->Name_Base(),LIMITDIR,GetLogin());
		if(!access(name,R_OK))
		{
			personal_selection = (char *)realloc(personal_selection,strlen(name)+1);
			strcpy(personal_selection,name);
			Read_Index(name);
		}
		else
			free(name);
	}
	cx3=NULL;
#ifndef WIN32
	if(db->context->pswd==CXKEY3)
	{
		try
		{
			cx3 = new CX3(this);
		}
		catch(...)
		{
			cx3=NULL;
		}
	}
#endif
	if(!REC || REC>db->last_cadr())
		record=db->last_cadr();
	else    record=REC;
	if(!record || record>db->last_cadr() || db->Check_Del(record))
	{
		record=0;
	}
	if(atr==0)
	{
		index=find_page(record);
		Set_Index();
	}
	hyperform=0;
	for(int i=1;i<=db->Num_Fields();i++)
	{
		if(db->context->pswd==CXKEY3 && db->Field_Descr(i)->k)
		{
			hyperform=i;
			break;
		}
		if(db->Field_Descr(i)->a==X_POINTER && strstr(db->Name_Subbase(i),HYPERFORM)!=NULL)
		{
			hyperform=i;
			break;
		}
	}

	Form_Restruct();
	New_Index();

	act_field=0;
	while(act_field<num_fields && tags[act_field].des.atr & NO_POS)
		act_field++;

	db->Cadr_Read(Record(index));

	if(db->share!=NULL)
	{
		Ind_Update();
	}
	form_update();
	name=(char *)malloc(strlen(db->Name_Base())+strlen(HISTORY)+2);
	sprintf(name,"%s/%s",db->Name_Base(),HISTORY);
	if(!access(name,W_OK))
		ed=new ED(name);
	else    ed=NULL;
	free(name);
	f_mark_field=&mark_field;
	f_num_mark_fields=&num_mark_fields;
	if(db->share!=NULL)
		bcopy(&name_form,&db->share->form,sizeof name_form);
	if(!record)
	{
		form_cond|=NEW;
		New_Index();
		db->Cadr_Read(record=0);
		Table_Normalize();
		form_update();
		form_cond|=EDIT;
	}
	if(ret==c_GetSel)
	{
		Write();
		record=Record(index);
		db->Link_Index(output,level);
		index=find_page(record);
		db->Cadr_Read(Record(index));
		Go_To_Index(index);
	}
	load_menu();
	bg_orig=(char *)malloc(bg_size=size);
	memcpy(bg_orig,background,size);
}

void CX_BROWSER::Cadr_Read()
{
	if((cx_cond&HIST))
	{
		CX_BASE *hist;
		char *name=(char *)malloc(strlen(db->Name_Base())+strlen(LOGDB)+3);
		sprintf(name,"%s/%s",db->Name_Base(),LOGDB);
		try
		{
			hist = new CX_BASE(name);
		}
		catch(...)
		{
			goto A1;
		}
		free(name);
		char *hist_old=NULL;

		CX_FIND f(hist);
		long page;
		char str[32];

		sprintf(str,"%d",(int)Record(tags[act_field].index));
		if((page=f.Find_First(1,str,0))<=0)
		{
			delete hist;
			goto A1;
		}
		bzero(db->cadr,db->len_cadr);
		int rec;
		for(rec=0;page>0 && (hist_record==0 || rec<hist_record);rec++)
		{
			hist->Get_Slot(page,2,hist_old);
			strcpy(info,hist_old);
			strcat(info," ");
			hist->Get_Slot(page,3,hist_old);
			strcat(info,hist_old);
			strcat(info," ");
			hist->Get_Slot(page,4,hist_old);
			strcat(info,hist_old);
			strcat(info," ");
			hist->Get_Slot(page,5,hist_old);
			for(int i=0;i<db->len_cadr;i++)
				db->cadr[i]+=hist_old[i];
			page=f.Next();
			hist->Get_Slot(page,1,hist_old);
			if(atoi(hist_old)!=atoi(str))
			{
				hist_record=rec+1;
				break;
			}
		}
		if(hist_record==0)
			hist_record=rec;
		free(hist_old);
		delete hist;
		return;
	}
A1:
	cx_cond&=~HIST;
	if(tags!=NULL)
		db->Cadr_Read(Record(tags[act_field].index));
}

void CX_BROWSER::Load_Env(CX_BROWSER *std)
{
	if(mark!=NULL)
		free(mark);
	mark=NULL;
	cx_cond=0;
	num_mark=std->num_mark;
	if (std->personal_selection!=NULL && *std->personal_selection)
	{
		personal_selection = (char *)realloc(personal_selection,strlen(std->personal_selection)+1);
		strcpy(personal_selection,std->personal_selection);
	}else{
	   if (personal_selection!=NULL)
	   free(personal_selection);
	   personal_selection=NULL;
	}

	if(num_mark)
	{
		mark=(long *)malloc(num_mark*sizeof (long));
		bcopy(std->mark,mark,num_mark*sizeof (long));
	}
	level=std->level;
	if(std->form_cond&BW)
		form_cond|=BW;
	Set_Index();
	record=std->record;
	bzero(obr,sizeof obr);
	index=find_page(record);
	New_Index();
}

void CX_BROWSER::Go_To_Field(struct tag_descriptor td,long idx)
{
	if(!slacmp(tags[act_field].des.sla,td.sla))
		return;
	for(int i=0;i<num_fields;i++)
	{
		if(!slacmp(tags[i].des.sla,td.sla))
		{
			if(tags[i].des.atr & NO_POS)
				return;
			if(form_cond&TABLE)
			{
				if(Record(tags[i].index)!=idx)
					continue;
			}
			act_field=i;
			return;
		}
	}
	if(form_cond&ARRAY)
	{
		int i,first=-1,last=-1;
		struct sla sla[SLA_DEEP];

		bcopy(td.sla,sla,sizeof sla);
		sla->m=0;
		for(i=0;i<num_fields;i++)
		{
			struct sla sla1[SLA_DEEP];

			bcopy(tags[i].des.sla,sla1,sizeof sla1);
			sla1->m=0;
			if(!slacmp(sla,sla1))
				break;
		}
		if(i==num_fields)
			return;
		for(i=0;i<num_fields;i++)
			if(tags[i].des.atr&TABL)
			{
				if(first<0)
					first=i;
				last=i;
			}
		if(first<0 || last<0)
			return;
		act_field=first;
		bcopy(td.sla,sla,sizeof sla);
		sla->m=1;
		for(i=first;i<=last;i++)
		{
			if(!slacmp(tags[i].des.sla,sla))
			{
				act_field=i;
				break;
			}
		}
		sla->m=-1;
		int max=(read_only==0)+db->Num_Elem_Array(Record(index),sla);
		if(td.sla->m>=max)
			return;
		for(;;)
		{
			int found=0;
			for(i=first;i<=last;i++)
			{
				if(tags[i].des.sla->m>=max-1)
					return;
				tags[i].des.sla->m++;
				if(!slacmp(tags[i].des.sla,td.sla))
				{
					act_field=i;
					found=1;
				}
			}
			if(found)
				return;
		}
	}
}

void CX_BROWSER::Get_Form(char *base,struct x_form *x)
{
	if(x->form==0 && !*x->blank)
		Select_Form(base,x);
	if(x->form)
		create_Form(base,x->form);
	else
		create_Form(base,x->blank);

	for(int i=0;i<num_fields;i++)
	{
		if(tags[i].des.atr&TABL)
			continue;
		tags[i].index=index;
	}

	create_geom(x0,y0,l,term->l_y()-level);
	restore_bg();
}

int CX_BROWSER::Export(char *name)
{
	int fd=open(name,O_RDWR|O_CREAT|O_BINARY,0644);
	if(fd<0)
		return(-1);
	lseek(fd,0,SEEK_END);
	int i=Export(fd);
	close(fd);
	return(i);
}

int CX_BROWSER::Export(int fd)
{
	int y,x,i;

	for(y=0,x=0,i=0;i<size && background[i];i++)
	{

//                if(x>=colon && x<colon+l-2 && y<h+line-2 && y>=line)
		if(y>=line && y<h+line-2)
			write(fd,background+i,1);
		if(background[i]=='\n')
		{
			x=0;
			y++;
		}
		else if(background[i]=='\t')
			x=x+8-x%8;
		else
			x++;
		for(int j=0;j<num_fields;j++)
		{
			if(tags[j].des.x==x && tags[j].des.y==y && tags[j].str!=NULL)
			{
				unsigned char str[256];
				char cmd[32];

				if(string_digit(tags[j].str,db->int_delimiter))
				{
					sprintf(cmd,"%%%ds",(int)tags[j].des.l);
					sprintf((char *)str,cmd,tags[j].str);
				}
				else    strncpy((char *)str,tags[j].str,(sizeof str) -1);
				if(tags[j].des.l<(int)(sizeof str))
					str[tags[j].des.l]=0;
				int k,l=strlen((char *)str);
				for(k=0;k<l;k++)
					if(str[k]<' ')
						str[k]='?';
				write(fd,str,l);
				while(l<tags[j].des.l)
				{
					write(fd," ",1);
					l++;
				}
				i+=l;
				x+=l;
				break;
			}
		}
	}
	return(0);
}

void CX_BROWSER::Save_Index()
{
	char *ch,str[64];

	if((ch=getenv("SSH_CLIENT"))!=NULL || (ch=getenv("TELNET_CLIENT"))!=NULL)
	{
		strncpy(str,ch,(sizeof str)-1);
		char  *ch=strchr(str,' ');
		if(ch !=NULL)
			*ch=0;
	}
	else    *str=0;
	if(db!=NULL && db->insert && !inherit)
	{
		char *name=(char *)malloc(strlen(db->Name_Base())+strlen(TMPDIR)+strlen(GetLogin())+strlen(str)+16);
		sprintf(name,"%s/%s/.vs.%s[%s]",db->Name_Base(),TMPDIR,GetLogin(),str);
		Write_Index(name);
		free(name);
	}
}

void CX_BROWSER::Restore_Status(struct last_status *last)
{
	char *ch,str[64];
	if((ch=getenv("SSH_CLIENT"))!=NULL || (ch=getenv("TELNET_CLIENT"))!=NULL)
	{
		strcpy(str,ch);
		char  *ch=strchr(str,' ');
		if(ch!=NULL)
			*ch=0;
	}
	else    *str=0;
	char *name=(char *)malloc(strlen(db->Name_Base())+strlen(TMPDIR)+strlen(GetLogin())+strlen(str)+16);
	sprintf(name,"%s/%s/%s",db->Name_Base(),LIMITDIR,GetLogin());
	if(!access(name,R_OK))
	{
		personal_selection=(char *)realloc(personal_selection,strlen(name)+1);
		strcpy(personal_selection,name);
		Read_Index(name);
	}
	if(personal_selection==NULL)
	{
		sprintf(name,"%s/%s/.vs.%s[%s]",db->Name_Base(),TMPDIR,GetLogin(),str);
		if(!access(name,W_OK))
		{
			Next_Index_File(name);
			Add_Stack("Load_From_History",db->max_index);
		}
	}
	sprintf(name,"%s/%s/.vl.%s[%s]",db->Name_Base(),TMPDIR,GetLogin(),str);
	unlink(name);
	sprintf(name,"%s/.BW",db->Name_Base());
	if(!access(name,R_OK))
		form_cond|=BW;
	free(name);
	Act_Field(last->field);
	if(!last->record && record>0)
		last->record=record;
	if(!(form_cond&NEW))
		Go_To_Index(find_page(last->record));
}

void CX_BROWSER::Form_Refresh(struct tag_descriptor *td)
{
	struct tag_descriptor td1;

	if(td==NULL)
	{
		bcopy(&tags[act_field].des,&td1,sizeof td1);
		td=&td1;
	}
	if(hyperform)
	{
		char *name=NULL;
		long page;
		struct tag newtag;

		Del_Label();
		db->Get_Slot(record,hyperform,name);
		if(name==NULL || !*name)
			goto REST_FORM;
		bcopy(tags+act_field,&newtag,sizeof newtag);
		if(db->context->pswd==CXKEY3)
		{
			background=(char *)realloc(background,strlen(name)+1);
			strcpy(background,name);
			free(name);
			get_blank();
			Get_Size();
		}
		else
		{
			if(name!=NULL && (page=atoi(name+1)))
			{
				name=(char *)realloc(name,strlen(db->Name_Base())+strlen(HYPERFORM)+2);
				sprintf(name,"%s/%s",db->Name_Base(),HYPERFORM);
				create_Form(name,page);
				create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
				free(name);
				New_Index();
				if(!(form_cond&TABLE || form_cond&ARRAY))
				{
					Form_Restruct();
				}
				Go_To_Field(*td,Record(index));
			}
			else
			{
REST_FORM:
				struct x_form form=name_form;
				Get_Form(db->Name_Base(),&form);
			}
		}
		set_form();
		for(int i=0;i<num_fields;i++)
		{
			if(!slacmp(tags[i].des.sla,newtag.des.sla))
			{
				if(form_cond&TABLE)
				{
					if(Record(tags[i].index)!=record)
						continue;
				}
				act_field=i;
				break;
			}
		}
	}
	if((form_cond&TABLE || form_cond&ARRAY) && !(form_cond&MANUAL))
	{
		int i=act_field;
		int shift=0;

		if(num_colon)
			shift=(act_field-first_line)%num_colon;
		else    shift=0;

		create_geom(x0,y0,l,term->l_y()-level);

		if(!hyperform)
		{
			if(!strcmp(name_form.blank,"Manual"))
			{
				int y=tags[0].des.y;
				for(i=0;i<num_fields;i++)
					if(tags[i].des.y!=y)
						break;
				num_fields=i;
				for(i=0;background[i];i++)
				{
					if(background[i]=='\n')
						break;
				}
				background[i+1]=0;
				size=i+1;
				Get_Size();
			}
			else
			{
				if(!(form_cond&MANUAL))
					create_Form(db->Name_Base());
			}
		}
		Form_Restruct();

		for(i=1; i<=act_field && shift>0 && !possible(act_field-i);i++)
			shift--;
		New_Index();
		if(form_cond&ARRAY)
		{
			Go_To_Field(*td,Record(index));
		}
		else if(form_cond&TABLE)
		{
			act_field+=shift;
		}
	}
}
void CX_BROWSER::Refresh(int arg)
{
	if(arg==0)
	{
		if(form_cond&EDIT || form_cond&NEW)
		{
			form_update();
			return;
		}
		db->update();
		Go_To_Index(index);
		Table_Normalize();
		form_update();
		CX_Show();
	}
	else
	{
		int f=term->get_box(0,0,term->l_x(),term->l_y());
		term->refresh();
		delete_menu();
		term->scrbufout();
		term->restore_box(f);
		term->scrbufout();
		term->BlackWhite(0,0,term->l_x()+1,term->l_y());
		term->free_box(f);
		term->cursor_invisible();
		Go_To_Index(index);
		form_update();
		CX_Show();
		load_menu();
	}
}

int CX_BROWSER::Get_Slot(long record, struct tag *tag,char *&str)
{
#ifndef WIN32
	if((tag->des.sla->n > db->Num_Fields()) && cx3!=NULL)
	{
		int color=cx3->Get_Show(record,tag,str);
		if(color<0 || str==NULL)
			return(-1);
		int l=strlen(str);
		if(color==0)
			color=7;
		db->share->color.bg=(color>>4)&0xf;
		db->share->color.fg=color&0xf;
		db->share->font.fnt=-1;
		db->share->font.atr=0;
		return(l);
	}
#endif
	int i=0;
	if(db!=NULL)
	{
		if(db->share!=NULL)
		{
			bcopy(&tag->des,&db->share->slot,sizeof db->share->slot);
			db->share->font.fnt=-1;
			i=db->Get_Slot(record,tag->des.sla,str);
			bzero(&db->share->slot,sizeof db->share->slot);
		}
		else
			i=db->Get_Slot(record,tag->des.sla,str);
	}
	if(tag->des.atr&SECURE && (db->context->pswd==CXKEY5||db->context->pswd==CXKEY6))
	{
		if(str==NULL)
			return(-1);
		char *ch=str;
		while(*ch)
			*(ch++)='*';
	}
	return(i);
}

void CX_BROWSER::delete_menu()
{
//        del_menu();
	clean_menu();
}

void CX_BROWSER::load_menu(int page)
{
	if(form_atr&NOMENU)
		return;
	char *name_base=db->Name_Base();
	char *icon_base=(char *)malloc(strlen(name_base)+strlen(GetLogin())+strlen(ICONSDB)+3);

	sprintf(icon_base,"%s/%s",name_base,GetLogin());
	if(access(icon_base,R_OK))
		strcpy(icon_base,name_base);
	if(if_base(icon_base,ICONSDB))
	{
		strcat(icon_base,"/");
		strcat(icon_base,ICONSDB);
	}
	else
	{
		icon_base=(char *)realloc(icon_base,strlen(ICONSDEF)+1);
		strcpy(icon_base,ICONSDEF);
	}
	Load_Menu(icon_base,page);
	free(icon_base);
	show_menu();
}

int CX_BROWSER::V3Show()
{
	return(cx3!=NULL);
}

void CX_BROWSER::Read_Only(int i)
{
	read_only=i;
	if(i==2)
		hyperform=0;
	Go_To_Index(index);
}

void CX_BROWSER::Set_Parent_Record(long record)
{
	parent_record=record;
}

void hist_log(long record,char *name)
{
	CX_BASE *hist=NULL;
 
	try
	{
		hist = new CX_BASE(name);
	}
	catch(...)
	{
		return;
	}

	struct pics *ch=term->Screen_Buf();
	int w=term->l_x()+1;
	int h=term->l_y()+1;

	char str[32];

	long page=hist->New_Record();

	sprintf(str,"%d",(int)record);
	hist->Put_Slot(page,1,str);

	get_date(get_act_date(),str);
	hist->Put_Slot(page,2,str);

	get_time(get_act_time(),str);
	hist->Put_Slot(page,3,str);
	hist->Put_Slot(page,4,GetLogin());

	hist->Put_Slot(page,5,(char *)ch,w*h*sizeof (struct pics));
	sprintf(str,"%d",w);
	hist->Put_Slot(page,"^6.1",str);
	sprintf(str,"%d",h);
	hist->Put_Slot(page,"^6.2",str);

	hist->Unlock(page);
}
int CX_BROWSER::cxerror()
{
	return flag_cxerror;
}
