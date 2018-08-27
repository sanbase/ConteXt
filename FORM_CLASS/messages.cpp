/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:messages.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 

extern Terminal *term; 
extern Line *EditLine; 


char *message(int i) 
{ 
	static int pmes=-1; 
	static char *mes_str=NULL; 
	static CX_BASE *mess_db=NULL;
	if(i<0)
	{
		if(mess_db!=NULL)
			delete mess_db;
		mess_db=NULL;
		if(mes_str!=NULL)
			free(mes_str);
		mes_str=NULL;
		pmes=-1;
		return("");
	}
	if(i==pmes) 
		return(mes_str); 
	if(mess_db==NULL)
	{
		try
		    {
			mess_db=new CX_BASE(MSGDEF);
		}
		catch(...)
		{
			return("");
		}
	}
	mess_db->Get_Slot(i,1,mes_str);
	if(mes_str!=NULL) 
	{ 
		pmes=i; 
		return(mes_str); 
	} 
	pmes=-1; 
	return(""); 
} 


/*** функция центровки сообщения */ 
void centr(char *str) 
{ 
	int size,num; 
	register char *ch; 
	register int i; 
	char str1[256]; 

	if(str[strlen(str)-1]!=' ') 
		strcat(str," "); 
	if(!(size=strlen(str))) 
		return; 
	num=term->l_x()-size;      /* столько пробелов добавить */

	while(num>0) 
	{ 
		size=strlen(str)+1; 
		for(i=0,ch=str1;i<size;ch++,i++) 
		{ 
			if(str[i]==' ' && num) 
			{ 
				*ch=' '; 
				ch++; 
				num--; 
				for(;str[i]==' ' && i<size;i++,ch++) 
					*ch=str[i]; 
			} 
			*ch=str[i]; 
		} 
		strcpy(str,str1); 
	} 
	return; 
} 

static char *Keys[]= 
{ 
	"ESC",  "0-9",    "A-Z",     "F12",     "F3",  "Delete",      "Space",    "",
	"BackSpace",  "Tab.", "Ctrl/Enter", "F1",      "F2",  "Enter", "F4",  "F5", 
	"F6",   "F7",  "End",        "Home",    "Down","Up",    "->",  "<-", 
	"Ins",  "",    "PageUp",    "PageDown", "F8",  "F9",    "F10", "F11" 
}; 

void help_line(int i) 
{ 
	char str[256],*ch; 
	if(i>0) 
	{ 
		if((ch=message(i))!=NULL) 
			strcpy(str,ch); 
		if(*str==0) 
			return; 
		strcat(str," "); 
	} 
	else    *str=0; 
	hot_line(str); 
} 

void hot_line(char *str1)
{ 
	register int i; 
	char *ch; 
	char str[256];

	if(!*str1) 
	{ 
		term->Set_Color(0,7); 
		term->dpp(0,term->l_y()); 
		term->dpn(term->l_x(),' '); 
		return; 
	} 
	strcpy(str,str1); 
	centr(str); 
	term->dpp(0,term->l_y()); 
	term->Set_Color(0x103,0xa); 
	term->Set_Font(1,0); 
	term->dps(str); 
	term->Set_Color(0,7); 
	term->Set_Font(0,0); 

	for(i=0;i<32;i++) 
	{ 
		if((ch=strstr(str,Keys[i]))!=NULL) 
		{ 
			term->dpp(ch-str,term->l_y()); 
			term->dps(Keys[i]); 
		} 
	} 
}

void set_hot_key_status(char *key,int bg,int fg)
{
	char *ch;
	char str[256];
	int i;

	for(i=0;i<term->l_x()-1;i++)
	{
		str[i]=term->get_ch(i,term->l_y());
	}
	str[i]=0;
	if((ch=strstr(str,key))!=NULL)
	{
		int x=ch-str;
		int y=term->l_y();
		while(term->scr->Color(x,y).bg==0)
			x++;
		term->Set_Font(1,0);
		term->Set_Color(bg,fg);
		term->dpp(x,y);
		while(x<term->l_x() && term->scr->Color(x,y).bg!=0)
		{
			char ch=term->scr->Char(x,y).ch;
			term->dpo(ch);
			x++;
		}
		term->Set_Font(0,0);
	}
}

#ifndef WIN32 
#include <setjmp.h> 
static jmp_buf ret_buf; 
static void time_out(int i) 
{ 
	if(i==SIGALRM) 
		longjmp(ret_buf,1); 
} 
#endif 

int dial(char *mess,int atr,struct color *color,int font,char *outmess,int secure)
{ 
	int f,i=0; 
	if(mess==NULL) 
		return(0); 

	char *but2=NULL, *but1=NULL;
	if(atr<3&&secure!=0)
	{
		but2=strrchr(mess,'\n');
		if (but2!=NULL)
		{
			*but2=0;
			but2++;
		}
		but1=strchr(mess,'\n');
		if (but1!=NULL)
		{
			*but1=0;
			but1++;
		}
	}

	f=term->get_box(0,0,term->l_x(),term->l_y()); 
	term->BlackWhite(0,0,term->l_x(),term->l_y());
	int len=strlen(mess)+(atr==4?0:atr==5?32:8);

	if (but1!=NULL)
	 len=len+strlen(but1)-3;
	if (but2!=NULL)
	len=len+strlen(but2)-3;
	int x=(term->l_x()-len)/2; 
	int y=term->l_y()/2-5; 
	term->MultiColor(x,y,len+2,3); 
	term->BOX(x,y,len+2,3,' ',41,41,41,41);

	term->dpp(x+1,y+1); 
	if(color==NULL || (color->bg==0 && color->fg==0)) 
		term->Set_Color(41,0);
	else    term->Set_Color(color->bg,color->fg); 

	if(font==-1) 
		term->Set_Font(0,1); 
	else 
		term->Set_Font(font); 
	term->dps(mess);
	term->Set_Font(0,0);
	if(atr<3)
	{
		if (secure==0)
			i=yes(atr); 
		else
		{
			if(color==NULL || (color->bg==0 && color->fg==0))
				term->Set_Color(0x100+41,0);
			else    term->Set_Color(color->bg,color->fg);

			term->Set_Font(1,0);
			i=Button(atr,but1,but2);
		}
	}
	else 
	{ 
		if(atr==3) 
		{ 
			char str[32]; 
			*str=0; 
			term->cursor_visible(); 
			term->Set_Color(120,0);
			EditLine->edit(0,str,8,8,x+strlen(mess)+1,y+1,secure?-9:9);
			term->cursor_invisible(); 
			i=atoi(str); 
		} 
		if(atr==4)
		{ 
#ifndef WIN32 
			void (*old_sig)(int)=signal(SIGALRM,time_out); 
			signal(SIGALRM,time_out); 
			if(setjmp(ret_buf)) 
			{ 
				signal(SIGALRM,old_sig); 
				goto END; 
			} 
			alarm(4); 
			term->dpi(); 
			alarm(0); 
			signal(SIGALRM,old_sig); 
#else 
			term->dpi(); 
#endif
		} 
		else
		{
			term->cursor_visible();
			term->Set_Color(120,0);
			len=strlen(mess);
			if(outmess!=NULL)
			{
				i=EditLine->edit(0,outmess,64,32,x+len+1,y+1,secure);
			}
			else
			{
				*mess=0;
				i=EditLine->edit(0,mess,64,32,x+len+1,y+1,secure);
			}
			term->cursor_invisible();
		}
	} 
END:
	term->refresh();
	term->restore_box(f); 
	term->free_box(f); 
	return(i); 
} 

//extern int rus_lat; 
int Button(int a,char *but1,char *but2)
{
	if (but1==NULL||strlen(but1)<=0)
	{
		term->Set_Font(0,0);
		return (yes(a));
	}
	if (but2==NULL||strlen(but2)<=0)
	{
		term->Set_Font(0,0);
		return (yes(a));
	}
	int i;
	char *y=but1;
	char *n=but2;

	int x_y,x_n,y_y;
	i=a;
	x_y=term->x_c()+1;
	x_n=x_y+strlen(but1)+1;
	y_y=term->y_c();
	struct clr color=term->S_color();
BEGIN:
	term->Set_Color(color);
	term->dpp(x_y,y_y);
	term->dps(y); 
	term->dpp(x_n,y_y);
	term->dps(n);
	term->Set_Color(0x200+016,0);
	if(i)
	{ 
		term->dpp(x_y,y_y); 
		term->dps(y); 
		term->dpp(x_y,y_y); 
	}
	else    { 
		term->dpp(x_n,y_y); 
		term->dps(n); 
		term->dpp(x_n,y_y); 
	}
	switch(term->dpi())
	{
	case 0:
		if(term->ev().y==y_y && term->ev().b==1)
		{
			if(term->ev().x>=x_y && term->ev().x<x_n-1)
			{
				if(i==0)
				{
					i=1;
					break;
				}
				term->Set_Color(color);
				return(i);
			}
			if(term->ev().x>=x_n && term->ev().x<x_n+(int)strlen(but2))
			{
				if(i==1)
				{
					i=0;
					break;
				}
				term->Set_Color(color);
				return(i);
			}
		}
		break;
	case CR:
		i=0;
		break;
	case CL:
		i=1;
		break;
	case 'y':
	case 'Y':
	case 'д':
	case 'Д':
		return(1);
	case 'n':
	case 'N':
	case 'н':
	case 'Н':
	case F10:
		return(F10);
//                case F10:
//                        i=F10;
	case '\r':
		term->Set_Color(color);
		return(i);
	default:
		break;
	}
	goto BEGIN;
}
int yes(int a) 
{ 
	int i; 
	char *y=" y "; 
	char *n=" n "; 
	/*** 
			if(rus_lat!=0) 
			{ 
				y=" д "; 
				n=" н "; 
			} 
		*/ 
	int x_y,x_n,y_y; 
	i=a; 
	x_y=term->x_c()+1; 
	x_n=x_y+3; 
	y_y=term->y_c(); 
	struct clr color=term->S_color();
BEGIN: 
	term->Set_Color(color);
	term->dpp(x_y,y_y); 
	term->dps(y); 
	term->dps(n); 
	term->Set_Color(016,0);
	if(i) 
	{ 
		term->dpp(x_y,y_y); 
		term->dps(y); 
		term->dpp(x_y,y_y); 
	} 
	else    { 
		term->dpp(x_n,y_y); 
		term->dps(n); 
		term->dpp(x_n,y_y); 
	} 
	switch(term->dpi()) 
	{ 
	case 0: 
		if(term->ev().y==y_y && term->ev().b==1) 
		{ 
			if(term->ev().x>=x_y && term->ev().x<x_y+3) 
			{ 
				if(i==0) 
				{ 
					i=1; 
					break; 
				} 
				term->Set_Color(color);
				return(i); 
			} 
			if(term->ev().x>=x_n && term->ev().x<x_n+3) 
			{ 
				if(i==1) 
				{ 
					i=0; 
					break; 
				} 
				term->Set_Color(color);
				return(i); 
			} 
		} 
		break; 
	case CR: 
		i=0; 
		break; 
	case CL: 
		i=1; 
		break; 
	case 'y': 
	case 'Y': 
	case 'д': 
	case 'Д': 
		return(1); 
	case 'n': 
	case 'N': 
	case 'н': 
	case 'Н': 
	case F10: 
		return(0); 
//                case F10: 
//                        i=F10; 
	case '\r': 
		term->Set_Color(color);
		return(i); 
	default: 
		break; 
	} 
	goto BEGIN; 
} 

int X1,Y1; 
static int calc_codes(int i) 
{ 
	if(i==0) 
	{ 
		if(term->ev().y==Y1+3) 
		{ 
			if(term->ev().x>=X1+2 && term->ev().x<=X1+4) 
				return('7'); 
			if(term->ev().x>=X1+6 && term->ev().x<=X1+8) 
				return('8'); 
			if(term->ev().x>=X1+10 && term->ev().x<=X1+12) 
				return('9'); 
			if(term->ev().x>=X1+16 && term->ev().x<=X1+18) 
				return('+'); 
			if(term->ev().x>=X1+20 && term->ev().x<=X1+22) 
				return('('); 
			if(term->ev().x>=X1+24 && term->ev().x<=X1+26) 
				return(F10); 
		} 
		if(term->ev().y==Y1+5) 
		{ 
			if(term->ev().x>=X1+2 && term->ev().x<=X1+4) 
				return('4'); 
			if(term->ev().x>=X1+6 && term->ev().x<=X1+8) 
				return('5'); 
			if(term->ev().x>=X1+10 && term->ev().x<=X1+12) 
				return('6'); 
			if(term->ev().x>=X1+16 && term->ev().x<=X1+18) 
				return('-'); 
			if(term->ev().x>=X1+20 && term->ev().x<=X1+22) 
				return(')'); 
		} 
		if(term->ev().y==Y1+7) 
		{ 
			if(term->ev().x>=X1+2 && term->ev().x<=X1+4) 
				return('1'); 
			if(term->ev().x>=X1+6 && term->ev().x<=X1+8) 
				return('2'); 
			if(term->ev().x>=X1+10 && term->ev().x<=X1+12) 
				return('3'); 
			if(term->ev().x>=X1+16 && term->ev().x<=X1+18) 
				return('*'); 
			if(term->ev().x>=X1+20 && term->ev().x<=X1+22) 
				return('\r'); 
		} 
		if(term->ev().y==Y1+9) 
		{ 
			if(term->ev().x>=X1+2 && term->ev().x<=X1+4) 
				return('0'); 
			if(term->ev().x>=X1+6 && term->ev().x<=X1+8) 
				return('.'); 
			if(term->ev().x>=X1+10 && term->ev().x<=X1+12) 
				return(DEL); 
			if(term->ev().x>=X1+16 && term->ev().x<=X1+18) 
				return('/'); 
			if(term->ev().x>=X1+20 && term->ev().x<=X1+22) 
				return(F8); 
		} 
	} 
	return(i); 
} 

int calc=0; 

int Xmouse(int i) 
{ 
	static struct Keys K[]= 
	    { 
		{
			"Esc",27				}
		, 
		{
			"F1",F1				}
		, 
		{
			"F2",F2				}
		, 
		{
			"F3",F3				}
		, 
		{
			"F4",F4				}
		, 
		{
			"F5",F5				}
		, 
		{
			"F6",F6				}
		, 
		{
			"F7",F7				}
		, 
		{
			"F8",F8				}
		, 
		{
			"F9",F9				}
		, 
		{
			"F10",F10				}
		, 
		{
			"F11",F11				}
		, 
		{
			"F12",F12				}
		, 
		{
			"Down",CD				}
		, 
		{
			"Up",CU				}
		, 
		{
			"->",CR				}
		, 
		{
			"<-",CL				}
		, 
		{
			"End",EN				}
		, 
		{
			"Home",HM				}
		, 
		{
			"Ins",IS				}
		, 
		{
			"PageUp",PU				}
		, 
		{
			"PageDown",PD				}
		, 
		{
			"BackSpace",'\b'				}
		, 
		{
			"Tab",'\t'				}
		, 
		{
			"Ctrl/Enter",'\n'				}
		,
		{
			"Enter",'\r'				}
		,
		{
			"Delete",127				}
	}; 
	char str[256]; 
	int l=0,x,y=term->l_y(); 
	int clr; 

	if(calc) 
		i=calc_codes(i); 

	if(term->ev().y!=y) 
		goto END; 
	x=term->ev().x; 
	while(x && (clr=term->get_fg(x,y))!=(7)) x--; 
	while(x && (clr=term->get_fg(x,y))==(7)) x--; 
	if(x) x++; 
	while((clr=term->get_ch(x,y))==' ') x++; 
	while((str[l]=term->get_ch(x++,y))!=' ') l++; 
	str[l]=0; 
	for(x=0;x<(int)(sizeof K/sizeof (struct Keys));x++) 
	{ 
		if(!strcmp(str,K[x].name)) 
		{ 
			term->flush_mouse(); 
			return(K[x].code); 
		} 
	} 

END: 
	return(i); 

} 

static struct color Type_Color_Def[]=
{
	{
		016, 06 		}
,      //UNKNOWN
	{
		016,010 		}
,      //X_STRING
	{
		07,010  		}
,      //X_DATE
	{
		07,010  		}
,      //X_TIME
	{
		016, 01 		}
,      //X_POINTER
	{
		017, 01 		}
,      //X_VARIANT
	{
		012,010 		}
,      //X_INTEGER
	{
		012,010 		}
,      //X_UNSIGNED
	{
		012,010 		}
,      //X_FLOAT
	{
		012,010 		}
,      //X_DOUBLE
	{
		0,   03 		}
,      //X_STRUCTURE
	{
		012,010 		}
,      //X_TEXT
	{
		014,010 		}
,      //X_BINARY
	{
		017,010 		}
,      //X_FILENAME
	{
		015,010 		}
,      //X_EXPRESSION
	{
		016, 04 		}
,      //X_IMAGE
	{
		015, 07 		}       //X_COMPLEX
};

static int colors_load=0;

struct color Type_Color(CX_BASE *db,int type)
{
	static struct color Type_Color_Extent[17];
	static int colors_load_extent=0;
	int fd;

	if(!colors_load_extent)
	{
		char *type_colors=(char *)malloc(strlen(db->Name_Base())+32);
		colors_load_extent=1;
		sprintf(type_colors,"%s/.cx_colors",db->Name_Base());
		if((fd=open(type_colors,O_RDWR|O_BINARY))>0)
		{
			read(fd,Type_Color_Extent,sizeof Type_Color_Extent);
			close(fd);
			colors_load_extent=2;
		}
		free(type_colors);
	}
	if(!colors_load)
	{
		char *home=getenv("HOME");

		colors_load=1;
		if(home!=NULL)
		{
			char *type_colors=(char *)malloc(strlen(home)+32);

			sprintf(type_colors,"%s/.cx_colors",home);
			if((fd=open(type_colors,O_RDWR|O_BINARY))>0)
			{
				read(fd,Type_Color_Def,sizeof Type_Color_Def);
				close(fd);
			}
			free(type_colors);
		}
	}
	if(colors_load_extent>1)
	{
		if(type<0 || type>=(int)((sizeof Type_Color_Extent)/sizeof (struct color)))
			type=0;
		return(Type_Color_Extent[type]);
	}
	else
	{
		if(type<0 || type>=(int)((sizeof Type_Color_Def)/sizeof (struct color)))
			type=0;
		return(Type_Color_Def[type]);
	}
}
