/**********************************************************************
*<
FILE: MtlDefine.h

DESCRIPTION:	Material Definition

HISTORY:

*>	Copyright (c) 2015, All Rights Reserved.
**********************************************************************/

typedef struct MaterialHeader
{
    char Signature[4];
    uint32_t Version;
} MaterialHeader;

enum AlphaBlendModeType
{
	ABMT_Unknown = 0,
	ABMT_None,
	ABMT_Standard,
	ABMT_Additive,
	ABMT_Multiplicative,
};
enum AlphaBlendFunc : uint32_t {
	ABF_ONE = 0,
	ABF_ZERO = 1,
	ABF_SRC_COLOR = 2,
	ABF_ONE_MINUS_SRC_COLOR = 3,
	ABF_DST_COLOR = 4,
	ABF_ONE_MINUS_DST_COLOR = 5,
	ABF_SRC_ALPHA = 6,
	ABF_ONE_MINUS_SRC_ALPHA = 7,
	ABF_DST_ALPHA = 8,
	ABF_ONE_MINUS_DST_ALPHA = 9,
	ABF_SRC_ALPHA_SATURATE = 10
};


typedef struct BaseMaterial
{
	bool TileU;
	bool TileV;
	float UOffset;
    float VOffset;
    float UScale;
    float VScale;
    float Alpha;
	AlphaBlendModeType AlphaBlendMode;
    bool BlendState;
	AlphaBlendFunc BlendFunc1;
	AlphaBlendFunc BlendFunc2;
    ::byte AlphaTestRef;
    bool AlphaTest;
    bool ZBufferWrite;
    bool ZBufferTest;
    bool ScreenSpaceReflections;
    bool WetnessControlScreenSpaceReflections;
    bool Decal;
    bool TwoSided;
    bool DecalNoFade;
    bool NonOccluder;
    bool Refraction;
    bool RefractionFalloff;
    float RefractionPower;
    bool EnvironmentMapping;
    float EnvironmentMappingMaskScale;
    bool GrayscaleToPaletteColor;
} BaseMaterial;

typedef struct BGSMFile : BaseMaterial
{
    tstring DiffuseTexture;
    tstring NormalTexture;
    tstring SmoothSpecTexture;
    tstring GreyscaleTexture;
    tstring EnvmapTexture;
    tstring GlowTexture;
    tstring InnerLayerTexture;
    tstring WrinklesTexture;
    tstring DisplacementTexture;
    bool EnableEditorAlphaRef;
    bool RimLighting;
    float RimPower;
    float BackLightPower;
    bool SubsurfaceLighting;
    float SubsurfaceLightingRolloff;
    bool SpecularEnabled;
	Niflib::Color3 SpecularColor;
    float SpecularMult;
    float Smoothness;
    float FresnelPower;
    float WetnessControlSpecScale;
    float WetnessControlSpecPowerScale;
    float WetnessControlSpecMinvar;
    float WetnessControlEnvMapScale;
    float WetnessControlFresnelPower;
    float WetnessControlMetalness;

    tstring RootMaterialPath;
    bool AnisoLighting;
    bool EmitEnabled;
	Niflib::Color3 EmittanceColor;// if( EmitEnabled)
    float EmittanceMult;
    bool ModelSpaceNormals;
    bool ExternalEmittance;
    bool BackLighting;
    bool ReceiveShadows;
    bool HideSecret;
    bool CastShadows;
    bool DissolveFade;
    bool AssumeShadowmask;
    bool Glowmap;
    bool EnvironmentMappingWindow;
    bool EnvironmentMappingEye;
    bool Hair;
    Niflib::Color3 HairTintColor;
    bool Tree;
    bool Facegen;
    bool SkinTint;
    bool Tessellate;
    float DisplacementTextureBias;
    float DisplacementTextureScale;
    float TessellationPNScale;
    float TessellationBaseFactor;
    float TessellationFadeDistance;
    float GrayscaleToPaletteScale;
    bool SkewSpecularAlpha; // (header.Version >= 1)    
} BGSMFile;


typedef struct BGEMFile : BaseMaterial {
	tstring BaseTexture;
	tstring GrayscaleTexture;
	tstring EnvmapTexture;
	tstring NormalTexture;
	tstring EnvmapMaskTexture;
	bool BloodEnabled;
	bool EffectLightingEnabled;
	bool FalloffEnabled;
	bool FalloffColorEnabled;
	bool GrayscaleToPaletteAlpha;
	bool SoftEnabled;
	Niflib::Color3 BaseColor;
	float BaseColorScale; // 1.0f
	float FalloffStartAngle;
	float FalloffStopAngle;
	float FalloffStartOpacity;
	float FalloffStopOpacity;
	float LightingInfluence; // 1.0f
	::byte EnvmapMinLOD;
	float SoftDepth; // 100.0f
} BGEMFile;

extern bool InitialzeBGSM(BGSMFile& bgsm);
extern bool ReadBGSMFile(const tstring& filename, BGSMFile& bgsm);
extern bool SaveBGSMFile(const tstring& filename, const BGSMFile& bgsm);

extern bool InitialzeBGEM(BGEMFile& bgem);
extern bool ReadBGEMFile(const tstring& filename, BGEMFile& bgem);
extern bool SaveBGEMFile(const tstring& filename, const BGEMFile& bgem);
extern AlphaBlendModeType ConvertAlphaBlendMode(bool BlendState, AlphaBlendFunc BlendFunc1, AlphaBlendFunc BlendFunc2);
extern void ConvertAlphaBlendMode(AlphaBlendModeType type, bool& BlendState, AlphaBlendFunc& BlendFunc1, AlphaBlendFunc& BlendFunc2);
