/**********************************************************************
*<
FILE: MtlUtils.cpp

DESCRIPTION:  Material Utilities

HISTORY:

*>	Copyright (c) 2015, All Rights Reserved.
**********************************************************************/
#include "pch.h"
#include "niutils.h"
#include "mtldefine.h"
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <malloc.h>
#include <sstream>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

const static Niflib::Color3 empty_color3(0.0f, 0.0f, 0.0f);


const EnumLookupType AlphaBlendModeTypes[] = {
	{ ABMT_Unknown,			TEXT("Unknown") },
	{ ABMT_None,			TEXT("None") },
	{ ABMT_Standard,		TEXT("Standard") },
	{ ABMT_Additive,		TEXT("Additive") },
	{ ABMT_Multiplicative,	TEXT("Multiplicative") },
	{ -1, nullptr },
};

void ConvertAlphaBlendMode(AlphaBlendModeType type, bool& BlendState, AlphaBlendFunc& BlendFunc1, AlphaBlendFunc& BlendFunc2)
{
	switch (type) {
	case ABMT_Unknown:	BlendState = false, BlendFunc1 = ABF_SRC_ALPHA, BlendFunc2 = ABF_ONE_MINUS_SRC_ALPHA; break;
	case ABMT_None:		BlendState = false, BlendFunc1 = ABF_ONE, BlendFunc2 = ABF_ONE; break;
	case ABMT_Standard:	BlendState = true, BlendFunc1 = ABF_SRC_ALPHA, BlendFunc2 = ABF_ONE_MINUS_SRC_ALPHA; break;
	case ABMT_Additive:	BlendState = true, BlendFunc1 = ABF_SRC_ALPHA, BlendFunc2 = ABF_ONE; break;
	case ABMT_Multiplicative:	BlendState = true, BlendFunc1 = ABF_DST_COLOR, BlendFunc2 = ABF_ZERO; break;
	}
}

AlphaBlendModeType ConvertAlphaBlendMode(bool BlendState, AlphaBlendFunc BlendFunc1, AlphaBlendFunc BlendFunc2)
{
	if (BlendState == false && BlendFunc1 == ABF_SRC_ALPHA && BlendFunc2 == ABF_ONE_MINUS_SRC_ALPHA) return ABMT_Unknown;
	if (BlendState == false && BlendFunc1 == ABF_ONE && BlendFunc2 == ABF_ONE) return ABMT_None;
	if (BlendState == true && BlendFunc1 == ABF_SRC_ALPHA && BlendFunc2 == ABF_ONE_MINUS_SRC_ALPHA) return ABMT_Standard;
	if (BlendState == true && BlendFunc1 == ABF_SRC_ALPHA && BlendFunc2 == ABF_ONE) return ABMT_Additive;
	if (BlendState == true && BlendFunc1 == ABF_DST_COLOR && BlendFunc2 == ABF_ZERO) return ABMT_Multiplicative;
	return ABMT_None;
}

#define READ_VALUE(dst, x)  if (!ReadObject(src, ##dst . ##x, #x)) goto error;
#define SAVE_VALUE(src, x)  if (!SaveObject(dst, ##src . ##x, #x)) goto error;

template <typename T>
bool ReadObject(FILE* file, T& x, const char *name) {
	return (fread(&x, sizeof(x), 1, file) == 1);
}

template <>
bool ReadObject<tstring>(FILE* src, tstring& x, const char *name) {
	USES_CONVERSION;
	int len = 0;
	char *ptr = nullptr;
	if (!ReadObject(src, len, "")) goto error;
	if (len >= MAX_PATH) goto error;

	ptr = static_cast<char*>(_alloca(len));
	if (fread(ptr, 1, len, src) != len) goto error;
	x = A2T(ptr);
	return true;
error: return false;
}


template <typename T>
bool SaveObject(FILE* file, const T& x, const char *name) {
	return (fwrite(&x, sizeof(x), 1, file) == 1);
}

template <>
bool SaveObject<tstring>(FILE* file, const tstring& x, const char *name) {
	USES_CONVERSION;
	ULONG len = ULONG(x.size() + 1); // include null terminator
	char *ptr = static_cast<char*>(_alloca(len));
	strcpy(ptr, T2A(x.c_str()));
	for (ULONG i = 0; i < len; ++i)
		if (ptr[i] == '\\') ptr[i] = '/';
	if (!SaveObject(file, len, "")) goto error;
	if (fwrite(ptr, 1, len, file) != len) goto error;
	return true;
error: return false;
}

template <typename T>
bool ReadObject(const rapidjson::Value& value, T& x, const char *name) {
	// TODO: ASSERT()
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, int& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		x = value.GetInt();
		return true;
	} else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, float& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		x = value.GetDouble();
		return true;
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, double& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		x = value.GetDouble();
		return true;
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, bool& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		x = value.GetBool();
		return true;
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, Niflib::Color3& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		const char *str = value.GetString();
		if (str && str[0] == '#') {
			int ival = strtol(str + 1, nullptr, 16);
			x.r = Niflib::ConvertByteToFloat((ival & 0xFF0000) >> 16);
			x.g = Niflib::ConvertByteToFloat((ival & 0x00FF00) >> 8);
			x.b = Niflib::ConvertByteToFloat((ival & 0x0000FF) >> 0);
			return true;
		}
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, wstring& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		USES_CONVERSION;
		const char *str = value.GetString();
		x = A2W(str);
		return true;
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject(const rapidjson::Value& value, string& x, const char *name) {
	if (name == nullptr || name[0] == 0) {
		x = value.GetString();
		return true;
	}
	else {
		rapidjson::Value::ConstMemberIterator itr = value.FindMember(name);
		if (itr != value.MemberEnd())
			return ReadObject((*itr).value, x, nullptr);
	}
	return false;
}

template <>
bool ReadObject<AlphaBlendModeType>(const rapidjson::Value& value, AlphaBlendModeType& x, const char *name) {
	tstring temp;
	if (!ReadObject(value, temp, name)) return false;
	x = AlphaBlendModeType(StringToEnum(temp.c_str(), AlphaBlendModeTypes));
	return true;
}

static bool ReadMtlHeader(FILE* src, MaterialHeader& hdr)
{
	READ_VALUE(hdr,Signature);
	READ_VALUE(hdr,Version);
	return true;
	error: return false;
}

static bool SaveMtlHeader(FILE* dst, MaterialHeader& hdr)
{
	SAVE_VALUE(hdr, Signature);
	SAVE_VALUE(hdr, Version);
	return true;
error: return false;
}

static bool ReadBaseMaterial(FILE* src, const MaterialHeader& hdr, BaseMaterial& mtl)
{
	int tile_uv;
	if (!ReadObject(src, tile_uv, "")) goto error;
	mtl.TileU = (tile_uv & 0x1) != 0;
	mtl.TileV = (tile_uv & 0x2) != 0;
	READ_VALUE(mtl,UOffset);
	READ_VALUE(mtl,VOffset);
	READ_VALUE(mtl,UScale);
	READ_VALUE(mtl,VScale);
	READ_VALUE(mtl,Alpha);

	READ_VALUE(mtl,BlendState);
	READ_VALUE(mtl,BlendFunc1);
	READ_VALUE(mtl,BlendFunc2);
	mtl.AlphaBlendMode = ConvertAlphaBlendMode(mtl.BlendState, mtl.BlendFunc1, mtl.BlendFunc2);

	READ_VALUE(mtl,AlphaTestRef);
	READ_VALUE(mtl,AlphaTest);
	READ_VALUE(mtl,ZBufferWrite);
	READ_VALUE(mtl,ZBufferTest);
	READ_VALUE(mtl,ScreenSpaceReflections);
	READ_VALUE(mtl,WetnessControlScreenSpaceReflections);
	READ_VALUE(mtl,Decal);
	READ_VALUE(mtl,TwoSided);
	READ_VALUE(mtl,DecalNoFade);
	READ_VALUE(mtl,NonOccluder);
	READ_VALUE(mtl,Refraction);
	READ_VALUE(mtl,RefractionFalloff);
	READ_VALUE(mtl,RefractionPower);
	READ_VALUE(mtl,EnvironmentMapping);
	READ_VALUE(mtl,EnvironmentMappingMaskScale);
	READ_VALUE(mtl,GrayscaleToPaletteColor);
	return true;
error: return false;
}

static bool SaveBaseMaterial(FILE* dst, const MaterialHeader& hdr, const BaseMaterial& mtl)
{
	int tile_uv = (mtl.TileU ? 0x1 : 0) | (mtl.TileV ? 0x2 : 0);
	if (!SaveObject(dst, tile_uv, "")) goto error;
	SAVE_VALUE(mtl, UOffset);
	SAVE_VALUE(mtl, VOffset);
	SAVE_VALUE(mtl, UScale);
	SAVE_VALUE(mtl, VScale);
	SAVE_VALUE(mtl, Alpha);
	SAVE_VALUE(mtl, BlendState);
	SAVE_VALUE(mtl, BlendFunc1);
	SAVE_VALUE(mtl, BlendFunc2);
	SAVE_VALUE(mtl, AlphaTestRef);
	SAVE_VALUE(mtl, AlphaTest);
	SAVE_VALUE(mtl, ZBufferWrite);
	SAVE_VALUE(mtl, ZBufferTest);
	SAVE_VALUE(mtl, ScreenSpaceReflections);
	SAVE_VALUE(mtl, WetnessControlScreenSpaceReflections);
	SAVE_VALUE(mtl, Decal);
	SAVE_VALUE(mtl, TwoSided);
	SAVE_VALUE(mtl, DecalNoFade);
	SAVE_VALUE(mtl, NonOccluder);
	SAVE_VALUE(mtl, Refraction);
	SAVE_VALUE(mtl, RefractionFalloff);
	SAVE_VALUE(mtl, RefractionPower);
	SAVE_VALUE(mtl, EnvironmentMapping);
	SAVE_VALUE(mtl, EnvironmentMappingMaskScale);
	SAVE_VALUE(mtl, GrayscaleToPaletteColor);
	return true;
error: return false;
}

static bool ReadBGSM(FILE* src, const MaterialHeader& hdr, BGSMFile& mtl)
{
	if (!ReadBaseMaterial(src, hdr, mtl)) goto error;
	READ_VALUE(mtl,DiffuseTexture);
	READ_VALUE(mtl,NormalTexture);
	READ_VALUE(mtl,SmoothSpecTexture);
	READ_VALUE(mtl,GreyscaleTexture);
	READ_VALUE(mtl,EnvmapTexture);
	READ_VALUE(mtl,GlowTexture);
	READ_VALUE(mtl,InnerLayerTexture);
	READ_VALUE(mtl,WrinklesTexture);
	READ_VALUE(mtl,DisplacementTexture);
	READ_VALUE(mtl,EnableEditorAlphaRef);
	READ_VALUE(mtl,RimLighting);
	READ_VALUE(mtl,RimPower);
	READ_VALUE(mtl,BackLightPower);
	READ_VALUE(mtl,SubsurfaceLighting);
	READ_VALUE(mtl,SubsurfaceLightingRolloff);
	READ_VALUE(mtl,SpecularEnabled);
	READ_VALUE(mtl,SpecularColor);
	READ_VALUE(mtl,SpecularMult);
	READ_VALUE(mtl,Smoothness);
	READ_VALUE(mtl,FresnelPower);
	READ_VALUE(mtl,WetnessControlSpecScale);
	READ_VALUE(mtl,WetnessControlSpecPowerScale);
	READ_VALUE(mtl,WetnessControlSpecMinvar);
	READ_VALUE(mtl,WetnessControlEnvMapScale);
	READ_VALUE(mtl,WetnessControlFresnelPower);
	READ_VALUE(mtl,WetnessControlMetalness);

	READ_VALUE(mtl,RootMaterialPath);
	READ_VALUE(mtl,AnisoLighting);
	READ_VALUE(mtl,EmitEnabled);
	if (mtl.EmitEnabled)
		READ_VALUE(mtl,EmittanceColor);
	READ_VALUE(mtl,EmittanceMult);
	READ_VALUE(mtl,ModelSpaceNormals);
	READ_VALUE(mtl,ExternalEmittance);
	READ_VALUE(mtl,BackLighting);
	READ_VALUE(mtl,ReceiveShadows);
	READ_VALUE(mtl,HideSecret);
	READ_VALUE(mtl,CastShadows);
	READ_VALUE(mtl,DissolveFade);
	READ_VALUE(mtl,AssumeShadowmask);
	READ_VALUE(mtl,Glowmap);
	READ_VALUE(mtl,EnvironmentMappingWindow);
	READ_VALUE(mtl,EnvironmentMappingEye);
	READ_VALUE(mtl,Hair);
	READ_VALUE(mtl,HairTintColor);
	READ_VALUE(mtl,Tree);
	READ_VALUE(mtl,Facegen);
	READ_VALUE(mtl,SkinTint);
	READ_VALUE(mtl,Tessellate);
	READ_VALUE(mtl,DisplacementTextureBias);
	READ_VALUE(mtl,DisplacementTextureScale);
	READ_VALUE(mtl,TessellationPNScale);
	READ_VALUE(mtl,TessellationBaseFactor);
	READ_VALUE(mtl,TessellationFadeDistance);
	READ_VALUE(mtl,GrayscaleToPaletteScale);
	if (hdr.Version >= 1) {
		READ_VALUE(mtl,SkewSpecularAlpha);
	} else {
		mtl.SkewSpecularAlpha = 0;
	}
	return true;
error: return false;
}


static bool SaveBGSM(FILE* dst, const MaterialHeader& hdr, const BGSMFile& mtl)
{
	if (!SaveBaseMaterial(dst, hdr, mtl)) goto error;
	SAVE_VALUE(mtl, DiffuseTexture);
	SAVE_VALUE(mtl, NormalTexture);
	SAVE_VALUE(mtl, SmoothSpecTexture);
	SAVE_VALUE(mtl, GreyscaleTexture);
	SAVE_VALUE(mtl, EnvmapTexture);
	SAVE_VALUE(mtl, GlowTexture);
	SAVE_VALUE(mtl, InnerLayerTexture);
	SAVE_VALUE(mtl, WrinklesTexture);
	SAVE_VALUE(mtl, DisplacementTexture);
	SAVE_VALUE(mtl, EnableEditorAlphaRef);
	SAVE_VALUE(mtl, RimLighting);
	SAVE_VALUE(mtl, RimPower);
	SAVE_VALUE(mtl, BackLightPower);
	SAVE_VALUE(mtl, SubsurfaceLighting);
	SAVE_VALUE(mtl, SubsurfaceLightingRolloff);
	SAVE_VALUE(mtl, SpecularEnabled);
	SAVE_VALUE(mtl, SpecularColor);
	SAVE_VALUE(mtl, SpecularMult);
	SAVE_VALUE(mtl, Smoothness);
	SAVE_VALUE(mtl, FresnelPower);
	SAVE_VALUE(mtl, WetnessControlSpecScale);
	SAVE_VALUE(mtl, WetnessControlSpecPowerScale);
	SAVE_VALUE(mtl, WetnessControlSpecMinvar);
	SAVE_VALUE(mtl, WetnessControlEnvMapScale);
	SAVE_VALUE(mtl, WetnessControlFresnelPower);
	SAVE_VALUE(mtl, WetnessControlMetalness);
	SAVE_VALUE(mtl, RootMaterialPath);
	SAVE_VALUE(mtl, AnisoLighting);
	SAVE_VALUE(mtl, EmitEnabled);
	if (mtl.EmitEnabled)
		SAVE_VALUE(mtl, EmittanceColor);
	SAVE_VALUE(mtl, EmittanceMult);
	SAVE_VALUE(mtl, ModelSpaceNormals);
	SAVE_VALUE(mtl, ExternalEmittance);
	SAVE_VALUE(mtl, BackLighting);
	SAVE_VALUE(mtl, ReceiveShadows);
	SAVE_VALUE(mtl, HideSecret);
	SAVE_VALUE(mtl, CastShadows);
	SAVE_VALUE(mtl, DissolveFade);
	SAVE_VALUE(mtl, AssumeShadowmask);
	SAVE_VALUE(mtl, Glowmap);
	SAVE_VALUE(mtl, EnvironmentMappingWindow);
	SAVE_VALUE(mtl, EnvironmentMappingEye);
	SAVE_VALUE(mtl, Hair);
	SAVE_VALUE(mtl, HairTintColor);
	SAVE_VALUE(mtl, Tree);
	SAVE_VALUE(mtl, Facegen);
	SAVE_VALUE(mtl, SkinTint);
	SAVE_VALUE(mtl, Tessellate);
	SAVE_VALUE(mtl, DisplacementTextureBias);
	SAVE_VALUE(mtl, DisplacementTextureScale);
	SAVE_VALUE(mtl, TessellationPNScale);
	SAVE_VALUE(mtl, TessellationBaseFactor);
	SAVE_VALUE(mtl, TessellationFadeDistance);
	SAVE_VALUE(mtl, GrayscaleToPaletteScale);
	if (hdr.Version >= 1) {
		SAVE_VALUE(mtl, SkewSpecularAlpha);
	}
	return true;
error: return false;
}

#undef READ_VALUE

#define READ_VALUE(dst, x, def)  if (!ReadObject(src, ##dst . ##x, #x)) goto error;

static bool ReadBGEM(FILE* src, const MaterialHeader& hdr, BGEMFile& mtl)
{
	if (!ReadBaseMaterial(src, hdr, mtl)) goto error;
	READ_VALUE(mtl, BaseTexture, TEXT(""));
	READ_VALUE(mtl, GrayscaleTexture, TEXT(""));
	READ_VALUE(mtl, EnvmapTexture, TEXT(""));
	READ_VALUE(mtl, NormalTexture, TEXT(""));
	READ_VALUE(mtl, EnvmapMaskTexture, TEXT(""));
	READ_VALUE(mtl, BloodEnabled, false);
	READ_VALUE(mtl, EffectLightingEnabled, false);
	READ_VALUE(mtl, FalloffEnabled, false);
	READ_VALUE(mtl, FalloffColorEnabled, false);
	READ_VALUE(mtl, GrayscaleToPaletteAlpha, false);
	READ_VALUE(mtl, SoftEnabled, false);
	READ_VALUE(mtl, BaseColor, empty_color3);
	READ_VALUE(mtl, BaseColorScale, 1.0f);
	READ_VALUE(mtl, FalloffStartAngle, 0.0f);
	READ_VALUE(mtl, FalloffStopAngle, 0.0f);
	READ_VALUE(mtl, FalloffStartOpacity, 0.0f);
	READ_VALUE(mtl, FalloffStopOpacity, 0.0f);
	READ_VALUE(mtl, LightingInfluence, 1.0f);
	READ_VALUE(mtl, EnvmapMinLOD, '\x0');
	READ_VALUE(mtl, SoftDepth, 100.0f);

	return true;
error: return false;
}

static bool SaveBGEM(FILE* dst, const MaterialHeader& hdr, const BGEMFile& mtl)
{
	if (!SaveBaseMaterial(dst, hdr, mtl)) goto error;
	SAVE_VALUE(mtl, BaseTexture);
	SAVE_VALUE(mtl, GrayscaleTexture);
	SAVE_VALUE(mtl, EnvmapTexture);
	SAVE_VALUE(mtl, NormalTexture);
	SAVE_VALUE(mtl, EnvmapMaskTexture);
	SAVE_VALUE(mtl, BloodEnabled);
	SAVE_VALUE(mtl, EffectLightingEnabled);
	SAVE_VALUE(mtl, FalloffEnabled);
	SAVE_VALUE(mtl, FalloffColorEnabled);
	SAVE_VALUE(mtl, GrayscaleToPaletteAlpha);
	SAVE_VALUE(mtl, SoftEnabled);
	SAVE_VALUE(mtl, BaseColor);
	SAVE_VALUE(mtl, BaseColorScale);
	SAVE_VALUE(mtl, FalloffStartAngle);
	SAVE_VALUE(mtl, FalloffStopAngle);
	SAVE_VALUE(mtl, FalloffStartOpacity);
	SAVE_VALUE(mtl, FalloffStopOpacity);
	SAVE_VALUE(mtl, LightingInfluence);
	SAVE_VALUE(mtl, EnvmapMinLOD);
	SAVE_VALUE(mtl, SoftDepth);
	return true;
error: return false;
}
#undef READ_VALUE

#define READ_INT(dst, x, def)    if (!ReadObject(src, ##dst . ##x, "i"#x)) ##dst.##x = def;
#define READ_FLOAT(dst, x, def)  if (!ReadObject(src, ##dst . ##x, "f"#x)) ##dst.##x = def;
#define READ_STRING(dst, x, def) if (!ReadObject(src, ##dst . ##x, "s"#x)) ##dst.##x = def;
#define READ_BOOL(dst, x, def)   if (!ReadObject(src, ##dst . ##x, "b"#x)) ##dst.##x = def;
#define READ_ENUM(dst, x, def)   if (!ReadObject(src, ##dst . ##x, "e"#x)) ##dst.##x = def;
#define READ_BYTE(dst, x, def)   if (!ReadObject(src, ##dst . ##x, "f"#x)) ##dst.##x = def;
#define READ_COLOR(dst, x, def)  if (!ReadObject(src, ##dst . ##x, "c"#x)) ##dst.##x = def;



static bool ReadBaseMaterial(rapidjson::Value& src, const MaterialHeader& hdr, BaseMaterial& mtl)
{
	bool TileU, TileV;
	ReadObject(src, TileU, "bTileU");
	ReadObject(src, TileV, "bTileV");

	READ_BOOL(mtl, TileU, false);
	READ_BOOL(mtl, TileV, false);
	READ_FLOAT(mtl, UOffset, 0.0f);
	READ_FLOAT(mtl, VOffset, 0.0f);
	READ_FLOAT(mtl, UScale, 1.0f);
	READ_FLOAT(mtl, VScale, 1.0f);
	READ_FLOAT(mtl, Alpha, 1.0f);
	READ_ENUM(mtl, AlphaBlendMode, ABMT_Unknown);
	ConvertAlphaBlendMode(mtl.AlphaBlendMode, mtl.BlendState, mtl.BlendFunc1, mtl.BlendFunc2);
	READ_BYTE(mtl, AlphaTestRef, '\0');
	READ_BOOL(mtl, AlphaTest, false);
	READ_BOOL(mtl, ZBufferWrite, true);
	READ_BOOL(mtl, ZBufferTest, true);
	READ_BOOL(mtl, ScreenSpaceReflections, false);
	READ_BOOL(mtl, WetnessControlScreenSpaceReflections, false);
	READ_BOOL(mtl, Decal, false);
	READ_BOOL(mtl, TwoSided, false);
	READ_BOOL(mtl, DecalNoFade, false);
	READ_BOOL(mtl, NonOccluder, false);
	READ_BOOL(mtl, Refraction, false);
	READ_BOOL(mtl, RefractionFalloff, false);
	READ_FLOAT(mtl, RefractionPower, 0.0f);
	READ_BOOL(mtl, EnvironmentMapping, false);
	READ_FLOAT(mtl, EnvironmentMappingMaskScale, 1.0f);
	READ_BOOL(mtl, GrayscaleToPaletteColor, false);

	return true;
}

static bool ReadBGEM(rapidjson::Value& src, const MaterialHeader& hdr, BGEMFile& mtl)
{
	if (!ReadBaseMaterial(src, hdr, mtl)) goto error;

	READ_STRING(mtl, BaseTexture, TEXT(""));
	READ_STRING(mtl, GrayscaleTexture, TEXT(""));
	READ_STRING(mtl, EnvmapTexture, TEXT(""));
	READ_STRING(mtl, NormalTexture, TEXT(""));
	READ_STRING(mtl, EnvmapMaskTexture, TEXT(""));
	READ_BOOL(mtl, BloodEnabled, false);
	READ_BOOL(mtl, EffectLightingEnabled, false);
	READ_BOOL(mtl, FalloffEnabled, false);
	READ_BOOL(mtl, FalloffColorEnabled, false);
	READ_BOOL(mtl, GrayscaleToPaletteAlpha, false);
	READ_BOOL(mtl, SoftEnabled, false);
	READ_COLOR(mtl, BaseColor, empty_color3);
	READ_FLOAT(mtl, BaseColorScale, 1.0f);
	READ_FLOAT(mtl, FalloffStartAngle, 0.0f);
	READ_FLOAT(mtl, FalloffStopAngle, 0.0f);
	READ_FLOAT(mtl, FalloffStartOpacity, 0.0f);
	READ_FLOAT(mtl, FalloffStopOpacity, 0.0f);
	READ_FLOAT(mtl, LightingInfluence, 1.0f);
	READ_BYTE(mtl, EnvmapMinLOD, '\x0');
	READ_FLOAT(mtl, SoftDepth, 100.0f);
	return true;
error: return false;
}

static bool ReadBGSM(rapidjson::Value& src, const MaterialHeader& hdr, BGSMFile& mtl)
{
	if (!ReadBaseMaterial(src, hdr, mtl)) goto error;

	READ_STRING(mtl, DiffuseTexture, TEXT(""));
	READ_STRING(mtl, NormalTexture, TEXT(""));
	READ_STRING(mtl, SmoothSpecTexture, TEXT(""));
	READ_STRING(mtl, GreyscaleTexture, TEXT(""));
	READ_STRING(mtl, EnvmapTexture, TEXT(""));
	READ_STRING(mtl, GlowTexture, TEXT(""));
	READ_STRING(mtl, InnerLayerTexture, TEXT(""));
	READ_STRING(mtl, WrinklesTexture, TEXT(""));
	READ_STRING(mtl, DisplacementTexture, TEXT(""));
	READ_BOOL(mtl, EnableEditorAlphaRef, false);
	READ_BOOL(mtl, RimLighting, false);
	READ_FLOAT(mtl, RimPower, 0.0f);
	READ_FLOAT(mtl, BackLightPower, 0.0f);
	READ_BOOL(mtl, SubsurfaceLighting, false);
	READ_FLOAT(mtl, SubsurfaceLightingRolloff, 0.0f);
	READ_BOOL(mtl, SpecularEnabled, false);
	READ_COLOR(mtl, SpecularColor, empty_color3);
	READ_FLOAT(mtl, SpecularMult, 0.0f);
	READ_FLOAT(mtl, Smoothness, 0.0f);
	READ_FLOAT(mtl, FresnelPower, 0.0f);
	READ_FLOAT(mtl, WetnessControlSpecScale, 0.0f);
	READ_FLOAT(mtl, WetnessControlSpecPowerScale, 0.0f);
	READ_FLOAT(mtl, WetnessControlSpecMinvar, 0.0f);
	READ_FLOAT(mtl, WetnessControlEnvMapScale, 0.0f);
	READ_FLOAT(mtl, WetnessControlFresnelPower, 0.0f);
	READ_FLOAT(mtl, WetnessControlMetalness, 0.0f);

	READ_STRING(mtl, RootMaterialPath, TEXT(""));
	READ_BOOL(mtl, AnisoLighting, false);
	READ_BOOL(mtl, EmitEnabled, false);
	READ_COLOR(mtl, EmittanceColor, empty_color3);// if(mtl,  EmitEnabled);
	READ_FLOAT(mtl, EmittanceMult, 0.0f);
	READ_BOOL(mtl, ModelSpaceNormals, false);
	READ_BOOL(mtl, ExternalEmittance, false);
	READ_BOOL(mtl, BackLighting, false);
	READ_BOOL(mtl, ReceiveShadows, false);
	READ_BOOL(mtl, HideSecret, false);
	READ_BOOL(mtl, CastShadows, false);
	READ_BOOL(mtl, DissolveFade, false);
	READ_BOOL(mtl, AssumeShadowmask, false);
	READ_BOOL(mtl, Glowmap, false);
	READ_BOOL(mtl, EnvironmentMappingWindow, false);
	READ_BOOL(mtl, EnvironmentMappingEye, false);
	READ_BOOL(mtl, Hair, false);
	READ_COLOR(mtl, HairTintColor, empty_color3);
	READ_BOOL(mtl, Tree, false);
	READ_BOOL(mtl, Facegen, false);
	READ_BOOL(mtl, SkinTint, false);
	READ_BOOL(mtl, Tessellate, false);
	READ_FLOAT(mtl, DisplacementTextureBias, 0.0f);
	READ_FLOAT(mtl, DisplacementTextureScale, 0.0f);
	READ_FLOAT(mtl, TessellationPNScale, 0.0f);
	READ_FLOAT(mtl, TessellationBaseFactor, 0.0f);
	READ_FLOAT(mtl, TessellationFadeDistance, 0.0f);
	READ_FLOAT(mtl, GrayscaleToPaletteScale, 0.0f);
	READ_BOOL(mtl, SkewSpecularAlpha, false); // (mtl, header.Version >= 1);    
	READ_BOOL(mtl, GrayscaleToPaletteColor, false);
	return true;
error: return false;
}


bool ReadBGSMFile(const tstring& filename, BGSMFile& bgsm)
{
	FILE *file = _tfopen(filename.c_str(), TEXT("rb"));
	if (file == nullptr) return false;

	bool result = false;
	MaterialHeader hdr;
	if (!ReadMtlHeader(file, hdr)) goto error;
	if (strncmp(hdr.Signature, "BGSM", 4) == 0) {
		if (!ReadBGSM(file, hdr, bgsm)) goto error;
		result = true;
	}
	else if (strncmp(hdr.Signature, "BGEM", 4) == 0) {
		return false;
	} else {
		// read as JSON files
		fseek(file, 0, SEEK_SET);
		struct _stat filestat;
		_tstat(filename.c_str(), &filestat);
		char * json = static_cast<char*>(malloc(filestat.st_size+1));
		try
		{
			if (fread(json, sizeof(char), filestat.st_size, file) == filestat.st_size)
			{
				json[filestat.st_size] = 0;
				rapidjson::Document d;
				d.Parse(json);

				strncpy(hdr.Signature, "BGSM", 4);
				hdr.Version = 1;
			}
			result = true;
		}
		catch(...)
		{
		}
		free(json);
	}
	goto exit;
error:
	result = false;
exit:
	fclose(file);
	return result;
}

bool SaveBGSMFile(const tstring& filename, const BGSMFile& bgsm)
{
	FILE *file = _tfopen(filename.c_str(), TEXT("wb"));
	if (file == nullptr) return false;

	bool result = false;
	MaterialHeader hdr;
	strncpy(hdr.Signature, "BGSM", 4);
	hdr.Version = 1;
	if (!SaveMtlHeader(file, hdr)) goto error;
	if (!SaveBGSM(file, hdr, bgsm)) goto error;
	result = true;
	goto exit;
error:
	result = false;
exit:
	fclose(file);
	return result;
}

bool ReadBGEMFile(const tstring& filename, BGEMFile& BGEM)
{
	FILE *file = _tfopen(filename.c_str(), TEXT("rb"));
	if (file == nullptr) return false;

	bool result = false;
	MaterialHeader hdr;
	if (!ReadMtlHeader(file, hdr)) goto error;
	if (strncmp(hdr.Signature, "BGEM", 4) == 0) {
		if (!ReadBGEM(file, hdr, BGEM)) goto error;
		result = true;
	}
	else if (strncmp(hdr.Signature, "BGEM", 4) == 0) {
		return false;
	}
	else {
		// read as JSON files
		fseek(file, 0, SEEK_SET);
		struct _stat filestat;
		_tstat(filename.c_str(), &filestat);
		char * json = static_cast<char*>(malloc(filestat.st_size + 1));
		try
		{
			if (fread(json, sizeof(char), filestat.st_size, file) == filestat.st_size)
			{
				json[filestat.st_size] = 0;
				rapidjson::Document d;
				d.Parse(json);

				strncpy(hdr.Signature, "BGEM", 4);
				hdr.Version = 1;
			}
			result = true;
		}
		catch (...)
		{
		}
		free(json);
	}
	goto exit;
error:
	result = false;
exit:
	fclose(file);
	return result;
}


bool SaveBGEMFile(const tstring& filename, const BGEMFile& bgem)
{
	FILE *file = _tfopen(filename.c_str(), TEXT("wb"));
	if (file == nullptr) return false;

	bool result = false;
	MaterialHeader hdr;
	strncpy(hdr.Signature, "BGEM", 4);
	hdr.Version = 1;
	if (!SaveMtlHeader(file, hdr)) goto error;
	if (!SaveBGEM(file, hdr, bgem)) goto error;
	result = true;
	goto exit;
error:
	result = false;
exit:
	fclose(file);
	return result;
}

#undef READ_VALUE

#if 0
#pragma region TYPE

typedef const byte * LPCBYTE;

struct TYPE
{
	virtual ~TYPE(){}
	virtual bool init(LPVOID val, LPVOID default, DWORD size) const = 0;
	virtual bool recall(const rapidjson::Value& in, LPVOID val) const = 0;
	virtual bool store(rapidjson::Value& out, LPCVOID val) = 0;
};

template<typename U>
struct VTYPE : public TYPE
{
	bool init(LPVOID val, LPVOID defvalue, DWORD size) const override { return ::InitValue(static_cast<U*>(val), static_cast<U*>(defvalue), size); }
	bool recall(const rapidjson::Value& in, LPVOID val) const override { return ::Recall(in, *static_cast<U*>(val)); }
	bool store(rapidjson::Value& out, LPCVOID val) override { return ::Store(out, *static_cast<U const *>(val)); }
};

struct VARIABLE
{
	VARIABLE() : ShortName(nullptr), MemberAddr(0), MemberType(nullptr), DefaultValue(nullptr), ValueSize(0) {}

	template<typename U>
	VARIABLE(LPCSTR sName, U const & member, U const & defvalue)
		: ShortName(sName), MemberAddr(static_cast<LPCBYTE>(&member) - static_cast<LPCBYTE>(nullptr))
		, MemberType(new VTYPE<U>())
	{
		SetDefault(defvalue);
	}

	template<typename U>
	VARIABLE(LPCSTR sName, U const & member)
		: ShortName(sName), MemberAddr(static_cast<LPCBYTE>(&member) - static_cast<LPCBYTE>(nullptr))
		, MemberType(new VTYPE<U>()), DefaultValue(nullptr), ValueSize(sizeof(U))
	{}

	~VARIABLE() {
		if (ValueSize && DefaultValue) {
			delete DefaultValue; DefaultValue = nullptr; ValueSize = 0;
		}
		if (MemberType) {
			delete MemberType; MemberType = nullptr;
		}
	}
	template<typename T> void InitValue(T& pThis) const { MemberType->init(GetMember(pThis), DefaultValue, ValueSize); }
	template<typename T> LPVOID GetMember(T& pThis) const { return static_cast<void*>(static_cast<char*>(&pThis) + this->MemberAddr); }
	template<typename U> void SetDefault(U defvalue) { ValueSize = sizeof(U); DefaultValue = new U(defvalue); }
	template <typename U> const U* GetDefault() const { return static_cast<U*>(DefaultValue); }

	template <typename T> 
	bool ReadValue(rapidjson::Value& src, T& pThis) const {
		return ReadValuePtr(src, &pThis);
	}
	bool ReadValuePtr(rapidjson::Value& src, LPVOID pThis) const {
		if (!MemberType->recall(src, GetMember(pThis)))
			return MemberType->init(GetMember(pThis), DefaultValue, ValueSize);
		return true;
	}

	LPCSTR ShortName;
	intptr_t MemberAddr;
	TYPE *MemberType;
	LPVOID DefaultValue;
	DWORD ValueSize;
};

#define BEGIN_INI_MAP(T) \
	LPCSTR GetTypeName() const { return #T; } \
	void *GetMember(T& pThis, const VARIABLE* memptr) const { return (void*)(((char*)&pThis) + memptr->MemberAddr); } \
	const VARIABLE* GetInfBaseDefmap() const { return nullptr; } \
	const VARIABLE* GetInfDefmap() const { return InternalGetInfDefmap(); } \
	static VARIABLE* InternalGetInfDefmap() { \
	const T* pThis = 0; static VARIABLE map[] = { \

#define BEGIN_INI_MAP_WITH_BASE(T, U) \
	LPCSTR GetTypeName() const { return #T; } \
	void *GetMember(T& pThis, const VARIABLE* memptr) const { return (void*)(((char*)&pThis) + memptr->MemberAddr); } \
	const VARIABLE* GetInfBaseDefmap() const { return U::GetInfDefmap(); } \
	const VARIABLE* GetInfDefmap() const { return InternalGetInfDefmap(); } \
	static VARIABLE* InternalGetInfDefmap() { \
	const T* pThis = 0; static VARIABLE map[] = { \

#define END_INI_MAP() \
	VARIABLE() };\
	return map;} \

#define ADDITEM(member, defval) \
	VARIABLE(#member, pThis->member, defval), \

#define READ_BOOL(member, defval) \
	VARIABLE("b"#member, pThis->member, defval), \

#define READ_INT(member, defval) \
	VARIABLE("i"#member, pThis->member, defval), \

#define READ_FLOAT(member, defval) \
	VARIABLE("f"#member, pThis->member, defval), \

#define READ_BYTE(member, defval) \
	VARIABLE("f"#member, pThis->member, defval), \

#define READ_STRING(member) \
	VARIABLE("s"#member, pThis->member), \

#define READ_ENUM(member, defval) \
	VARIABLE("e"#member, pThis->member, ##defval), \

#define READ_CUSTOM(name, member) \
	VARIABLE(name, pThis->member), \

#define ADDCLASS(member) \
	VARIABLE(#member, pThis->member), \

template <typename T>
void InitValue(T* val, T* defval, DWORD size) {
	if (defval == nullptr) {
		memset(val, 0, size);
	} else {
		*val = *defval;
	}
}

//bool Recall(const VARIABLE* defmap, rapidjson::Value& in, LPVOID val) {
//	
//}
//bool Store(const VARIABLE* defmap, rapidjson::Value& out, LPCVOID val);
//unsigned int SizeOf(const VARIABLE* defmap, rapidjson::Value& in, LPCVOID val);

bool Init(const VARIABLE* defmap, LPVOID val) {
	for (const VARIABLE* var = defmap; var->ShortName != nullptr; ++var)
		var->InitValue(val);
	return true;
}
bool Recall(const VARIABLE* defmap, rapidjson::Value& in, LPVOID val) {
	for (const VARIABLE* var = defmap; var->ShortName != nullptr; ++var)
		var->ReadValuePtr(in, val);
	return true;
}

bool Store(const VARIABLE* defmap, rapidjson::Value& out, LPCVOID val);

#pragma endregion

struct MaterialSerializer
{
	BEGIN_INI_MAP(BaseMaterial)
		READ_BOOL(TileU, false)
		READ_BOOL(TileV, false)
		READ_FLOAT(UOffset, 0.0f)
		READ_FLOAT(VOffset, 0.0f)
		READ_FLOAT(UScale, 1.0f)
		READ_FLOAT(VScale, 1.0f)
		READ_FLOAT(Alpha, 1.0f)
		READ_ENUM(AlphaBlendMode, Unknown)
		READ_BYTE(AlphaTestRef, '\0')
		READ_BOOL(AlphaTest, false)
		READ_BOOL(ZBufferWrite, true)
		READ_BOOL(ZBufferTest, true)
		READ_BOOL(ScreenSpaceReflections, false)
		READ_BOOL(WetnessControlScreenSpaceReflections, false)
		READ_BOOL(Decal, false)
		READ_BOOL(TwoSided, false)
		READ_BOOL(DecalNoFade, false)
		READ_BOOL(NonOccluder, false)
		READ_BOOL(Refraction, false)
		READ_BOOL(RefractionFalloff, false)
		READ_FLOAT(RefractionPower, 0.0f)
		READ_BOOL(EnvironmentMapping, false)
		READ_FLOAT(EnvironmentMappingMaskScale, 1.0f)
		READ_BOOL(GrayscaleToPaletteColor, false)
	END_INI_MAP()

	void Init(BGSMFile& value) const { ::Init(this->GetInfDefmap(), &value); }

	bool Read(BGSMFile& value, rapidjson::Value& src) const {
		return ::Recall(this->InternalGetInfDefmap(), src, &value);
	}
};

const static tstring empty_string;

struct BGSMSerializer : MaterialSerializer
{
	BEGIN_INI_MAP_WITH_BASE(BGSMFile, MaterialSerializer)
		READ_STRING(DiffuseTexture)
		READ_STRING(NormalTexture)
		READ_STRING(SmoothSpecTexture)
		READ_STRING(GreyscaleTexture)
		READ_STRING(EnvmapTexture)
		READ_STRING(GlowTexture)
		READ_STRING(InnerLayerTexture)
		READ_STRING(WrinklesTexture)
		READ_STRING(DisplacementTexture)
		READ_BOOL(EnableEditorAlphaRef, false)
		READ_BOOL(RimLighting, false)
		READ_FLOAT(RimPower, 0.0f)
		READ_FLOAT(BackLightPower, 0.0f)
		READ_BOOL(SubsurfaceLighting, false)
		READ_FLOAT(SubsurfaceLightingRolloff, 0.0f)
		READ_BOOL(SpecularEnabled, false)
		READ_CUSTOM("cSpecularColor", SpecularColor)
		READ_FLOAT(SpecularMult, 0.0f)
		READ_FLOAT(Smoothness, 0.0f)
		READ_FLOAT(FresnelPower, 0.0f)
		READ_FLOAT(WetnessControlSpecScale, 0.0f)
		READ_FLOAT(WetnessControlSpecPowerScale, 0.0f)
		READ_FLOAT(WetnessControlSpecMinvar, 0.0f)
		READ_FLOAT(WetnessControlEnvMapScale, 0.0f)
		READ_FLOAT(WetnessControlFresnelPower, 0.0f)
		READ_FLOAT(WetnessControlMetalness, 0.0f)

		READ_STRING(RootMaterialPath)
		READ_BOOL(AnisoLighting, false)
		READ_BOOL(EmitEnabled, false)
		READ_CUSTOM("cEmittanceColor", EmittanceColor)// if( EmitEnabled)
		READ_FLOAT(EmittanceMult, 0.0f)
		READ_BOOL(ModelSpaceNormals, false)
		READ_BOOL(ExternalEmittance, false)
		READ_BOOL(BackLighting, false)
		READ_BOOL(ReceiveShadows, false)
		READ_BOOL(HideSecret, false)
		READ_BOOL(CastShadows, false)
		READ_BOOL(DissolveFade, false)
		READ_BOOL(AssumeShadowmask, false)
		READ_BOOL(Glowmap, false)
		READ_BOOL(EnvironmentMappingWindow, false)
		READ_BOOL(EnvironmentMappingEye, false)
		READ_BOOL(Hair, false)
		READ_CUSTOM("cHairTintColor", HairTintColor)
		READ_BOOL(Tree, false)
		READ_BOOL(Facegen, false)
		READ_BOOL(SkinTint, false)
		READ_BOOL(Tessellate, false)
		READ_FLOAT(DisplacementTextureBias, 0.0f)
		READ_FLOAT(DisplacementTextureScale, 0.0f)
		READ_FLOAT(TessellationPNScale, 0.0f)
		READ_FLOAT(TessellationBaseFactor, 0.0f)
		READ_FLOAT(TessellationFadeDistance, 0.0f)
		READ_FLOAT(GrayscaleToPaletteScale, 0.0f)
		READ_BOOL(SkewSpecularAlpha, false) // (header.Version >= 1)    
		READ_BOOL(GrayscaleToPaletteColor, false)
	END_INI_MAP()

	void Init(BGSMFile& value) const { ::Init(this->GetInfDefmap(), &value); }

	bool Read(BGSMFile& value, rapidjson::Value& src) const {
		return ::Recall(this->InternalGetInfDefmap(), src, &value);
	}
};

#endif

bool InitialzeBGSM(BGSMFile& bgsm)
{
	// TODO: Set defaults.  no done by empty json so defaults always are set
	//bgsm = BGSMFile();
	MaterialHeader hdr; 
	strncpy(hdr.Signature, "BGSM", 4);
	hdr.Version = 1;
	rapidjson::Document d;
	d.Parse("{}");
	return ReadBGSM(d, hdr, bgsm);
}


bool InitialzeBGEM(BGEMFile& bgem)
{
	// TODO: Set defaults.  no done by empty json so defaults always are set
	MaterialHeader hdr;
	strncpy(hdr.Signature, "BGEM", 4);
	hdr.Version = 1;
	rapidjson::Document d;
	d.Parse("{}");
	return ReadBGEM(d, hdr, bgem);
}
