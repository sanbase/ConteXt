#ifdef METHODS_EXPORTS
#define METHODS_API __declspec(dllexport)
#else
#define METHODS_API __declspec(dllimport)
#endif

#include "CX_Browser.h"

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
