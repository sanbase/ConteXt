#ifndef _CX3_H
#define _CX3_H

#include "CX_Browser.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

struct xy
{
	unsigned short n; /* номер поля                */
	unsigned short m; /* номер поля ссылки         */
	unsigned short t; /* номер поля ссылки ссылки  */
	unsigned short e; /* элемент массива           */
	unsigned short f; /* элемент массива массива   */
	unsigned short x; /* координата x              */
	unsigned short y; /* координата y              */
	unsigned short a; /* атрибут поля              */
	unsigned char  l; /* длина поля                */
	char           p; /* смещение кадра от текущего*/
};

struct father
{
	char name[64];
	long page;
	short field;
};

struct cx3share
{
	unsigned short ret;
	unsigned short pt;
	long page;
	char mess[256-19];
	long ppage;
	char ind_name[16];
	struct father sour;
};

class CX3
{
private:
	char *cadr;
	struct cx3share *shbuf;
	int shmid;
	int len;        // size of cadr
	int length;     // size of shared menory
	struct cx_share *share;
	CX_BROWSER *cx5;
	pid_t use_show;
public:
	CX3(CX_BROWSER *);
	~CX3();
	int Get_Show(long record,struct tag_descriptor *des,char *&str);
};

#endif
