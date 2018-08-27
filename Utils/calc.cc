#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tty_codes.h>
double calculation(char *str)
{
	double rez;
	char *beg,*end;
	char *out=(char *)malloc(strlen(str)+1);
	strcpy(out,str);
B1:
	beg=NULL;
	for(int i=0;out[i];i++)
	{
		if(out[i]=='*' || out[i]=='/')
		{
			beg=out+i;
			char op=*beg;

			for(i=beg-out-1;i>=0;i--)
				if(out[i]=='+' || out[i]=='-' || out[i]=='*' || out[i]=='/')
					break;
			rez=atof(out+i+1);
			out[i+1]=0;
			if(op=='*')
				rez*=atof(beg+1);
			else    rez/=atof(beg+1);
			for(i=beg-out+1;out[i];i++)
				if(out[i]=='+' || out[i]=='-' || out[i]=='*' || out[i]=='/')
					break;
			char *out1=(char *)malloc(strlen(out)+32+strlen(out+i));
			sprintf(out1,"%s%f%s",out,rez,out+i);
			out=(char *)realloc(out,strlen(out1)+1);
			strcpy(out,out1);
			free(out1);
			goto B1;
		}
	}
	rez=atof(out);
	beg=out;
	while(*beg=='-' || *beg=='+')
		beg++;
BEG:
	while(*beg && *beg!='+' && *beg!='-')
		beg++;
	switch(*beg)
	{
		case '+':
			rez+=atof(beg+1);
			break;
		case '-':
			rez-=atof(beg+1);
			break;
		default:
			free(out);
			return(rez);
	}
	beg++;
	goto BEG;
}


double Calc(char *str)
{
	char *beg,*end;
	double rez=0;
	int i;
	char *out=(char *)malloc(strlen(str)+1);
	strcpy(out,str);

	beg=out;
	while((beg=strchr(beg,' '))!=NULL)
	{
		bcopy(beg+1,beg,strlen(beg));
		beg++;
	}
BEG:
	if((beg=strrchr(out,'('))!=NULL)
	{
		if((end=strchr(beg,')'))==NULL)
			return(0);      /* нарушен балланс скобок */
		*end=0;
		*beg=0;
		for(i=1;beg[i];i++)
			if(beg[i]=='+' || beg[i]=='-' || beg[i]=='*' || beg[i]=='/')
				break;
		if(beg[i]==0)   /* что-то типа ((3.5)) */
		{
			bcopy(beg+1,beg,strlen(beg+1));
			strcat(out,end+1);
			goto BEG;
		}
		rez=calculation(beg+1);
		char *out1=(char *)malloc(strlen(out)+strlen(end+1)+32);
		sprintf(out1,"%s%f%s",out,rez,end+1);
		out=(char *)realloc(out,strlen(out1)+1);
		strcpy(out,out1);
		free(out1);
		goto BEG;
	}
	rez=calculation(out);
	free(out);
	return(rez);
}

main()
{
	int ret;
	char *str;
	double rez=0;

	str=(char *)malloc(128);
	dpbeg("");
	dpp(0,0);
	Set_Color(01,03);
	dpo(es);
	Set_Color(0,7);
	*str=0;
	BOX(20,10,29,11,' ',0x69,0x69);
	goriz_s(20,12,28);
	Set_Color(0x100+30,14);

	dpp(22,13); dps(" 7 "); dpp(36,13); dps(" + ");
	dpp(26,13); dps(" 8 "); dpp(36,15); dps(" - ");
	dpp(30,13); dps(" 9 "); dpp(36,17); dps(" * ");
	dpp(22,15); dps(" 4 "); dpp(36,19); dps(" / ");
	dpp(26,15); dps(" 5 "); dpp(40,13); dps(" ( ");
	dpp(30,15); dps(" 6 "); dpp(40,15); dps(" ) ");
	dpp(22,17); dps(" 1 "); dpp(40,17); dps(" = ");
	dpp(26,17); dps(" 2 "); dpp(40,19); dps(" C ");
	dpp(30,17); dps(" 3 ");
	dpp(22,19); dps(" 0 ");
	dpp(26,19); dps(" . ");
	dpp(30,19); dps("Del");

	Set_Color(0x100+12,14);
	dpp(44,13); dps("Off");
	Set_Color(0x108,016);
BEGIN:
	if((ret=edit(0,str,128,25,22,11,0))==F10)
		goto EXIT;
	if(ret==F8)
	{
		*str=0;
		goto BEGIN;
	}
	rez=Calc(str);
	sprintf(str,"%f",rez);

	if(strchr(str,'.')!=NULL)
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
	if(atof(str)==0)
		strcpy(str,"0");

	goto BEGIN;
EXIT:
	dpend();
	free(str);
	exit(0);
}

extern struct mouse ev;

int Xmouse(int i)
{
	if(i==0)
	{
		if(ev.y==13)
		{
			if(ev.x>=22 && ev.x<=24)
				return('7');
			if(ev.x>=26 && ev.x<=28)
				return('8');
			if(ev.x>=30 && ev.x<=32)
				return('9');
			if(ev.x>=36 && ev.x<=38)
				return('+');
			if(ev.x>=40 && ev.x<=42)
				return('(');
			if(ev.x>=44 && ev.x<=46)
				return(F10);
		}
		if(ev.y==15)
		{
			if(ev.x>=22 && ev.x<=24)
				return('4');
			if(ev.x>=26 && ev.x<=28)
				return('5');
			if(ev.x>=30 && ev.x<=32)
				return('6');
			if(ev.x>=36 && ev.x<=38)
				return('-');
			if(ev.x>=40 && ev.x<=42)
				return(')');
		}
		if(ev.y==17)
		{
			if(ev.x>=22 && ev.x<=24)
				return('1');
			if(ev.x>=26 && ev.x<=28)
				return('2');
			if(ev.x>=30 && ev.x<=32)
				return('3');
			if(ev.x>=36 && ev.x<=38)
				return('*');
			if(ev.x>=40 && ev.x<=42)
				return(rn);
		}
		if(ev.y==19)
		{
			if(ev.x>=22 && ev.x<=24)
				return('0');
			if(ev.x>=26 && ev.x<=28)
				return('.');
			if(ev.x>=30 && ev.x<=32)
				return(dc);
			if(ev.x>=36 && ev.x<=38)
				return('/');
			if(ev.x>=40 && ev.x<=42)
				return(F8);
		}
	}
	return(i);
}
