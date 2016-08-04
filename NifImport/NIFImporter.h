/**********************************************************************
*<
FILE: NifImporter.h

DESCRIPTION:	NIF Importer

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __NIFIMPORTER_H__
#define __NIFIMPORTER_H__

#include "BaseImporter.h"
#include "IniSection.h"
#include <obj/NiParticleSystem.h>
#include <obj/NiPSysGravityModifier.h>
#include <obj/NiTimeController.h>
#include <AnimKey.h>
#include <obj/BSTriShape.h>
#include <obj/NiTextureProperty.h>
#include "../NifProps/iNifProps.h"

namespace Niflib
{
	class NiTextKeyExtraData;
}

// NIF Importer
class NifImporter : public BaseImporter, IFileResolver//, public IniFileSection
{
public:
	// Ini settings
	bool showTextures; // show textures in viewport
	bool removeIllegalFaces;
	bool removeDegenerateFaces;
	bool enableAutoSmooth;
	float autoSmoothAngle;
	bool flipUVTextures;
	bool enableSkinSupport;
	bool goToSkeletonBindPosition;
	bool enableCollision;
	int vertexColorMode;
	int useNiftoolsShader;
	bool mergeNonAccum;
	bool enableLights;
	bool enableCameras;
	bool importUPB;
	bool doNotReuseExistingBones;

	// Biped/Bones related settings
	bool importBones;
	tstring skeleton;
	float bipedHeight;
	tstring skeletonCheck;
	float bipedAngle;
	float bipedAnkleAttach;
	bool bipedTrianglePelvis;
	bool importSkeleton;
	bool useBiped;
	bool hasSkeleton;
	bool isBiped;
	bool removeUnusedImportedBones;
	bool forceRotation;
	bool browseForSkeleton;
	tstring defaultSkeletonName;
	float minBoneWidth;
	float maxBoneWidth;
	float boneWidthToLengthRatio;
	bool createNubsForBones;
	tstringlist dummyNodeMatches;
	bool convertBillboardsToDummyNodes;
	bool uncontrolledDummies;
	bool ignoreRootNode;
	bool autoDetect;
	tstringlist rotate90Degrees;
	bool supportPrnStrings;
	bool importBonesAsDummy;
	bool disableBSDismemberSkinModifier;


	// Animation related Settings
	bool replaceTCBRotationWithBezier;
	bool enableAnimations;
	bool requireMultipleKeys;
	bool applyOverallTransformToSkinAndBones;
	bool clearAnimation;
	bool addNoteTracks;
	bool addTimeTags;

	// Particle System settings
	bool	enableParticleSystems;

	// Collision settings
	float bhkScaleFactor;

	bool weldVertices;
	float weldVertexThresh;

	bool dummyBonesAsLines;
	int unnamedCounter;

	vector<Niflib::NiObjectRef> blocks;
	vector<Niflib::NiNodeRef> nodes;
	map<tstring, int> ctrlCount; // counter for number of controllers referencing a node

	typedef map<Niflib::NiObjectNETRef, INode*> NodeToNodeMap;
	typedef map<tstring, INode*, ltstr> NameToNodeMap;
	NodeToNodeMap nodeMap;
	NameToNodeMap nodeNameMap;


	NifImporter(const TCHAR *Name, ImpInterface *I, Interface *GI, BOOL SuppressPrompts);
	virtual void Initialize();
	virtual void ReadBlocks();
	void BuildNodes();
	void BuildNodes(Niflib::NiNodeRef object, vector<Niflib::NiNodeRef>& nodes);

	// Ini File related routines
	virtual void LoadIniSettings();
	virtual void SaveIniSettings();

	void ApplyAppSettings(bool initialize = false);

	bool HasSkeleton();
	bool IsBiped();
	void ImportBones(vector<Niflib::NiNodeRef>& bones);
	INode* ImportBonesWithRagdoll(vector<Niflib::NiNodeRef>& bones, INode* ragdollParent);
	void ImportBones(Niflib::NiNodeRef blocks, bool recurse = true);
	INode* ImportBonesWithRagdoll(Niflib::NiNodeRef blocks, INode* ragdollParent, bool recurse = true);
	void ImportBipeds(vector<Niflib::NiNodeRef>& blocks);
	void AlignBiped(IBipMaster* master, Niflib::NiNodeRef block);
	bool ImportMeshes(Niflib::NiNodeRef block);
	tstring FindImage(const tstring& name) const;
	tstring FindMaterial(const tstring& name) const;

	bool FindFile(const tstring& name, tstring& resolved_name) const override;
	bool FindFileByType(const tstring& name, FileType type, tstring& resolved_name) const override;
	bool GetRelativePath(tstring& name, FileType type) const override;

	void ImportBones(NiNodeRef node, bool recurse, INode * collisionParent);

	bool ImportUPB(INode *node, Niflib::NiNodeRef block);

	void SetTriangles(Mesh& mesh, const vector<Niflib::Triangle>& v);
	void SetNormals(Mesh& mesh, const vector<Niflib::Triangle>& t, const vector<Niflib::Vector3>& v);

	bool ImportMesh(Niflib::NiTriBasedGeomRef triBasedGeom);
	bool ImportBSTriShape(Niflib::BSTriShapeRef triBasedGeom);
	bool ImportMultipleGeometry(Niflib::NiNodeRef parent, vector<Niflib::NiTriBasedGeomRef>& glist);
	StdMat2 *ImportMaterialAndTextures(ImpNode *node, Niflib::NiAVObjectRef avObject);
	bool ImportMaterialAndTextures(ImpNode *node, vector<Niflib::NiTriBasedGeomRef>& glist);
	bool ImportNiftoolsShader(ImpNode *node, Niflib::NiAVObjectRef avObject, StdMat2 *m);
	bool ImportFO4Shader(ImpNode* node, NiAVObjectRef avObject, StdMat2* mtl);
	bool ImportTransform(ImpNode *node, Niflib::NiAVObjectRef avObject);
	bool ImportMesh(ImpNode *node, TriObject *o, Niflib::NiTriBasedGeomRef triGeom, Niflib::NiTriBasedGeomDataRef triGeomData, vector<Niflib::Triangle>& tris);
	bool ImportVertexColor(INode *tnode, TriObject *o, vector<Niflib::Triangle>& tris, vector<Niflib::Color4> cv, int cv_offset = 0);
	bool ImportSkin(ImpNode *node, Niflib::NiTriBasedGeomRef triGeom, int v_start = 0);
	bool ImportSkin(ImpNode *node, Niflib::BSTriShapeRef shape, int v_start = 0);

	bool ImportSpecialNodes();
	bool ImportParticleSystems(Niflib::NiNodeRef root);
	bool ImportParticleSystem(Niflib::NiParticleSystemRef particleSystem);
	SimpleObject* ImportPCloud(Niflib::NiParticleSystemRef particleSystem);
	INode* CreateGravityWarp(Niflib::NiPSysGravityModifierRef gravModifier, INode* parentNode);

	Texmap* CreateTexture(Niflib::TexDesc& desc);
	Texmap* CreateTexture(const Niflib::NiTexturePropertyRef& desc);
	Texmap* CreateTexture(const tstring& name, TexClampMode mode=WRAP_S_WRAP_T, TexCoord offset = TexCoord(0.0f, 0.0f), TexCoord tiling = TexCoord(1.0f, 1.0f));
	Texmap* CreateNormalBump(LPCTSTR name, Texmap* nmap);
	Texmap* CreateMask(LPCTSTR name, Texmap* nmap, Texmap* mask);
	Texmap* MakeAlpha(Texmap* tex);

	INode *CreateBone(const tstring& name, Point3 startPos, Point3 endPos, Point3 zAxis);
	INode *CreateHelper(const tstring& name, Point3 startPos);
	INode *CreateCamera(const tstring& name);

	INode *CreateImportRagdollNode(const TCHAR *name, Object *obj, INode* parent);
	
	INode *CreateImportNode(const TCHAR *name, Object *obj, INode* parent);

	bool ImportLights(Niflib::NiNodeRef node);
	bool ImportLights(vector<Niflib::NiLightRef> lights);

	// Primary Collision entry point.  Tests for bhk objects
	bool ImportCollision(Niflib::NiNodeRef node);
	INode* ImportCollisionWithRagdoll(Niflib::NiNodeRef node, INode* ragdollParent);

	void RegisterNode(Niflib::NiObjectNETRef node, INode* inode);
	INode *FindNode(Niflib::NiObjectNETRef node);

	INode *GetNode(Niflib::NiNodeRef node);
	INode *GetNode(Niflib::NiObjectNETRef obj);

	void RegisterNode(const string& name, INode* inode);
	void RegisterNode(const wstring& name, INode* inode);
	INode *GetNode(const string& name);
	INode *GetNode(const wstring& name);
	INode *GetNode(const TSTR& name);

	void SetNodeName(INode* inode, const LPCTSTR name);

	tstring GetSkeleton(AppSettings *appSettings);

	bool ShowDialog();
	virtual bool DoImport();

	// Animation Helpers
	bool ImportAnimation();
	void ClearAnimation();
	void ClearAnimation(INode *node);
	bool AddNoteTracks(float time, string name, string target, Ref<NiTextKeyExtraData> textKeyData, bool loop);

	bool GetControllerTimeRange(Control *c, Interval& range);
	bool GetControllerTimeRange(const NiTimeControllerRef& controller, Interval& range);
	bool GetControllerTimeRange(const list<NiTimeControllerRef>& controllers, Interval& range);

	bool ImportMtlAndTexAnimation( const list<NiTimeControllerRef>& controllers, Mtl* mat );
	bool ImportTextureAnimation( int subAnimID, KeyType keyType, vector<FloatKey> keys, Texmap* tex );
	bool ImportMaterialAnimation( int paramBlockID, int subAnimID, KeyType keyType, vector<FloatKey> keys, Mtl* mtl );
	Texmap* GetMaterialTextureSubMap(Mtl* mat, int id);

	void WeldVertices(Mesh& mesh);

	bool IsSkyrim() const;
	bool IsFallout3() const;
	bool IsFallout4() const;
	bool IsOblivion() const;
	bool IsMorrowind() const;

protected:
	NifImporter();
	INode * ImportCollision(NiNodeRef node, INode * parent);
};

#endif
