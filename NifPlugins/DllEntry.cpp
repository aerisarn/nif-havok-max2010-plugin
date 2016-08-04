/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: tazpn (Theo)

	HISTORY: 

 *>	Copyright (c) 2006, All Rights Reserved.
 **********************************************************************/

#include <notify.h>


extern void DoNotifyNodeHide(void *param, NotifyInfo *info);
extern void DoNotifyNodeUnHide(void *param, NotifyInfo *info);
extern Class_ID BHKLISTOBJECT_CLASS_ID;

extern ClassDesc2* GetMaxNifImportDesc();
extern ClassDesc2* GetNifExportDesc();
extern ClassDesc2* GetNifPropsDesc();
extern ClassDesc2* GetNifFurnitureDesc();
extern ClassDesc2* GetKfExportDesc();
extern ClassDesc2* GetbhkSphereDesc();
extern ClassDesc2* GetbhkCapsuleDesc();
extern ClassDesc2* GetbhkRigidBodyModifierDesc();
extern ClassDesc2* GetbhkBoxDesc();
extern ClassDesc* GetDDSLibClassDesc();
extern ClassDesc2* GetbhkListObjDesc();
extern ClassDesc2* GetbhkProxyObjDesc();
extern ClassDesc2* GetBSDSModifierDesc();
extern ClassDesc2* GetBSSIModifierDesc();
extern ClassDesc2* GetNifShaderDesc();
extern ClassDesc2* GetFO4ShaderDesc();
extern ClassDesc2* GetBGSMFileClassDesc();
extern ClassDesc2* GetBGEMFileClassDesc();


enum ClassDescType
{
   CD_Import,
   CD_Export,
   CD_Props,
   CD_Furniture,
   CD_KFExport,
   CD_Count
};

static void InitializeLibSettings();
static void InitializeHavok();
extern void InitializeNifProps();

HINSTANCE hInstance;
static int controlsInit = FALSE;
static int libVersion = VERSION_3DSMAX;
static int foundOlderReleaseConflict = -1;
static int nClasses = 0;
static ClassDesc2* classDescriptions[30];
static bool classDescEnabled[CD_Count];

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
#if VERSION_3DSMAX < (14000<<16) // Version 14 (2012)
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
#endif
		InitCommonControls();			// Initialize Win95 controls
		RegisterNotification(DoNotifyNodeHide, nullptr, NOTIFY_NODE_HIDE); 
		RegisterNotification(DoNotifyNodeUnHide, nullptr, NOTIFY_NODE_UNHIDE); 
	}
	if (fdwReason == DLL_PROCESS_ATTACH)
		InitializeLibSettings();
	return (TRUE);
}

void InitializeLibSettings()
{
   TCHAR iniName[MAX_PATH];
   GetIniFileName(iniName);
   libVersion = GetIniValue(TEXT("System"), TEXT("MaxSDKVersion"), libVersion, iniName);
   if (libVersion == 0)
      libVersion = VERSION_3DSMAX;

   nClasses = 0;
   if ( GetIniValue<bool>(TEXT("MaxNifExport"), TEXT("Enable"), true, iniName) ) {
      classDescEnabled[CD_Export] = true;
      classDescriptions[nClasses++] = GetNifExportDesc();
   }
   if ( GetIniValue<bool>(TEXT("MaxNifImport"), TEXT("Enable"), true, iniName) ) {
      classDescEnabled[CD_Import] = true;
      classDescriptions[nClasses++] = GetMaxNifImportDesc();
   }
   if ( GetIniValue<bool>(TEXT("NifProps"), TEXT("Enable"), true, iniName) ) {
      classDescEnabled[CD_Props] = true;
      classDescriptions[nClasses++] = GetNifPropsDesc();
	  classDescriptions[nClasses++] = GetbhkListObjDesc();
	  classDescriptions[nClasses++] = GetbhkProxyObjDesc();
      classDescriptions[nClasses++] = GetbhkRigidBodyModifierDesc();
      classDescriptions[nClasses++] = GetbhkSphereDesc();
      classDescriptions[nClasses++] = GetbhkCapsuleDesc();
      classDescriptions[nClasses++] = GetbhkBoxDesc();
      classDescriptions[nClasses++] = GetBSDSModifierDesc();
      classDescriptions[nClasses++] = GetNifShaderDesc();
	  classDescriptions[nClasses++] = GetBSSIModifierDesc();
	  classDescriptions[nClasses++] = GetFO4ShaderDesc();	  
	  classDescriptions[nClasses++] = GetBGSMFileClassDesc();
	  classDescriptions[nClasses++] = GetBGEMFileClassDesc();	  
   }
   if ( GetIniValue<bool>(TEXT("NifFurniture"), TEXT("Enable"), true, iniName) ) {
      classDescEnabled[CD_Furniture] = true;
      classDescriptions[nClasses++] = GetNifFurnitureDesc();
   }
   if ( GetIniValue<bool>(TEXT("KFExport"), TEXT("Enable"), false, iniName) ) {
      classDescEnabled[CD_KFExport] = true;
      classDescriptions[nClasses++] = GetKfExportDesc();
   }
#ifdef GAME_VER
   classDescriptions[nClasses++] = (ClassDesc2 *)GetDDSLibClassDesc();
#endif
   InitializeHavok();

   InitializeNifProps();
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
   return TEXT("Niftools Max Plugins");
	//return GetString(IDS_LIBDESCRIPTION);
}

static LPTSTR PathMerge(LPTSTR base, LPCTSTR file)
{
   PathRemoveFileSpec(base);
   PathAppend(base, file);
   return base;
}

static HMODULE DelayLoadLibraryA(LPCSTR dllname)
{
	CHAR curfile[_MAX_PATH];
	GetModuleFileNameA(hInstance, curfile, MAX_PATH);
	PathRemoveFileSpecA(curfile);
	PathAppendA(curfile, dllname);
	HMODULE hdll = LoadLibraryA(curfile);
	if (hdll == nullptr)
		return LoadLibraryA(dllname);
	return hdll;
}
static HMODULE DelayLoadLibraryW(LPCWSTR dllname)
{
	WCHAR curfile[_MAX_PATH];
	GetModuleFileNameW(hInstance, curfile, MAX_PATH);
	PathRemoveFileSpecW(curfile);
	PathAppendW(curfile, dllname);
	HMODULE hdll = LoadLibraryW(curfile);
	if (hdll == nullptr)
		return LoadLibraryW(dllname);
	return hdll;
}


// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
   // Has conflict exit now.
   if (foundOlderReleaseConflict > 0) {
      return 0;
   } else if ( foundOlderReleaseConflict < 0 ) { 
      foundOlderReleaseConflict = 0;

      // Check for older releases
      if (  (classDescEnabled[CD_Import] && nullptr != GetModuleHandle(TEXT("MaxNifImport.dli")))
         || (classDescEnabled[CD_Export] && nullptr != GetModuleHandle(TEXT("NifExport.dle")))
         || (classDescEnabled[CD_Furniture] && nullptr != GetModuleHandle(TEXT("NifFurniture.dlo")))
         || (classDescEnabled[CD_Props]  && nullptr != GetModuleHandle(TEXT("NifProps.dlu")))
         )
      {
         foundOlderReleaseConflict = 1;
      }
      else
      {
         // do more aggressive access search now in case we are loaded after them
         TCHAR filename[MAX_PATH];
         GetModuleFileName(hInstance, filename, MAX_PATH);

         if (classDescEnabled[CD_Import] && -1 != _taccess(PathMerge(filename, TEXT("MaxNifImport.dli")), 0))
            foundOlderReleaseConflict = 1;
         else if (classDescEnabled[CD_Export] && -1 != _taccess(PathMerge(filename, TEXT("NifExport.dle")), 0))
            foundOlderReleaseConflict = 1;
         else if (classDescEnabled[CD_Furniture] && -1 != _taccess(PathMerge(filename, TEXT("NifFurniture.dlo")), 0))
            foundOlderReleaseConflict = 1;
         else if (classDescEnabled[CD_Props] && -1 != _taccess(PathMerge(filename, TEXT("NifProps.dlu")), 0))
            foundOlderReleaseConflict = 1;
      }
      if (foundOlderReleaseConflict > 0)
      {
         TCHAR buffer[512];
         _stprintf(buffer,
			 TEXT("An older release of the Niftools Max Plugins was found.\n\n")
			 TEXT("Please remove the following files from your 3dsmax\\plugins directory:\n")
			 TEXT("%s%s%s%s")
			 TEXT("The current version will be disabled.")
            , classDescEnabled[CD_Import] ? TEXT("\tMaxNifImport.dli\n") : TEXT("")
            , classDescEnabled[CD_Export] ? TEXT("\tNifExport.dle\n") : TEXT("")
            , classDescEnabled[CD_Furniture] ? TEXT("\tNifFurniture.dlo\n") : TEXT("")
            , classDescEnabled[CD_Props] ? TEXT("\tNifProps.dlu\n\n") : TEXT("")
            );
         ::MessageBox( nullptr
            , buffer
            , TEXT("Niftools Max Plugins")
            , MB_ICONSTOP|MB_OK
            );
         return 0;
      }
   }
	return nClasses;
}

// This function returns the number of plug-in classes this DLL
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   return classDescriptions[i];
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : nullptr;
	return nullptr;
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
   return ULONG(libVersion);
}

static void DoNotifyNodeHide(void *param, NotifyInfo *info)
{
	int code = info->intcode;
	INode *node = (INode*)info->callParam;
	if (Object* obj = node->GetObjectRef())
	{
		// Look for messages in network\Max.log
		// MAXScript_interface->Log()->LogEntry(SYSLOG_DEBUG, NO_DIALOG, "NifTools Max Plugin", 
		// 	"Entered DoNotifyNodeHide; node is -%s- and class ID is %ld\n", node->GetName(), obj->ClassID().PartA());

	   if (obj->ClassID() == BHKLISTOBJECT_CLASS_ID)
	   {
		   const int PB_MESHLIST = 1;
		   IParamBlock2* pblock2 = obj->GetParamBlockByID(0);
		   int nBlocks = pblock2->Count(PB_MESHLIST);
		   for (int i = 0;i < pblock2->Count(PB_MESHLIST); i++)
		   {
			   INode *tnode = nullptr;
			   pblock2->GetValue(PB_MESHLIST,0,tnode,FOREVER,i);	
			   if (tnode != nullptr)
			   {
				   tnode->Hide(TRUE);
			   }
		   }
	   }
	}
}

static void DoNotifyNodeUnHide(void *param, NotifyInfo *info)
{
	int code = info->intcode;
	INode *node = (INode*)info->callParam;
	if (Object* obj = node->GetObjectRef())
	{
		// Look for messages in network\Max.log
		// MAXScript_interface->Log()->LogEntry(SYSLOG_DEBUG, NO_DIALOG, "NifTools Max Plugin", 
		// 	"Entered DoNotifyNodeUnHide; node is -%s- and class ID is %ld\n", node->GetName(), obj->ClassID().PartA());

	   if (obj->ClassID() == BHKLISTOBJECT_CLASS_ID)
	   {
		   const int PB_MESHLIST = 1;
		   IParamBlock2* pblock2 = obj->GetParamBlockByID(0);
		   int nBlocks = pblock2->Count(PB_MESHLIST);
		   for (int i = 0;i < pblock2->Count(PB_MESHLIST); i++)
		   {
			   INode *tnode = nullptr;
			   pblock2->GetValue(PB_MESHLIST,0,tnode,FOREVER,i);	
			   if (tnode != nullptr)
			   {
				   tnode->Hide(FALSE);
			   }
		   }
	   }
	}
}

#include "Inertia.h"
static void InitializeHavok()
{
	// disable in 64-bit as the base Niflib versions are more functional
#ifndef WIN64
	HMODULE hNifHavok = DelayLoadLibraryA("NifMopp.dll");
	if ( hNifHavok != nullptr )
	{
		Niflib::Inertia::SetCalcMassPropertiesBox( 
			(Niflib::Inertia::fnCalcMassPropertiesBox)GetProcAddress(hNifHavok, "CalcMassPropertiesBox") );

		Niflib::Inertia::SetCalcMassPropertiesSphere( 
			(Niflib::Inertia::fnCalcMassPropertiesSphere)GetProcAddress(hNifHavok, "CalcMassPropertiesSphere") );

		Niflib::Inertia::SetCalcMassPropertiesCapsule( 
			(Niflib::Inertia::fnCalcMassPropertiesCapsule)GetProcAddress(hNifHavok, "CalcMassPropertiesCapsule") );

		Niflib::Inertia::SetCalcMassPropertiesPolyhedron( 
			(Niflib::Inertia::fnCalcMassPropertiesPolyhedron)GetProcAddress(hNifHavok, "CalcMassPropertiesPolyhedron") );

		Niflib::Inertia::SetCombineMassProperties( 
			(Niflib::Inertia::fnCombineMassProperties)GetProcAddress(hNifHavok, "CombineMassProperties") );
	}
#endif
}

// Include delayloading
//#include "DelayMaxCompat.cpp"
#include <DelayImp.h>   // Required for hooking
#pragma comment(lib, "Delayimp.lib")
// delayHookFunc - Delay load hooking function
FARPROC WINAPI delayHookFailureFunc(unsigned dliNotify, PDelayLoadInfo pdli)
{
	FARPROC fp = NULL;   // Default return value
						 // NOTE: The members of the DelayLoadInfo structure pointed
						 // to by pdli shows the results of progress made so far. 
	switch (dliNotify) {

	case dliFailLoadLib:
		// LoadLibrary failed.
		// In here a second attempt could be made to load the dll somehow. 
		// If fp is still NULL, the ERROR_MOD_NOT_FOUND exception will be raised.
		fp = (FARPROC)DelayLoadLibraryA(pdli->szDll);
		break;

	case dliFailGetProc:
		// GetProcAddress failed.
		// A second attempt could be made to get the function pointer somehow. 
		// We can override and give our own function pointer in fp.
		// Ofcourse, fp is still going to be NULL, 
		// the ERROR_PROC_NOT_FOUND exception will be raised.
		fp = (FARPROC)NULL;
		break;
	}

	return(fp);
}

// __delayLoadHelper gets the hook function in here:
PfnDliHook __pfnDliFailureHook2 = delayHookFailureFunc;
