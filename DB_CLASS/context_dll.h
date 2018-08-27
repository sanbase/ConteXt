
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SANTEXT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SANTEXT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#include "CX_BASE.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define SANTEXT_API __declspec(dllexport)

// This class is exported from the santext.dll

extern  CONTEXT_API CX_BASE *Open_Folder(const char *);
extern  CONTEXT_API long New_Record(CX_BASE *);
extern  CONTEXT_API int  Get_Slot (const CX_BASE *,long record,const char *field,char &*);
extern  CONTEXT_API int  Put_Slot (const CX_BASE *,long record,const char *field,const char *);
extern  CONTEXT_API int  Get_Field(const CX_BASE *,long record,int field,char &*);
extern  CONTEXT_API int  Put_Field(const CX_BASE *,long record,int field,const char *);
extern  CONTEXT_API CX_FIND *Open_Find(CX_BASE *);
extern  CONTEXT_API long Find_First_Field(const *CX_FIND,int field,const char *str,int atr);
extern  CONTEXT_API long Find_First_Slot(const *CX_FIND,const char *field,const char *str,int atr);
extern  CONTEXT_API long Next_Field(const *CX_FIND,long record,char *field);
extern  CONTEXT_API long Next_Slot(const *CX_FIND,long record,int field);
extern  CONTEXT_API void Close_Folder(CX_BASE *);

#ifdef __cplusplus
}
#endif
