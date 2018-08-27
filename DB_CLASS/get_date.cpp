/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:get_date.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#include <time.h> 
 
#ifdef  SHARED_LIB 
extern  int     (*_cxG_sscanf)(); 
#define sscanf  (*_cxG_sscanf) 
#endif 
 
static char  *dateformat = "%2d%*c%2d%*c%4d"; 
static int euro=-1; 
 
static void set_env() 
{ 
	char *ch=NULL;
	if(euro!=-1) 
		return; 
	euro=1; 
	if((ch=getenv("TZ"))==NULL) 
		return; 
	if(strstr(ch,"Canada")!=NULL || strstr(ch,"US")!=NULL) 
		euro=0; 
} 
long date_from_rec(char *ch) 
{ 
	unsigned char *a=NULL;
 
	if(ch==NULL) 
		return(0); 
	a=(unsigned char *)ch; 
	return(*a+(a[1]<<8)+(a[2]<<16)); 
} 
int month_length(int y,int m ) 
{ 
	static short days[] = 
	{ 
		31,28,31,30,31,30,31,31,30,31,30,31 
	}; 
	return ( y <= 0 || m <= 0 || m > 12 ) ? 0 : 
	days[m-1] + ( m == 2 && !(y&3) && ((y%100) || !(y%400)) ); 
} 
 
 
long link_date(int d,int m,int y ) 
{ 
	long j = 0L; 
	if( y > 0 && m > 0 && m <= 12 && d > 0 && d <= month_length( y, m ) ) 
	{ 
		if( m > 2 )  m -= 3;   
		else 
		{ 
			y--;   
			m += 9;  
		} 
		j = y/100; 
		j = (146097L*j)/4 + (1461L*(y-100*j))/4 + (153*m+2)/5 + d + 75; 
	} 
	return j; 
} 
 
 
int get_dmy(long date,int *day,int *month,int *year ) 
{ 
	int d = 0, m = 0, y = 0; 
	if( date >= 382 ) 
	{ 
		date -= 75; 
		y = (4*date - 1)/146097; 
		d = (4*date - 1 - 146097*y)/4; 
		date = (4*d + 3)/1461;           
		d = (4*d + 7 - 1461*date)/4; 
		m = (5*d - 3)/153;               
		d = (5*d + 2 - 153*m)/5; 
		y = 100*y + date; 
		if( m < 10 ) m += 3;   
		else 
		{ 
			y++;   
			m -= 9;  
		} 
	} 
	*year  = y; 
	*month = m;     
	*day   = d; 
 
	return (y); 
} 
 
 
void get_date(long dat,char *str) 
{ 
	int y=0, m=0, d=0; 
	if(str==NULL) 
		return; 
	dat&=0xFFFFFF; 
	if(get_dmy(dat, &d, &m, &y )) 
	{ 
		set_env(); 
		if(euro) 
			sprintf( str, "%02d.%02d.%04d", d, m, y ); 
		else 
			sprintf( str, "%02d/%02d/%04d", m, d, y ); 
	} 
	else 
		*str = 0; 
} 
 
long conv_date(char *str) 
{ 
	int d, m, y; 
	int e=euro; 
 
	if(str==NULL) 
		return(0); 
	set_env(); 
	if(str[2]=='.') 
		e=1; 
	if(str[2]=='/') 
		e=0; 
	if(!*str) 
		return(0); 
	if(e) 
		return (sscanf(str,dateformat,&d,&m,&y)!=3)?0:link_date(d,m,y); 
	else 
		return (sscanf(str,dateformat,&m,&d,&y)!=3)?0:link_date(d,m,y); 
} 
 
int check_date(char *str) 
{ 
	int d, m, y; 
 
	if(str==NULL) 
		return(0); 
	set_env(); 
	if(euro) 
		return  *str && ( sscanf( str, dateformat, &d, &m, &y ) != 3 || y <= 0 || 
		    m <= 0 || m > 12 || d <= 0 || d > month_length( y, m ) ); 
	else 
		return  *str && ( sscanf( str, dateformat, &m, &d, &y ) != 3 || y <= 0 || 
		    m <= 0 || m > 12 || d <= 0 || d > month_length( y, m ) ); 
} 
 
void get_time(long t,char *str) 
{ 
	int sek; 
	int min; 
	int hour; 
	unsigned long tim=t; 
 
	if(str==NULL) 
		return; 
	if(tim>1) 
		tim--; 
	else 
	{ 
		*str=0; 
		return; 
	} 
	hour=tim/3600; 
	tim-=hour*3600; 
	hour%=24; 
	min=tim/60; 
	tim-=min*60; 
	min%=60; 
	sek=tim%60; 
	sprintf(str,"%02d:%02d:%02d",hour,min,sek); 
} 
void get_unix_time(long tim,char *str) 
{ 
	char str1[64],str2[64]; 
 
	if(str==NULL) 
		return; 
	if(tim>1) 
		tim--; 
	else 
	{ 
		*str=0; 
		return; 
	} 
	get_date((tim/(60*60*24))+conv_date("01.01.1970"),str1); 
	get_time(tim%(60*60*24),str2); 
	sprintf(str,"%s %s",str1,str2); 
} 
long conv_time(char *str) 
{ 
	char buf[64]; 
	int tim; 
 
	if(str==NULL || !*str) 
		return(0); 
	while(*str==' ') 
		str++; 
	bzero(buf,sizeof buf); 
	if(strlen(str)<10) 
	{ 
		bcopy(str,buf,9); 
		buf[2]=0; buf[5]=0; buf[8]=0; 
		tim=atoi(buf); 
		tim*=60; 
		tim+=atoi(buf+3); 
		tim*=60; 
		tim+=atoi(buf+6); 
	} 
	else 
	{ 
		bcopy(str,buf,10); 
		buf[10]=0; 
		if((tim=(conv_date(buf)-conv_date("01.01.1970"))*24*60*60)<0) 
			return(0); 
		char *ch=strchr(str,' '); 
		if(ch!=NULL) 
			tim+=conv_time(ch+1); 
	} 
	return(tim+1); 
} 
 
long time(); 
 
char *local_time() 
{ 
	static char str[64]; 
	local_time(str); 
	return(str); 
} 
 
void local_time(char *str) 
{ 
	time_t  date=0l;
	struct tm *tm; 
 
	if(str==NULL) 
		return; 
	date=time(0); 
	tm=(struct tm *)localtime(&date); 
	if(euro) 
		sprintf(str,"%02d.%02d.%04d %02d:%02d:%02d",tm->tm_mday,tm->tm_mon+1,tm->tm_year+1900,tm->tm_hour,tm->tm_min,tm->tm_sec); 
	else 
		sprintf(str,"%02d/%02d/%04d %02d:%02d:%02d",tm->tm_mon+1,tm->tm_mday,tm->tm_year+1900,tm->tm_hour,tm->tm_min,tm->tm_sec); 
} 
 
long get_act_date() 
{ 
	char str[32]; 
 
	local_time(str); 
	return(conv_date(str)); 
} 
long get_act_time() 
{ 
	time_t date;
	struct tm *tm; 
 
	date=time(0); 
	tm=(struct tm *)localtime(&date); 
	return(tm->tm_sec+60*(tm->tm_min+60*(tm->tm_hour))); 
} 
