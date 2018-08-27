/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:cmd.cpp
*/
#include "stdafx.h" 

#include "CX_Browser.h" 

extern Terminal *term; 
extern Line *EditLine; 

int CX_BROWSER::Command() 
{ 
	int ret; 
	int f,typ,len=35; 
	char str[256],*ch=NULL; 
	char str1[LINESIZE]; 
	int img_x=0,img_y=0;

	Write(); 
	typ=db->Read(record=Record(tags[act_field].index),tags[act_field].des.sla,ch).field.a; 
	if(ch!=NULL) 
		free(ch); 
	if(typ<0) 
		typ=0; 
	sla_to_str(tags[act_field].des.sla,str); 
	if((ch=db->Name_Field(tags[act_field].des.sla))==NULL) 
		sprintf(str1,"%s",tags[act_field].name); 
	else 
		sprintf(str1,"%s",ch); 

	if((int)strlen(str1)+4>len) 
		len=strlen(str1)+4; 
	f=term->get_box(2,term->l_y()-5,len+2,5); 
	term->MultiColor(2,term->l_y()-5,len,4); 
	term->BOX(2,term->l_y()-5,len,4,' ',41,41,41,41);
//        term->Set_Font(0,0);
	term->Set_Color(41,0xf);
	term->dpp(1+len-strlen(str),term->l_y()-5);
	term->dps(str); 

	term->Set_Font(5,1);
	term->Set_Color(41,0);
	char str2[64];
//        sprintf(str2," Object:%ld",record);
	sprintf(str2," Object:%d->%d",record,tags[act_field].index);
	term->dpp(3,term->l_y()-5);
	term->dps(str2);

	term->Set_Color(41,0);
	term->dpp(3,term->l_y()-4);
	term->dps(str1); 
	if(db->cx_cond&SORT && !(cx_cond&HIST))
	{
		term->Show_Image(img_x=len,img_y=term->l_y()-4,"Images/CX/ltgreen.dot.gif",message(68));
	}
	*str=0; 
ECM: 
	term->cursor_visible(); 
	term->scr->draw_frame(2,term->l_y()-5,len,4);
	term->Set_Color(120,0);
	if((ret=EditLine->edit(0,str,64,len-2,3,term->l_y()-3,0))==PU || ret==PD)
	{ 
		char *str1=NULL; 
		Get_Slot(Record(tags[act_field].index),tags+act_field,str1); 
		if(str1!=NULL) 
		{ 
			int len=strlen(str); 
			strncat(str,str1,(sizeof str)-len-1); 
			free(str1); 
		} 
		goto ECM; 
	} 
	if(ret==F1)
	{
		Help(2,level+1);
		goto ECM;
	}
	if(ret==0 &&  term->mouse().y==img_y && term->mouse().x==img_x)
	{
		Cmd_Exe(c_NoSel);
		term->Del_Image(img_x,img_y);
		img_x=0 ;
		img_y=0;
		goto ECM;
	}
	term->cursor_invisible(); 
	ch=str; 
/*
	while(*ch && *ch==' ') 
		ch++; 
*/
	if(*ch=='"') 
	{ 
		ch++; 
		char *ch1=strchr(ch,'"'); 
		if(ch1!=NULL) 
			*ch1=0; 
	} 
	strcpy(str1,ch); 
	strcpy(str,str1); 
	if(ret==F8 || (ret=='\r' && !*str)) 
	{ 
		term->restore_box(f); 
		term->BlackWhite(0,0,term->l_x(),term->l_y()); 
		if(db->cx_cond&SORT && !(cx_cond&HIST)) 
		{ 
			if(ret==F8) 
				Cmd_Exe(c_RestIdx); 
			else if(!*str) 
			{ 
				if(img_x!=0)
					term->Del_Image(img_x,img_y);
				img_x=0;
				if(dial(message(18),1)) 
					Cmd_Exe(c_NoSel); 
			} 
		} 
		goto END; 
	} 
	if(ret==F7) 
	{ 
		if(db->Field_Descr(tags[act_field].des.sla[0].n)->a==X_POINTER) 
		{ 
			struct sla sla1[SLA_DEEP]; 

			bzero(sla1,sizeof sla1); 
			bcopy(tags[act_field].des.sla+1,sla1,(SLA_DEEP-1)*sizeof(struct sla)); 
			long page=Select_From_DB(db->Name_Subbase(tags[act_field].des.sla[0].n),sla1,tags[act_field].des.l); 
			CX_BASE *subbase; 
			try 
			    { 
				subbase = new CX_BASE(db->Name_Subbase(tags[act_field].des.sla[0].n));
			} 
			catch(...) 
			{ 
				if(img_x!=0)
					term->Del_Image(img_x,img_y);
				return(0); 
			} 
			char *ch=NULL; 
			subbase->Get_Slot(page,tags[act_field].des.sla+1,ch); 
			if(ch!=NULL) 
			{ 
				strcpy(str,ch); 
				free(ch); 
			} 
			delete subbase; 
		} 
		else 
		{ 

			CX_BROWSER map(this); 

//                        map.db->Map();
			map.db->Inherit(map.db); 
			map.Load_Env(this); 

			map.db->Cadr_Read(Record(tags[act_field].index)); 
			map.form_update(); 

			if(map.Action()=='\r') 
				strcpy(str,map.tags[map.act_field].str); 
//                        Load_Env(&map); 
		} 
		goto ECM; 
	} 
	term->restore_box(f); 
	term->BlackWhite(0,0,term->l_x(),term->l_y()); 
	term->MultiColor(x0,y0,l,h); 
	if(*str=='#') 
	{ 
		long ind=atoi(str+1); 
		if(ind>db->max_index) 
			ind=db->max_index; 
		if(!(form_cond&SHOWDEL)) 
		{ 
			while(ind<=db->max_index && db->Check_Del(Record(ind))) ind++; 
			while(ind>0 && db->Check_Del(Record(ind))) ind--; 
			if(ind==0) 
				goto END; 

		} 
		index=ind; 
		goto END; 
	} 
	if(!strcmp(str,"+") || !strcmp(str,"$")) 
	{ 
		int f=-1,x,y,n=0; 
		double summ=0; 
		char *ch=NULL; 
		long max=*str=='+'?db->max_index:index; 

		if(max>1000 || db->Field_Descr(tags[act_field].des.sla[0].n)->a==0) 
		{ 
			x=(term->l_x()-50)/2; 
			y=(term->l_y()/2); 
			f=term->get_box(x-2,y-2,50+6,5); 
			term->BOX(x-1,y-1,52,3,' ',6,0xf,6,0xf); 
//                        term->MultiColor(x-1,y-1,52,3); 
			term->Set_Color(03,0); 
			term->dpp(x,y); 
			term->scrbufout(); 
		} 
		for(int page=1;page<=max;page++) 
		{ 
			long rec=Record(page); 

			if(f>=0 && max>50 && (page%(max/50))==0 && n<50) 
			{ 
				term->dpo(' '); 
				n++; 
				term->scrbufout(); 
			} 
			if(db->Check_Del(rec)) 
				continue; 
			Get_Slot(rec,tags+act_field,ch); 
			if(ch!=NULL) 
				summ+=db->atof(ch);
		} 
		if(ch!=NULL) 
			free(ch); 
		if(f>=0) 
		{ 
			term->restore_box(f); 
			term->free_box(f); 
			f=-1; 
		} 
		term->dpp(4,y0+h-1); 
		term->Set_Color(0x200+016,16); 
		sprintf(str," Sum = %f",summ); 
		if(strchr(str,'.')!=NULL) 
		{ 
			int i; 
			for(i=strlen(str)-1;i&&str[i]=='0';i--) 
				str[i]=0; 
		} 
		strcat(str," "); 
		term->dps(str); 
		term->dpp(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1); 
		if(img_x!=0)
			term->Del_Image(img_x,img_y);
		return(Xmouse(term->dpi())); 
	} 
	if(f>=0) 
		term->free_box(f); 

	if(!*str || ret==F10) 
		goto EXIT;

	struct query query; 

	query.str=str; 
	memcpy(query.sla,tags[act_field].des.sla,sizeof query.sla); 
	Query(&query); 
END: 
	Go_To_Index(index); 
EXIT:
	if(img_x!=0)
		term->Del_Image(img_x,img_y);
	term->MultiColor(x0,y0,l,h);
	return(0); 
} 

extern int calc; 
extern int X1,Y1; 
void CX_BROWSER::Calculator() 
{ 
	int ret,f; 

	char *str; 
	double rez=0; 

	str=(char *)malloc(128); 
	calc=1; 
	term->Set_Color(0,7); 
	*str=0; 
	X1=(term->l_x()-30)/2; 
	Y1=(term->l_y()-12)/2; 
	term->MultiColor(X1,Y1,29,11); 
	f=term->get_box(X1,Y1,31,12); 
	term->BOX(X1,Y1,29,11,' ',6,0x7,6,0x7); 
	term->goriz_s(X1,Y1+2,28); 
	term->Set_Color(0X100+30,14); 

	term->dpp(X1+ 2,Y1+3); term->dps(" 7 "); term->dpp(X1+16,Y1+3); term->dps(" + ");
	term->dpp(X1+ 6,Y1+3); term->dps(" 8 "); term->dpp(X1+16,Y1+5); term->dps(" - ");
	term->dpp(X1+10,Y1+3); term->dps(" 9 "); term->dpp(X1+16,Y1+7); term->dps(" * ");
	term->dpp(X1+ 2,Y1+5); term->dps(" 4 "); term->dpp(X1+16,Y1+9); term->dps(" / ");
	term->dpp(X1+ 6,Y1+5); term->dps(" 5 "); term->dpp(X1+20,Y1+3); term->dps(" ( ");
	term->dpp(X1+10,Y1+5); term->dps(" 6 "); term->dpp(X1+20,Y1+5); term->dps(" ) ");
	term->dpp(X1+ 2,Y1+7); term->dps(" 1 "); term->dpp(X1+20,Y1+7); term->dps(" = ");
	term->dpp(X1+ 6,Y1+7); term->dps(" 2 "); term->dpp(X1+20,Y1+9); term->dps(" C ");
	term->dpp(X1+10,Y1+7); term->dps(" 3 ");
	term->dpp(X1+ 2,Y1+9); term->dps(" 0 ");
	term->dpp(X1+ 6,Y1+9); term->dps(" . ");
	term->dpp(X1+10,Y1+9); term->dps("Del");

	term->Set_Color(0x100+12,14); 
	term->dpp(X1+24,Y1+3);term->dps("Off");
	term->Set_Color(0x108,016);
BEGIN: 
	term->cursor_visible(); 
	if((ret=EditLine->edit(0,str,126,25,X1+2,Y1+1,0))==F10) 
		goto EXIT; 
	if(ret==PU || ret==PD) 
	{ 
		strcat(str,tags[act_field].str); 
		goto BEGIN; 
	} 
	if(ret==F8) 
	{ 
		*str=0; 
		goto BEGIN; 
	} 
	rez=db->Expression(record,str); 
	sprintf(str,"%f",rez); 

	if(strchr(str,'.')!=NULL) 
	{ 
		for(int i=strlen(str)-1;i;i--) 
		{ 
			if(str[i]=='0') 
			{ 
				str[i]=0; 
				if(str[i-1]=='.') 
				{ 
					str[i-1]=0; 
					break; 
				} 
			} 
			else    break; 
		} 
	} 
	if(db->atof(str)==0)
		strcpy(str,"0"); 

	goto BEGIN; 
EXIT: 
	term->restore_box(f); 
	term->scrbufout(); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	term->free_box(f); 
	free(str); 
	calc=0; 
	term->cursor_invisible(); 
	return; 
} 

int CX_BROWSER::Show_Stack() 
{ 
	int len=0,f; 
	int x,y,h1=num_stack_sel; 

	if(!num_stack_sel) 
		return(0); 
	if(h1>term->l_y()-5) 
		h1=term->l_y()-5; 
	int i; 
	for(i=0;i<h1;i++) 
	{ 
		if((stack+i)==NULL) 
			continue; 
		int j=strlen(stack[i].query)+strlen(stack[i].name)+13; 
		if(j>len) 
			len=j; 
	} 
	if(len>term->l_x()-5) 
		len=term->l_x()-5; 
	x=(term->l_x()-len)/2; 
	y=(term->l_y()-h1)/2; 
	f=term->get_box(x,y,len+2,h1+5); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	term->MultiColor(x,y,len,h1+2); 
	term->BOX(x,y,len,h1+2,' ',6,0xe,6,0xe); 
	for(i=0;i<h1;i++) 
	{ 
		char str[64]; 

		if((stack+i)==NULL) 
			continue; 
		term->dpp(x+1,y+i+1); 
		sprintf(str,"%6d ",(int)stack[i].num_index); 
		term->dps(str); 
		term->dps(" "); 
		term->dps(stack[i].name); 
		term->dps(": "); 
		term->dps(stack[i].query); 
	} 
	hot_line("F10 exit "); 
	term->dpi(); 
	term->restore_box(f); 
	term->free_box(f); 
	term->BlackWhite(0,0,term->l_x(),term->l_y()); 
	term->MultiColor(x0,y0,l,h); 
	return(0); 
} 

int CX_BROWSER::Show_Sum()
{
	int f=-1,x,y,n=0;
	double summ=0;
	char *ch=NULL;
//        long max=*str=='+'?db->max_index:index;
	long max=db->max_index;
	char str[256];
	if(max>1000 || db->Field_Descr(tags[act_field].des.sla[0].n)->a==0)
	{
		x=(term->l_x()-50)/2;
		y=(term->l_y()/2);
		f=term->get_box(x-2,y-2,50+6,5);
		term->BOX(x-1,y-1,52,3,' ',6,0xf,6,0xf);
		term->Set_Color(03,0);
		term->dpp(x,y);
		term->scrbufout();
	}
	for(int page=1;page<=max;page++)
	{
		long rec=Record(page);

		if(f>=0 && max>50 && (page%(max/50))==0 && n<50)
		{
			term->dpo(' ');
			n++;
			term->scrbufout();
		}
		if(db->Check_Del(rec))
			continue;
		Get_Slot(rec,tags+act_field,ch);
		if(ch!=NULL)
			summ+=db->atof(ch);
	}
	if(ch!=NULL)
		free(ch);
	if(f>=0)
	{
		term->restore_box(f);
		term->free_box(f);
		f=-1;
	}
	term->dpp(4,y0+h-1);
	term->Set_Color(0x200+0xf,0);
	term->Set_Font(1,2);

	sprintf(str," Sum = %f",summ);
	if(strchr(str,'.')!=NULL)
	{
		int i;
		for(i=strlen(str)-1;i&&str[i]=='0';i--)
			str[i]=0;
	}
	strcat(str," ");
	term->dps(str);
	term->dpp(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1);
	return(Xmouse(term->dpi()));
}
