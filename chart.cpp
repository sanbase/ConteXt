/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:chart.cpp
*/
#include "stdafx.h" 
 
#include <math.h> 
#include "CX_Browser.h" 
#include <time.h> 
 
extern Terminal *term; 
extern Line *line; 
 
extern struct image *images; 
 
static char *dot[]= 
{ 
	"Images/CX/yellow.dot.gif", 
	"Images/CX/red.dot.gif", 
	"Images/CX/ltgreen.dot.gif", 
	"Images/CX/blue.dot.gif", 
	"Images/CX/white.dot.gif", 
	"Images/CX/purple.dot.gif", 
	"Images/CX/orange.dot.gif", 
	"Images/CX/cyan.dot.gif", 
	"Images/CX/green.dot.gif", 
	"" 
}; 
static int chart_color[]={0xe,0xc,0xa,0xb,0xf,0x9,35,0x3,0x2,0x1}; 
static void filtr(float *buf,int len,int step,int n) 
{ 
	float fltr[23],b=0,*buf_tmp,a; 
	int i,j,len_tmp,st; 
 
	if(term->l_x()<64) 
		return; 
	st=(term->l_x()-29)*term->font_W()/len; 
	len_tmp=len*st; 
 
	buf_tmp=(float *)calloc((len_tmp+23),sizeof (float)); 
 
	for(i=-11;i<=11;i++) 
	{ 
		double x=3.1415*0.55*i/11.0; 
		b+=(fltr[i+11]=(float)cos(x)/(1.0+x*x)); 
	} 
	for(i=0;i<len;i++) 
	{ 
		int j; 
		double a,b,c; 
 
		a=buf[i*step+n]; 
		if(i==len-1 || st<=0) 
			c=0; 
		else 
		{ 
			b=buf[(i+1)*step+n]; 
			c=(b-a)/st; 
		} 
 
		for(j=0;j<st;j++) 
		{ 
			buf_tmp[i*st+11+j]=a; 
			a+=c; 
		} 
	} 
 
	for(a=0,i=0;i<23 && i<len;i++) 
		a+=buf[i*step+n]; 
	j=i; 
	for(i=0;j>0 && i<j/2;i++) 
		buf_tmp[i]=a/j; 
 
	for(a=0,i=len-1;i>len-23 && i>=0;i--) 
		a+=buf[i*step+n]; 
	j=len-i-1; 
	for(i=len_tmp+11;j>0 && i<len_tmp+23;i++) 
		buf_tmp[i]=a/j; 
 
	for(i=0;i<len_tmp;i++) 
	{ 
		for(a=0,j=0;b>0 && j<23;j++) 
			a+=buf_tmp[i+j]*fltr[j]/b; 
		buf_tmp[i]=a; 
	} 
	for(i=0;i<len;i++) 
		buf[i*step+n]=buf_tmp[i*st]; 
	free(buf_tmp); 
} 
 
static void draw_gbox(int x0,int l,int HIGH,int *mark,int max_index,int width) 
{ 
	term->box2(3,3,width-6,HIGH+3,' ',07,0,016,23); 
	term->vert_s(24,3,HIGH+2); 
	term->shadow(3,3,width-6,HIGH+3); 
	term->MultiColor(3,3,width-6,HIGH+3); 
	term->MultiColor(x0,term->l_y(),l,1); // help line 
 
	for(int j=4;j<4+HIGH+1;j++) 
		for(int i=0;i<(width-29);i++) 
		{ 
			term->dpp(i+25,j); 
			if(max_index>1 && i>((mark[0]-1)*(term->l_x()-29))/(max_index-1) && i<=((mark[1]-1)*(term->l_x()-29))/(max_index-1)) 
				term->Set_Color(0x200+23,03); 
			else    term->Set_Color(0x200+24,03); 
			term->dpo(' '); 
		} 
} 
 
float get_diff(float a[],float *value,int l,int step, int i) 
{ 
	float v1,v2; 
 
	v1=value[(int)(l*a[0]-1)*step + i]; 
	if(v1==0) 
		return(100.0); 
	v2=value[(int)(l*a[1]-1)*step + i]; 
	return((float)100.0*(v2-v1)/v1); 
} 
 
static void diff(float *buf,int len,int step,int n) 
{ 
	int i; 
 
	for(i=0;i<len-1;i++) 
		buf[i*step+n]=buf[(i+1)*step+n]-buf[i*step+n]; 
	buf[(len-1)*step+n]=0; 
} 
 
 
static void integral(float *buf,int len,int step,int n) 
{ 
	int i; 
	float a=0; 
 
	a=buf[n]; 
	buf[n]=0; 
	for(i=1;i<len;i++) 
	{ 
		a+=buf[i*step+n]; 
		buf[i*step+n]=a; 
	} 
} 
 
 
int CX_BROWSER::Draw_Graph(int circle) 
{ 
	if(num_mark_fields<2) 
	{ 
		dial(message(17),4);
		return(-1); 
	} 
	if(num_mark_fields>9) 
		num_mark_fields=9; 
	form_cond&=~MARK; 
	double min,max,sum=0; 
	double *value = (double *)calloc(num_mark_fields,sizeof (double)); 
	char *ch=NULL; 
	int i,f=-1; 
 
	for(int field=0;field<num_mark_fields;field++) 
	{ 
		if(db->max_index>1000  || (db->max_index>term->l_x()-29 && mark_field[field].des.sla[0].n>db->Num_Fields())) 
		{ 
			int x=29/2; 
			int y=(term->l_y()/2); 
			int size=0; 
 
			for(int page=1;page<=db->max_index;page++) 
				if(page%(db->max_index/(term->l_x()-29))==0) 
					size++; 
			f=term->get_box(x-2,y-2,size+6,5); 
			term->BOX(x-1,y-1,size+2,3,' ',6,0xf,6,0xf), 
			term->Set_Color(016,0); 
			term->dpp(x,y); 
			term->scrbufout(); 
			break; 
		} 
	} 
	term->Set_Color(016,0); 
	for(int page=1;page<=db->max_index;page++) 
	{ 
		if(db->max_index>=(term->l_x()-29) && f>=0 && page%(db->max_index/(term->l_x()-29))==0) 
		{ 
			term->dpo(' '); 
			term->scrbufout(); 
		} 
		if(db->Check_Del(Record(page))) 
			continue; 
		for(int field=0;field<num_mark_fields;field++) 
		{ 
/*
			Get_Slot(Record(page),mark_field+field,ch); 
			value[field]+=atof(ch); 
*/
			value[field]+=db->Get_Value(Record(page),mark_field[field].des.sla);
		} 
	} 
	min=max=value[0]; 
 
	for(i=0;i<num_mark_fields;i++) 
	{ 
//                if(value[i]<0) 
//                        value[i]=-value[i]; 
		sum+=value[i]; 
	} 
 
	if(sum==0) 
		sum=1; 
	for(i=0;i<num_mark_fields;i++) 
	{ 
		if(value[i]>max) 
			max=value[i]; 
		if(value[i]<min) 
			min=value[i]; 
	} 
	for(i=0;i<num_mark_fields;i++) 
	{ 
		value[i]=value[i]/sum; 
	} 
	if(f>=0) 
	{ 
		term->restore_box(f); 
		term->free_box(f); 
	} 
	f=term->get_box(0,0,term->l_x(),term->l_y()); 
 
	int HIGH=term->l_y()-7; 
	if(HIGH>20) 
		HIGH=20; 
 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	draw_gbox(x0,l,HIGH,(int *)NULL,0,circle?8+30+((HIGH+1)*term->font_H()/term->font_W()):term->l_x()); 
 
	double zoom; 
	if(max==0 && min==0) 
		zoom=0; 
	else 
		zoom=sum/(max-(min<0?min:0)); 
	int shift=(int)(min>0?0:((HIGH)*zoom*min/sum)); 
	if(circle) 
	{ 
		term->Set_Dimension(30,4,((HIGH+1)*term->font_H()/term->font_W()),HIGH+1); 
	} 
	int x=0; 
	for(i=0;i<HIGH && i<num_mark_fields;i++) 
	{ 
		char str[LINESIZE],*name; 
		char summa[32],*ch; 
 
		term->Set_Color(23,chart_color[i%10]); 
		term->dpp(6,5+i*2); 
		if((name=mark_field[i].name)==NULL) 
			name=db->Name_Field(mark_field[i].des.sla); 
		if(name!=NULL) 
			strncpy(str,name,17); 
		else    *str=0; 
		str[17]=0; 
		term->dps(str); 
		term->Set_Color(0x200+23,chart_color[i%10]); 
		term->dpp(6,6+i*2); 
		sprintf(summa,"%f",value[i]*sum); 
		if(strchr(summa,'.')!=NULL) 
		{ 
			ch=summa+strlen(summa)-1; 
			while(*ch=='0') 
			{ 
				*ch=0; 
				ch--; 
			} 
			if(*ch=='.') 
				*ch=0; 
		} 
		sprintf(str,"%.2f%%=%s",100.0*value[i],summa); 
		term->dps(str); 
		if(*dot[i]) 
			term->Show_Image(4,5+i*2,dot[i],name); 
	} 
	hot_line("F10 exit "); 
	term->scrbufout(); 
	for(i=0;i<HIGH && i<num_mark_fields;i++) 
	{ 
		if(circle) 
		{ 
			int j=(int)(360*value[i]); 
			if(j<0) j=-j; 
			if(!j) 
				j=1; 
			x+=j; 
			if((i==num_mark_fields-1) && x<360) 
				j+=(360-x); 
			term->DrawArc(j,chart_color[i%10]); 
		} 
		else 
			term->DrawRectangle(chart_color[i%10],26+i*((term->l_x()-29)/num_mark_fields),4+HIGH+shift,(term->l_x()-30)/num_mark_fields,value[i]*(HIGH-1)*zoom,1); 
	} 
	term->dpi(); 
	if(ch!=NULL) 
		free(ch); 
	free(value); 
 
	for(i=0;i<HIGH && i<num_mark_fields;i++) 
		term->Del_Image(4,5+i*2); 
 
	num_mark_fields=0; 
	term->Del_Rectangles(); 
	term->Set_Dimension(0,0,0,0); 
	term->restore_box(f); 
	term->free_box(f); 
	term->BlackWhite(0,0,term->l_x(),term->l_y()); 
	term->MultiColor(x0,y0,l,h+1); 
	return(0);
} 
 
static int _rep=0; 
static int last_num; 
 
void Del_Chart() 
{ 
	term->Del_All_Objects(POLYGON); 
	term->Del_All_Objects(POLYLINE); 
	term->Del_All_Objects(LINE);
	term->Del_Rectangles();
 
	_rep=0; 
	last_num=0; 
} 
 
/* 
	atr==0 independent scales 
	atr==1 common scale 
	atr==2 intensity graph 
	atr==3 bar graph 
*/ 
 
int CX_BROWSER::Draw_Chart(int atr) 
{ 
	float *value=NULL,*max=NULL,*min=NULL,div,poz_x,a,*val; 
	int x,y,n; 
	int step=1,i,j,*int_val,f,size; 
	long page=0; 
	char str[LINESIZE],line[LINESIZE],*ch=NULL; 
	int mark[2],mark_poz; 
	int filtr_flg,poz_y,HIGH=20; 
	int show_poz=0,num_filtr,num_diff,num_int; 
	long tim; 
	int ind_flg; 
	int num_sel=0; 
	int sum_flg; 
	int num_val=0; 
	static int interval=1; 
	struct vp {long page; double value[10];} *vp=NULL;
	char *tag[10]; 
	bzero(tag,sizeof tag); 
 
	if(term->l_x()<64) 
		return(0); 
	if(interval>=db->max_index) 
		interval=1; 
	if(db->max_index<2) 
		goto EXIT; 
	page=index; 
	poz_y=10*term->font_H(); 
	HIGH=term->l_y()-7; 
	if(HIGH>20) 
		HIGH=20; 
	cx_cond|=CHRT; 

	if(atr==2) 
	{
		int n=num_mark_fields;
		if(num_mark_fields>1)
			num_mark_fields=1;
		Cmd_Exe(c_SortA); 
		num_mark_fields=n;
	}
	if(interval<1) 
		interval=1; 
	num_int=0; 
A1: 

	delete_menu(); 
	Del_Chart(); 
	sum_flg=0; 
	ind_flg=0; 
	mark[0]=mark[1]=0; 
	num_filtr=num_diff=0; 
	mark_poz=0; 
	filtr_flg=0; 
	if(num_mark_fields>9) 
		num_mark_fields=9; 

	if(!num_mark_fields) 
	{ 
		if((mark_field=(struct tag *)realloc(mark_field,(++num_mark_fields)*sizeof (struct tag)))==NULL) 
		{ 
			cx_cond&=~CHRT; 
			if(atr==2) 
				Cmd_Exe(c_RestIdx); 
			for(i=0;i<9;i++) 
				if(tag[i]!=NULL) 
					free(tag[i]); 
			return(-1); 
		} 
		bzero(mark_field+num_mark_fields-1,sizeof(struct tag)); 
		mark_field[0].des=tags[act_field].des; 
	} 
	if(atr==2) 
	{ 
		char *ch1=NULL,*ch2=NULL; 
		int counter=0; 

//                if(num_mark_fields>1)
		{
			vp=(struct vp *)realloc(vp,sizeof(struct vp));
			bzero(vp,sizeof(struct vp));
			num_val=1;
		}

		for(i=1;i<=db->max_index;i++) 
		{ 
			if(db->Check_Del(Record(i))) 
				continue; 
			Get_Slot(Record(i),mark_field,ch1);
			if(ch1==NULL) 
				continue; 

			if(strlen(ch1)>tags[act_field].des.l)
				ch1[tags[act_field].des.l]=0;

			if(ch2==NULL || strcmp(ch1,ch2)) 
			{ 
				vp=(struct vp *)realloc(vp,(++num_val)*sizeof(struct vp));

				bzero(vp+num_val-1,sizeof(struct vp));


				if(num_mark_fields>1)
				{
					vp[num_val-1].page=Record(i);
					for(int j=0;j<=num_mark_fields-1;j++)
					{
						vp[num_val-1].value[j]=db->Get_Value(vp[num_val-1].page,mark_field[j].des.sla);
					}
				}
				else
				{
					vp[num_val-1].page=Record(i-1);
					vp[num_val-1].value[0]=counter;
				}

				if(ch2) 
					free(ch2); 
				ch2=ch1; 
				ch1=NULL; 
				counter=0; 
			} 
			else if(num_mark_fields>1)
			{
				for(int j=0;j<=num_mark_fields-1;j++)
				{
					vp[num_val-1].value[j]+=db->Get_Value(Record(i),mark_field[j].des.sla);
				}
			}
			counter++; 
		} 
		if(ch1) 
			free(ch1); 
		if(ch2) 
			free(ch2); 
//                step=1;

		step=num_mark_fields;
//                num_val=db->max_index;


	} 
	else 
	{ 
		step=num_mark_fields; 
		num_val=db->max_index; 
	} 
	if(interval<=0) 
		interval=1; 
	num_val/=interval; 
	max=(float *)realloc(max,step*sizeof (float)); 
	min=(float *)realloc(min,step*sizeof (float)); 
	for(i=0;i<step;i++) 
	{ 
		max[i]= -1e32; 
		min[i]= +1e32; 
	} 
	size=(num_val>(term->l_x()-29)*term->font_W())?((term->l_x()-29)*term->font_W()):num_val; 
	if(value!=NULL) 
		free(value); 
	value=(float *)calloc(step*(size+1),sizeof (float)); 
	div=(float)num_val/((term->l_x()-29)*term->font_W()); 
	if(div<1.0) 
		div=1.0; 
	poz_x=0; 
 
	x=(term->l_x()-size/10)/2; 
	y=term->l_y()/2+1; 
	f=term->get_box(x,y,size/10+5,5); 
 
	term->BOX(x,y,size/10+3,3,' ',0,0xf,0,0xf); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	term->MultiColor(x,y,size/10+3,3); 
	term->dpp(x+1,y+1); 
	tim=time(0); 
	val=(float *)calloc(step,sizeof (float)); 
	n=0; 
	for(a=0,x=1,j=0;x<=num_val*(interval>1?interval:1) && j<size;x++) 
	{ 
		if(atr==2 && num_mark_fields<2)
		{
//                        for(int j=0;j<num_mark_fields;j++)
				val[0]=(float)vp[x].value[0];
		}
		else
		{ 
			if(db->Check_Del(Record(x))) 
			{ 
				x++; 
				size--; 
				j--; 
				continue; 
			} 
			for(i=0;i<step;i++) 
			{ 
/*
				Get_Slot(Record(x),mark_field+i,ch); 
				if(ch!=NULL) 
					val[i]+=atof(ch);
*/
				if(atr==2)
					val[i]+=(float)vp[x].value[i];
				else
					val[i]+=db->Get_Value(Record(x),mark_field[i].des.sla);

				n++; 
			} 
		} 
		if(n<1) 
			n=1; 
		if(interval>1) 
		{ 
			if(x%interval) 
				continue; 
		} 
		term->Set_Color(016,0); 
		if(x-a>=div) 
		{ 
			a+=div; 
			double b; 
			for(b=0,i=0;i<step;i++) 
			{ 
				double a; 
 
				if(num_int) 
				{ 
					a=val[i]; 
					a+=j>0?value[i+(j-1)*step]:0; 
				} 
				else 
				{ 
					a=val[i]/(double)((n!=step)?n:step); 
					a*=(double)step; 
					a*=interval; 
				} 
				value[i+j*step]=a; 

				if(atr<3) 
				{ 
					if(a>max[i]) 
						max[i]=a; 
					if(a<min[i]) 
						min[i]=a; 
				} 
				else    b+=a; 
				val[i]=0; 
			} 
			if(atr>=3) 
			{ 
				if(b>max[0]) 
					max[0]=b; 
				if(b<min[0]) 
					min[0]=b; 
			} 
			j++; 
			if(!(j%10)) 
			{ 
				term->dpo(' '); 
				if(ind_flg) 
					term->scrbufout(); 
			} 
			n=0; 
		} 
		if(ind_flg==0 && time(0)>tim) 
			ind_flg=1; 
	} 
	free(val); 
	term->restore_box(f); 
	term->free_box(f); 
//        term->zero_box(25,4,(term->l_x()-29),HIGH);
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
FILTR: 
	Load_Menu(ICONSDEF,3); 
	if(num_mark_fields<2) 
	{ 
		*images[6].name=0; 
	} 
	if(!(db->cx_cond&SORT)) 
	{ 
		*images[7].name=0; 
		*images[8].name=0; 
	} 
	show_menu(); 
 
	Del_Chart(); 
	while(filtr_flg)
	{ 
		for(i=0;i<step;i++) 
			filtr(value,size,step,i); 
DIFF: 
		for(i=0;i<step;i++) 
		{ 
			max[i]= -1e32; 
			min[i]= +1e32; 
		} 
		double a; 
		for(j=0;j<size;j++) 
		{ 
			for(a=0,i=0;i<step;i++) 
			{ 
				if(atr>=3) 
				{ 
					a+=value[i+j*step]; 
				} 
				else 
				{ 
					if(value[i+j*step]>max[i]) 
						max[i]=value[i+j*step]; 
					if(value[i+j*step]<min[i]) 
						min[i]=value[i+j*step]; 
				} 
			} 
			if(atr>=3) 
			{ 
				if(a>max[0]) 
					max[0]=a; 
				if(a<min[0]) 
					min[0]=a; 
			} 
		} 
		if(filtr_flg) 
			filtr_flg--; 
	} 
	if(atr==1) /* abs value */
	{ 
		float MAX=-1e32; 
		float MIN= 1e32; 
 
		for(i=0;i<step;i++) 
		{ 
			if(max[i]>MAX) 
				MAX=max[i]; 
			if(min[i]<MIN) 
				MIN=min[i]; 
		} 
		for(i=0;i<step;i++) 
		{ 
			max[i]=MAX; 
			min[i]=MIN; 
		} 
	} 
	int_val=(int *)calloc(size+1,sizeof (int)); 
	if(page<=0) 
		page=1; 
	if(page>num_val) 
		page=num_val; 
	f=term->get_box(0,0,term->l_x(),term->l_y()); 
	draw_gbox(x0,l,HIGH,mark,num_val,term->l_x()); 
	term->scrbufout(); 
DRAW: 
	help_line(14); 
	if(atr>=3) 
	{ 
		char *name; 
		int ll=term->font_W(); 
		int hh=term->font_H(); 

		int *X=(int *)malloc(step*(size+2)*sizeof (int));
		int *Y=(int *)malloc(step*(size+2)*sizeof (int));
		for(i=0;i<step;i++)
		{
			X[i*(size+2)]=25*ll;
			Y[i*(size+2)]=(5+HIGH)*hh;
			for(j=0;j<size;j++)
			{
				int x=25*ll+ll*j*(term->l_x()-29)/size;
				int y=0;
				float a=value[i+j*step];

				if(max[0]==min[0])
					a=HIGH*hh/2;
				else
				{
					double scale=max[0];
					if(scale<0) scale=-scale;
					if(scale==0)
						scale=1;
					if(min[0]<0 && max[0]>0)
					{
						scale+=-min[0];
						y=(int)(-(HIGH*hh*min[0])/scale);
					}
					a*=HIGH*hh/scale;
				}
				int_val[0]=(int)a;
				if(size<250)
				{
					term->DrawRectangle(chart_color[i%10],x,(5+HIGH)*hh-y,50*ll/size,(double)int_val[0],2);
				}
				X[i*(size+2)+j+1]=x;
				Y[i*(size+2)+j+1]=(int)a;
				y+=int_val[0]+1;
			}
		}
		for(j=0;j<size;j++)
		{
			int a=0;
			for(i=0;i<step;i++)
			{
				a+=Y[i*(size+2)+j+1];
				Y[i*(size+2)+j+1]=(5+HIGH)*hh-a;
			}
		}
		for(i=step-1;i>=0;i--)
		{
			X[i*(size+2)+size+1]=X[i*(size+2)+size];
			Y[i*(size+2)+size+1]=Y[i*(size+2)];
			if(size>=250)
				term->Put_Polygon(X+i*(size+2),Y+i*(size+2),size+2,chart_color[i%10],1);
			if((name=mark_field[i].name)==NULL)
				name=db->Name_Field(mark_field[i].des.sla);
			if(*dot[i])
				term->Show_Image(4,5+i*2,dot[i],name);
		}
		set_reper_coord(25,4,(term->l_x()-29),HIGH+1);
		free(X);
		free(Y);
	} 
	else for(i=0;i<step;i++) 
	{ 
		int j; 
		char *name; 
		int color=chart_color[i%10]; 
 
		for(j=0;j<size;j++) 
		{ 
			float a=value[i+j*step]; 
 
			if(max[i]==min[i]) 
				a=HIGH*term->font_H()/2; 
			else 
			{ 
				a*=HIGH*term->font_H()/(max[i]-min[i]); 
				a-=min[i]*HIGH*term->font_H()/(max[i]-min[i]); 
			} 
			int_val[j]=(int)a; 
		} 
		Chart(int_val,size,25,4,(term->l_x()-29),HIGH+1,color,i+1); 
		if((name=mark_field[i].name)==NULL) 
			name=db->Name_Field(mark_field[i].des.sla); 
		if(*dot[i]) 
			term->Show_Image(4,5+i*2,dot[i],name); 
	} 
A2: 
	term->restore_box(f); 
 
	draw_gbox(x0,l,HIGH,mark,num_val,term->l_x()); 
	set_reper_coord(25,4,(term->l_x()-29),HIGH+1); 
 
	if(db->cx_cond&SORT) 
		sprintf(str," Record: %d->%d from: %d(%d) ",(int)page,(atr==2?(int)vp[page].page:(int)Record(page)),num_val,db->insert); 
	else 
		sprintf(str," Record: %d from: %d ",(int)page,num_val); 
	term->Set_Color(0,15); 
	term->dpp(25,3); 
	term->dps(str); 
	*str=0; 
	if(sum_flg) 
	{ 
		term->dpp(5,HIGH+5); 
		if(sum_flg>0) 
			term->dps("Sum"); 
		else    term->dps("Diff"); 
	} 
	if(interval>1) 
		sprintf(str," Period=%d ",interval); 
	if(num_filtr) 
	{ 
		sprintf(line," Filter=%d ",num_filtr); 
		strcat(str,line); 
	} 
	if(num_diff) 
	{ 
		sprintf(line," Derivative=%d ",num_diff); 
		strcat(str,line); 
	} 
	if(num_int) 
	{ 
		sprintf(line," Integral=%d ",num_int); 
		strcat(str,line); 
	} 
	term->Set_Color(23,15); 
	if(*str) 
	{ 
		term->dpp(27,HIGH+5); 
		term->dps(str); 
	} 
	if(num_sel) 
	{ 
		sprintf(str,"%d",num_sel); 
		term->dpp(term->l_x()-5,3); 
		term->dps(str); 
	} 
	if(atr==2) 
		db->Get_Slot(vp[page].page,1,ch); 
	else 
		db->Get_Slot(Record(page*interval),1,ch); 
	strncpy(line,ch,20); 
	line[20]=0; 
	term->dpp(4,3); 
	term->dps(line); 
 
	for(i=0;i<step;i++) 
	{ 
		char *name; 
		int color=chart_color[i%10]; 
		if(!dot[i]) 
			break; 
		if(atr==2) 
		{ 
			char st[32],str[256]; 
 
			Get_Slot(vp[page].page,mark_field,ch);
			if(ch!=NULL) 
				strncpy(str,ch,16); 
			else    *str=0; 

			if(step==1)
				sprintf(st,"/%.0f",vp[page].value[i]);
			else
				sprintf(st,"/%f",vp[page].value[i]);
			char *ch=strchr(st,'.');
			if(ch!=NULL)
			{
				int j=strlen(st);
				while(st[j-1]=='0')
				{
					st[j-1]=0;
					j--;
				}
				if(st[j-1]=='.')
					st[j-1]=0;
			}

			strcat(str,st); 
			sprintf(line,"%.17s",str); 
		} 
		else 
		{ 
			Get_Slot(Record(page*interval),mark_field+i,ch); 
			sprintf(line,"%.17s",ch); 
		} 
/* 
		if(mark_poz>1) 
		{ 
			float a=get_diff(mark_ch,value,size,step,i); 
			sprintf(line,"(%+.1f%%)",a); 
			strcat(ch,line); 
		} 
*/ 
		if((name=mark_field[i].name)==NULL) 
			name=db->Name_Field(mark_field[i].des.sla); 
		if(name!=NULL) 
		{ 
			strncpy(str,name,18); 
			str[18]=0; 
		} 
		else    *str=0; 
		term->dpp(6,5+i*2); 
		term->Set_Color(23,color); 
		term->dps(str); 
		term->dpp(6,6+i*2); 
		term->Set_Color(0x200+23,color); 
		term->dps(line); 
		if(i==show_poz) 
		{ 
			if(atr==3) 
			{ 
				if(max[0]>0 && min[0]<0) 
					sprintf(line,"%17.2f",max[0]-((max[0]-min[0])*(poz_y))/(HIGH*term->font_H())); 
				else 
					sprintf(line,"%17.2f",max[0]-((max[0])*(poz_y))/(HIGH*term->font_H())); 
			} 
			else 
				sprintf(line,"%17.2f",max[i]-((max[i]-min[i])*(poz_y))/(HIGH*term->font_H())); 
			term->dpp(5,4+HIGH); 
			term->dps(line); 
		} 
	} 
	poz_x=(page-1)*(((term->l_x()-29)*term->font_W())/(float)(num_val-1)); 
	Reper_Up(term->font_W()*25+(int)poz_x,term->font_H()*5+poz_y); 
A3: 
	i=Xmouse(term->dpi()); 
BEG: 
	switch(i) 
	{ 
	case 0: 
		if(term->ev().x>term->l_x()) 
		{ 
			switch(i=get_menu_cmd(term->ev().x,term->ev().y,term->ev().b)) 
			{ 
				case c_GoNext: 
					i=CR; 
					goto BEG; 
				case c_GoPrev: 
					i=CL; 
					goto BEG; 
				case c_RestIdx: 
					if(num_sel) 
					{ 
						Cmd_Exe(c_RestIdx); 
						num_sel--; 
					} 
					free(int_val); 
//                                        term->zero_box(25,4,(term->l_x()-29),HIGH); 
					term->restore_box(f); 
					term->free_box(f); 
					goto A1; 
				case c_Chart: 
				case c_Array: 
					if(i==c_Chart && atr==3) 
					{ 
						if(num_mark_fields>1) 
						{ 
							Del_Chart(); 
							atr=!dial(message(11),1); 
						} 
						else    atr=0; 
					} 
					else if(i==c_Array && (atr==1 || atr==0)) 
						atr=3; 
					i=IS; 
					goto BEG; 
				case c_Bar: 
				case c_Pie: 
				{ 
					Del_Chart(); 
					for(j=0;j<HIGH && j<step;j++) 
						term->Del_Image(4,5+j*2); 
					term->restore_box(f); 
					term->free_box(f); 
					f=-1; 
 
					term->scrbufout(); 
					int num_mark_fields_std=num_mark_fields; 
					struct tag *mark_field_std=(struct tag *)malloc(num_mark_fields*sizeof (struct tag)); 
					bcopy(mark_field,mark_field_std,num_mark_fields*sizeof (struct tag)); 
 
					delete_menu(); 
					Cmd_Exe(i); 
 
					num_mark_fields=num_mark_fields_std; 
					mark_field=(struct tag *)realloc(mark_field,num_mark_fields*sizeof (struct tag)); 
					bcopy(mark_field_std,mark_field,num_mark_fields*sizeof (struct tag)); 
					free(mark_field_std); 
 
					term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
					goto FILTR; 
				} 
				case c_Save: 
					i='\r'; 
					goto BEG; 
				case 11:        // Mark 
					i=F2; 
					goto BEG; 
				case c_Exit: 
					i=F10; 
					goto BEG; 
				case c_Stack: 
					for(i=0;i<HIGH && i<step;i++) 
						term->Del_Image(4,5+i*2); 
					Del_Chart(); 
					term->restore_box(f); 
					term->free_box(f); 
					f=-1; 
					term->scrbufout(); 
					delete_menu(); 
					Cmd_Exe(c_Stack); 
 
					goto FILTR; 
				default: 
					goto A3; 
			} 
		} 
		if(term->ev().x>=3 && term->ev().x<=term->l_x()-5 && term->ev().y>=4 && term->ev().y<HIGH+5) 
		{ 
			if(term->ev().b!=1) 
			{ 
				i='\r'; 
				goto BEG; 
			} 
 
			if(term->ev().x<24 && (term->ev().y-5)/2<=step) 
				show_poz=(term->ev().y-5)/2; 
			else 
			{ 
				if(term->ev().x>=24) 
					page=(num_val*(term->ev().x-24))/(term->l_x()-29)+1; 
				if(page>num_val) 
					page=num_val; 
				poz_y=(term->ev().y-4)*term->font_H(); 
				if(mark_poz) 
				{ 
					if(mark[0]<page) 
						mark[1]=page; 
					else 
					{ 
						mark[1]=mark[0]; 
						mark[0]=page; 
					} 
					mark_poz=2; 
				} 
			} 
 
		} 
		goto A2; 
	case '+': 
		if(step>1) 
		{ 
			max[0]= -1e32; 
			min[0]= +1e32; 
			for(j=0;j<size;j++) 
			{ 
				double a=0; 
				for(n=0;n<step;n++) 
					a+=value[n+j*step]; 
				value[j]=a; 
				if(value[j]>max[0]) 
					max[0]=value[j]; 
				if(value[j]<min[0]) 
					min[0]=value[j]; 
			} 
			sum_flg=1; 
			for(i=0;i<HIGH && i<step;i++) 
				term->Del_Image(4,5+i*2); 
			step=1; 
			goto FILTR; 
		} 
		goto DRAW; 
	case '-': 
		if(step==2) 
		{ 
			max[0]= -1e32; 
			min[0]= +1e32; 
			for(j=0;j<size;j++) 
			{ 
				value[j]=value[2*j]-value[2*j+1]; 
				if(value[j]>max[0]) 
					max[0]=value[j]; 
				if(value[j]<min[0]) 
					min[0]=value[j]; 
			} 
			sum_flg=-1; 
			for(i=0;i<HIGH && i<step;i++) 
				term->Del_Image(4,5+i*2); 
			step=1; 
			goto FILTR; 
		} 
		goto DRAW; 
	case '\r': 
		Go_To_Index(page); 
		break; 
	case CL: 
	case CU: 
		if(i==CL) 
		{ 
			if(page<=1) 
				goto A3; 
			if(mark_poz) 
			{ 
				if(mark[0]==page) 
					mark[0]--; 
				else if(mark[1]==page) 
					mark[1]--; 
			} 
			page--; 
		} 
		else 
			if(poz_y) 
				poz_y--; 
		goto A2; 
	case CR: 
	case CD: 
		if(i==CR) 
		{ 
			if(page>=num_val) 
				goto A3; 
			if(mark_poz) 
			{ 
				if(mark[0]==page) 
					mark[0]++; 
				else if(mark[1]==page) 
					mark[1]++; 
			} 
			page++; 
		} 
		else if(poz_y<(HIGH+1)*term->font_H()) 
			poz_y++; 
 
		goto A2; 
	case F2: 
		if(mark_poz>1) 
		{ 
			mark_poz=0; 
			mark[0]=mark[1]=0; 
			goto DRAW; 
		} 
		mark[0]=page; 
		mark[1]=num_val; 
		mark_poz=1; 
		goto DRAW; 
	case F3: 
		for(i=0;i<step;i++) 
			diff(value,size,step,i); 
		num_diff++; 
		free(int_val); 
//                term->zero_box(25,4,(term->l_x()-29),HIGH); 
		Del_Chart(); 
		term->restore_box(f); 
		term->free_box(f); 
		goto DIFF; 
	case F4: 
		if(num_int) 
			for(i=0;i<step;i++) 
				integral(value,size,step,i); 
		num_int++; 
		free(int_val); 
//                term->zero_box(25,4,(term->l_x()-29),HIGH); 
		Del_Chart(); 
		term->restore_box(f); 
		term->free_box(f); 
		if(num_int==1) 
			goto A1; 
		goto DIFF; 
	case F5: 
		filtr_flg=3; 
		num_filtr++; 
		free(int_val); 
//                term->zero_box(25,4,(term->l_x()-29),HIGH); 
		term->restore_box(f); 
		term->free_box(f); 
		goto FILTR; 
	case IS: 
	case F6: 
		if(mark_poz && atr!=2) 
		{ 
			char str[64]; 
			struct query query; 
 
			sprintf(str,"[%d-%d]",mark[0],mark[1]); 
			query.str=str; 
			memcpy(query.sla,tags[act_field].des.sla,sizeof query.sla); 
			Query(&query); 
 
			num_sel++; 
			Go_To_Index(page); 
			if(db->share!=NULL) 
				*db->share->output=0; 
		} 
		for(i=0;i<HIGH && i<step;i++) 
			term->Del_Image(4,5+i*2); 
		Del_Chart(); 
		term->restore_box(f); 
		term->free_box(f); 
		term->scrbufout(); 
		goto A1; 
	case F7: 
		term->cursor_visible(); 
		Del_Chart(); 
		for(i=0;i<HIGH && i<step;i++) 
			term->Del_Image(4,5+i*2); 
		interval=dial(message(65),3); 
		term->cursor_invisible(); 
		term->restore_box(f); 
		term->free_box(f); 
		term->scrbufout(); 
		goto A1; 
	case F10: 
		break; 
	default: 
		goto DRAW; 
	} 
	term->restore_box(f); 
	term->free_box(f); 
	free(int_val); 
	free(value); 
	free(max); 
	free(min); 
	free(mark_field); 
EXIT: 
 
	for(i=0;i<HIGH && i<step;i++) 
		term->Del_Image(4,5+i*2); 
	mark_field=NULL; 
	num_mark_fields=0; 
	Del_Chart(); 
	if(ch!=NULL) 
		free(ch); 
	if(vp!=NULL) 
		free(vp); 
	cx_cond&=~CHRT; 
	term->BlackWhite(0,0,term->l_x(),term->l_y()); 
	term->MultiColor(x0,y0,l,h+1); 
	if(atr==2) 
		Cmd_Exe(c_RestIdx); 
	delete_menu(); 
	for(i=0;i<9;i++) 
		if(tag[i]!=NULL) 
			free(tag[i]); 
	return(0); 
} 
 
int CX_BROWSER::Intensity_Graph() 
{ 
	return(Draw_Chart(2)); 
} 
 
static int reper_x,reper_y; 
static int xx,yy,ll,hh; 
 
void CX_BROWSER::Chart(int *value,int num,int x,int y,int l,int h,int color,int num_chart) 
{ 
	int i; 
 
	if(term->Color()<64 || num<3) 
		return; 
	set_reper_coord(x,y,l,h); 
	int *xcoord = (int *)malloc(num*sizeof (int)); 
	int *ycoord = (int *)malloc(num*sizeof (int)); 
 
	for(i=0;i<num;i++) 
	{ 
		xcoord[i]=xx+i*ll/(num-1); 
		ycoord[i]=yy+hh-value[i]; 
	} 
	if(num_chart==last_num+1) 
	{ 
		term->Put_Polygon(xcoord,ycoord,num,color); 
		last_num++; 
	} 
	free(xcoord); 
	free(ycoord); 
} 
 
 
void CX_BROWSER::Reper_Down() 
{ 
	if(!_rep) 
		return; 
 
	term->Del_Last_Object(LINE); 
	term->Del_Last_Object(LINE); 
	_rep=0; 
} 
 
void CX_BROWSER::set_reper_coord(int x,int y,int l,int h) 
{ 
	xx=x*term->font_W(); 
	ll=l*term->font_W(); 
	yy=y*term->font_H(); 
	hh=h*term->font_H(); 
} 
 
void CX_BROWSER::Reper_Up(int x, int y) 
{ 
	if(term->Color()<64) 
		return; 
	if(_rep) 
		Reper_Down(); 
	reper_x=x; 
	reper_y=y; 
	term->Put_Line(reper_x,yy+term->font_H()/2,reper_x,yy+hh,7); 
	term->Put_Line(xx,reper_y,xx+ll,reper_y,7); 
	_rep=1; 
} 
