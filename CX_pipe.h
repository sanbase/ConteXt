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

typedef struct _shmdesc
{
	struct _shmdesc *next;
	int shmid;
	int cpid;
	int segsz;
	key_t key;
	mode_t mode;
	char *fname;
} shmdesc;

class CX3
{
private:
	char *cadr;
	struct cx3share *shbuf;
	int shmid;
	int len;        // size of cadr
	int length;     // size of shared memory
	shmdesc *shmchain;


	struct cx_share *share;
	CX_BROWSER *cx5;
	pid_t use_show;
	pid_t use_check;

	int shmctl( int shmid, int cmd, struct shmid_ds *buf);
	int shmget( key_t key, int size, int shmflg );
	void *shmat( int shmid, void *shmaddr, int shmflg );
	shmdesc *shmlookup( int shmid );

	pid_t Creat_pipe(key_t uid, char *name_base,short N,int len);
	int Connect(pid_t *pid,int sign);
	void del_proc(pid_t *process);
	void copy_cadr(long record);
	void restore_cadr(long record);
public:
	CX3(CX_BROWSER *);
	~CX3();
	int Get_Show(long record,struct tag *des,char *&str);
	int Check_Line(long record,struct tag *des,char *str);
	int Check_Cadr(long record,struct tag *des,int del);
};

#endif
