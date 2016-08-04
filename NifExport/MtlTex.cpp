#include "pch.h"
#include "stdmat.h"
#include "shaders.h"
#include "AppSettings.h"
#include "gen/enums.h"
#include "obj/NiWireframeProperty.h"
#include "obj/NiAlphaProperty.h"
#include "obj/NiStencilProperty.h"
#include "obj/NiShadeProperty.h"
#include "obj/NiVertexColorProperty.h"
#include "obj/NiDitherProperty.h"
#include "obj/NiSpecularProperty.h"
#include "obj/NiTextureProperty.h"
#include "obj/NiImage.h"
#include "obj/WaterShaderProperty.h"
#include "obj/SkyShaderProperty.h"
#include "obj/TallGrassShaderProperty.h"
#include "obj/Lighting30ShaderProperty.h"
#include "obj/BSShaderNoLightingProperty.h"
#include "obj/BSShaderPPLightingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/BSLightingShaderProperty.h"
#include "ObjectRegistry.h"
#include <obj/BSEffectShaderProperty.h>
#include "../NifProps/iNifProps.h"
#include "../MtlUtils/mtldefine.h"
#include <obj/BSTriShape.h>

enum {
	C_BASE, C_DARK, C_DETAIL, C_GLOSS, C_GLOW, C_BUMP, C_NORMAL,
	C_DECAL0, C_DECAL1, C_DECAL2, C_DECAL3, C_ENVMASK, C_ENV,
	C_HEIGHT, C_REFLECTION, C_OPACITY, C_SPECULAR, C_PARALLAX,
	C_BACKLIGHT,
};

#undef GNORMAL_CLASS_ID
static const Class_ID GNORMAL_CLASS_ID(0x243e22c6, 0x63f6a014);

void Exporter::makeTexture(NiAVObjectRef &parent, Mtl *mtl)
{
	USES_CONVERSION;
	BitmapTex *bmTex = getTexture(mtl);
	if (!bmTex)
		return;

	LPCTSTR sTexPrefix = mTexPrefix.c_str();

	if (IsSkyrim() || IsFallout4())
	{
		NiGeometryRef geom = DynamicCast<NiGeometry>(parent);

		BSLightingShaderPropertyRef texProp = new BSLightingShaderProperty();

		//0=default 1=EnvMap, 2=Glow, 5=Skin, 6=Hair, 7=Unknown, 11=Ice/Parallax, 15=Eye.
		BSLightingShaderPropertyShaderType shaderType = Niflib::LSPST_DEFAULT;

		SkyrimShaderPropertyFlags1 flags1 = (SkyrimShaderPropertyFlags1)(SLSF1_SPECULAR | SLSF1_RECIEVE_SHADOWS | SLSF1_CAST_SHADOWS | SLSF1_OWN_EMIT | SLSF1_REMAPPABLE_TEXTURES | SLSF1_ZBUFFER_TEST);
		SkyrimShaderPropertyFlags2 flags2 = (SkyrimShaderPropertyFlags2)(SLSF2_ZBUFFER_WRITE );

		if (geom && geom->IsSkin()) {
			flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
			flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_ENVMAP_LIGHT_FADE | SLSF2_DOUBLE_SIDED);
		}

		TexClampMode clampMode = WRAP_S_WRAP_T;
		switch (bmTex->GetTextureTiling())
		{
		case 3: clampMode = WRAP_S_WRAP_T; break;
		case 1: clampMode = WRAP_S_CLAMP_T; break;
		case 2: clampMode = CLAMP_S_WRAP_T; break;
		case 0: clampMode = CLAMP_S_CLAMP_T; break;
		}
		texProp->SetTextureClampMode(clampMode);

		BSShaderTextureSetRef texset = new BSShaderTextureSet();
		texProp->SetTextureSet(texset);

		vector<string> textures;
		textures.resize(9);

		texProp->SetSpecularPower_Glossiness(80);
		texProp->SetSpecularColor(Color3(0.933f, 0.855f, 0.804f));
		texProp->SetSpecularStrength(1.0f);
		texProp->SetLightingEffect1(0.3f);
		texProp->SetLightingEffect2(2.0f);
		texProp->SetEnvironmentMapScale(1.0f);
		texProp->SetUnknownFloat1(FLT_MAX); // must be set to avoid hangs

		TSTR diffuseStr, normalStr, glowStr, dispStr, envStr, envMaskStr, parallaxStr, backlightStr;

		if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
			StdMat2 *m = (StdMat2*)mtl;

			if (m->GetMapState(ID_DI) == 2) {
				diffuseStr = m->GetMapName(ID_DI);
				if (Texmap *texMap = m->GetSubTexmap(ID_DI)) {
					GetTexFullName(texMap, diffuseStr);
				}
			}
			else if (m->GetMapState(ID_AM) == 2) {
				diffuseStr = m->GetMapName(ID_AM);
				if (Texmap *texMap = m->GetSubTexmap(ID_AM)) {
					GetTexFullName(texMap, diffuseStr);
				}
			}
			if (m->GetMapState(ID_BU) == 2) {
				normalStr = m->GetMapName(ID_BU);
				if (Texmap *texMap = m->GetSubTexmap(ID_BU)) {
					if (texMap->ClassID() == GNORMAL_CLASS_ID) {
						texMap = texMap->GetSubTexmap(0);
						GetTexFullName(texMap, normalStr);
					}
					else {
						GetTexFullName(texMap, normalStr);
					}
				}
			}
			if (m->GetMapState(ID_RL) == 2) {
				envStr = m->GetMapName(ID_RL);
				if (Texmap *texMap = m->GetSubTexmap(ID_RL)) {
					if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
						Texmap *envMap = texMap->GetSubTexmap(0);
						Texmap *envMaskMap = texMap->GetSubTexmap(1);
						GetTexFullName(envMap, envStr);
						GetTexFullName(envMaskMap, envMaskStr);
					}
					else {
						GetTexFullName(texMap, envStr);
					}
					shaderType = Niflib::LSPST_ENVIRONMENT_MAP;
					flags1 = (SkyrimShaderPropertyFlags1)(flags1 | Niflib::SLSF1_ENVIRONMENT_MAPPING);
					flags2 = (SkyrimShaderPropertyFlags2)(flags2 | Niflib::SLSF2_ENVMAP_LIGHT_FADE);
				}
			}
			if (m->GetMapState(ID_SP) == 2) {
				glowStr = m->GetMapName(ID_SP);
				if (Texmap *texMap = m->GetSubTexmap(ID_SP)) {
					GetTexFullName(texMap, glowStr);
					flags1 = (SkyrimShaderPropertyFlags1)(flags1 | Niflib::SLSF1_MODEL_SPACE_NORMALS);
				}
			}
			
			if (m->GetMapState(ID_SI) == 2) {
				glowStr = m->GetMapName(ID_SI);
				if (Texmap *texMap = m->GetSubTexmap(ID_SI)) {
					GetTexFullName(texMap, glowStr);
				}
			}
			if (m->GetMapState(ID_DP) == 2) {
				dispStr = m->GetMapName(ID_DP);
				if (Texmap *texMap = m->GetSubTexmap(ID_DP)) {
					GetTexFullName(texMap, dispStr);
				}
			}

			TimeValue t = 0;
			texProp->SetSpecularPower_Glossiness(m->GetShininess(t) * 100.0f);
			//texProp->SetSpecularColor(m->getsh);
			//texProp->SetSpecularStrength(m->GetShinStr(t) < 1.0f ? 3.0f : 0.0f);
			texProp->SetAlpha(m->GetOpacity(t));
			texProp->SetSpecularColor(TOCOLOR3(m->GetSpecular(t)));
			//texProp->SetEmissiveColor(TOCOLOR(m->GetEmmis(t)));

			texProp->SetShaderFlags1(flags1);
			texProp->SetShaderFlags2(flags2);
		}

		textures[0] = T2AString(mAppSettings->GetRelativeTexPath(diffuseStr, sTexPrefix));
		if (normalStr.isNull()) {
			char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			_splitpath(textures[0].c_str(), drive, dir, fname, ext);
			strcat(fname, "_n");
			_makepath(path_buffer, drive, dir, fname, ext);
			textures[1] = path_buffer;
		}
		else
			textures[1] = T2AString(mAppSettings->GetRelativeTexPath(normalStr, sTexPrefix));
		if (!glowStr.isNull())
			textures[2] = T2AString(mAppSettings->GetRelativeTexPath(glowStr, sTexPrefix));
		if (!parallaxStr.isNull())
			textures[3] = T2AString(mAppSettings->GetRelativeTexPath(parallaxStr, sTexPrefix));
		if (!envStr.isNull())
			textures[4] = T2AString(mAppSettings->GetRelativeTexPath(envStr, sTexPrefix));
		if (!envMaskStr.isNull())
			textures[5] = T2AString(mAppSettings->GetRelativeTexPath(envMaskStr, sTexPrefix));
		if (!dispStr.isNull())
			textures[6] = T2AString(mAppSettings->GetRelativeTexPath(dispStr, sTexPrefix));
		if (!backlightStr.isNull())
			textures[7] = T2AString(mAppSettings->GetRelativeTexPath(backlightStr, sTexPrefix));

		texset->SetTextures(textures);

		texProp->SetSkyrimShaderType(shaderType);

		// shader must be first, alpha can be second
		NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
		vector<NiPropertyRef> properties = parent->GetProperties();
		parent->ClearProperties();
		parent->AddProperty(prop);
		if (properties.size() > 0)
			parent->AddProperty(properties[0]);
	}
	else if (IsFallout3())
	{
		BSShaderPPLightingPropertyRef texProp = new BSShaderPPLightingProperty();

		texProp->SetFlags(1);
		texProp->SetShaderType(SHADER_DEFAULT);
		BSShaderTextureSetRef texset = new BSShaderTextureSet();
		texProp->SetTextureSet(texset);

		vector<string> textures;
		textures.resize(6);

		TSTR diffuseStr, normalStr, glowStr, dispStr, envStr, envMaskStr;

		if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
			StdMat2 *m = (StdMat2*)mtl;

			if (m->GetMapState(ID_DI) == 2) {
				diffuseStr = m->GetMapName(ID_DI);
				if (Texmap *texMap = m->GetSubTexmap(ID_DI)) {
					GetTexFullName(texMap, diffuseStr);
				}
			}
			else if (m->GetMapState(ID_AM) == 2) {
				diffuseStr = m->GetMapName(ID_AM);
				if (Texmap *texMap = m->GetSubTexmap(ID_AM)) {
					GetTexFullName(texMap, diffuseStr);
				}
			}
			if (m->GetMapState(ID_BU) == 2) {
				normalStr = m->GetMapName(ID_BU);
				if (Texmap *texMap = m->GetSubTexmap(ID_BU)) {
					if (texMap->ClassID() == GNORMAL_CLASS_ID) {
						texMap = texMap->GetSubTexmap(0);
						GetTexFullName(texMap, normalStr);
					}
					else {
						GetTexFullName(texMap, normalStr);
					}
				}
			}
			if (m->GetMapState(ID_RL) == 2) {
				envStr = m->GetMapName(ID_RL);
				if (Texmap *texMap = m->GetSubTexmap(ID_RL)) {
					if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
						Texmap *envMap = texMap->GetSubTexmap(0);
						Texmap *envMaskMap = texMap->GetSubTexmap(1);
						GetTexFullName(envMap, envStr);
						GetTexFullName(envMaskMap, envMaskStr);
					}
					else {
						GetTexFullName(texMap, envStr);
					}
				}
			}
			if (m->GetMapState(ID_SI) == 2) {
				glowStr = m->GetMapName(ID_SI);
				if (Texmap *texMap = m->GetSubTexmap(ID_SI)) {
					GetTexFullName(texMap, glowStr);
				}
			}
			if (m->GetMapState(ID_DP) == 2) {
				dispStr = m->GetMapName(ID_DP);
				if (Texmap *texMap = m->GetSubTexmap(ID_DP)) {
					GetTexFullName(texMap, dispStr);
				}
			}
		}
		textures[0] = T2AString(mAppSettings->GetRelativeTexPath(diffuseStr, sTexPrefix));
		if (normalStr.isNull()) {
			char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			_splitpath(textures[0].c_str(), drive, dir, fname, ext);
			strcat(fname, "_n");
			_makepath(path_buffer, drive, dir, fname, ext);
			textures[1] = path_buffer;
		}
		else
			textures[1] = T2AString(mAppSettings->GetRelativeTexPath(normalStr, sTexPrefix));
		if (!envMaskStr.isNull())
			textures[2] = T2AString(mAppSettings->GetRelativeTexPath(envMaskStr, sTexPrefix));
		if (!glowStr.isNull())
			textures[3] = T2AString(mAppSettings->GetRelativeTexPath(glowStr, sTexPrefix));
		if (glowStr.isNull()) {
			char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			_splitpath(textures[0].c_str(), drive, dir, fname, ext);
			strcat(fname, "_g");
			_makepath(path_buffer, drive, dir, fname, ext);
			textures[3] = path_buffer;
		}
		else {
			textures[3] = T2AString(mAppSettings->GetRelativeTexPath(glowStr, sTexPrefix));
		}
		if (!dispStr.isNull())
			textures[4] = T2AString(mAppSettings->GetRelativeTexPath(dispStr, sTexPrefix));
		if (!envStr.isNull())
			textures[5] = T2AString(mAppSettings->GetRelativeTexPath(envStr, sTexPrefix));

		BSShaderFlags shFlags = BSShaderFlags(SF_SPECULAR | SF_SHADOW_MAP | SF_SHADOW_FRUSTUM | SF_EMPTY | SF_ZBUFFER_TEST);
		if (!envStr.isNull() || !dispStr.isNull())
			shFlags = BSShaderFlags(shFlags | SF_MULTIPLE_TEXTURES);
		texProp->SetShaderFlags(shFlags);

		texset->SetTextures(textures);

		NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
		parent->AddProperty(prop);
	}
	else if (Exporter::mNifVersionInt >= VER_4_0_0_0)
	{
		NiTexturingPropertyRef texProp = CreateNiObject<NiTexturingProperty>();
		texProp->SetApplyMode(APPLY_MODULATE);
		texProp->SetTextureCount(7);

		TexDesc td;
		NiTriBasedGeom *shape(DynamicCast<NiTriBasedGeom>(parent));
		if (makeTextureDesc(bmTex, td, shape->GetData()))
			texProp->SetTexture(BASE_MAP, td);

		NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
		parent->AddProperty(prop);
	}
	else
	{
		NiTexturePropertyRef texProp = CreateNiObject<NiTextureProperty>();
		NiImageRef imgProp = CreateNiObject<NiImage>();
		texProp->SetImage(imgProp);

		// Get file name and check if it matches the "app" settings in the ini file
		TSTR mapPath = bmTex->GetMapName();
		if (mAppSettings)
		{
			string newPath = T2AString(mAppSettings->GetRelativeTexPath(mapPath, sTexPrefix));
			imgProp->SetExternalTexture(newPath);
		}
		else
		{
			TSTR p, f;
			SplitPathFile(mapPath, &p, &f);
			TSTR newPath = mTexPrefix.empty() ? f : (TSTR(sTexPrefix) + _T("\\") + f);
			imgProp->SetExternalTexture(T2A(newPath.data()));
		}
		NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
		parent->AddProperty(prop);
	}
}

bool Exporter::makeTextureDesc(BitmapTex *bmTex, TexDesc& td, NiTriBasedGeomDataRef shape)
{
	USES_CONVERSION;
	LPCTSTR sTexPrefix = mTexPrefix.c_str();

	td.source = new NiSourceTexture();

	// Filtering
	switch (bmTex->GetFilterType())
	{
	case FILTER_PYR:  td.filterMode = FILTER_TRILERP; break;
	case FILTER_SAT:  td.filterMode = FILTER_BILERP; break;
	case FILTER_NADA: td.filterMode = FILTER_NEAREST; break;
	}

	td.clampMode = TexClampMode();
	switch (bmTex->GetTextureTiling())
	{
	case 3: td.clampMode = WRAP_S_WRAP_T; break;
	case 1: td.clampMode = WRAP_S_CLAMP_T; break;
	case 2: td.clampMode = CLAMP_S_WRAP_T; break;
	case 0: td.clampMode = CLAMP_S_CLAMP_T; break;
	}

	if (UVGen *uvGen = bmTex->GetTheUVGen()) {
		if (uvGen && uvGen->IsStdUVGen()) {
			StdUVGen *uvg = (StdUVGen*)uvGen;
			td.uvSet = shape->GetUVSetIndex(uvg->GetMapChannel());
			if (td.uvSet == -1) td.uvSet = 1;
		}
		if (RefTargetHandle ref = uvGen->GetReference(0)) {
			TexCoord trans, tiling;
			float wangle;
			bool ok = true;
			if (ok) ok &= getMAXScriptValue(ref, TEXT("U_Offset"), 0, trans.u);
			if (ok) ok &= getMAXScriptValue(ref, TEXT("V_Offset"), 0, trans.v);
			if (ok) ok &= getMAXScriptValue(ref, TEXT("U_Tiling"), 0, tiling.u);
			if (ok) ok &= getMAXScriptValue(ref, TEXT("V_Tiling"), 0, tiling.v);
			if (ok) ok &= getMAXScriptValue(ref, TEXT("W_Angle"), 0, wangle);
			if (ok) {
				if (trans.u != 0.0f || trans.v != 0.0f || tiling.u != 1.0f || tiling.v != 1.0f || wangle != 0.0f) {
					td.hasTextureTransform = true;
					td.translation = trans;
					td.tiling = tiling;
					td.wRotation = TORAD(wangle);
					td.transformType_ = 1;
					td.centerOffset = TexCoord(0.5, 0.5);
				}
			}
		}
	}
	if (Exporter::mNifVersionInt >= VER_20_0_0_4)
	{
		td.source->SetPixelLayout(PIX_LAY_DEFAULT);
		td.source->SetMipMapFormat(MIP_FMT_DEFAULT);
		td.source->SetAlphaFormat(ALPHA_DEFAULT);
	}

	// Get file name and check if it matches the "app" settings in the ini file
	TSTR mapPath;
	mapPath = bmTex->GetMapName();

	if (mAppSettings)
	{
		string newPath = T2AString(mAppSettings->GetRelativeTexPath(mapPath, sTexPrefix));
		td.source->SetExternalTexture(newPath);
	}
	else
	{
		TSTR p, f;
		SplitPathFile(mapPath, &p, &f);
		TSTR newPath;
		if (!mTexPrefix.empty())
			newPath = TSTR(sTexPrefix) + _T("\\") + f;
		else
			newPath = f;

		td.source->SetExternalTexture(T2A(newPath));
	}
	return true;
}

void Exporter::makeMaterial(NiAVObjectRef &parent, Mtl *mtl)
{
	USES_CONVERSION;

	if (IsFallout4() && exportFO4Shader(parent, mtl)) 
		return;		

	// Fill-in using the Civ4 Shader if available
	if (exportNiftoolsShader(parent, mtl))
		return;

	if (!IsSkyrim() && !IsFallout4())
	{
		string name;
		NiMaterialPropertyRef mtlProp(CreateNiObject<NiMaterialProperty>());
		if (mtl)
		{
			Color c;

			c = mtl->GetAmbient();
			mtlProp->SetAmbientColor(Color3(c.r, c.g, c.b));

			c = mtl->GetDiffuse();
			mtlProp->SetDiffuseColor(Color3(c.r, c.g, c.b));

			c = mtl->GetSpecular();
			mtlProp->SetSpecularColor(Color3(c.r, c.g, c.b));

			c = (mtl->GetSelfIllumColorOn()) ? mtl->GetSelfIllumColor() : Color(0, 0, 0);
			mtlProp->SetEmissiveColor(Color3(c.r, c.g, c.b));

			mtlProp->SetTransparency(1);
			mtlProp->SetGlossiness(mtl->GetShininess() * 100.0f);
			name = T2A(mtl->GetName());

			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
			{
				StdMat2 * smtl = (StdMat2*)mtl;
				mtlProp->SetTransparency(smtl->GetOpacity(0));

				if (smtl->SupportsShaders()) {
					if (Shader *s = smtl->GetShader()) {
						if (smtl->GetWire()) {
							NiWireframePropertyRef wireProp = new NiWireframeProperty();
							wireProp->SetFlags(1);
							parent->AddProperty(wireProp);
						}
						if (smtl->GetTwoSided()) {
							NiStencilPropertyRef stencil = new NiStencilProperty();
							stencil->SetStencilFunction(TEST_GREATER);
							stencil->SetStencilState(false);
							stencil->SetPassAction(ACTION_INCREMENT);
							stencil->SetFaceDrawMode(DRAW_BOTH);
							stencil->SetFlags(19840);
							parent->AddProperty(stencil);
						}
						if (smtl->IsFaceted()) {
							NiShadePropertyRef shade = CreateNiObject<NiShadeProperty>();
							shade->SetFlags(0);
							parent->AddProperty(shade);
						}
					}
				}
			}
		}
		else
		{
			mtlProp->SetAmbientColor(Color3(0.588f, 0.588f, 0.588f));
			mtlProp->SetDiffuseColor(Color3(1, 1, 1));
			mtlProp->SetSpecularColor(Color3(0.9f, 0.9f, 0.9f));
			mtlProp->SetEmissiveColor(Color3(0, 0, 0));
			mtlProp->SetTransparency(1);
			mtlProp->SetGlossiness(10);
			name = "default";
		}

		mtlProp->SetName(name);

		parent->AddProperty(DynamicCast<NiProperty>(mtlProp));
	}

	makeTexture(parent, mtl);
}


Mtl *Exporter::getMaterial(INode *node, int subMtl)
{
	Mtl *nodeMtl = node->GetMtl();
	if (nodeMtl)
	{
		int numSub = nodeMtl->NumSubMtls();
		if (numSub > 0)
			return nodeMtl->GetSubMtl(subMtl % numSub);

		return nodeMtl;
	}
	return nullptr;
}

BitmapTex *Exporter::getTexture(Mtl *mtl)
{
	if (!mtl)
		return nullptr;
	int texMaps = mtl->NumSubTexmaps();
	if (!texMaps)
		return nullptr;
	for (int i = 0; i < texMaps; i++)
	{
		Texmap *texMap = mtl->GetSubTexmap(i);
		if (!texMap)
			continue;

		if (texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
			return (BitmapTex*)texMap;
		}
		else if (texMap && texMap->ClassID() == GNORMAL_CLASS_ID) {
			texMap = texMap->GetSubTexmap(0);
			if (texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
				return ((BitmapTex*)texMap);
			}
		}
	}
	return nullptr;
}

BitmapTex *Exporter::getTexture(Mtl *mtl, int i)
{
	if (mtl) {
		int texMaps = mtl->NumSubTexmaps();
		if (i < texMaps) {
			if (Texmap *texMap = mtl->GetSubTexmap(i)) {
				if (texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					return (BitmapTex*)texMap;
				}
				else if (texMap && texMap->ClassID() == GNORMAL_CLASS_ID) {
					texMap = texMap->GetSubTexmap(0);
					if (texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
						return ((BitmapTex*)texMap);
					}
				}
			}
		}
	}
	return nullptr;
}

void Exporter::getTextureMatrix(Matrix3 &mat, Mtl *mtl)
{
	BitmapTex *tex = getTexture(mtl);
	if (tex)
		tex->GetUVTransform(mat);
	else
		mat.IdentityMatrix();
}


bool Exporter::exportNiftoolsShader(NiAVObjectRef parent, Mtl* mtl)
{
	USES_CONVERSION;
	if (!mtl)
		return false;

	LPCTSTR sTexPrefix = mTexPrefix.c_str();
	RefTargetHandle ref = mtl->GetReference(2/*shader*/);
	if (!ref)
		return false;

	const Class_ID civ4Shader(0x670a77d0, 0x23ab5c7f);
	const Class_ID NIFSHADER_CLASS_ID(0x566e8ccb, 0xb091bd48);

	TSTR shaderByName; Class_ID shaderID = Class_ID(0, 0);
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat2 * smtl = (StdMat2*)mtl;
		if (smtl->SupportsShaders()) {
			if (Shader *s = smtl->GetShader()) {
				s->GetClassName(shaderByName);
				shaderID = s->ClassID();
			}
		}
	}
	if (shaderID != NIFSHADER_CLASS_ID && shaderID != civ4Shader)
	{
		if (shaderByName != TSTR(TEXT("CivilizationIV Shader")))
			return false;
	}
	Color ambient = Color(0.0f, 0.0f, 0.0f), diffuse = Color(0.0f, 0.0f, 0.0f), specular = Color(0.0f, 0.0f, 0.0f), emittance = Color(0.0f, 0.0f, 0.0f);
	float shininess = 0.0f, alpha = 0.0f, Magnitude = 0.0f, LumaScale = 0.0f, LumaOffset = 0.0f;
	int TestRef = 0, srcBlend = 0, destBlend = 0, TestMode = 0;
	bool AlphaTestEnable = false;
	int ApplyMode = 0, SrcVertexMode = 0, LightingMode = 0;
	bool VertexColorsEnable = false, SpecularEnable = false, NoSorter = false, Dither = false;
	int alphaMode = 0, BaseTextureExport = 0, DarkTextureExport = 0, DetailTextureExport = 0;
	int Decal1TextureExport = 0, Decal2TextureExport = 0, GlossTextureExport = 0, GlowTextureExport = 0;
	TSTR CustomShader;
	int ShaderViewerTechnique = 0, ShaderExportTechnique = 0;
	bool UseNormalMaps = false;
	int NormalMapTechnique = 0;
	float envmapscale = 1.0f, specularStrength = 1.0f, refractionStrength = 0.0f, lighteff1 = 0.3f, lighteff2 = 2.0f;

	Color skinTintColor = Color(0.0f, 0.0f, 0.0f), hairTintColor = Color(0.0f, 0.0f, 0.0f);
	float maxPasses = 1.0f, parallaxScale = 1.0f;
	float parallaxInnerThickness, parallaxRefractionScale = 0.0f;
	Point2 parallaxInnerTextureScale;
	float parallaxEnvmapStr = 0.0f, eyeCubemapScale = 0.0f;
	Point3 leftEyeReflCenter, rightEyeReflCenter;


	bool ok = true;
	if (ok) ok &= getMAXScriptValue(ref, TEXT("ambient"), 0, ambient);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("diffuse"), 0, diffuse);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("specular"), 0, specular);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("emittance"), 0, emittance);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("shininess"), 0, shininess);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("alpha"), 0, alpha);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("Bump_Map_Magnitude"), 0, Magnitude);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("Bump_Map_Luma_Scale"), 0, LumaScale);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("Bump_Map_Luma_offset"), 0, LumaOffset);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("TestRef"), 0, TestRef);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("AlphaTestEnable"), 0, AlphaTestEnable);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("SpecularEnable"), 0, SpecularEnable);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("NoSorter"), 0, NoSorter);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("Dither"), 0, Dither);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("srcBlend"), 0, srcBlend);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("destBlend"), 0, destBlend);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("TestMode"), 0, TestMode);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("ApplyMode"), 0, ApplyMode);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("SourceVertexMode"), 0, SrcVertexMode);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("LightingMode"), 0, LightingMode);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("alphaMode"), 0, alphaMode);
	if (ok) ok &= getMAXScriptValue(ref, TEXT("CustomShader"), 0, CustomShader);
	if (!getMAXScriptValue(ref, TEXT("EnvMapScale"), 0, envmapscale))
		envmapscale = 1.0f;
	if (!getMAXScriptValue(ref, TEXT("specularLevel"), 0, specularStrength))
		specularStrength = 1.0f;
	if (!getMAXScriptValue(ref, TEXT("RefractionStrength"), 0, refractionStrength))
		refractionStrength = 0.0f;
	if (!getMAXScriptValue(ref, TEXT("LightingEffect1"), 0, lighteff1))
		lighteff1 = 0.3f;
	if (!getMAXScriptValue(ref, TEXT("LightingEffect2"), 0, lighteff2))
		lighteff2 = 2.0f;
	if (!getMAXScriptValue(ref, TEXT("SkinTintColor"), 0, skinTintColor))
		skinTintColor = Color(0.0f, 0.0f, 0.0f);
	if (!getMAXScriptValue(ref, TEXT("HairTintColor"), 0, hairTintColor))
		hairTintColor = Color(0.0f, 0.0f, 0.0f);
	if (!getMAXScriptValue(ref, TEXT("MaxPasses"), 0, maxPasses))
		maxPasses = 1.0f;
	if (!getMAXScriptValue(ref, TEXT("ParallaxScale"), 0, parallaxScale))
		parallaxScale = 1.0f;
	if (!getMAXScriptValue(ref, TEXT("ParallaxInnerThickness"), 0, parallaxInnerThickness))
		parallaxInnerThickness = 0.0f;
	if (!getMAXScriptValue(ref, TEXT("ParallaxRefractionScale"), 0, parallaxRefractionScale))
		parallaxRefractionScale = 0.0f;
	//if (!getMAXScriptValue(ref, TEXT("ParallaxInnerTextureScale"), 0, parallaxInnerTextureScale))
	//	parallaxInnerTextureScale = Point2();
	if (!getMAXScriptValue(ref, TEXT("ParallaxEnvmapStr"), 0, parallaxEnvmapStr))
		parallaxEnvmapStr = 0.0f;
	if (!getMAXScriptValue(ref, TEXT("EyeCubemapScale"), 0, eyeCubemapScale))
		eyeCubemapScale = 0.0f;
	//if (!getMAXScriptValue(ref, TEXT("LeftEyeReflCenter"), 0, leftEyeReflCenter))
	//	leftEyeReflCenter = Point3();
	//if (!getMAXScriptValue(ref, TEXT("RightEyeReflCenter"), 0, rightEyeReflCenter))
	//	rightEyeReflCenter = Point3();
	if (!getMAXScriptValue(ref, TEXT("Vertex_Color_Enable"), 0, VertexColorsEnable) &&
		!getMAXScriptValue(ref, TEXT("VertexColorsEnable"), 0, VertexColorsEnable))
		VertexColorsEnable = false;


	if (ok) // civ4 shader
	{
		if (!IsSkyrim() && !IsFallout4()) // skyrim does not use material properties
		{
			NiMaterialPropertyRef mtlProp = CreateNiObject<NiMaterialProperty>();
			parent->AddProperty(mtlProp);

			mtlProp->SetName(T2A(mtl->GetName()));
			mtlProp->SetAmbientColor(TOCOLOR3(ambient));
			mtlProp->SetDiffuseColor(TOCOLOR3(diffuse));
			mtlProp->SetSpecularColor(TOCOLOR3(specular));
			mtlProp->SetEmissiveColor(TOCOLOR3(emittance));
			mtlProp->SetGlossiness(shininess);
			mtlProp->SetTransparency(alpha);
		}
		if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
		{
			StdMat2 * smtl = (StdMat2*)mtl;
			if (smtl->SupportsShaders() && !IsSkyrim() && !IsFallout4()) {
				if (Shader *s = smtl->GetShader()) {
					if (smtl->GetWire()) {
						NiWireframePropertyRef wireProp = CreateNiObject<NiWireframeProperty>();
						wireProp->SetFlags(1);
						parent->AddProperty(wireProp);
					}
					if (smtl->GetTwoSided()) {
						NiStencilPropertyRef stencil = CreateNiObject<NiStencilProperty>();
						stencil->SetStencilFunction(TEST_GREATER);
						stencil->SetStencilState(false);
						stencil->SetPassAction(ACTION_INCREMENT);
						stencil->SetFaceDrawMode(DRAW_BOTH);
						parent->AddProperty(stencil);
					}
					if (smtl->IsFaceted()) {
						NiShadePropertyRef shade = CreateNiObject<NiShadeProperty>();
						shade->SetFlags(0);
						parent->AddProperty(shade);
					}
				}
			}
		}
		//EEDOCAN: ck doesn't read this
		if (!IsSkyrim() && mVertexColors && VertexColorsEnable) {
			NiVertexColorPropertyRef vertexColor = CreateNiObject<NiVertexColorProperty>();
			parent->AddProperty(vertexColor);
			vertexColor->SetVertexMode(VertMode(SrcVertexMode));
			vertexColor->SetLightingMode(LightMode(LightingMode));
			vertexColor->SetFlags((LightingMode << 3) + (SrcVertexMode << 4));
		}
		if (SpecularEnable && !IsSkyrim() && !IsFallout4()) {
			NiSpecularPropertyRef prop = CreateNiObject<NiSpecularProperty>();
			parent->AddProperty(prop);
			prop->SetFlags(1);

		}
		if (Dither && !IsSkyrim() && !IsFallout4()) {
			NiDitherPropertyRef prop = CreateNiObject<NiDitherProperty>();
			parent->AddProperty(prop);
			prop->SetFlags(1);
		}
		if (IsSkyrim() || (alphaMode != 0 || AlphaTestEnable)) {
			// always add alpha ???
			NiAlphaPropertyRef alphaProp = CreateNiObject<NiAlphaProperty>();
			alphaProp->SetBlendState(true);
			if (alphaMode == 0) { // automatic
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BlendFunc(srcBlend));
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BlendFunc(destBlend));
			}
			else if (alphaMode == 1) { // None
				alphaProp->SetBlendState(false);
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BF_SRC_ALPHA);
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BF_ONE_MINUS_SRC_ALPHA);
			}
			else if (alphaMode == 2) { // Standard
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BF_SRC_ALPHA);
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BF_ONE_MINUS_SRC_ALPHA);
			}
			else if (alphaMode == 3) { // Additive
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BF_ONE);
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BF_ONE);
			}
			else if (alphaMode == 4) { // Multiplicative
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BF_ZERO);
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BF_SRC_COLOR);
			}
			else { // Advanced
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BlendFunc(srcBlend));
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BlendFunc(destBlend));
			}
			alphaProp->SetTestFunc(NiAlphaProperty::TestFunc(TestMode));
			alphaProp->SetTriangleSortMode(NoSorter);
			alphaProp->SetTestThreshold(TestRef);
			alphaProp->SetTestState(AlphaTestEnable);

			if (IsSkyrim()) {
				alphaProp->SetFlags(4844);
				alphaProp->SetTestThreshold(128);
			}

			parent->AddProperty(alphaProp);
		}

		bool useDefaultShader = true;
		if (IsSkyrim() || IsFallout4())
		{
			if (BSLightingShaderPropertyRef texProp = new BSLightingShaderProperty())
			{
				extern const EnumLookupType BSShaderTypes[];
				int shaderType = StringToEnum(CustomShader, BSShaderTypes);
				if (shaderType < 100 || shaderType > 200)
					shaderType = 0;
				else
					shaderType -= 100;
				texProp->SetSkyrimShaderType((BSLightingShaderPropertyShaderType)shaderType);

				BSShaderTextureSetRef texset = new BSShaderTextureSet();
				texProp->SetTextureSet(texset);

				vector<string> textures;
				textures.resize(9);

				NiGeometryRef geom = DynamicCast<NiGeometry>(parent);

				SkyrimShaderPropertyFlags1 flags1 = (SkyrimShaderPropertyFlags1)(SLSF1_RECIEVE_SHADOWS | SLSF1_CAST_SHADOWS | SLSF1_OWN_EMIT | SLSF1_REMAPPABLE_TEXTURES | SLSF1_ZBUFFER_TEST);
				SkyrimShaderPropertyFlags2 flags2 = (SkyrimShaderPropertyFlags2)(SLSF2_ZBUFFER_WRITE);

				if (geom && geom->IsSkin()) {
					flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
					flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_ENVMAP_LIGHT_FADE | SLSF2_DOUBLE_SIDED);
				}

				if (shaderType == 5)
					flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_SOFT_LIGHTING);

				texProp->SetSpecularPower_Glossiness(80);
				texProp->SetSpecularColor(Color3(0.933f, 0.855f, 0.804f));
				texProp->SetSpecularStrength(1.0f);
				texProp->SetLightingEffect1(0.3f);
				texProp->SetLightingEffect2(2.0f);
				texProp->SetEnvironmentMapScale(1.0f);
				texProp->SetTextureClampMode(WRAP_S_WRAP_T);
				texProp->SetUnknownFloat1(FLT_MAX); // must be set to avoid hangs

				TSTR diffuseStr, normalStr, glowStr, dispStr, envStr, envMaskStr, backlightStr, parallaxStr;

				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;

					if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
						StdMat2 *m = (StdMat2*)mtl;
						if (m->GetMapState(C_BASE) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
								GetTexFullName(texMap, diffuseStr);
							}
						}

						if (m->GetMapState(C_NORMAL) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_NORMAL)) {
								if (texMap->ClassID() == GNORMAL_CLASS_ID) {
									texMap = texMap->GetSubTexmap(0);
									GetTexFullName(texMap, normalStr);
								}
								else {
									GetTexFullName(texMap, normalStr);
								}
							}
						}
						if (m->GetMapState(C_ENVMASK) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_ENVMASK)) {
								if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
									Texmap *envMap = texMap->GetSubTexmap(0);
									Texmap *envMaskMap = texMap->GetSubTexmap(1);
									GetTexFullName(envMap, envStr);
									GetTexFullName(envMaskMap, envMaskStr);
								}
								else {
									GetTexFullName(texMap, envMaskStr);
								}
							}
						}
						if (m->GetMapState(C_GLOW) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_GLOW)) {
								GetTexFullName(texMap, glowStr);
							}
						}
						if (m->GetMapState(C_HEIGHT) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_HEIGHT)) {
								GetTexFullName(texMap, dispStr);
							}
						}
						if (m->GetMapState(C_ENV) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_ENV)) {
								if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
									GetTexFullName(texMap->GetSubTexmap(0), envStr);
									GetTexFullName(texMap->GetSubTexmap(1), envMaskStr);
								}
								else {
									GetTexFullName(texMap, envStr);
								}
							}
						}
						if (m->GetMapState(C_BACKLIGHT) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_BACKLIGHT)) {
								GetTexFullName(texMap, backlightStr);
							}
						}
						if (m->GetMapState(C_PARALLAX) == 2) {
							if (Texmap *texMap = m->GetSubTexmap(C_PARALLAX)) {
								GetTexFullName(texMap, parallaxStr);
							}
						}
					}
					textures[0] = T2AString(mAppSettings->GetRelativeTexPath(diffuseStr, sTexPrefix));
					if (normalStr.isNull()) {
						char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
						_splitpath(textures[0].c_str(), drive, dir, fname, ext);
						strcat(fname, "_n");
						_makepath(path_buffer, drive, dir, fname, ext);
						textures[1] = path_buffer;
					}
					else
						textures[1] = T2AString(mAppSettings->GetRelativeTexPath(normalStr, sTexPrefix));
					if (!glowStr.isNull())
					{
						textures[2] = T2AString(mAppSettings->GetRelativeTexPath(glowStr, sTexPrefix));
						flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SKINNED | SLSF1_FACEGEN_RGB_TINT);
						flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_SOFT_LIGHTING);
					}
					if (!dispStr.isNull())
					{
						textures[3] = T2AString(mAppSettings->GetRelativeTexPath(dispStr, sTexPrefix));
					}
					if (!envStr.isNull())
					{
						textures[4] = T2AString(mAppSettings->GetRelativeTexPath(envStr, sTexPrefix));
						flags1 = (SkyrimShaderPropertyFlags1)(flags1 | Niflib::SLSF1_ENVIRONMENT_MAPPING);
						flags2 = (SkyrimShaderPropertyFlags2)(flags2 | Niflib::SLSF2_ENVMAP_LIGHT_FADE);
					}
					if (!envMaskStr.isNull())
					{
						textures[5] = T2AString(mAppSettings->GetRelativeTexPath(envMaskStr, sTexPrefix));
						flags1 = (SkyrimShaderPropertyFlags1)(flags1 | Niflib::SLSF1_ENVIRONMENT_MAPPING);
						flags2 = (SkyrimShaderPropertyFlags2)(flags2 | Niflib::SLSF2_ENVMAP_LIGHT_FADE);
					}
					if (!backlightStr.isNull())
					{
						textures[7] = T2AString(mAppSettings->GetRelativeTexPath(backlightStr, sTexPrefix));
						flags2 = (SkyrimShaderPropertyFlags2)(flags2 | Niflib::SLSF2_BACK_LIGHTING);
					}
					if (!parallaxStr.isNull())
					{
						textures[8] = T2AString(mAppSettings->GetRelativeTexPath(parallaxStr, sTexPrefix));
					}

					if (m->GetTwoSided()) flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_DOUBLE_SIDED);

					if (VertexColorsEnable)
						flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_VERTEX_COLORS);
					if (SpecularEnable)
						flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR);

					//NiTriBasedGeomRef shape(DynamicCast<NiTriBasedGeom>(parent));
					//if (shape != nullptr) {
					//	NiGeometryDataRef data = shape->GetData();
					//	if (data != nullptr) {
					//		if (data->GetNormals().size() > 0)
					//			flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_MODEL_SPACE_NORMALS);
					//	}
					//}
					texProp->SetShaderFlags1(flags1);
					texProp->SetShaderFlags2(flags2);

					TimeValue t = 0;
					texProp->SetSpecularPower_Glossiness(m->GetShininess(t) * 100.0f);
					//texProp->SetSpecularStrength(m->GetShinStr(t) < 1.0f ? 3.0f : 0.0f);
					//texProp->SetSpecularColor(TOCOLOR3(m->GetSpecular(t)));
					texProp->SetEmissiveColor(TOCOLOR3(emittance));
					texProp->SetAlpha(m->GetOpacity(t) / 100.0f);
					texProp->SetEnvironmentMapScale(envmapscale);
					//texProp->SetSpecularStrength(specularStrength);
					texProp->SetRefractionStrength(refractionStrength);
					texProp->SetLightingEffect1(lighteff1);
					texProp->SetLightingEffect2(lighteff2);

					texProp->SetEnvironmentMapScale(envmapscale);
					texProp->SetSkinTintColor(TOCOLOR3(skinTintColor));
					texProp->SetHairTintColor(TOCOLOR3(hairTintColor));
					texProp->SetMaxPasses(maxPasses);
					texProp->SetScale(parallaxScale);
					texProp->SetParallaxInnerLayerThickness(parallaxInnerThickness);
					texProp->SetParallaxRefractionScale(parallaxRefractionScale);
					texProp->SetParallaxInnerLayerTextureScale(TexCoord(parallaxInnerTextureScale.x, parallaxInnerTextureScale.y));
					texProp->SetEnvironmentMapScale(parallaxEnvmapStr);
					texProp->SetEyeCubemapScale(eyeCubemapScale);
					texProp->SetLeftEyeReflectionCenter(TOVECTOR3(leftEyeReflCenter));
					texProp->SetRightEyeReflectionCenter(TOVECTOR3(rightEyeReflCenter));

					//texProp->SetEmissiveColor(TOCOLOR(m->GetEmmis(t)));
				}
				
				texset->SetTextures(textures);

				// shader must be first, alpha can be second
				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				vector<NiPropertyRef> properties = parent->GetProperties();
				parent->ClearProperties();
				parent->AddProperty(prop);
				if (properties.size() > 0)
					parent->AddProperty(properties[0]);

				// New code to fix parenting of BSLighting branches to the geometry, specific to skyrim only
				// TODO: Add in checks for an existing shader or figure out what the other Bethesda specific node goes into this slot.
				//NiGeometryRef temp = DynamicCast<NiGeometry>(parent);
				//temp->SetBSProperty(0, prop);
			}
			useDefaultShader = false;
		}
		else if (IsFallout3())
		{
			NiObjectRef root;
			if (CustomShader != nullptr && _tcslen(CustomShader) != 0)
				root = Niflib::ObjectRegistry::CreateObject(T2A(CustomShader));

			if (root == nullptr) {
				root = new BSShaderPPLightingProperty();
			}

			if (BSShaderPPLightingPropertyRef texProp = DynamicCast<BSShaderPPLightingProperty>(root))
			{

				texProp->SetFlags(1);
				if (DynamicCast<Lighting30ShaderProperty>(root) != nullptr)
					texProp->SetShaderType(SHADER_LIGHTING30);
				else
					texProp->SetShaderType(SHADER_DEFAULT);

				BSShaderTextureSetRef texset = new BSShaderTextureSet();
				texProp->SetTextureSet(texset);

				vector<string> textures;
				textures.resize(6);

				TSTR diffuseStr, normalStr, glowStr, dispStr, envStr, envMaskStr;

				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;
					if (m->GetMapState(C_BASE) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
							GetTexFullName(texMap, diffuseStr);
						}
					}
					if (m->GetMapState(C_BUMP) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BUMP)) {
							if (texMap->ClassID() == GNORMAL_CLASS_ID) {
								texMap = texMap->GetSubTexmap(0);
								GetTexFullName(texMap, normalStr);
							}
							else {
								GetTexFullName(texMap, normalStr);
							}
						}
					}
					if (m->GetMapState(C_NORMAL) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_NORMAL)) {
							if (texMap->ClassID() == GNORMAL_CLASS_ID) {
								texMap = texMap->GetSubTexmap(0);
								GetTexFullName(texMap, normalStr);
							}
							else {
								GetTexFullName(texMap, normalStr);
							}
						}
					}
					if (m->GetMapState(C_ENVMASK) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_ENVMASK)) {
							if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
								Texmap *envMap = texMap->GetSubTexmap(0);
								Texmap *envMaskMap = texMap->GetSubTexmap(1);
								GetTexFullName(envMap, envStr);
								GetTexFullName(envMaskMap, envMaskStr);
							}
							else {
								GetTexFullName(texMap, envMaskStr);
							}
						}
					}
					if (m->GetMapState(C_GLOW) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_GLOW)) {
							GetTexFullName(texMap, glowStr);
						}
					}
					if (m->GetMapState(C_HEIGHT) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_HEIGHT)) {
							GetTexFullName(texMap, dispStr);
						}
					}
					if (m->GetMapState(C_ENV) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_ENV)) {
							if (texMap->ClassID() == Class_ID(MASK_CLASS_ID, 0)) {
								GetTexFullName(texMap->GetSubTexmap(0), envStr);
								GetTexFullName(texMap->GetSubTexmap(1), envMaskStr);
							}
							else {
								GetTexFullName(texMap, envStr);
							}
						}
					}
				}
				textures[0] = T2AString(mAppSettings->GetRelativeTexPath(diffuseStr, sTexPrefix));
				if (normalStr.isNull()) {
					char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
					_splitpath(textures[0].c_str(), drive, dir, fname, ext);
					strcat(fname, "_n");
					_makepath(path_buffer, drive, dir, fname, ext);
					textures[1] = path_buffer;
				}
				else
					textures[1] = T2AString(mAppSettings->GetRelativeTexPath(normalStr, sTexPrefix));
				if (!glowStr.isNull())
					textures[2] = T2AString(mAppSettings->GetRelativeTexPath(glowStr, sTexPrefix));
				if (!envMaskStr.isNull())
					textures[2] = T2AString(mAppSettings->GetRelativeTexPath(envMaskStr, sTexPrefix));
				if (!dispStr.isNull())
					textures[4] = T2AString(mAppSettings->GetRelativeTexPath(dispStr, sTexPrefix));
				if (!envStr.isNull())
					textures[5] = T2AString(mAppSettings->GetRelativeTexPath(envStr, sTexPrefix));

				BSShaderFlags shFlags = BSShaderFlags(SF_SPECULAR | SF_SHADOW_MAP | SF_SHADOW_FRUSTUM | SF_EMPTY | SF_ZBUFFER_TEST);
				if (!envStr.isNull() || !dispStr.isNull() || !envMaskStr.isNull())
					shFlags = BSShaderFlags(shFlags | SF_MULTIPLE_TEXTURES);
				texProp->SetShaderFlags(shFlags);

				texset->SetTextures(textures);

				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				parent->AddProperty(prop);
				useDefaultShader = false;
			}
			else if (BSShaderNoLightingPropertyRef texProp = StaticCast<BSShaderNoLightingProperty>(root))
			{
				texProp->SetFlags(1);
				texProp->SetShaderType(SHADER_NOLIGHTING);

				TSTR diffuseStr;
				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;
					if (m->GetMapState(C_BASE) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
							GetTexFullName(texMap, diffuseStr);
						}
					}
				}
				texProp->SetFileName(T2A(diffuseStr));
				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				parent->AddProperty(prop);
				useDefaultShader = false;
			}
			else if (WaterShaderPropertyRef texProp = StaticCast<WaterShaderProperty>(root))
			{
				texProp->SetFlags(1);
				texProp->SetShaderType(SHADER_WATER);

				TSTR diffuseStr;
				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;
					if (m->GetMapState(C_BASE) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
							GetTexFullName(texMap, diffuseStr);
						}
					}
				}
				//texProp->SetFileName( string(diffuseStr.data()) );
				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				parent->AddProperty(prop);
				useDefaultShader = false;
			}
			else if (SkyShaderPropertyRef texProp = StaticCast<SkyShaderProperty>(root))
			{
				texProp->SetFlags(1);
				texProp->SetShaderType(SHADER_SKY);

				TSTR diffuseStr;
				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;
					if (m->GetMapState(C_BASE) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
							GetTexFullName(texMap, diffuseStr);
						}
					}
				}
				texProp->SetFileName(T2A(diffuseStr));
				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				parent->AddProperty(prop);
				useDefaultShader = false;
			}
			else if (TallGrassShaderPropertyRef texProp = StaticCast<TallGrassShaderProperty>(root))
			{
				texProp->SetFlags(1);
				texProp->SetShaderType(SHADER_TALL_GRASS);

				TSTR diffuseStr;
				if (mtl && mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
					StdMat2 *m = (StdMat2*)mtl;
					if (m->GetMapState(C_BASE) == 2) {
						if (Texmap *texMap = m->GetSubTexmap(C_BASE)) {
							GetTexFullName(texMap, diffuseStr);
						}
					}
				}
				texProp->SetFileName(T2A(diffuseStr));
				NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
				parent->AddProperty(prop);
				useDefaultShader = false;
			}
		}
		if (useDefaultShader)
		{
			StdMat2 *m2 = nullptr;
			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) m2 = (StdMat2*)mtl;

			int ntex = mtl->NumSubTexmaps();
			if (ntex > 0)
			{
				int maxTexture = 7;
				NiTexturingPropertyRef texProp;
				for (int i = 0; i < ntex; ++i) {
					BitmapTex *bmTex = getTexture(mtl, i);
					if (!bmTex || (m2 && (m2->GetMapState(i)) != 2))
						continue;

					TexType textype = (TexType)i;

					// Fallout 3 only
					if (textype >= C_ENVMASK)
						continue;

					if (texProp == nullptr)
					{
						texProp = new NiTexturingProperty();
						texProp->SetApplyMode(Niflib::ApplyMode(ApplyMode));
						texProp->SetTextureCount(7);
					}
					if (Exporter::mNifVersionInt <= 0x14010003)
					{
						if (textype == C_DECAL0)
							texProp->SetTextureCount(9);
						else if (textype == C_DECAL1)
							texProp->SetTextureCount(10);
						else if (textype > C_DECAL1)
							continue;
					}
					else if (textype >= texProp->GetTextureCount())
					{
						texProp->SetTextureCount(textype + 1);
					}

					TexDesc td;
					NiTriBasedGeomRef shape(DynamicCast<NiTriBasedGeom>(parent));
					if (shape != nullptr && makeTextureDesc(bmTex, td, shape->GetData())) {
						if (textype == BUMP_MAP) {
							td.source->SetPixelLayout(PIX_LAY_BUMPMAP);
							texProp->SetLumaOffset(LumaOffset);
							texProp->SetLumaScale(LumaScale);

							Matrix22 m2;
							m2[0][0] = m2[1][1] = Magnitude;
							m2[0][1] = m2[1][0] = 0.0f;
							texProp->SetBumpMapMatrix(m2);
						}
						texProp->SetTexture(textype, td);

						// kludge for setting decal maps without messing up the file sizes
						if (Exporter::mNifVersionInt <= 0x14010003)
						{
							if (textype == C_DECAL0)
								texProp->SetTextureCount(7);
							else if (textype == C_DECAL1)
								texProp->SetTextureCount(8);
						}

						if (textype > maxTexture)
							maxTexture = textype;
					}
				}
				if (Exporter::mNifVersionInt < 0x14010003 && maxTexture > 8)
					texProp->SetTextureCount(8);
				else if (Exporter::mNifVersionInt > 0x14010003 && maxTexture > 12)
					texProp->SetTextureCount(12);
				if (texProp != nullptr)
				{
					parent->AddProperty(texProp);
				}
			}
		}
		return true;
	}
	return false;
}


bool Exporter::exportFO4Shader(NiAVObjectRef parent, Mtl* mtl)
{
	USES_CONVERSION;
	if (!mtl) return false;
	if (!IsFallout4()) return false;

	LPCTSTR sTexPrefix = mTexPrefix.c_str();
	RefTargetHandle ref = mtl->GetReference(2/*shader*/);
	if (!ref) return false;

	const Class_ID FO4SHADER_CLASS_ID(0x7a6bc2e7, 0x71106f41);

	Shader *s = nullptr;
	TSTR shaderByName; Class_ID shaderID = Class_ID(0, 0);
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat2 * smtl = (StdMat2*)mtl;
		if (smtl->SupportsShaders()) {
			if (s = smtl->GetShader()) {
				s->GetClassName(shaderByName);
				shaderID = s->ClassID();
			}
		}
	}
	if (shaderID != FO4SHADER_CLASS_ID)
		return false;

	NiAVObjectRef geom = DynamicCast<NiAVObject>(parent);
	BSTriShapeRef tri_shape = DynamicCast<BSTriShape>(parent);

	if (IBSShaderMaterialData *data = static_cast<IBSShaderMaterialData*>(s->GetInterface(I_BSSHADERDATA)))
	{
		if (data->HasBGSM())
		{
			BGSMFile* bgsm = data->GetBGSMData();

			BSLightingShaderPropertyRef texProp = new BSLightingShaderProperty();
			texProp->SetName(T2A(data->GetMaterialName()));

			TexClampMode clampMode = TexClampMode((bgsm->TileU ? 0x01 : 0) | (bgsm->TileV ? 0x02 : 0));
			texProp->SetTextureClampMode(clampMode);

			texProp->SetUvOffset(TexCoord(bgsm->UOffset, bgsm->VOffset));
			texProp->SetUvScale(TexCoord(bgsm->UScale, bgsm->VScale));


			if (bgsm->BlendState || bgsm->AlphaTest) {
				NiAlphaPropertyRef alphaProp = new NiAlphaProperty();
				alphaProp->SetBlendState(bgsm->BlendState);
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BlendFunc(bgsm->BlendFunc1));
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BlendFunc(bgsm->BlendFunc2));
				alphaProp->SetTestFunc(NiAlphaProperty::TF_GREATER);
				//alphaProp->SetTriangleSortMode(NoSorter);
				alphaProp->SetTestThreshold(bgsm->AlphaTestRef);
				alphaProp->SetTestState(bgsm->AlphaTest);
				parent->AddProperty(alphaProp);
			}


			BSShaderTextureSetRef texset = new BSShaderTextureSet();
			texProp->SetTextureSet(texset);

			vector<string> textures;
			textures.resize(10);


			SkyrimShaderPropertyFlags1 flags1 = SkyrimShaderPropertyFlags1(0);
			SkyrimShaderPropertyFlags2 flags2 = SkyrimShaderPropertyFlags2(0);

			if (geom && geom->IsSkin()) {
				flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
				flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_ENVMAP_LIGHT_FADE | SLSF2_DOUBLE_SIDED);
			}

			if (tri_shape && tri_shape->HasColors())
				flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_VERTEX_COLORS);

			if (bgsm->ZBufferWrite) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_ZBUFFER_WRITE);
			if (bgsm->ZBufferTest) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_ZBUFFER_TEST);
			//if (bgsm->ScreenSpaceReflections)  = SkyrimShaderPropertyFlags1(flags1 | SLSF21_reZBUFFER_TEST);
			//bool WetnessControlScreenSpaceReflections;
			if (bgsm->Decal) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_DECAL);
			if (bgsm->TwoSided) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_DOUBLE_SIDED);
			if (bgsm->Decal && !bgsm->DecalNoFade) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_DYNAMIC_DECAL);// ????
			//if (bgsm->NonOccluder) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_PARALLAX_OCCLUSION); 
			if (bgsm->Refraction)  flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_REFRACTION);
			if (bgsm->RefractionFalloff) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_USE_FALLOFF);
			texProp->SetRefractionStrength(bgsm->RefractionPower);
			if (bgsm->EnvironmentMapping) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_ENVIRONMENT_MAPPING);
			texProp->SetEnvironmentMapScale(bgsm->EnvironmentMappingMaskScale);
			if (bgsm->GrayscaleToPaletteColor) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_GREYSCALE_TO_PALETTECOLOR);

			textures[0] = T2AString(bgsm->DiffuseTexture);
			textures[1] = T2AString(bgsm->NormalTexture);
			textures[2] = T2AString(bgsm->GlowTexture);
			textures[3] = T2AString(bgsm->GreyscaleTexture);
			textures[4] = T2AString(bgsm->EnvmapTexture);
			textures[7] = T2AString(bgsm->SmoothSpecTexture);
			//textures[0] = T2AString(bgsm->InnerLayerTexture);
			//textures[0] = T2AString(bgsm->WrinklesTexture);
			//textures[0] = T2AString(bgsm->DisplacementTexture);

			//bool EnableEditorAlphaRef;
			if (bgsm->RimLighting) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_RIM_LIGHTING);
			//texProp->SetRimPower(bgsm->RimPower);			
			texProp->SetBacklightPower(bgsm->BackLightPower);
			if (bgsm->SubsurfaceLighting) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_SOFT_LIGHTING);
			texProp->SetSubsurfaceRolloff(bgsm->SubsurfaceLightingRolloff);
			texProp->SetUnknownFloat1(FLT_MAX); // must be set to avoid hangs
			if (bgsm->SpecularEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_SPECULAR);
			texProp->SetSpecularColor(bgsm->SpecularColor);
			texProp->SetSpecularStrength(bgsm->SpecularMult);
			texProp->SetSpecularPower_Glossiness(bgsm->Smoothness);
			texProp->SetFresnelPower(bgsm->FresnelPower);
			texProp->SetWetnessSpecScale(bgsm->WetnessControlSpecScale);
			texProp->SetWetnessSpecPower(bgsm->WetnessControlSpecPowerScale);
			texProp->SetWetnessMinVar(bgsm->WetnessControlSpecMinvar);
			texProp->SetWetnessEnvMapScale(bgsm->WetnessControlEnvMapScale);
			texProp->SetWetnessFresnelPower(bgsm->WetnessControlFresnelPower);
			texProp->SetWetnessMetalness(bgsm->WetnessControlMetalness);

			texProp->SetRootMaterial(T2AString(bgsm->RootMaterialPath));
			if (bgsm->AnisoLighting) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_ANISOTROPIC_LIGHTING);
			if (!bgsm->EmitEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_OWN_EMIT);  // ????
			texProp->SetEmissiveColor(bgsm->EmittanceColor);
			texProp->SetEmissiveMultiple(bgsm->EmittanceMult);
			if (bgsm->ModelSpaceNormals) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_MODEL_SPACE_NORMALS);
			if (bgsm->ExternalEmittance) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_EXTERNAL_EMITTANCE);
			if (bgsm->BackLighting) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_BACK_LIGHTING);
			if (bgsm->ReceiveShadows) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_RECIEVE_SHADOWS);
			if (bgsm->HideSecret) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_LOCALMAP_HIDE_SECRET);
			if (bgsm->CastShadows) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_CAST_SHADOWS);
			if (bgsm->DissolveFade) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_NO_FADE); // ????
			if (bgsm->AssumeShadowmask) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_ASSUME_SHADOWMASK);
			if (bgsm->Glowmap) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_GLOW_MAP);
			//if (bgsm->EnvironmentMappingWindow) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_EYE_ENVIRONMENT_MAPPING);
			if (bgsm->EnvironmentMappingEye) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_EYE_ENVIRONMENT_MAPPING);
			if (bgsm->Hair) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_HAIR_SOFT_LIGHTING);
			texProp->SetHairTintColor(bgsm->HairTintColor);
			if (bgsm->Tree) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_TREE_ANIM);
			if (bgsm->Facegen) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_FACEGEN_DETAIL_MAP);
			if (bgsm->SkinTint) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_FACEGEN_RGB_TINT);
			//if (bgsm->Tessellate;
			//float DisplacementTextureBias;
			//float DisplacementTextureScale;
			//float TessellationPNScale;
			//float TessellationBaseFactor;
			//float TessellationFadeDistance;
			texProp->SetGrayscaleToPaletteScale(bgsm->GrayscaleToPaletteScale);
			//bool SkewSpecularAlpha; // (header.Version >= 1)    
			
			texProp->SetShaderFlags1(flags1);
			texProp->SetShaderFlags2(flags2);

			//0=default 1=EnvMap, 2=Glow, 5=Skin, 6=Hair, 7=Unknown, 11=Ice/Parallax, 15=Eye.
			BSLightingShaderPropertyShaderType shaderType = Niflib::LSPST_DEFAULT;
			if (bgsm->EnvironmentMapping) shaderType = Niflib::LSPST_ENVIRONMENT_MAP;
			if (bgsm->Glowmap) shaderType = Niflib::LSPST_GLOW_SHADER;
			if (bgsm->SkinTint) shaderType = Niflib::LSPST_SKIN_TINT;
			if (bgsm->Hair) shaderType = Niflib::LSPST_HAIR_TINT;

			if (!bgsm->DisplacementTexture.empty()) shaderType = Niflib::LSPST_HEIGHTMAP; // ???
			if (bgsm->Facegen) shaderType = Niflib::LSPST_FACE_TINT; // ???

			texProp->SetSkyrimShaderType(shaderType);

			texset->SetTextures(textures);

			// shader must be first, alpha can be second
			NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
			vector<NiPropertyRef> properties = parent->GetProperties();
			parent->ClearProperties();
			parent->AddProperty(prop);
			if (properties.size() > 0)
				parent->AddProperty(properties[0]);

			return true;
		}
		else if (data->HasBGEM())
		{

			BGEMFile* bgem = data->GetBGEMData();

			BSEffectShaderPropertyRef texProp = new BSEffectShaderProperty();
			texProp->SetName(T2A(data->GetMaterialName()));

			TexClampMode clampMode = TexClampMode((bgem->TileU ? 0x01 : 0) | (bgem->TileV ? 0x02 : 0));
			texProp->SetTextureClampMode(clampMode);

			texProp->SetUvOffset(TexCoord(bgem->UOffset, bgem->VOffset));
			texProp->SetUvScale(TexCoord(bgem->UScale, bgem->VScale));


			if (bgem->BlendState || bgem->AlphaTest) {
				NiAlphaPropertyRef alphaProp = new NiAlphaProperty();
				alphaProp->SetBlendState(bgem->BlendState);
				alphaProp->SetSourceBlendFunc(NiAlphaProperty::BlendFunc(bgem->BlendFunc1));
				alphaProp->SetDestBlendFunc(NiAlphaProperty::BlendFunc(bgem->BlendFunc2));
				alphaProp->SetTestFunc(NiAlphaProperty::TF_GREATER);
				//alphaProp->SetTriangleSortMode(NoSorter);
				alphaProp->SetTestThreshold(bgem->AlphaTestRef);
				alphaProp->SetTestState(bgem->AlphaTest);
				parent->AddProperty(alphaProp);
			}


			vector<string> textures;
			textures.resize(10);

			SkyrimShaderPropertyFlags1 flags1 = SkyrimShaderPropertyFlags1(0);
			SkyrimShaderPropertyFlags2 flags2 = SkyrimShaderPropertyFlags2(0);

			if (geom && geom->IsSkin()) {
				flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
				flags2 = (SkyrimShaderPropertyFlags2)(flags2 | SLSF2_ENVMAP_LIGHT_FADE | SLSF2_DOUBLE_SIDED);
			}

			if (tri_shape && tri_shape->HasColors())
				flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_VERTEX_COLORS);

			if (bgem->ZBufferWrite) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_ZBUFFER_WRITE);
			if (bgem->ZBufferTest) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_ZBUFFER_TEST);
			//if (bgsm->ScreenSpaceReflections)  = SkyrimShaderPropertyFlags1(flags1 | SLSF21_reZBUFFER_TEST);
			//bool WetnessControlScreenSpaceReflections;
			if (bgem->Decal) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_DECAL);
			if (bgem->TwoSided) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_DOUBLE_SIDED);
			if (bgem->Decal && !bgem->DecalNoFade) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_DYNAMIC_DECAL);// ????
			if (bgem->Decal && bgem->DecalNoFade) flags2 = SkyrimShaderPropertyFlags2(flags2 | SLSF2_NO_FADE);
			
			//if (bgsm->NonOccluder) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_PARALLAX_OCCLUSION); 
			if (bgem->Refraction)  flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_REFRACTION);
			if (bgem->RefractionFalloff) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_RECIEVE_SHADOWS);
			//texProp->SetRefractionStrength(bgem->RefractionPower);
			if (bgem->EnvironmentMapping) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_ENVIRONMENT_MAPPING);
			texProp->SetEnvironmentMapScale(bgem->EnvironmentMappingMaskScale);
			if (bgem->GrayscaleToPaletteColor) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_GREYSCALE_TO_PALETTECOLOR);

			texProp->SetSourceTexture(T2AString(bgem->BaseTexture));
			texProp->SetNormalTexture(T2AString(bgem->NormalTexture));
			texProp->SetEnvMapTexture(T2AString(bgem->EnvmapTexture));
			texProp->SetGreyscaleTexture(T2AString(bgem->GrayscaleTexture));
			texProp->SetEnvMaskTexture(T2AString(bgem->EnvmapMaskTexture));

			if (bgem->NonOccluder) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_EXTERNAL_EMITTANCE);

			if (bgem->BloodEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | 0); // ???? never used
			if (bgem->EffectLightingEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF2_EFFECT_LIGHTING);
			if (bgem->FalloffEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_USE_FALLOFF);
			//if (bgem->FalloffEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_VERTEX_ALPHA);
			if (bgem->FalloffColorEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_FIRE_REFRACTION);
			if (bgem->GrayscaleToPaletteAlpha) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_GREYSCALE_TO_PALETTEALPHA);
			if (bgem->SoftEnabled) flags1 = SkyrimShaderPropertyFlags1(flags1 | SLSF1_SOFT_EFFECT);

			texProp->SetEmissiveColor(TOCOLOR4(bgem->BaseColor));
			texProp->SetEmissiveMultiple(bgem->BaseColorScale);
			texProp->SetFalloffStartAngle(bgem->FalloffStartAngle);
			texProp->SetFalloffStopAngle(bgem->FalloffStopAngle);
			texProp->SetFalloffStartOpacity(bgem->FalloffStartOpacity);
			texProp->SetFalloffStopOpacity(bgem->FalloffStopOpacity);
			texProp->SetSoftFalloffDepth(bgem->SoftDepth);
			//bgem->LightingInfluence
			//bgem->EnvmapMinLOD

			// shader must be first, alpha can be second
			NiPropertyRef prop = DynamicCast<NiProperty>(texProp);
			vector<NiPropertyRef> properties = parent->GetProperties();
			parent->ClearProperties();
			parent->AddProperty(prop);
			if (properties.size() > 0)
				parent->AddProperty(properties[0]);



			return true;
		}
	}

	return false;
}

void Exporter::updateSkinnedMaterial(NiGeometryRef shape)
{
	if (!shape) return;
	if (IsSkyrim() || IsFallout4()) {
		if ( BSEffectShaderPropertyRef effectShaderRef = shape->GetBSPropertyOfType<Niflib::BSEffectShaderProperty>() )
		{
			SkyrimShaderPropertyFlags1 flags1 = effectShaderRef->GetShaderFlags1();
			flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
			effectShaderRef->SetShaderFlags1(flags1);
		}
		if ( BSLightingShaderPropertyRef lightingShaderRef = shape->GetBSPropertyOfType<Niflib::BSLightingShaderProperty>() )
		{
			SkyrimShaderPropertyFlags1 flags1 = lightingShaderRef->GetShaderFlags1();
			flags1 = (SkyrimShaderPropertyFlags1)(flags1 | SLSF1_SPECULAR | SLSF1_SKINNED | SLSF1_VERTEX_ALPHA);
			lightingShaderRef->SetShaderFlags1(flags1);
		}
	}
}