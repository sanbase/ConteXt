// santext.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include "context_dll.h"

BOOL APIENTRY DllMain( HANDLE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
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

CONTEXT_API CX_BASE *Open_Folder(char *name)
{
	try
	{
		return new CX_BASE(name);
	}
	catch (...)
	{
		return NULL;
	}
}

CONTEXT_API long New_Record(CX_BASE *db)
{
	return(db->New_Record());
}

CONTEXT_API int  Get_Slot (CX_BASE *db,long record, char *field,char &*str)
{
	return(db->Get_Slot(record,field,str));
}

CONTEXT_API int  Put_Slot (CX_BASE *db,long record, char *field, char *str)
{
	return(db->Put_Slot(record,field,str));
}

CONTEXT_API int  Get_Field(CX_BASE *db,long record,int field,char &*str)
{
	return(db->Get_Slot(db,record,field,str));
}

CONTEXT_API int  Put_Field(CX_BASE *db,long record,int field, char *str)
{
	return(db->Put_Slot(record,field,str));
}

CONTEXT_API long Find_First_Field(*CX_FIND f,int field, char *str,int atr)
{
	return(f->Find_First(field,str,atr));
}

CONTEXT_API CX_FIND *Open_Find(CX_BASE *db)
{
	return new CX_FIND(db);
}

CONTEXT_API long Find_First_Slot(*CX_FIND f, char *field, char *str,int atr)
{
	return(f->Find_First(field,str,atr));
}

CONTEXT_API long Next_Field(*CX_FIND f)
{
	return(f->Next());
}

CONTEXT_API void Close_Folder(CX_BASE *db);
{
	delete db;
}
