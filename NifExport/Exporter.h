#ifndef __EXPORTER_H__
#define __EXPORTER_H__
#include <gen/BSVertexData.h>
#include <obj/BSTriShape.h>
#include <obj/BSSubIndexTriShape.h>

namespace Niflib
{
	class NiTimeController;
	class NiControllerManager;
	class NiControllerSequence;

	class bhkConvexVerticesShape;
	class bhkNiTriStripsShape;
	class bhkPackedNiTriStripsShape;

}
using namespace Niflib;

class BitmapTex;
class AppSettings;
class StdMat2;


class Exporter
{

public:

	enum Result
	{
		Error,
		Ok,
		Abort,
		Skip
	};

	enum ExportType
	{
		NIF_WO_ANIM = 0,
		NIF_WO_KF = 1,
		SINGLE_KF_WITH_NIF = 2,
		SINGLE_KF_WO_NIF = 3,
		MULTI_KF_WITH_NIF = 4,
		MULTI_KF_WO_NIF = 5,
		NIF_WITH_MGR = 6,
	};
	enum AccumType
	{
		AT_NONE = 0,
		AT_X = 0x01,
		AT_Y = 0x02,
		AT_Z = 0x04,

		AT_XYZ = AT_X | AT_Y | AT_Z,
		AT_FORCE = 0x80000000,
	};

	// Callback for post-processing instructions
	struct NiCallback
	{
		NiCallback() {};
		virtual ~NiCallback() {};
		virtual Result execute() = 0;
	};

	class CancelExporterException {};


	/* exporter version */
	static int          mVersion;

	/* export options, static for dialog usage. */
	static bool         mUseRegistry;
	static bool         mSelectedOnly;
	static bool         mTriStrips;
	static bool         mExportHidden;
	static bool         mExportFurn;
	static bool         mExportLights;
	static tstring      mTexPrefix;
	static bool         mVertexColors;
	static float        mWeldThresh;
	static float        mNormThresh;
	static float        mUVWThresh;
	static bool         mExportCollision;
	static bool         mRemapIndices;
	static bool         mExportExtraNodes;
	static bool         mExportSkin;
	static bool         mUserPropBuffer;
	static bool         mFlattenHierarchy;
	static bool         mRemoveUnreferencedBones;
	static bool         mSortNodesToEnd;
	static tstring      mGameName;
	static tstring      mNifVersion;
	static int          mNifVersionInt;
	static int          mNifUserVersion;
	static int          mNifUserVersion2;
	static bool         mSkeletonOnly;
	static bool         mExportCameras;
	static bool         mGenerateBoneCollision;
	static bool         mExportTransforms;
	static float        mDefaultPriority;
	static ExportType   mExportType;
	static bool         mMultiplePartitions;
	static int          mBonesPerVertex;
	static int          mBonesPerPartition;
	static bool         mUseTimeTags;
	static bool         mAutoDetect;
	static bool         mAllowAccum;
	static tstring      mCreatorName;
	static bool         mCollapseTransforms;
	static bool         mZeroTransforms;
	static bool         mFixNormals;
	static bool         mTangentAndBinormalExtraData;
	static tstringlist  mRotate90Degrees;
	static bool         mSupportPrnStrings;
	static bool         mSuppressPrompts;
	static bool         mUseAlternateStripper;
	static float        bhkScaleFactor;
	static int          mTangentAndBinormalMethod;
	static bool         mStartNifskopeAfterStart;
	static tstring      mNifskopeDir;
	static bool         mTriPartStrips;
	static tstring      mRootType;
	static tstringlist  mRootTypes;
	static bool         mDebugEnabled;

	Exporter(Interface *i, AppSettings *appSettings);

	Result                    doExport(NiNodeRef &root, INode *node);

	/* config is written to the registry and the root node
	of the scene, so that it is restored when opening it
	again. creating a new document will make the exporter
	use the settings stored in the registry. */

	// writes config to root node
	static void                writeConfig(INode *node);
	// writes config to registry
	static void                writeConfig(Interface *i);
	// writes config to registry
	static void                writeKfConfig(Interface *i);

	// reads config from root node
	static void                readConfig(INode *node);
	// reads config from registry
	static void                readConfig(Interface *i);
	// reads config from registry
	static void                readKfConfig(Interface *i);

	static AppSettings * exportAppSettings();
	static AppSettings * importAppSettings(tstring fname);

public:
	typedef vector<unsigned short> TriStrip;
	typedef list<TriStrip> TriStrips;
	typedef vector<Triangle> Triangles;
	typedef vector<TexCoord> TexCoords;
	struct VertexGroup
	{
		int        idx;
		Point3      pt;
		Point3      norm;
		TexCoords  uvs;
		Color4     color;
	};

	struct FaceGroup
	{
		vector<VertexGroup> vgrp;
		vector<int>            vmap;
		vector<Vector3>      verts;
		vector<Vector3>      vnorms;
		Triangles            faces;
		vector<TexCoords>      uvs;
		std::map<int, int>  uvMapping; // first = Max index, second = Nif index
		vector<Color4>      vcolors;
		vector<int>         vidx;
		vector<int>         fidx;
		TriStrips           strips;
	};

	// maps face groups to material ID
	typedef std::map<int, FaceGroup>    FaceGroups;
	typedef std::set<INode*> INodeMap;
	typedef std::map<tstring, NiNodeRef>    NodeMap;
	typedef std::map<INode*, NiNodeRef>    NodeToNodeMap;
	typedef std::list<NiCallback*> CallbackList;
	typedef std::list<Ref<NiNode> > NodeList;

	Interface           *mI;
	NiNodeRef            mNiRoot;
	AppSettings         *mAppSettings;
	NodeMap              mNameMap;
	NodeToNodeMap        mNodeMap;
	INodeMap             mCollisionNodes;
	INodeMap             mHandledNodes;
	INode*               mSceneCollisionNode;
	CallbackList         mPostExportCallbacks;
	Box3                 mBoundingBox;
	NodeList             mAnimationRoots;
	Interval             globalRange;
    float                bhkAppScaleFactor;


	Result               exportNodes(NiNodeRef &root, INode *node);
	void                 updateRootTarget(Ref<bhkShape> ref, NiNodeRef ni_node);
	void                 updateRigidBody(bhkRigidBodyRef& ref);
	Result               exportCollision(NiNodeRef &root, INode *node);
	Result               scanForCollision(INode *node);
	bool                 isCollision(INode *node);
	bool                 isHandled(INode *node);
	bool                 markAsHandled(INode* node);
	Result               scanForAnimation(INode* node);
	Result               scanForIgnore(INode *node);

	/* utility functions */
	Mtl                  *getMaterial(INode *node, int subMtl);
	bool                 TMNegParity(const Matrix3 &m);
	void                 convertMatrix(Matrix33 &dst, const Matrix3 &src);
	void                 nodeTransform(Matrix33 &rot, Vector3 &trans, INode *node, TimeValue t, bool local = true);
	void                 nodeTransform(QuaternionXYZW &rot, Vector3 &trans, INode *node, TimeValue t, bool local = true);
	void                 objectTransform(Matrix33 &rot, Vector3 &trans, INode *node, TimeValue t, bool local = true);
	void                 objectTransform(QuaternionXYZW &rot, Vector3 &trans, INode *node, TimeValue t, bool local = true);
	Matrix3              getNodeTransform(INode *node, TimeValue t, bool local = true);
	Matrix3              getObjectTransform(INode *node, TimeValue t, bool local = true);
	Point3               getVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	bool                 equal(const Vector3 &a, const Point3 &b, float thresh);
	BitmapTex            *getTexture(Mtl *mtl);
	BitmapTex            *getTexture(Mtl *mtl, int i);
	void                 getTextureMatrix(Matrix3 &mat, Mtl *mtl);
	NiNodeRef            makeNode(NiNodeRef &parent, INode *maxNode, bool local = true);
	NiNodeRef            createNode();
	NiNodeRef            createNode(INode* maxNode, const string& name);
	bool                 findNode(const string& name, NiNodeRef& node);
	NiNodeRef            getNode(INode* maxNode);
	NiNodeRef            findNode(INode* maxNode);
	void                 setName(NiNodeRef node, const string& name);
	// returns true if the node contains collision objects
	bool                 isCollisionGroup(INode *maxNode, bool root = true);
	// returns true if the node contains meshes
	bool                 isMeshGroup(INode *maxNode, bool root = true);
	void                 CalcBoundingBox(INode *node, Box3& box, int all = 1);
	void                 CalcBoundingSphere(INode *node, Point3 center, float& radius, int all = 1);

	/* tristrips */
	void                 strippify(TriStrips &strips, vector<Vector3> &verts, vector<Vector3> &norms, const Triangles &tris);
	void                 strippify(FaceGroup &grp);
	NiTriStripsDataRef   makeTriStripsData(const TriStrips &strips);

	/* mesh export */
	// adds a vertex to a face group if it doesn't exist yet. returns new or previous index into the
	// vertex array.
	int                  addVertex(FaceGroup &grp, int face, int vi, Mesh *mesh, const Matrix3 &texm, vector<Color4>& vertColors);
	// creates face groups from faces with same sub material id
	bool                 splitMesh(INode *node, Mesh &, FaceGroups &grps, TimeValue t, vector<Color4>& vertColors, bool noSplit);
	// creates a NiTriStrips or NiTriShape hierarchy from a face group
    NiAVObjectRef        makeMesh(NiNodeRef &parent, INode*node, Mtl *mtl, FaceGroup &grp, bool exportStrips);
	// creates a BSTriShape hierarchy from a face group
	NiAVObjectRef        makeBSTriShape(NiNodeRef &parent, INode*node, Mtl *mtl, FaceGroup &grp);
	// splits mesh and converts it into nif blocks
	Result               exportMesh(NiNodeRef &parent, INode *node, TimeValue t);

	/* texture & material */
	// creates NiTexturingProperty + NiSourceTexture
	void                 makeTexture(NiAVObjectRef &parent, Mtl *mtl);
	bool                 makeTextureDesc(BitmapTex *bmTex, Niflib::TexDesc &td, NiTriBasedGeomDataRef shape);
	// creates a NiMaterialProperty
	void                 makeMaterial(NiAVObjectRef &parent, Mtl *mtl);
	bool                 exportNiftoolsShader(NiAVObjectRef parent, Mtl* mtl);
	bool                 exportFO4Shader(NiAVObjectRef parent, Mtl* mtl);
	void                 updateSkinnedMaterial(NiGeometryRef shape);

	/* havok & collision */
	int                  addVertex(vector<Vector3> &verts, vector<Vector3> &vnorms, const Point3 &pt, const Point3 &norm);
	void                 addFace(Triangles &tris, vector<Vector3> &verts, vector<Vector3> &vnorms, int face, const int vi[3], Mesh *mesh, Matrix3& tm);
	bool                 makeCollisionHierarchy(NiNodeRef &parent, INode *node, TimeValue t);

	/* creates a bhkRigidBody */
   void                  InitializeRigidBody(bhkRigidBodyRef body, INode *node);
	bhkRigidBodyRef      makeCollisionBody(INode *node);
	/* creates a collision shape from a node */
	bhkShapeRef          makeCollisionShape(INode *node, Matrix3& tm, bhkRigidBodyRef body, int mtlDefault);

	bhkShapeRef          makeTriStripsShape(INode *node, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeBoxShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeSphereShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeCapsuleShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeListShape(INode *node, Matrix3& tm, bhkRigidBodyRef body, int mtlDefault);
	bhkShapeRef          makebhkBoxShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makebhkSphereShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makebhkCapsuleShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeConvexShape(INode *node, Object* obj, Matrix3& tm, int mtlDefault);

	Ref<bhkConvexVerticesShape> makeConvexShape(Mesh& mesh, Matrix3& tm, int mtlDefault);
	Ref<bhkNiTriStripsShape>    makeTriStripsShape(Mesh& mesh, Matrix3& sm, int mtlDefault);
	Ref<bhkPackedNiTriStripsShape>    makePackedTriStripsShape(Mesh& mesh, Matrix3& sm, int mtlDefault, int layer, int colFilter);
	Ref<bhkCompressedMeshShape> makeCompressedMeshShape(Mesh& mesh, Matrix3& sm, int mtlDefault, int layer, int colFilter);
	

	bhkShapeRef          makeProxyShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyBoxShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxySphereShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyConvexShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyTriStripShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyPackedTriStripShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyCapsuleShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyOBBShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeProxyCMSD(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault);

	bhkShapeRef          makeModifierShape(INode *node, Object* obj, Modifier* mod, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModBoxShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModSphereShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModCapsuleShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModConvexShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModOBBShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModTriStripShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModPackedTriStripShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModPackedTriStripShape(INode *tnode, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModPackedTriStripShape(INodeTab &map, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModCMSD(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModCMSD(INode *tnode, Matrix3& tm, int mtlDefault);
	bhkShapeRef          makeModCMSD(INodeTab &map, Matrix3& tm, int mtlDefault);

	/* skin export */
	bool                 makeSkin(NiAVObjectRef shape, INode *node, FaceGroup &grp, TimeValue t);
	bool                 UpdateBoneWeights(INode* node, Exporter::FaceGroup& grp, vector<NiNodeRef>& boneList, vector<BSVertexData>& vertData, vector<BSSkinBoneTrans>& boneTrans);
	bool                 CreateSegmentation(INode* node, BSSubIndexTriShapeRef ref, FaceGroup& grp);
	bool                 exportSkin();

	/* animation export */
	Result               doAnimExport(Ref<NiControllerSequence> root);
	Result               doAnimExport(Ref<NiControllerManager> ctrl, INode *node);
	bool                 isNodeTracked(INode *node);
	bool                 isNodeKeyed(INode *node);
	Ref<NiTimeController> CreateController(INode *node, Interval range);
	static void          InitializeTimeController(Ref<NiTimeController> ctrl, NiNodeRef parent);
	Result               exportGeomMorpherControl(Modifier* mod, vector<Niflib::Vector3>& verts, vector<int>& baseVertIdx, NiObjectNETRef owner);
	void                 GetTimeRange(Control *c, Interval& range);
	Interval             GetTimeRange(INode *node);

	/* misc export */
	bool                 exportUPB(NiNodeRef &root, INode *node);
	bool                 removeUnreferencedBones(NiNodeRef node);
	void                 sortNodes(NiNodeRef node);
	NiNodeRef            exportBone(NiNodeRef parent, INode *node);
	Result               exportLight(NiNodeRef root, INode *node, GenLight* light);
	void                 getChildNodes(INode *node, vector<NiNodeRef>&list);
	bool                 exportPrn(NiNodeRef &root, INode *node);
	NiNodeRef            createAccumNode(NiNodeRef parent, INode *node);
	int                  countNodes(INode *node);
	bool                 isSkeletonRoot(INode *node);
	void                 ApplyAllSkinOffsets(NiAVObjectRef & root);
	void                 sortVector3(vector<Vector3>& vector);
	void                 sortVector4(vector<Vector4>& vector);

	//true havok
	Result exportRagdoll(NiNodeRef &parent, INode *node);

	bhkRigidBodyRef makeRigidBodyFromHavok(INode * node);

	/* specific versions */
	bool IsFallout4() const;
	bool IsFallout3() const;
	bool IsOblivion() const;
	bool IsMorrowind() const;
	bool IsSkyrim() const;

	/* Progress Bar stuff */
	enum ProgressSection
	{
		Geometry,
		Animation,
		Collision,
		Skin,
		ProgressSectionCount
	};
	int progressCounters[ProgressSectionCount];
	int progressMax[ProgressSectionCount];
	void ProgressUpdate(ProgressSection section, const TCHAR *s);
};

#endif 
