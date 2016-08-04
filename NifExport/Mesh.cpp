#include "pch.h"
#include "niutils.h"
#include "iskin.h"
#if VERSION_3DSMAX > ((5000<<16)+(15<<8)+0) // Version 5
#  include "MeshNormalSpec.h"
#endif
#ifdef USE_BIPED
#  include <cs/BipedApi.h>
#endif
#include <float.h>
#include "../NifProps/iNifProps.h"
#include "obj/NiBSBoneLODController.h"
#include "obj/NiTransformController.h"
#include "obj/bhkBlendController.h"
#include "obj/bhkBlendCollisionObject.h"
#include "obj/bhkSphereShape.h"
#include "obj/bhkCapsuleShape.h"
#include "obj/BSDismemberSkinInstance.h"
#include "ObjectRegistry.h"
#include <obj/BSTriShape.h>
#include <obj/BSSkin__Instance.h>
#include <obj/BSSkin__BoneData.h>
#include <obj/BSSubIndexTriShape.h>
#include <gen/SphereBV.h>

#include "../max_nif_plugin/NifProps/Havok/HavokHelper.h"

#pragma region Comparison Utilities
inline bool equals(float a, float b, float thresh) {
	return (fabsf(a - b) <= thresh);
}
inline bool equals(const Vector3 &a, const Point3 &b, float thresh) {
	return (fabsf(a.x - b.x) <= thresh) && (fabsf(a.y - b.y) <= thresh) && (fabsf(a.z - b.z) <= thresh);
}
inline bool equals(const Color4 &a, const Color4 &b, float thresh) {
	return (fabsf(a.r - b.r) <= thresh) && (fabsf(a.g - b.g) <= thresh) && (fabsf(a.b - b.b) <= thresh);
}
inline bool equals(const Point3 &a, const Point3 &b, float thresh) {
	return a.Equals(b, thresh);
}
inline bool equals(const Point3 &a, const Point3 &b) {
	return (a == b);
}

struct VertexCompare
{
	typedef Exporter::VertexGroup VertexGroup;
	typedef Exporter::FaceGroup FaceGroup;
	VertexCompare(FaceGroup& g, float pt, float nt, float vt)
		: grp(g), thresh(pt), normthresh(nt), vthresh(vt) {
		if (normthresh > thresh)
			normthresh = thresh;
		if (vthresh > thresh)
			vthresh = thresh;
	}
	inline bool operator()(const TexCoord& lhs, const TexCoord& rhs, float thresh) const {
		return compare(lhs, rhs, thresh) < 0;
	}
	inline bool operator()(const VertexGroup& lhs, const VertexGroup& rhs) const {
		return compare(lhs, rhs) < 0;
	}
	inline bool operator()(int lhs, const VertexGroup& rhs) const {
		return compare(lhs, rhs) < 0;
	}
	inline bool operator()(const VertexGroup& lhs, int rhs) const {
		return compare(lhs, rhs) < 0;
	}
	inline bool operator()(int lhs, int rhs) const {
		return compare(lhs, rhs) < 0;
	}

	inline int compare(float a, float b, float thresh) const {
		if (equals(a, b, thresh)) return 0;
		return std::less<float>()(a, b) ? -1 : 1;
	}
	inline int compare(const Point3 &a, const Point3 &b, float thresh) const {
		int d;
		if ((d = compare(a.x, b.x, thresh)) != 0) return d;
		if ((d = compare(a.y, b.y, thresh)) != 0) return d;
		if ((d = compare(a.z, b.z, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(const Vector3 &a, const Point3 &b, float thresh) const {
		int d;
		if ((d = compare(a.x, b.x, thresh)) != 0) return d;
		if ((d = compare(a.y, b.y, thresh)) != 0) return d;
		if ((d = compare(a.z, b.z, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(const Point3 &a, const Vector3 &b, float thresh) const {
		int d;
		if ((d = compare(a.x, b.x, thresh)) != 0) return d;
		if ((d = compare(a.y, b.y, thresh)) != 0) return d;
		if ((d = compare(a.z, b.z, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(const Vector3 &a, const Vector3 &b, float thresh) const {
		int d;
		if ((d = compare(a.x, b.x, thresh)) != 0) return d;
		if ((d = compare(a.y, b.y, thresh)) != 0) return d;
		if ((d = compare(a.z, b.z, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(const VertexGroup& lhs, const VertexGroup& rhs) const {
		int d;
		if ((d = compare(lhs.pt, rhs.pt, thresh)) != 0) return d;
		if ((d = compare(lhs.norm, rhs.norm, normthresh)) != 0) return d;
		if ((d = compare(lhs.color, rhs.color, vthresh)) != 0) return d;
		if ((d = lhs.uvs.size() - rhs.uvs.size()) != 0) return d;
		for (int i = 0; i < lhs.uvs.size(); ++i) {
			if ((d = compare(lhs.uvs[i], rhs.uvs[i], vthresh)) != 0) return d;
		}
		return 0;
	}
	inline int compare(const TexCoord& lhs, const TexCoord& rhs, float thresh) const {
		int d;
		if ((d = compare(lhs.u, rhs.u, thresh)) != 0) return d;
		if ((d = compare(lhs.v, rhs.v, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(const Color4 &a, const Color4 &b, float thresh) const {
		int d;
		if ((d = compare(a.r, b.r, thresh)) != 0) return d;
		if ((d = compare(a.g, b.g, thresh)) != 0) return d;
		if ((d = compare(a.b, b.b, thresh)) != 0) return d;
		return 0;
	}
	inline int compare(int lhs, const VertexGroup& rhs) const {
		return compare(grp.vgrp[lhs], rhs);
	}
	inline int compare(const VertexGroup& lhs, int rhs) const {
		return compare(lhs, grp.vgrp[rhs]);
	}
	inline int compare(int lhs, int rhs) const {
		return compare(grp.vgrp[lhs], grp.vgrp[rhs]);
	}
	FaceGroup& grp;
	float thresh, normthresh, vthresh;
};
typedef std::pair< std::vector<int>::iterator, std::vector<int>::iterator > IntRange;



namespace std
{
	template<>
	struct less<Triangle> : public binary_function<Triangle, Triangle, bool>
	{
		bool operator()(const Triangle& s1, const Triangle& s2) const {
			int d = 0;
			if (d == 0) d = (s1[0] - s2[0]);
			if (d == 0) d = (s1[1] - s2[1]);
			if (d == 0) d = (s1[2] - s2[2]);
			return d < 0;
		}
	};
	template<>
	struct less<SkinWeight> : public binary_function<SkinWeight, SkinWeight, bool>
	{
		bool operator()(const SkinWeight& lhs, const SkinWeight& rhs) {
			if (lhs.weight == 0.0) {
				if (rhs.weight == 0.0) {
					return rhs.index < lhs.index;
				}
				else {
					return true;
				}
				return false;
			}
			else if (rhs.weight == lhs.weight) {
				return lhs.index < rhs.index;
			}
			else {
				return rhs.weight < lhs.weight;
			}
		}
	};
}

inline Triangle& rotate(Triangle &t)
{
	if (t[1] < t[0] && t[1] < t[2]) {
		t.Set(t[1], t[2], t[0]);
	}
	else if (t[2] < t[0]) {
		t.Set(t[2], t[0], t[1]);
	}
	return t;
}
typedef std::map<Triangle, int> FaceMap;


#pragma endregion

Exporter::Result Exporter::exportMesh(NiNodeRef &ninode, INode *node, TimeValue t)
{
	USES_CONVERSION;
	char buffer[128];
	ObjectState os = node->EvalWorldState(t);

	bool local = !mFlattenHierarchy;

	TriObject *tri = (TriObject *)os.obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
	if (!tri)
		return Skip;

	Mesh *copymesh = NULL;
	Mesh *mesh = &tri->GetMesh();

	Matrix3 mtx(true), rtx(true);
	if (Exporter::mCollapseTransforms)
	{
		mtx = GetNodeLocalTM(node, t);
		mtx.NoTrans();
		Quat q(mtx);
		q.MakeMatrix(rtx);
		mesh = copymesh = new Mesh(*mesh);
		{
			int n = mesh->getNumVerts();
			for (unsigned int i = 0; i < n; ++i) {
				Point3& vert = mesh->getVert(i);
				vert = mtx * vert;
			}
			mesh->checkNormals(TRUE);
#if VERSION_3DSMAX > ((5000<<16)+(15<<8)+0) // Version 6+
			MeshNormalSpec *specNorms = mesh->GetSpecifiedNormals();
			if (NULL != specNorms) {
				specNorms->CheckNormals();
				for (unsigned int i = 0; i < specNorms->GetNumNormals(); ++i) {
					Point3& norm = specNorms->Normal(i);
					norm = (rtx * norm).Normalize();
				}
			}
#endif
		}
	}
	// Note that calling setVCDisplayData will clear things like normals so we set this up first
	vector<Color4> vertColors;
	if (mVertexColors)
	{
		bool hasvc = false;
		if (mesh->mapSupport(MAP_ALPHA))
		{
			mesh->setVCDisplayData(MAP_ALPHA);         int n = mesh->getNumVertCol();
			if (n > vertColors.size())
				vertColors.assign(n, Color4(1.0f, 1.0f, 1.0f, 1.0f));
			VertColor *vertCol = mesh->vertColArray;
			if (vertCol) {
				for (int i = 0; i < n; ++i) {
					VertColor c = vertCol[i];
					float a = (c.x + c.y + c.z) / 3.0f;
					vertColors[i].a = a;
					hasvc |= (a != 1.0f);
				}
			}
		}
		if (mesh->mapSupport(0))
		{
			mesh->setVCDisplayData(0);
			VertColor *vertCol = mesh->vertColArray;
			int n = mesh->getNumVertCol();
			if (n > vertColors.size())
				vertColors.assign(n, Color4(1.0f, 1.0f, 1.0f, 1.0f));
			if (vertCol) {
				for (int i = 0; i < n; ++i) {
					VertColor col = vertCol[i];
					vertColors[i] = Color4(col.x, col.y, col.z, vertColors[i].a);
					hasvc |= (col.x != 1.0f || col.y != 1.0f || col.z != 1.0f);
				}
			}
		}
		if (!hasvc) vertColors.clear();
	}

#if VERSION_3DSMAX <= ((5000<<16)+(15<<8)+0) // Version 5
	mesh->checkNormals(TRUE);
#else
	MeshNormalSpec *specNorms = mesh->GetSpecifiedNormals();
	if (NULL != specNorms) {
		specNorms->CheckNormals();
		if (specNorms->GetNumNormals() == 0)
			mesh->checkNormals(TRUE);
	}
	else {
		mesh->checkNormals(TRUE);
	}
#endif

	Result result = Ok;

	Modifier* geomMorpherMod = GetMorpherModifier(node);
	bool noSplit = FALSE;
	//	bool noSplit = (NULL != geomMorpherMod);

	while (1)
	{
		FaceGroups grps;
		if (!splitMesh(node, *mesh, grps, t, vertColors, noSplit))
		{
			result = Error;
			break;
		}
		bool exportStrips = mTriStrips && (Exporter::mNifVersionInt > VER_4_2_2_0);

		Matrix44 tm = Matrix44::IDENTITY;
		if (mExportExtraNodes || (mExportType != NIF_WO_ANIM && isNodeKeyed(node))) {
			tm = TOMATRIX4(getObjectTransform(node, t, false) * Inverse(getNodeTransform(node, t, false)));
		}
		else {
			Matrix33 rot; Vector3 trans;
			objectTransform(rot, trans, node, t, local);
			tm = Matrix44(trans, rot, 1.0f);
		}
		tm = TOMATRIX4(Inverse(mtx)) * tm;

		string basename = T2AHelper(buffer, node->NodeName(), _countof(buffer));
		LPCSTR format = (!basename.empty() && grps.size() > 1) ? "%s:%d" : "%s";

		int i = 1;
		FaceGroups::iterator grp;
		for (grp = grps.begin(); grp != grps.end(); ++grp, ++i)
		{
			string name = FormatString(format, basename.data(), i);
			NiAVObjectRef shape = makeMesh(ninode, node, getMaterial(node, grp->first), grp->second, exportStrips);
			if (shape == nullptr)
			{
				result = Error;
				break;
			}

			if (node->IsHidden())
				shape->SetVisibility(false);

			shape->SetName(name);
			shape->SetLocalTransform(tm);

			NiTriBasedGeomRef triShape = DynamicCast<NiTriBasedGeom>(shape);
			if (triShape != nullptr)
			{
				if (Exporter::mZeroTransforms) {
					triShape->ApplyTransforms();
				}

				if (makeSkin(shape, node, grp->second, t))
				{
					// fix material flags know that its known this has a skin
					if (IsSkyrim()) {
						updateSkinnedMaterial(shape);
					}
				}

				if (geomMorpherMod && triShape != nullptr) {
					vector<Vector3> verts = triShape->GetData()->GetVertices();
					exportGeomMorpherControl(geomMorpherMod, verts, triShape->GetData()->GetVertexIndices(), shape);
					triShape->GetData()->SetConsistencyFlags(CT_VOLATILE);
				}
			}
		}

		break;
	}

	if (tri != os.obj)
		tri->DeleteMe();

	if (copymesh)
		delete copymesh;

	return result;
}

NiAVObjectRef Exporter::makeMesh(NiNodeRef &parent, INode* node, Mtl *mtl, FaceGroup &grp, bool exportStrips)
{
	if (IsFallout4()) {
		return makeBSTriShape(parent, node, mtl, grp);
	}

	USES_CONVERSION;
	NiTriBasedGeomRef shape;
	NiTriBasedGeomDataRef data;

	//if (Exporter::mFixNormals) {
	//   FixNormals(grp.faces, grp.verts, grp.vnorms);
	//}

   TSTR shapeType, shapeDataType;
   if ( node->GetUserPropString(TEXT("ShapeType"), shapeType) )
      shape = DynamicCast<NiTriBasedGeom>(Niflib::ObjectRegistry::CreateObject(T2A(shapeType)));
   if (shape == NULL)
      shape = (exportStrips) ? (NiTriBasedGeom*)new NiTriStrips() : (NiTriBasedGeom*)new NiTriShape();
   if ( node->GetUserPropString(TEXT("ShapeDataType"), shapeDataType) )
      data = DynamicCast<NiTriBasedGeomData>(Niflib::ObjectRegistry::CreateObject(T2A(shapeDataType)));
   if (data == NULL)
      data = (exportStrips) ? (NiTriBasedGeomData*)new NiTriStripsData(grp.faces, !mUseAlternateStripper) : (NiTriBasedGeomData*)new NiTriShapeData(grp.faces);

	if (IsFallout3() || IsSkyrim())
		shape->SetFlags(14);

	data->SetVertices(grp.verts);
	data->SetNormals(grp.vnorms);
	data->SetVertexIndices(grp.vidx);
	data->SetUVSetMap(grp.uvMapping);

	int nUVs = grp.uvs.size();
	if (IsFallout3() || IsSkyrim())
		nUVs = min(1, nUVs);
	data->SetUVSetCount(nUVs);
	for (int i = 0; i < nUVs; ++i) {
		data->SetUVSet(i, grp.uvs[i]);
	}

   if (IsSkyrim() /*&& mVertexColors*/ ) // people seem to always want them in skyrim so give them what they want
   {
      if ( grp.vcolors.size() == 0 )
         grp.vcolors.resize(grp.verts.size(), Color4(1.0f,1.0f,1.0f,1.0f));
      data->SetVertexColors(grp.vcolors);
   }
	else if (mVertexColors && grp.vcolors.size() > 0)
	{
		bool allWhite = true;
		Color4 white(1.0f, 1.0f, 1.0f, 1.0f);
		for (int i = 0, n = grp.vcolors.size(); i < n; ++i) {
			if (white != grp.vcolors[i]) {
				allWhite = false;
				break;
			}
		}
		if (!allWhite)
			data->SetVertexColors(grp.vcolors);
	}

	data->SetConsistencyFlags(CT_STATIC);
	shape->SetData(data);

	if (Exporter::mTangentAndBinormalExtraData && (Exporter::mNifVersionInt > VER_4_2_2_0))
	{
		// enable traditional tangents and binormals for non-oblivion meshes
		if (!IsOblivion() && (Exporter::mNifVersionInt >= VER_10_0_1_0))
			data->SetTspaceFlag(0x01);
		shape->UpdateTangentSpace(Exporter::mTangentAndBinormalMethod);
	}

	parent->AddChild(DynamicCast<NiAVObject>(shape));

	NiAVObjectRef av(DynamicCast<NiAVObject>(shape));
	makeMaterial(av, mtl);
	shape->SetActiveMaterial(0);

	return shape;
}


static bool CalcTangentSpace(const Exporter::Triangles& tris, const vector<Vector3>& verts, const vector<Vector3>& norms, 
	const vector<TexCoord>& uvs, vector<Vector3>& tangents, vector<Vector3>& binormals)
{
	const int method = 0;
	if (verts.empty() || uvs.empty() || norms.empty()) return false;

	tangents.resize(verts.size());
	binormals.resize(verts.size());
	if (method == 0) // Nifskope algorithm
	{
		for (int t = 0; t < (int)tris.size(); t++) {
			const Triangle & tri = tris[t];

			int i1 = tri[0];
			int i2 = tri[1];
			int i3 = tri[2];

			const Vector3 v1 = verts[i1];
			const Vector3 v2 = verts[i2];
			const Vector3 v3 = verts[i3];

			const TexCoord w1 = uvs[i1];
			const TexCoord w2 = uvs[i2];
			const TexCoord w3 = uvs[i3];

			Vector3 v2v1 = v2 - v1;
			Vector3 v3v1 = v3 - v1;

			TexCoord w2w1(w2.u - w1.u, w2.v - w1.v);
			TexCoord w3w1(w3.u - w1.u, w3.v - w1.v);

			float r = w2w1.u * w3w1.v - w3w1.u * w2w1.v;

			r = (r >= 0.0f ? +1.0f : -1.0f);

			Vector3 sdir(
				(w3w1.v * v2v1.x - w2w1.v * v3v1.x) * r,
				(w3w1.v * v2v1.y - w2w1.v * v3v1.y) * r,
				(w3w1.v * v2v1.z - w2w1.v * v3v1.z) * r
				);

			Vector3 tdir(
				(w2w1.u * v3v1.x - w3w1.u * v2v1.x) * r,
				(w2w1.u * v3v1.y - w3w1.u * v2v1.y) * r,
				(w2w1.u * v3v1.z - w3w1.u * v2v1.z) * r
				);
			sdir = sdir.Normalized();
			tdir = tdir.Normalized();

			// no duplication, just smoothing
			for (int j = 0; j < 3; j++) {
				int i = tri[j];
				tangents[i] += tdir;
				binormals[i] += sdir;
			}
		}

		// for each vertex calculate tangent and binormal
		for (unsigned i = 0; i < verts.size(); i++) {
			const Vector3 & n = norms[i];

			Vector3 & t = tangents[i];
			Vector3 & b = binormals[i];

			if (t == Vector3() || b == Vector3()) {
				t.x = n.y;
				t.y = n.z;
				t.z = n.x;
				b = n.CrossProduct(t);
			}
			else {
				t = t.Normalized();
				t = (t - n * n.DotProduct(t));
				t = t.Normalized();

				b = b.Normalized();
				b = (b - n * n.DotProduct(b));
				b = (b - t * t.DotProduct(b));
				b = b.Normalized();
			}
		}
	}
	else if (method == 1) // Obsidian Algorithm
	{
		for (unsigned int faceNo = 0; faceNo < tris.size(); ++faceNo)   // for each face
		{
			const Triangle & t = tris[faceNo];  // get face
			int i0 = t[0], i1 = t[1], i2 = t[2];		// get vertex numbers
			Vector3 side_0 = verts[i0] - verts[i1];
			Vector3 side_1 = verts[i2] - verts[i1];

			float delta_U_0 = uvs[i0].u - uvs[i1].u;
			float delta_U_1 = uvs[i2].u - uvs[i1].u;
			float delta_V_0 = uvs[i0].v - uvs[i1].v;
			float delta_V_1 = uvs[i2].v - uvs[i1].v;

			Vector3 face_tangent = (side_0 * delta_V_1 - side_1 * delta_V_0).Normalized();
			Vector3 face_bi_tangent = (side_0 * delta_U_1 - side_1 * delta_U_0).Normalized();
			Vector3 face_normal = (side_0 ^ side_1).Normalized();

			// no duplication, just smoothing
			for (int j = 0; j <= 2; j++) {
				int i = t[j];
				tangents[i] += face_tangent;
				binormals[i] += face_bi_tangent;
			}
		}

		// for each.getPosition(), normalize the Tangent and Binormal
		for (unsigned int i = 0; i < verts.size(); i++) {
			binormals[i] = binormals[i].Normalized();
			tangents[i] = tangents[i].Normalized();
		}
	}
	return true;
}

static void UpdateAABBBox(const Vector3& v, Vector3& lows, Vector3& highs)
{
	if (v.x > highs.x) highs.x = v.x;
	else if (v.x < lows.x) lows.x = v.x;
	if (v.y > highs.y) highs.y = v.y;
	else if (v.y < lows.y) lows.y = v.y;
	if (v.z > highs.z) highs.z = v.z;
	else if (v.z < lows.z) lows.z = v.z;
}

static void CalcAxisAlignedBox(const vector<Vector3>& vertices, SphereBV& bounds)
{
	//--Calculate center & radius--//

	//Set lows and highs to first vertex
	Vector3 lows = vertices[0];
	Vector3 highs = vertices[0];

	//Iterate through the vertices, adjusting the stored values
	//if a vertex with lower or higher values is found
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		UpdateAABBBox(vertices[i], lows, highs);
	}

	//Now we know the extent of the shape, so the center will be the average
	//of the lows and highs
	bounds.center = (highs + lows) / 2.0f;

	//The radius will be the largest distance from the center
	Vector3 diff;
	float dist2(0.0f), maxdist2(0.0f);
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		const Vector3 & v = vertices[i];

		diff = bounds.center - v;
		dist2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (dist2 > maxdist2) maxdist2 = dist2;
	};
	bounds.radius = sqrt(maxdist2);
}

typedef std::pair<float, int> bone_weight;
struct bone_weight_sort : std::less<bone_weight>
{
	bool operator()(const bone_weight& _Left, const bone_weight& _Right) const {
		return std::less<bone_weight>()(_Right, _Left);
	}
};
struct sum_weights {
	float operator()(float total, bone_weight& w) const {
		return total + w.first;
	}
};

bool Exporter::UpdateBoneWeights(INode* node, Exporter::FaceGroup& grp, 
	vector<NiNodeRef>& boneList, vector<BSVertexData>& vertData, vector<BSSkinBoneTrans>& boneTrans)
{
	//get the skin modifier
	Modifier *mod = GetSkin(node);
	if (!mod) return false;

	ISkin *skin = (ISkin *)mod->GetInterface(I_SKIN);
	if (!skin) return false;

	ISkinContextData *skinData = skin->GetContextInterface(node);
	if (!skinData) return false;
	std::vector<Triangle> tris = grp.faces;
	// Get bone references (may not actually exist in proper structure at this time)
	//vector<bool> boneUsed;
	vector<int> boneVertCount;
	int totalBones = skin->GetNumBones();
	boneList.resize(totalBones);
	boneTrans.resize(totalBones);
	boneVertCount.resize(totalBones);
	//boneUsed.resize(totalBones);
	for (int i = 0; i < totalBones; ++i) {
		INode *bone = skin->GetBone(i);
		if (bone->GetParentNode()==NULL)
			throw runtime_error("Unparented IBONE!");
		boneList[i] = getNode(bone);
		if (boneList[i]->GetParent()==NULL)
			throw runtime_error("Unparented NBONE!"+boneList[i]->GetName());
		BSSkinBoneTrans& bt = boneTrans[i];
		Matrix44 tm = TOMATRIX4(bone->GetNodeTM(0));
		bt.SetTransform(tm);
	}

	vector<Vector3> lows(totalBones, Vector3(FLT_MAX, FLT_MAX, FLT_MAX));
	vector<Vector3> highs(totalBones, Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX));

	vector<Vector3>& verts = grp.verts;
	vector<int>& vidx = grp.vidx;
	vector<int>& vmap = grp.vmap;
	int nv = vidx.size();
	for (int i = 0; i < nv; ++i)
	{
		BSVertexData& vd = vertData[i];
		int vm_i = vmap[i];
		int vi = vidx[i];
		int nbones = skinData->GetNumAssignedBones(vi);
		if (nbones == 0) {
			vd.SetBoneWeight(0, 0, 1.0f);// assume first bone is most important bone
		} else  {
			vector<bone_weight> weights;
			for (int j = 0; j < nbones; ++j) {
				int boneIndex = skinData->GetAssignedBone(vi, j);
				float weight = skinData->GetBoneWeight(vi, j);
				weights.push_back(bone_weight(weight, boneIndex));
			}
			std::sort(weights.begin(), weights.end(), bone_weight_sort());
			nbones = std::min(4, nbones);
			vector<bone_weight>::iterator bwend = weights.begin();
			std::advance(bwend, nbones);
			float total = std::accumulate(weights.begin(), bwend, 0.0f, sum_weights());
			for (int j = 0; j < nbones; ++j) {
				int bi = weights[j].second;
				vd.SetBoneWeight(j, bi, weights[j].first / total);

				UpdateAABBBox(verts[vm_i], lows[bi], highs[bi]);
				//BSSkinBoneTrans& bt = boneTrans[bi];
				//bt.bsCenter += verts[vmap[i]]; // intermediate bounding sphere calc
				boneVertCount[bi]++;
			}
		}
	}
	for (int i = 0; i < totalBones; ++i) {
		BSSkinBoneTrans& bt = boneTrans[i];
		Vector3& btlows = lows[i];
		Vector3& bthighs = highs[i];
		int vc = boneVertCount[i];
		if (vc){
			bt.bounds.center = ((bthighs + btlows) / 2.0f);
			bt.bounds.radius = 0.0f;
		}else{
			bt.bounds.center = Vector3(0.0f,0.0f,0.0f);
			bt.bounds.radius = 0.0f;
		}
	}
	float radsq = 0.0f;
	for (int i = 0; i<nv; ++i) {
		BSVertexData& vd = vertData[i];
		
		for (int j = 0; j < 4; ++j) {
			int bi = vd.boneIndices[j];
			float bw = vd.boneWeights[j];
			if (bi == 0 && bw == 0.0f) break;

			BSSkinBoneTrans& bt = boneTrans[bi];
			float mag = (bt.bounds.center - verts[vmap[i]]).Magnitude();
			bt.bounds.radius = max(bt.bounds.radius, mag);
		}
	}
	Matrix3 wm = node->GetNodeTM(0);
	for (int i = 0; i < totalBones; ++i) {
		INode *bone = skin->GetBone(i);
		BSSkinBoneTrans& bt = boneTrans[i];
		Matrix44 btm(bt.GetTranslation(), bt.GetRotation(), bt.GetScale());
		bt.bounds.center = btm.Inverse() * bt.bounds.center;

		Matrix3 bm = bone->GetNodeTM(0);
		bm.Invert();
		Matrix44 tm = TOMATRIX4(wm * bm);
		
		bt.SetTransform(tm);
	}	// TODO: remove unused bones
	return true;
}

bool Exporter::CreateSegmentation(INode* node, BSSubIndexTriShapeRef shape, FaceGroup& grp)
{
	USES_CONVERSION;
	int ntris = grp.faces.size();
	std::vector<Triangle> tris;
	tris.reserve(ntris);
	BitArray used_array;
	used_array.SetSize(ntris);

	
	Modifier *subIndexModifier = GetBSSubIndexModifier(node);
	if (subIndexModifier == nullptr) return false;
	
	IBSSubIndexModifier *si_skin = static_cast<IBSSubIndexModifier *>(subIndexModifier->GetInterface(I_BSSUBINDEXSKINMODIFIER));
	if (si_skin == nullptr) return false;

	IBSSubIndexModifierData* bssimod = nullptr;
	Tab<IBSSubIndexModifierData*> modData = si_skin->GetModifierData();
	for (int i = 0; i < modData.Count(); ++i) {
		bssimod = modData[i];
		break;
	}
	if (bssimod == nullptr) return false;

	DWORD numSegments = bssimod->GetNumPartitions();
	vector<BSSITSSegment> segments(numSegments);
	BSSIMaterialSection section;
	section.materials.reserve(numSegments + numSegments*5); // just approximation
	int triangleOffset = 0;
	int segmentOffset = 0;
	for (int i = 0; i < numSegments; ++i)
	{
		auto& segment = segments[i];
		auto& partition = bssimod->GetPartition(i);
		auto& materials = partition.materials;

		segment.triangleOffset = triangleOffset;
		segment.triangleCount = 0;
		segment.materialHash = 0xFFFFFFFF;
		
		int parentSegment = segmentOffset++;
		BSSIMaterial seperator;
		seperator.materialHash = 0xFFFFFFFF;
		seperator.bodyPartIndex = i;
		section.materials.push_back(seperator);
		section.emptyMaterials.push_back(parentSegment);

		partition.id = i;

		// skip the empty materials
		bool emptyMaterial = (materials.size() == 1 && (materials[0].materialHash == 0xFFFFFFFF || materials[0].materialHash == 0));

		if (!emptyMaterial)
			segment.subIndexRecord.resize(materials.size());

		for (int j = 0; j < materials.size(); ++j)
		{
			auto& material = materials[j];

			BSSITSSubSegment *si_record = nullptr;
			if (!emptyMaterial)
			{
				si_record = &segment.subIndexRecord[j];
				si_record->triangleOffset = triangleOffset;
				si_record->segmentOffset = parentSegment;
				
				BSSIMaterial mat;
				mat.materialHash = material.materialHash;
				//mat.bodyPartIndex = j + 1 + (material.visible ? 100 : 0);
				mat.bodyPartIndex = material.id;
				for (int k = 0; k < material.data.size(); ++k)
					mat.extraData.push_back(material.data[k]);
				section.materials.push_back(mat);
				++segmentOffset;
			}

			auto& facesel = bssimod->GetFaceSel(i, j);
			for (int k = 0; k < ntris; ++k) {
				if (facesel[k]) {
					int fi = grp.fidx[k];
					used_array.Set(fi);
					tris.push_back(grp.faces[fi]);
					triangleOffset+=3;
					++segment.triangleCount;
					if(si_record) ++si_record->triangleCount;
				}
			}
		}	
	}
	section.numSegments = numSegments;
	section.numMaterials = section.materials.size();

	shape->SetMaterialSections(section);
	shape->SetSegments(segments);

	TSTR ssf_file = bssimod->GetSSF();
	shape->SetSSF(T2A(ssf_file));

	int diff = (grp.faces.size() - tris.size());
	if (diff != 0)
	{
		for (int i = 0; i < ntris; ++i)
		{
			if (!used_array[i])
				tris.push_back(grp.faces[i]);
		}
		diff = (grp.faces.size() - tris.size());
		if (diff != 0)
		{
			tstringstream error;
			error << "Skin Segmentation Error: Not all faces selected on '" << node->GetName() << "' there are " << diff << " unselected faces out of " << grp.faces.size() << "." << ends;
			throw runtime_error(T2AString(error.str()));
		}
	}
	grp.faces = tris;

	return true;
}

NiAVObjectRef Exporter::makeBSTriShape(NiNodeRef &parent, INode* node, Mtl *mtl, FaceGroup &grp)
{
	USES_CONVERSION;
	BSTriShapeRef shape;
	static TexCoords empty_uvs;

	//if (Exporter::mFixNormals) {
	//   FixNormals(grp.faces, grp.verts, grp.vnorms);
	//}

	TSTR shapeType, shapeDataType;
	if (node->GetUserPropString(TEXT("ShapeType"), shapeType))
		shape = DynamicCast<BSTriShape>(Niflib::ObjectRegistry::CreateObject(T2A(shapeType)));

	const Triangles& tris = grp.faces;
	const vector<Vector3>& verts = grp.verts;
	const vector<Vector3>& norms = grp.vnorms;
	const vector<Color4>& vcs = grp.vcolors;
	const vector<TexCoord>& uvs = grp.uvs.empty() ? empty_uvs : grp.uvs[0];
	bool has_uv, has_vc, has_normal, has_tangent, has_skin;
	has_uv = !uvs.empty();
	has_vc = mVertexColors && !vcs.empty();
	has_normal = !norms.empty();
	
	vector<Vector3> tangents, binormals;
	has_tangent = CalcTangentSpace(tris, verts, norms, uvs, tangents, binormals);
	
	vector<BSVertexData> vertexData;
	int nverts = verts.size();
	vertexData.resize(nverts);
	for (int i = 0; i < nverts; ++i)
	{
		BSVertexData& vd = vertexData[i];
		vd.SetVertex(verts[i]);
		if (has_normal) vd.SetNormal(norms[i]);
		if (has_uv) vd.SetUV(uvs[i]);
		if (has_vc) vd.SetVertexColor(vcs[i]);
		if (has_tangent) vd.SetTangent(tangents[i]);
		if (has_tangent) vd.SetBitangent(binormals[i]);
	}
	vector<NiNodeRef> boneList;
	vector<BSSkinBoneTrans> boneTrans;
	has_skin = UpdateBoneWeights(node, grp, boneList, vertexData, boneTrans);
	if (has_skin) {
		BSSkin__InstanceRef skin_instance = new BSSkin__Instance();
		skin_instance->SetBones(boneList);

		BSSkin__BoneDataRef skin_data = new BSSkin__BoneData();
		skin_data->SetBoneTransforms(boneTrans);
		skin_instance->SetBoneData(skin_data);
		
		shape = new BSSubIndexTriShape;
		shape->SetSkin(skin_instance);

		// reorder triangles
		CreateSegmentation(node, shape, grp);
	}
	if (shape == nullptr) {
		shape = new BSTriShape;
	}

	SphereBV bounds;
	CalcAxisAlignedBox(grp.verts, bounds);
	shape->SetBounds(bounds);

	shape->SetFlags(14);
	shape->SetVertexFlags(has_uv, has_vc, has_normal, has_tangent, has_skin, false);
	shape->SetVertexData(vertexData);
	shape->SetTriangles(tris);
	parent->AddChild(DynamicCast<NiAVObject>(shape));


	NiAVObjectRef av(DynamicCast<NiAVObject>(shape));
	makeMaterial(av, mtl);
	//shape->SetActiveMaterial(0);

	return shape;
}

int Exporter::addVertex(FaceGroup &grp, int face, int vi, Mesh *mesh, const Matrix3 &texm, vector<Color4>& vertColors)
{
	VertexGroup vg;
	int vidx;
	vidx = vg.idx = mesh->faces[face].v[vi];
	vg.pt = mesh->verts[vidx];
#if VERSION_3DSMAX <= ((5000<<16)+(15<<8)+0) // Version 5
	vg.norm = getVertexNormal(mesh, face, mesh->getRVertPtr(vidx));
#else
	MeshNormalSpec *specNorms = mesh->GetSpecifiedNormals();
	if (NULL != specNorms && specNorms->GetNumNormals() != 0)
		vg.norm = specNorms->GetNormal(face, vi);
	else
		vg.norm = getVertexNormal(mesh, face, mesh->getRVertPtr(vidx));
#endif

	int nmaps = grp.uvMapping.size();
	map<int, int>::iterator UVSetIter;
	vg.uvs.resize(nmaps > 0 ? nmaps : 1);
	if (nmaps > 0) {
		for (UVSetIter = grp.uvMapping.begin(); UVSetIter != grp.uvMapping.end(); UVSetIter++)
		{
			int maxUVIdx = (*UVSetIter).first;
			int nifUVIdx = (*UVSetIter).second;
			TexCoord& uvs = vg.uvs[nifUVIdx];
			UVVert *uv = mesh->mapVerts(maxUVIdx);
			TVFace *tv = mesh->mapFaces(maxUVIdx);
			if (uv && tv) {
				Point3 uvw = uv[tv[face].t[vi]] * texm;
				uvs.u = uvw[0];
				uvs.v = uvw[1] + 1.0f;
			}
		}
	}
	else {
		if (mesh->tVerts && mesh->tvFace) {
			Point3 uvw = mesh->tVerts[mesh->tvFace[face].t[vi]] * texm;
			vg.uvs[0].u = uvw[0];
			vg.uvs[0].v = uvw[1] + 1.0f;
		}
	}
	vg.color = Color4(1.0f, 1.0f, 1.0f);
	if (mVertexColors && !vertColors.empty()) {
		TVFace *vcFace = mesh->vcFaceData ? mesh->vcFaceData : mesh->vcFace;
		if (vcFace) {
			int vidx = vcFace[face].t[vi];
			vg.color = vertColors[vidx];
		}
	}

	VertexCompare vc(grp, Exporter::mWeldThresh, Exporter::mNormThresh, Exporter::mUVWThresh);
	int n = grp.verts.size();
#if 0
	IntRange range = std::equal_range(grp.vmap.begin(), grp.vmap.end(), vg, vc);
	if (range.first != range.second) {
		return (*range.first);
	}
	grp.vmap.insert(range.first, n);
#else
	for (int i = 0; i < grp.vgrp.size(); i++) {
		if (vc.compare(vg, i) == 0)
			return i;
	}
	grp.vmap.push_back(n);
#endif
	grp.vidx.push_back(vidx);
	grp.vgrp.push_back(vg);
	grp.verts.push_back(TOVECTOR3(vg.pt));
	grp.vnorms.push_back(TOVECTOR3(vg.norm));
	for (int i = 0; i < grp.uvs.size(); ++i) {
		TexCoords& uvs = grp.uvs[i];
		uvs.push_back(vg.uvs[i]);
	}
	grp.vcolors.push_back(vg.color);
	return n;
}

bool Exporter::splitMesh(INode *node, Mesh& mesh, FaceGroups &grps, TimeValue t, vector<Color4>& vertColors, bool noSplit)
{
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm = node->GetObjTMAfterWSM(t);

	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	int vi[3];
	if (TMNegParity(tm)) {
		vi[0] = 2; vi[1] = 1; vi[2] = 0;
	}
	else {
		vi[0] = 0; vi[1] = 1; vi[2] = 2;
	}

	Matrix3 flip;
	flip.IdentityMatrix();
	flip.Scale(Point3(1, -1, 1));

	int nv = mesh.getNumVerts();
	int nf = mesh.getNumFaces();

	if (noSplit)
	{
		int nv = mesh.getNumVerts();
		int nf = mesh.getNumFaces();
		// Dont split the mesh at all.  For debugging purposes.
		FaceGroup& grp = grps[0];
		grp.vidx.resize(nv, -1);
		grp.verts.resize(nv);
		grp.faces.resize(nf);
		grp.uvs.resize(nv);
		grp.vnorms.resize(nv);
		grp.fidx.resize(nf);

		Matrix3 texm;
		getTextureMatrix(texm, getMaterial(node, 0));
		texm *= flip;

		for (int face = 0; face < nf; ++face) {
			grp.fidx[face] = face;
			for (int vi = 0; vi < 3; ++vi) {
				int idx = mesh.faces[face].getVert(vi);
				grp.faces[face][vi] = idx;

				// Calculate normal
				Point3 norm;
#if VERSION_3DSMAX <= ((5000<<16)+(15<<8)+0) // Version 5
				norm = getVertexNormal(&mesh, face, mesh.getRVertPtr(idx));
#else
				MeshNormalSpec *specNorms = mesh.GetSpecifiedNormals();
				if (NULL != specNorms && specNorms->GetNumNormals() != 0)
					norm = specNorms->GetNormal(face, vi);
				else
					norm = getVertexNormal(&mesh, face, mesh.getRVertPtr(idx));
#endif
				Point3 uv;
				if (mesh.tVerts && mesh.tvFace) {
					uv = mesh.tVerts[mesh.tvFace[face].t[vi]] * texm;
					uv.y += 1.0f;
				}

				if (grp.vidx[idx] == idx) {
					ASSERT(grp.verts[idx] == TOVECTOR3(mesh.getVert(idx)));
					//ASSERT(vg.norm == norm);
					//Point3 uv = mesh.getTVert(idx);
					//if (mesh.getNumTVerts() > 0)
					//{
					//	ASSERT(grp.uvs[idx].u == uv.x && grp.uvs[idx].v == uv.y);
					//}
				}
				else {
					grp.vidx[idx] = idx;
					grp.verts[idx] = TOVECTOR3(mesh.getVert(idx));
					//grp.uvs[idx].u = uv.x;
					//grp.uvs[idx].v = uv.y;
					grp.vnorms[idx] = TOVECTOR3(norm);
				}
			}
		}
		for (int i = 0; i < nv; ++i) {
			ASSERT(grp.vidx[i] != -1);
		}
	}
	else
	{
		int face, numSubMtls = nodeMtl ? nodeMtl->NumSubMtls() : 0;
		for (face = 0; face < mesh.getNumFaces(); face++)
		{
			int mtlID = (numSubMtls != 0) ? (mesh.faces[face].getMatID() % numSubMtls) : 0;
			Mtl *mtl = getMaterial(node, mtlID);
			Matrix3 texm;
			getTextureMatrix(texm, mtl);
			texm *= flip;

			FaceGroup& grp = grps[mtlID];

			if (grp.uvMapping.size() == 0) // Only needs to be done once per face group
			{
				int nmaps = 0;
				int nmapsStart = max(1, mesh.getNumMaps() - (mesh.mapSupport(0) ? 1 : 0)); // Omit vertex color map.
				for (int ii = 1; ii <= nmapsStart; ii++) // Winnow out the unsupported maps.
				{
					if (!mesh.mapSupport(ii)) continue;
					grp.uvMapping[ii] = nmaps++;
				}
				grp.uvs.resize(nmaps == 0 ? 1 : nmaps);
			}
			if (nv > int(grp.verts.capacity()))
			{
				grp.vgrp.reserve(nv);
				grp.verts.reserve(nv);
				grp.vnorms.reserve(nv);
				for (int i = 0; i < grp.uvs.size(); ++i)
					grp.uvs[i].reserve(nv);
				grp.vcolors.reserve(nv);
				grp.vidx.reserve(nv);
			}
			if (nf > int(grp.faces.capacity()))
			{
				grp.faces.reserve(nf);
				grp.fidx.reserve(nf);
			}

			Triangle tri;
			for (int i = 0; i < 3; i++)
				tri[i] = addVertex(grp, face, vi[i], &mesh, texm, vertColors);
			grp.faces.push_back(tri);

			if (grp.fidx.size() < nf)
				grp.fidx.resize(nf, -1);
			grp.fidx[face] = grp.faces.size() - 1;
		}
	}

	return true;
}

// Callback interface to register a Skin after entire structure is built due to contraints
//   in the nif library
struct SkinInstance : public Exporter::NiCallback
{
	typedef vector<SkinWeight> SkinWeightList;
	typedef vector<SkinWeightList> BoneWeightList;

	Exporter *owner;
	// Common Data
	NiTriBasedGeomRef shape;
	vector<NiNodeRef> boneList;
	NiObject * (*SkinInstConstructor)();

	// Bone to weight map
	BoneWeightList boneWeights;

	// Fallout 3 dismemberment
	vector<BodyPartList> partitions;
	vector<int> facePartList;

	Matrix3 bone_init_tm;
	Matrix3 node_init_tm;

	SkinInstance(Exporter *Owner) : owner(Owner), SkinInstConstructor(NULL) {}
	virtual ~SkinInstance() {}
	virtual Exporter::Result execute();
};

bool Exporter::makeSkin(NiAVObjectRef shape, INode *node, FaceGroup &grp, TimeValue t)
{
	if (!mExportSkin)
		return false;

	if (grp.verts.empty())
		return false;

	//get the skin modifier
	Modifier *mod = GetSkin(node);
	if (!mod)
		return false;

	ISkin *skin = (ISkin *)mod->GetInterface(I_SKIN);
	if (!skin)
		return false;

	ISkinContextData *skinData = skin->GetContextInterface(node);
	if (!skinData)
		return false;

	if (grp.strips.empty())
		strippify(grp);

	// Create new call back to finish export
	SkinInstance* si = new SkinInstance(this);
	mPostExportCallbacks.push_back(si);

	skin->GetSkinInitTM(node, si->bone_init_tm, false);
	skin->GetSkinInitTM(node, si->node_init_tm, true);

	si->shape = shape;

	// Get bone references (may not actually exist in proper structure at this time)
	int totalBones = skin->GetNumBones();
	si->boneWeights.resize(totalBones);
	si->boneList.resize(totalBones);
	for (int i = 0; i < totalBones; ++i) {
		si->boneList[i] = getNode(skin->GetBone(i));
	}

	vector<int>& vidx = grp.vidx;
	int nv = vidx.size();
	for (int i = 0; i < nv; ++i)
	{
		int vi = vidx[i];
		int nbones = skinData->GetNumAssignedBones(vi);
		for (int j = 0; j < nbones; ++j)
		{
			SkinWeight sw;
			sw.index = i;
			sw.weight = skinData->GetBoneWeight(vi, j);
			int boneIndex = skinData->GetAssignedBone(vi, j);

			SkinInstance::SkinWeightList& weights = si->boneWeights[boneIndex];
			weights.push_back(sw);
		}
	}

	// remove unused bones
	vector<NiNodeRef>::iterator bitr = si->boneList.begin();
	SkinInstance::BoneWeightList::iterator switr = si->boneWeights.begin();
	for (int i = 0; i < totalBones; ++i) {
		vector<SkinWeight> &weights = (*switr);
		if (weights.empty())
		{
			bitr = si->boneList.erase(bitr);
			switr = si->boneWeights.erase(switr);
		}
		else
		{
			++bitr, ++switr;
		}
	}

	// Check for dismemberment
	if (IsFallout3() || IsSkyrim()) {
		Modifier *dismemberSkinMod = GetBSDismemberSkin(node);
		if (dismemberSkinMod)
		{
			NiTriBasedGeomRef triShape = DynamicCast<NiTriBasedGeom>(shape);
			if (IBSDismemberSkinModifier *disSkin = (IBSDismemberSkinModifier *)dismemberSkinMod->GetInterface(I_BSDISMEMBERSKINMODIFIER)) {
				Tab<IBSDismemberSkinModifierData*> modData = disSkin->GetModifierData();
				if (modData.Count() >= 1) {
					IBSDismemberSkinModifierData* bsdsmd = modData[0];
					si->SkinInstConstructor = BSDismemberSkinInstance::Create;
					Tab<BSDSPartitionData> &flags = bsdsmd->GetPartitionFlags();
					GenericNamedSelSetList &fselSet = bsdsmd->GetFaceSelList();

					FaceMap fmap;
					NiTriBasedGeomDataRef data = DynamicCast<NiTriBasedGeomData>(triShape->GetData());
					vector<Triangle> tris = data->GetTriangles();
					for (int i = 0; i < tris.size(); ++i) {
						Triangle tri = tris[i];
						fmap[rotate(tri)] = i;
					}
					// Build up list of partitions and face to partition map
					si->partitions.resize(flags.Count());
					si->facePartList.resize(grp.faces.size(), -1);
					for (int i = 0; i < flags.Count(); ++i) {
						BodyPartList& bp = si->partitions[i];
						bp.bodyPart = (BSDismemberBodyPartType)flags[i].bodyPart;
						bp.partFlag = (BSPartFlag)(flags[i].partFlag | PF_START_NET_BONESET);

						BitArray& fSelect = fselSet[i];
						for (int j = 0; j < fSelect.GetSize(); ++j) {
							if (fSelect[j]) {
								Triangle tri = grp.faces[grp.fidx[j]];
								FaceMap::iterator fitr = fmap.find(rotate(tri));
								if (fitr != fmap.end())
									si->facePartList[(*fitr).second] = i;
							}
						}
					}
				}
			}
		}
	}


	return true;
}

Exporter::Result SkinInstance::execute()
{
	if (boneList.size()<=0)
		throw runtime_error("NO BONES!");
	shape->BindSkinWith(boneList, SkinInstConstructor);
	unsigned int bone = 0;
	for (BoneWeightList::iterator bitr = boneWeights.begin(); bitr != boneWeights.end(); ++bitr, ++bone) {
		shape->SetBoneWeights(bone, (*bitr));
	}
	int* faceMap = NULL;
	if (partitions.size() > 0) {
		BSDismemberSkinInstanceRef dismem = DynamicCast<BSDismemberSkinInstance>(shape->GetSkinInstance());
		if (dismem != NULL)
			dismem->SetPartitions(partitions);
		faceMap = &facePartList[0];
	}
	if (Exporter::mNifVersionInt > VER_4_0_0_2)
	{
		if (Exporter::mMultiplePartitions)
			shape->GenHardwareSkinInfo(Exporter::mBonesPerPartition, Exporter::mBonesPerVertex, Exporter::mTriPartStrips, faceMap);
		else
			shape->GenHardwareSkinInfo(0, 0, Exporter::mTriPartStrips);
	}

	return Exporter::Ok;
}

static void FillBoneController(Exporter* exporter, NiBSBoneLODControllerRef boneCtrl, INode *node)
{
	USES_CONVERSION;
	char buffer[128];
	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		INode * child = node->GetChildNode(i);
		FillBoneController(exporter, boneCtrl, child);

		TSTR upb;
		child->GetUserPropBuffer(upb);
		if (!upb.isNull())
		{
			// Check for bonelod and add bones to bone controller
			string ubpdata = T2AHelper(buffer, upb.data(), _countof(buffer));
			stringlist tokens = TokenizeString(ubpdata.c_str(), "\r\n", true);
			for (stringlist::iterator itr = tokens.begin(); itr != tokens.end(); ++itr) {
				string& line = (*itr);
				if (wildmatch("*#", line)) { // ends with #
					stringlist bonelod = TokenizeString(line.c_str(), "#", true);
					for (stringlist::iterator token = bonelod.begin(); token != bonelod.end(); ++token) {
						if (wildmatch("??BoneLOD", (*token).c_str())) {
							if (++token == bonelod.end())
								break;
							if (strmatch("Bone", (*token).c_str())) {
								if (++token == bonelod.end())
									break;
								int group = 0;
								std::stringstream str(*token);
								str >> group;
								boneCtrl->AddNodeToGroup(group, exporter->getNode(child));
							}
						}
					}
				}
			}
		}
	}
}

void Exporter::InitializeRigidBody(bhkRigidBodyRef body, INode *node)
{
	float value;
	if (node->GetUserPropFloat(TEXT("Mass"), value))
		body->SetMass(value);
	if (node->GetUserPropFloat(TEXT("Ellasticity"), value))
		body->SetRestitution(value);
	if (node->GetUserPropFloat(TEXT("Friction"), value))
		body->SetFriction(value);
	if (node->GetUserPropFloat(TEXT("Unyielding"), value))
		body->SetFriction(value);

	body->SetLayer(OblivionLayer(OL_BIPED | 0x200));
	body->SetLayerCopy(OblivionLayer(OL_BIPED | 0x200));

	Matrix3 tm = node->GetObjTMAfterWSM(0);
	body->SetRotation(TOQUATXYZW(Quat(tm)));
	body->SetTranslation(TOVECTOR4(tm.GetTrans() / 7.0f));
}

NiNodeRef Exporter::exportBone(NiNodeRef parent, INode *node)
{

	USES_CONVERSION;
	bool local = !mFlattenHierarchy;
	NiNodeRef newParent = makeNode(parent, node, local);

	// Special Skeleton Only handling routines
	if (mSkeletonOnly)
	{
		//Skeleton export: a controller appears during skyrim's chicken round trip
		if (Exporter::mNifVersionInt >= VER_10_0_1_0 && !IsSkyrim())
			InitializeTimeController(new NiTransformController(), newParent);

		bool isBoneRoot = false;
		if (IsOblivion() || IsFallout3() /*|| IsSkyrim()*/)
		{
			// Check for Bone Root
			TSTR upb;
			node->GetUserPropBuffer(upb);
			string upbdata = T2A(upb.data());
			stringlist tokens = TokenizeString(upbdata.c_str(), "\r\n", true);
			for (stringlist::iterator itr = tokens.begin(); itr != tokens.end(); ++itr) {
				string& line = (*itr);
				if (wildmatch("*#", line)) { // ends with #
					stringlist bonelod = TokenizeString(line.c_str(), "#", true);
					for (stringlist::iterator token = bonelod.begin(); token != bonelod.end(); ++token) {
						if (wildmatch("??BoneLOD", (*token).c_str())) {
							if (++token == bonelod.end())
								break;
							if (strmatch("BoneRoot", (*token).c_str())) {
								isBoneRoot = true;
								NiBSBoneLODControllerRef boneCtrl = new NiBSBoneLODController();
								InitializeTimeController(boneCtrl, newParent);
								FillBoneController(this, boneCtrl, node);
								break;
							}
						}
					}
				}
			}

			if (!isBoneRoot )
				InitializeTimeController(new bhkBlendController(), newParent);

			if (mGenerateBoneCollision)
			{
				Matrix3 tm = node->GetObjTMAfterWSM(0);

				bhkShapeRef shape;
				int nc = node->NumberOfChildren();
				if (nc == 0) {
					// Nothing
				}
				else if (nc == 1) {
					// Capsule
					INode *child = node->GetChildNode(0);
					Matrix3 ctm = Inverse(tm) * child->GetObjTMAfterWSM(0);
					float len = ctm.GetTrans().Length();
					float boxLen = mBoundingBox.Width().Length();
					float ratio = len / boxLen;
					if (ratio < 0.05) {
						// do nothing
					}
					else if (ratio < 0.15) {
						// Perpendicular Capsule
						Point3 center = (ctm.GetTrans() / 2.0f) + tm.GetTrans();
						Matrix3 rtm = tm * RotateXMatrix(TORAD(90));
						rtm.SetTranslate(center);

						Point3 pt1 = VectorTransform(Point3(len, 0.0f, 0.0f), rtm);
						Point3 pt2 = VectorTransform(Point3(-len, 0.0f, 0.0f), rtm);
                  float radius = len / bhkAppScaleFactor / 2.0f ;

						bhkCapsuleShapeRef capsule = new bhkCapsuleShape();
						capsule->SetRadius(radius);
						capsule->SetRadius1(radius);
						capsule->SetRadius2(radius);
						capsule->SetFirstPoint( TOVECTOR3(pt1 / bhkAppScaleFactor) );
						capsule->SetSecondPoint( TOVECTOR3(pt2 / bhkAppScaleFactor) );
						capsule->SetMaterial(HAV_MAT_SKIN);

						shape = StaticCast<bhkShape>(capsule);
					}
					else {
						// Normal Capsule
						Point3 center = (ctm.GetTrans() / 2.0f) + tm.GetTrans();
					}
				}
				else {
					// Sphere
					float radius = 0.0f;
					CalcBoundingSphere(node, tm.GetTrans(), radius, 0);

					bhkSphereShapeRef sphere = new bhkSphereShape();
					sphere->SetRadius(radius / bhkAppScaleFactor);
					sphere->SetMaterial(HAV_MAT_SKIN);
					shape = StaticCast<bhkShape>(sphere);
				}

				if (shape)
				{
					bhkBlendCollisionObjectRef blendObj = new bhkBlendCollisionObject();
					bhkRigidBodyRef body = new bhkRigidBody();

					InitializeRigidBody(body, node);
					body->SetMotionSystem(MotionSystem(6));
					body->SetQualityType(MO_QUAL_KEYFRAMED);
					body->SetShape(StaticCast<bhkShape>(shape));
					blendObj->SetBody(StaticCast<NiObject>(body));
					newParent->SetCollisionObject(StaticCast<NiCollisionObject>(blendObj));
				}
			}

		}

		if (IsSkyrim()) {
			
			TSTR ragdollNodeName = FormatText(TEXT("Ragdoll_%s"), node->GetName());
			INode* ragdollParent = GetCOREInterface()->GetINodeByName(ragdollNodeName);
			if (ragdollParent) {
	//			throw exception(ragdollNodeName);
				HavokExport exporter;
				exporter.makeHavokRigidBody(newParent, ragdollParent, bhkAppScaleFactor);
			}
		}

		if (mExportType != NIF_WO_ANIM && isNodeTracked(node)) {
			NiNodeRef accumNode = createAccumNode(newParent, node);

			// Transfer collision object to accum and create blend on accum
			if (IsOblivion() || IsFallout3() || IsSkyrim()) {
//				if (!IsSkyrim())
					InitializeTimeController(new bhkBlendController(), accumNode);
				accumNode->SetCollisionObject(newParent->GetCollisionObject());
				newParent->SetCollisionObject(NiCollisionObjectRef());
			}
			newParent = accumNode;
		}
		else if (isSkeletonRoot(node)) {
			newParent = createAccumNode(newParent, node);
		}
	}
	else // normal handling
	{
		// Check for Accum Root using 
		if (mExportType == NIF_WO_KF) {
			// Add controllers
			if (Exporter::mAllowAccum) {
				newParent = createAccumNode(newParent, node);
			}
		}
		else if (mExportType != NIF_WO_ANIM && isNodeTracked(node)) {
			newParent = createAccumNode(newParent, node);
		}
		else if (isSkeletonRoot(node)) {
			newParent = createAccumNode(newParent, node);
		}
	}

	return newParent;
}
