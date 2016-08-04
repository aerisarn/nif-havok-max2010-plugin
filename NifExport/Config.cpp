#include "pch.h"
#include "AppSettings.h"
#include "niutils.h"

#define REGPATH TEXT("Software\\NifTools\\MaxPlugins")

static LPCTSTR NifExportSection = TEXT("MaxNifExport");
static LPCTSTR KfExportSection = TEXT("KfExport");
static LPCTSTR CollisionSection = TEXT("Collision");


void regSet(HKEY hKey, const TCHAR *value, float f);
void regSet(HKEY hKey, const TCHAR *value, bool b);
void regSet(HKEY hKey, const TCHAR *value, DWORD dw);
void regSet(HKEY hKey, const TCHAR *value, int i);
void regSet(HKEY hKey, const TCHAR *value, const tstring &str);
void regGet(HKEY hKey, const TCHAR *value, float &f);
void regGet(HKEY hKey, const TCHAR *value, bool &b);
void regGet(HKEY hKey, const TCHAR *value, DWORD &v);
void regGet(HKEY hKey, const TCHAR *value, int &i);
void regGet(HKEY hKey, const TCHAR *value, tstring &str);

void Exporter::writeConfig(INode *node)
{
}

void Exporter::writeConfig(Interface *i)
{
   if (mUseRegistry)
   {
      HKEY hKey;
      if (RegCreateKey(HKEY_CURRENT_USER, REGPATH, &hKey) != ERROR_SUCCESS)
         return;

      regSet(hKey, TEXT("npx_ver"), mVersion);
      regSet(hKey, TEXT("npx_tristrips"), mTriStrips);
      regSet(hKey, TEXT("npx_hidden"), mExportHidden);
      regSet(hKey, TEXT("npx_furn"), mExportFurn);
      regSet(hKey, TEXT("npx_lights"), mExportLights);
      regSet(hKey, TEXT("npx_vcolors"), mVertexColors);
      //	regSet(hKey, TEXT("npx_wthresh"), mWeldThresh);
      regSet(hKey, TEXT("npx_tprefix"), mTexPrefix);
      regSet(hKey, TEXT("npx_coll"), mExportCollision);
      regSet(hKey, TEXT("npx_remap"), mRemapIndices);

      RegCloseKey(hKey);
   }
   else
   {
      TCHAR iniName[MAX_PATH];
      GetIniFileName(iniName);

      SetIniValue(NifExportSection, TEXT("GenerateStrips"), mTriStrips, iniName);    
      SetIniValue(NifExportSection, TEXT("IncludeHidden"), mExportHidden, iniName);
      SetIniValue(NifExportSection, TEXT("FurnatureMarkers"), mExportFurn, iniName);
      SetIniValue(NifExportSection, TEXT("Lights"), mExportLights, iniName);
      SetIniValue(NifExportSection, TEXT("VertexColors"), mVertexColors, iniName);
      SetIniValue(NifExportSection, TEXT("TexturePrefix"), mTexPrefix, iniName);
      SetIniValue(NifExportSection, TEXT("ExportCollision"), mExportCollision, iniName);
      SetIniValue(NifExportSection, TEXT("RemapIndices"), mRemapIndices, iniName);

      SetIniValue(NifExportSection, TEXT("ExportExtraNodes"), mExportExtraNodes, iniName);
      SetIniValue(NifExportSection, TEXT("ExportSkin"), mExportSkin, iniName);
      SetIniValue(NifExportSection, TEXT("UserPropBuffer"), mUserPropBuffer, iniName);
      SetIniValue(NifExportSection, TEXT("FlattenHierarchy"), mFlattenHierarchy, iniName);
      SetIniValue(NifExportSection, TEXT("RemoveUnreferencedBones"), mRemoveUnreferencedBones, iniName);
      SetIniValue(NifExportSection, TEXT("SortNodesToEnd"), mSortNodesToEnd, iniName);
      SetIniValue(NifExportSection, TEXT("SkeletonOnly"), mSkeletonOnly, iniName);
      SetIniValue(NifExportSection, TEXT("Cameras"), mExportCameras, iniName);
      SetIniValue(NifExportSection, TEXT("GenerateBoneCollision"), mGenerateBoneCollision, iniName);

      SetIniValue(NifExportSection, TEXT("ExportTransforms"), mExportTransforms, iniName);
      SetIniValue<int>(NifExportSection, TEXT("ExportType"), mExportType, iniName);     
      SetIniValue<float>(KfExportSection, TEXT("Priority"), mDefaultPriority, iniName);

      SetIniValue(NifExportSection, TEXT("MultiplePartitions"), mMultiplePartitions, iniName);
      SetIniValue<int>(NifExportSection, TEXT("BonesPerVertex"), mBonesPerVertex, iniName);     
      SetIniValue<int>(KfExportSection, TEXT("BonesPerPartition"), mBonesPerPartition, iniName);
      //SetIniValue(NifExportSection, TEXT("UseTimeTags"), mUseTimeTags, iniName);

      SetIniValue(NifExportSection, TEXT("AllowAccum"), mAllowAccum, iniName);
      SetIniValue(NifExportSection, TEXT("CollapseTransforms"), mCollapseTransforms, iniName);
      SetIniValue(NifExportSection, TEXT("ZeroTransforms"), mZeroTransforms, iniName);
      SetIniValue(NifExportSection, TEXT("FixNormals"), mFixNormals, iniName);
      SetIniValue(NifExportSection, TEXT("TangentAndBinormalExtraData"), mTangentAndBinormalExtraData, iniName);
      SetIniValue(NifExportSection, TEXT("UseAlternateStripper"), mUseAlternateStripper, iniName);
      SetIniValue(NifExportSection, TEXT("TangentAndBinormalMethod"), mTangentAndBinormalMethod, iniName);
      SetIniValue(NifExportSection, TEXT("StartNifskopeAfterStart"), mStartNifskopeAfterStart, iniName);
      //SetIniValue(CollisionSection, TEXT("bhkScaleFactor"), bhkScaleFactor, iniName);

      SetIniValue<tstring>(NifExportSection, TEXT("Creator"), mCreatorName, iniName);
      SetIniValue(NifExportSection, TEXT("GeneratePartitionStrips"), mTriPartStrips, iniName);

      SetIniValue(NifExportSection, TEXT("WeldVertexThresh"), mWeldThresh, iniName);
      SetIniValue(NifExportSection, TEXT("WeldVertexThresh"), mNormThresh, iniName);
      SetIniValue(NifExportSection, TEXT("WeldUVWThresh"), mUVWThresh, iniName);

	  SetIniValue<tstring>(NifExportSection, TEXT("RootType"), mRootType, iniName);
   }
}

void Exporter::readConfig(INode *node)
{

}

void Exporter::readConfig(Interface *i)
{
   if (mUseRegistry)
   {
      HKEY hKey;
      if (RegCreateKey(HKEY_CURRENT_USER, REGPATH, &hKey) != ERROR_SUCCESS)
         return;

      DWORD ver;
      regGet(hKey, TEXT("npx_ver"), ver);
      regGet(hKey, TEXT("npx_tristrips"), mTriStrips);
      regGet(hKey, TEXT("npx_hidden"), mExportHidden);
      regGet(hKey, TEXT("npx_furn"), mExportFurn);
      regGet(hKey, TEXT("npx_lights"), mExportLights);
      regGet(hKey, TEXT("npx_vcolors"), mVertexColors);
      //	regGet(hKey, TEXT("npx_wthresh"), mWeldThresh);
      regGet(hKey, TEXT("npx_tprefix"), mTexPrefix);
      regGet(hKey, TEXT("npx_coll"), mExportCollision);
      regGet(hKey, TEXT("npx_remap"), mRemapIndices);
   }
   else
   {
      TCHAR iniName[MAX_PATH];
      GetIniFileName(iniName);

      //mVersion = GetIniValue<int>(NifExportSection, TEXT("Version"), 013, iniName);
      mTriStrips = GetIniValue<bool>(NifExportSection, TEXT("GenerateStrips"), true, iniName);
      mExportHidden = GetIniValue<bool>(NifExportSection, TEXT("IncludeHidden"), false, iniName);
      mExportFurn = GetIniValue<bool>(NifExportSection, TEXT("FurnatureMarkers"), true, iniName);
      mExportLights = GetIniValue<bool>(NifExportSection, TEXT("Lights"), false, iniName);
      mVertexColors = GetIniValue<bool>(NifExportSection, TEXT("VertexColors"), true, iniName);
      mWeldThresh = GetIniValue<float>(NifExportSection, TEXT("WeldVertexThresh"), 0.01f, iniName);
      mNormThresh = GetIniValue<float>(NifExportSection, TEXT("WeldNormThresh"), 0.01f, iniName);
      mUVWThresh = GetIniValue<float>(NifExportSection, TEXT("WeldUVWThresh"), 0.01f, iniName);

      mTexPrefix = GetIniValue<tstring>(NifExportSection, TEXT("TexturePrefix"), TEXT("textures"), iniName);
      mExportCollision = GetIniValue<bool>(NifExportSection, TEXT("ExportCollision"), true, iniName);
      mRemapIndices = GetIniValue(NifExportSection, TEXT("RemapIndices"), true, iniName);

      mExportExtraNodes = GetIniValue(NifExportSection, TEXT("ExportExtraNodes"), false, iniName);
      mExportSkin = GetIniValue(NifExportSection, TEXT("ExportSkin"), false, iniName);
      mUserPropBuffer = GetIniValue(NifExportSection, TEXT("UserPropBuffer"), false, iniName);
      mFlattenHierarchy = GetIniValue(NifExportSection, TEXT("FlattenHierarchy"), false, iniName);
      mRemoveUnreferencedBones = GetIniValue(NifExportSection, TEXT("RemoveUnreferencedBones"), false, iniName);
      mSortNodesToEnd = GetIniValue(NifExportSection, TEXT("SortNodesToEnd"), false, iniName);
      mSkeletonOnly = GetIniValue(NifExportSection, TEXT("SkeletonOnly"), false, iniName);
      mExportCameras = GetIniValue(NifExportSection, TEXT("Cameras"), false, iniName);
      mGenerateBoneCollision = GetIniValue(NifExportSection, TEXT("GenerateBoneCollision"), false, iniName);

      mExportTransforms = GetIniValue(KfExportSection, TEXT("Transforms"), true, iniName);
      mDefaultPriority = GetIniValue<float>(KfExportSection, TEXT("Priority"), 0.0f, iniName);
      mExportType = ExportType(GetIniValue<int>(NifExportSection, TEXT("ExportType"), NIF_WO_ANIM, iniName));

      mMultiplePartitions = GetIniValue(NifExportSection, TEXT("MultiplePartitions"), false, iniName);
      mBonesPerVertex = GetIniValue<int>(NifExportSection, TEXT("BonesPerVertex"), 4, iniName);     
      mBonesPerPartition = GetIniValue<int>(NifExportSection, TEXT("BonesPerPartition"), 20, iniName);

      //mUseTimeTags = GetIniValue(NifExportSection, TEXT("UseTimeTags"), false, iniName);
      mAllowAccum = GetIniValue(NifExportSection, TEXT("AllowAccum"), true, iniName);
      mCollapseTransforms = GetIniValue(NifExportSection, TEXT("CollapseTransforms"), false, iniName);
      mZeroTransforms = GetIniValue(NifExportSection, TEXT("ZeroTransforms"), false, iniName);
      mFixNormals = GetIniValue(NifExportSection, TEXT("FixNormals"), false, iniName);
      mTangentAndBinormalExtraData = GetIniValue(NifExportSection, TEXT("TangentAndBinormalExtraData"), false, iniName);
      mTangentAndBinormalMethod = GetIniValue<int>(NifExportSection, TEXT("TangentAndBinormalMethod"), 0, iniName);

      mUseAlternateStripper = GetIniValue(NifExportSection, TEXT("UseAlternateStripper"), false, iniName);
      mCreatorName = GetIniValue<tstring>(NifExportSection, TEXT("Creator"), TEXT(""), iniName);

      bhkScaleFactor = GetIniValue<float>(CollisionSection, TEXT("bhkScaleFactor"), 7.0f, iniName);

      mStartNifskopeAfterStart = GetIniValue(NifExportSection, TEXT("StartNifskopeAfterStart"), false, iniName);
      mNifskopeDir = ExpandEnvironment(GetIndirectValue(GetIniValue<tstring>(TEXT("System"), TEXT("NifskopeDir"), TEXT(""), iniName).c_str()));
      if (mNifskopeDir.empty())
          mNifskopeDir = ExpandEnvironment(GetIndirectValue(GetIniValue<tstring>(TEXT("System"), TEXT("AltNifskopeDir"), TEXT(""), iniName).c_str()));
      mTriPartStrips = GetIniValue<bool>(NifExportSection, TEXT("GeneratePartitionStrips"), true, iniName);

	  mRootType = GetIniValue<tstring>(NifExportSection, TEXT("RootType"), TEXT("NiNode"), iniName);
	  mRootTypes = TokenizeString(GetIniValue<tstring>(NifExportSection, TEXT("RootTypes"), TEXT("NiNode;BSFadeNode"), iniName).c_str(), TEXT(";"));

	  mDebugEnabled = GetIniValue(NifExportSection, TEXT("EnableDebug"), false, iniName);
  }
}


void Exporter::readKfConfig(Interface *i)
{
   TCHAR iniName[MAX_PATH];
   GetIniFileName(iniName);

   mExportHidden = GetIniValue(KfExportSection, TEXT("IncludeHidden"), false, iniName);
   mExportLights = GetIniValue(KfExportSection, TEXT("Lights"), false, iniName);
   mExportCameras = GetIniValue(KfExportSection, TEXT("Cameras"), false, iniName);
   mExportTransforms = GetIniValue(KfExportSection, TEXT("Transforms"), true, iniName);
   mDefaultPriority = GetIniValue<float>(KfExportSection, TEXT("Priority"), 0.0f, iniName);
}

void Exporter::writeKfConfig(Interface *i)
{
   TCHAR iniName[MAX_PATH];
   LPCTSTR pluginDir = i->GetDir(APP_PLUGCFG_DIR);
   PathCombine(iniName, pluginDir, TEXT("MaxNifTools.ini"));

   SetIniValue(KfExportSection, TEXT("IncludeHidden"), mExportHidden, iniName);
   SetIniValue(KfExportSection, TEXT("Lights"), mExportLights, iniName);
   SetIniValue(KfExportSection, TEXT("Cameras"), mExportCameras, iniName);
   SetIniValue(KfExportSection, TEXT("Transforms"), mExportTransforms, iniName);
   SetIniValue<float>(KfExportSection, TEXT("Priority"), mDefaultPriority, iniName);
}


AppSettings * Exporter::exportAppSettings()
{
   TCHAR iniName[MAX_PATH];
   GetIniFileName(iniName);

   // Update the current app version
   AppSettings * appSettings = FindAppSetting(Exporter::mGameName);
   if (appSettings == NULL && !TheAppSettings.empty())
      appSettings = &TheAppSettings.front();

   if (Exporter::mAutoDetect){
      SetIniValue<tstring>(NifExportSection, TEXT("CurrentApp"), TEXT("AUTO"), iniName);
   } else {
      SetIniValue<tstring>(NifExportSection, TEXT("CurrentApp"), appSettings->Name, iniName);
   }
   SetIniValue<tstring>(NifExportSection, TEXT("LastSelectedApp"), appSettings->Name, iniName);

   appSettings->NiVersion = Exporter::mNifVersion;
   appSettings->NiUserVersion = Exporter::mNifUserVersion;
	appSettings->NiUserVersion2 = Exporter::mNifUserVersion2;
   appSettings->rotate90Degrees = Exporter::mRotate90Degrees;
   appSettings->supportPrnStrings = Exporter::mSupportPrnStrings;

   return appSettings;
}

AppSettings *Exporter::importAppSettings(tstring fname)
{
   AppSettings *appSettings = NULL;
   TCHAR iniName[MAX_PATH];
   GetIniFileName(iniName);

   // Locate which application to use. If Auto, find first app where this file appears in the root path list
   tstring curapp = GetIniValue<tstring>(NifExportSection, TEXT("CurrentApp"), TEXT("AUTO"), iniName);
   tstring lastselapp = GetIniValue<tstring>(NifExportSection, TEXT("LastSelectedApp"), TEXT(""), iniName);
   if (0 == _tcsicmp(curapp.c_str(), TEXT("AUTO"))) {
      Exporter::mAutoDetect = true;
      // Scan Root paths
      for (AppSettingsMap::iterator itr = TheAppSettings.begin(), end = TheAppSettings.end(); itr != end; ++itr){
         if ((*itr).IsFileInRootPaths(fname)) {
            appSettings = &(*itr);
            break;
         }
      }
   } else {
      Exporter::mAutoDetect = false;
      appSettings = FindAppSetting(curapp);
   }
   if (appSettings == NULL && !lastselapp.empty())
	   appSettings = FindAppSetting(lastselapp);
   if (appSettings == NULL && !TheAppSettings.empty())
      appSettings = &TheAppSettings.front();

   if (!appSettings)
      return NULL;

   Exporter::mGameName = appSettings->Name;
   Exporter::mNifVersion = appSettings->NiVersion;
   Exporter::mNifUserVersion = appSettings->NiUserVersion;
	Exporter::mNifUserVersion2 = appSettings->NiUserVersion2;
   if (!appSettings->rotate90Degrees.empty())
      Exporter::mRotate90Degrees = appSettings->rotate90Degrees;
   Exporter::mSupportPrnStrings = appSettings->supportPrnStrings;

   return appSettings;
}

void regSet(HKEY hKey, const TCHAR *value, float f)
{
	DWORD dw = *((DWORD*)&f);
    RegSetValueEx(hKey, value, NULL, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
}

void regSet(HKEY hKey, const TCHAR *value, bool b)
{
	DWORD dw = (DWORD)b;
    RegSetValueEx(hKey, value, NULL, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
}

void regSet(HKEY hKey, const TCHAR *value, DWORD dw)
{
    RegSetValueEx(hKey, value, NULL, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
}

void regSet(HKEY hKey, const TCHAR *value, int v)
{
	DWORD dw = (DWORD)v;
    RegSetValueEx(hKey, value, NULL, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
}

void regSet(HKEY hKey, const TCHAR *value, const tstring &str)
{
	RegSetValueEx(hKey, value, NULL, REG_SZ, (LPBYTE)str.c_str(), str.length()+1);
}

void regGet(HKEY hKey, const TCHAR *value, float &f)
{
	DWORD dw, type, cdata = sizeof(DWORD);
    if (RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)&dw, &cdata)==ERROR_SUCCESS && type==REG_DWORD)
		f = *((float*)&dw);
}

void regGet(HKEY hKey, const TCHAR *value, bool &b)
{
	DWORD dw, type, cdata = sizeof(DWORD);
    if (RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)&dw, &cdata)==ERROR_SUCCESS && type==REG_DWORD)
		b = (bool)dw;
}

void regGet(HKEY hKey, const TCHAR *value, DWORD &v)
{
	DWORD dw, type, cdata = sizeof(DWORD);
    if (RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)&dw, &cdata)==ERROR_SUCCESS && type==REG_DWORD)
		v = dw;
}

void regGet(HKEY hKey, const TCHAR *value, int &v)
{
	DWORD dw, type, cdata = sizeof(DWORD);
    if (RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)&dw, &cdata)==ERROR_SUCCESS && type==REG_DWORD)
		v = dw;
}

void regGet(HKEY hKey, const TCHAR *value, tstring &str)
{
	TCHAR buff[MAX_PATH];
	DWORD type, cdata = MAX_PATH;
    if (RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)buff, &cdata)==ERROR_SUCCESS && type==REG_SZ)
		str = buff;
}
