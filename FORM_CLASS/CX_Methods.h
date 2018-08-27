#ifndef _CX_METHODS
#define _CX_METHODS
#ifdef WIN32

#ifdef METHODS_EXPORTS
#define METHODS_API __declspec(dllexport)
#else
#define METHODS_API __declspec(dllimport)
#endif

#include "CX_Browser.h"
#include <sys/wait.h>

class METHODS_API Methods
{
public:
	CX_BASE *base;

	Methods(CX_BASE *db);

	int  Write_Slot(char *);
	int  Write_Cadr();
	int  Action(int);
	void Abort();
	void Virtual(long record,struct sla *sla);
	void Virtual(long record,int slot);
	void Virtual(long record,char *slot);
	int Dial(char *str,int arg);
};

Methods::Methods(CX_BASE *db)
{
	bzero(this, sizeof this);
	base=db;
}

void Methods::Virtual(long record,int slot)
{
	struct sla sla[SLA_DEEP];
	bzero(sla,sizeof sla);
	sla->n=slot;
	Virtual(record,sla);
}

void Methods::Virtual(long record,char *slot)
{
	struct sla sla[SLA_DEEP];
	str_to_sla(slot,sla);
	Virtual(record,sla);
}

//----------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern METHODS_API Methods *Create(CX_BASE *db);
extern METHODS_API void Virtual(Methods *m,long record,struct sla *sla);
extern METHODS_API int  Write_Slot(Methods *m,char *);
extern METHODS_API int  Write_Cadr(Methods *m);
extern METHODS_API int  Action(Methods *m,int);
extern METHODS_API void Abort(Methods *m);

#ifdef __cplusplus
}
#endif

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserve)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

METHODS_API Methods *Create(CX_BASE *db)
{
	return new Methods(db);
}

METHODS_API void Virtual(Methods *m,long record,struct sla *sla)
{
	m->Virtual(record,sla);
}

METHODS_API int  Write_Slot(Methods *m,char *str)
{
	return(m->Write_Slot(str));
}

METHODS_API int  Write_Cadr(Methods *m)
{
	return(m->Write_Cadr());
}

METHODS_API int  Action(Methods *m,int act)
{
	return(m->Action(act));
}

METHODS_API void Abort(Methods *m)
{
	m->Abort();
}

METHODS_API void Dial(Methods *m, char *str,int arg)
{
	return(m->Dial(str,arg));
}

#else

#include "CX_Browser.h"
#include <setjmp.h>

Terminal *term=NULL;

static void usr1(int sig)
{
	if(sig==SIGHUP)
	{
		kill(SIGHUP,getppid());
		sleep(1);
		exit(0);
	}
};


static Methods *method;
static sigset_t mask;
static struct sigaction s;

class Methods
{
public:
	CX_BASE *base;
	Methods(char *name_base)
	{
		bzero(this, sizeof this);
		try
		{
			base = new CX_BASE(name_base);
		}
		catch(...)
		{
			return;
		}
	}
	~Methods()
	{
		if(base)
			delete base;
	}

	int  Write_Slot(char *);
	int  Write_Cadr();
	int  Action(int);
	void Abort();
	void Virtual(long record,struct sla *sla);

	void Virtual(long record,int slot)
	{
		struct sla sla[SLA_DEEP];
		bzero(sla,sizeof sla);
		sla->n=slot;
		Virtual(record,sla);
	}

	void Virtual(long record,char *slot)
	{
		struct sla sla[SLA_DEEP];
		str_to_sla(slot,sla);
		Virtual(record,sla);
	}
	int Dial(char *str,int arg)
	{
		strcpy(method->base->share->output,str);
		method->base->share->ret=arg;
		method->base->share->cmd=c_Dial;
		s.sa_handler=usr1;
		sigemptyset(&s.sa_mask);
		sigaction(SIGUSR1,&s,NULL);

		sigemptyset(&mask);
		sigaddset(&mask,SIGUSR1);
		sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL);

		if(write(1,"1",1)<0)
			return(0);

		sigemptyset(&mask);
		sigsuspend((sigset_t *)&mask);

		return(int(method->base->share->cmd));
	}
};


#ifndef sigmask
#define sigmask sigbit
#endif


static void abort(int sig)
{
	if(method!=NULL)
	{
		method->Abort();
		delete method;
	}
	exit(0);
}

static jmp_buf on_alrm;

void onalrm(int sig)
{
	longjmp(on_alrm,1);
}

main(int argc,char **argv)
{
	char line[LINESIZE];

	if(argc!=2)
		exit(0);
	signal(SIGHUP,usr1);
	try
	{
		method = new Methods(argv[0]);
	}
	catch(...)
	{
		write(1,"0",1);
		exit(0);
	}

	if(method->base->Map(argv[1])<0)
	{
		delete method;
		write(1,"0",1);
		exit(0);
	}

	s.sa_flags=SA_RESTART;
	s.sa_handler=usr1;
	sigemptyset(&s.sa_mask);
	sigaction(SIGUSR1,&s,NULL);

	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL);

	signal(SIGTERM,abort);
	signal(SIGKILL,abort);
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, onalrm );

	write(1,"1",1);

	for(;;)
	{
		int act=0;
		struct sla sla[SLA_DEEP];
		alarm(10);
		if(setjmp(on_alrm))
		{
			if(getppid()==1)
			{
				method->Abort();
				abort(0);
			}
		}
		sigemptyset(&mask);
		sigsuspend((sigset_t *)&mask);
		alarm(0);
		if(method->base->share==NULL)
			break;
		method->base->flush_sb();
		method->base->cadr_record=method->base->share->cadr_record;

		int i=method->base->share->cmd;

		method->base->share->cmd=0;
		switch(i)
		{
			case CHK_LINE:
				bcopy(method->base->share->output,line,LINESIZE);
				bzero(method->base->share->output,LINESIZE);
				if(!(method->base->share->ret=method->Write_Slot(line)))
					*method->base->share->output=0;
				else if(!strcmp(line,method->base->share->output))
					*method->base->share->output=0;
				break;
			case CHK_CADR:
				bzero(method->base->share->output,LINESIZE);
				method->base->share->ret=method->Write_Cadr();
				break;
			case CHK_ACT:
				bcopy(method->base->share->output,&act,sizeof act);
				bzero(method->base->share->output,LINESIZE);
				method->base->share->ret=method->Action(act);
				break;
			case VIRTUAL:
				bcopy(method->base->share->slot.sla,sla,sizeof sla);
				method->Virtual(method->base->share->record,sla);
				break;
			case NEWINDEX:
				method->base->Erase_Index(method->base->share->ret);
				method->base->Link_Index(method->base->share->output,method->base->share->ret);
				break;
		}
		method->base->Unlock(method->base->share->record);

		s.sa_handler=usr1;
		sigemptyset(&s.sa_mask);
		sigaction(SIGUSR1,&s,NULL);

		sigemptyset(&mask);
		sigaddset(&mask,SIGUSR1);
		sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL);
		if(write(1,"1",1)<0)
			break;
	}
	abort(0);
}
#endif

#define OUTPUT base->share->output
#define BG     base->share->color.bg
#define FG     base->share->color.fg
#define FNT    base->share->font.fnt
#define FATR   base->share->font.atr
#define RECORD base->share->record
#define RETURN base->share->ret
#define LEN    base->share->slot.l
#define TAG    base->share->slot
#define SLA    base->share->slot.sla
#define CMD    base->share->cmd
#define BLANK  base->share->form.blank

#endif
