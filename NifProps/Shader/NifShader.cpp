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
#include "toneop.h"
#include "..\NifProps\NifProps.h"
#include "3dsmaxport.h"

// Class Ids
const Class_ID NIFSHADER_CLASS_ID(0x566e8ccb, 0xb091bd48);

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
   {-1, NULL},
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
   {-1, NULL},
};
const EnumLookupType ApplyModes[] = {
   { 0, TEXT("REPLACE")},
   { 1, TEXT("DECAL")},
   { 2, TEXT("MODULATE")},
   {-1, NULL},
};

const EnumLookupType VertexModes[] = {
   { 0, TEXT("IGNORE")},
   { 1, TEXT("EMISSIVE")},
   { 2, TEXT("AMB_DIFF")},
   {-1, NULL},
};
const EnumLookupType LightModes[] = {
   { 0, TEXT("E")},
   { 1, TEXT("E_A_D")},
   {-1, NULL},
};

const EnumLookupType ShaderTypes[] = {
   { 0, TEXT("Default")},
   { 1, TEXT("BSShader")},
   {-1, NULL},
};

extern const EnumLookupType BSShaderTypes[] = {
   { 0, TEXT("Default")},
   { 1, TEXT("BSShaderPPLightingProperty")},
   { 2, TEXT("BSShaderNoLightingProperty")},
   { 3, TEXT("WaterShaderProperty")},
   { 4, TEXT("SkyShaderProperty")},
	//{ 5, TEXT("DistantLODShaderProperty")},
	//{ 6, TEXT("BSDistantTreeShaderProperty")},
	{ 7, TEXT("TallGrassShaderProperty")},
	//{ 8, TEXT("VolumetricFogShaderProperty")},
	//{ 9, TEXT("HairShaderProperty")},
	{10, TEXT("Lighting30ShaderProperty")},
	{11, TEXT("BSEffectShaderProperty")},
	{100, TEXT("SkyrimDefault")},
	{101, TEXT("SkyrimEnvMap")},
	{102, TEXT("SkyrimGlow")},
	{103, TEXT("SkyrimHeightMap")},
	{104, TEXT("SkyrimFaceTint")},
	{105, TEXT("SkyrimSkinTint")},
	{105, TEXT("SkyrimSkin")},
	{106, TEXT("SkyrimHairTint")},
	{106, TEXT("SkyrimHair")},
	{107, TEXT("SkyrimParallax_Material")},
	{107, TEXT("SkyrimWorldMultiTexture")},
	{107, TEXT("SkyrimWorldMap1")},
	{111, TEXT("SkyrimParallax")},
	{113, TEXT("SkyrimWorldMap2")},
	{114, TEXT("SkyrimSparkleSnow")},
	{115, TEXT("SkyrimWorldMap3")},
	{116, TEXT("SkyrimEye")},
	{118, TEXT("SkyrimWorldMap4")},
	{119, TEXT("SkyrimLodMultiTexture")},

	{-1, NULL},
};


struct TexChannel
{
	DWORD channelName;
	DWORD maxName;
	DWORD channelType;
};

static const TexChannel texChannelNames[STD2_NMAX_TEXMAPS] = {
   {IDS_CHAN_BASE,      IDS_MAXCHAN_DIFFUSE,       CLR_CHANNEL    },             //C_BASE,
   {IDS_CHAN_DARK,      IDS_MAXCHAN_SELFILLUMMAP,  CLR_CHANNEL    },			 //C_DARK,
   {IDS_CHAN_DETAIL,    IDS_MAXCHAN_DETAIL,        CLR_CHANNEL    },			 //C_DETAIL,
   {IDS_CHAN_GLOSS,     IDS_MAXCHAN_GLOSS,         MONO_CHANNEL   },			 //C_GLOSS,
   {IDS_CHAN_GLOW,      IDS_MAXCHAN_GLOW,          CLR_CHANNEL    },			 //C_GLOW,
   {IDS_CHAN_BUMP,      IDS_MAXCHAN_BUMP,          BUMP_CHANNEL   },			 //C_BUMP,
   {IDS_CHAN_NORMAL,    IDS_MAXCHAN_NORMAL,        CLR_CHANNEL    },			 //C_NORMAL,
   {IDS_CHAN_UNK2,      IDS_CHAN_UNK2,             CLR_CHANNEL    },			 //C_DECAL0,
   {IDS_CHAN_DECAL1,    IDS_CHAN_DECAL1,           CLR_CHANNEL    },			 //C_DECAL1,
   {IDS_CHAN_DECAL2,    IDS_CHAN_DECAL2,           CLR_CHANNEL    },			 //C_DECAL2,
   {IDS_CHAN_DECAL3,    IDS_CHAN_DECAL3,           CLR_CHANNEL    },			 //C_DECAL3,
   {IDS_CHAN_ENVMASK,   IDS_CHAN_ENVMASK,          CLR_CHANNEL    },			 //C_ENVMASK,
   {IDS_CHAN_ENV,       IDS_CHAN_ENV,              CLR_CHANNEL    },			 //C_ENV,
   {IDS_CHAN_HEIGHT,    IDS_CHAN_HEIGHT,           MONO_CHANNEL   },			 //C_HEIGHT,
   {IDS_CHAN_REFLECTION,IDS_CHAN_REFLECTION,       REFL_CHANNEL   },			 //C_REFLECTION,
   {IDS_CHAN_OPACITY,   IDS_MAXCHAN_OPACITY,       MONO_CHANNEL   },			 //C_OPACITY,
   {IDS_CHAN_SPECULAR,  IDS_CHAN_SPECULAR,         CLR_CHANNEL    },			 //C_SPECULAR,
   {IDS_CHAN_PARALLAX,  IDS_CHAN_PARALLAX,         MONO_CHANNEL   },			 //C_PARALLAX,
   {IDS_CHAN_BACKLIGHT, IDS_CHAN_BACKLIGHT,        CLR_CHANNEL    },			 //C_BACKLIGHT,
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },		   
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
   {IDS_CHAN_EMPTY,     IDS_MAXCHAN_EMPTY,         UNSUPPORTED_CHANNEL },
};

enum
{
	C_BASE,
	C_DARK,
	C_DETAIL,
	C_GLOSS,
	C_GLOW,
	C_BUMP,
	C_NORMAL,
	C_DECAL0,
	C_DECAL1,
	C_DECAL2,
	C_DECAL3,
	C_ENVMASK,
	C_ENV,
	C_HEIGHT,
	C_REFLECTION,
	C_OPACITY,
	C_SPECULAR,
	C_PARALLAX,
	C_BACKLIGHT,
	C_RESERVED1,
	C_RESERVED2,
	C_RESERVED3,
	C_RESERVED4,
	C_RESERVED5,
	C_MAX_SUPPORTED,
};



// map from custom channel to standard map
static const int nifShaderStdIDToChannel[N_ID_CHANNELS] = {
   -1,         // 0 - ambient
   C_BASE,     // 1 - diffuse           
   C_DARK,     // 2 - specular
   C_GLOSS,    // 3 - Glossiness (Shininess in 3ds Max release 2.0 and earlier)
   C_SPECULAR, // 4 - Specular Level (Shininess strength in 3ds Max release 2.0 and earlier)
   C_GLOW,     // 5 - self-illumination 
   C_OPACITY,  // 6 - opacity
   -1,         // 7 - filter color
   C_BUMP,     // 8 - bump              
   C_REFLECTION,//9 - reflection        
   -1,         // 10 - refraction 
   C_HEIGHT,   // 11 - displacement
};

// Rollups
enum { basic_params, };

enum
{
	ns_mat_ambient, ns_mat_diffuse, ns_mat_specular,
	ns_mat_selfillumclr, ns_mat_selfillum,
	ns_mat_glossiness, ns_mat_speclevel, ns_mat_softenlevel,
	ns_mat_shininess, ns_mat_alpha, ns_mat_dither,
	ns_mat_selfillumon, ns_mat_specenable,
	ns_mat_emittance,
	//////////////////////////////////////////////////////////////////////////
	ns_alpha_mode, ns_alpha_src, ns_alpha_dest,
	//////////////////////////////////////////////////////////////////////////
	ns_vertex_colors_enable, ns_vertex_srcmode, ns_vertex_light,
	//////////////////////////////////////////////////////////////////////////
	ns_apply_mode,
	//////////////////////////////////////////////////////////////////////////
	ns_test_ref, ns_testmode, ns_alphatest_enable, ns_no_sorter,
	//////////////////////////////////////////////////////////////////////////
	ns_shader_name,
	//////////////////////////////////////////////////////////////////////////
	ns_bump_magnitude, ns_luma_scale, ns_luma_offset,
	//////////////////////////////////////////////////////////////////////////
	ns_envmap_scale, ns_refraction_str, ns_lighteff1, ns_lighteff2,

	ns_skin_tint_color, ns_hair_tint_color, ns_max_passes,
	ns_shader_scale, ns_parallax_inner_thickness, ns_parallax_refraction_scale,
	ns_parallax_inner_texture_scale, ns_parallax_envmap_str,
	ns_sparkle_parameters, ns_eye_cubemap_scale,
	ns_left_eye_refl_center, ns_right_eye_refl_center,

};

const ULONG SHADER_PARAMS = (STD_PARAM_SELFILLUM | STD_PARAM_SELFILLUM_CLR
	| STD_PARAM_AMBIENT_CLR | STD_PARAM_DIFFUSE_CLR
	| STD_PARAM_SPECULAR_CLR | STD_PARAM_GLOSSINESS
	| STD_PARAM_SELFILLUM_CLR_ON | STD_PARAM_REFL_LEV
	);

class NifShaderDlg;

class NifShader : public Shader {
	friend class NifShaderCB;
	friend class NifShaderDlg;
	BOOL rolloutOpen;
protected:
	IParamBlock2      *pb;   // ref 0
	Interval    ivalid;

	NifShaderDlg* pDlg;

	BOOL bSelfIllumClrOn;
	Color cSelfIllumClr, cAmbientClr, cDiffuseClr, cSpecularClr;
	float fGlossiness, fSelfIllum, fSpecularLevel, fSoftenLevel;

public:
	NifShader();
	~NifShader();
	ULONG SupportStdParams() { return SHADER_PARAMS; }

	// copy std params, for switching shaders
	void CopyStdParams(Shader* pFrom);

	// texture maps
	long nTexChannelsSupported() { return C_MAX_SUPPORTED; }
	TSTR GetTexChannelName(long nChan) { return GetString(texChannelNames[nChan].channelName); }
	TSTR GetTexChannelInternalName(long nChan) { return GetString(texChannelNames[nChan].maxName); }
	long ChannelType(long nChan) { return texChannelNames[nChan].channelType; }
	long StdIDToChannel(long stdID) { return nifShaderStdIDToChannel[stdID]; }

	//BOOL KeyAtTime(int id,TimeValue t) { return pb->KeyFrameAtTime(id,t); }
	ULONG GetRequirements(int subMtlNum) { return MTLREQ_TRANSP|MTLREQ_PHONG; }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int);
	ShaderParamDlg* GetParamDlg(int) { return (ShaderParamDlg*)pDlg; }
	void SetParamDlg(ShaderParamDlg* newDlg, int) { pDlg = (NifShaderDlg*)newDlg; }

	Class_ID ClassID() { return NIFSHADER_CLASS_ID; }
	SClass_ID SuperClassID() { return SHADER_CLASS_ID; }
	TSTR GetName() { return GetString(IDS_SH_NAME); }
	void GetClassName(TSTR& s) { s = GetName(); }
	void DeleteThis() { delete this; }

	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return (i == 0) ? pb : NULL; }
	TSTR SubAnimName(int i) { return TSTR(GetString(IDS_SH_PARAMETERS)); }
	int SubNumToRefNum(int subNum) { return subNum; }

	// add direct ParamBlock2 access
	int   NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pb; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pb->ID() == id) ? pb : NULL; }

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return (i == 0) ? pb : NULL; }
	void SetReference(int i, RefTargetHandle rtarg)
	{
		if (i == 0) pb = (IParamBlock2*)rtarg; else assert(0);
	}
	void NotifyChanged() { NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); }

	void Update(TimeValue t, Interval& valid);
	void Reset();
	RefTargetHandle Clone(RemapDir &remap /*=DefaultRemapDir()*/);
#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
#else
	RefResult	NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
#endif

	void GetIllumParams(ShadeContext &sc, IllumParams& ip);

	// Strauss Shader specific section
	void  Illum(ShadeContext &sc, IllumParams &ip);

	// std params not supported
	void SetLockDS(BOOL lock) { }
	BOOL GetLockDS() { return FALSE; }
	void SetLockAD(BOOL lock) { }
	BOOL GetLockAD() { return FALSE; }
	void SetLockADTex(BOOL lock) { }
	BOOL GetLockADTex() { return FALSE; }

	virtual void SetSelfIllum(float v, TimeValue t) { fSelfIllum = v;       pb->SetValue(ns_mat_selfillum, t, v, 0); }
	virtual void SetSelfIllumClrOn(BOOL on) { bSelfIllumClrOn = on; pb->SetValue(ns_mat_selfillumon, 0, on, 0); }
	virtual void SetSelfIllumClr(Color c, TimeValue t) { cSelfIllumClr = c;    pb->SetValue(ns_mat_selfillumclr, t, c, 0); }
	virtual void SetAmbientClr(Color c, TimeValue t) { cAmbientClr = c;      pb->SetValue(ns_mat_ambient, t, c, 0); }
	virtual void SetDiffuseClr(Color c, TimeValue t) { cDiffuseClr = c;      pb->SetValue(ns_mat_diffuse, t, c, 0); }
	virtual void SetSpecularClr(Color c, TimeValue t) { cSpecularClr = c;     pb->SetValue(ns_mat_specular, t, c, 0); }
	virtual void SetGlossiness(float v, TimeValue t) { fGlossiness = v;      pb->SetValue(ns_mat_glossiness, t, v, 0); }
	virtual void SetSpecularLevel(float v, TimeValue t) { fSpecularLevel = v;   pb->SetValue(ns_mat_speclevel, t, v, 0); }
	virtual void SetSoftenLevel(float v, TimeValue t) { fSoftenLevel = v;     pb->SetValue(ns_mat_softenlevel, t, v, 0); }

	virtual BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace) { return bSelfIllumClrOn; }
	virtual Color GetAmbientClr(int mtlNum = 0, BOOL backFace = 0) { return cAmbientClr; }
	virtual Color GetDiffuseClr(int mtlNum = 0, BOOL backFace = 0) { return cDiffuseClr; }
	virtual Color GetSpecularClr(int mtlNum = 0, BOOL backFace = 0) { return cSpecularClr; }
	virtual Color GetSelfIllumClr(int mtlNum = 0, BOOL backFace = 0) { return cSelfIllumClr; }
	virtual float GetSelfIllum(int mtlNum = 0, BOOL backFace = 0) { return fSelfIllum; }
	virtual float GetGlossiness(int mtlNum = 0, BOOL backFace = 0) { return fGlossiness; }
	virtual float GetSpecularLevel(int mtlNum = 0, BOOL backFace = 0) { return fSpecularLevel; }
	virtual float GetSoftenLevel(int mtlNum = 0, BOOL backFace = 0) { return fSoftenLevel; }

	virtual BOOL IsSelfIllumClrOn() { return pb->GetInt(ns_mat_selfillumon, 0, 0) ? TRUE : FALSE; }
	virtual Color GetAmbientClr(TimeValue t) { return pb->GetColor(ns_mat_ambient, 0, 0); }
	virtual Color GetDiffuseClr(TimeValue t) { return pb->GetColor(ns_mat_diffuse, 0, 0); }
	virtual Color GetSpecularClr(TimeValue t) { return pb->GetColor(ns_mat_specular, 0, 0); }
	virtual float GetGlossiness(TimeValue t) { return pb->GetFloat(ns_mat_glossiness, 0, 0); }
	virtual float GetSpecularLevel(TimeValue t) { return pb->GetFloat(ns_mat_speclevel, 0, 0); }
	virtual float GetSoftenLevel(TimeValue t) { return pb->GetFloat(ns_mat_softenlevel, 0, 0); }
	virtual float GetSelfIllum(TimeValue t) { return pb->GetFloat(ns_mat_selfillum, 0, 0); }
	virtual Color GetSelfIllumClr(TimeValue t) { return pb->GetColor(ns_mat_selfillumclr, 0, 0); }
	virtual float EvalHiliteCurve2(float x, float y, int level = 0) { return 0.0f; }

	void SetPanelOpen(BOOL open) { rolloutOpen = open; }

	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rClr);

	float EvalHiliteCurve(float x) {
		double phExp = pow(2.0, fGlossiness * 10.0); // expensive.!! TBD
		return fSpecularLevel*(float)pow((double)cos(x*PI), phExp);
	}

};

///////////// Class Descriptor ////////////////////////
class NifShaderClassDesc :public ClassDesc2 {
public:
	int            IsPublic() { return TRUE; }
	void *         Create(BOOL loading = FALSE) { return new NifShader(); }
	const TCHAR *  ClassName() { return GetString(IDS_NIF_SHADER_NAME); }
	SClass_ID      SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID       ClassID() { return NIFSHADER_CLASS_ID; }
	const TCHAR*   Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*   InternalName() { return _T("NifShader"); }   // returns fixed parsable name (scripter-visible name)
	HINSTANCE      HInstance() { return hInstance; }          // returns owning module handle
};

NifShaderClassDesc NifShaderDesc;
extern ClassDesc2 * GetNifShaderDesc() { return &NifShaderDesc; }

static ParamBlockDesc2 param_blk(
	shader_params, _T("shaderParameters"), 0, &NifShaderDesc, P_AUTO_CONSTRUCT, 0,
	//rollout
	ns_mat_ambient, _T("ambient"), TYPE_RGBA, P_ANIMATABLE, IDS_MAT_AMBIENT,
		p_default, Color(0.0f, 0.0f, 0.0f),
		//p_ui, basic_params, TYPE_COLORSWATCH, IDC_CLR_AMBIENT,
		p_end,
	ns_mat_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_MAT_DIFFUSE,
		p_default, Color(0.5f, 0.5f, 0.5f),
		//p_ui, basic_params, TYPE_COLORSWATCH, IDC_CLR_DIFFUSE,
		p_end,
	ns_mat_specular, _T("specular"), TYPE_RGBA, P_ANIMATABLE, IDS_MAT_SPECULAR,
		p_default, Color(1.0f, 1.0f, 1.0f),
		//p_ui, basic_params, TYPE_COLORSWATCH, IDC_CLR_SPECULAR,
		p_end,
	ns_mat_emittance, _T("emittance"), TYPE_RGBA, P_ANIMATABLE, IDS_MAT_EMITTANCE,
		p_default, Color(0.0f, 0.0f, 0.0f),
		//p_ui, basic_params, TYPE_COLORSWATCH, IDC_CLR_EMITTANCE,
		p_end,
	ns_mat_specenable, _T("SpecularEnable"), TYPE_BOOL, 0, IDS_MAT_SPECULARENABLE,
		p_default, FALSE,
		//p_ui, basic_params, TYPE_SINGLECHEKBOX, IDC_OPT_ENABLE,
		p_end,
	ns_mat_selfillumon, _T("useSelfIllumColor"), TYPE_BOOL, 0, IDS_MAT_SELFILLUMON,
		p_default, FALSE,
		p_end,
	ns_mat_selfillumclr, _T("selfillumclr"), TYPE_RGBA, P_ANIMATABLE, IDS_MAT_SELFILLUMCLR,
		p_default, Color(0.0f, 0.0f, 0.0f),
		p_end,
	ns_mat_selfillum, _T("selfillum"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_SELFILLUM,
		p_default, 0.0f,
		p_end,
	ns_mat_glossiness, _T("glossiness"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_GLOSSINESS,
		p_default, 0.0f,
		p_range, 0.0, 200.0,
		p_end,
	ns_mat_speclevel, _T("specularLevel"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_SPECLEVEL,
		p_default, 1.0f,
		p_range, 0.0, 999.0,
		p_end,
	ns_mat_softenlevel, _T("softenlevel"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_SOFTENLEVEL,
		p_default, 0.0f,
		p_end,
	ns_mat_shininess, _T("shininess"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_SHININESS,
		p_default, 10.0f,
		p_end,
	ns_mat_alpha, _T("alpha"), TYPE_FLOAT, P_ANIMATABLE, IDS_MAT_ALPHA,
		p_default, 1.0f,
		p_end,
	ns_mat_dither, _T("Dither"), TYPE_BOOL, P_ANIMATABLE, IDS_MAT_DITHER,
		p_default, FALSE,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_alpha_mode, _T("alphaMode"), TYPE_INT, 0, IDS_ALPHA_MODE,
		p_default, 0,
		p_range, 0, 5,
		p_end,
	ns_alpha_src, _T("srcBlend"), TYPE_INT, 0, IDS_ALPHA_SRC,
		p_default, 6,
		p_end,
	ns_alpha_dest, _T("destBlend"), TYPE_INT, 0, IDS_ALPHA_DEST,
		p_default, 7,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_vertex_colors_enable, _T("VertexColorsEnable"), TYPE_BOOL, 0, IDS_VERTEXCOLORENABLE,
		p_default, TRUE,
		p_end,
	ns_vertex_srcmode, _T("SrcVertexMode"), TYPE_INT, 0, IDS_SRC_VERTEX_MODE,
		p_default, 2,
		p_end,
	ns_vertex_light, _T("LightingMode"), TYPE_INT, 0, IDS_LIGHTING_MODE,
		p_default, 1,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_apply_mode, _T("ApplyMode"), TYPE_INT, 0, IDS_APPLY_MODE,
		p_default, 2,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_test_ref, _T("TestRef"), TYPE_INT, 0, IDS_TEST_REF,
		p_default, 0,
		p_range, 0, 255,
		p_end,
	ns_testmode, _T("TestMode"), TYPE_INT, 0, IDS_TEST_MODE,
		p_default, 4,
		p_end,
	ns_alphatest_enable, _T("AlphaTestEnable"), TYPE_BOOL, 0, IDS_ALPHATEST_ENABLE,
		p_default, FALSE,
		p_end,
	ns_no_sorter, _T("NoSorter"), TYPE_BOOL, 0, IDS_ALPHA_NOSORTER,
		p_default, FALSE,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_shader_name, _T("CustomShader"), TYPE_STRING, 0, IDS_SHADER_NAME,
		p_default, _T(""),
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_bump_magnitude, _T("Magnitude"), TYPE_FLOAT, 0, IDS_BUMP_MAGNITUDE,
		p_default, 1.0f,
		p_range, -2.0f, 2.0f,
		p_end,
	ns_luma_scale, _T("Luma Scale"), TYPE_FLOAT, 0, IDS_LUMA_SCALE,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_end,
	ns_luma_offset, _T("Luma Offset"), TYPE_FLOAT, 0, IDS_LUMA_OFFSET,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
	ns_refraction_str, _T("RefractionStrength"), TYPE_FLOAT, 0, IDS_REFRACTION_STR,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_end,
	ns_lighteff1, _T("LightEff1"), TYPE_FLOAT, 0, IDS_LIGHTEFF1,
		p_default, 0.3f,
		p_range, 0.0f, 2.0f,
		p_end,
	ns_lighteff2, _T("LightEff2"), TYPE_FLOAT, 0, IDS_LIGHTEFF2,
		p_default, 2.0f,
		p_range, 0.0f, 2.0f,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_envmap_scale, _T("EnvMapScale"), TYPE_FLOAT, 0, IDS_ENVMAP_SCALE,
		p_default, 1.0f,
		p_range, 0.0f, 2.0f,
		p_end,
	//////////////////////////////////////////////////////////////////////////
	ns_skin_tint_color, _T("SkinTintColor"), TYPE_RGBA, 0, IDS_SKIN_TINT_COLOR,
		p_default, Color(0.0f, 0.0f, 0.0f),
		p_end,
	ns_hair_tint_color, _T("HairTintColor"), TYPE_RGBA, 0, IDS_HAIR_TINT_COLOR,
		p_default, Color(0.0f, 0.0f, 0.0f),
		p_end,
	ns_max_passes, _T("MaxPasses"), TYPE_FLOAT, 0, IDS_MAX_PASSES,
		p_default, 1.0f,
		p_end,
	ns_shader_scale, _T("ParallaxScale"), TYPE_FLOAT, 0, IDS_SHADER_SCALE,
		p_default, 1.0f,
		p_end,
	ns_parallax_inner_thickness, _T("ParallaxInnerThickness"), TYPE_FLOAT, 0, IDS_PARALLAX_INNER_THICKNESS,
		p_default, 0.0f,
		p_end,
	ns_parallax_refraction_scale, _T("ParallaxRefractionScale"), TYPE_FLOAT, 0, IDS_PARALLAX_REFRACTION_SCALE,
		p_default, 0.0f,
		p_end,
	//ns_parallax_inner_texture_scale, _T("ParallaxInnerTextureScale"), TYPE_POINT2, 0, IDS_PARALLAX_INNER_TEXTURE_SCALE,
	//	p_default, Point2(0.0f, 0.0f),
	//	p_end,
	ns_parallax_envmap_str, _T("ParallaxEnvmapStr"), TYPE_FLOAT, 0, IDS_PARALLAX_ENVMAP_STR,
		p_default, 0.0f,
		p_end,
	//ns_sparkle_parameters, _T("SparkleParameters"), TYPE_QUAT, 0, IDS_SPARKLE_PARAMETERS,
		//p_default, Quat(0.0f, 0.0f, 0.0f, 1.0f),
		//p_end,
	ns_eye_cubemap_scale, _T("EyeCubemapScale"), TYPE_FLOAT, 0, IDS_EYE_CUBEMAP_SCALE,
		p_default, 0.0f,
		p_end,
	//ns_left_eye_refl_center, _T("LeftEyeReflCenter"), TYPE_POINT3, 0, IDS_LEFT_EYE_REFL_CENTER,
	//	p_default, Point3(0.0f, 0.0f, 0.0f),
	//	p_end,
	//ns_right_eye_refl_center, _T("RightEyeReflCenter"), TYPE_POINT3, 0, IDS_RIGHT_EYE_REFL_CENTER,
	//	p_default, Point3(0.0f, 0.0f, 0.0f),
	//	p_end,
	p_end);

NifShader::NifShader()
{
	pb = NULL;
	NifShaderDesc.MakeAutoParamBlocks(this);   // make and intialize paramblock2
	pDlg = NULL;

	bSelfIllumClrOn = FALSE;
	cSelfIllumClr = cAmbientClr = cDiffuseClr = cSpecularClr = Color(0.0f, 0.0f, 0.0f);
	fGlossiness = fSelfIllum = fSpecularLevel = fSoftenLevel = 0.0f;

	ivalid.SetEmpty();
	rolloutOpen = TRUE;

	Reset();
}

NifShader::~NifShader()
{

}

void NifShader::CopyStdParams(Shader* pFrom)
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


RefTargetHandle NifShader::Clone(RemapDir &remap)
{
	NifShader* pShader = new NifShader();
	pShader->ReplaceReference(0, remap.CloneRef(pb));
	return pShader;
}

void NifShader::Update(TimeValue t, Interval &valid) {
	Point3 p;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
	}
	valid &= ivalid;
}

void NifShader::Reset()
{
	NifShaderDesc.MakeAutoParamBlocks(this);

	ivalid.SetEmpty();
	macroRecorder->Disable();  // don't want to see this parameter reset in macrorecorder
	SetSoftenLevel(0.1f, 0);
	SetAmbientClr(Color(0.588f, 0.588f, 0.588f), 0);
	SetDiffuseClr(Color(0.588f, 0.588f, 0.588f), 0);
	SetSpecularClr(Color(0.9f, 0.9f, 0.9f), 0);
	SetGlossiness(.10f, 0);   // change from .25, 11/6/00
	SetSpecularLevel(.0f, 0);

	SetSelfIllum(.0f, 0);
	SetSelfIllumClr(Color(.0f, .0f, .0f), 0);
	SetSelfIllumClrOn(FALSE);
	SetLockADTex(TRUE);
	SetLockAD(TRUE); // DS 10/26/00: changed to TRUE
	SetLockDS(FALSE);
	macroRecorder->Enable();
}

///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//
//#define USE_BLINN_SHADER
#define USE_STRAUSS_SHADER
//#define USE_CUSTOM_SHADER
#ifdef USE_BLINN_SHADER

void NifShader::GetIllumParams(ShadeContext &sc, IllumParams& ip)
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


void NifShader::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc* l = NULL;
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
	} else {
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

void NifShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{ rcol *= ip.channels[ID_SP]; };

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

void NifShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rClr)
{
	float opac = 0.0f;
	float g = ip.channels[C_GLOW].r;
	float m = 0.0f;
	Color Cd = ip.channels[C_BASE];

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

void NifShaderCombineComponents(ShadeContext &sc, IllumParams& ip)
{
	float o = (ip.hasComponents & HAS_REFRACT) ? ip.finalAttenuation : 1.0f;

	ip.finalC = o * (ip.ambIllumOut + ip.diffIllumOut) + ip.specIllumOut
		+ ip.reflIllumOut + ip.transIllumOut;
}


void NifShader::GetIllumParams(ShadeContext &sc, IllumParams& ip)
{
	ip.stdParams = SupportStdParams();
	// ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	TimeValue t = 0;
	ip.channels[C_BASE] = GetDiffuseClr();
	ip.channels[C_OPACITY] = Color(1.0f, 1.0f, 1.0f);
	//ip.channels[(ID_DI)] = GetDiffuseClr();
	//ip.channels[StdIDToChannel(ID_SP)] = GetSpecularClr();
	//ip.channels[StdIDToChannel(ID_SH)].r = GetGlossiness();
	//ip.channels[StdIDToChannel(ID_SS)].r = GetSpecularLevel();
	//if( IsSelfIllumClrOn() )
		//ip.channels[C_GLOW] = pb->GetColor(ns_mat_emittance, 0, 0);
	//else
	//   ip.channels[C_GLOW].r = ip.channels[C_GLOW].g = ip.channels[C_GLOW].b = GetSelfIllum();
}


void NifShader::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *l;
	Color lightClr;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if (sp.x == stopX && sp.y == stopY)
		sp.x = stopX;
#endif

	float opac = ip.channels[C_OPACITY].r;
	float g = ip.channels[C_GLOW].r;
	float m = 0.0f;
	Color Cd = ip.channels[C_BASE];
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
				} else
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

	} else {
		// no refraction, transparent?
		ip.finalAttenuation = opac;
		if (ip.hasComponents & HAS_OPACITY) {
			// ip.finalT = Cd * (1.0f-opac);
			Cd = greyVal * Color(1.0f, 1.0f, 1.0f) + clrVal * Cd;
			ip.finalT = transpColor(TRANSP_FILTER, opac, Cd, Cd);
		}
	}

	NifShaderCombineComponents(sc, ip);
}
#endif
#ifdef USE_CUSTOM_SHADER

//---------------------------------------------------------------------------
// Called to combine the various color and shading components
void NifShaderCombineComponents(ShadeContext &sc, IllumParams& ip)
{
	ip.finalC = (ip.ambIllumOut + ip.diffIllumOut + ip.selfIllumOut) + ip.specIllumOut;
}

//---------------------------------------------------------------------------
void NifShader::GetIllumParams(ShadeContext &sc, IllumParams &ip)
{
	ip.stdParams = SupportStdParams();
	ip.channels[C_BASE] = pb->GetColor(ns_mat_diffuse, 0, 0);
	ip.channels[C_GLOW] = pb->GetColor(ns_mat_selfillumclr, 0, 0);
	ip.channels[C_SPECULAR] = pb->GetColor(ns_mat_specular, 0, 0);

}

//---------------------------------------------------------------------------
void NifShader::Illum(ShadeContext &sc, IllumParams &ip)
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

	NifShaderCombineComponents(sc, ip);
}


//---------------------------------------------------------------------------
void NifShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{
	rcol *= ip.channels[C_SPECULAR];
};
#endif

///////////////////////////////////////////////////////////////////////////////////
//
// Strauss shader dlg panel
//
class NifShaderDlg : public ShaderParamDlg {
public:
	NifShader* pShader;
	StdMat2* pMtl;
	HPALETTE hOldPal;
	HWND     hwmEdit; // window handle of the materials editor dialog
	IMtlParams* pMtlPar;
	HWND     hwHilite;   // the hilite window
	HWND     hRollup; // Rollup panel
	TimeValue   curTime;
	BOOL     valid;
	BOOL     isActive;
	BOOL     inUpdate;

	TexDADMgr dadMgr;

	IColorSwatch* clrAmbient;
	IColorSwatch* clrDiffuse;
	IColorSwatch* clrSpecular;
	IColorSwatch* clrEmittance;

	ICustButton* texMButDiffuse;

	ISpinnerControl *pShininessSpinner;
	ISpinnerControl *pAlphaSpinner;

	ISpinnerControl *pParallaxOffsetSpinner;
	ISpinnerControl *pBumpMagnitudeSpinner;
	ISpinnerControl *pTestRefSpinner;

	ISpinnerControl *pSpecularStrSpinner;
	ISpinnerControl *pRefractionStrSpinner;
	ISpinnerControl *pLightingEff1Spinner;
	ISpinnerControl *pLightingEff2Spinner;

	ISpinnerControl *pEnvMapScaleSpinner; // 1- EnvMap
	IColorSwatch* clrSkinTintColor; // 5 - Skin Tint
	IColorSwatch* clrHairTintColor; // 6 - Hair Tint
	ISpinnerControl *pMaxPassesSpinner; //  7 - Parallax Occ Material
	ISpinnerControl *pParallaxScaleSpinner; //  7 - Parallax Occ Material
	ISpinnerControl *pParallaxInnerThicknessSpinner; //  11 - Multilayer Parallax 
	ISpinnerControl *pParallaxRefractionScaleSpinner; //  11 - Multilayer Parallax 
	ISpinnerControl *pParallaxEnvmapStrSpinner; //  11 - Multilayer Parallax 
	ISpinnerControl *pEyeCubemapScaleSpinner; //  16 - Multilayer Parallax 


	NifShaderDlg(HWND hwMtlEdit, IMtlParams *pParams);
	~NifShaderDlg();
	void DeleteThis() { delete this; }

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		return -1;
	}

	// Methods
	INT_PTR PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	Class_ID ClassID() { return NIFSHADER_CLASS_ID; }

	void SetThing(ReferenceTarget *m) { pMtl = (StdMat2*)m; }
	void SetThings(StdMat2* theMtl, Shader* theShader)
   {  if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (NifShader*)theShader;
		if (pShader)pShader->SetParamDlg(this, 0);
		pMtl = theMtl;
	}
	ReferenceTarget* GetThing() { return pMtl; } // mtl is the thing! (for DAD!)
	Shader* GetShader() { return pShader; }

	void SetTime(TimeValue t) {
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
	void ActivateDlg(BOOL dlgOn) { isActive = dlgOn; }
	HWND GetHWnd() { return hRollup; }
	void NotifyChanged() { pShader->NotifyChanged(); }
	void LoadDialog(BOOL draw);
	void ReloadDialog() { Interval v; pShader->Update(pMtlPar->GetTime(), v); LoadDialog(FALSE); }
	void UpdateDialog(ParamID paramId) { if (!inUpdate) ReloadDialog(); }

	void UpdateMtlDisplay() { pMtlPar->MtlChanged(); } // redraw viewports
	void UpdateHilite();
	void UpdateColSwatches();
	void UpdateMapButtons();
	void UpdateOpacity();

	void InitializeControls(HWND hwnd);
	void ReleaseControls();
	void UpdateControls();
	void CommitValues();
	void UpdateVisible();

	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

INT_PTR CALLBACK  NifShaderDlg::DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	NifShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (NifShaderDlg*)lParam;
		DLSetWindowLongPtr(hwndDlg, lParam);
	}
	else {
		if ((theDlg = DLGetWindowLongPtr<NifShaderDlg *>(hwndDlg)) == NULL)
			return FALSE;
	}
	theDlg->isActive = 1;
	INT_PTR res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* NifShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int)
{
	Interval v;
	Update(imp->GetTime(), v);

	NifShaderDlg *pDlg = new NifShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings(theMtl, this);
	LoadStdShaderResources();
	int rollupflags = rolloutOpen ? 0 : APPENDROLL_CLOSED;
	if (!hOldRollup)
	{
		pDlg->hRollup = imp->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_NIFSHADER),
			NifShaderDlg::DlgProc, GetString(IDS_NIF_SHADER_BASIC),
			(LPARAM)pDlg, rollupflags);
	}
	else
	{
		pDlg->hRollup = imp->ReplaceRollupPage(hOldRollup, hInstance, MAKEINTRESOURCE(IDD_NIFSHADER),
			NifShaderDlg::DlgProc, GetString(IDS_NIF_SHADER_BASIC),
			(LPARAM)pDlg, rollupflags | ROLLUP_SAVECAT | ROLLUP_USEREPLACEDCAT);
	}
	return (ShaderParamDlg*)pDlg;
}

#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
RefResult NifShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
#else
RefResult NifShader::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
#endif
{
	switch (message) {
	case REFMSG_CHANGE:
		ivalid.SetEmpty();
		if (hTarget == pb) {
			// update UI if paramblock changed, possibly from scripter
			ParamID changingParam = pb->LastNotifyParamID();
			// reload the dialog if present
			if (pDlg) {
				pDlg->UpdateDialog(changingParam);
			}
		}
		break;
	}
	return(REF_SUCCEED);
}


NifShaderDlg::NifShaderDlg(HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	isActive = valid = FALSE;
	inUpdate = FALSE;

	clrAmbient = clrDiffuse = clrSpecular = clrEmittance = NULL;
	texMButDiffuse = NULL;
	pShininessSpinner = pAlphaSpinner = pEnvMapScaleSpinner = NULL;
	pParallaxOffsetSpinner = pBumpMagnitudeSpinner = pTestRefSpinner = NULL;
	pRefractionStrSpinner = pLightingEff1Spinner = pLightingEff2Spinner = NULL;
	pSpecularStrSpinner = NULL;

	clrSkinTintColor = clrHairTintColor = NULL;
	pMaxPassesSpinner = pParallaxScaleSpinner = NULL; 
	pParallaxInnerThicknessSpinner = pParallaxRefractionScaleSpinner = NULL; 
	pParallaxEnvmapStrSpinner = pEyeCubemapScaleSpinner = NULL;

}

NifShaderDlg::~NifShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if (pShader) pShader->SetParamDlg(NULL, 0);

	DLSetWindowLongPtr(hRollup, NULL);
	DLSetWindowLongPtr(hwHilite, NULL);

	ReleaseControls();

	hwHilite = hRollup = NULL;
}


void  NifShaderDlg::LoadDialog(BOOL draw)
{
	if (pShader && hRollup) {
		UpdateControls();
		UpdateColSwatches();
		UpdateHilite();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void NifShaderDlg::UpdateMapButtons()
{
	int state = pMtl->GetMapState(0);
	texMButDiffuse->SetText(mapStates[state]);

#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
	TSTR nm = pMtl->GetMapName(0);
	texMButDiffuse->SetTooltip(TRUE, nm);
#endif
}


void NifShaderDlg::UpdateOpacity()
{
	//trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	//trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
	UpdateHilite();
}

void NifShaderDlg::UpdateColSwatches()
{
	//cs[0]->SetKeyBrackets( pShader->KeyAtTime(ns_mat_diffuse,curTime) );
	//cs[0]->SetColor( pShader->GetDiffuseClr() );
}


void NifShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite, &r);
	DrawHilite(hdc, r, pShader);
	ReleaseDC(hwHilite, hdc);
}

INT_PTR NifShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	switch (msg) {
	case WM_INITDIALOG:
	{
		HDC theHDC = GetDC(hwndDlg);
		hOldPal = GetGPort()->PlugPalette(theHDC);
		ReleaseDC(hwndDlg, theHDC);

		InitializeControls(hwndDlg);
		LoadDialog(TRUE);
	}
	break;

	case WM_PAINT:
		if (!valid)
		{
			valid = TRUE;
			ReloadDialog();
		}
		return FALSE;

	case WM_CLOSE:
	case WM_DESTROY:
	case WM_NCDESTROY:
		ReleaseControls();
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_MAP_DIFFUSE:
			PostMessage(hwmEdit, WM_TEXMAP_BUTTON, 0, (LPARAM)pMtl);
			UpdateMapButtons();
			UpdateMtlDisplay();
			break;

		case IDC_CHK_DITHER:
			CommitValues();
			//UpdateControls();
			UpdateMtlDisplay();
			break;

		case IDC_CHK_SPECENABLE:
			CommitValues();
			//UpdateControls();
			UpdateMtlDisplay();
			break;

		default:
			CommitValues();
			break;
		}
	}
	break;

	case WM_NOTIFY:
	{
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			CommitValues();
			break;
		}
	}
	break;


	case CC_COLOR_SEL:
	case CC_COLOR_DROP:
	{
		switch (LOWORD(wParam))
		{
		case IDC_CLR_AMBIENT:   clrAmbient->EditThis(FALSE); break;
		case IDC_CLR_DIFFUSE:   clrDiffuse->EditThis(FALSE); break;
		case IDC_CLR_SPECULAR:  clrSpecular->EditThis(FALSE); break;
		case IDC_CLR_EMITTANCE: clrEmittance->EditThis(FALSE); break;
		}
	}
	break;
	case CC_COLOR_BUTTONDOWN:
		theHold.Begin();
		break;
	case CC_COLOR_BUTTONUP:
		if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
		else theHold.Cancel();
		UpdateMtlDisplay();
		break;
	case CC_COLOR_CHANGE:
	{
		int buttonUp = HIWORD(wParam);
		if (buttonUp) theHold.Begin();
		CommitValues();
		//UpdateControls();
		if (buttonUp) {
			theHold.Accept(GetString(IDS_DS_PARAMCHG));
			// DS: 5/3/99-  this was commented out. I put it back in, because
			// it is necessary for the Reset button in the color picker to 
			// update the viewport.          
			UpdateMtlDisplay();
		}
	}
	break;

	case CC_SPINNER_CHANGE:
		if (!theHold.Holding()) theHold.Begin();
		CommitValues();
		// UpdateControls();
		// UpdateHilite();
		// UpdateMtlDisplay();
		break;

	case CC_SPINNER_BUTTONDOWN:
		theHold.Begin();
		break;

	case WM_CUSTEDIT_ENTER:
	case CC_SPINNER_BUTTONUP:
		if (HIWORD(wParam) || msg == WM_CUSTEDIT_ENTER)
			theHold.Accept(GetString(IDS_DS_PARAMCHG));
		else
			theHold.Cancel();
		UpdateMtlDisplay();
		break;
	}
	//exit:
	return FALSE;
}


void NifShaderDlg::InitializeControls(HWND hWnd)
{

	HDC theHDC = GetDC(hWnd);
	hOldPal = GetGPort()->PlugPalette(theHDC);
	ReleaseDC(hWnd, theHDC);

	//////////////////////////////////////////////////////////////////////////
	clrAmbient = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_AMBIENT));
	clrDiffuse = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_DIFFUSE));
	clrSpecular = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_SPECULAR));
	clrEmittance = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_EMITTANCE));

	texMButDiffuse = GetICustButton(GetDlgItem(hWnd, IDC_MAP_DIFFUSE));
	texMButDiffuse->SetRightClickNotify(TRUE);
	texMButDiffuse->SetDADMgr(&dadMgr);

	pShininessSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_SHININESS));
	pShininessSpinner->SetLimits(0.0f, 200.0f, TRUE);
	pShininessSpinner->SetScale(10.0f);
	pShininessSpinner->SetResetValue(10.0f);
	pShininessSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_SHININESS), EDITTYPE_POS_FLOAT);

	pAlphaSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_ALPHA));
	pAlphaSpinner->SetLimits(0.0f, 1.0f, TRUE);
	pAlphaSpinner->SetScale(0.1f);
	pAlphaSpinner->SetResetValue(0.0f);
	pAlphaSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_ALPHA), EDITTYPE_POS_FLOAT);

	
	pSpecularStrSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_SPECULAR_STR));
	pSpecularStrSpinner->SetLimits(0.0f, 999.0f, TRUE);
	pSpecularStrSpinner->SetScale(1.0f);
	pSpecularStrSpinner->SetResetValue(1.0f);
	pSpecularStrSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_SPECULAR_STR), EDITTYPE_POS_FLOAT);

	pRefractionStrSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_REFRACT_STR));
	pRefractionStrSpinner->SetLimits(0.0f, 1.0f, TRUE);
	pRefractionStrSpinner->SetScale(0.1f);
	pRefractionStrSpinner->SetResetValue(1.0f);
	pRefractionStrSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_REFRACT_STR), EDITTYPE_POS_FLOAT);

	pLightingEff1Spinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_LIGHTEFF_1));
	pLightingEff1Spinner->SetLimits(0.0f, 2.0f, TRUE);
	pLightingEff1Spinner->SetScale(0.1f);
	pLightingEff1Spinner->SetResetValue(0.3f);
	pLightingEff1Spinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_LIGHTEFF_1), EDITTYPE_POS_FLOAT);

	pLightingEff2Spinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_LIGHTEFF_2));
	pLightingEff2Spinner->SetLimits(0.0f, 2.0f, TRUE);
	pLightingEff2Spinner->SetScale(0.1f);
	pLightingEff2Spinner->SetResetValue(2.0f);
	pLightingEff2Spinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_LIGHTEFF_2), EDITTYPE_POS_FLOAT);


	pEnvMapScaleSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_ENVMAP_SCALE));
	pEnvMapScaleSpinner->SetLimits(0.0f, 2.0f, TRUE);
	pEnvMapScaleSpinner->SetScale(0.1f);
	pEnvMapScaleSpinner->SetResetValue(1.0f);
	pEnvMapScaleSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_ENVMAP_SCALE), EDITTYPE_POS_FLOAT);

	clrSkinTintColor = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_SKINTINT));
	clrHairTintColor = GetIColorSwatch(GetDlgItem(hWnd, IDC_CLR_HAIRTINT));

	pMaxPassesSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_MAX_PASSES));
	pMaxPassesSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pMaxPassesSpinner->SetScale(1.0f);
	pMaxPassesSpinner->SetResetValue(0.0f);
	pMaxPassesSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_MAX_PASSES), EDITTYPE_POS_FLOAT);

	pParallaxScaleSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_PARALLAX_SCALE));
	pParallaxScaleSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pParallaxScaleSpinner->SetScale(1.0f);
	pParallaxScaleSpinner->SetResetValue(0.0f);
	pParallaxScaleSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_PARALLAX_SCALE), EDITTYPE_POS_FLOAT);

	pParallaxInnerThicknessSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_PARALLAX_LAYER_THICK));
	pParallaxInnerThicknessSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pParallaxInnerThicknessSpinner->SetScale(0.1f);
	pParallaxInnerThicknessSpinner->SetResetValue(0.0f);
	pParallaxInnerThicknessSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_PARALLAX_LAYER_THICK), EDITTYPE_POS_FLOAT);

	pParallaxRefractionScaleSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_PARALLAX_REFRACT_SCALE));
	pParallaxRefractionScaleSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pParallaxRefractionScaleSpinner->SetScale(0.1f);
	pParallaxRefractionScaleSpinner->SetResetValue(0.0f);
	pParallaxRefractionScaleSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_PARALLAX_REFRACT_SCALE), EDITTYPE_POS_FLOAT);

	pParallaxEnvmapStrSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_PARALLAX_ENVMAP_STR));
	pParallaxEnvmapStrSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pParallaxEnvmapStrSpinner->SetScale(0.1f);
	pParallaxEnvmapStrSpinner->SetResetValue(0.0f);
	pParallaxEnvmapStrSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_PARALLAX_ENVMAP_STR), EDITTYPE_POS_FLOAT);

	pEyeCubemapScaleSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_CUBEMAP_SCALE));
	pEyeCubemapScaleSpinner->SetLimits(0.0f, 1000.0f, FALSE);
	pEyeCubemapScaleSpinner->SetScale(0.1f);
	pEyeCubemapScaleSpinner->SetResetValue(0.0f);
	pEyeCubemapScaleSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_CUBEMAP_SCALE), EDITTYPE_POS_FLOAT);

	//////////////////////////////////////////////////////////////////////////
	for (const EnumLookupType* flag = TransparencyModes; flag->name != NULL; ++flag) {
		SendDlgItemMessage(hWnd, IDC_CBO_TRANS_SRC, CB_ADDSTRING, 0, LPARAM(flag->name));
		SendDlgItemMessage(hWnd, IDC_CBO_TRANS_DEST, CB_ADDSTRING, 0, LPARAM(flag->name));
	}
	//////////////////////////////////////////////////////////////////////////
	for (const EnumLookupType* flag = VertexModes; flag->name != NULL; ++flag)
		SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_SRC, CB_ADDSTRING, 0, LPARAM(flag->name));

	for (const EnumLookupType* flag = LightModes; flag->name != NULL; ++flag)
		SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_LIGHT, CB_ADDSTRING, 0, LPARAM(flag->name));

	//////////////////////////////////////////////////////////////////////////
	for (const EnumLookupType* flag = ApplyModes; flag->name != NULL; ++flag)
		SendDlgItemMessage(hWnd, IDC_CBO_APPLY_MODE, CB_ADDSTRING, 0, LPARAM(flag->name));

	//////////////////////////////////////////////////////////////////////////
	for (const EnumLookupType* flag = TestModes; flag->name != NULL; ++flag)
		SendDlgItemMessage(hWnd, IDC_CBO_TESTMODE, CB_ADDSTRING, 0, LPARAM(flag->name));

	pTestRefSpinner = GetISpinner(GetDlgItem(hWnd, IDC_SPN_TESTREF));
	pTestRefSpinner->SetLimits(0, 256, TRUE);
	pTestRefSpinner->SetScale(1.0f);
	pTestRefSpinner->SetResetValue(0.0f);
	pTestRefSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_EDT_TESTREF), EDITTYPE_POS_INT);

	for (const EnumLookupType* flag = BSShaderTypes; flag->name != NULL; ++flag)
		SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_ADDSTRING, 0, LPARAM(flag->name));
	SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_SETCURSEL, 0, 0);

	UpdateControls();
}


void NifShaderDlg::ReleaseControls()
{
	if (hOldPal)
	{
		HDC hdc = GetDC(hRollup);
		GetGPort()->RestorePalette(hdc, hOldPal);
		ReleaseDC(hRollup, hdc);
		hOldPal = NULL;
	}

	if (clrAmbient) { ReleaseIColorSwatch(clrAmbient); clrAmbient = NULL; }
	if (clrDiffuse) { ReleaseIColorSwatch(clrDiffuse); clrDiffuse = NULL; };
	if (clrSpecular) { ReleaseIColorSwatch(clrSpecular); clrSpecular = NULL; };
	if (clrEmittance) { ReleaseIColorSwatch(clrEmittance); clrEmittance = NULL; };

	if (texMButDiffuse) { ReleaseICustButton(texMButDiffuse); texMButDiffuse = NULL; };

	if (pShininessSpinner) { ReleaseISpinner(pShininessSpinner); pShininessSpinner = NULL; };
	if (pAlphaSpinner) { ReleaseISpinner(pAlphaSpinner); pAlphaSpinner = NULL; };

	if (pParallaxOffsetSpinner) { ReleaseISpinner(pParallaxOffsetSpinner); pParallaxOffsetSpinner = NULL; };
	if (pBumpMagnitudeSpinner) { ReleaseISpinner(pBumpMagnitudeSpinner); pBumpMagnitudeSpinner = NULL; };
	if (pTestRefSpinner) { ReleaseISpinner(pTestRefSpinner); pTestRefSpinner = NULL; };

	if (pEnvMapScaleSpinner) { ReleaseISpinner(pEnvMapScaleSpinner); pEnvMapScaleSpinner = NULL; };
	if (pSpecularStrSpinner) { ReleaseISpinner(pSpecularStrSpinner); pSpecularStrSpinner = NULL; };
	if (pRefractionStrSpinner) { ReleaseISpinner(pRefractionStrSpinner); pRefractionStrSpinner = NULL; };
	if (pLightingEff1Spinner) { ReleaseISpinner(pLightingEff1Spinner); pLightingEff1Spinner = NULL; };
	if (pLightingEff2Spinner) { ReleaseISpinner(pLightingEff2Spinner); pLightingEff2Spinner = NULL; };

	if (clrSkinTintColor) { ReleaseIColorSwatch(clrSkinTintColor); clrSkinTintColor = NULL; };
	if (clrHairTintColor) { ReleaseIColorSwatch(clrHairTintColor); clrHairTintColor = NULL; };
	if (pMaxPassesSpinner) { ReleaseISpinner(pMaxPassesSpinner); pMaxPassesSpinner = NULL; };
	if (pParallaxScaleSpinner) { ReleaseISpinner(pParallaxScaleSpinner); pParallaxScaleSpinner = NULL; };
	if (pParallaxInnerThicknessSpinner) { ReleaseISpinner(pParallaxInnerThicknessSpinner); pParallaxInnerThicknessSpinner = NULL; };
	if (pParallaxRefractionScaleSpinner) { ReleaseISpinner(pParallaxRefractionScaleSpinner); pParallaxRefractionScaleSpinner = NULL; };
	if (pParallaxEnvmapStrSpinner) { ReleaseISpinner(pParallaxEnvmapStrSpinner); pParallaxEnvmapStrSpinner = NULL; };
	if (pEyeCubemapScaleSpinner) { ReleaseISpinner(pEyeCubemapScaleSpinner); pEyeCubemapScaleSpinner = NULL; };

}

void NifShaderDlg::UpdateControls()
{
	if (inUpdate)
		return;

	BOOL update = inUpdate;
	inUpdate = TRUE;

	HWND hWnd = this->hRollup;
	IParamBlock2 *pb = pShader->pb;
	clrAmbient->SetColor(pb->GetColor(ns_mat_ambient, 0, 0));
	clrDiffuse->SetColor(pb->GetColor(ns_mat_diffuse, 0, 0));
	clrSpecular->SetColor(pb->GetColor(ns_mat_specular, 0, 0));
	clrEmittance->SetColor(pb->GetColor(ns_mat_emittance, 0, 0));

	UpdateMapButtons();

	CheckDlgButton(hWnd, IDC_CHK_SPECENABLE, pb->GetInt(ns_mat_specenable, 0, 0));
	CheckDlgButton(hWnd, IDC_CHK_DITHER, pb->GetInt(ns_mat_dither, 0, 0));

	CheckDlgButton(hWnd, IDC_CHK_DITHER, pb->GetInt(ns_mat_dither, 0, 0));

	pShininessSpinner->SetValue(pb->GetFloat(ns_mat_glossiness, 0, 0), 0);
	pAlphaSpinner->SetValue(pb->GetFloat(ns_mat_alpha, 0, 0), 0);

	//////////////////////////////////////////////////////////////////////////

	int alphaMode = pb->GetInt(ns_alpha_mode, 0, 0);
	int rdoCtrl = IDC_RDO_TRANS_AUTO + alphaMode;
	BOOL allowSrcDest = (rdoCtrl == IDC_RDO_TRANS_AUTO) || (rdoCtrl == IDC_RDO_TRANS_ADV);
	CheckRadioButton(hWnd, IDC_RDO_TRANS_AUTO, IDC_RDO_TRANS_ADV, rdoCtrl);
	EnableWindow(GetDlgItem(hWnd, IDC_CBO_TRANS_SRC), allowSrcDest);
	EnableWindow(GetDlgItem(hWnd, IDC_CBO_TRANS_DEST), allowSrcDest);

	int srcMode = pb->GetInt(ns_alpha_src, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_TRANS_SRC, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(srcMode, TransparencyModes).data()));

	int dstMode = pb->GetInt(ns_alpha_dest, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_TRANS_DEST, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(dstMode, TransparencyModes).data()));

	//////////////////////////////////////////////////////////////////////////

	CheckDlgButton(hWnd, IDC_CHK_VERTEXENABLE, pb->GetInt(ns_vertex_colors_enable, 0, 0));

	int vertSrc = pb->GetInt(ns_vertex_srcmode, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_SRC, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(vertSrc, VertexModes).data()));

	int vertLight = pb->GetInt(ns_vertex_light, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_LIGHT, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(vertLight, LightModes).data()));

	//////////////////////////////////////////////////////////////////////////

	int applyMode = pb->GetInt(ns_apply_mode, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_APPLY_MODE, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(applyMode, ApplyModes).data()));

	//////////////////////////////////////////////////////////////////////////

	int testMode = pb->GetInt(ns_testmode, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CBO_TESTMODE, CB_SELECTSTRING, WPARAM(-1), LPARAM(EnumToString(testMode, TestModes).data()));

	CheckDlgButton(hWnd, IDC_CHK_ALPHATESTENABLE, pb->GetInt(ns_alphatest_enable, 0, 0));
	CheckDlgButton(hWnd, IDC_CHK_NOSORTER, pb->GetInt(ns_no_sorter, 0, 0));

	pTestRefSpinner->SetValue(pb->GetInt(ns_test_ref, 0, 0), 0);

	TSTR customShader = pb->GetStr(ns_shader_name, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CUSTOM_SHADER, CB_SELECTSTRING, WPARAM(-1), LPARAM(customShader.data()));
	
	pSpecularStrSpinner->SetValue(pb->GetFloat(ns_mat_speclevel, 0, 0), 0);
	pRefractionStrSpinner->SetValue(pb->GetFloat(ns_refraction_str, 0, 0), 0);
	pLightingEff1Spinner->SetValue(pb->GetFloat(ns_lighteff1, 0, 0), 0);
	pLightingEff2Spinner->SetValue(pb->GetFloat(ns_lighteff2, 0, 0), 0);

	pEnvMapScaleSpinner->SetValue(pb->GetFloat(ns_envmap_scale, 0, 0), 0);
	clrSkinTintColor->SetColor(pb->GetColor(ns_skin_tint_color, 0, 0), 0);
	clrHairTintColor->SetColor(pb->GetColor(ns_hair_tint_color, 0, 0), 0);
	pMaxPassesSpinner->SetValue(pb->GetFloat(ns_max_passes, 0, 0), 0);
	pParallaxScaleSpinner->SetValue(pb->GetFloat(ns_shader_scale, 0, 0), 0);
	pParallaxInnerThicknessSpinner->SetValue(pb->GetFloat(ns_parallax_inner_thickness, 0, 0), 0);
	pParallaxRefractionScaleSpinner->SetValue(pb->GetFloat(ns_parallax_refraction_scale, 0, 0), 0);
	pParallaxEnvmapStrSpinner->SetValue(pb->GetFloat(ns_parallax_envmap_str, 0, 0), 0);
	pEyeCubemapScaleSpinner->SetValue(pb->GetFloat(ns_eye_cubemap_scale, 0, 0), 0);

	UpdateVisible();
	NotifyChanged();
	inUpdate = update;
}

void NifShaderDlg::UpdateVisible()
{
	TCHAR customShader[64];
	GetDlgItemText(this->GetHWnd(), IDC_CUSTOM_SHADER, customShader, _countof(customShader));
	int value = StringToEnum(customShader, BSShaderTypes);

	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_ENVMAP_SCALE), value == 101 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_ENVMAP_SCALE), value == 101 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_ENVMAP_SCALE), value == 101 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_SKINTINT), value == 105 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_CLR_SKINTINT), value == 105 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_HAIRTINT), value == 106 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_CLR_HAIRTINT), value == 106 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_MAX_PASSES), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_MAX_PASSES), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_MAX_PASSES), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_PARALLAX_SCALE), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_PARALLAX_SCALE), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_PARALLAX_SCALE), value == 107 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_PARALLAX_LAYER_THICK), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_PARALLAX_LAYER_THICK), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_PARALLAX_LAYER_THICK), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_PARALLAX_REFRACT_SCALE), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_PARALLAX_REFRACT_SCALE), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_PARALLAX_REFRACT_SCALE), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_PARALLAX_ENVMAP_STR), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_PARALLAX_ENVMAP_STR), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_PARALLAX_ENVMAP_STR), value == 111 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_LBL_CUBEMAP_SCALE), value == 116 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_EDT_CUBEMAP_SCALE), value == 116 ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(this->GetHWnd(), IDC_SPN_CUBEMAP_SCALE), value == 116 ? SW_SHOW : SW_HIDE);
}

void NifShaderDlg::CommitValues()
{
	BOOL update = inUpdate;
	inUpdate = TRUE;

	HWND hWnd = this->hRollup;
	IParamBlock2 *pb = pShader->pb;

	pShader->SetAmbientClr(Color(clrAmbient->GetColor()), 0);
	pShader->SetDiffuseClr(Color(clrDiffuse->GetColor()), 0);
	pShader->SetSpecularClr(Color(clrSpecular->GetColor()), 0);

	Color cEmit(clrEmittance->GetColor());
	pb->SetValue(ns_mat_emittance, 0, cEmit, 0);

	pb->SetValue(ns_mat_specenable, 0, (int)IsDlgButtonChecked(hWnd, IDC_CHK_SPECENABLE), 0);
	pb->SetValue(ns_mat_dither, 0, (int)IsDlgButtonChecked(hWnd, IDC_CHK_DITHER), 0);

	pShader->SetGlossiness(pShininessSpinner->GetFVal(), 0);
	pb->SetValue(ns_mat_alpha, 0, pAlphaSpinner->GetFVal(), 0);

	//////////////////////////////////////////////////////////////////////////
	for (int ctrl = IDC_RDO_TRANS_AUTO; ctrl <= IDC_RDO_TRANS_ADV; ++ctrl) {
		if (IsDlgButtonChecked(hWnd, ctrl)) {
			pb->SetValue(ns_alpha_mode, 0, ctrl - IDC_RDO_TRANS_AUTO, 0);
			break;
		}
	}
	int srcMode = SendDlgItemMessage(hWnd, IDC_CBO_TRANS_SRC, CB_GETCURSEL, 0, 0);
	pb->SetValue(ns_alpha_src, 0, srcMode, 0);

	int dstMode = SendDlgItemMessage(hWnd, IDC_CBO_TRANS_DEST, CB_GETCURSEL, 0, 0);
	pb->SetValue(ns_alpha_dest, 0, dstMode, 0);

	//////////////////////////////////////////////////////////////////////////

	pb->SetValue(ns_vertex_colors_enable, 0, (int)IsDlgButtonChecked(hWnd, IDC_CHK_VERTEXENABLE), 0);

	int vertSrc = SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_SRC, CB_GETCURSEL, 0, 0);
	pb->SetValue(ns_vertex_srcmode, 0, vertSrc, 0);

	int vertLight = SendDlgItemMessage(hWnd, IDC_CBO_VERTEX_LIGHT, CB_GETCURSEL, 0, 0);
	pb->SetValue(ns_vertex_light, 0, vertLight, 0);

	//////////////////////////////////////////////////////////////////////////

	int applyMode = SendDlgItemMessage(hWnd, IDC_CBO_APPLY_MODE, CB_GETCURSEL, 0, 0);
	pb->SetValue(ns_apply_mode, 0, applyMode, 0);

	pb->SetValue(ns_alphatest_enable, 0, (int)IsDlgButtonChecked(hWnd, IDC_CHK_ALPHATESTENABLE), 0);
	pb->SetValue(ns_no_sorter, 0, (int)IsDlgButtonChecked(hWnd, IDC_CHK_NOSORTER), 0);

	pb->SetValue(ns_test_ref, 0, pTestRefSpinner->GetIVal(), 0);

	TCHAR customShader[64];
	GetDlgItemText(hWnd, IDC_CUSTOM_SHADER, customShader, _countof(customShader));
	pb->SetValue(ns_shader_name, 0, customShader, 0);

	pb->SetValue(ns_mat_speclevel, 0, pSpecularStrSpinner->GetFVal(), 0);
	pShader->fSpecularLevel = pSpecularStrSpinner->GetFVal();
	pb->SetValue(ns_envmap_scale, 0, pEnvMapScaleSpinner->GetFVal(), 0);
	pb->SetValue(ns_refraction_str, 0, pRefractionStrSpinner->GetFVal(), 0);
	pb->SetValue(ns_lighteff1, 0, pLightingEff1Spinner->GetFVal(), 0);
	pb->SetValue(ns_lighteff2, 0, pLightingEff2Spinner->GetFVal(), 0);

	pb->SetValue(ns_envmap_scale, 0, pEnvMapScaleSpinner->GetFVal(), 0);
#if VERSION_3DSMAX > (8000<<16) // Version 16 (2014)
	pb->SetValue(ns_skin_tint_color, 0, Color(clrSkinTintColor->GetColor()), 0);
	pb->SetValue(ns_hair_tint_color, 0, Color(clrHairTintColor->GetColor()), 0);
#endif
	pb->SetValue(ns_max_passes, 0, pMaxPassesSpinner->GetFVal(), 0);
	pb->SetValue(ns_shader_scale, 0, pParallaxScaleSpinner->GetFVal(), 0);
	pb->SetValue(ns_parallax_inner_thickness, 0, pParallaxInnerThicknessSpinner->GetFVal(), 0);
	pb->SetValue(ns_parallax_refraction_scale, 0, pParallaxRefractionScaleSpinner->GetFVal(), 0);
	pb->SetValue(ns_parallax_envmap_str, 0, pParallaxEnvmapStrSpinner->GetFVal(), 0);
	pb->SetValue(ns_eye_cubemap_scale, 0, pEyeCubemapScaleSpinner->GetFVal(), 0);

	//////////////////////////////////////////////////////////////////////////

	UpdateVisible();
	inUpdate = update;
}
