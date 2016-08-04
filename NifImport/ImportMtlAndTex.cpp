/**********************************************************************
*<
FILE: ImportMtlAndTex.cpp

DESCRIPTION:	Material and Texture Import routines

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "stdafx.h"
#include "niutils.h"
#include <shaders.h>
#include <maxtypes.h>
#include "MaxNifImport.h"
#include "obj/NiWireframeProperty.h"
#include "obj/NiAlphaProperty.h"
#include "obj/NiStencilProperty.h"
#include "obj/NiShadeProperty.h"
#include "obj/NiVertexColorProperty.h"
#include "obj/NiDitherProperty.h"
#include "obj/NiSpecularProperty.h"
#include "obj/NiTextureProperty.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/BSShaderNoLightingProperty.h"
#include "obj/BSShaderPPLightingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/SkyShaderProperty.h"
#include "obj/TileShaderProperty.h"
#include "obj/TallGrassShaderProperty.h"
#include "obj/Lighting30ShaderProperty.h"
#include "obj/NiImage.h"
#include "objectParams.h"
#include <obj/BSEffectShaderProperty.h>
#include "../mtlutils/mtldefine.h"
#include "../NifProps/iNifProps.h"
using namespace Niflib;

enum {
	C_BASE, C_DARK, C_DETAIL, C_GLOSS, C_GLOW, C_BUMP, C_NORMAL,
	C_DECAL0, C_DECAL1, C_DECAL2, C_DECAL3, C_ENVMASK, C_ENV,
	C_HEIGHT, C_REFLECTION, C_OPACITY, C_SPECULAR, C_PARALLAX,
	C_BACKLIGHT,
};

#undef GNORMAL_CLASS_ID
static const Class_ID GNORMAL_CLASS_ID(0x243e22c6, 0x63f6a014);
static const Class_ID NIFSHADER_CLASS_ID(0x566e8ccb, 0xb091bd48);
static const Class_ID civ4Shader(0x670a77d0, 0x23ab5c7f);
static const Class_ID FO4SHADER_CLASS_ID(0x7a6bc2e7, 0x71106f41);

Texmap* NifImporter::CreateNormalBump(LPCTSTR name, Texmap* nmap)
{
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

Texmap* NifImporter::CreateMask(LPCTSTR name, Texmap* map, Texmap* mask)
{
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

Texmap* NifImporter::MakeAlpha(Texmap* tex)
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

Texmap* NifImporter::CreateTexture(TexDesc& desc)
{
	BitmapManager *bmpMgr = TheManager;
	if (NiSourceTextureRef texSrc = desc.source) {
		tstring filename = A2TString(texSrc->GetTextureFileName());
		if (bmpMgr->CanImport(filename.c_str())) {
			BitmapTex *bmpTex = NewDefaultBitmapTex();
			tstring name = A2TString(texSrc->GetName());
			if (name.empty()) {
				TCHAR buffer[MAX_PATH];
				_tcscpy(buffer, PathFindFileName(filename.c_str()));
				PathRemoveExtension(buffer);
				name = buffer;
			}
			bmpTex->SetName(name.c_str());
			bmpTex->SetMapName(const_cast<LPTSTR>(FindImage(filename).c_str()));
			bmpTex->SetAlphaAsMono(TRUE);
			bmpTex->SetAlphaSource(ALPHA_DEFAULT);

			switch (desc.filterMode)
			{
			case FILTER_TRILERP: bmpTex->SetFilterType(FILTER_PYR); break;
			case FILTER_BILERP:  bmpTex->SetFilterType(FILTER_SAT); break;
			case FILTER_NEAREST: bmpTex->SetFilterType(FILTER_NADA); break;
			}

			if (UVGen *uvGen = bmpTex->GetTheUVGen()) {
				if (uvGen && uvGen->IsStdUVGen()) {
					StdUVGen *uvg = (StdUVGen*)uvGen;
					uvg->SetMapChannel(desc.uvSet + 1);
				}

				switch (desc.clampMode)
				{
				case WRAP_S_WRAP_T: uvGen->SetTextureTiling(3); break;
				case WRAP_S_CLAMP_T: uvGen->SetTextureTiling(1); break;
				case CLAMP_S_WRAP_T: uvGen->SetTextureTiling(2); break;
				case CLAMP_S_CLAMP_T:uvGen->SetTextureTiling(0); break;
				}

				if (desc.hasTextureTransform) {
					if (RefTargetHandle ref = uvGen->GetReference(0)) {
						TexCoord trans = desc.translation;
						TexCoord tiling = desc.tiling;
						float wangle = TODEG(desc.wRotation);

						setMAXScriptValue(ref, TEXT("U_Offset"), 0, trans.u);
						setMAXScriptValue(ref, TEXT("V_Offset"), 0, trans.v);
						setMAXScriptValue(ref, TEXT("U_Tiling"), 0, tiling.u);
						setMAXScriptValue(ref, TEXT("V_Tiling"), 0, tiling.v);
						setMAXScriptValue(ref, TEXT("W_Angle"), 0, wangle);
					}
				}
			}
			if (showTextures) {
				bmpTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
				bmpTex->ActivateTexDisplay(TRUE);
				bmpTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}

			return bmpTex;
		}
	}
	return nullptr;
}

Texmap* NifImporter::CreateTexture(const NiTexturePropertyRef& texSrc)
{
	BitmapManager *bmpMgr = TheManager;
	if (NiImageRef imgRef = texSrc->GetImage()) {
		tstring filename = A2TString(imgRef->GetTextureFileName());
		if (bmpMgr->CanImport(filename.c_str())) {
			BitmapTex *bmpTex = NewDefaultBitmapTex();
			tstring name = A2TString(texSrc->GetName());
			if (name.empty()) {
				TCHAR buffer[MAX_PATH];
				_tcscpy(buffer, PathFindFileName(filename.c_str()));
				PathRemoveExtension(buffer);
				name = buffer;
			}
			bmpTex->SetName(name.c_str());
			bmpTex->SetMapName(const_cast<TCHAR*>(FindImage(filename).c_str()));
			bmpTex->SetAlphaAsMono(TRUE);
			bmpTex->SetAlphaSource(ALPHA_DEFAULT);

			bmpTex->SetFilterType(FILTER_PYR);

			if (showTextures) {
				bmpTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
				bmpTex->ActivateTexDisplay(TRUE);
				bmpTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}

			if (UVGen *uvGen = bmpTex->GetTheUVGen()) {
				uvGen->SetTextureTiling(0);
			}

			return bmpTex;
		}
	}
	return nullptr;
}

Texmap* NifImporter::CreateTexture(const tstring& filename, TexClampMode mode, TexCoord offset, TexCoord tiling)
{
	if (filename.empty())
		return nullptr;

	BitmapManager *bmpMgr = TheManager;
	if (bmpMgr->CanImport(filename.c_str())) {
		BitmapTex *bmpTex = NewDefaultBitmapTex();
		tstring name = filename;
		if (name.empty()) {
			TCHAR buffer[MAX_PATH];
			_tcscpy(buffer, PathFindFileName(filename.c_str()));
			PathRemoveExtension(buffer);
			name = buffer;
		}
		bmpTex->SetName(name.c_str());
		bmpTex->SetMapName(const_cast<LPTSTR>(FindImage(filename).c_str()));
		bmpTex->SetAlphaAsMono(TRUE);
		bmpTex->SetAlphaSource(ALPHA_DEFAULT);

		bmpTex->SetFilterType(FILTER_PYR);

		if (showTextures) {
			bmpTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
			bmpTex->ActivateTexDisplay(TRUE);
			bmpTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}

		if (UVGen *uvGen = bmpTex->GetTheUVGen()) {
			switch (mode)
			{
			case WRAP_S_WRAP_T: uvGen->SetTextureTiling(3); break;
			case WRAP_S_CLAMP_T: uvGen->SetTextureTiling(1); break;
			case CLAMP_S_WRAP_T: uvGen->SetTextureTiling(2); break;
			case CLAMP_S_CLAMP_T:uvGen->SetTextureTiling(0); break;
			}
			if (RefTargetHandle ref = uvGen->GetReference(0)) {
				setMAXScriptValue(ref, TEXT("U_Offset"), 0, offset.u);
				setMAXScriptValue(ref, TEXT("V_Offset"), 0, offset.v);
				setMAXScriptValue(ref, TEXT("U_Tiling"), 0, tiling.u);
				setMAXScriptValue(ref, TEXT("V_Tiling"), 0, tiling.v);
			}
		}

		return bmpTex;
	}
	return nullptr;
}

StdMat2 *NifImporter::ImportMaterialAndTextures(ImpNode *node, NiAVObjectRef avObject)
{
	USES_CONVERSION;
	// Texture
	vector<NiPropertyRef> props = avObject->GetProperties();
	NiMaterialPropertyRef matRef = SelectFirstObjectOfType<NiMaterialProperty>(props);
	BSShaderPropertyRef shaderRef = SelectFirstObjectOfType<BSShaderProperty>(props);
	BSEffectShaderPropertyRef effectShaderRef = SelectFirstObjectOfType<BSEffectShaderProperty>(props);
	BSLightingShaderPropertyRef lightingShaderRef = SelectFirstObjectOfType<BSLightingShaderProperty>(props);
	
	//NiGeometryRef geoprop = DynamicCast<NiGeometry>(avObject);
	//BSEffectShaderPropertyRef effectShaderRef = geoprop->GetBSPropertyOfType<Niflib::BSEffectShaderProperty>();
	//BSLightingShaderPropertyRef lightingShaderRef = geoprop->GetBSPropertyOfType<Niflib::BSLightingShaderProperty>();

	if (matRef != nullptr || shaderRef != nullptr || effectShaderRef != nullptr || lightingShaderRef != nullptr) {

		StdMat2 *m = NewDefaultStdMat();
		RefTargetHandle ref = m->GetReference(2/*shader*/);

		if (matRef != nullptr) {
			m->SetName(A2T(matRef->GetName().c_str()));
		}
		else {
			m->SetName(A2T(avObject->GetName().c_str()));
		}
		if (showTextures) {
			m->SetMtlFlag(MTL_DISPLAY_ENABLE_FLAGS, TRUE);
		}

		// try the civ4 shader first then default back to normal shaders
		if (IsFallout4() && ImportFO4Shader(node, avObject, m) ) {
			return m;
		}
		else if (ImportNiftoolsShader(node, avObject, m)) {
			return m;
		}

		TexClampMode mode = WRAP_S_WRAP_T;
		TexCoord offset = TexCoord(0.0f, 0.0f);
		TexCoord tiling = TexCoord(1.0f, 1.0f);

		list<NiTimeControllerRef> controllers;
		NiTexturingPropertyRef texRef = avObject->GetPropertyByType(NiTexturingProperty::TYPE);
		NiWireframePropertyRef wireRef = avObject->GetPropertyByType(NiWireframeProperty::TYPE);
		NiAlphaPropertyRef alphaRef = avObject->GetPropertyByType(NiAlphaProperty::TYPE);
		NiStencilPropertyRef stencilRef = avObject->GetPropertyByType(NiStencilProperty::TYPE);
		NiShadePropertyRef shadeRef = avObject->GetPropertyByType(NiShadeProperty::TYPE);

		if (IsFallout3() || IsSkyrim() || IsFallout4()) {
			m->SetAmbient(Color(0.588f, 0.588f, 0.588f), 0);
			m->SetDiffuse(Color(0.588f, 0.588f, 0.588f), 0);
			m->SetSpecular(Color(0.902f, 0.902f, 0.902f), 0);
		}
		else if (matRef != nullptr) {
			m->SetAmbient(TOCOLOR(matRef->GetAmbientColor()), 0);
			m->SetDiffuse(TOCOLOR(matRef->GetDiffuseColor()), 0);
			m->SetSpecular(TOCOLOR(matRef->GetSpecularColor()), 0);
		}
		if (matRef != nullptr) {
			Color c = TOCOLOR(matRef->GetEmissiveColor());
			if (c.r != 0 || c.b != 0 || c.g != 0) {
				m->SetSelfIllumColorOn(TRUE);
				m->SetSelfIllumColor(c, 0);
			}
			m->SetShinStr(0.0, 0);
			//m->SetShininess(matRef->GetGlossiness() / 100.0, 0);
			m->SetOpacity(matRef->GetTransparency(), 0);
		}

		bool hasShaderAttributes = (wireRef != nullptr) || (stencilRef != nullptr) || (shadeRef != nullptr);
		if (m->SupportsShaders() && hasShaderAttributes) {
			if (Shader *s = m->GetShader()) {
				if (wireRef != nullptr && (wireRef->GetFlags() & 1)) {
					BOOL value = TRUE;
					m->SetWire(value);
				}
				if (stencilRef != nullptr) {
					if (stencilRef->GetFaceDrawMode() == DRAW_BOTH) {
						BOOL value = TRUE;
						m->SetTwoSided(value);
					}
				}
				if (shadeRef != nullptr && shadeRef->GetFlags() & 1) {
					m->SetFaceted(TRUE);
				}
			}
		}

		if (nullptr != texRef)
		{
			// Handle Base/Detail ???
			if (texRef->HasTexture(DECAL_0_MAP)) {
				if (Texmap* tex = CreateTexture(texRef->GetTexture(DECAL_0_MAP)))
					m->SetSubTexmap(ID_DI, tex);
				if (texRef->HasTexture(BASE_MAP)) {
					m->LockAmbDiffTex(FALSE);
					if (Texmap* tex = CreateTexture(texRef->GetTexture(BASE_MAP)))
						m->SetSubTexmap(ID_AM, tex);
				}
			}
			else if (texRef->HasTexture(BASE_MAP)) {
				if (Texmap* tex = CreateTexture(texRef->GetTexture(BASE_MAP))) {
					m->SetSubTexmap(ID_DI, tex);
					if (showTextures) gi->ActivateTexture(tex, m);
				}
			}
			// Handle Bump map
			if (texRef->HasTexture(BUMP_MAP)) {
				if (Texmap* tex = CreateTexture(texRef->GetTexture(BUMP_MAP)))
					m->SetSubTexmap(ID_BU, CreateNormalBump(nullptr, tex));
			}
			// Shiny map
			if (texRef->HasTexture(GLOSS_MAP)) {
				if (Texmap* tex = CreateTexture(texRef->GetTexture(GLOSS_MAP)))
					m->SetSubTexmap(ID_SS, tex);
			}
			// Self illumination
			if (texRef->HasTexture(GLOW_MAP)) {
				if (Texmap* tex = CreateTexture(texRef->GetTexture(GLOW_MAP)))
					m->SetSubTexmap(ID_SI, tex);
			}

			// Custom Shader Handling
			int nTex = texRef->GetShaderTextureCount();
			if (nTex > 0) {
				list<NiExtraDataRef> data = avObject->GetExtraData();
				NiGeometryRef trigeom = DynamicCast<NiGeometry>(avObject);
				if (trigeom->HasShader()) {
					for (list<NiExtraDataRef>::iterator itr = data.begin(); itr != data.end(); ++itr) {
						if (NiIntegerExtraDataRef idx = DynamicCast<NiIntegerExtraData>(*itr)) {
							string name = idx->GetName();
							if (wildmatch("*Index", name)) {
								int shader = idx->GetData();
								if (shader < nTex) {
									if (name == "NormalMapIndex") {
										if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
											m->SetSubTexmap(ID_BU, CreateNormalBump(nullptr, tex));
									}
									else if (name == "SpecularIntensity") {
										if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
											m->SetSubTexmap(ID_SP, CreateNormalBump(nullptr, tex));
									}
								}
							}
						}
					}
				}
			}
		}
		if (NiTexturePropertyRef tex2Ref = avObject->GetPropertyByType(NiTextureProperty::TYPE)) {
			// Handle Base/Detail ???
			if (Texmap* tex = CreateTexture(tex2Ref)) {
				m->SetSubTexmap(ID_DI, tex);
			}
		}
		if (BSShaderNoLightingPropertyRef noLightShadeRef = SelectFirstObjectOfType<BSShaderNoLightingProperty>(props)) {
			if (Texmap* tex = CreateTexture(A2TString(noLightShadeRef->GetFileName()))) {
				m->SetSubTexmap(ID_DI, tex);
			}
		}
		BSShaderTextureSetRef textureSet = nullptr;

		bool VertexColorsEnable = false;
		if (BSShaderPPLightingPropertyRef ppLightShadeRef = SelectFirstObjectOfType<BSShaderPPLightingProperty>(props)) {
			textureSet = ppLightShadeRef->GetTextureSet();
		}
		if (SkyShaderPropertyRef skyShadeRef = SelectFirstObjectOfType<SkyShaderProperty>(props)) {
			if (Texmap* tex = CreateTexture(A2TString(skyShadeRef->GetFileName()))) {
				m->SetSubTexmap(ID_DI, tex);
			}
		}
		if (TileShaderPropertyRef tileShadeRef = SelectFirstObjectOfType<TileShaderProperty>(props)) {
			if (Texmap* tex = CreateTexture(A2TString(tileShadeRef->GetFileName()))) {
				m->SetSubTexmap(ID_DI, tex);
			}
		}
		if (TallGrassShaderPropertyRef grassShadeRef = SelectFirstObjectOfType<TallGrassShaderProperty>(props)) {
			if (Texmap* tex = CreateTexture(A2TString(grassShadeRef->GetFileName()))) {
				m->SetSubTexmap(ID_DI, tex);
			}
		}
		if (Lighting30ShaderPropertyRef lighting30ShadeRef = SelectFirstObjectOfType<Lighting30ShaderProperty>(props)) {
			textureSet = lighting30ShadeRef->GetTextureSet();
		}
		if (lightingShaderRef != nullptr) {
			textureSet = lightingShaderRef->GetTextureSet();
			if (enableAnimations)
				controllers = lightingShaderRef->GetControllers();
			VertexColorsEnable = (lightingShaderRef->GetShaderFlags2() & SLSF2_VERTEX_COLORS) != 0;

			mode = lightingShaderRef->GetTextureClampMode();
			offset = lightingShaderRef->GetUvOffset();
			tiling = lightingShaderRef->GetUvScale();

			m->SetOpacity(lightingShaderRef->GetAlpha(), INFINITE);
		}
		if (effectShaderRef != nullptr) {
			VertexColorsEnable = (effectShaderRef->GetShaderFlags2() & SLSF2_VERTEX_COLORS) != 0;

			mode = (TexClampMode)effectShaderRef->GetTextureClampMode();
			offset = effectShaderRef->GetUvOffset();
			tiling = effectShaderRef->GetUvScale();

			if (enableAnimations)
				controllers = effectShaderRef->GetControllers();
		}
		if (textureSet != nullptr)
		{
			if (Texmap* tex = CreateTexture(A2TString(textureSet->GetTexture(0)), mode, offset, tiling))
				m->SetSubTexmap(ID_DI, tex);
			if (Texmap* tex = CreateTexture(A2TString(textureSet->GetTexture(1)), mode, offset, tiling))
				m->SetSubTexmap(ID_BU, CreateNormalBump(nullptr, tex));
			if (Texmap* tex = CreateTexture(A2TString(textureSet->GetTexture(2)), mode, offset, tiling))
				m->SetSubTexmap(ID_SI, tex);
			if (Texmap* tex = CreateTexture(A2TString(textureSet->GetTexture(4)), mode, offset, tiling)) {
				if (Texmap* mask = CreateTexture(A2TString(textureSet->GetTexture(5)), mode, offset, tiling))
					tex = CreateMask(nullptr, tex, mask);
				m->SetSubTexmap(ID_RL, tex);
				m->SetTexmapAmt(ID_RL, 0, INFINITE);
				tex->SetOutputLevel(INFINITE, 0);
			}
			if (alphaRef) {
				// add opacity channel
				if (Texmap* tex = CreateTexture(A2TString(textureSet->GetTexture(0)), mode, offset, tiling)) {
					m->SetSubTexmap(ID_OP, MakeAlpha(tex));
					m->SetTexmapAmt(ID_OP, 0.5f, INFINITE);
				}
			}
		}
		if (effectShaderRef != nullptr)
		{
			if (ref != nullptr)
			{
				Color emittance = TOCOLOR(effectShaderRef->GetEmissiveColor());
				setMAXScriptValue(ref, TEXT("emittance"), 0, emittance);
				bool specularEnable = (effectShaderRef->GetShaderFlags2() & SLSF1_SPECULAR) != 0;
				setMAXScriptValue(ref, TEXT("SpecularEnable"), 0, specularEnable);
			}
			// Create the source texture
			if (Texmap* tex = CreateTexture(A2TString(effectShaderRef->GetSourceTexture()), mode, offset, tiling))
				m->SetSubTexmap(ID_DI, tex);
			if (Texmap* tex = CreateTexture(A2TString(effectShaderRef->GetGreyscaleTexture()), mode, offset, tiling))
				m->SetSubTexmap(ID_SI, tex);
		}
		if (enableAnimations && !controllers.empty())
		{
			// Import any texture animations
			if (ImportMtlAndTexAnimation(controllers, m))
			{
				Interval range;
				if (GetControllerTimeRange(controllers, range))
				{
					if (range.Empty() || range.Start() != range.End()) {
						gi->SetAnimRange(range);
					}
				}
			}
		}
		if (ref != nullptr)
		{
			setMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable);
			setMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable);
		}

		return m;
	}
	return nullptr;
}


bool NifImporter::ImportMaterialAndTextures(ImpNode *node, vector<NiTriBasedGeomRef>& glist)
{
	MultiMtl *mtl = nullptr;
	int isubmtl = 0;
	for (vector<NiTriBasedGeomRef>::iterator itr = glist.begin(), end = glist.end(); itr != end; ++itr, ++isubmtl) {
		NiTriBasedGeomDataRef triGeomData = StaticCast<NiTriBasedGeomData>((*itr)->GetData());
		if (StdMat2* submtl = ImportMaterialAndTextures(node, (*itr)))
		{
			if (mtl == nullptr)
				mtl = NewDefaultMultiMtl();
			// SubMatIDs do not have to be contigious so we just use the offset
			mtl->SetSubMtlAndName(isubmtl, submtl, submtl->GetName());
		}
	}
	if (mtl != nullptr)
	{
		gi->GetMaterialLibrary().Add(mtl);
		node->GetINode()->SetMtl(mtl);
		return true;
	}
	return false;
}

bool NifImporter::ImportNiftoolsShader(ImpNode *node, NiAVObjectRef avObject, StdMat2 *mtl)
{
	if (!useNiftoolsShader || !mtl || !mtl->SupportsShaders())
		return false;

	if (!(useNiftoolsShader == 1 && mtl->SwitchShader(NIFSHADER_CLASS_ID)))
		return false;

	//if (useNiftoolsShader != 1 || !mtl->SwitchShader(NIFSHADER_CLASS_ID)) {
	//	if ((useNiftoolsShader & 2) == 0 || !mtl->SwitchShader(civ4Shader))
	//		return false;
	//}

	TSTR shaderByName;
	if (Shader *s = mtl->GetShader())
		s->GetClassName(shaderByName);

	Class_ID shaderID = mtl->ClassID();

	TexClampMode mode = WRAP_S_WRAP_T;
	TexCoord offset = TexCoord(0.0f, 0.0f);
	TexCoord tiling = TexCoord(1.0f, 1.0f);

	RefTargetHandle ref = mtl->GetReference(2/*shader*/);
	if (!ref)
		return false;

	NiGeometryRef geom = DynamicCast<NiGeometry>(avObject);
	vector<NiPropertyRef> props = avObject->GetProperties();

	if (NiMaterialPropertyRef matRef = SelectFirstObjectOfType<NiMaterialProperty>(props)) {
		Color ambient = TOCOLOR(matRef->GetAmbientColor());
		Color diffuse = TOCOLOR(matRef->GetDiffuseColor());
		Color specular = TOCOLOR(matRef->GetSpecularColor());
		Color emittance = TOCOLOR(matRef->GetEmissiveColor());
		float shininess = matRef->GetGlossiness();
		float alpha = matRef->GetTransparency();

		if (IsFallout3() || IsSkyrim() || IsFallout4()) {
			ambient = diffuse = Color(0.588f, 0.588f, 0.588f);
			//specular = Color(0.902f, 0.902f, 0.902f);
		}

		mtl->SetShinStr(0.0, 0);
		mtl->SetShininess((float)(shininess / 100.0), 0);
		mtl->SetOpacity(alpha*100.0f, 0);

		setMAXScriptValue(ref, TEXT("ambient"), 0, ambient);
		setMAXScriptValue(ref, TEXT("diffuse"), 0, diffuse);
		setMAXScriptValue(ref, TEXT("specular"), 0, specular);
		setMAXScriptValue(ref, TEXT("emittance"), 0, emittance);
		setMAXScriptValue(ref, TEXT("shininess"), 0, shininess);
		setMAXScriptValue(ref, TEXT("alpha"), 0, alpha);
	}
	if (NiShadePropertyRef shadeRef = SelectFirstObjectOfType<NiShadeProperty>(props)) {
		if (shadeRef->GetFlags() & 1) {
			mtl->SetFaceted(TRUE);
		}
	}
	if (NiWireframePropertyRef wireRef = SelectFirstObjectOfType<NiWireframeProperty>(props)) {
		if (wireRef->GetFlags() & 1) {
			mtl->SetWire(TRUE);
		}
	}
	if (NiStencilPropertyRef stencilRef = SelectFirstObjectOfType<NiStencilProperty>(props)) {
		mtl->SetTwoSided(TRUE);
	}
	bool Dither = false;
	bool SpecularEnable = false;
	if (NiDitherPropertyRef ditherRef = SelectFirstObjectOfType<NiDitherProperty>(props)) {
		Dither = (ditherRef->GetFlags() & 1) ? true : false;
	}
	if (NiSpecularPropertyRef specRef = SelectFirstObjectOfType<NiSpecularProperty>(props)) {
		SpecularEnable = (specRef->GetFlags() & 1) ? true : false;
	}
	setMAXScriptValue(ref, TEXT("Dither"), 0, Dither);
	setMAXScriptValue(ref, TEXT("SpecularEnable"), 0, SpecularEnable);

	if (NiVertexColorPropertyRef vertexColor = SelectFirstObjectOfType<NiVertexColorProperty>(props)) {
		int SrcVertexMode = vertexColor->GetVertexMode();
		int LightingMode = vertexColor->GetLightingMode();
		bool VertexColorsEnable = true;
		setMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable);
		setMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable);
		setMAXScriptValue(ref, TEXT("SourceVertexMode"), 0, SrcVertexMode);
		setMAXScriptValue(ref, TEXT("SrcVertexMode"), 0, SrcVertexMode);
		setMAXScriptValue(ref, TEXT("LightingMode"), 0, LightingMode);
	}
	else {
		bool VertexColorsEnable = false;
		setMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable);
		setMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable);
	}
	NiAlphaPropertyRef alphaRef = SelectFirstObjectOfType<NiAlphaProperty>(props);
	if (!alphaRef && geom) alphaRef = geom->GetBSPropertyOfType<NiAlphaProperty>();
	if (alphaRef) {
		int TestRef = alphaRef->GetTestThreshold();
		int srcBlend = alphaRef->GetSourceBlendFunc();
		int destBlend = alphaRef->GetDestBlendFunc();
		int TestMode = alphaRef->GetTestFunc();
		bool AlphaTestEnable = alphaRef->GetTestState();
		bool NoSorter = alphaRef->GetTriangleSortMode();
		bool alphaBlend = alphaRef->GetBlendState();
		int alphaMode = 1;

		if (!alphaBlend) {
			alphaMode = 1; // none
		}
		else if (srcBlend == NiAlphaProperty::BF_SRC_ALPHA && destBlend == NiAlphaProperty::BF_ONE_MINUS_SRC_ALPHA) {
			alphaMode = 0; // standard or automatic?
		}
		else if (srcBlend == NiAlphaProperty::BF_ONE && destBlend == NiAlphaProperty::BF_ONE) {
			alphaMode = 3;
		}
		else if (srcBlend == NiAlphaProperty::BF_ZERO && destBlend == NiAlphaProperty::BF_SRC_COLOR) {
			alphaMode = 4;
		}
		else {
			alphaMode = 5;
		}
		setMAXScriptValue(ref, TEXT("AlphaTestEnable"), 0, AlphaTestEnable);
		setMAXScriptValue(ref, TEXT("alphaMode"), 0, alphaMode);
		setMAXScriptValue(ref, TEXT("srcBlend"), 0, srcBlend);
		setMAXScriptValue(ref, TEXT("destBlend"), 0, destBlend);
		setMAXScriptValue(ref, TEXT("NoSorter"), 0, NoSorter);
		setMAXScriptValue(ref, TEXT("TestRef"), 0, TestRef);
		setMAXScriptValue(ref, TEXT("TestMode"), 0, TestMode);
	}
	if (NiTexturingPropertyRef texRef = SelectFirstObjectOfType<NiTexturingProperty>(props)) {
		Matrix22 m2 = texRef->GetBumpMapMatrix();
		float Magnitude = (m2[0][0] + m2[1][1]) / 2.0f;
		float LumaScale = texRef->GetLumaScale();
		float LumaOffset = texRef->GetLumaOffset();
		int ApplyMode = texRef->GetApplyMode();

		setMAXScriptValue(ref, TEXT("Bump_Map_Magnitude"), 0, Magnitude);
		setMAXScriptValue(ref, TEXT("Bump_Map_Luma_Scale"), 0, LumaScale);
		setMAXScriptValue(ref, TEXT("Bump_Map_Luma_offset"), 0, LumaOffset);
		setMAXScriptValue(ref, TEXT("ApplyMode"), 0, ApplyMode);

		int ntex = mtl->NumSubTexmaps();
		if (ntex > 0)
		{
			for (int i = 0; i < ntex; ++i) {
				TexType textype = (TexType)i;
				if (nifVersion <= 0x14010003) {
					if (textype > C_NORMAL)
						textype = (TexType)(i + 2);
				}
				if (texRef->HasTexture(textype)) {
					if (Texmap* tex = CreateTexture(texRef->GetTexture(textype))) {
						mtl->SetSubTexmap(i, tex);
					}
				}
			}
		}
		// Custom Shader Handling
		int nTex = texRef->GetShaderTextureCount();
		if (nTex > 0) {
			NiGeometryRef trigeom = DynamicCast<NiGeometry>(avObject);
			if (trigeom->HasShader()) {
				list<NiExtraDataRef> data = avObject->GetExtraData();
				for (list<NiExtraDataRef>::iterator itr = data.begin(); itr != data.end(); ++itr) {
					if (NiIntegerExtraDataRef idx = DynamicCast<NiIntegerExtraData>(*itr)) {
						string name = idx->GetName();
						if (wildmatch("*Index", name)) {
							int shader = idx->GetData();
							if (shader < nTex) {
								if (name == "NormalMapIndex") {
									if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
										mtl->SetSubTexmap(C_BUMP, CreateNormalBump(nullptr, tex));
								}
								else if (name == "SpecularIntensityIndex") {
									if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
										mtl->SetSubTexmap(C_GLOSS, tex);
								}
								else if (name == "EnvironmentMapIndex") {
									if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
										mtl->SetSubTexmap(C_ENVMASK, tex);
								}
								else if (name == "EnvironmentIntensityIndex") {
									if (Texmap* tex = CreateTexture(texRef->GetShaderTexture(shader)))
										mtl->SetSubTexmap(C_ENV, tex);
								}
							}
						}
					}
				}
			}
		}
	}
	if (NiTexturePropertyRef tex2Ref = avObject->GetPropertyByType(NiTextureProperty::TYPE)) {
		// Handle Base/Detail ???
		if (Texmap* tex = CreateTexture(tex2Ref)) {
			mtl->SetSubTexmap(ID_DI, tex);
		}
	}
	//if (BSShaderPropertyRef shaderRef = SelectFirstObjectOfType<BSShaderProperty>(props)) {
	//	float envmapscale = shaderRef->GetEnvmapScale();
	//	setMAXScriptValue(ref, TEXT("EnvMapScale"), 0, envmapscale);		
	//}
	if (BSShaderNoLightingPropertyRef noLightShadeRef = SelectFirstObjectOfType<BSShaderNoLightingProperty>(props)) {
		if (Texmap* tex = CreateTexture(A2TString(noLightShadeRef->GetFileName()))) {
			mtl->SetSubTexmap(ID_DI, tex);
		}
		TSTR tname = A2TString(noLightShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	if (BSShaderPPLightingPropertyRef ppLightShadeRef = SelectFirstObjectOfType<BSShaderPPLightingProperty>(props)) {
		if (BSShaderTextureSetRef textures = ppLightShadeRef->GetTextureSet()) {
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(0))))
				mtl->SetSubTexmap(C_BASE, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(1))))
				mtl->SetSubTexmap(C_BUMP, CreateNormalBump(nullptr, tex));
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(2))))
				mtl->SetSubTexmap(C_ENVMASK, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(3))))
				mtl->SetSubTexmap(C_GLOW, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(4))))
				mtl->SetSubTexmap(C_HEIGHT, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(5))))
				mtl->SetSubTexmap(C_ENV, tex);
		}
		TSTR tname = A2TString(ppLightShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	if (SkyShaderPropertyRef skyShadeRef = SelectFirstObjectOfType<SkyShaderProperty>(props)) {
		if (Texmap* tex = CreateTexture(A2TString(skyShadeRef->GetFileName()))) {
			mtl->SetSubTexmap(ID_DI, tex);
		}
		TSTR tname = A2TString(skyShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	if (TileShaderPropertyRef tileShadeRef = SelectFirstObjectOfType<TileShaderProperty>(props)) {
		if (Texmap* tex = CreateTexture(A2TString(tileShadeRef->GetFileName()))) {
			mtl->SetSubTexmap(ID_DI, tex);
		}
		TSTR tname = A2TString(tileShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	if (TallGrassShaderPropertyRef grassShadeRef = SelectFirstObjectOfType<TallGrassShaderProperty>(props)) {
		if (Texmap* tex = CreateTexture(A2TString(grassShadeRef->GetFileName()))) {
			mtl->SetSubTexmap(ID_DI, tex);
		}
		TSTR tname = A2TString(grassShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	if (Lighting30ShaderPropertyRef lighting30ShadeRef = SelectFirstObjectOfType<Lighting30ShaderProperty>(props)) {
		if (BSShaderTextureSetRef textures = lighting30ShadeRef->GetTextureSet()) {
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(0))))
				mtl->SetSubTexmap(C_BASE, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(1))))
				mtl->SetSubTexmap(C_BUMP, CreateNormalBump(nullptr, tex));
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(2))))
				mtl->SetSubTexmap(C_ENVMASK, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(3))))
				mtl->SetSubTexmap(C_GLOW, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(4))))
				mtl->SetSubTexmap(C_HEIGHT, tex);
			if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(5))))
				mtl->SetSubTexmap(C_ENV, tex);
		}
		TSTR tname = A2TString(lighting30ShadeRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}

	if (BSLightingShaderPropertyRef lightingShaderRef = SelectFirstObjectOfType<BSLightingShaderProperty>(props))
	{
		mode = lightingShaderRef->GetTextureClampMode();
		offset = lightingShaderRef->GetUvOffset();
		tiling = lightingShaderRef->GetUvScale();

		// Material like properties
		{
			//Color ambient = TOCOLOR(lightingShaderRef->GetAmbientColor());
			//Color diffuse = TOCOLOR(lightingShaderRef->GetDiffuseColor());
			Color specular = TOCOLOR(lightingShaderRef->GetSpecularColor());
			Color emittance = TOCOLOR(lightingShaderRef->GetEmissiveColor());
			float shininess = lightingShaderRef->GetSpecularPower_Glossiness();
			float alpha = lightingShaderRef->GetAlpha();
			float refractionStr = lightingShaderRef->GetRefractionStrength();
			float lighteff1 = lightingShaderRef->GetLightingEffect1();
			float lighteff2 = lightingShaderRef->GetLightingEffect2();
			float specularStr = lightingShaderRef->GetSpecularStrength();

			float envmapscale = lightingShaderRef->GetEnvironmentMapScale();
			Color skinTintColor = TOCOLOR(lightingShaderRef->GetSkinTintColor());
			Color hairTintColor = TOCOLOR(lightingShaderRef->GetHairTintColor());
			float maxPasses = lightingShaderRef->GetMaxPasses();
			float parallaxScale = lightingShaderRef->GetScale();
			float parallaxInnerThickness = lightingShaderRef->GetParallaxInnerLayerThickness();
			float parallaxRefractionScale = lightingShaderRef->GetParallaxRefractionScale();
			TexCoord tx = lightingShaderRef->GetParallaxInnerLayerTextureScale();
			Point2 parallaxInnerTextureScale(tx.u, tx.v);			
			float parallaxEnvmapStr = lightingShaderRef->GetEnvironmentMapScale();
			float eyeCubemapScale = lightingShaderRef->GetEyeCubemapScale();
			Point3 leftEyeReflCenter = TOPOINT3(lightingShaderRef->GetLeftEyeReflectionCenter());
			Point3 rightEyeReflCenter = TOPOINT3(lightingShaderRef->GetRightEyeReflectionCenter());

			Color ambient = Color(0.588f, 0.588f, 0.588f);
			Color diffuse = Color(0.588f, 0.588f, 0.588f);

			bool VertexColorsEnable = (lightingShaderRef->GetShaderFlags2() & SLSF2_VERTEX_COLORS) != 0;
			setMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable);
			setMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable);

			bool specularEnable = (lightingShaderRef->GetShaderFlags2() & SLSF1_SPECULAR) != 0;
			setMAXScriptValue(ref, TEXT("SpecularEnable"), 0, specularEnable);

			mtl->SetShinStr(0.0, 0);
			mtl->SetShininess((float)(shininess / 100.0), 0);
			mtl->SetOpacity(alpha*100.0f, 0);

			setMAXScriptValue(ref, TEXT("ambient"), 0, ambient);
			setMAXScriptValue(ref, TEXT("diffuse"), 0, diffuse);
			setMAXScriptValue(ref, TEXT("specular"), 0, specular);
			setMAXScriptValue(ref, TEXT("emittance"), 0, emittance);
			setMAXScriptValue(ref, TEXT("shininess"), 0, shininess);
			setMAXScriptValue(ref, TEXT("alpha"), 0, alpha);

			setMAXScriptValue(ref, TEXT("specularLevel"), 0, specularStr);
			setMAXScriptValue(ref, TEXT("RefractionStrength"), 0, refractionStr);
			setMAXScriptValue(ref, TEXT("LightingEffect1"), 0, lighteff1);
			setMAXScriptValue(ref, TEXT("LightingEffect2"), 0, lighteff2);

			setMAXScriptValue(ref, TEXT("EnvMapScale"), 0, envmapscale);
			setMAXScriptValue(ref, TEXT("SkinTintColor"), 0, skinTintColor);
			setMAXScriptValue(ref, TEXT("HairTintColor"), 0, hairTintColor);
			setMAXScriptValue(ref, TEXT("MaxPasses"), 0, maxPasses);
			setMAXScriptValue(ref, TEXT("ParallaxScale"), 0, parallaxScale);
			setMAXScriptValue(ref, TEXT("ParallaxInnerThickness"), 0, parallaxInnerThickness);
			setMAXScriptValue(ref, TEXT("ParallaxRefractionScale"), 0, parallaxRefractionScale);
			//setMAXScriptValue(ref, TEXT("ParallaxInnerTextureScale"), 0, parallaxInnerTextureScale);
			setMAXScriptValue(ref, TEXT("ParallaxEnvmapStr"), 0, parallaxEnvmapStr);
			setMAXScriptValue(ref, TEXT("EyeCubemapScale"), 0, eyeCubemapScale);
			//setMAXScriptValue(ref, TEXT("LeftEyeReflCenter"), 0, leftEyeReflCenter);
			//setMAXScriptValue(ref, TEXT("RightEyeReflCenter"), 0, rightEyeReflCenter);
		}
		// Texture Set
		{
			if (BSShaderTextureSetRef textures = lightingShaderRef->GetTextureSet()) {
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(0)), mode, offset, tiling))
					mtl->SetSubTexmap(C_BASE, tex);
				if (alphaRef) { // add opacity channel					
					if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(0)), mode, offset, tiling)) {
						mtl->SetSubTexmap(C_OPACITY, MakeAlpha(tex));
						mtl->SetTexmapAmt(C_OPACITY, 0.5f, INFINITE);
					}
				}
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(1)), mode, offset, tiling))
					mtl->SetSubTexmap(C_BUMP, CreateNormalBump(nullptr, tex));
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(2)), mode, offset, tiling)) // Glow/Skin/Hair
					mtl->SetSubTexmap(C_GLOW, tex);
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(3)), mode, offset, tiling)) // Height/Parallax
					mtl->SetSubTexmap(C_HEIGHT, tex);
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(4)), mode, offset, tiling)) // Environment
					mtl->SetSubTexmap(C_ENV, tex);
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(5)), mode, offset, tiling)) // Environment Mask
					mtl->SetSubTexmap(C_ENVMASK, tex);
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(7)), mode, offset, tiling)) // Back Light
					mtl->SetSubTexmap(C_BACKLIGHT, tex);
				if (Texmap* tex = CreateTexture(A2TString(textures->GetTexture(8)), mode, offset, tiling)) //Parallax
					mtl->SetSubTexmap(C_PARALLAX, tex);
			}
		}
		if (enableAnimations)
		{
			// Import any texture animations
			if (ImportMtlAndTexAnimation(lightingShaderRef->GetControllers(), mtl))
			{
				Interval range;
				if (GetControllerTimeRange(lightingShaderRef->GetControllers(), range))
				{
					if (range.Empty() || range.Start() != range.End()) {
						gi->SetAnimRange(range);
					}
				}
			}
		}

		if (NiStencilPropertyRef stencilRef = SelectFirstObjectOfType<NiStencilProperty>(props)) {
			// TODO: handle stencil 
		}
		int flags2 = lightingShaderRef->GetShaderFlags2();
		mtl->SetTwoSided((flags2 & SLSF2_DOUBLE_SIDED) ? TRUE : FALSE);

		extern const EnumLookupType BSShaderTypes[];
		TSTR shaderType = EnumToString(lightingShaderRef->GetSkyrimShaderType() + 100, BSShaderTypes);
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, shaderType);
	}

	BSEffectShaderPropertyRef effectShaderRef = SelectFirstObjectOfType<BSEffectShaderProperty>(props);
	if (!effectShaderRef && geom) effectShaderRef = geom->GetBSPropertyOfType<Niflib::BSEffectShaderProperty>();
	if (effectShaderRef)
	{
		mode = (TexClampMode)effectShaderRef->GetTextureClampMode();
		offset = effectShaderRef->GetUvOffset();
		tiling = effectShaderRef->GetUvScale();
		{
			Color emittance = TOCOLOR(effectShaderRef->GetEmissiveColor());
			setMAXScriptValue(ref, TEXT("emittance"), 0, emittance);

			bool VertexColorsEnable = (effectShaderRef->GetShaderFlags2() & SLSF2_VERTEX_COLORS) != 0;
			setMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable);
			setMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable);

			bool specularEnable = (effectShaderRef->GetShaderFlags2() & SLSF1_SPECULAR) != 0;
			setMAXScriptValue(ref, TEXT("SpecularEnable"), 0, specularEnable);
		}
		// Create the source texture
		if (Texmap* tex = CreateTexture(A2TString(effectShaderRef->GetSourceTexture()), mode, offset, tiling))
			mtl->SetSubTexmap(C_BASE, tex);
		if (Texmap* tex = CreateTexture(A2TString(effectShaderRef->GetGreyscaleTexture()), mode, offset, tiling))
			mtl->SetSubTexmap(C_GLOW, tex);

		int flags2 = effectShaderRef->GetShaderFlags2();
		mtl->SetTwoSided((flags2 & SLSF2_DOUBLE_SIDED) ? TRUE : FALSE);

		if (enableAnimations)
		{
			// Import any texture animations
			if (ImportMtlAndTexAnimation(effectShaderRef->GetControllers(), mtl))
			{
				Interval range;
				if (GetControllerTimeRange(effectShaderRef->GetControllers(), range))
				{
					if (range.Empty() || range.Start() != range.End()) {
						gi->SetAnimRange(range);
					}
					// get start/stop times
					//mtl->SetOpacFalloff(effectShaderRef->GetFalloffStartOpacity(), range.Start());
					//mtl->SetOpacFalloff(effectShaderRef->GetFalloffStopOpacity(), range.End());

					//m->SetOpacFalloff(effectShaderRef->GetFalloffStartAngle(), range.Start());
					//m->SetOpacFalloff(effectShaderRef->GetFalloffStopAngle(), range.End());
				}
			}
		}

		TSTR tname = A2TString(effectShaderRef->GetType().GetTypeName()).c_str();
		setMAXScriptValue(ref, TEXT("CustomShader"), 0, tname);
	}
	return true;
}

bool ReadMaterialFromNIF(BaseMaterial& mat, const vector<NiPropertyRef>& props)
{
	if (BSLightingShaderPropertyRef shaderRef = SelectFirstObjectOfType<BSLightingShaderProperty>(props))
	{
		switch (shaderRef->GetTextureClampMode())
		{
		case CLAMP_S_CLAMP_T:	mat.TileU = false, mat.TileV = false; break;
		case CLAMP_S_WRAP_T:	mat.TileU = false, mat.TileV = true; break;
		case WRAP_S_CLAMP_T:	mat.TileU = true, mat.TileV = false; break;
		case WRAP_S_WRAP_T: 	mat.TileU = true, mat.TileV = true; break;
		}
		mat.UOffset = shaderRef->GetUvOffset().u;
		mat.VOffset = shaderRef->GetUvOffset().v;
		mat.UScale = shaderRef->GetUvScale().u;
		mat.VScale = shaderRef->GetUvScale().v;
		mat.Alpha = shaderRef->GetAlpha();

		auto flag1 = shaderRef->GetShaderFlags1();
		auto flag2 = shaderRef->GetShaderFlags2();
		mat.ZBufferWrite = (flag2 & SLSF2_ZBUFFER_WRITE) != 0;
		mat.ZBufferTest = (flag1 & SLSF1_ZBUFFER_TEST) != 0;
		mat.ScreenSpaceReflections;
		mat.WetnessControlScreenSpaceReflections;
		mat.Decal = (flag1 & SLSF1_DECAL) != 0;
		mat.TwoSided = (flag2 & SLSF2_DOUBLE_SIDED) != 0;
		mat.DecalNoFade = (flag2 & SLSF2_NO_FADE) != 0;
		//mat.NonOccluder = (flag1 & SLSF1_PARALLAX_OCCLUSION) != 0;
		mat.Refraction = (flag1 & SLSF1_REFRACTION) != 0;
		mat.RefractionFalloff = (flag1 & SLSF1_USE_FALLOFF) != 0;
		mat.RefractionPower = shaderRef->GetRefractionStrength();
		mat.EnvironmentMapping = (flag1 & SLSF1_ENVIRONMENT_MAPPING) != 0;
		mat.EnvironmentMappingMaskScale = shaderRef->GetEnvironmentMapScale();
		mat.GrayscaleToPaletteColor = (flag1 & SLSF1_GREYSCALE_TO_PALETTECOLOR) != 0;
	}
	if (NiAlphaPropertyRef alphaProp = SelectFirstObjectOfType<NiAlphaProperty>(props))
	{
		mat.BlendState = alphaProp->GetBlendState();
		mat.BlendFunc1 = AlphaBlendFunc(alphaProp->GetSourceBlendFunc());
		mat.BlendFunc2 = AlphaBlendFunc(alphaProp->GetDestBlendFunc());
		mat.AlphaTest = alphaProp->GetTestState();
		mat.AlphaTestRef = alphaProp->GetTestThreshold();
		mat.AlphaBlendMode = ConvertAlphaBlendMode(mat.BlendState, mat.BlendFunc1, mat.BlendFunc2);
	}
	return true;
}

bool ReadBGSMFromNIF(BGSMFile& bgsm, vector<NiPropertyRef>& props)
{
	ReadMaterialFromNIF(bgsm, props);

	if (BSLightingShaderPropertyRef shaderRef = SelectFirstObjectOfType<BSLightingShaderProperty>(props))
	{
		if (BSShaderTextureSetRef textureSet = shaderRef->GetTextureSet()) {
			bgsm.DiffuseTexture = A2TString(textureSet->GetTexture(0));
			bgsm.NormalTexture = A2TString(textureSet->GetTexture(1));
			bgsm.GlowTexture = A2TString(textureSet->GetTexture(2));
			bgsm.GreyscaleTexture = A2TString(textureSet->GetTexture(3));
			bgsm.EnvmapTexture = A2TString(textureSet->GetTexture(4));

			bgsm.SmoothSpecTexture = A2TString(textureSet->GetTexture(7));
			//tstring GlowTexture;
			//tstring InnerLayerTexture;
			//tstring WrinklesTexture;
			//tstring DisplacementTexture;
		}
		auto flags1 = shaderRef->GetShaderFlags1();
		auto flags2 = shaderRef->GetShaderFlags2();
		
		//bool EnableEditorAlphaRef;
		bgsm.RimLighting = (flags2 & SLSF2_RIM_LIGHTING) != 0;
		//bgsm.RimPower = shaderRef->GetRimPower();			
		bgsm.BackLightPower = shaderRef->GetBacklightPower();
		bgsm.SubsurfaceLighting =  (flags2 & SLSF2_SOFT_LIGHTING) != 0;
		bgsm.SubsurfaceLightingRolloff = shaderRef->GetSubsurfaceRolloff();
		bgsm.SpecularEnabled = (flags1 & SLSF1_SPECULAR) != 0;
		bgsm.SpecularColor = shaderRef->GetSpecularColor();
		bgsm.SpecularMult = shaderRef->GetSpecularStrength();
		bgsm.Smoothness = shaderRef->GetSpecularPower_Glossiness();
		bgsm.FresnelPower = shaderRef->GetFresnelPower();
		bgsm.WetnessControlSpecScale = shaderRef->GetWetnessSpecScale();
		bgsm.WetnessControlSpecPowerScale = shaderRef->GetWetnessSpecPower();
		bgsm.WetnessControlSpecMinvar = shaderRef->GetWetnessMinVar();
		bgsm.WetnessControlEnvMapScale = shaderRef->GetWetnessEnvMapScale();
		bgsm.WetnessControlFresnelPower = shaderRef->GetWetnessFresnelPower();
		bgsm.WetnessControlMetalness = shaderRef->GetWetnessMetalness();

		bgsm.RootMaterialPath = A2TString(shaderRef->GetRootMaterial());
		bgsm.AnisoLighting = (flags2 & SLSF2_ANISOTROPIC_LIGHTING) != 0;
		bgsm.EmitEnabled = (flags1 & SLSF1_OWN_EMIT) != 0;  // ????
		shaderRef->SetEmissiveColor(bgsm.EmittanceColor);
		shaderRef->SetEmissiveMultiple(bgsm.EmittanceMult);
		bgsm.ModelSpaceNormals = (flags1 & SLSF1_MODEL_SPACE_NORMALS) != 0;
		bgsm.ExternalEmittance = (flags1 & SLSF1_EXTERNAL_EMITTANCE) != 0;
		bgsm.BackLighting = (flags2 & SLSF2_BACK_LIGHTING) != 0;
		bgsm.ReceiveShadows = (flags1 & SLSF1_RECIEVE_SHADOWS) != 0;
		bgsm.HideSecret = (flags1 & SLSF1_LOCALMAP_HIDE_SECRET) != 0;
		bgsm.CastShadows = (flags1 & SLSF1_CAST_SHADOWS) != 0;
		bgsm.DissolveFade = (flags2 & SLSF2_NO_FADE) != 0; // ????
		bgsm.AssumeShadowmask = (flags2 & SLSF2_ASSUME_SHADOWMASK) != 0;
		bgsm.Glowmap = (flags2 & SLSF2_GLOW_MAP) != 0;
		// bgsm.EnvironmentMappingWindow = (flags1 & SLSF1_EYE_ENVIRONMENT_MAPPING) != 0;
		bgsm.EnvironmentMappingEye = (flags1 & SLSF1_EYE_ENVIRONMENT_MAPPING) != 0;
		bgsm.Hair = (flags1 & SLSF1_HAIR_SOFT_LIGHTING) != 0;
		shaderRef->SetHairTintColor(bgsm.HairTintColor);
		bgsm.Tree = (flags2 & SLSF2_TREE_ANIM) != 0;
		bgsm.Facegen = (flags1 & SLSF1_FACEGEN_DETAIL_MAP) != 0;
		bgsm.SkinTint = (flags1 & SLSF1_FACEGEN_RGB_TINT) != 0;
		//if (bgsm.Tessellate;
		//float DisplacementTextureBias;
		//float DisplacementTextureScale;
		//float TessellationPNScale;
		//float TessellationBaseFactor;
		//float TessellationFadeDistance;
		bgsm.GrayscaleToPaletteScale = shaderRef->GetGrayscaleToPaletteScale();
		//bool SkewSpecularAlpha; // (header.Version >= 1)    

	}
	return true;
}

bool ReadBGEMFromNIF(BGEMFile& bgem, vector<NiPropertyRef>& props)
{
	ReadMaterialFromNIF(bgem, props);

	if (BSEffectShaderPropertyRef shaderRef = SelectFirstObjectOfType<BSEffectShaderProperty>(props))
	{
		bgem.BaseTexture = A2TString(shaderRef->GetSourceTexture());
		bgem.NormalTexture = A2TString(shaderRef->GetNormalTexture());
		bgem.GrayscaleTexture = A2TString(shaderRef->GetGreyscaleTexture());
		bgem.EnvmapTexture = A2TString(shaderRef->GetEnvMapTexture());
		bgem.EnvmapTexture = A2TString(shaderRef->GetEnvMaskTexture());

		auto flags1 = shaderRef->GetShaderFlags1();
		auto flags2 = shaderRef->GetShaderFlags2();

		bgem.ZBufferWrite = (flags2 | SLSF2_ZBUFFER_WRITE) != 0;
		bgem.ZBufferTest = (flags1 | SLSF1_ZBUFFER_TEST) != 0;
		//bgsm->ScreenSpaceReflections = (flags1 | SLSF21_reZBUFFER_TEST) != 0;
		//bool WetnessControlScreenSpaceReflections;
		bgem.Decal = (flags1 | SLSF1_DECAL) != 0;
		bgem.TwoSided = (flags2 | SLSF2_DOUBLE_SIDED) != 0;
		//bgem.Decal && !bgem.DecalNoFade = (flags1 | SLSF1_DYNAMIC_DECAL) != 0;// ????
		//bgem.Decal && bgem.DecalNoFade = (flags2 | SLSF2_NO_FADE) != 0;
		bgem.DecalNoFade = (flags2 | SLSF2_NO_FADE) != 0;

		//bgsm->NonOccluder = (flags1 | SLSF1_PARALLAX_OCCLUSION) != 0; 
		bgem.Refraction = (flags1 | SLSF1_REFRACTION) != 0;
		bgem.RefractionFalloff = (flags1 | SLSF1_RECIEVE_SHADOWS) != 0;
		//bgem.RefractionPower = shaderRef->GetRefractionStrength();
		bgem.EnvironmentMapping = (flags1 | SLSF1_ENVIRONMENT_MAPPING) != 0;
		bgem.EnvironmentMappingMaskScale = shaderRef->GetEnvironmentMapScale();
		bgem.GrayscaleToPaletteColor = (flags1 | SLSF1_GREYSCALE_TO_PALETTECOLOR) != 0;

		bgem.BaseTexture = A2TString(shaderRef->GetSourceTexture());
		bgem.NormalTexture = A2TString(shaderRef->GetNormalTexture());
		bgem.EnvmapTexture = A2TString(shaderRef->GetEnvMapTexture());
		bgem.GrayscaleTexture = A2TString(shaderRef->GetGreyscaleTexture());
		bgem.EnvmapMaskTexture = A2TString(shaderRef->GetEnvMaskTexture());

		bgem.NonOccluder = (flags1 | SLSF1_EXTERNAL_EMITTANCE) != 0;
		bgem.BloodEnabled = (flags1 | 0) != 0; // ???? never used
		bgem.EffectLightingEnabled = (flags1 | SLSF2_EFFECT_LIGHTING) != 0;
		bgem.FalloffEnabled = (flags1 | SLSF1_USE_FALLOFF) != 0;
		//bgem.FalloffEnabled = (flags1 | SLSF1_VERTEX_ALPHA) != 0;
		bgem.FalloffColorEnabled = (flags1 | SLSF1_FIRE_REFRACTION) != 0;
		bgem.GrayscaleToPaletteAlpha = (flags1 | SLSF1_GREYSCALE_TO_PALETTEALPHA) != 0;
		bgem.SoftEnabled = (flags1 | SLSF1_SOFT_EFFECT) != 0;

		bgem.BaseColor = TOCOLOR3(shaderRef->GetEmissiveColor());
		bgem.BaseColorScale = shaderRef->GetEmissiveMultiple();
		bgem.FalloffStartAngle = shaderRef->GetFalloffStartAngle();
		bgem.FalloffStopAngle = shaderRef->GetFalloffStopAngle();
		bgem.FalloffStartOpacity = shaderRef->GetFalloffStartOpacity();
		bgem.FalloffStopOpacity = shaderRef->GetFalloffStopOpacity();
		bgem.SoftDepth = shaderRef->GetSoftFalloffDepth();
		//bgem->LightingInfluence
		//bgem->EnvmapMinLOD
	}
	return true;
}


bool NifImporter::ImportFO4Shader(ImpNode *node, NiAVObjectRef avObject, StdMat2 *mtl)
{
	USES_CONVERSION;
	if (!useNiftoolsShader || !mtl || !mtl->SupportsShaders())
		return false;

	if (!(useNiftoolsShader == 1 && mtl->SwitchShader(FO4SHADER_CLASS_ID)))
		return false;
	
	NiGeometryRef geom = DynamicCast<NiGeometry>(avObject);
	vector<NiPropertyRef> props = avObject->GetProperties();

	if ( Shader *s = mtl->GetShader() )
	{
		if ( IBSShaderMaterialData *data = static_cast<IBSShaderMaterialData*>(s->GetInterface(I_BSSHADERDATA)) )
		{
			if (BSLightingShaderPropertyRef lightingShaderRef = SelectFirstObjectOfType<BSLightingShaderProperty>(props))
			{
				BGSMFile materialData;
				bool success = false;
				string name = lightingShaderRef->GetName();
				if (wildmatch("*.BGSM", name) )
				{
					tstring filename = FindMaterial(A2TString(name));
					TSTR tsname = A2TString(name).c_str();
					TSTR tsfile = filename.c_str();
					data->SetMaterialName(tsname);
					data->SetFileName(tsfile);
					success = ReadBGSMFile(filename, materialData);
				}
				if (!success)
				{
					InitialzeBGSM(materialData);
					ReadBGSMFromNIF(materialData, props);
				}
				data->LoadBGSM(materialData);
				data->LoadMaterial(mtl, this);
			}
			if (BSEffectShaderPropertyRef effectShaderRef = SelectFirstObjectOfType<BSEffectShaderProperty>(props))
			{
				BGEMFile materialData;
				bool success = false;
				TSTR tsfile;
				string name = effectShaderRef->GetName();
				TSTR tsname = A2TString(name).c_str();
				if (wildmatch("*.BGEM", name))
				{
					tstring filename = FindMaterial(A2TString(name));
					tsfile = filename.c_str();
					data->SetMaterialName(tsname);
					data->SetFileName(tsfile);
					success = ReadBGEMFile(filename, materialData);
				} else {
					tsfile = tsname;
				}
				if (!success)
				{
					InitialzeBGEM(materialData);
					ReadBGEMFromNIF(materialData, props);
				}
				data->LoadBGEM(materialData);
				data->LoadMaterial(mtl, this);
				data->SetMaterialName(tsname);
				data->SetFileName(tsfile);
			}
			return true;
		}
	}
	return false;
}

tstring NifImporter::FindImage(const tstring& name) const
{
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(name.c_str())) {
		if (-1 != _taccess(name.c_str(), 0))
			return name;
	}
	if (!path.empty()) {
		PathCombine(buffer, path.c_str(), name.c_str()); // try as-is
		if (-1 != _taccess(buffer, 0))
			return tstring(buffer);

		// try only filename in nif directory
		PathCombine(buffer, path.c_str(), PathFindFileName(name.c_str()));
		if (-1 != _taccess(buffer, 0))
			return tstring(buffer);
	}
	if (appSettings != nullptr) {
		return appSettings->FindImage(name);
	}
	return name;
}

Texmap* NifImporter::GetMaterialTextureSubMap(Mtl* mat, int id)
{
	if (mat && mat->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat2 *mtl = (StdMat2*)mat;
		Shader *s = mtl->GetShader();
		if (s != nullptr && s->ClassID() == NIFSHADER_CLASS_ID) {
			switch (id)
			{
			case ID_DI:  return mat->GetSubTexmap(C_BASE);
			case ID_BU:  return mat->GetSubTexmap(C_BUMP);
			case ID_RL:  return mat->GetSubTexmap(C_REFLECTION);
			case ID_OP:  return mat->GetSubTexmap(C_OPACITY);
			case ID_SP:  return mat->GetSubTexmap(C_SPECULAR);
			case ID_SH:  return mat->GetSubTexmap(C_GLOSS);
			case ID_SI:  return mat->GetSubTexmap(C_GLOW);
			case ID_DP:  return mat->GetSubTexmap(C_HEIGHT);
			}
		}
	}
	if (mat != nullptr)
	{
		return mat->GetSubTexmap(id);
	}
	return nullptr;
}

tstring NifImporter::FindMaterial(const tstring& name) const
{
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(name.c_str())) {
		if (-1 != _taccess(name.c_str(), 0))
			return tstring(name);
	}
	if (!path.empty()) {
		PathCombine(buffer, path.c_str(), name.c_str()); // try as-is
		if (-1 != _taccess(buffer, 0))
			return tstring(buffer);

		// try only filename in nif directory
		PathCombine(buffer, path.c_str(), PathFindFileName(name.c_str()));
		if (-1 != _taccess(buffer, 0))
			return tstring(buffer);
	}
	// basically try to find the materials part and then import
	_tcscpy(buffer, name.c_str());
	PathMakePretty(buffer);
	for (LPCTSTR filepart = buffer; filepart != nullptr; filepart = PathFindNextComponent(filepart)) {
		if (wildmatch(TEXT("materials\\*"), filepart)) {
			if (appSettings != nullptr) {
				return appSettings->FindMaterial(filepart);
			}
			break;
		}
	}

	if (appSettings != nullptr) {
		return appSettings->FindMaterial(name);
	}
	return name;
}