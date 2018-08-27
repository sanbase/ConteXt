#include "StdAfx.h" 
 
#include <stdio.h>
#include <sys/types.h>
#include "tty_codes.h"
#include <stdlib.h>
#include <string.h>

struct area {unsigned char x,y,h,l; union fpic ar[1];};

static struct area *get_area(int x,int y,int l,int h);
static int put_area(int x,int y,struct area *are);
static struct area **region;
static int num_reg=0;

/* Читает с экрана область с координатами x,y,l,h во внутренний массив */
/* Возвращает номер области или -1 при недостатке памяти
или свободных областей */
int get_box(int x,int y,int l,int h)
{
	int re;

	for(re=0;re<num_reg;re++) 
		if(region[re]==0) break;
	if(re==num_reg)
	{
		re=num_reg;
		region=(struct area  **)realloc(region,(++num_reg)*sizeof (struct area *));
	}
	if((region[re]=get_area(x,y,l,h))==NULL)
	{
		region[re]=0; return(-1);
	}
	return(re);
}
/* Освободить область номер n */
void free_box(int n)
{
	if(n<0 || n>=num_reg)
		return;
	if(region[n]!=0)
		free((void *)region[n]);
	region[n]=0;
}

int copy_box(int a, int b)
{
	if(region[a]->h!=region[b]->h || region[a]->l!=region[b]->l)
		return(-1);
	bcopy(region[a]->ar,region[b]->ar,region[a]->h*region[a]->l*(sizeof (union fpic)));
}

int restore_box(int n)
{
	if(n<0 || n>=num_reg)
		return(-1);
	if(region[n]==0)
		return(-1);
	put_area(region[n]->x,region[n]->y,region[n]);
	return(0);
}
/* Поместить область номер n на экран с координатами x,y  */
int put_box(int x,int y,int n)
{
	if(n<0 || n>=num_reg)
		return(-1);
	if(region[n]==0)
		return(-1);
	put_area(x,y,region[n]);
	return(0);
}
/* очистить область (только в памяти!) */
int clear_box(int x,int y,int n)
{
	register int i,j;
	union fpic cc;

	if(n<0 || n>=num_reg)
		return(-1);

	if(region[n]==0)
		return(-1);

	cc.f=0;
	cc.p.ch=' ';
	cc.p.clr.fg=03;

	for(i=0;i<region[n]->h;i++)
		for(j=0;j<region[n]->l;j++)
				put_f_old(x+j,y+i,cc);
	return(0);
}
int get_h(int n)
{
	if(region[n]==0)
		return(-1);
	return(region[n]->h);
}
int get_l(int n)
{
	if(region[n]==0)
		return(-1);
	return(region[n]->l);
}
struct area *get_reg(int n)
{
	static struct area a;
	if(n>=0)
		return(region[n]);
	bzero(&a,sizeof a);
	return(&a);
}
static struct area *get_area(int x,int y,int l,int h)
{
	register int i,j;
	struct area *buf;

	if(l>t.param.xdim)
		l=t.param.xdim;
	if(y>t.param.ydim)
		y=t.param.ydim;
	if((buf=(struct area *)malloc(sizeof (struct area)+(l*h*(sizeof (union fpic)))))==NULL)
		return(NULL);
	for(i=0;i<h;i++)
		for(j=0;j<l;j++)
			buf->ar[i*l+j]=get_f(x+j,y+i);
	buf->h=h; buf->l=l;
	buf->x=x; buf->y=y;
	return(buf);
}
int superp(int n1,int n2)
{
	if(region[n1]==0 || region[n2]==0) return(-1);
	return(0);
}

static int put_area(int x,int y,struct area *are)
{
	register int i,j;

	for(i=0;i<are->h;i++)
		for(j=0;j<are->l;j++)
			put_f(x+j,y+i,are->ar[i*are->l+j]);
	return(0);
}

struct pics get_pics(int x, int y, int n)
{
	struct area *are=region[n];

	if(x>are->l || y>are->h)
	{
		struct pics pic;

		bzero(&pic,sizeof pic);
		return(pic);
	}
	return(are->ar[x+y*are->l].p);
}
void put_pics(int x, int y, int n, struct pics pics)
{
	struct area *are=region[n];

	if(x>are->l || y>are->h)
		return;
	are->ar[x+y*are->l].p=pics;
}
