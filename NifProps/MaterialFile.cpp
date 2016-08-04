#include "MaterialFile.h"

const int currentVersion = 1;

const USHORT VERSION_CHUNKID = 0x0230;
const USHORT NAME_CHUNKID = 0x0231;
const USHORT FILENAME_CHUNKID = 0x0232;
const USHORT FILE_CHUNKID = 0x0233;

#define LOAD_VALUE(dst, x)  if (!ReadObject(iload, ##dst . ##x, #x)) goto error;
#define SAVE_VALUE(src, x)  if (!SaveObject(isave, ##src . ##x, #x)) goto error;


template <typename T>
bool ReadObject(ILoad* iload, T& x, const char *name) {
	ULONG nw = 0;
	IOResult res = iload->Read(static_cast<char*>(static_cast<LPVOID>(&x)), sizeof(x), &nw);
	if (res != IO_OK) return false;
	if (sizeof(x) != nw) return false;
	return true;
}

template <>
bool ReadObject<tstring>(ILoad* iload, tstring& x, const char *name) {
	//wchar_t *ptr = nullptr;
	//int len = 0;
	//if (!ReadObject(iload, len, "")) return IO_ERROR;
	//if (len >= MAX_PATH) return IO_ERROR;
	//IOResult res = iload->ReadWStringChunk(&ptr);
	//x = tstring(ptr, len);
	//return (res == IO_OK);

	USES_CONVERSION;
	ULONG len = 0;
	wchar_t *ptr = nullptr;
	if (!ReadObject(iload, len, "")) goto error;
	if (len >= MAX_PATH) goto error;
	ULONG nw = 0;
	ULONG size = sizeof(wchar_t)*(len);
	ptr = static_cast<wchar_t*>(_alloca(size));
	IOResult res = iload->Read(ptr, size, &nw);
	if (res != IO_OK) return false;
	if (size != nw) return false;
	ptr[len-1] = 0;
	x = W2T(ptr);
	return true;
error: return false;
}


template <typename T>
bool SaveObject(ISave* file, T& x, const char *name) {
	ULONG nw = 0;
	IOResult res = file->Write(static_cast<char*>(static_cast<void*>(&x)), sizeof(x), &nw);
	if (res != IO_OK || nw != sizeof(x)) return false;
	return true;
}

template <>
bool SaveObject<tstring>(ISave* file, tstring& x, const char *name) {
	//ULONG nw = 0;
	//int len = x.size();
	//IOResult res = file->Write(&len, sizeof(len), &nw);
	//if (res != IO_OK) return res;
	//res = file->WriteWString(x.c_str());
	//return (res == IO_OK);
	USES_CONVERSION;
	ULONG len = ULONG(x.size() + 1); // include null terminator
	ULONG size = ULONG(sizeof(wchar_t)*len);
	LPCWSTR ptr = T2W(x.c_str());
	if (!SaveObject(file, len, "")) return false;
	ULONG nw = 0;
	IOResult res = file->Write(ptr, size, &nw);
	if (res != IO_OK || nw != size) return false;
	return true;
}

IOResult MaterialReference::LoadMaterialChunk(BaseMaterial &mtl, ILoad* iload)
{
	int tile_uv;
	if (!ReadObject(iload, tile_uv, "")) goto error;
	mtl.TileU = (tile_uv & 0x1) != 0;
	mtl.TileV = (tile_uv & 0x2) != 0;
	LOAD_VALUE(mtl, UOffset);
	LOAD_VALUE(mtl, VOffset);
	LOAD_VALUE(mtl, UScale);
	LOAD_VALUE(mtl, VScale);
	LOAD_VALUE(mtl, Alpha);
	LOAD_VALUE(mtl, BlendState);
	LOAD_VALUE(mtl, BlendFunc1);
	LOAD_VALUE(mtl, BlendFunc2);
	mtl.AlphaBlendMode = ConvertAlphaBlendMode(mtl.BlendState, mtl.BlendFunc1, mtl.BlendFunc2);
	LOAD_VALUE(mtl, AlphaTestRef);
	LOAD_VALUE(mtl, AlphaTest);
	LOAD_VALUE(mtl, ZBufferWrite);
	LOAD_VALUE(mtl, ZBufferTest);
	LOAD_VALUE(mtl, ScreenSpaceReflections);
	LOAD_VALUE(mtl, WetnessControlScreenSpaceReflections);
	LOAD_VALUE(mtl, Decal);
	LOAD_VALUE(mtl, TwoSided);
	LOAD_VALUE(mtl, DecalNoFade);
	LOAD_VALUE(mtl, NonOccluder);
	LOAD_VALUE(mtl, Refraction);
	LOAD_VALUE(mtl, RefractionFalloff);
	LOAD_VALUE(mtl, RefractionPower);
	LOAD_VALUE(mtl, EnvironmentMapping);
	LOAD_VALUE(mtl, EnvironmentMappingMaskScale);
	LOAD_VALUE(mtl, GrayscaleToPaletteColor);
	return IO_OK;
error: return IO_ERROR;
}

IOResult MaterialReference::SaveMaterialChunk(BaseMaterial& mtl, ISave* isave)
{
	int tile_uv = (mtl.TileU ? 0x1 : 0) | (mtl.TileV ? 0x02 : 0);
	if (!SaveObject(isave, tile_uv, "")) goto error;
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
	return IO_OK;
error: return IO_ERROR;
}


#pragma region ("BGSM Class Serializer")
class BGSMFileClassDesc :public ClassDesc2 {
public:
	int            IsPublic()  override { return FALSE; }
	void *         Create(BOOL loading = FALSE)  override { return new BGSMFileReference(); }
	const TCHAR *  ClassName()  override { return GetString(IDS_BGSM_FILE); }
	SClass_ID      SuperClassID()  override { return MATERIALFILE_CLASS_ID; }
	Class_ID       ClassID()  override { return BGSMFILE_CLASS_ID; }
	const TCHAR*   Category()  override { return GetString(IDS_CATEGORY); }
	const TCHAR*   InternalName()  override { return _T("MaterialFile"); }
	HINSTANCE      HInstance()  override { return hInstance; }          // returns owning module handle
};

static BGSMFileClassDesc theBGSMFileClassDesc;

extern ClassDesc2* GetBGSMFileClassDesc()
{
	return &theBGSMFileClassDesc;
}

IOResult BGSMFileReference::Load(ILoad* iload)
{
	USES_CONVERSION;
	IOResult res;
	ULONG nb;
	int version = 1;
	wchar_t *ptr = nullptr;
	TSTR name, filename;

	res = MaterialReference::Load(iload);
	if (res != IO_OK) return res;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case VERSION_CHUNKID:
			res = iload->Read(&version, sizeof(version), &nb);
			break;
		case NAME_CHUNKID:
			res = iload->ReadWStringChunk(&ptr);
			this->materialName = W2T(ptr);
			break;
		case FILENAME_CHUNKID:
			res = iload->ReadWStringChunk(&ptr);
			this->materialFileName = W2T(ptr);
			break;
		case FILE_CHUNKID:
			res = LoadMaterialChunk(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}

	return IO_OK;
}

IOResult BGSMFileReference::Save(ISave* isave)
{
	IOResult res;
	ULONG nb;
	res = MaterialReference::Save(isave);
	if (res != IO_OK) return res;

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write(&currentVersion, sizeof(int), &nb);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(NAME_CHUNKID);
	res = isave->WriteWString(materialName);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(FILENAME_CHUNKID);
	res = isave->WriteWString(materialFileName);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(FILE_CHUNKID);
	res = SaveMaterialChunk(isave);
	isave->EndChunk();
	if (res != IO_OK) return res;

	return res;
}

IOResult BGSMFileReference::LoadMaterialChunk(ILoad* iload)
{
	IOResult res = MaterialReference::LoadMaterialChunk(file, iload);
	if (res != IO_OK) return res;

	LOAD_VALUE(file, DiffuseTexture);
	LOAD_VALUE(file, NormalTexture);
	LOAD_VALUE(file, SmoothSpecTexture);
	LOAD_VALUE(file, GreyscaleTexture);
	LOAD_VALUE(file, EnvmapTexture);
	LOAD_VALUE(file, GlowTexture);
	LOAD_VALUE(file, InnerLayerTexture);
	LOAD_VALUE(file, WrinklesTexture);
	LOAD_VALUE(file, DisplacementTexture);
	LOAD_VALUE(file, EnableEditorAlphaRef);
	LOAD_VALUE(file, RimLighting);
	LOAD_VALUE(file, RimPower);
	LOAD_VALUE(file, BackLightPower);
	LOAD_VALUE(file, SubsurfaceLighting);
	LOAD_VALUE(file, SubsurfaceLightingRolloff);
	LOAD_VALUE(file, SpecularEnabled);
	LOAD_VALUE(file, SpecularColor);
	LOAD_VALUE(file, SpecularMult);
	LOAD_VALUE(file, Smoothness);
	LOAD_VALUE(file, FresnelPower);
	LOAD_VALUE(file, WetnessControlSpecScale);
	LOAD_VALUE(file, WetnessControlSpecPowerScale);
	LOAD_VALUE(file, WetnessControlSpecMinvar);
	LOAD_VALUE(file, WetnessControlEnvMapScale);
	LOAD_VALUE(file, WetnessControlFresnelPower);
	LOAD_VALUE(file, WetnessControlMetalness);
	LOAD_VALUE(file, RootMaterialPath);
	LOAD_VALUE(file, AnisoLighting);
	LOAD_VALUE(file, EmitEnabled);
	LOAD_VALUE(file, EmittanceColor);
	LOAD_VALUE(file, EmittanceMult);
	LOAD_VALUE(file, ModelSpaceNormals);
	LOAD_VALUE(file, ExternalEmittance);
	LOAD_VALUE(file, BackLighting);
	LOAD_VALUE(file, ReceiveShadows);
	LOAD_VALUE(file, HideSecret);
	LOAD_VALUE(file, CastShadows);
	LOAD_VALUE(file, DissolveFade);
	LOAD_VALUE(file, AssumeShadowmask);
	LOAD_VALUE(file, Glowmap);
	LOAD_VALUE(file, EnvironmentMappingWindow);
	LOAD_VALUE(file, EnvironmentMappingEye);
	LOAD_VALUE(file, Hair);
	LOAD_VALUE(file, HairTintColor);
	LOAD_VALUE(file, Tree);
	LOAD_VALUE(file, Facegen);
	LOAD_VALUE(file, SkinTint);
	LOAD_VALUE(file, Tessellate);
	LOAD_VALUE(file, DisplacementTextureBias);
	LOAD_VALUE(file, DisplacementTextureScale);
	LOAD_VALUE(file, TessellationPNScale);
	LOAD_VALUE(file, TessellationBaseFactor);
	LOAD_VALUE(file, TessellationFadeDistance);
	LOAD_VALUE(file, GrayscaleToPaletteScale);
	LOAD_VALUE(file, SkewSpecularAlpha);
	return IO_OK;
error: return IO_ERROR;
}

IOResult BGSMFileReference::SaveMaterialChunk(ISave* isave)
{
	IOResult res = MaterialReference::SaveMaterialChunk(file, isave);
	if (res != IO_OK) return res;

	SAVE_VALUE(file, DiffuseTexture);
	SAVE_VALUE(file, NormalTexture);
	SAVE_VALUE(file, SmoothSpecTexture);
	SAVE_VALUE(file, GreyscaleTexture);
	SAVE_VALUE(file, EnvmapTexture);
	SAVE_VALUE(file, GlowTexture);
	SAVE_VALUE(file, InnerLayerTexture);
	SAVE_VALUE(file, WrinklesTexture);
	SAVE_VALUE(file, DisplacementTexture);
	SAVE_VALUE(file, EnableEditorAlphaRef);
	SAVE_VALUE(file, RimLighting);
	SAVE_VALUE(file, RimPower);
	SAVE_VALUE(file, BackLightPower);
	SAVE_VALUE(file, SubsurfaceLighting);
	SAVE_VALUE(file, SubsurfaceLightingRolloff);
	SAVE_VALUE(file, SpecularEnabled);
	SAVE_VALUE(file, SpecularColor);
	SAVE_VALUE(file, SpecularMult);
	SAVE_VALUE(file, Smoothness);
	SAVE_VALUE(file, FresnelPower);
	SAVE_VALUE(file, WetnessControlSpecScale);
	SAVE_VALUE(file, WetnessControlSpecPowerScale);
	SAVE_VALUE(file, WetnessControlSpecMinvar);
	SAVE_VALUE(file, WetnessControlEnvMapScale);
	SAVE_VALUE(file, WetnessControlFresnelPower);
	SAVE_VALUE(file, WetnessControlMetalness);
	SAVE_VALUE(file, RootMaterialPath);
	SAVE_VALUE(file, AnisoLighting);
	SAVE_VALUE(file, EmitEnabled);
	SAVE_VALUE(file, EmittanceColor);
	SAVE_VALUE(file, EmittanceMult);
	SAVE_VALUE(file, ModelSpaceNormals);
	SAVE_VALUE(file, ExternalEmittance);
	SAVE_VALUE(file, BackLighting);
	SAVE_VALUE(file, ReceiveShadows);
	SAVE_VALUE(file, HideSecret);
	SAVE_VALUE(file, CastShadows);
	SAVE_VALUE(file, DissolveFade);
	SAVE_VALUE(file, AssumeShadowmask);
	SAVE_VALUE(file, Glowmap);
	SAVE_VALUE(file, EnvironmentMappingWindow);
	SAVE_VALUE(file, EnvironmentMappingEye);
	SAVE_VALUE(file, Hair);
	SAVE_VALUE(file, HairTintColor);
	SAVE_VALUE(file, Tree);
	SAVE_VALUE(file, Facegen);
	SAVE_VALUE(file, SkinTint);
	SAVE_VALUE(file, Tessellate);
	SAVE_VALUE(file, DisplacementTextureBias);
	SAVE_VALUE(file, DisplacementTextureScale);
	SAVE_VALUE(file, TessellationPNScale);
	SAVE_VALUE(file, TessellationBaseFactor);
	SAVE_VALUE(file, TessellationFadeDistance);
	SAVE_VALUE(file, GrayscaleToPaletteScale);
	SAVE_VALUE(file, SkewSpecularAlpha);
	return IO_OK;
error: return IO_ERROR;
}
#pragma endregion 



#pragma region ("BGEM Class Serializer")
class BGEMFileClassDesc :public ClassDesc2 {
public:
	int            IsPublic()  override { return FALSE; }
	void *         Create(BOOL loading = FALSE)  override { return new BGEMFileReference(); }
	const TCHAR *  ClassName()  override { return GetString(IDS_BGEM_FILE); }
	SClass_ID      SuperClassID()  override { return MATERIALFILE_CLASS_ID; }
	Class_ID       ClassID()  override { return BGEMFILE_CLASS_ID; }
	const TCHAR*   Category()  override { return GetString(IDS_CATEGORY); }
	const TCHAR*   InternalName()  override { return _T("MaterialFile"); }
	HINSTANCE      HInstance()  override { return hInstance; }          // returns owning module handle
};

static BGEMFileClassDesc theBGEMFileClassDesc;

extern ClassDesc2* GetBGEMFileClassDesc()
{
	return &theBGEMFileClassDesc;
}

IOResult BGEMFileReference::Load(ILoad* iload)
{
	USES_CONVERSION;
	IOResult res;
	ULONG nb;
	int version = 1;
	wchar_t *ptr = nullptr;
	TSTR name, filename;

	res = MaterialReference::Load(iload);
	if (res != IO_OK) return res;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case VERSION_CHUNKID:
			res = iload->Read(&version, sizeof(version), &nb);
			break;
		case NAME_CHUNKID:
			res = iload->ReadWStringChunk(&ptr);
			this->materialName = W2T(ptr);
			break;
		case FILENAME_CHUNKID:
			res = iload->ReadWStringChunk(&ptr);
			this->materialFileName = W2T(ptr);
			break;
		case FILE_CHUNKID:
			res = LoadMaterialChunk(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}

	return IO_OK;
}

IOResult BGEMFileReference::Save(ISave* isave)
{
	IOResult res;
	ULONG nb;
	res = MaterialReference::Save(isave);
	if (res != IO_OK) return res;

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write(&currentVersion, sizeof(int), &nb);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(NAME_CHUNKID);
	res = isave->WriteWString(materialName);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(FILENAME_CHUNKID);
	res = isave->WriteWString(materialFileName);
	isave->EndChunk();
	if (res != IO_OK) return res;

	isave->BeginChunk(FILE_CHUNKID);
	res = SaveMaterialChunk(isave);
	isave->EndChunk();
	if (res != IO_OK) return res;

	return res;
}

IOResult BGEMFileReference::LoadMaterialChunk(ILoad* iload)
{
	IOResult res = MaterialReference::LoadMaterialChunk(file, iload);
	if (res != IO_OK) return res;

	LOAD_VALUE(file, BaseTexture);
	LOAD_VALUE(file, GrayscaleTexture);
	LOAD_VALUE(file, EnvmapTexture);
	LOAD_VALUE(file, NormalTexture);
	LOAD_VALUE(file, EnvmapMaskTexture);
	LOAD_VALUE(file, BloodEnabled);
	LOAD_VALUE(file, EffectLightingEnabled);
	LOAD_VALUE(file, FalloffEnabled);
	LOAD_VALUE(file, FalloffColorEnabled);
	LOAD_VALUE(file, GrayscaleToPaletteAlpha);
	LOAD_VALUE(file, SoftEnabled);
	LOAD_VALUE(file, BaseColor);
	LOAD_VALUE(file, BaseColorScale);
	LOAD_VALUE(file, FalloffStartAngle);
	LOAD_VALUE(file, FalloffStopAngle);
	LOAD_VALUE(file, FalloffStartOpacity);
	LOAD_VALUE(file, FalloffStopOpacity);
	LOAD_VALUE(file, LightingInfluence);
	LOAD_VALUE(file, EnvmapMinLOD);
	LOAD_VALUE(file, SoftDepth);
	return IO_OK;
error: return IO_ERROR;
}

IOResult BGEMFileReference::SaveMaterialChunk(ISave* isave)
{
	IOResult res = MaterialReference::SaveMaterialChunk(file, isave);
	if (res != IO_OK) return res;

	SAVE_VALUE(file, BaseTexture);
	SAVE_VALUE(file, GrayscaleTexture);
	SAVE_VALUE(file, EnvmapTexture);
	SAVE_VALUE(file, NormalTexture);
	SAVE_VALUE(file, EnvmapMaskTexture);
	SAVE_VALUE(file, BloodEnabled);
	SAVE_VALUE(file, EffectLightingEnabled);
	SAVE_VALUE(file, FalloffEnabled);
	SAVE_VALUE(file, FalloffColorEnabled);
	SAVE_VALUE(file, GrayscaleToPaletteAlpha);
	SAVE_VALUE(file, SoftEnabled);
	SAVE_VALUE(file, BaseColor);
	SAVE_VALUE(file, BaseColorScale);
	SAVE_VALUE(file, FalloffStartAngle);
	SAVE_VALUE(file, FalloffStopAngle);
	SAVE_VALUE(file, FalloffStartOpacity);
	SAVE_VALUE(file, FalloffStopOpacity);
	SAVE_VALUE(file, LightingInfluence);
	SAVE_VALUE(file, EnvmapMinLOD);
	SAVE_VALUE(file, SoftDepth);

	return IO_OK;
error: return IO_ERROR;
}
#pragma endregion 
