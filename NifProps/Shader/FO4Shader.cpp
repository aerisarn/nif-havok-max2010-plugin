//////////////////////////////////////////////////////////////////////////////
//
//    Strauss Shader plug-in, implementation
//
//    Created: 10/26/98 Kells Elmquist
//
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "..\NifProps\NifProps.h"
#include "3dsmaxport.h"
#include "../iNifProps.h"
#include "../materialfile.h"
#include <notify.h>

extern TCHAR *GetString(int id);
extern TSTR shortDescription;

// Class Ids
#undef GNORMAL_CLASS_ID
static const Class_ID GNORMAL_CLASS_ID(0x243e22c6, 0x63f6a014);
const Class_ID FO4SHADER_CLASS_ID(0x7a6bc2e7, 0x71106f41);

enum {
	ref_base, ref_mtl, ref_bgsm, ref_bgem,
	ref_activemtl, ref_oldmtl, MAX_REFERENCES
};

// Paramblock2 name
enum { shader_params, };
// Paramblock2 parameter list

const EnumLookupType TransparencyModes[] = {
   { 0, TEXT("ONE")},
   { 1, TEXT("ZERO")},
   { 2, TEXT("SRCCOLOR")},
   { 3, TEXT("INVSRCCOLOR")},
   { 4, TEXT("DESTCOLOR")},
   { 5, TEXT("INVDESTCOLOR")},
   { 6, TEXT("SRCALPHA")},
   { 7, TEXT("INVSRCALPHA")},
   { 8, TEXT("DESTALPHA")},
   { 9, TEXT("INVDESTALPHA")},
   {10, TEXT("SRCALPHASAT")},
   {-1, nullptr},
};
const EnumLookupType TestModes[] = {
   { 0, TEXT("ALWAYS")},
   { 1, TEXT("LESS")},
   { 2, TEXT("EQUAL")},
   { 3, TEXT("LESSEQUAL")},
   { 4, TEXT("GREATER")},
   { 5, TEXT("NOTEQUAL")},
   { 6, TEXT("GREATEREQUAL")},
   { 7, TEXT("NEVER")},
   {-1, nullptr},
};
const EnumLookupType ApplyModes[] = {
   { 0, TEXT("REPLACE")},
   { 1, TEXT("DECAL")},
   { 2, TEXT("MODULATE")},
   {-1, nullptr},
};

const EnumLookupType VertexModes[] = {
   { 0, TEXT("IGNORE")},
   { 1, TEXT("EMISSIVE")},
   { 2, TEXT("AMB_DIFF")},
   {-1, nullptr},
};
const EnumLookupType LightModes[] = {
   { 0, TEXT("E")},
   { 1, TEXT("E_A_D")},
   {-1, nullptr},
};

const EnumLookupType MaterialFileTypes[] = {
   { MFT_BGSM, TEXT("BGSM - Lighting Shader")},
   { MFT_BGEM, TEXT("BGEM - Effect Shader")},
   {-1, nullptr},
};
const EnumLookupType MaterialFileTypesShort[] = {
	{ MFT_BGSM, TEXT("BGSM") },
	{ MFT_BGEM, TEXT("BGEM") },
	{ -1, nullptr },
};


struct TexChannel
{
	DWORD channelName;
	DWORD maxName;
	DWORD channelType;
};

static const TexChannel texChannelNames[STD2_NMAX_TEXMAPS] = {
   {IDS_CHAN_BASE,      IDS_MAXCHAN_DIFFUSE,       CLR_CHANNEL    },             //C_DIFFUSE,
   {IDS_CHAN_NORMAL,    IDS_MAXCHAN_NORMAL,        CLR_CHANNEL    },			 //C_NORMAL,
   {IDS_CHAN_SPECULAR,  IDS_MAXCHAN_DETAIL,        CLR_CHANNEL    },			 //C_SMOOTHSPEC,
   {IDS_CHAN_GREYSCALE, IDS_CHAN_GREYSCALE,        CLR_CHANNEL    },			 //C_GREYSCALE,
   {IDS_CHAN_ENV,       IDS_CHAN_ENV,              CLR_CHANNEL    },			 //C_ENVMAP,
   {IDS_CHAN_GLOW,      IDS_MAXCHAN_GLOW,          CLR_CHANNEL    },			 //C_GLOW,
   {IDS_CHAN_INNER,     IDS_CHAN_INNER,            CLR_CHANNEL    },			 //C_INNERLAYER,
   {IDS_CHAN_WRINKLES,  IDS_CHAN_WRINKLES,         CLR_CHANNEL    },			 //C_WRINKLES,
   {IDS_CHAN_DISPLACE,  IDS_CHAN_DISPLACE,         CLR_CHANNEL    },			 //C_DISPLACE,
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
};

enum
{
	C_DIFFUSE,
	C_NORMAL,
	C_SMOOTHSPEC,
	C_GREYSCALE,
	C_ENVMAP,
	C_GLOW,
	C_INNERLAYER,
	C_WRINKLES,
	C_DISPLACE,
	C_MAX_SUPPORTED,
};



// map from custom channel to standard map
static const int FO4ShaderStdIDToChannel[] = {
	-1,           // 0 - ambient
	C_DIFFUSE,    // 1 - diffuse           
	-1,			  // 2 - specular
	-1,           // 3 - Glossiness (Shininess in 3ds Max release 2.0 and earlier)
	-1,           // 4 - Specular Level (Shininess strength in 3ds Max release 2.0 and earlier)
	C_GLOW,       // 5 - self-illumination 
	C_MAX_SUPPORTED, // 6 - opacity
	-1,           // 7 - filter color
	C_NORMAL,     // 8 - bump              
	-1,           // 9 - reflection        
	-1,           // 10 - refraction 
	-1,           // 11 - displacement
	-1,           // 12 - 
	-1,           // 13 -  
	-1,           // 14 -  
	-1,           // 15 -  
	-1,           // 16 -  
	-1,           // 17 -  
	-1,           // 18 -  
	-1,           // 19 -  
	-1,           // 21 -  
	-1,           // 22 -  
	-1,           // 23 -  
};

const ULONG SHADER_PARAMS = (STD_PARAM_SELFILLUM | STD_PARAM_SELFILLUM_CLR
	| STD_PARAM_GLOSSINESS // || STD_PARAM_SPECULAR_CLR
	| STD_PARAM_SELFILLUM_CLR_ON // | STD_PARAM_SPECULAR_LEV 
	);
// const ULONG SHADER_PARAMS = 0;

class Fallout4FileResolver : public IFileResolver
{
	mutable AppSettings* pSettings;
public:
	Fallout4FileResolver() { pSettings = nullptr; }
	void ResolveSettings() const
	{
		if (pSettings == nullptr) {
			if (!AppSettings::Initialized())
				AppSettings::Initialize(GetCOREInterface());
			pSettings = FindAppSetting(TEXT("Fallout 4"));
		}
	}
	bool FindFile(const tstring& name, tstring& resolved_name) const override
	{
		ResolveSettings();
		if (pSettings == nullptr) {
			resolved_name = name;
			return false;
		}
		return pSettings->FindFile(name, resolved_name);
	}
	bool FindFileByType(const tstring& name, FileType type, tstring& resolved_name) const override
	{
		ResolveSettings();
		if (pSettings == nullptr) {
			resolved_name = name;
			return false;
		}
		if (type == FT_Material) { resolved_name = pSettings->FindMaterial(name); return true; }
		if (type == FT_Texture) { resolved_name = pSettings->FindImage(name); return true; }
		return FindFile(name, resolved_name);
	}
	bool GetRelativePath(tstring& name, FileType type) const override
	{
		ResolveSettings();
		if (pSettings == nullptr) {
			return false;
		}
		if (type == FT_Texture) {
			name = pSettings->GetRelativeTexPath(name, TEXT("textures"));
			return true;
		}
		return false;
	}
};
static Fallout4FileResolver fo4Resolver;

class FO4ShaderDlg;

class FO4Shader : public Shader, IBSShaderMaterialData {
	friend class FO4ShaderCB;
	friend class FO4ShaderDlg;
	friend class FO4ShaderBaseRollup;
	friend class FO4ShaderMtlRollup;
	friend class FO4ShaderBGSMRollup;
	friend class FO4ShaderBGEMRollup;
	BOOL rolloutOpen;
	BOOL inSync;
public:
	IParamBlock2      *pb_base;     // ref 0
	IParamBlock2      *pb_mtl;      // ref 1
	IParamBlock2      *pb_bgsm;     // ref 2
	IParamBlock2      *pb_bgem;     // ref 3
	MaterialReference *pMtlFileRef; // ref 4
	MaterialReference *pMtlFileRefOld; // ref 5
	Interval    ivalid;
	FO4ShaderDlg* pDlg;

	BOOL bSelfIllumClrOn;
	Color cSelfIllumClr, cAmbientClr, cDiffuseClr, cSpecularClr;
	float fGlossiness, fSelfIllum, fSpecularLevel, fSoftenLevel;


public:
	FO4Shader();
	~FO4Shader();
	ULONG SupportStdParams() override { return SHADER_PARAMS; }

	// copy std params, for switching shaders
	void CopyStdParams(Shader* pFrom) override;

	// texture maps
	long nTexChannelsSupported() override { return STD2_NMAX_TEXMAPS - 4; }
	TSTR GetTexChannelName(long nChan) override { return GetString(texChannelNames[nChan].channelName); }
	TSTR GetTexChannelInternalName(long nChan) override { return GetString(texChannelNames[nChan].maxName); }
	long ChannelType(long nChan) override { return texChannelNames[nChan].channelType; }
	long StdIDToChannel(long stdID) override { return FO4ShaderStdIDToChannel[stdID]; }

	//BOOL KeyAtTime(int id,TimeValue t) { return pb->KeyFrameAtTime(id,t); }
	ULONG GetRequirements(int subMtlNum) override { return MTLREQ_TRANSP | MTLREQ_PHONG; }

	int NParamDlgs() override;
	ShaderParamDlg* GetParamDlg(int n) override;
	void SetParamDlg(ShaderParamDlg* newDlg, int n) override;
	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int) override;

	Class_ID ClassID() override { return FO4SHADER_CLASS_ID; }
	SClass_ID SuperClassID() override { return SHADER_CLASS_ID; }
	TSTR GetName() override { return GetString(IDS_FO4_SHADER); }
	void GetClassName(TSTR& s) override { s = GetName(); }
	void DeleteThis() override { delete this; }

	int NumSubs() override { return 1; }

	Animatable* SubAnim(int i) override;
	TSTR SubAnimName(int i) override;

	int SubNumToRefNum(int subNum) override { return subNum; }

	// add direct ParamBlock2 access
	int   NumParamBlocks() override { return 4; }
	IParamBlock2* GetParamBlock(int i) override;
	IParamBlock2* GetParamBlockByID(BlockID id) override;
	int NumRefs() override;

	RefTargetHandle GetReference(int i) override;
	void SetReference(int i, RefTargetHandle rtarg) override;
	void NotifyChanged();
	void ReloadDialog();

	void* GetInterface(ULONG id) override {
		return (id == I_BSSHADERDATA) ? static_cast<IBSShaderMaterialData*>(this) : Shader::GetInterface(id);
	}

	IOResult Load(ILoad* iload) override;
	IOResult Save(ISave* isave) override;

	void Update(TimeValue t, Interval& valid) override;
	void Reset() override;
	RefTargetHandle Clone(RemapDir &remap /*=DefaultRemapDir()*/) override;
#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) override;
#else
	RefResult	NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
#endif

	void GetIllumParams(ShadeContext &sc, IllumParams& ip) override;

	// Strauss Shader specific section
	void  Illum(ShadeContext &sc, IllumParams &ip) override;

	// std params not supported
	void SetLockDS(BOOL lock)  override { }
	BOOL GetLockDS()  override { return FALSE; }
	void SetLockAD(BOOL lock)  override { }
	BOOL GetLockAD()  override { return FALSE; }
	void SetLockADTex(BOOL lock)  override { }
	BOOL GetLockADTex()  override { return FALSE; }

	//virtual void SetSelfIllum(float v, TimeValue t)  override { }
	//virtual void SetSelfIllumClrOn(BOOL on)  override { }
	//virtual void SetSelfIllumClr(Color c, TimeValue t) override { }
	//virtual void SetAmbientClr(Color c, TimeValue t)  override { }
	//virtual void SetDiffuseClr(Color c, TimeValue t)  override { }
	//virtual void SetSpecularClr(Color c, TimeValue t)  override { }
	//virtual void SetGlossiness(float v, TimeValue t)  override { }
	//virtual void SetSpecularLevel(float v, TimeValue t)  override { }
	//virtual void SetSoftenLevel(float v, TimeValue t)  override { }

	//virtual BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace)  override { return IsSelfIllumClrOn(); }
	//virtual Color GetAmbientClr(int mtlNum, BOOL backFace)  override { return GetAmbientClr(0); }
	//virtual Color GetDiffuseClr(int mtlNum, BOOL backFace)  override { return GetDiffuseClr(0); }
	//virtual Color GetSpecularClr(int mtlNum, BOOL backFace)  override { return GetSpecularClr(0); }
	//virtual Color GetSelfIllumClr(int mtlNum, BOOL backFace)  override { return GetSelfIllumClr(0); }
	//virtual float GetSelfIllum(int mtlNum, BOOL backFace)  override { return GetSelfIllum(0); }
	//virtual float GetGlossiness(int mtlNum, BOOL backFace)  override { return GetGlossiness(0); }
	//virtual float GetSpecularLevel(int mtlNum, BOOL backFace)  override { return GetSpecularLevel(0); }
	//virtual float GetSoftenLevel(int mtlNum, BOOL backFace)  override { return GetSoftenLevel(0); }

	//virtual Color GetAmbientClr(TimeValue t)  override { return Color(0.588f, 0.588f, 0.588f); }
	//virtual Color GetDiffuseClr(TimeValue t)  override { return Color(0.588f, 0.588f, 0.588f); }
	//virtual Color GetSpecularClr(TimeValue t)  override { return Color(0.9f, 0.9f, 0.9f); }// { return pb_bgsm ? pb_bgsm->GetColor(fos_specularcolor) : Color(0.9f, 0.9f, 0.9f); }
	//virtual float GetGlossiness(TimeValue t)  override { return 1.0f; }//{ return pb_bgsm ? pb_bgsm->GetFloat(fos_smoothness) : 0.0f; }
	//virtual float GetSpecularLevel(TimeValue t)  override { return 0.0f; } //{ return pb_bgsm ? pb_bgsm->GetFloat(fos_specularmult) : 0.0f; }
	//virtual float GetSoftenLevel(TimeValue t)  override { return 0.1f; }
	//virtual BOOL IsSelfIllumClrOn()  override { return FALSE; } // return pb_bgsm && pb_bgsm->GetInt(fos_emitenabled) ? TRUE : FALSE; }
	//virtual float GetSelfIllum(TimeValue t)  override { return pb_bgsm ? pb_bgsm->GetFloat(fos_emittancemult) : 0.0f; }
	//virtual Color GetSelfIllumClr(TimeValue t)  override { return pb_bgsm ? pb_bgsm->GetColor(fos_emittancecolor) : Color(0.9f, 0.9f, 0.9f); }
	//virtual float EvalHiliteCurve2(float x, float y, int level = 0)  override { return 0.0f; }

	virtual void SetSelfIllum(float v, TimeValue t) { fSelfIllum = v; }
	virtual void SetSelfIllumClrOn(BOOL on) { bSelfIllumClrOn = on; }
	virtual void SetSelfIllumClr(Color c, TimeValue t) { cSelfIllumClr = c;}
	virtual void SetAmbientClr(Color c, TimeValue t) { cAmbientClr = c; }
	virtual void SetDiffuseClr(Color c, TimeValue t) { cDiffuseClr = c; }
	virtual void SetSpecularClr(Color c, TimeValue t) { cSpecularClr = c;}
	virtual void SetGlossiness(float v, TimeValue t) { fGlossiness = v;  }
	virtual void SetSpecularLevel(float v, TimeValue t) { fSpecularLevel = v;}
	virtual void SetSoftenLevel(float v, TimeValue t) { fSoftenLevel = v; }

	virtual BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace) { return bSelfIllumClrOn; }
	virtual Color GetAmbientClr(int mtlNum, BOOL backFace) { return cAmbientClr; }
	virtual Color GetDiffuseClr(int mtlNum, BOOL backFace) { return cDiffuseClr; }
	virtual Color GetSpecularClr(int mtlNum, BOOL backFace) { return cSpecularClr; }
	virtual Color GetSelfIllumClr(int mtlNum, BOOL backFace) { return cSelfIllumClr; }
	virtual float GetSelfIllum(int mtlNum, BOOL backFace) { return fSelfIllum; }
	virtual float GetGlossiness(int mtlNum, BOOL backFace) { return fGlossiness; }
	virtual float GetSpecularLevel(int mtlNum, BOOL backFace) { return fSpecularLevel; }
	virtual float GetSoftenLevel(int mtlNum, BOOL backFace) { return fSoftenLevel; }

	virtual BOOL IsSelfIllumClrOn() { return bSelfIllumClrOn; }
	virtual Color GetAmbientClr(TimeValue t) { return cAmbientClr; }
	virtual Color GetDiffuseClr(TimeValue t) { return cDiffuseClr; }
	virtual Color GetSpecularClr(TimeValue t) { return cSpecularClr; }
	virtual float GetGlossiness(TimeValue t) { return fGlossiness; }
	virtual float GetSpecularLevel(TimeValue t) { return fSpecularLevel; }
	virtual float GetSoftenLevel(TimeValue t) { return fSoftenLevel; }
	virtual float GetSelfIllum(TimeValue t) { return fSelfIllum; }
	virtual Color GetSelfIllumClr(TimeValue t) { return cSelfIllumClr; }
	virtual float EvalHiliteCurve2(float x, float y, int level = 0) { return 0.0f; }

	void SetPanelOpen(BOOL open) { rolloutOpen = open; }

	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rClr) override;

	float EvalHiliteCurve(float x)  override {
		double phExp = pow(2.0, GetGlossiness(0) * 10.0); // expensive.!! TBD
		return GetSpecularLevel(0)*static_cast<float>(pow(static_cast<double>(cos(x*PI)), phExp));
	}


	// IBSShaderMaterialData
	BOOL HasBGSM() const override { return pMtlFileRef && pMtlFileRef->ClassID() == BGSMFILE_CLASS_ID; }
	BGSMFile* GetBGSMData() const override { return HasBGSM() ? &static_cast<BGSMFileReference*>(pMtlFileRef)->file : nullptr; }
	BOOL LoadBGSM(BGSMFile&) override;

	BOOL HasBGEM() const override { return pMtlFileRef && pMtlFileRef->ClassID() == BGEMFILE_CLASS_ID; }
	BGEMFile* GetBGEMData() const override { return HasBGEM() ? &static_cast<BGEMFileReference*>(pMtlFileRef)->file : nullptr; }
	BOOL LoadBGEM(BGEMFile&) override;

	BaseMaterial* GetMtlData() const {
		return HasBGSM() ? static_cast<BaseMaterial*>(GetBGSMData()) : HasBGEM() ? static_cast<BaseMaterial*>(GetBGEMData()) : nullptr;
	}

	void FixRollups();
	BOOL ChangeShader(const Class_ID& clsid);

	//getName
	LPCTSTR GetMaterialName() const override { return pMtlFileRef ? pMtlFileRef->materialName : TEXT(""); }
	LPCTSTR GetFileName() const override { return pMtlFileRef ? pMtlFileRef->materialFileName : TEXT(""); }
	void SetMaterialName(LPCTSTR name) override { pMtlFileRef->SetName(name); }
	void SetFileName(LPCTSTR path) override { pMtlFileRef->SetFileName(path); }
	void SetFileName(LPCTSTR name, LPCTSTR path) const { pMtlFileRef->SetName(name), pMtlFileRef->SetFileName(path); }
	BOOL UpdateMaterial(StdMat2* mtl) override;
	void ClearDialogRollup(int rollup);
	typedef Texmap* (*pfCreateWrapper)(LPCTSTR name, Texmap* nmap);
	Texmap * GetOrCreateTexture(StdMat2* mtl, BaseMaterial* base_mtl, int map, tstring texture, IFileResolver* resolver, pfCreateWrapper wrapper = nullptr);
	BOOL LoadMaterial(StdMat2* mtl, IFileResolver* resolver) override;

	BOOL LoadMaterial(BaseMaterial&);
	Texmap* CreateTexture(const tstring& name, BaseMaterial* base_material, IFileResolver* resolver);
	bool UpdateTextureName(StdMat2* mtl, int map, tstring& texture, IFileResolver* resolver);
	bool UpdateTextureName(Texmap* tex, tstring& texture, IFileResolver* resolver);
	void SyncTexMapsToFilenames(StdMat2* mtl, IFileResolver* resolver);

};

///////////// Class Descriptor ////////////////////////
class FO4ShaderClassDesc :public ClassDesc2 {
public:
	int            IsPublic()  override { return TRUE; }
	void *         Create(BOOL loading = FALSE)  override { return new FO4Shader(); }
	const TCHAR *  ClassName()  override { return GetString(IDS_FO4_SHADER); }
	SClass_ID      SuperClassID()  override { return SHADER_CLASS_ID; }
	Class_ID       ClassID()  override { return FO4SHADER_CLASS_ID; }
	const TCHAR*   Category()  override { return GetString(IDS_CATEGORY); }
	const TCHAR*   InternalName()  override { return _T("FO4Shader"); }   // returns fixed parsable name (scripter-visible name)
	HINSTANCE      HInstance()  override { return hInstance; }          // returns owning module handle
};

FO4ShaderClassDesc FO4ShaderDesc;
extern ClassDesc2 * GetFO4ShaderDesc() { return &FO4ShaderDesc; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base Rollout

class BasePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;

		switch (id)
		{
		case fos_shader_type:
		{
			int idx = StringToEnum(v.s, MaterialFileTypesShort);
			if (idx == 0) pShader->ChangeShader(BGSMFILE_CLASS_ID);
			if (idx == 1) pShader->ChangeShader(BGEMFILE_CLASS_ID);
		}	break;
		case fos_name:
			pShader->SetMaterialName(v.s);
			break;
		case fos_filename:
			pShader->SetFileName(v.s);
			break;
		}
	}

	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;

		switch (id)
		{
		case fos_shader_type:
		{
			static TSTR name;
			name = EnumToStringRaw(pShader->HasBGEM() ? 1 : 0, MaterialFileTypesShort);
			v.s = const_cast<LPTSTR>(name.data());
		}	break;
		case fos_name:
		{ //Set v.s to the assetid so we are not loosing the source file name.
			static TSTR name;
			name = pShader->GetMaterialName();
			v.s = const_cast<LPTSTR>(name.data());
		}	break;
		case fos_filename:
		{ //Set v.s to the assetid so we are not loosing the source file name.
			static TSTR filename;
			filename = pShader->GetFileName();
			v.s = const_cast<LPTSTR>(filename.data());
		}	break;
		}
	}
};

static BasePBAccessor base_pb_accessor;

// extra rollout dialog proc
class BaseDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg)
		{
		case WM_INITDIALOG: {
			auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner());
			//m->UpdateExtraParams(m->GetShader()->SupportStdParams());
			return TRUE;
		}
		}
		return FALSE;
	}
	void DeleteThis() override { }
};

static BaseDlgProc baseDlgProc;

// base parameters
static ParamBlockDesc2 fos_base_blk(fos_shader, _T("baseShader"), 0, &FO4ShaderDesc, P_AUTO_CONSTRUCT /*+ P_AUTO_UI*/, ref_base,
	//rollout
	//IDD_FO4SHADER_BASE, IDS_FOS_BASENAME, 0, APPENDROLL_CLOSED, &baseDlgProc,
	// params

	fos_shader_type, _T("shaderType"), TYPE_STRING, P_TRANSIENT, IDS_FOS_SHADERTYPE,
	p_accessor, &base_pb_accessor,
	p_end,

	fos_name, _T("name"), TYPE_STRING, P_TRANSIENT, IDS_FOS_NAME,
	p_accessor, &base_pb_accessor,
	p_end,

	fos_filename, _T("fileName"), TYPE_STRING, P_TRANSIENT, IDS_FOS_FILENAME,
	p_accessor, &base_pb_accessor,
	p_end,

	p_end
	);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Material Rollout

class MaterialPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		auto* pValue = pShader->pMtlFileRef->GetBaseMaterial();
		if (pValue == nullptr)
			return;

		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		switch (id)
		{
		case fos_tileu: pValue->TileU = v.i ? true : false; break;
		case fos_tilev: pValue->TileV = v.i ? true : false; break;
		case fos_uoffset: pValue->UOffset = v.f; break;
		case fos_voffset: pValue->VOffset = v.f; break;
		case fos_uscale: pValue->UScale = v.f; break;
		case fos_vscale: pValue->VScale = v.f; break;
		case fos_alpha: pValue->Alpha = v.f; break;
		case fos_alphablendmode: pValue->AlphaBlendMode = AlphaBlendModeType(v.i); break;
		case fos_blendstate: pValue->BlendState = v.i ? true : false; break;
		case fos_blendfunc1: pValue->BlendFunc1 = AlphaBlendFunc(v.i); break;
		case fos_blendfunc2: pValue->BlendFunc2 = AlphaBlendFunc(v.i); break;
		case fos_alphatestref: pValue->AlphaTestRef = v.i; break;
		case fos_alphatest: pValue->AlphaTest = v.i ? true : false; break;
		case fos_zbufferwrite: pValue->ZBufferWrite = v.i ? true : false; break;
		case fos_zbuffertest: pValue->ZBufferTest = v.i ? true : false; break;
		case fos_screenspacereflections: pValue->ScreenSpaceReflections = v.i ? true : false; break;
		case fos_wetnesscontrolscreenspacereflections: pValue->WetnessControlScreenSpaceReflections = v.i ? true : false; break;
		case fos_decal: pValue->Decal = v.i ? true : false; break;
		case fos_twosided: pValue->TwoSided = v.i ? true : false; break;
		case fos_decalnofade: pValue->DecalNoFade = v.i ? true : false; break;
		case fos_nonoccluder: pValue->NonOccluder = v.i ? true : false; break;
		case fos_refraction: pValue->Refraction = v.i ? true : false; break;
		case fos_refractionfalloff: pValue->RefractionFalloff = v.i ? true : false; break;
		case fos_refractionpower: pValue->RefractionPower = v.f; break;
		case fos_environmentmapping: pValue->EnvironmentMapping = v.i ? true : false; break;
		case fos_environmentmappingmaskscale: pValue->EnvironmentMappingMaskScale = v.f; break;
		case fos_grayscaletopalettecolor: pValue->GrayscaleToPaletteColor = v.i ? true : false; break;
		}
	}

	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		auto* pValue = pShader->pMtlFileRef->GetBaseMaterial();
		if (pValue == nullptr)
			return;

		switch (id)
		{
		case fos_tileu: v.i = pValue->TileU ? TRUE : FALSE; break;
		case fos_tilev: v.i = pValue->TileV ? TRUE : FALSE; break;
		case fos_uoffset: v.f = pValue->UOffset; break;
		case fos_voffset: v.f = pValue->VOffset; break;
		case fos_uscale: v.f = pValue->UScale; break;
		case fos_vscale: v.f = pValue->VScale; break;
		case fos_alpha: v.f = pValue->Alpha; break;
		case fos_alphablendmode: v.i = pValue->AlphaBlendMode; break;
		case fos_blendstate: v.i = pValue->BlendState ? TRUE : FALSE; break;
		case fos_blendfunc1: v.i = pValue->BlendFunc1; break;
		case fos_blendfunc2: v.i = pValue->BlendFunc2; break;
		case fos_alphatestref: v.i = pValue->AlphaTestRef; break;
		case fos_alphatest: v.i = pValue->AlphaTest ? TRUE : FALSE; break;
		case fos_zbufferwrite: v.i = pValue->ZBufferWrite ? TRUE : FALSE; break;
		case fos_zbuffertest: v.i = pValue->ZBufferTest ? TRUE : FALSE; break;
		case fos_screenspacereflections: v.i = pValue->ScreenSpaceReflections ? TRUE : FALSE; break;
		case fos_wetnesscontrolscreenspacereflections: v.i = pValue->WetnessControlScreenSpaceReflections ? TRUE : FALSE; break;
		case fos_decal: v.i = pValue->Decal ? TRUE : FALSE; break;
		case fos_twosided: v.i = pValue->TwoSided ? TRUE : FALSE; break;
		case fos_decalnofade: v.i = pValue->DecalNoFade ? TRUE : FALSE; break;
		case fos_nonoccluder: v.i = pValue->NonOccluder ? TRUE : FALSE; break;
		case fos_refraction: v.i = pValue->Refraction ? TRUE : FALSE; break;
		case fos_refractionfalloff: v.i = pValue->RefractionFalloff ? TRUE : FALSE; break;
		case fos_refractionpower: v.f = pValue->RefractionPower; break;
		case fos_environmentmapping: v.i = pValue->EnvironmentMapping ? TRUE : FALSE; break;
		case fos_environmentmappingmaskscale: v.f = pValue->EnvironmentMappingMaskScale; break;
		case fos_grayscaletopalettecolor: v.i = pValue->GrayscaleToPaletteColor ? TRUE : FALSE; break;
		}
	}
};

static MaterialPBAccessor mtl_pb_accessor;

// extra rollout dialog proc
class MaterialDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg)
		{
		case WM_INITDIALOG: {
			//if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
			//	;
			//m->UpdateExtraParams(m->GetShader()->SupportStdParams());
			return TRUE;
		case WM_NCDESTROY:
		case WM_DESTROY:
		case WM_CLOSE:
			if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
				m->ClearDialogRollup(fos_mtl);
			return FALSE;
		}
		}
		return FALSE;
	}
	void DeleteThis() override { }
};

static MaterialDlgProc materialDlgProc;

// base parameters
static ParamBlockDesc2 fos_mtl_blk(fos_mtl, _T("baseMaterial"), 0, &FO4ShaderDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, ref_mtl,
	//rollout
	IDD_FO4SHADER_MTL, IDS_FOS_MTLNAME, 0, APPENDROLL_CLOSED, &materialDlgProc,
	// params

	fos_tileu, _T("tileU"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_TILEU,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_TILEU_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_tilev, _T("tileV"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_TILEV,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_TILEV_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_uoffset, _T("uOffset"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_UOFFSET,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_UOFFSET_EDIT, IDC_FOS_UOFFSET_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_voffset, _T("vOffset"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_VOFFSET,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_VOFFSET_EDIT, IDC_FOS_VOFFSET_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_uscale, _T("uScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_USCALE,
	p_default, 1.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_USCALE_EDIT, IDC_FOS_USCALE_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_vscale, _T("vScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_VSCALE,
	p_default, 1.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_VSCALE_EDIT, IDC_FOS_VSCALE_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_alpha, _T("alpha"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_ALPHA,
	p_default, 1.0,
	p_range, 0.0, 1.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_ALPHA_EDIT, IDC_FOS_ALPHA_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_alphablendmode, _T("alphaBlendMode"), TYPE_INT, P_TRANSIENT, IDS_FOS_ALPHABLENDMODE,
	p_default, 0,
	p_range, 0, 10,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_FOS_ALPHABLENDMODE_EDIT, IDC_FOS_ALPHABLENDMODE_SPIN, 1.0f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_blendstate, _T("blendState"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_BLENDSTATE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_BLENDSTATE_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_blendfunc1, _T("blendFuncSrc"), TYPE_INT, P_TRANSIENT, IDS_FOS_BLENDFUNC1,
	p_default, 0,
	p_range, 0, 10,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_FOS_BLENDFUNC1_EDIT, IDC_FOS_BLENDFUNC1_SPIN, 1.0f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_blendfunc2, _T("blendFuncDst"), TYPE_INT, P_TRANSIENT, IDS_FOS_BLENDFUNC2,
	p_default, 0,
	p_range, 0, 10,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_FOS_BLENDFUNC2_EDIT, IDC_FOS_BLENDFUNC2_SPIN, 1.0f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_alphatestref, _T("alphaTestRef"), TYPE_INT, P_TRANSIENT, IDS_FOS_ALPHATESTREF,
	p_default, 0,
	p_range, 0, 256,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_FOS_ALPHATESTREF_EDIT, IDC_FOS_ALPHATESTREF_SPIN, 1.0f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_alphatest, _T("alphaTest"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ALPHATEST,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ALPHATEST_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_zbufferwrite, _T("zBufferWrite"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ZBUFFERWRITE,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ZBUFFERWRITE_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_zbuffertest, _T("zBufferTest"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ZBUFFERTEST,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ZBUFFERTEST_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_screenspacereflections, _T("screenSpaceReflections"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SCREENSPACEREFLECTIONS,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SCREENSPACEREFLECTIONS_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_wetnesscontrolscreenspacereflections, _T("wetnessControlScreenSpaceReflections"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_WETNESSCONTROLSCREENSPACEREFLECTIONS,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_WETNESSCONTROLSCREENSPACEREFLECTIONS_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_decal, _T("decal"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_DECAL,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_DECAL_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_twosided, _T("twoSided"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_TWOSIDED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_TWOSIDED_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_decalnofade, _T("decalNoFade"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_DECALNOFADE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_DECALNOFADE_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_nonoccluder, _T("nonOccluder"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_NONOCCLUDER,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_NONOCCLUDER_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_refraction, _T("refraction"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_REFRACTION,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_REFRACTION_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_refractionfalloff, _T("refractionFalloff"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_REFRACTIONFALLOFF,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_REFRACTIONFALLOFF_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_refractionpower, _T("refractionPower"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_REFRACTIONPOWER,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_REFRACTIONPOWER_EDIT, IDC_FOS_REFRACTIONPOWER_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_environmentmapping, _T("environmentMapping"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ENVIRONMENTMAPPING,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ENVIRONMENTMAPPING_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_environmentmappingmaskscale, _T("environmentMappingMaskScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_ENVIRONMENTMAPPINGMASKSCALE,
	p_default, 1.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_ENVIRONMENTMAPPINGMASKSCALE_EDIT, IDC_FOS_ENVIRONMENTMAPPINGMASKSCALE_SPIN, 0.1f,
	p_accessor, &mtl_pb_accessor,
	p_end,

	fos_grayscaletopalettecolor, _T("grayscaleToPaletteColor"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_GRAYSCALETOPALETTECOLOR,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_GRAYSCALETOPALETTECOLOR_CHK,
	p_accessor, &mtl_pb_accessor,
	p_end,

	p_end
	);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BGSM Rollout

class BGSMPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)   override
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		auto* pValue = pShader->GetBGSMData();
		if (pValue == nullptr)
			return;

		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		switch (id)
		{
		case fos_diffusetexture: {  pValue->DiffuseTexture = v.s; } break;
		case fos_normaltexture: {  pValue->NormalTexture = v.s; } break;
		case fos_smoothspectexture: {  pValue->SmoothSpecTexture = v.s; } break;
		case fos_greyscaletexture: {  pValue->GreyscaleTexture = v.s; } break;
		case fos_envmaptexture: {  pValue->EnvmapTexture = v.s; } break;
		case fos_glowtexture: {  pValue->GlowTexture = v.s; } break;
		case fos_innerlayertexture: {  pValue->InnerLayerTexture = v.s; } break;
		case fos_wrinklestexture: {  pValue->WrinklesTexture = v.s; } break;
		case fos_displacementtexture: {  pValue->DisplacementTexture = v.s; } break;
		case fos_enableeditoralpharef: pValue->EnableEditorAlphaRef = v.i ? true : false; break;
		case fos_rimlighting: pValue->RimLighting = v.i ? true : false; break;
		case fos_rimpower: pValue->RimPower = v.f; break;
		case fos_backlightpower: pValue->BackLightPower = v.f; break;
		case fos_subsurfacelighting: pValue->SubsurfaceLighting = v.i ? true : false; break;
		case fos_subsurfacelightingrolloff: pValue->SubsurfaceLightingRolloff = v.f; break;
		case fos_specularenabled: pValue->SpecularEnabled = v.i ? true : false; break;
		case fos_specularcolor: if (v.p) pValue->SpecularColor = TOCOLOR3(*v.p); break;
		case fos_specularmult: pValue->SpecularMult = v.f; break;
		case fos_smoothness: pValue->Smoothness = v.f; break;
		case fos_fresnelpower: pValue->FresnelPower = v.f; break;
		case fos_wetnesscontrolspecscale: pValue->WetnessControlSpecScale = v.f; break;
		case fos_wetnesscontrolspecpowerscale: pValue->WetnessControlSpecPowerScale = v.f; break;
		case fos_wetnesscontrolspecminvar: pValue->WetnessControlSpecMinvar = v.f; break;
		case fos_wetnesscontrolenvmapscale: pValue->WetnessControlEnvMapScale = v.f; break;
		case fos_wetnesscontrolfresnelpower: pValue->WetnessControlFresnelPower = v.f; break;
		case fos_wetnesscontrolmetalness: pValue->WetnessControlMetalness = v.f; break;
		case fos_rootmaterialpath: {  pValue->RootMaterialPath = v.s; } break;
		case fos_anisolighting: pValue->AnisoLighting = v.i ? true : false; break;
		case fos_emitenabled: pValue->EmitEnabled = v.i ? true : false; break;
		case fos_emittancecolor: if (v.p) pValue->EmittanceColor = TOCOLOR3(*v.p); break;
		case fos_emittancemult: pValue->EmittanceMult = v.f; break;
		case fos_modelspacenormals: pValue->ModelSpaceNormals = v.i ? true : false; break;
		case fos_externalemittance: pValue->ExternalEmittance = v.i ? true : false; break;
		case fos_backlighting: pValue->BackLighting = v.i ? true : false; break;
		case fos_receiveshadows: pValue->ReceiveShadows = v.i ? true : false; break;
		case fos_hidesecret: pValue->HideSecret = v.i ? true : false; break;
		case fos_castshadows: pValue->CastShadows = v.i ? true : false; break;
		case fos_dissolvefade: pValue->DissolveFade = v.i ? true : false; break;
		case fos_assumeshadowmask: pValue->AssumeShadowmask = v.i ? true : false; break;
		case fos_glowmap: pValue->Glowmap = v.i ? true : false; break;
		case fos_environmentmappingwindow: pValue->EnvironmentMappingWindow = v.i ? true : false; break;
		case fos_environmentmappingeye: pValue->EnvironmentMappingEye = v.i ? true : false; break;
		case fos_hair: pValue->Hair = v.i ? true : false; break;
		case fos_hairtintcolor: if (v.p) pValue->HairTintColor = TOCOLOR3(*v.p); break;
		case fos_tree: pValue->Tree = v.i ? true : false; break;
		case fos_facegen: pValue->Facegen = v.i ? true : false; break;
		case fos_skintint: pValue->SkinTint = v.i ? true : false; break;
		case fos_tessellate: pValue->Tessellate = v.i ? true : false; break;
		case fos_displacementtexturebias: pValue->DisplacementTextureBias = v.f; break;
		case fos_displacementtexturescale: pValue->DisplacementTextureScale = v.f; break;
		case fos_tessellationpnscale: pValue->TessellationPNScale = v.f; break;
		case fos_tessellationbasefactor: pValue->TessellationBaseFactor = v.f; break;
		case fos_tessellationfadedistance: pValue->TessellationFadeDistance = v.f; break;
		case fos_grayscaletopalettescale: pValue->GrayscaleToPaletteScale = v.f; break;
		case fos_skewspecularalpha: pValue->SkewSpecularAlpha = v.i ? true : false; break;
		}
	}

	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) override
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		auto* pValue = pShader->GetBGSMData();
		if (pValue == nullptr)
			return;
		switch (id)
		{
		case fos_diffusetexture: {  static TSTR value; value = pValue->DiffuseTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_normaltexture: {  static TSTR value; value = pValue->NormalTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_smoothspectexture: {  static TSTR value; value = pValue->SmoothSpecTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_greyscaletexture: {  static TSTR value; value = pValue->GreyscaleTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_envmaptexture: {  static TSTR value; value = pValue->EnvmapTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_glowtexture: {  static TSTR value; value = pValue->GlowTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_innerlayertexture: {  static TSTR value; value = pValue->InnerLayerTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_wrinklestexture: {  static TSTR value; value = pValue->WrinklesTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_displacementtexture: {  static TSTR value; value = pValue->DisplacementTexture.c_str(); v.s = const_cast<LPTSTR>(value.data()); } break;
		case fos_enableeditoralpharef: v.i = pValue->EnableEditorAlphaRef ? TRUE : FALSE; break;
		case fos_rimlighting: v.i = pValue->RimLighting ? TRUE : FALSE; break;
		case fos_rimpower: v.f = pValue->RimPower; break;
		case fos_backlightpower: v.f = pValue->BackLightPower; break;
		case fos_subsurfacelighting: v.i = pValue->SubsurfaceLighting ? TRUE : FALSE; break;
		case fos_subsurfacelightingrolloff: v.f = pValue->SubsurfaceLightingRolloff; break;
		case fos_specularenabled: v.i = pValue->SpecularEnabled ? TRUE : FALSE; break;
		case fos_specularcolor: if (v.p) *v.p = TOCOLOR(pValue->SpecularColor); break;
		case fos_specularmult: v.f = pValue->SpecularMult; break;
		case fos_smoothness: v.f = pValue->Smoothness; break;
		case fos_fresnelpower: v.f = pValue->FresnelPower; break;
		case fos_wetnesscontrolspecscale: v.f = pValue->WetnessControlSpecScale; break;
		case fos_wetnesscontrolspecpowerscale: v.f = pValue->WetnessControlSpecPowerScale; break;
		case fos_wetnesscontrolspecminvar: v.f = pValue->WetnessControlSpecMinvar; break;
		case fos_wetnesscontrolenvmapscale: v.f = pValue->WetnessControlEnvMapScale; break;
		case fos_wetnesscontrolfresnelpower: v.f = pValue->WetnessControlFresnelPower; break;
		case fos_wetnesscontrolmetalness: v.f = pValue->WetnessControlMetalness; break;
		case fos_rootmaterialpath: {  static TSTR value; value = pValue->RootMaterialPath.c_str(); v.s = value; } break;
		case fos_anisolighting: v.i = pValue->AnisoLighting ? TRUE : FALSE; break;
		case fos_emitenabled: v.i = pValue->EmitEnabled ? TRUE : FALSE; break;
		case fos_emittancecolor: if (v.p) *v.p = TOCOLOR(pValue->EmittanceColor); break;
		case fos_emittancemult: v.f = pValue->EmittanceMult; break;
		case fos_modelspacenormals: v.i = pValue->ModelSpaceNormals ? TRUE : FALSE; break;
		case fos_externalemittance: v.i = pValue->ExternalEmittance ? TRUE : FALSE; break;
		case fos_backlighting: v.i = pValue->BackLighting ? TRUE : FALSE; break;
		case fos_receiveshadows: v.i = pValue->ReceiveShadows ? TRUE : FALSE; break;
		case fos_hidesecret: v.i = pValue->HideSecret ? TRUE : FALSE; break;
		case fos_castshadows: v.i = pValue->CastShadows ? TRUE : FALSE; break;
		case fos_dissolvefade: v.i = pValue->DissolveFade ? TRUE : FALSE; break;
		case fos_assumeshadowmask: v.i = pValue->AssumeShadowmask ? TRUE : FALSE; break;
		case fos_glowmap: v.i = pValue->Glowmap ? TRUE : FALSE; break;
		case fos_environmentmappingwindow: v.i = pValue->EnvironmentMappingWindow ? TRUE : FALSE; break;
		case fos_environmentmappingeye: v.i = pValue->EnvironmentMappingEye ? TRUE : FALSE; break;
		case fos_hair: v.i = pValue->Hair ? TRUE : FALSE; break;
		case fos_hairtintcolor: if (v.p) *v.p = TOCOLOR(pValue->HairTintColor); break;
		case fos_tree: v.i = pValue->Tree ? TRUE : FALSE; break;
		case fos_facegen: v.i = pValue->Facegen ? TRUE : FALSE; break;
		case fos_skintint: v.i = pValue->SkinTint ? TRUE : FALSE; break;
		case fos_tessellate: v.i = pValue->Tessellate ? TRUE : FALSE; break;
		case fos_displacementtexturebias: v.f = pValue->DisplacementTextureBias; break;
		case fos_displacementtexturescale: v.f = pValue->DisplacementTextureScale; break;
		case fos_tessellationpnscale: v.f = pValue->TessellationPNScale; break;
		case fos_tessellationbasefactor: v.f = pValue->TessellationBaseFactor; break;
		case fos_tessellationfadedistance: v.f = pValue->TessellationFadeDistance; break;
		case fos_grayscaletopalettescale: v.f = pValue->GrayscaleToPaletteScale; break;
		case fos_skewspecularalpha: v.i = pValue->SkewSpecularAlpha ? TRUE : FALSE; break;
		}
	}
};

static BGSMPBAccessor bgsm_pb_accessor;

// extra rollout dialog proc
class BGSMDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg)
		{
		case WM_INITDIALOG: {
			//if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
			//	;
			//m->UpdateExtraParams(m->GetShader()->SupportStdParams());
			return TRUE;
		case WM_NCDESTROY:
		case WM_DESTROY:
		case WM_CLOSE:
			if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
				m->ClearDialogRollup(fos_bgsm);
			return FALSE;
		}
		}
		return FALSE;
	}
	void DeleteThis() override { }
};

static BGSMDlgProc bgsmDlgProc;

// base parameters
static ParamBlockDesc2 fos_bgsm_blk(fos_bgsm, _T("bgsmMaterial"), 0, &FO4ShaderDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, ref_bgsm,
	//rollout
	IDD_FO4SHADER_BGSM, IDS_FOS_BGSMNAME, 0, APPENDROLL_CLOSED, &bgsmDlgProc,
	// params

	fos_diffusetexture, _T("diffuseTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_DIFFUSETEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_DIFFUSETEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_normaltexture, _T("normalTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_NORMALTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_NORMALTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_smoothspectexture, _T("smoothSpecTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_SMOOTHSPECTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_SMOOTHSPECTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_greyscaletexture, _T("greyscaleTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_GREYSCALETEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_GREYSCALETEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_envmaptexture, _T("envmapTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_ENVMAPTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_ENVMAPTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_glowtexture, _T("glowTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_GLOWTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_GLOWTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_innerlayertexture, _T("innerLayerTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_INNERLAYERTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_INNERLAYERTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wrinklestexture, _T("wrinklesTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_WRINKLESTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_WRINKLESTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_displacementtexture, _T("displacementTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_DISPLACEMENTTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_DISPLACEMENTTEXTURE_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_enableeditoralpharef, _T("enableEditorAlphaRef"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ENABLEEDITORALPHAREF,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ENABLEEDITORALPHAREF_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_rimlighting, _T("rimLighting"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_RIMLIGHTING,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_RIMLIGHTING_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_rimpower, _T("rimPower"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_RIMPOWER,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_RIMPOWER_EDIT, IDC_FOS_RIMPOWER_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_backlightpower, _T("backLightPower"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_BACKLIGHTPOWER,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_BACKLIGHTPOWER_EDIT, IDC_FOS_BACKLIGHTPOWER_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_subsurfacelighting, _T("subsurfaceLighting"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SUBSURFACELIGHTING,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SUBSURFACELIGHTING_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_subsurfacelightingrolloff, _T("subsurfaceLightingRolloff"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_SUBSURFACELIGHTINGROLLOFF,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_SUBSURFACELIGHTINGROLLOFF_EDIT, IDC_FOS_SUBSURFACELIGHTINGROLLOFF_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_specularenabled, _T("specularEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SPECULARENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SPECULARENABLED_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_specularcolor, _T("specularColor"), TYPE_RGBA, P_TRANSIENT, IDS_FOS_SPECULARCOLOR,
	p_ui, TYPE_COLORSWATCH, IDC_FOS_SPECULARCOLOR_COLOR,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_specularmult, _T("specularMult"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_SPECULARMULT,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_SPECULARMULT_EDIT, IDC_FOS_SPECULARMULT_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_smoothness, _T("smoothness"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_SMOOTHNESS,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_SMOOTHNESS_EDIT, IDC_FOS_SMOOTHNESS_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_fresnelpower, _T("fresnelPower"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_FRESNELPOWER,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_FRESNELPOWER_EDIT, IDC_FOS_FRESNELPOWER_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolspecscale, _T("wetnessControlSpecScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLSPECSCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLSPECSCALE_EDIT, IDC_FOS_WETNESSCONTROLSPECSCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolspecpowerscale, _T("wetnessControlSpecPowerScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLSPECPOWERSCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLSPECPOWERSCALE_EDIT, IDC_FOS_WETNESSCONTROLSPECPOWERSCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolspecminvar, _T("wetnessControlSpecMinvar"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLSPECMINVAR,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLSPECMINVAR_EDIT, IDC_FOS_WETNESSCONTROLSPECMINVAR_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolenvmapscale, _T("wetnessControlEnvMapScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLENVMAPSCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLENVMAPSCALE_EDIT, IDC_FOS_WETNESSCONTROLENVMAPSCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolfresnelpower, _T("wetnessControlFresnelPower"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLFRESNELPOWER,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLFRESNELPOWER_EDIT, IDC_FOS_WETNESSCONTROLFRESNELPOWER_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_wetnesscontrolmetalness, _T("wetnessControlMetalness"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_WETNESSCONTROLMETALNESS,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_WETNESSCONTROLMETALNESS_EDIT, IDC_FOS_WETNESSCONTROLMETALNESS_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_rootmaterialpath, _T("rootMaterialPath"), TYPE_STRING, P_TRANSIENT, IDS_FOS_ROOTMATERIALPATH,
	p_ui, TYPE_EDITBOX, IDC_FOS_ROOTMATERIALPATH_EDIT,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_anisolighting, _T("anisoLighting"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ANISOLIGHTING,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ANISOLIGHTING_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_emitenabled, _T("emitEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_EMITENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_EMITENABLED_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_emittancecolor, _T("emittanceColor"), TYPE_RGBA, P_TRANSIENT, IDS_FOS_EMITTANCECOLOR,
	p_ui, TYPE_COLORSWATCH, IDC_FOS_EMITTANCECOLOR_COLOR,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_emittancemult, _T("emittanceMult"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_EMITTANCEMULT,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_EMITMULT_EDIT, IDC_FOS_EMITMULT_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_modelspacenormals, _T("modelSpaceNormals"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_MODELSPACENORMALS,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_MODELSPACENORMALS_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_externalemittance, _T("externalEmittance"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_EXTERNALEMITTANCE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_EXTERNALEMITTANCE_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_backlighting, _T("backLighting"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_BACKLIGHTING,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_BACKLIGHTING_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_receiveshadows, _T("receiveShadows"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_RECEIVESHADOWS,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_RECEIVESHADOWS_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_hidesecret, _T("hideSecret"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_HIDESECRET,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_HIDESECRET_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_castshadows, _T("castShadows"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_CASTSHADOWS,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_CASTSHADOWS_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_dissolvefade, _T("dissolveFade"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_DISSOLVEFADE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_DISSOLVEFADE_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_assumeshadowmask, _T("assumeShadowmask"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ASSUMESHADOWMASK,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ASSUMESHADOWMASK_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_glowmap, _T("glowmap"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_GLOWMAP,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_GLOWMAP_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_environmentmappingwindow, _T("environmentMappingWindow"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ENVIRONMENTMAPPINGWINDOW,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ENVIRONMENTMAPPINGWINDOW_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_environmentmappingeye, _T("environmentMappingEye"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_ENVIRONMENTMAPPINGEYE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_ENVIRONMENTMAPPINGEYE_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_hair, _T("hair"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_HAIR,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_HAIR_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_hairtintcolor, _T("hairTintColor"), TYPE_RGBA, P_TRANSIENT, IDS_FOS_HAIRTINTCOLOR,
	p_ui, TYPE_COLORSWATCH, IDC_FOS_HAIRTINTCOLOR_COLOR,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_tree, _T("tree"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_TREE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_TREE_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_facegen, _T("facegen"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_FACEGEN,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_FACEGEN_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_skintint, _T("skinTint"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SKINTINT,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SKINTINT_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_tessellate, _T("tessellate"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_TESSELLATE,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_TESSELLATE_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_displacementtexturebias, _T("displacementTextureBias"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_DISPLACEMENTTEXTUREBIAS,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_DISPLACEMENTTEXTUREBIAS_EDIT, IDC_FOS_DISPLACEMENTTEXTUREBIAS_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_displacementtexturescale, _T("displacementTextureScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_DISPLACEMENTTEXTURESCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_DISPLACEMENTTEXTURESCALE_EDIT, IDC_FOS_DISPLACEMENTTEXTURESCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_tessellationpnscale, _T("tessellationPNScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_TESSELLATIONPNSCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_TESSELLATIONPNSCALE_EDIT, IDC_FOS_TESSELLATIONPNSCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_tessellationbasefactor, _T("tessellationBaseFactor"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_TESSELLATIONBASEFACTOR,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_TESSELLATIONBASEFACTOR_EDIT, IDC_FOS_TESSELLATIONBASEFACTOR_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_tessellationfadedistance, _T("tessellationFadeDistance"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_TESSELLATIONFADEDISTANCE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_TESSELLATIONFADEDISTANCE_EDIT, IDC_FOS_TESSELLATIONFADEDISTANCE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_grayscaletopalettescale, _T("grayscaleToPaletteScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_GRAYSCALETOPALETTESCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_GRAYSCALETOPALETTESCALE_EDIT, IDC_FOS_GRAYSCALETOPALETTESCALE_SPIN, 0.1f,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	fos_skewspecularalpha, _T("skewSpecularAlpha"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SKEWSPECULARALPHA,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SKEWSPECULARALPHA_CHK,
	p_accessor, &bgsm_pb_accessor,
	p_end,

	p_end
	);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BGEM Rollout

class BGEMPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)   override
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		auto* pValue = pShader->GetBGEMData();
		if (pValue == nullptr)
			return;
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		switch (id)
		{
		case fos_basetexture: {  pValue->BaseTexture = v.s; } break;
		case fos_grayscaletexture: {  pValue->GrayscaleTexture = v.s; } break;
		case fos_envmaptexture2: {  pValue->EnvmapTexture = v.s; } break;
		case fos_normaltexture2: {  pValue->NormalTexture = v.s; } break;
		case fos_envmapmasktexture: {  pValue->EnvmapMaskTexture = v.s; } break;
		case fos_bloodenabled: pValue->BloodEnabled = v.i ? true : false; break;
		case fos_effectlightingenabled: pValue->EffectLightingEnabled = v.i ? true : false; break;
		case fos_falloffenabled: pValue->FalloffEnabled = v.i ? true : false; break;
		case fos_falloffcolorenabled: pValue->FalloffColorEnabled = v.i ? true : false; break;
		case fos_grayscaletopalettealpha: pValue->GrayscaleToPaletteAlpha = v.i ? true : false; break;
		case fos_softenabled: pValue->SoftEnabled = v.i ? true : false; break;
		case fos_basecolor: if (v.p) pValue->BaseColor = TOCOLOR3(*v.p); break;
		case fos_basecolorscale: pValue->BaseColorScale = v.f; break;
		case fos_falloffstartangle: pValue->FalloffStartAngle = v.f; break;
		case fos_falloffstopangle: pValue->FalloffStopAngle = v.f; break;
		case fos_falloffstartopacity: pValue->FalloffStartOpacity = v.f; break;
		case fos_falloffstopopacity: pValue->FalloffStopOpacity = v.f; break;
		case fos_lightinginfluence: pValue->LightingInfluence = v.f; break;
		case fos_envmapminlod: pValue->EnvmapMinLOD = v.i; break;
		case fos_softdepth: pValue->SoftDepth = v.f; break;
		}
	}

	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) override
	{
		auto* pShader = static_cast<FO4Shader*>(owner);
		IParamMap2* map = pShader->pb_base ? pShader->pb_base->GetMap() : NULL;
		auto* pValue = pShader->GetBGEMData();
		if (pValue == nullptr)
			return;
		switch (id)
		{
		case fos_basetexture: {  static TSTR value; value = pValue->BaseTexture.c_str(); v.s = value; } break;
		case fos_grayscaletexture: {  static TSTR value; value = pValue->GrayscaleTexture.c_str(); v.s = value; } break;
		case fos_envmaptexture2: {  static TSTR value; value = pValue->EnvmapTexture.c_str(); v.s = value; } break;
		case fos_normaltexture2: {  static TSTR value; value = pValue->NormalTexture.c_str(); v.s = value; } break;
		case fos_envmapmasktexture: {  static TSTR value; value = pValue->EnvmapMaskTexture.c_str(); v.s = value; } break;
		case fos_bloodenabled: v.i = pValue->BloodEnabled ? TRUE : FALSE; break;
		case fos_effectlightingenabled: v.i = pValue->EffectLightingEnabled ? TRUE : FALSE; break;
		case fos_falloffenabled: v.i = pValue->FalloffEnabled ? TRUE : FALSE; break;
		case fos_falloffcolorenabled: v.i = pValue->FalloffColorEnabled ? TRUE : FALSE; break;
		case fos_grayscaletopalettealpha: v.i = pValue->GrayscaleToPaletteAlpha ? TRUE : FALSE; break;
		case fos_softenabled: v.i = pValue->SoftEnabled ? TRUE : FALSE; break;
		case fos_basecolor: if (v.p) *v.p = TOCOLOR(pValue->BaseColor); break;
		case fos_basecolorscale: v.f = pValue->BaseColorScale; break;
		case fos_falloffstartangle: v.f = pValue->FalloffStartAngle; break;
		case fos_falloffstopangle: v.f = pValue->FalloffStopAngle; break;
		case fos_falloffstartopacity: v.f = pValue->FalloffStartOpacity; break;
		case fos_falloffstopopacity: v.f = pValue->FalloffStopOpacity; break;
		case fos_lightinginfluence: v.f = pValue->LightingInfluence; break;
		case fos_envmapminlod: v.i = pValue->EnvmapMinLOD; break;
		case fos_softdepth: v.f = pValue->SoftDepth; break;
		}
	}
};

static BGEMPBAccessor bgem_pb_accessor;

// extra rollout dialog proc
class BGEMDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg)
		{
		case WM_INITDIALOG: {
			//if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
			//	;
			//m->UpdateExtraParams(m->GetShader()->SupportStdParams());
			return TRUE;
		case WM_NCDESTROY:
		case WM_DESTROY:
		case WM_CLOSE:
			if (auto* m = static_cast<FO4Shader*>(map->GetParamBlock()->GetOwner()))
				m->ClearDialogRollup(fos_bgem);
			return FALSE;
		}
		}
		return FALSE;
	}
	void DeleteThis() override { }
};

static BGEMDlgProc bgemDlgProc;

// base parameters
static ParamBlockDesc2 fos_bgem_blk(fos_bgem, _T("bgemMaterial"), 0, &FO4ShaderDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, ref_bgem,
	//rollout
	IDD_FO4SHADER_BGEM, IDS_FOS_BGEMNAME, 0, APPENDROLL_CLOSED, &bgemDlgProc,
	// params
	fos_basetexture, _T("baseTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_BASETEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_BASETEXTURE_EDIT,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_grayscaletexture, _T("grayscaleTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_GRAYSCALETEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_GRAYSCALETEXTURE_EDIT,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_envmaptexture2, _T("envmapTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_ENVMAPTEXTURE2,
	p_ui, TYPE_EDITBOX, IDC_FOS_ENVMAPTEXTURE2_EDIT,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_normaltexture2, _T("normalTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_NORMALTEXTURE2,
	p_ui, TYPE_EDITBOX, IDC_FOS_NORMALTEXTURE2_EDIT,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_envmapmasktexture, _T("envmapMaskTexture"), TYPE_STRING, P_TRANSIENT, IDS_FOS_ENVMAPMASKTEXTURE,
	p_ui, TYPE_EDITBOX, IDC_FOS_ENVMAPMASKTEXTURE_EDIT,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_bloodenabled, _T("bloodEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_BLOODENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_BLOODENABLED_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_effectlightingenabled, _T("effectLightingEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_EFFECTLIGHTINGENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_EFFECTLIGHTINGENABLED_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffenabled, _T("falloffEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_FALLOFFENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_FALLOFFENABLED_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffcolorenabled, _T("falloffColorEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_FALLOFFCOLORENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_FALLOFFCOLORENABLED_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_grayscaletopalettealpha, _T("grayscaleToPaletteAlpha"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_GRAYSCALETOPALETTEALPHA,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_GRAYSCALETOPALETTEALPHA_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_softenabled, _T("softEnabled"), TYPE_BOOL, P_TRANSIENT, IDS_FOS_SOFTENABLED,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, IDC_FOS_SOFTENABLED_CHK,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_basecolor, _T("baseColor"), TYPE_RGBA, P_TRANSIENT, IDS_FOS_BASECOLOR,
	p_ui, TYPE_COLORSWATCH, IDC_FOS_BASECOLOR_COLOR,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_basecolorscale, _T("baseColorScale"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_BASECOLORSCALE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_BASECOLORSCALE_EDIT, IDC_FOS_BASECOLORSCALE_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffstartangle, _T("falloffStartAngle"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_FALLOFFSTARTANGLE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_FALLOFFSTARTANGLE_EDIT, IDC_FOS_FALLOFFSTARTANGLE_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffstopangle, _T("falloffStopAngle"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_FALLOFFSTOPANGLE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_FALLOFFSTOPANGLE_EDIT, IDC_FOS_FALLOFFSTOPANGLE_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffstartopacity, _T("falloffStartOpacity"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_FALLOFFSTARTOPACITY,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_FALLOFFSTARTOPACITY_EDIT, IDC_FOS_FALLOFFSTARTOPACITY_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_falloffstopopacity, _T("falloffStopOpacity"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_FALLOFFSTOPOPACITY,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_FALLOFFSTOPOPACITY_EDIT, IDC_FOS_FALLOFFSTOPOPACITY_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_lightinginfluence, _T("lightingInfluence"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_LIGHTINGINFLUENCE,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_LIGHTINGINFLUENCE_EDIT, IDC_FOS_LIGHTINGINFLUENCE_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_envmapminlod, _T("envmapMinLOD"), TYPE_INT, P_TRANSIENT, IDS_FOS_ENVMAPMINLOD,
	p_default, 0,
	p_range, 0, 256,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_FOS_ENVMAPMINLOD_EDIT, IDC_FOS_ENVMAPMINLOD_SPIN, 1.0f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	fos_softdepth, _T("softDepth"), TYPE_FLOAT, P_TRANSIENT, IDS_FOS_SOFTDEPTH,
	p_default, 0.0,
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOS_SOFTDEPTH_EDIT, IDC_FOS_SOFTDEPTH_SPIN, 0.1f,
	p_accessor, &bgem_pb_accessor,
	p_end,

	p_end
	);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FO4Shader::FO4Shader()
{
	pb_base = nullptr;
	pb_mtl = nullptr;
	pb_bgsm = nullptr;
	pb_bgem = nullptr;
	pMtlFileRef = nullptr;
	pMtlFileRefOld = nullptr;
	FO4ShaderDesc.MakeAutoParamBlocks(this);   // make and intialize paramblock2
	pDlg = nullptr;

	ivalid.SetEmpty();
	rolloutOpen = TRUE;
	inSync = FALSE;

	FO4Shader::Reset();
}

FO4Shader::~FO4Shader()
{

}

void FO4Shader::CopyStdParams(Shader* pFrom)
{
	// We don't want to see this parameter copying in macrorecorder
	macroRecorder->Disable();

	TimeValue t = 0;

	SetSelfIllum(pFrom->GetSelfIllum(0, 0), t);
	SetSelfIllumClrOn(pFrom->IsSelfIllumClrOn(0, 0));
	SetSelfIllumClr(pFrom->GetSelfIllumClr(0, 0), t);
	SetAmbientClr(pFrom->GetAmbientClr(0, 0), t);
	SetDiffuseClr(pFrom->GetDiffuseClr(0, 0), t);
	SetSpecularClr(pFrom->GetSpecularClr(0, 0), t);
	SetGlossiness(pFrom->GetGlossiness(0, 0), t);
	SetSpecularLevel(pFrom->GetSpecularLevel(0, 0), t);
	SetSoftenLevel(pFrom->GetSoftenLevel(0, 0), t);

	macroRecorder->Enable();
	ivalid.SetEmpty();
}


RefTargetHandle FO4Shader::Clone(RemapDir &remap)
{
	FO4Shader* pShader = new FO4Shader();
	// dont copy the oldmtl
	for (int i = 0; i < MAX_REFERENCES - 1; ++i)
		pShader->ReplaceReference(i, remap.CloneRef(GetReference(i)));
	return pShader;
}

Animatable* FO4Shader::SubAnim(int i)
{
	switch (i) {
	case 0: return pb_base;
	case 1: return pb_mtl;
	case 2: return pb_bgsm;
	case 3: return pb_bgem;
	}
	return nullptr;
}

TSTR FO4Shader::SubAnimName(int i)
{
	USES_CONVERSION;
	switch (i) {
	case 0: return TSTR(GetString(IDS_FOS_BASENAME));
	case 1: return TSTR(GetString(IDS_FOS_MTLNAME));
	case 2: return TSTR(GetString(IDS_FOS_BGSMNAME));
	case 3: return TSTR(GetString(IDS_FOS_BGEMNAME));
	}
	return TSTR(GetString(IDS_FOS_BASENAME));
}

IParamBlock2* FO4Shader::GetParamBlock(int i)
{
	switch (i) {
	case 0: return pb_base;
	case 1: return pb_mtl;
	case 2: return pb_bgsm;
	case 3: return pb_bgem;
	}
	return nullptr;
}

IParamBlock2* FO4Shader::GetParamBlockByID(BlockID id)
{
	if (pb_base->ID() == id) return pb_base;
	if (pb_mtl->ID() == id) return pb_mtl;
	if (pb_bgsm->ID() == id) return pb_bgsm;
	if (pb_bgem->ID() == id) return pb_bgem;
	return nullptr;
}

int FO4Shader::NumRefs() {
	return MAX_REFERENCES;
}

RefTargetHandle FO4Shader::GetReference(int i)
{
	switch (i) {
	case ref_base: return pb_base;
	case ref_mtl: return pb_mtl;
	case ref_bgsm: return pb_bgsm;
	case ref_bgem: return pb_bgem;
	case ref_activemtl: return pMtlFileRef;
	case ref_oldmtl: return pMtlFileRefOld;
	}
	return nullptr;
}

void FO4Shader::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
	case ref_base: pb_base = static_cast<IParamBlock2*>(rtarg); return;
	case ref_mtl: pb_mtl = static_cast<IParamBlock2*>(rtarg); return;
	case ref_bgsm: pb_bgsm = static_cast<IParamBlock2*>(rtarg); return;
	case ref_bgem: pb_bgem = static_cast<IParamBlock2*>(rtarg); return;
	case ref_activemtl: pMtlFileRef = static_cast<MaterialReference*>(rtarg); return;
	case ref_oldmtl: pMtlFileRefOld = static_cast<MaterialReference*>(rtarg); return;
	}
	assert(0);
}

IOResult FO4Shader::Load(ILoad* iload)
{
	return Shader::Load(iload);
}

IOResult FO4Shader::Save(ISave* isave)
{
	return Shader::Save(isave);
}

void FO4Shader::Update(TimeValue t, Interval &valid) {
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
	}
	valid &= ivalid;
}

void FO4Shader::Reset()
{
	FO4ShaderDesc.MakeAutoParamBlocks(this);

	ivalid.SetEmpty();
	macroRecorder->Disable();  // don't want to see this parameter reset in macrorecorder

	SetSoftenLevel(0.1f, 0);
	SetAmbientClr(Color(0.588f, 0.588f, 0.588f), 0);
	SetDiffuseClr(Color(0.588f, 0.588f, 0.588f), 0);
	SetSpecularClr(Color(0.9f, 0.9f, 0.9f), 0);
	SetGlossiness(0.10f, 0);   // change from .25, 11/6/00
	SetSpecularLevel(0.0f, 0);

	SetSelfIllum(.0f, 0);
	SetSelfIllumClr(Color(.0f, .0f, .0f), 0);
	SetSelfIllumClrOn(FALSE);
	SetLockADTex(TRUE);
	SetLockAD(TRUE); // DS 10/26/00: changed to TRUE
	SetLockDS(FALSE);

	// create reference for BGSM file
	ReplaceReference(ref_activemtl, new BGSMFileReference(), 1);
	//ReplaceReference(ref_oldmtl, new BGSMFileReference(), 1);

	macroRecorder->Enable();
}

///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//
//#define USE_BLINN_SHADER
#define USE_STRAUSS_SHADER
//#define USE_CUSTOM_SHADER
#ifdef USE_BLINN_SHADER

void FO4Shader::GetIllumParams(ShadeContext &sc, IllumParams& ip)
{
	ip.stdParams = SupportStdParams();
	// ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	TimeValue t = 0;
	ip.channels[C_BASE] = GetDiffuseClr();
	//ip.channels[(ID_DI)] = GetDiffuseClr();
	//ip.channels[StdIDToChannel(ID_SP)] = GetSpecularClr();
	//ip.channels[StdIDToChannel(ID_SH)].r = GetGlossiness();
	//ip.channels[StdIDToChannel(ID_SS)].r = GetSpecularLevel();
	//if( IsSelfIllumClrOn() )
	//ip.channels[C_GLOW] = pb->GetColor(ns_mat_emittance, 0, 0);
	//else
	//   ip.channels[C_GLOW].r = ip.channels[C_GLOW].g = ip.channels[C_GLOW].b = GetSelfIllum();
}


void FO4Shader::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc* l = nullptr;
	Color lightCol;

	// Blinn style phong
	BOOL is_shiny = (ip.channels[StdIDToChannel(ID_SS)].r > 0.0f) ? 1 : 0;
	double phExp = pow(2.0, ip.channels[StdIDToChannel(ID_SH)].r * 10.0) * 4.0; // expensive.!!  TBD

	for (int i = 0; i < sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (l->Illuminate(sc, sc.Normal(), lightCol, L, NL, diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
			}
			if (NL <= 0.0f)
				continue;

			// diffuse
			if (l->affectDiffuse)
				ip.diffIllumOut += diffCoef * lightCol;

			// specular (Phong2) 
			if (is_shiny&&l->affectSpecular) {
				Point3 H = Normalize(L - sc.V());
				float c = DotProd(sc.Normal(), H);
				if (c > 0.0f) {
					c = (float)pow((double)c, phExp); // could use table lookup for speed
					ip.specIllumOut += c * ip.channels[StdIDToChannel(ID_SS)].r * lightCol;
				}
			}
		}
	}


	// Apply mono self illumination
	if (!IsSelfIllumClrOn()) {
		// lerp between diffuse & white
		// changed back, fixed in getIllumParams, KE 4/27
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
		if (si > 0.0f) {
			si = UBound(si);
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f - si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f - si;
		}
	}
	else {
		// colored self illum, 
		ip.selfIllumOut += ip.channels[ID_SI];
	}
	// now we can multiply by the clrs, 
	ip.ambIllumOut *= ip.channels[ID_AM];
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[ID_DI];
	ip.specIllumOut *= ip.channels[ID_SP];

	ShadeTransmission(sc, ip, ip.channels[ID_RR], ip.refractAmt);
	ShadeReflection(sc, ip, ip.channels[ID_RL]);
	CombineComponents(sc, ip);
}

void FO4Shader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{
	rcol *= ip.channels[ID_SP];
};

#endif
#ifdef USE_STRAUSS_SHADER

// my magic constants
static float SpecBoost = 1.3f;

// Strauss's Magic Constants
static float kf = 1.12f;
static float kf2 = 1.0f / (kf * kf);
static float kf3 = 1.0f / ((1.0f - kf) * (1.0f - kf));
static float kg = 1.01f;
static float kg2 = 1.0f / (kg * kg);
static float kg3 = 1.0f / ((1.0f - kg) * (1.0f - kg));
static float kj = 0.1f; //.1 strauss

static float OneOverHalfPi = 1.0f / (0.5f * Pi);

inline float F(float x) {
	float xb = Bound(x);
	float xkf = 1.0f / ((xb - kf)*(xb - kf));
	return (xkf - kf2) / (kf3 - kf2);
}

inline float G(float x) {
	float xb = Bound(x);
	float xkg = 1.0f / ((xb - kg)*(xb - kg));
	return (kg3 - xkg) / (kg3 - kg2);
}

#define REFL_BRIGHTNESS_ADJUST   3.0f;

void FO4Shader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rClr)
{
	float opac = 0.0f;
	float g = ip.channels[C_SMOOTHSPEC].r;
	float m = 0.0f;
	Color Cd = ip.channels[C_DIFFUSE];

	float rn = opac - (1.0f - g * g * g) * opac;

	// the reflection of the reflection vector is just the view vector
	// so dot(v, r) is 1, to any power is still 1
	float a, b;
	// NB: this has been transformed for existing in-pointing v
	float NV = Dot(sc.V(), sc.Normal());
	Point3 R = sc.V() - 2.0f * NV * sc.Normal();
	float NR = Dot(sc.Normal(), R);
	a = (float)acos(NR) * OneOverHalfPi;
	b = (float)acos(NV) * OneOverHalfPi;

	float fa = F(a);
	float j = fa * G(a) * G(b);
	float rj = Bound(rn + (rn + kj)*j);
	Color white(1.0f, 1.0f, 1.0f);

	Color Cs = white + m * (1.0f - fa) * (Cd - white);
	rClr *= Cs * rj * REFL_BRIGHTNESS_ADJUST;
}

static int stopX = -1;
static int stopY = -1;

static float   greyVal = 0.3f;
static float   clrVal = 0.3f;

static float   softThresh = 0.15f;

void FO4ShaderCombineComponents(ShadeContext &sc, IllumParams& ip)
{
	float o = (ip.hasComponents & HAS_REFRACT) ? ip.finalAttenuation : 1.0f;

	ip.finalC = o * (ip.ambIllumOut + ip.diffIllumOut) + ip.specIllumOut
		+ ip.reflIllumOut + ip.transIllumOut;
}


void FO4Shader::GetIllumParams(ShadeContext &sc, IllumParams& ip)
{
	ip.stdParams = SupportStdParams();
	// ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	//ip.shFlags = 0;
	TimeValue t = 0;
	ip.channels[C_DIFFUSE] = GetDiffuseClr(0);
	ip.channels[C_SMOOTHSPEC] = GetSpecularClr(0);
	if (IsSelfIllumClrOn())
		ip.channels[C_GLOW] = GetSelfIllumClr(0);
	else
		ip.channels[C_GLOW] = Color(0, 0, 0);

	//ip.channels[C_BASE] = GetDiffuseClr();
	//ip.channels[C_OPACITY] = Color(1.0f, 1.0f, 1.0f);
	//ip.channels[(ID_DI)] = GetDiffuseClr();
	//ip.channels[StdIDToChannel(ID_SP)] = GetSpecularClr();
	//ip.channels[StdIDToChannel(ID_SH)].r = GetGlossiness();
	//ip.channels[StdIDToChannel(ID_SS)].r = GetSpecularLevel();
	//if (IsSelfIllumClrOn())
	//	ip.channels[C_GLOW] = GetSelfIllumClr(0);
	//else
	//	ip.channels[C_GLOW].r = ip.channels[C_GLOW].g = ip.channels[C_GLOW].b = GetSelfIllum();
}


void FO4Shader::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *l;
	Color lightClr;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if (sp.x == stopX && sp.y == stopY)
		sp.x = stopX;
#endif

	//float opac = ip.channels[C_OPACITY].r;
	//float opac = ip.channels[C_].r;
	float opac = 1.0f;
	//float g = ip.channels[C_SMOOTHSPEC].r;
	float g = 0.10;
	float m = 0.0f;
	Color Cd = ip.channels[C_DIFFUSE];
	// BOOL dimDiffuse = ip.hasComponents & HAS_REFLECT;
	BOOL dimDiffuse = ip.hasComponents & HAS_REFLECT_MAP;

	float rd;
	float g3 = Cube(g);
	if (dimDiffuse)
		rd = (1.0f - g3) * opac;
	else
		rd = (1.0f - m * g3) * opac;  //ke 10/28/98

	float rn = opac - (1.0f - g3) * opac;

	float h = (g == 1.0f) ? 600.0f : 3.0f / (1.0f - g);
	float d = 1.0f - m * g;

	for (int i = 0; i < sc.nLights; i++) {
		l = sc.Light(i);
		float NL, Kl;
		Point3 L;
		if (l->Illuminate(sc, sc.Normal(), lightClr, L, NL, Kl)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightClr;
				continue;
			}
			if (NL <= 0.0f)
				continue;

			// diffuse
			if (l->affectDiffuse) {
				ip.diffIllumOut += Kl * d * rd * lightClr;
			}

			// specular  
			if (l->affectSpecular) {
				// strauss uses the reflected LIGHT vector
				Point3 R = L - 2.0f * NL * sc.Normal();
				R = Normalize(R);

				float RV = -Dot(R, sc.V());

				float s;
				if (RV < 0.0f) {
					// soften
					if (NL < softThresh)
						RV *= SoftSpline2(NL / softThresh);
					// specular function
					s = SpecBoost * (float)pow(-RV, h);
				}
				else
					continue;

				float a, b;
				a = (float)acos(NL) * OneOverHalfPi;
				b = (float)acos(-Dot(sc.Normal(), sc.V())) * OneOverHalfPi;

				float fa = F(a);
				float j = fa * G(a) * G(b);
				float rj = rn > 0.0f ? Bound(rn + (rn + kj)*j) : rn;
				Color Cl = lightClr;
				// normalize the light color in case it's really bright
				float I = NormClr(Cl);
				Color Cs = Cl + m * (1.0f - fa) * (Cd - Cl);

				ip.specIllumOut += s * rj * I * Cs;

			} // p_end, if specular
		}  // p_end, illuminate

	} // for each light

	// now we can multiply by the clrs, except specular, which is already done
	ip.ambIllumOut *= 0.5f * rd * Cd;
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= Cd;

	// next due reflection
	if (ip.hasComponents & HAS_REFLECT) {
		Color rc = ip.channels[ip.stdIDToChannel[ID_RL]];
		AffectReflection(sc, ip, rc);
		ip.reflIllumOut = rc * ip.reflectAmt;
	}

	// last do refraction/ opacity
	if ((ip.hasComponents & HAS_REFRACT)) {
		// Set up attenuation opacity for Refraction map. dim diffuse & spec by this
		ip.finalAttenuation = ip.finalOpac * (1.0f - ip.refractAmt);

		// Make more opaque where specular hilite occurs:
		float max = Max(ip.specIllumOut);
		if (max > 1.0f) max = 1.0f;
		float newOpac = ip.finalAttenuation + max - ip.finalAttenuation * max;

		// Evaluate refraction map, filtered by filter color.
		//    Color tClr = ((StdMat2*)(ip.pMtl))->TranspColor( newOpac, ip.channels[filtChan], ip.channels[diffChan]);
		//Color tClr = transpColor( TRANSP_FILTER, newOpac, Cd, Cd );
		//ip.transIllumOut = ip.channels[ ip.stdIDToChannel[ ID_RR ] ] * tClr;
		ip.transIllumOut = Color(0.0f, 0.0f, 0.0f);

		// no transparency when doing refraction
		ip.finalT.Black();

	}
	else {
		// no refraction, transparent?
		ip.finalAttenuation = opac;
		if (ip.hasComponents & HAS_OPACITY) {
			// ip.finalT = Cd * (1.0f-opac);
			Cd = greyVal * Color(1.0f, 1.0f, 1.0f) + clrVal * Cd;
			ip.finalT = transpColor(TRANSP_FILTER, opac, Cd, Cd);
		}
	}

	FO4ShaderCombineComponents(sc, ip);
}
#endif
#ifdef USE_CUSTOM_SHADER

//---------------------------------------------------------------------------
// Called to combine the various color and shading components
void FO4ShaderCombineComponents(ShadeContext &sc, IllumParams& ip)
{
	ip.finalC = (ip.ambIllumOut + ip.diffIllumOut + ip.selfIllumOut) + ip.specIllumOut;
}

//---------------------------------------------------------------------------
void FO4Shader::GetIllumParams(ShadeContext &sc, IllumParams &ip)
{
	ip.stdParams = SupportStdParams();
	ip.channels[C_BASE] = pb->GetColor(ns_mat_diffuse, 0, 0);
	ip.channels[C_GLOW] = pb->GetColor(ns_mat_selfillumclr, 0, 0);
	ip.channels[C_SPECULAR] = pb->GetColor(ns_mat_specular, 0, 0);

}

//---------------------------------------------------------------------------
void FO4Shader::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *pLight;
	Color lightCol;

	// Get our parameters our of the channels
	Color base = ip.channels[C_BASE];
	Color dark = ip.channels[C_DARK];
	Color detail = ip.channels[C_DETAIL];
	Color decal1 = ip.channels[C_DECAL1];
	Color bump = ip.channels[C_BUMP];
	Color gloss = ip.channels[C_GLOSS];
	Color glow = ip.channels[C_GLOW];
	Color reflection = ip.channels[C_REFLECTION];
	Color specular = ip.channels[C_SPECULAR];
	Color emittance = pb->GetColor(ns_mat_emittance);
	int iApplyMode = pb->GetInt(ns_apply_mode);
	bool bSpecularOn = pb->GetInt(ns_mat_specenable) != 0;
	float fShininess = pb->GetFloat(ns_mat_shininess);
	Color ambient = Color(pb->GetColor(ns_mat_ambient));

	ip.specIllumOut.Black();

	if (iApplyMode)
	{
		for (int i = 0; i < sc.nLights; i++)
		{
			register float fNdotL, fDiffCoef;
			Point3 L;

			pLight = sc.Light(i);
#if MAX_RELEASE < 4000
			if (pLight->Illuminate(sc, ip.N, lightCol, L, fNdotL, fDiffCoef))
#else
			if (pLight->Illuminate(sc, sc.Normal(), lightCol, L, fNdotL, fDiffCoef))
#endif
			{
				if (pLight->ambientOnly)
				{
					ip.ambIllumOut += lightCol;
					continue;
				}

				if (fNdotL <= 0.0f)
					continue;

				if (pLight->affectDiffuse)
					ip.diffIllumOut += fDiffCoef * lightCol;

				if (bSpecularOn && pLight->affectSpecular)
				{
#if MAX_RELEASE < 4000
					Point3 H = Normalize(L - ip.V);
					float c = DotProd(ip.N, H);
#else
					Point3 H = Normalize(L - sc.V());
					float c = DotProd(sc.Normal(), H);
#endif
					if (c > 0.0f)
					{
						c = (float)pow(c, fShininess);
						// c * bright * lightCol;
						ip.specIllumOut += c * lightCol;
					}
				}
			}
		}
	}
	else
	{
		ip.ambIllumOut.Black();
		ip.diffIllumOut.White();
	}

	ip.ambIllumOut *= ambient;
	ip.diffIllumOut *= dark * (base * detail);  // + decal;
	ip.selfIllumOut = emittance + glow + decal1;
	ip.specIllumOut *= specular;

	FO4ShaderCombineComponents(sc, ip);
}


//---------------------------------------------------------------------------
void FO4Shader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{
	rcol *= ip.channels[C_SPECULAR];
};
#endif



BOOL FO4Shader::LoadMaterial(BaseMaterial& bgsm)
{
	return TRUE;
}



static Texmap* CreateNormalBump(LPCTSTR name, Texmap* nmap)
{
	Interface *gi = GetCOREInterface();
	Texmap *texmap = (Texmap*)gi->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID);
	if (texmap != nullptr)
	{
		TSTR tname = (name == nullptr) ? FormatText(TEXT("Norm %s"), nmap->GetName().data()) : TSTR(name);
		texmap->SetName(tname);
		texmap->SetSubTexmap(0, nmap);
		return texmap;
	}
	return nmap;
}

static Texmap* CreateMask(LPCTSTR name, Texmap* map, Texmap* mask)
{
	Interface *gi = GetCOREInterface();
	Texmap *texmap = (Texmap*)gi->CreateInstance(TEXMAP_CLASS_ID, Class_ID(MASK_CLASS_ID, 0));
	if (texmap != nullptr)
	{
		TSTR tname = (name == nullptr) ? FormatText(TEXT("Mask %s"), map->GetName().data()) : TSTR(name);
		texmap->SetName(tname);
		texmap->SetSubTexmap(0, map);
		texmap->SetSubTexmap(1, mask);
		return texmap;
	}
	return map;
}

static Texmap* MakeAlpha(Texmap* tex)
{
	if (BitmapTex *bmp_tex = GetIBitmapTextInterface(tex)) {
		bmp_tex->SetName(TEXT("Alpha"));
		bmp_tex->SetAlphaAsMono(TRUE);
		bmp_tex->SetAlphaSource(ALPHA_FILE);
		bmp_tex->SetPremultAlpha(FALSE);
		bmp_tex->SetOutputLevel(INFINITE, 0.0f);
	}
	return tex;
}

Texmap* FO4Shader::CreateTexture(const tstring& name, BaseMaterial* base_material, IFileResolver* resolver)
{
	USES_CONVERSION;
	if (name.empty())
		return nullptr;

	BitmapManager *bmpMgr = TheManager;
	if (bmpMgr->CanImport(name.c_str())) {
		BitmapTex *bmpTex = NewDefaultBitmapTex();

		tstring filename;
		if (resolver)
			resolver->FindFileByType(name, IFileResolver::FT_Texture, filename);
		else
			filename = name;
		bmpTex->SetName(name.c_str());
		bmpTex->SetMapName(const_cast<LPTSTR>(filename.c_str()));
		bmpTex->SetAlphaAsMono(TRUE);
		bmpTex->SetAlphaSource(0/*ALPHA_NONE*/);

		bmpTex->SetFilterType(FILTER_PYR);

		if (true) {
			bmpTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
			bmpTex->ActivateTexDisplay(TRUE);
			bmpTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}

		if (UVGen *uvGen = bmpTex->GetTheUVGen()) {
			int textureTiling = ((base_material->TileU ? 0x1 : 0) | (base_material->TileV ? 0x2 : 0));
			uvGen->SetTextureTiling(textureTiling);
			if (RefTargetHandle ref = uvGen->GetReference(0)) {
				setMAXScriptValue(ref, TEXT("U_Offset"), 0, base_material->UOffset);
				setMAXScriptValue(ref, TEXT("V_Offset"), 0, base_material->VOffset);
				setMAXScriptValue(ref, TEXT("U_Tiling"), 0, base_material->UScale);
				setMAXScriptValue(ref, TEXT("V_Tiling"), 0, base_material->VScale);
			}
		}

		return bmpTex;
	}
	return nullptr;
}

void FO4Shader::SyncTexMapsToFilenames(StdMat2* mtl, IFileResolver* resolver)
{
	if (inSync)
		return;

	BaseMaterial* base_mtl = GetMtlData();
	if (base_mtl == nullptr)
		return;

	try
	{
		inSync = TRUE;
		if (this->HasBGSM())
		{
			auto* bgsm = GetBGSMData();

			UpdateTextureName(mtl, C_DIFFUSE, bgsm->DiffuseTexture, resolver);
			UpdateTextureName(mtl, C_SMOOTHSPEC, bgsm->SmoothSpecTexture, resolver);
			UpdateTextureName(mtl, C_GREYSCALE, bgsm->GreyscaleTexture, resolver);
			UpdateTextureName(mtl, C_GLOW, bgsm->GlowTexture, resolver);
			UpdateTextureName(mtl, C_INNERLAYER, bgsm->InnerLayerTexture, resolver);
			UpdateTextureName(mtl, C_WRINKLES, bgsm->WrinklesTexture, resolver);
			UpdateTextureName(mtl, C_DISPLACE, bgsm->DisplacementTexture, resolver);

			Texmap * tex = mtl->GetSubTexmap(C_NORMAL);
			if (tex && (tex->ClassID() == GNORMAL_CLASS_ID)) {
				Texmap *normal = tex->GetSubTexmap(0);
				UpdateTextureName(normal, bgsm->NormalTexture, resolver);
			}
			else {
				UpdateTextureName(tex, bgsm->NormalTexture, resolver);
			}
			tex = mtl->GetSubTexmap(C_ENVMAP);
			if (tex && tex->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
				Texmap *envMap = tex->GetSubTexmap(0);
				UpdateTextureName(envMap, bgsm->EnvmapTexture, resolver);
			}
			else {
				UpdateTextureName(tex, bgsm->EnvmapTexture, resolver);
			}
		}
		if (this->HasBGEM())
		{
			auto* bgem = GetBGEMData();

			UpdateTextureName(mtl, C_DIFFUSE, bgem->BaseTexture, resolver);
			UpdateTextureName(mtl, C_GREYSCALE, bgem->GrayscaleTexture, resolver);
			Texmap * tex = mtl->GetSubTexmap(C_NORMAL);
			if (tex && (tex->ClassID() == GNORMAL_CLASS_ID)) {
				Texmap *normal = tex->GetSubTexmap(0);
				UpdateTextureName(normal, bgem->NormalTexture, resolver);
			}
			else {
				UpdateTextureName(tex, bgem->NormalTexture, resolver);
			}
			tex = mtl->GetSubTexmap(C_ENVMAP);
			if (tex->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
				Texmap *envMap = tex->GetSubTexmap(0);
				UpdateTextureName(envMap, bgem->EnvmapTexture, resolver);
				Texmap *envMapMask = tex->GetSubTexmap(1);
				UpdateTextureName(envMapMask, bgem->EnvmapMaskTexture, resolver);
			}
			else {
				UpdateTextureName(tex, bgem->EnvmapTexture, resolver);
			}
		}
	}
	catch (...)
	{

	}
	inSync = FALSE;
}

BOOL FO4Shader::UpdateMaterial(StdMat2* mtl)
{
	return TRUE;
}

Texmap * FO4Shader::GetOrCreateTexture(StdMat2* mtl, BaseMaterial* base_mtl, int map, tstring texture, IFileResolver* resolver, pfCreateWrapper wrapper)
{
	Texmap * tex = mtl->GetSubTexmap(map);
	if (tex != nullptr && strmatch(tex->GetName(), texture))
		return tex;

	if (tex && tex->NumSubTexmaps() > 0) {
		tex = tex->GetSubTexmap(0);
		if (tex != nullptr && strmatch(tex->GetName(), texture))
			return tex;
	}
	if (texture.empty())
		return nullptr;

	tex = CreateTexture(texture, base_mtl, resolver);
	tex = wrapper ? wrapper(nullptr, tex) : tex;
	if (tex) {
		mtl->SetSubTexmap(map, tex);
	}
	return tex;
}

bool FO4Shader::UpdateTextureName(StdMat2* mtl, int map, tstring& texture, IFileResolver* resolver)
{
	return UpdateTextureName(mtl->GetSubTexmap(map), texture, resolver);
}

bool FO4Shader::UpdateTextureName(Texmap * tex, tstring& texture, IFileResolver* resolver)
{
	if (tex == nullptr) {
		texture.clear();
		return true;
	}
	if (strmatch(tex->GetName(), texture))
		return true;
	tstring fname;
	if (GetTexFullName(tex, fname))
	{
		if (resolver->GetRelativePath(fname, IFileResolver::FT_Texture)) {
			if (strmatch(fname, texture))
				return true;
			texture = fname;
			return true;
		}
	}
	return false;
}


BOOL FO4Shader::LoadMaterial(StdMat2* mtl, IFileResolver* resolver)
{
	BaseMaterial* base_mtl = GetMtlData();
	if (base_mtl == nullptr)
		return FALSE;

	BOOL oldInSync = inSync;
	inSync = TRUE; // prevent updates to textures while assigning textures
	try
	{
		// handle base material stuff
		mtl->SetTwoSided(base_mtl->TwoSided ? TRUE : FALSE);
		mtl->SetOpacity(base_mtl->Alpha, INFINITE);

		if (this->HasBGSM())
		{
			auto* bgsm = GetBGSMData();
			mtl->SetSpecular(TOCOLOR(bgsm->SpecularColor), 0);
			mtl->GetSelfIllumColorOn(bgsm->Glowmap);

			GetOrCreateTexture(mtl, base_mtl, C_DIFFUSE, bgsm->DiffuseTexture, resolver);
			if (Texmap *tex = GetOrCreateTexture(mtl, base_mtl, C_NORMAL, bgsm->NormalTexture, resolver, CreateNormalBump)) 
				mtl->SetTexmapAmt(C_NORMAL, 0.3f, INFINITE);
			GetOrCreateTexture(mtl, base_mtl, C_SMOOTHSPEC, bgsm->SmoothSpecTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_GREYSCALE, bgsm->GreyscaleTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_ENVMAP, bgsm->EnvmapTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_GLOW, bgsm->GlowTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_INNERLAYER, bgsm->InnerLayerTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_WRINKLES, bgsm->WrinklesTexture, resolver);
			GetOrCreateTexture(mtl, base_mtl, C_DISPLACE, bgsm->DisplacementTexture, resolver);
		}
		if (this->HasBGEM())
		{
			auto* bgem = GetBGEMData();

			if (Texmap* tex = CreateTexture(bgem->BaseTexture, base_mtl, resolver))
				mtl->SetSubTexmap(C_DIFFUSE, tex);
			if (Texmap* tex = CreateTexture(bgem->NormalTexture, base_mtl, resolver))
				mtl->SetSubTexmap(C_NORMAL, CreateNormalBump(nullptr, tex));
			if (Texmap* tex = CreateTexture(bgem->EnvmapTexture, base_mtl, resolver)) {
				if (Texmap* mask = CreateTexture(bgem->EnvmapMaskTexture, base_mtl, resolver)) {
					tex = CreateMask(nullptr, tex, mask);
					mtl->SetSubTexmap(C_ENVMAP, tex);
					mtl->SetTexmapAmt(C_ENVMAP, 0.0f, INFINITE);
					tex->SetOutputLevel(INFINITE, 0.0f);
				}
			}
			if (Texmap* tex = CreateTexture(bgem->GrayscaleTexture, base_mtl, resolver))
				mtl->SetSubTexmap(C_GREYSCALE, tex);
		}
	}
	catch (...)
	{
	}
	inSync = oldInSync;
	return TRUE;
}

BOOL FO4Shader::ChangeShader(const Class_ID& clsid)
{
	// careful with references in this routine
	if (clsid == BGSMFILE_CLASS_ID) {
		if (!HasBGSM()) {
			MaterialReference* oldmtl = static_cast<MaterialReference*>(GetReference(ref_oldmtl));
			MaterialReference* curmtl = static_cast<MaterialReference*>(GetReference(ref_activemtl));
			if (!oldmtl || oldmtl->ClassID() == clsid) { // swap
				pMtlFileRef = oldmtl;
				pMtlFileRefOld = curmtl;
			}
		}
		if (GetReference(ref_activemtl) == nullptr) {
			ReplaceReference(ref_activemtl, new BGSMFileReference(), TRUE);
		}
	}
	else if (clsid == BGEMFILE_CLASS_ID) {
		if (!HasBGEM()) {
			MaterialReference* oldmtl = static_cast<MaterialReference*>(GetReference(ref_oldmtl));
			MaterialReference* curmtl = static_cast<MaterialReference*>(GetReference(ref_activemtl));
			if (!oldmtl || oldmtl->ClassID() == clsid) { // swap
				pMtlFileRef = oldmtl;
				pMtlFileRefOld = curmtl;
			}
		}
		if (GetReference(ref_activemtl) == nullptr) {
			ReplaceReference(ref_activemtl, new BGEMFileReference(), TRUE);
		}
	}
	FixRollups();
	return TRUE;
}


BOOL FO4Shader::LoadBGSM(BGSMFile& bgsm)
{
	// use the bgsm file as primary
	//LoadMaterial(bgsm);
	ChangeShader(BGSMFILE_CLASS_ID);
	if (HasBGSM()) {
		*GetBGSMData() = bgsm;
	}
	return TRUE;
}

BOOL FO4Shader::LoadBGEM(BGEMFile& bgem)
{
	ChangeShader(BGEMFILE_CLASS_ID);
	if (HasBGEM()) {
		*GetBGEMData() = bgem;
	}
	return TRUE;
}

class FO4ShaderRollupBase
{
public:
	FO4ShaderDlg *pDlg;
	HPALETTE hOldPal;
	HWND     hwHilite;   // the hilite window
	HWND     hRollup; // Rollup panel for Base Materials
	BOOL     isActive;
	BOOL     inUpdate;
	BOOL     valid;
	BOOL     preserveRollup;

	virtual ~FO4ShaderRollupBase();

	virtual void UpdateHilite();
	virtual void UpdateColSwatches() {}
	virtual void UpdateMapButtons();
	virtual void UpdateOpacity() { UpdateHilite(); }

	virtual void InitializeControls(HWND hwnd) = 0;
	virtual void ReleaseControls() = 0;
	virtual void UpdateControls() = 0;
	virtual void CommitValues() = 0;
	virtual void UpdateVisible() = 0;
	virtual void NotifyChanged();

	virtual void UpdateMtlDisplay();

	virtual void LoadPanel(BOOL);
	virtual void ReloadPanel();
	void FreeRollup();

	virtual INT_PTR PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	FO4ShaderRollupBase(FO4ShaderDlg *pDlg);
};

class FO4ShaderBaseRollup;
class FO4ShaderMtlRollup;
class FO4ShaderBGSMRollup;
class FO4ShaderBGEMRollup;

#pragma region ("FO4 Shader Dialog")

///////////////////////////////////////////////////////////////////////////////////
//
// Fallout4 Base Material dlg panel
//
class FO4ShaderDlg : public ShaderParamDlg {
public:
	FO4Shader* pShader;
	StdMat2* pMtl;
	IMtlParams* pMtlPar;
	HWND     hwmEdit; // window handle of the materials editor dialog
	FO4ShaderRollupBase *pBaseRollup;
	//FO4ShaderRollupBase *pMtlRollup;
	//FO4ShaderRollupBase *pBGSMRollup;
	//FO4ShaderRollupBase *pBGEMRollup;
	IAutoMParamDlg *pMtlRollup;
	IAutoMParamDlg *pBGSMRollup;
	IAutoMParamDlg *pBGEMRollup;

	TimeValue   curTime;
	BOOL     isActive;
	BOOL     inUpdate;


	FO4ShaderDlg(HWND hwMtlEdit, IMtlParams *pParams);
	~FO4ShaderDlg();
	void DeleteThis() override {
		DeleteRollups();
		if (pShader) pShader->pDlg = nullptr;
		pShader = nullptr;
		delete this;
	}

	// Methods
	Class_ID ClassID()  override { return FO4SHADER_CLASS_ID; }

	void SetThing(ReferenceTarget *m)  override { pMtl = static_cast<StdMat2*>(m); }

	void SetThings(StdMat2* theMtl, Shader* theShader) override;

	ReferenceTarget* GetThing()  override { return pMtl; } // mtl is the thing! (for DAD!)
	Shader* GetShader()  override { return pShader; }

	void SetTime(TimeValue t)  override {
		//DS 2/26/99: added interval test to prevent redrawing when not necessary
		curTime = t;
		if (!pShader->ivalid.InInterval(t)) {
			Interval v;
			pShader->Update(t, v);
			LoadDialog(TRUE);
		}
		else
			UpdateOpacity();  // always update opacity since it's not in validity computations
	}
	//BOOL KeyAtCurTime(int id) { return pShader->KeyAtTime(id,curTime); } 
	void ActivateDlg(BOOL dlgOn)  override { isActive = dlgOn; }
	HWND GetHWnd()  override {
		return pBaseRollup ? pBaseRollup->hRollup : nullptr;
	}
	void NotifyChanged()  const { pShader->NotifyChanged(); }
	void LoadDialog(BOOL draw) override;
	void ReloadDialog()  override { Interval v; pShader->Update(pMtlPar->GetTime(), v); LoadDialog(FALSE); }
	void UpdateDialog(ParamID paramId)  override { if (!inUpdate) ReloadDialog(); }

	void UpdateMtlDisplay()  const { pMtlPar->MtlChanged(); } // redraw viewports

															  // required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw)  override {
		return -1;
	}
	void DeleteRollups();

	void UpdateHilite();
	void UpdateColSwatches();
	void UpdateMapButtons() override;
	void UpdateOpacity() override;

	INT_PTR PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) override;

};

static void MapNotify(void *param, NotifyInfo *info)
{
	FO4Shader *pShader = (FO4Shader *)param;
	switch (info->intcode)
	{
	case NOTIFY_BITMAP_CHANGED:
		//pShader->
		break;
	}
}


FO4ShaderDlg::FO4ShaderDlg(HWND hwMtlEdit, IMtlParams *pParams)
{
	hwmEdit = hwMtlEdit;
	pBaseRollup = nullptr;
	pMtlRollup = nullptr;
	pBGSMRollup = nullptr;
	pBGEMRollup = nullptr;
	pMtl = nullptr;
	pShader = nullptr;
	pMtlPar = pParams;
	curTime = pMtlPar->GetTime();
	isActive = FALSE;
	inUpdate = FALSE;
}

FO4ShaderDlg::~FO4ShaderDlg()
{
	DeleteRollups();

	if (pShader) pShader->SetParamDlg(nullptr, 0);
}

void FO4ShaderDlg::SetThings(StdMat2* theMtl, Shader* theShader)
{
	if (pShader) pShader->SetParamDlg(nullptr, 0);
	pShader = static_cast<FO4Shader*>(theShader);
	if (pShader)pShader->SetParamDlg(this, 0);
	pMtl = theMtl;

	//pMtl->GetReference(MAPS_PB_REF);
//	pMtl->NotifyRefChanged()
//	RegisterNotification(MapNotify, pShader, REFMSG_CHANGE);
//;	MAPS_PB_REF
}

void  FO4ShaderDlg::LoadDialog(BOOL draw)
{
	if (pShader) {
		if (pBaseRollup) pBaseRollup->UpdateControls();
		//if (pMtlRollup) pMtlRollup->InvalidateUI();
		//if (pMtlRollup) pMtlRollup->UpdateControls();
		//if (pBGSMRollup) pBGSMRollup->UpdateControls();
		//if (pBGEMRollup) pBGEMRollup->UpdateControls();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void FO4ShaderDlg::UpdateMapButtons()
{
	if (pBaseRollup) pBaseRollup->UpdateMapButtons();
	//if (pMtlRollup) pMtlRollup->UpdateMapButtons();
	//if (pBGSMRollup) pBGSMRollup->UpdateMapButtons();
	//if (pBGEMRollup) pBGEMRollup->UpdateMapButtons();

	// update the filenames in the maps

	pShader->SyncTexMapsToFilenames(this->pMtl, &fo4Resolver);
}


void FO4ShaderDlg::UpdateOpacity()
{
	if (pBaseRollup) pBaseRollup->UpdateOpacity();
	//if (pMtlRollup) pMtlRollup->UpdateOpacity();
	//if (pBGSMRollup) pBGSMRollup->UpdateOpacity();
	//if (pBGEMRollup) pBGEMRollup->UpdateOpacity();

	//trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	//trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}

INT_PTR FO4ShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//if (pBaseRollup) pBaseRollup->PanelProc();
	//if (pMtlRollup) pMtlRollup->PanelProc();
	//if (pBGSMRollup) pBGSMRollup->PanelProc();
	//if (pBGEMRollup) pBGEMRollup->PanelProc();
	return FALSE;
}

void FO4ShaderDlg::UpdateColSwatches()
{
	//cs[0]->SetKeyBrackets( pShader->KeyAtTime(ns_mat_diffuse,curTime) );
	//cs[0]->SetColor( pShader->GetDiffuseClr() );
	if (pBaseRollup) pBaseRollup->UpdateColSwatches();
	//if (pMtlRollup) pMtlRollup->UpdateColSwatches();
	//if (pBGSMRollup) pBGSMRollup->UpdateColSwatches();
	//if (pBGEMRollup) pBGEMRollup->UpdateColSwatches();
}


void FO4ShaderDlg::UpdateHilite()
{
	if (pBaseRollup) pBaseRollup->UpdateHilite();
	//if (pMtlRollup) pMtlRollup->UpdateHilite();
	//if (pBGSMRollup) pBGSMRollup->UpdateHilite();
	//if (pBGEMRollup) pBGEMRollup->UpdateHilite();
}

#pragma endregion 

#pragma region ("Rollups")

class FO4ShaderBaseRollup : public FO4ShaderRollupBase
{
	typedef FO4ShaderRollupBase base;
public:
	ICustEdit *p_name_edit;
	ICustButton *iLoadBtn;
	ICustButton *iSaveBtn;

	FO4ShaderBaseRollup(FO4ShaderDlg *pDlg) : FO4ShaderRollupBase(pDlg), p_name_edit(nullptr), iLoadBtn(nullptr), iSaveBtn(nullptr)
	{}
	virtual ~FO4ShaderBaseRollup() {}

	void InitializeControls(HWND hwnd) override {
		p_name_edit = GetICustEdit(GetDlgItem(hwnd, IDC_ED_MTL_FILE));
		if (p_name_edit) p_name_edit->Enable(TRUE);

		iLoadBtn = GetICustButton(GetDlgItem(hwnd, IDC_BTN_MTL_LOAD));
		iSaveBtn = GetICustButton(GetDlgItem(hwnd, IDC_BTN_MTL_SAVE));
		if (iSaveBtn) iSaveBtn->Disable();

		for (const EnumLookupType* flag = MaterialFileTypes; flag->name != NULL; ++flag)
			SendDlgItemMessage(hwnd, IDC_CUSTOM_SHADER, CB_ADDSTRING, 0, LPARAM(flag->name));
		SendDlgItemMessage(hwnd, IDC_CUSTOM_SHADER, CB_SETCURSEL, 0, 0);
		//SendDlgItemMessage(hwnd, IDC_CUSTOM_SHADER, WM_SET)
		UpdateControls();
	}
	void ReleaseControls()  override {
		if (p_name_edit) { ReleaseICustEdit(p_name_edit), p_name_edit = nullptr; }
		if (iLoadBtn) { ReleaseICustButton(iLoadBtn); iLoadBtn = nullptr; }
		if (iSaveBtn) { ReleaseICustButton(iSaveBtn); iSaveBtn = nullptr; }
	}

	void UpdateControls() override;

	void CommitValues() override;

	void UpdateVisible()  override {}

	INT_PTR PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) override;
	static INT_PTR CALLBACK DlgRollupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


////////////////////////////////////////////////////////////////////////////
// 
//  Concrete Creation of Rollups in Shader Dialogs
//
FO4ShaderRollupBase::FO4ShaderRollupBase(FO4ShaderDlg *dlg)
{
	pDlg = dlg;
	hRollup = hwHilite = nullptr;
	hOldPal = nullptr;
	isActive = FALSE;
	inUpdate = FALSE;
	valid = FALSE;
	preserveRollup = FALSE;
}

void FO4ShaderRollupBase::FreeRollup()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	DLSetWindowLongPtr(hRollup, NULL);
	DLSetWindowLongPtr(hwHilite, NULL);

	FO4ShaderRollupBase::ReleaseControls();

	HWND hOldRollup = hRollup;
	hwHilite = hRollup = nullptr;

	if (!preserveRollup)
	{
		if (hOldRollup && pDlg && pDlg->pMtlPar)
			pDlg->pMtlPar->DeleteRollupPage(hOldRollup);
	}
}

FO4ShaderRollupBase::~FO4ShaderRollupBase()
{
	FreeRollup();
}

void FO4ShaderRollupBase::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite, &r);
	DrawHilite(hdc, r, pDlg->pShader);
	ReleaseDC(hwHilite, hdc);
}

void FO4ShaderRollupBase::UpdateMapButtons()
{
	//int state = pDlg->pMtl->GetMapState(0);
	//texMButDiffuse->SetText(mapStates[state]);

#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
	//TSTR nm = pMtl->GetMapName(0);
	//texMButDiffuse->SetTooltip(TRUE, nm);
#endif
}

void FO4ShaderRollupBase::ReleaseControls()
{

}


void FO4ShaderRollupBase::NotifyChanged()
{
	pDlg->NotifyChanged();
}

void FO4ShaderRollupBase::UpdateMtlDisplay()
{
	pDlg->UpdateMtlDisplay();
}

void FO4ShaderRollupBase::ReloadPanel()
{
	Interval v;
	pDlg->pShader->Update(pDlg->pMtlPar->GetTime(), v);
	LoadPanel(FALSE);
}

INT_PTR FO4ShaderRollupBase::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		HDC theHDC = GetDC(hwndDlg);
		hOldPal = GetGPort()->PlugPalette(theHDC);
		ReleaseDC(hwndDlg, theHDC);

		InitializeControls(hwndDlg);
		LoadPanel(TRUE);
	}
	break;

	case WM_PAINT:
		if (!valid)
		{
			valid = TRUE;
			ReloadPanel();
		}
		return FALSE;

	case WM_CLOSE:
	case WM_DESTROY:
	case WM_NCDESTROY:
		ReleaseControls();
		break;

	case WM_COMMAND:
	{
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			CommitValues();
			break;
		}
	} break;
	}

	return FALSE;
}

void FO4ShaderRollupBase::LoadPanel(BOOL)
{
	if (hRollup) {
		UpdateControls();
		UpdateColSwatches();
		UpdateHilite();
	}
}

void FO4ShaderBaseRollup::UpdateControls()
{
	USES_CONVERSION;
	FO4Shader* pShader = pDlg->pShader;
	if (inUpdate)
		return;

	BOOL update = inUpdate;
	inUpdate = TRUE;

	HWND hWnd = this->hRollup;
	if (p_name_edit && pShader->pMtlFileRef)
		p_name_edit->SetText(pShader->pMtlFileRef->materialName);

	if (pShader->HasBGSM())
		SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_SETCURSEL, WPARAM(0), LPARAM(0));
	else if (pShader->HasBGEM())
		SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_SETCURSEL, WPARAM(1), LPARAM(0));
	else
		SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_SETCURSEL, WPARAM(-1), LPARAM(0));

	UpdateHilite();
	UpdateVisible();
	NotifyChanged();
	inUpdate = update;
}

void FO4ShaderBaseRollup::CommitValues()
{
	FO4Shader* pShader = pDlg->pShader;
	if (p_name_edit && pShader->pMtlFileRef) {
		TCHAR buffer[120];
		p_name_edit->GetText(buffer, _countof(buffer));
		pShader->pMtlFileRef->materialName = buffer;
	}
}


INT_PTR FO4ShaderBaseRollup::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR tmp[MAX_PATH];
	Interface *gi = GetCOREInterface();
	FO4Shader* pShader = pDlg ? pDlg->pShader : nullptr;

	base::PanelProc(hwndDlg, msg, wParam, lParam);

	if (!pShader) return FALSE;

	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_NCDESTROY:
		ReleaseControls();
		pDlg->DeleteRollups();
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			switch (LOWORD(wParam))
			{
			case IDC_CUSTOM_SHADER:
				int idx = int(SendDlgItemMessage(hwndDlg, IDC_CUSTOM_SHADER, CB_GETCURSEL, WPARAM(0), LPARAM(0)));
				if (idx == 0) { pShader->ChangeShader(BGSMFILE_CLASS_ID); }
				if (idx == 1) { pShader->ChangeShader(BGEMFILE_CLASS_ID); }
				break;
			}
		}
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDC_BTN_MTL_LOAD:
			{
				TCHAR filter[] = // TEXT("All Material Files (*.BGSM,*.BGEM,*.JSON)\0*.BGSM;*.BGEM;*.JSON\0")
					TEXT("BGSM Lighting Shader (*.BGSM)\0*.BGSM\0")
					TEXT("BGEM Effect Shader (*.BGEM)\0*.BGEM\0")
					TEXT("BGSM Lighting Shader (*.JSON)\0*.JSON\0")
					TEXT("BGEM Effect Shader (*.JSON)\0*.JSON\0")
					TEXT("\0");
				if (_taccess(pShader->pMtlFileRef->materialFileName, 0) != -1) {
					_tcscpy(tmp, pShader->pMtlFileRef->materialFileName);
				}
				else {
					_tcscpy(tmp, pShader->pMtlFileRef->materialName);
				}

				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = gi->GetMAXHWnd();
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = tmp;
				ofn.nFilterIndex = pShader->HasBGSM() ? 1 : 2;
				ofn.nMaxFile = _countof(tmp);
				ofn.lpstrTitle = TEXT("Browse for Material File...");
				ofn.lpstrDefExt = pShader->HasBGSM() ? TEXT("BGSM") : TEXT("BGEM");
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					// trim
					pShader->pMtlFileRef->materialName = tmp;
					pShader->pMtlFileRef->materialFileName = tmp;

					if (_tcsicmp(PathFindExtension(tmp), TEXT(".BGSM")) == 0)
						pShader->ChangeShader(BGSMFILE_CLASS_ID);
					else if (_tcsicmp(PathFindExtension(tmp), TEXT(".BGEM")) == 0)
						pShader->ChangeShader(BGEMFILE_CLASS_ID);
					else if (ofn.nFilterIndex == 1 || ofn.nFilterIndex == 3)
						pShader->ChangeShader(BGSMFILE_CLASS_ID);
					else if (ofn.nFilterIndex == 2 || ofn.nFilterIndex == 4)
						pShader->ChangeShader(BGEMFILE_CLASS_ID);

					if (pShader->HasBGSM()) {
						BGSMFile materialData;
						if (ReadBGSMFile(tmp, materialData)) {
							pShader->LoadBGSM(materialData);
							pShader->LoadMaterial(pDlg->pMtl, &fo4Resolver);
						}
					}
					else if (pShader->HasBGEM()) {
						BGEMFile materialData;
						if (ReadBGEMFile(tmp, materialData)) {
							pShader->LoadBGEM(materialData);
							pShader->LoadMaterial(pDlg->pMtl, &fo4Resolver);
						}
					}
					// find the material prefix part
					PathRemoveExtension(tmp);
					PathAddExtension(tmp, pShader->HasBGSM() ? TEXT(".BGSM") : TEXT(".BGEM"));
					for (LPCTSTR filepart = tmp; filepart != nullptr; filepart = PathFindNextComponent(filepart)) {
						if (wildmatch(TEXT("materials\\*"), filepart)) {
							pShader->pMtlFileRef->materialName = filepart;
							break;
						}
					}
					pShader->NotifyChanged();
					pShader->ReloadDialog();
					UpdateControls();
				}
			} break;

			case IDC_BTN_MTL_SAVE:
			{
				TCHAR filter[] = // TEXT("All Material Files (*.BGSM,*.BGEM,*.JSON)\0*.BGSM;*.BGEM;*.JSON\0")
					TEXT("BGSM Lighting Shader (*.BGSM)\0*.BGSM\0")
					TEXT("BGEM Effect Shader (*.BGEM)\0*.BGEM\0")
					TEXT("\0");
				if (_taccess(pShader->pMtlFileRef->materialFileName, 0) != -1) {
					_tcscpy(tmp, pShader->pMtlFileRef->materialFileName);
				}
				else {
					_tcscpy(tmp, pShader->pMtlFileRef->materialName);
				}

				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = gi->GetMAXHWnd();
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = tmp;
				ofn.nFilterIndex = pShader->HasBGSM() ? 1 : 2;
				ofn.nMaxFile = _countof(tmp);
				ofn.lpstrTitle = TEXT("Browse for Material File...");
				ofn.lpstrDefExt = pShader->HasBGSM() ? TEXT("BGSM") : TEXT("BGEM");
				ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
				if (GetSaveFileName(&ofn)) {
					pShader->SyncTexMapsToFilenames(pDlg->pMtl, &fo4Resolver);
					if (pShader->HasBGSM()) {
						if (auto *data = pShader->GetBGSMData())
							SaveBGSMFile(tmp, *data);
					}
					else if (pShader->HasBGEM()) {
						if (auto *data = pShader->GetBGEMData())
							SaveBGEMFile(tmp, *data);
					}
					pShader->NotifyChanged();
					pShader->ReloadDialog();
					UpdateControls();
				}
			} break;
			}
		}
	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {

		case IDC_ED_MTL_FILE:
			if (p_name_edit) {
				TCHAR text[120];
				p_name_edit->GetText(text, _countof(text));
				pShader->SetMaterialName(text);
			} break;
		}
		break;

	}
	return FALSE;
}

INT_PTR CALLBACK  FO4ShaderBaseRollup::DlgRollupProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	FO4ShaderBaseRollup *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = reinterpret_cast<FO4ShaderBaseRollup*>(lParam);
		DLSetWindowLongPtr(hwndDlg, lParam);
	}
	else {
		if ((theDlg = DLGetWindowLongPtr<FO4ShaderBaseRollup *>(hwndDlg)) == nullptr)
			return FALSE;
	}
	++theDlg->isActive;
	INT_PTR res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	--theDlg->isActive;
	return res;
}


#pragma endregion

#pragma region (" Dialog Creation and Destruction ")

////////////////////////////////////////////////////////////////////////////
// 
//  Concrete Creation of Rollups in Shader Dialogs
//
int FO4Shader::NParamDlgs()
{
	return 1;
}

ShaderParamDlg* FO4Shader::GetParamDlg(int n)
{
	return reinterpret_cast<ShaderParamDlg*>(pDlg);
}

void FO4Shader::SetParamDlg(ShaderParamDlg* newDlg, int n)
{
	pDlg = reinterpret_cast<FO4ShaderDlg*>(newDlg);
}

ShaderParamDlg* FO4Shader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int n)
{
	Interval v;
	Update(imp->GetTime(), v);

	// if (pDlg) //?? check for existing dialog?
	pDlg = new FO4ShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings(theMtl, this);
	LoadStdShaderResources();
	int rollupflags = rolloutOpen ? 0 : APPENDROLL_CLOSED;
	{
		FO4ShaderBaseRollup *pRollup = new FO4ShaderBaseRollup(pDlg);
		pDlg->pBaseRollup = pRollup;

		if (hOldRollup) {
			HWND hRollup = imp->ReplaceRollupPage(hOldRollup, hInstance, MAKEINTRESOURCE(IDD_FO4SHADER_BASE),
				FO4ShaderBaseRollup::DlgRollupProc, GetString(IDS_FO4_SHADER_BASIC),
				reinterpret_cast<LPARAM>(pRollup), rollupflags, ROLLUP_CAT_STANDARD);
			pRollup->hRollup = hRollup;
		}
		else {
			HWND hRollup = imp->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_FO4SHADER_BASE),
				FO4ShaderBaseRollup::DlgRollupProc, GetString(IDS_FO4_SHADER_BASIC),
				reinterpret_cast<LPARAM>(pRollup), rollupflags, ROLLUP_CAT_STANDARD);
			pRollup->hRollup = hRollup;
		}
	}
	pDlg->pMtlRollup = FO4ShaderDesc.CreateParamDlg(fos_mtl, hwMtlEdit, imp, this);

	FixRollups();
	return static_cast<ShaderParamDlg*>(pDlg);
}

// fix references too rollups
void FO4Shader::FixRollups()
{
	if (pDlg == nullptr)
		return;
	if (HasBGSM())
	{
		if (pDlg->pBGEMRollup)
		{
			auto* pRollup = pDlg->pBGEMRollup;
			pDlg->pBGEMRollup = nullptr;
			pRollup->DeleteThis();
		}
		if (!pDlg->pBGSMRollup)
		{
			pDlg->pBGSMRollup = FO4ShaderDesc.CreateParamDlg(fos_bgsm, pDlg->hwmEdit, pDlg->pMtlPar, this);
		}
	}
	if (HasBGEM())
	{
		if (pDlg->pBGSMRollup)
		{
			auto* pRollup = pDlg->pBGSMRollup;
			pDlg->pBGSMRollup = nullptr;
			pRollup->DeleteThis();
		}
		if (!pDlg->pBGEMRollup)
		{
			pDlg->pBGEMRollup = FO4ShaderDesc.CreateParamDlg(fos_bgem, pDlg->hwmEdit, pDlg->pMtlPar, this);
		}
	}
}



#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
RefResult FO4Shader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
#else
RefResult FO4Shader::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
#endif
{
	switch (message) {
	case REFMSG_CHANGE:
		ivalid.SetEmpty();
		if (hTarget == pb_base) {
			// update UI if paramblock changed, possibly from scripter
			ParamID changingParam = pb_base->LastNotifyParamID();
			// reload the dialog if present
			if (pDlg) {
				pDlg->UpdateDialog(changingParam);
			}
		}
		break;
	}
	return(REF_SUCCEED);
}


void FO4ShaderDlg::DeleteRollups()
{
	delete pBaseRollup; pBaseRollup = nullptr;
	//if (pMtlRollup) pMtlRollup->DeleteThis(); pMtlRollup = nullptr;
	//if(pBGSMRollup) pBGSMRollup->DeleteThis(); pBGSMRollup = nullptr;
	//if(pBGEMRollup) pBGEMRollup->DeleteThis(); pBGEMRollup = nullptr;
}


void FO4Shader::NotifyChanged()
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void FO4Shader::ReloadDialog()
{
	try { FO4ShaderDesc.InvalidateUI(); }
	catch (...)
	{
	}
	//fos_base_blk.InvalidateUI();
	//fos_mtl_blk.InvalidateUI();
	//fos_bgsm_blk.InvalidateUI();
	//fos_bgem_blk.InvalidateUI();
}

void FO4Shader::ClearDialogRollup(int rollup)
{
	switch (rollup)
	{
	case fos_mtl: if (pDlg) pDlg->pMtlRollup = nullptr;
	case fos_bgsm: if (pDlg) pDlg->pBGSMRollup = nullptr;
	case fos_bgem: if (pDlg) pDlg->pBGEMRollup = nullptr;
	}
}


#pragma endregion
