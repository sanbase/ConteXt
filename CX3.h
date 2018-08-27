#ifndef _CX3_H
#define _CX3_H

#include "CX_Browser.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

struct xy
{
	unsigned short n; /* ����� ����                */
	unsigned short m; /* ����� ���� ��뫪�         */
	unsigned short t; /* ����� ���� ��뫪� ��뫪�  */
	unsigned short e; /* ����� ���ᨢ�           */
	unsigned short f; /* ����� ���ᨢ� ���ᨢ�   */
	unsigned short x; /* ���न��� x              */
	unsigned short y; /* ���न��� y              */
	unsigned short a; /* ��ਡ�� ����              */
	unsigned char  l; /* ����� ����                */
	char           p; /* ᬥ饭�� ���� �� ⥪�饣�*/
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
