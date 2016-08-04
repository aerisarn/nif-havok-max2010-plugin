#include "pch.h"
#include "../NifProps/bhkRigidBodyInterface.h"
#include "obj/bhkListShape.h"
#include "obj/bhkConvexVerticesShape.h"
#include "obj/bhkTransformShape.h"
#include "obj/bhkSphereShape.h"
#include "obj/bhkBoxShape.h"
#include "obj/bhkCapsuleShape.h"
#include "obj/hkPackedNiTriStripsData.h"
#include "obj/bhkPackedNiTriStripsShape.h"
#include "obj/bhkMoppBvTreeShape.h"

#include "..\NifProps\bhkHelperFuncs.h"
#include "..\NifProps\bhkHelperInterface.h"
#include "vectorstream.hpp"
#ifdef _DEBUG
#include <assert.h>
#include <crtdbg.h>
#define ASSERT _ASSERTE
#else
#define ASSERT(x)
#endif

static Class_ID SCUBA_CLASS_ID(0x6d3d77ac, 0x79c939a9);
extern Class_ID BHKRIGIDBODYMODIFIER_CLASS_ID;
extern Class_ID BHKLISTOBJECT_CLASS_ID;
extern Class_ID bhkBoxObject_CLASS_ID;
extern Class_ID BHKCAPSULEOBJECT_CLASS_ID;
extern Class_ID bhkSphereObject_CLASS_ID;
extern Class_ID BHKPROXYOBJECT_CLASS_ID;

enum
{
	CAPSULE_RADIUS = 0,
	CAPSULE_HEIGHT = 1,
};

extern bool GetHavokMaterialsFromIndex(int idx, /*HavokMaterial*/int* havk_material, /*SkyrimHavokMaterial*/int* skyrim_havok_material);
extern int GetEquivalentSkyrimMaterial(int havok_material);

inline int GetPBValue(IParamBlock2 *pb, int id, int defaultValue)
{
	int value = pb->GetInt(id, 0, 0);
	return (value < 0) ? defaultValue : value;
}

extern HINSTANCE hInstance;
class HavokMoppCode
{
private:
	struct RemoteCommand
	{
		virtual int Read(LPVOID buffer, int size) = 0;
		virtual int Write(LPCVOID buffer, int size) = 0;
	};

	typedef int(__stdcall * fnGenerateMoppCode)(int nVerts, Vector3 const* verts, int nTris, Triangle const *tris);
	typedef int(__stdcall * fnGenerateMoppCodeWithSubshapes)(int nShapes, int const *shapes, int nVerts, Vector3 const* verts, int nTris, Triangle const *tris);
	typedef int(__stdcall * fnRetrieveMoppCode)(int nBuffer, unsigned char *buffer);
	typedef int(__stdcall * fnRetrieveMoppScale)(float *value);
	typedef int(__stdcall * fnRetrieveMoppOrigin)(Vector3 *value);
	typedef int(__stdcall * fnExecuteCommand)(const char* cmd, RemoteCommand* ifc, DWORD dwTimeout);

	HMODULE hMoppLib;
	fnGenerateMoppCode GenerateMoppCode;
	fnRetrieveMoppCode RetrieveMoppCode;
	fnRetrieveMoppScale RetrieveMoppScale;
	fnRetrieveMoppOrigin RetrieveMoppOrigin;
	fnGenerateMoppCodeWithSubshapes GenerateMoppCodeWithSubshapes;
	fnExecuteCommand ExecuteCommand;

public:
	HavokMoppCode() : hMoppLib(nullptr), GenerateMoppCode(nullptr), RetrieveMoppCode(nullptr)
		, RetrieveMoppScale(nullptr), RetrieveMoppOrigin(nullptr), GenerateMoppCodeWithSubshapes(nullptr)
		, ExecuteCommand(nullptr)
	{
	}

	~HavokMoppCode() {
		if (hMoppLib) FreeLibrary(hMoppLib);
	}

	bool Initialize()
	{
		if (hMoppLib == NULL)
		{
			TCHAR curfile[_MAX_PATH];
			GetModuleFileName(hInstance, curfile, MAX_PATH);
			PathRemoveFileSpec(curfile);
			PathAppend(curfile, TEXT("NifMopp.dll"));
			hMoppLib = LoadLibrary(curfile);
			if (hMoppLib == NULL)
				hMoppLib = LoadLibrary(TEXT("NifMopp.dll"));
			GenerateMoppCode = (fnGenerateMoppCode)GetProcAddress(hMoppLib, "GenerateMoppCode");
			GenerateMoppCodeWithSubshapes = (fnGenerateMoppCodeWithSubshapes)GetProcAddress(hMoppLib, "GenerateMoppCodeWithSubshapes");
			RetrieveMoppCode = (fnRetrieveMoppCode)GetProcAddress(hMoppLib, "RetrieveMoppCode");
			RetrieveMoppScale = (fnRetrieveMoppScale)GetProcAddress(hMoppLib, "RetrieveMoppScale");
			RetrieveMoppOrigin = (fnRetrieveMoppOrigin)GetProcAddress(hMoppLib, "RetrieveMoppOrigin");
			// ExecuteCommand = (fnExecuteCommand)GetProcAddress(hMoppLib, "ExecuteCommand");
		}
		return (NULL != GenerateMoppCode  && NULL != RetrieveMoppCode
			&& NULL != RetrieveMoppScale && NULL != RetrieveMoppOrigin
			);
	}

	vector<Niflib::byte> CalculateMoppCode(vector<Niflib::Vector3> const & verts, vector<Niflib::Triangle> const & tris, Niflib::Vector3* origin, float* scale)
	{
		vector<Niflib::byte> code;
		if (Initialize())
		{
			int len = GenerateMoppCode(verts.size(), &verts[0], tris.size(), &tris[0]);
			if (len > 0)
			{
				code.resize(len);
				if (0 != RetrieveMoppCode(len, &code[0]))
				{
					if (NULL != scale)
						RetrieveMoppScale(scale);
					if (NULL != origin)
						RetrieveMoppOrigin(origin);
				}
				else
				{
					code.clear();
				}
			}
		}
		return code;
	}

	vector<Niflib::byte> CalculateMoppCode(vector<OblivionSubShape> const & shapes
		, vector<Niflib::Vector3> const & verts
		, vector<Niflib::Triangle> const & tris
		, Niflib::Vector3* origin, float* scale)
	{
		vector<Niflib::byte> code;
		if (Initialize())
		{
			vector<int> subshapeverts;
			for (vector<OblivionSubShape>::const_iterator itr = shapes.begin(); itr != shapes.end(); ++itr)
				subshapeverts.push_back(itr->numVertices);

			int len = 0;
			if (GenerateMoppCodeWithSubshapes != NULL)
				len = GenerateMoppCodeWithSubshapes(subshapeverts.size(), &subshapeverts[0], verts.size(), &verts[0], tris.size(), &tris[0]);
			else
				len = GenerateMoppCode(verts.size(), &verts[0], tris.size(), &tris[0]);
			if (len > 0)
			{
				code.resize(len);
				if (0 != RetrieveMoppCode(len, &code[0]))
				{
					if (NULL != scale)
						RetrieveMoppScale(scale);
					if (NULL != origin)
						RetrieveMoppOrigin(origin);
				}
				else
				{
					code.clear();
				}
			}
		}
		return code;
	}

	vector<Niflib::byte> CalculateMoppCode(Ref<bhkCompressedMeshShapeData> const & data, Niflib::Vector3* origin, float* scale)
	{
		vector<Niflib::byte> code;
		if (Initialize())
		{
			vector<int> subshapeverts;
			const vector<bhkCMSDBigTris>& bigTris = data->GetBigTris();
			const vector<Vector4>& bigVerts = data->GetBigVerts();
			const vector<bhkCMSDChunk>& chunks = data->GetChunks();

			int totalVerts = bigVerts.size();
			int totalTris = bigTris.size();
			for (const bhkCMSDChunk& chunk : chunks) {
				totalVerts += chunk.numVertices / 3;
				totalTris += chunk.numStrips;
			}
			vector<Vector3> verts; verts.reserve(totalVerts);
			vector<Triangle> tris; tris.reserve(totalTris);

			int voffset = 0;
			if (!bigVerts.empty())
			{
				int numVerts = data->GetBigVerts().size();
				subshapeverts.push_back(numVerts);
				for (int i = 0; i < bigVerts.size(); i++)
					verts.push_back(TOVECTOR3(bigVerts[i]));
				for (int i = 0; i < bigTris.size(); i++)
					tris.push_back(Niflib::Triangle(bigTris[i].triangle1, bigTris[i].triangle2, bigTris[i].triangle3));
				voffset += numVerts;
			}

			for (const bhkCMSDChunk& chunk : chunks) {
				auto chunkOrigin = TOVECTOR3(chunk.translation);
				int numVerts = chunk.numVertices / 3;
				int numIndices = chunk.numIndices;
				int numStrips = chunk.numStrips;
				const auto& offsets = chunk.vertices;
				const auto& indices = chunk.indices;
				const auto& strips = chunk.strips;

				for (auto n = 0; n < numVerts; n++) {
					verts.push_back(chunkOrigin + Vector3(offsets[3 * n], offsets[3 * n + 1], offsets[3 * n + 2]) / 1000.0f);
				}
				// Stripped tris
				int offset = 0;
				for (auto s = 0; s < numStrips; s++) {
					for (auto f = 0; f < strips[s] - 2; f++) {
						if ((f + 1) % 2 == 0)
							tris.push_back(Triangle(voffset + indices[offset + f + 2], voffset + indices[offset + f + 1], voffset + indices[offset + f + 0]));
						else
							tris.push_back(Triangle(voffset + indices[offset + f + 0], voffset + indices[offset + f + 1], voffset + indices[offset + f + 2]));
					}
					offset += strips[s];
				}
				// Non-stripped tris
				for (auto f = 0; f < (numIndices - offset); f += 3) {
					tris.push_back(Triangle(voffset + indices[offset + f + 0], voffset + indices[offset + f + 1], voffset + indices[offset + f + 2]));
				}
				subshapeverts.push_back(numVerts);
			}

			int len = 0;
			if (GenerateMoppCodeWithSubshapes != nullptr)
				len = GenerateMoppCodeWithSubshapes(subshapeverts.size(), &subshapeverts[0], verts.size(), &verts[0], tris.size(), &tris[0]);
			else
				len = GenerateMoppCode(verts.size(), &verts[0], tris.size(), &tris[0]);
			if (len > 0)
			{
				code.resize(len);
				if (0 != RetrieveMoppCode(len, &code[0]))
				{
					if (NULL != scale)
						RetrieveMoppScale(scale);
					if (NULL != origin)
						RetrieveMoppOrigin(origin);
				}
				else
				{
					code.clear();
				}
			}
		}
		return code;
	}

	struct VectorStreams : RemoteCommand
	{
		vectorstream instr;
		vectorstream outstr;
		int Read(LPVOID buffer, int size) { return instr.readsome((char*)buffer, size); }
		int Write(LPCVOID buffer, int size) { outstr.write((const char*)buffer, size); return size; }
	};

	void UpdateRigidBody(bhkRigidBodyRef& body)
	{
		if (ExecuteCommand != nullptr)
		{
			static int counter = 0;
			DWORD dwTimeoutMillseconds = 1000 * 10;
			VectorStreams streams;
			Niflib::NifInfo info(Exporter::mNifVersionInt, Exporter::mNifUserVersion, Exporter::mNifUserVersion2);
			WriteNifTree(streams.instr, body, info);
			if (Exporter::mDebugEnabled)
				WriteNifTree(FormatString("D:\\temp\\test%02d.nif", ++counter), body, info);
			streams.instr.flush();
			streams.instr.seekg(0);
			streams.instr.seekp(0);
			ExecuteCommand("UpdateRigidBody", &streams, dwTimeoutMillseconds);
			if (!streams.outstr.vector().empty())
			{
				streams.outstr.seekg(0);
				streams.outstr.seekp(0);

				vector<NiObjectRef> blocks = ReadNifList(streams.outstr, &info);
				bhkRigidBodyRef newBody = SelectFirstObjectOfType<bhkRigidBody>(blocks);
				if (newBody != nullptr)
					body = newBody;

				if (Exporter::mDebugEnabled)
					WriteNifTree(FormatString("D:\\temp\\test%02d_out.nif", counter), body, info);
			}
			else
			{
				// fallback calculation
				if (body->GetMass() != 0.0)
					body->UpdateMassProperties(1.0f, true, body->GetMass());
			}
		}
		else
		{
			// fallback calculation
			if (body->GetMass() != 0.0)
				body->UpdateMassProperties(1.0f, true, body->GetMass());
		}
	}
} TheHavokCode;

static vector<Niflib::byte> ConstructHKMesh(NiTriBasedGeomRef shape, Niflib::Vector3& origin, float& scale)
{
	NiTriBasedGeomDataRef data = shape->GetData();
	return TheHavokCode.CalculateMoppCode(data->GetVertices(), data->GetTriangles(), &origin, &scale);
}

static vector<Niflib::byte> ConstructHKMesh(bhkShapeRef shape, Niflib::Vector3& origin, float& scale)
{
	if (bhkPackedNiTriStripsShapeRef mesh = DynamicCast<bhkPackedNiTriStripsShape>(shape))
	{
		hkPackedNiTriStripsDataRef data = mesh->GetData();
		return TheHavokCode.CalculateMoppCode(mesh->GetSubShapes(), data->GetVertices(), data->GetTriangles(), &origin, &scale);
	}
	else if (bhkCompressedMeshShapeRef cmsd = DynamicCast<bhkCompressedMeshShape>(shape))
	{
		return TheHavokCode.CalculateMoppCode(cmsd->GetData(), &origin, &scale);
	}
	return vector<Niflib::byte>();
}

/*
To mimic the "Reset Transform" and "Reset Scale" behavior, the following code snippet should help:



	Interface *ip = theResetScale.ip;
	TimeValue t = ip->GetTime();

	Control *tmControl = node->GetTMController();
	BOOL lookAt = tmControl->GetRollController() ? TRUE : FALSE;

	Matrix3 ntm = node->GetNodeTM(t);
	Matrix3 ptm = node->GetParentTM(t);
	Matrix3 rtm = ntm * Inverse(ptm);
	Matrix3 otm(1);
	Quat rot;

	// Grab the trans, and then set it to 0
	Point3 trans = rtm.GetTrans();
	rtm.NoTrans();

	// We're only doing scale - save out the
	// rotation so we can put it back
	AffineParts parts;
	decomp_affine(rtm, &parts);
	rot = parts.q;

	// Build the offset tm
	otm.PreTranslate(node->GetObjOffsetPos());
	if (node->GetObjOffsetRot()!=IdentQuat()) {
		PreRotateMatrix(otm,node->GetObjOffsetRot());
		}

	Point3 tS(1,1,1);
	if ( node->GetObjOffsetScale().s != tS ) {
		ApplyScaling(otm,node->GetObjOffsetScale());
		}

	// Apply the relative tm to the offset
	otm = otm * rtm;
	decomp_affine(otm, &parts);
	node->SetObjOffsetPos(parts.t);
	node->SetObjOffsetScale(ScaleValue(parts.k*parts.f,parts.u));

	// Now set the transform controller with a matrix
	// that has no rotation or scale
	rtm.IdentityMatrix();
	rtm.SetTrans(trans);
	if (!lookAt) {
		PreRotateMatrix(rtm,rot);
		}

	// But first, want to keep children stationary.
	Matrix3 ttm = rtm*ptm;
	for (int i=0; iNumberOfChildren(); i++)  {
		Control *tmc  = node->GetChildNode(i)->GetTMController();
		Matrix3 oldtm = node->GetChildNode(i)->GetNodeTM(t);
		SetXFormPacket pk(oldtm,ttm);
		tmc->SetValue(t,&pk);
		}

	SetXFormPacket pckt(rtm);
	tmControl->SetValue(t,&pckt);



To mimic the "Align to world" behavior, the following code snippet should help:



	AffineParts parts;
	TimeValue currtime = m_pInterface->GetTime();
	Matrix3 m = pNode->GetNodeTM(currtime);
	decomp_affine(m, &parts);
	if (rotobj) {
		// if "affect obj only" we move it simply thus:
		pNode->SetObjOffsetRot(Inverse(parts.q));
	} else {
		// otherwise, "affect pivot only" we would do:
		IdentityTM ident;
		Matrix3 wax = ident;
		wax.SetTrans(m.GetTrans());  // world aligned axis,  centered at pivot point
		pNode->Rotate(currtime, wax, Inverse(parts.q),TRUE,FALSE, PIV_PIVOT_ONLY);
	}
	m_pInterface->RedrawViews(m_pInterface->GetTime(),REDRAW_NORMAL,NULL);

*/

int Exporter::addVertex(vector<Vector3> &verts, vector<Vector3> &vnorms, const Point3 &pt, const Point3 &norm)
{
	for (unsigned int i = 0; i < verts.size(); i++)
	{
		if (equal(verts[i], pt, mWeldThresh) &&
			equal(vnorms[i], norm, 0))
			return i;
	}

	verts.push_back(Vector3(pt.x, pt.y, pt.z));
	vnorms.push_back(Vector3(norm.x, norm.y, norm.z));

	return verts.size() - 1;
}

void Exporter::addFace(Triangles &tris, vector<Vector3> &verts, vector<Vector3> &vnorms,
	int face, const int vi[3], Mesh *mesh, Matrix3& tm)
{
	Triangle tri;
	for (int i = 0; i < 3; i++)
	{
		Point3 pt = mesh->verts[mesh->faces[face].v[vi[i]]] * tm;
		Point3 norm = getVertexNormal(mesh, face, mesh->getRVertPtr(mesh->faces[face].v[vi[i]])) * tm;
		tri[i] = addVertex(verts, vnorms, pt, norm);
	}
	tris.push_back(tri);
}

Exporter::Result Exporter::exportCollision(NiNodeRef &parent, INode *node)
{
	if (isHandled(node) || (node->IsHidden() && !mExportHidden))
		return Exporter::Skip;

	ProgressUpdate(Collision, FormatText(TEXT("'%s' Collision"), node->GetName()));

	// marked as collision?
	//bool coll = npIsCollision(node);
	bool coll = isCollision(node);

	bool local = !mFlattenHierarchy;
	NiNodeRef nodeParent = mFlattenHierarchy ? mNiRoot : parent;

	NiNodeRef newParent;
	if (coll)
	{
		markAsHandled(node);

		newParent = Exporter::findNode(node->GetParentNode());
		if (!newParent) {
			newParent = nodeParent;
		}
		//newParent = nodeParent; // always have collision one level up?

		TimeValue t = 0;

		bhkRigidBodyRef body = makeCollisionBody(node);
		if (body)
		{
			bool hasTrans = (body->IsDerivedType(bhkRigidBodyT::TYPE));
			Matrix3 tm = getNodeTransform(node, t, false); // has transform
			Matrix3 pm = TOMATRIX3(newParent->GetWorldTransform());
			tm = tm * Inverse(pm);

			Matrix44 rm4 = TOMATRIX4(tm, false);
			Vector3 trans; Matrix33 rm; float scale;
			rm4.Decompose(trans, rm, scale);
			QuaternionXYZW q = TOQUATXYZW(rm.AsQuaternion());
			body->SetRotation(q);
			body->SetTranslation(trans / bhkAppScaleFactor);

			if (hasTrans) {
				tm = getNodeTransform(node, t, false);
				tm.NoScale();
				tm.Invert();
			}
			else {
				tm = TOMATRIX3(newParent->GetWorldTransform());
				tm.NoScale();
				tm.Invert();
				//tm.IdentityMatrix();
			}

			bhkShapeRef shape = makeCollisionShape(node, tm, body, HavokMaterial(NP_DEFAULT_HVK_MATERIAL));
			if (shape)
			{
				body->SetShape(DynamicCast<bhkShape>(shape));
				updateRigidBody(body);

				bhkCollisionObjectRef co = new bhkCollisionObject();
				co->SetBody(DynamicCast<NiObject>(body));
				newParent->SetCollisionObject(DynamicCast<NiCollisionObject>(co));
				// full bhkRigidBody Calc
			}
		}
	}
	else if (isCollisionGroup(node) && !mFlattenHierarchy) {
		newParent = makeNode(nodeParent, node);
	}
	else {
		newParent = Exporter::findNode(node->GetParentNode());
		if (!newParent)
			newParent = nodeParent;
	}
	if (!newParent)
		newParent = nodeParent;

	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		Result result = exportCollision(newParent, node->GetChildNode(i));
		if (result != Ok && result != Skip)
			return result;
	}

	return Ok;
}


bhkRigidBodyRef Exporter::makeCollisionBody(INode *node)
{
	// get data from node
	int lyr = NP_DEFAULT_HVK_LAYER;
	int mtl = NP_DEFAULT_HVK_MATERIAL;
	int msys = NP_DEFAULT_HVK_MOTION_SYSTEM;
	int qtype = NP_DEFAULT_HVK_QUALITY_TYPE;
	float mass = NP_DEFAULT_HVK_MASS;
	float lindamp = NP_DEFAULT_HVK_LINEAR_DAMPING;
	float angdamp = NP_DEFAULT_HVK_ANGULAR_DAMPING;
	float frict = NP_DEFAULT_HVK_FRICTION;
	float maxlinvel = NP_DEFAULT_HVK_MAX_LINEAR_VELOCITY;
	float maxangvel = NP_DEFAULT_HVK_MAX_ANGULAR_VELOCITY;
	float resti = NP_DEFAULT_HVK_RESTITUTION;
	float pendepth = NP_DEFAULT_HVK_PENETRATION_DEPTH;
	Vector3 center(0, 0, 0);
	BOOL transenable = TRUE;

	if (bhkRigidBodyInterface *irb = (bhkRigidBodyInterface *)node->GetObjectRef()->GetInterface(BHKRIGIDBODYINTERFACE_DESC))
	{
		mass = irb->GetMass(0);
		frict = irb->GetFriction(0);
		resti = irb->GetRestitution(0);
		lyr = irb->GetLayer(0);
		msys = irb->GetMotionSystem(0);
		qtype = irb->GetQualityType(0);
		lindamp = irb->GetLinearDamping(0);
		angdamp = irb->GetAngularDamping(0);
		maxlinvel = irb->GetMaxLinearVelocity(0);
		pendepth = irb->GetPenetrationDepth(0);
		maxangvel = irb->GetMaxAngularVelocity(0);
		transenable = irb->GetEnableTransform(0);
	}
	else if (npIsCollision(node))
	{
		// Handle compatibility
		npGetProp(node, NP_HVK_MASS_OLD, mass, NP_DEFAULT_HVK_EMPTY);
		if (mass == NP_DEFAULT_HVK_EMPTY)
			npGetProp(node, NP_HVK_MASS, mass, NP_DEFAULT_HVK_MASS);
		npGetProp(node, NP_HVK_FRICTION_OLD, frict, NP_DEFAULT_HVK_EMPTY);
		if (frict == NP_DEFAULT_HVK_EMPTY)
			npGetProp(node, NP_HVK_FRICTION, frict, NP_DEFAULT_HVK_FRICTION);
		npGetProp(node, NP_HVK_RESTITUTION_OLD, resti, NP_DEFAULT_HVK_EMPTY);
		if (resti == NP_DEFAULT_HVK_EMPTY)
			npGetProp(node, NP_HVK_RESTITUTION, resti, NP_DEFAULT_HVK_RESTITUTION);

		npGetProp(node, NP_HVK_LAYER, lyr, NP_DEFAULT_HVK_LAYER);
		npGetProp(node, NP_HVK_MATERIAL, mtl, NP_DEFAULT_HVK_MATERIAL);
		npGetProp(node, NP_HVK_MOTION_SYSTEM, msys, NP_DEFAULT_HVK_MOTION_SYSTEM);
		npGetProp(node, NP_HVK_QUALITY_TYPE, qtype, NP_DEFAULT_HVK_QUALITY_TYPE);
		npGetProp(node, NP_HVK_LINEAR_DAMPING, lindamp, NP_DEFAULT_HVK_LINEAR_DAMPING);
		npGetProp(node, NP_HVK_ANGULAR_DAMPING, angdamp, NP_DEFAULT_HVK_ANGULAR_DAMPING);
		npGetProp(node, NP_HVK_MAX_LINEAR_VELOCITY, maxlinvel, NP_DEFAULT_HVK_MAX_LINEAR_VELOCITY);
		npGetProp(node, NP_HVK_MAX_ANGULAR_VELOCITY, maxangvel, NP_DEFAULT_HVK_MAX_ANGULAR_VELOCITY);
		npGetProp(node, NP_HVK_PENETRATION_DEPTH, pendepth, NP_DEFAULT_HVK_PENETRATION_DEPTH);
		npGetProp(node, NP_HVK_CENTER, center);
	}
	else
	{
		// Check self to see if is one of our bhkXXXObject classes
		if (Object* obj = node->GetObjectRef())
		{
			if (obj->SuperClassID() == HELPER_CLASS_ID &&
				obj->ClassID().PartB() == BHKRIGIDBODYCLASS_DESC.PartB())
			{
				// TODO: do standard body export
			}
		}

		// else check redirection 
	}


	// setup body
	bhkRigidBodyRef body = transenable ? new bhkRigidBodyT() : new bhkRigidBody();

	OblivionLayer obv_layer; SkyrimLayer sky_layer;
	GetHavokLayersFromIndex(lyr, (int*)&obv_layer, (int*)&sky_layer);
	body->SetLayer(obv_layer);
	body->SetLayerCopy(obv_layer);
	body->SetSkyrimLayer(sky_layer);

	body->SetMotionSystem(MotionSystem(msys));
	body->SetQualityType(MotionQuality(qtype));
	body->SetMass(mass);
	body->SetLinearDamping(lindamp);
	body->SetAngularDamping(angdamp);
	body->SetFriction(frict);
	body->SetRestitution(resti);
	body->SetMaxLinearVelocity(maxlinvel);
	body->SetMaxAngularVelocity(maxangvel);
	body->SetPenetrationDepth(pendepth);
	body->SetCenter(center);
	QuaternionXYZW q; q.x = q.y = q.z = 0; q.w = 1.0f;
	body->SetRotation(q);

	return body;
}

bhkNiTriStripsShapeRef Exporter::makeTriStripsShape(Mesh& mesh, Matrix3& sm, int mtlDefault)
{
	typedef vector<Triangle> Triangles;

	// setup shape data
	vector<Vector3> verts;
	vector<Vector3> vnorms;
	Triangles		tris;

	int vi[3];
	if (TMNegParity(sm)) {
		vi[0] = 2; vi[1] = 1; vi[2] = 0;
	}
	else {
		vi[0] = 0; vi[1] = 1; vi[2] = 2;
	}

	for (int i = 0; i < mesh.getNumFaces(); i++)
		addFace(tris, verts, vnorms, i, vi, &mesh, sm);

	NiTriStripsDataRef data = new NiTriStripsData(tris, Exporter::mUseAlternateStripper);
	data->SetVertices(verts);
	data->SetNormals(vnorms);

	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtlDefault, (int*)&material, (int*)&skyrimMaterial);

	// setup shape
	bhkNiTriStripsShapeRef shape = StaticCast<bhkNiTriStripsShape>(bhkNiTriStripsShape::Create());
	shape->SetNumStripsData(1);
	shape->SetStripsData(0, data);
	shape->SetNumDataLayers(1);
	shape->SetOblivionLayer(0, OL_STATIC);
	shape->SetMaterial(material);
	shape->SetSkyrimMaterial(skyrimMaterial);

	return shape;
}

static bhkMoppBvTreeShapeRef makeTreeShape(bhkShapeRef shape, int mtlDefault)
{
	if (!TheHavokCode.Initialize()) return shape;

	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtlDefault, (int*)&material, (int*)&skyrimMaterial);

	bhkMoppBvTreeShapeRef mopp = new bhkMoppBvTreeShape();
	mopp->SetMaterial(material);
	mopp->SetSkyrimMaterial(skyrimMaterial);
	mopp->SetShape(shape);

	try
	{
		Niflib::Vector3 offset;
		float scale;
		vector<Niflib::byte> moppcode = ConstructHKMesh(shape, offset, scale);
		mopp->SetMoppCode(moppcode);
		mopp->SetMoppOrigin(offset);
		mopp->SetMoppScale(scale);
	}
	catch (...)
	{
	}
	return mopp;
}

bhkPackedNiTriStripsShapeRef Exporter::makePackedTriStripsShape(Mesh& mesh, Matrix3& sm, int mtlDefault, int layer, int colFilter)
{
	// Need to separate the vertices based on material.  
	typedef vector<Triangle> Triangles;

	// setup shape data
	vector<Vector3> verts;
	vector<Vector3> norms;
	Triangles		tris;

	int vi[3];
	if (TMNegParity(sm)) {
		vi[0] = 2; vi[1] = 1; vi[2] = 0;
	}
	else {
		vi[0] = 0; vi[1] = 1; vi[2] = 2;
	}

	int nvert = mesh.getNumVerts();
	int nface = mesh.getNumFaces();
	mesh.buildNormals();

	tris.resize(nface);
	verts.resize(nvert);
	norms.resize(nface);
	for (int i = 0; i < nvert; ++i)
	{
		Point3 vert = (mesh.getVert(i) * sm) / bhkAppScaleFactor;
		verts[i] = TOVECTOR3(vert);
	}
	for (int i = 0; i < nface; ++i)
	{
		Triangle& tri = tris[i];
		norms[i] = TOVECTOR3(mesh.getFaceNormal(i));
		Face& face = mesh.faces[i];
		tri[0] = (USHORT)face.getVert(0);
		tri[1] = (USHORT)face.getVert(1);
		tri[2] = (USHORT)face.getVert(2);
	}

	hkPackedNiTriStripsDataRef data = new hkPackedNiTriStripsData();
	data->SetNumFaces(tris.size());
	data->SetVertices(verts);
	data->SetTriangles(tris);
	data->SetNormals(norms);

	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtlDefault, (int*)&material, (int*)&skyrimMaterial);

	OblivionLayer obv_layer; SkyrimLayer sky_layer;
	GetHavokLayersFromIndex(layer, (int*)&obv_layer, (int*)&sky_layer);

	// setup shape
	bhkPackedNiTriStripsShapeRef shape = new bhkPackedNiTriStripsShape();
	shape->SetData(data);

	OblivionSubShape subshape;
	subshape.layer = obv_layer;
	subshape.material = material;
	subshape.colFilter = colFilter;
	subshape.numVertices = verts.size();

	vector<OblivionSubShape> subshapes;
	subshapes.push_back(subshape);
	shape->SetSubShapes(subshapes);
	data->SetSubShapes(subshapes);
	return shape;
}

bhkConvexVerticesShapeRef Exporter::makeConvexShape(Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	bhkConvexVerticesShapeRef shape = StaticCast<bhkConvexVerticesShape>(bhkConvexVerticesShape::Create());
	Point3 center(0.0f, 0.0f, 0.0f);
	float radius = 0.10f;
	CalcCenteredSphere(mesh, center, radius);
	radius /= bhkAppScaleFactor;
	shape->SetRadius(radius);
	vector<Vector3> verts;
	vector<Vector4> norms;
	int nvert = mesh.getNumVerts();
	int nface = mesh.getNumFaces();
	mesh.checkNormals(FALSE);
	mesh.buildNormals();

	verts.resize(nvert);
	norms.resize(nface);
	for (int i = 0; i < nvert; ++i)
	{
		Point3 vert = (mesh.getVert(i) * tm) / bhkAppScaleFactor;
		verts[i] = TOVECTOR3(vert);
	}
	for (int i = 0; i < nface; ++i)
	{
		Vector4 &value = norms[i];
		Point3 &pt = mesh.getFaceNormal(i);
		value[0] = pt.x;
		value[1] = pt.y;
		value[2] = pt.z;
		value[3] = -(mesh.FaceCenter(i) * tm).Length() / bhkAppScaleFactor;
	}
	sortVector3(verts);
	sortVector4(norms);
	shape->SetVertices(verts);
	shape->SetNormalsAndDist(norms);

	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtlDefault, (int*)&material, (int*)&skyrimMaterial);
	shape->SetMaterial(material);
	shape->SetSkyrimMaterial(skyrimMaterial);

	return shape;
}


bhkShapeRef Exporter::makeCollisionShape(INode *node, Matrix3& tm, bhkRigidBodyRef body, int mtlDefault)
{
	bhkShapeRef shape;

	TimeValue t = 0;
	ObjectState os = node->EvalWorldState(t);
	if (node->IsGroupHead())
		shape = makeModPackedTriStripShape(node, tm, mtlDefault);
	else if (os.obj->ClassID() == SCUBA_CLASS_ID)
		shape = makeCapsuleShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == Class_ID(BOXOBJ_CLASS_ID, 0))
		shape = makeBoxShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == Class_ID(SPHERE_CLASS_ID, 0))
		shape = makeSphereShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == bhkBoxObject_CLASS_ID)
		shape = makebhkBoxShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == bhkSphereObject_CLASS_ID)
		shape = makebhkSphereShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == BHKCAPSULEOBJECT_CLASS_ID)
		shape = makebhkCapsuleShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->ClassID() == BHKLISTOBJECT_CLASS_ID)
		shape = makeListShape(node, tm, body, mtlDefault);
	else if (os.obj->ClassID() == BHKPROXYOBJECT_CLASS_ID)
		shape = makeProxyShape(node, os.obj, tm, mtlDefault);
	else if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{
		if (Modifier* mod = GetbhkCollisionModifier(node))
		{
			shape = makeModifierShape(node, os.obj, mod, tm, mtlDefault);
		}
		else
		{
			shape = makeTriStripsShape(node, tm, mtlDefault);
		}
	}
	return shape;
}

bhkShapeRef Exporter::makeBoxShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	Point3 scale = GetScale(tm);
	float length = 0;
	float height = 0;
	float width = 0;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(0))
	{
		pblock2->GetValue(BOXOBJ_LENGTH, 0, length, FOREVER);
		pblock2->GetValue(BOXOBJ_HEIGHT, 0, height, FOREVER);
		pblock2->GetValue(BOXOBJ_WIDTH, 0, width, FOREVER);
	}

	bhkBoxShapeRef box = new bhkBoxShape();
	Vector3 dim(width * scale[0], length * scale[1], height * scale[2]);

	// Adjust translation for center of z axis in box
	tm.Translate(Point3(0.0, 0.0, dim.z / 2.0));

	dim /= (bhkAppScaleFactor * 2);
	box->SetDimensions(dim);

	int mtl = mtlDefault;
	npGetProp(node, NP_HVK_MATERIAL, mtl, mtlDefault);
	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
	box->SetMaterial(material);
	box->SetSkyrimMaterial(skyrimMaterial);

	return bhkShapeRef(DynamicCast<bhkSphereRepShape>(box));
}

bhkShapeRef Exporter::makeSphereShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	Point3 scale = GetScale(tm);
	float s = (scale[0] + scale[1] + scale[2]) / 3.0f;

	float radius = 0;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(0))
	{
		pblock2->GetValue(SPHERE_RADIUS, 0, radius, FOREVER);
	}

	bhkSphereShapeRef sphere = new bhkSphereShape();
	sphere->SetRadius(radius * s);

	int mtl = mtlDefault;
	npGetProp(node, NP_HVK_MATERIAL, mtl, mtlDefault);
	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
	sphere->SetMaterial(material);
	sphere->SetSkyrimMaterial(skyrimMaterial);

	return bhkShapeRef(DynamicCast<bhkSphereRepShape>(sphere));
}

bhkShapeRef Exporter::makeCapsuleShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	Point3 scale = GetScale(tm);
	float s = (scale[0] + scale[1] + scale[2]) / 3.0f;

	float radius = 0.1f;
	float height = 0.1f;
	if (IParamBlock2* params = obj->GetParamBlockByID(0))
	{
		params->GetValue(CAPSULE_RADIUS, 0, radius, FOREVER);
		params->GetValue(CAPSULE_HEIGHT, 0, height, FOREVER);
	}

	bhkCapsuleShapeRef capsule = CreateNiObject<bhkCapsuleShape>();

	capsule->SetRadius(radius);
	capsule->SetRadius1(radius);
	capsule->SetRadius2(radius);

	int mtl = mtlDefault;
	npGetProp(node, NP_HVK_MATERIAL, mtl, mtlDefault);
	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
	capsule->SetMaterial(material);
	capsule->SetSkyrimMaterial(skyrimMaterial);

	return bhkShapeRef(DynamicCast<bhkSphereRepShape>(capsule));
}

bhkShapeRef Exporter::makebhkBoxShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	enum { box_params, };
	enum { PB_MATERIAL, PB_LENGTH, PB_WIDTH, PB_HEIGHT, };

	bhkShapeRef retval;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(box_params))
	{
		Point3 scale = GetScale(tm);
		float s = (scale[0] + scale[1] + scale[2]) / 3.0f;

		float length = 0, width = 0, height = 0;

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(GetPBValue(pblock2, PB_MATERIAL, mtlDefault), (int*)&material, (int*)&skyrimMaterial);

		pblock2->GetValue(PB_LENGTH, 0, length, FOREVER, 0);
		pblock2->GetValue(PB_WIDTH, 0, width, FOREVER, 0);
		pblock2->GetValue(PB_HEIGHT, 0, height, FOREVER, 0);

		bhkBoxShapeRef box = new bhkBoxShape();

		Vector3 dim(width * scale[0], length * scale[1], height * scale[2]);

		float radius = max(max(dim.x, dim.y), dim.z);
		box->SetRadius(radius);

		// Adjust translation for center of z axis in box
		tm.Translate(Point3(0.0, 0.0, dim.z / 2.0));
		box->SetDimensions(dim);
		box->SetMaterial(material);
		box->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = node->GetNodeTM(0) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(box);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(box);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}

	return retval;
}

bhkShapeRef	Exporter::makebhkSphereShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	bhkShapeRef retval;

	enum { sphere_params, };
	enum { PB_MATERIAL, PB_RADIUS, PB_SEGS, PB_SMOOTH, };

	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(sphere_params))
	{
		float radius = 0.0f;
		pblock2->GetValue(PB_RADIUS, 0, radius, FOREVER, 0);

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(GetPBValue(pblock2, PB_MATERIAL, mtlDefault), (int*)&material, (int*)&skyrimMaterial);

		bhkSphereShapeRef shape = new bhkSphereShape();
		shape->SetRadius(radius);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = node->GetNodeTM(0) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}

bhkShapeRef	Exporter::makebhkCapsuleShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	bhkShapeRef retval;

	enum { cap_params, };
	enum { PB_MATERIAL, PB_RADIUS1, PB_RADIUS2, PB_LENGTH, PB_SCALE };

	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(cap_params))
	{
		float radius1 = 0.0f, radius2 = 0.0f, len = 0.0f;
		pblock2->GetValue(PB_RADIUS1, 0, radius1, FOREVER, 0);
		pblock2->GetValue(PB_RADIUS2, 0, radius2, FOREVER, 0);
		pblock2->GetValue(PB_LENGTH, 0, len, FOREVER, 0);

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(GetPBValue(pblock2, PB_MATERIAL, mtlDefault), (int*)&material, (int*)&skyrimMaterial);

		bhkCapsuleShapeRef shape = new bhkCapsuleShape();
		shape->SetRadius((radius1 + radius2) / 2.0f);
		shape->SetRadius1(radius1);
		shape->SetRadius2(radius2);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = node->GetNodeTM(0) * tm;
		Point3 center = ltm.GetTrans();

		Matrix3 rot = ltm;
		rot.NoTrans();
		rot.NoScale();

		float distFromCenter = len*bhkAppScaleFactor / 2.0f;

		Point3 pt1 = ((TransMatrix(Point3(0.0f, 0.0f, +distFromCenter)) * rot).GetTrans() + center) / bhkAppScaleFactor;
		Point3 pt2 = ((TransMatrix(Point3(0.0f, 0.0f, -distFromCenter)) * rot).GetTrans() + center) / bhkAppScaleFactor;
		shape->SetFirstPoint(TOVECTOR3(pt1));
		shape->SetSecondPoint(TOVECTOR3(pt2));

		retval = StaticCast<bhkShape>(shape);
	}
	return retval;
}


bhkShapeRef Exporter::makeTriStripsShape(INode *node, Matrix3& tm, int mtlDefault)
{
	TimeValue t = 0;
	Matrix3 sm = ScaleMatrix(GetScale(tm));

	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	ObjectState os = node->EvalWorldState(t);

	TriObject *tri = (TriObject *)os.obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
	if (!tri)
		return false;

	Mesh &mesh = tri->GetMesh();
	mesh.buildNormals();

	bhkNiTriStripsShapeRef shape = makeTriStripsShape(mesh, sm, mtlDefault);

	int lyr = OL_STATIC;
	npGetProp(node, NP_HVK_LAYER, lyr, NP_DEFAULT_HVK_LAYER);
	OblivionLayer obv_layer; SkyrimLayer sky_layer;
	GetHavokLayersFromIndex(lyr, (int*)&obv_layer, (int*)&sky_layer);

	shape->SetNumDataLayers(1);
	shape->SetOblivionLayer(0, obv_layer);

	int mtl = NP_DEFAULT_HVK_MATERIAL;
	npGetProp(node, NP_HVK_MATERIAL, mtl, NP_DEFAULT_HVK_MATERIAL);
	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
	shape->SetMaterial(material);
	shape->SetSkyrimMaterial(skyrimMaterial);

	return StaticCast<bhkShape>(shape);
}

bhkShapeRef	Exporter::makeConvexShape(INode *node, Object* obj, Matrix3& tm, int mtlDefault)
{
	bhkShapeRef shape;

	return shape;
}

static void AccumulateNodesFromGroup(INode *node, INodeTab& map)
{
	map.Append(1, &node);
	if (node->IsGroupHead()) {
		for (int i = 0; i < node->NumberOfChildren(); i++)
			AccumulateNodesFromGroup(node->GetChildNode(i), map);
	}
}

Exporter::Result Exporter::scanForCollision(INode *node)
{
	if (node == NULL || (node->IsHidden() && !mExportHidden))
		return Exporter::Skip;
	// Get the bhk RigidBody modifier if available and then get the picked node.

	TSTR nodeName = node->GetName();
	if (Modifier * mod = GetbhkCollisionModifier(node)) {
		if (IParamBlock2* pblock = (IParamBlock2*)mod->GetReference(0)) {
			if (INode *collMesh = pblock->GetINode(0, 0)) {
				mCollisionNodes.insert(collMesh);
			}
			else {
				//if ( node->IsGroupMember() ){
				   // // skip groups ???
				//} else {
				   //if (mSceneCollisionNode != NULL) {
				   //   if (mExportCollision) {
				   //	  throw runtime_error("There are more than one Collision mesh found at the Scene Level.");
				   //   }
				   //} else {
				   //   mSceneCollisionNode = node;
				   //}
				//}
			}
		}
	}
	// Check self to see if is one of our bhkXXXObject classes
	if (Object* obj = node->GetObjectRef())
	{
		if (obj->ClassID() == BHKLISTOBJECT_CLASS_ID)
		{
			mCollisionNodes.insert(node);

			// process all children of groups as collision
			INodeTab map;
			const int PB_MESHLIST = 1;
			IParamBlock2* pblock2 = obj->GetParamBlockByID(0);
			int nBlocks = pblock2->Count(PB_MESHLIST);
			for (int i = 0; i < pblock2->Count(PB_MESHLIST); i++) {
				INode *tnode = NULL;
				pblock2->GetValue(PB_MESHLIST, 0, tnode, FOREVER, i);
				if (tnode != NULL && (!tnode->IsHidden() || mExportHidden)) {
					AccumulateNodesFromGroup(tnode, map);
				}
			}
			for (int i = 0; i < map.Count(); i++) {
				INode *cnode = map[i];
				if (!node->IsGroupHead())
					mCollisionNodes.insert(cnode);
				markAsHandled(cnode); // dont process collision since the list will 
			}
		}
		else if (obj->SuperClassID() == HELPER_CLASS_ID &&
			obj->ClassID().PartB() == BHKRIGIDBODYCLASS_DESC.PartB())
		{
			mCollisionNodes.insert(node);
		}
		else
		{
			Modifier* mod = GetbhkCollisionModifier(node);
			if (mod != NULL)
			{
				mCollisionNodes.insert(node);
			}
		}
	}
	// process legacy collision 
	if (npIsCollision(node))
	{
		mCollisionNodes.insert(node);
	}

	for (int i = 0; i < node->NumberOfChildren(); i++) {
		scanForCollision(node->GetChildNode(i));
	}
	return Exporter::Ok;
}

bool Exporter::isHandled(INode *node)
{
	return (mHandledNodes.find(node) != mHandledNodes.end());
}

bool Exporter::markAsHandled(INode* node)
{
	mHandledNodes.insert(node);
	return true;
}

bool Exporter::isCollision(INode *node)
{
	return (mCollisionNodes.find(node) != mCollisionNodes.end());
}

static void AccumulateSubShapesFromGroup(INode *node, INodeTab& packedShapes, INodeTab& cmsdShapes, INodeTab& otherShapes)
{
	ObjectState os = node->EvalWorldState(0);
	if (node->IsGroupHead()) {
		for (int i = 0; i < node->NumberOfChildren(); i++)
			AccumulateSubShapesFromGroup(node->GetChildNode(i), packedShapes, cmsdShapes, otherShapes);
	}
	else if ((os.obj->ClassID() == SCUBA_CLASS_ID)
		|| (os.obj->ClassID() == Class_ID(BOXOBJ_CLASS_ID, 0))
		|| (os.obj->ClassID() == Class_ID(SPHERE_CLASS_ID, 0))
		|| (os.obj->ClassID() == bhkBoxObject_CLASS_ID)
		|| (os.obj->ClassID() == bhkSphereObject_CLASS_ID)
		|| (os.obj->ClassID() == BHKCAPSULEOBJECT_CLASS_ID)
		|| (os.obj->ClassID() == BHKLISTOBJECT_CLASS_ID)
		)
	{
		otherShapes.Append(1, &node);
	}
	else if (os.obj->ClassID() == BHKPROXYOBJECT_CLASS_ID)
	{
		enum { list_params, bv_mesh, };  // pblock2 ID
		enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
		enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

		int type = bv_type_none;
		if (IParamBlock2* pblock2 = os.obj->GetParamBlockByID(list_params))
			pblock2->GetValue(PB_BOUND_TYPE, 0, type, FOREVER, 0);
		if (type == bv_type_packed)
			packedShapes.Insert(0, 1, &node);
		else if (type == bv_type_cmsd)
			cmsdShapes.Insert(0, 1, &node);
		else
			otherShapes.Insert(0, 1, &node);
	}
	else if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{
		if (Modifier* mod = GetbhkCollisionModifier(node))
		{
			enum { havok_params };
			enum { PB_BOUND_TYPE, PB_MATERIAL, };
			enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

			int type = bv_type_none;
			if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
				pblock2->GetValue(PB_BOUND_TYPE, 0, type, FOREVER, 0);
			if (type == bv_type_packed)
				packedShapes.Insert(0, 1, &node);
			else if (type == bv_type_cmsd)
				cmsdShapes.Insert(0, 1, &node);
			else
				otherShapes.Insert(0, 1, &node);
		}
		else
		{
			packedShapes.Insert(0, 1, &node);
		}
	}
}

bhkShapeRef Exporter::makeListShape(INode *node, Matrix3& tm, bhkRigidBodyRef body, int mtlDefault)
{
	const int PB_MATERIAL = 0;
	const int PB_MESHLIST = 1;
	IParamBlock2* pblock2 = node->GetObjectRef()->GetParamBlockByID(0);
	int nBlocks = pblock2->Count(PB_MESHLIST);
	if (nBlocks > 0)
	{
		if (bhkRigidBodyInterface *irb = (bhkRigidBodyInterface *)node->GetObjectRef()->GetInterface(BHKRIGIDBODYINTERFACE_DESC))
		{
			float mass = irb->GetMass(0);
			float frict = irb->GetFriction(0);
			float resti = irb->GetRestitution(0);
			int lyr = irb->GetLayer(0);
			int msys = irb->GetMotionSystem(0);
			int qtype = irb->GetQualityType(0);
			float lindamp = irb->GetLinearDamping(0);
			float angdamp = irb->GetAngularDamping(0);
			float maxlinvel = irb->GetMaxLinearVelocity(0);
			float maxangvel = irb->GetMaxAngularVelocity(0);
			float pendepth = irb->GetPenetrationDepth(0);

			OblivionLayer obv_layer; SkyrimLayer sky_layer;
			GetHavokLayersFromIndex(lyr, (int*)&obv_layer, (int*)&sky_layer);

			body->SetLayer(obv_layer);
			body->SetLayerCopy(obv_layer);
			body->SetSkyrimLayer(sky_layer);
			body->SetMotionSystem(MotionSystem(msys));
			body->SetQualityType(MotionQuality(qtype));
			body->SetMass(mass);
			body->SetLinearDamping(lindamp);
			body->SetAngularDamping(angdamp);
			body->SetFriction(frict);
			body->SetRestitution(resti);
			body->SetMaxLinearVelocity(maxlinvel);
			body->SetMaxAngularVelocity(maxangvel);
			body->SetPenetrationDepth(pendepth);
		}
		// Accumulate potential bhkPackedStripShapes

		bhkListShapeRef shape = new bhkListShape();
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		// Locate all packed shapes for efficient mopp packing
		INodeTab packedShapes, cmsdShapes, otherShapes;
		for (int i = 0; i < nBlocks; i++) {
			INode *tnode = NULL;
			pblock2->GetValue(PB_MESHLIST, 0, tnode, FOREVER, i);
			if (tnode != NULL)
				AccumulateSubShapesFromGroup(tnode, packedShapes, cmsdShapes, otherShapes);
		}

		vector<bhkShapeRef> shapes;

		if (packedShapes.Count() > 0) {
			if (bhkShapeRef subshape = makeModPackedTriStripShape(packedShapes, tm, mtl))
				shapes.push_back(subshape);
		}
		if (cmsdShapes.Count() > 0) {
			if (bhkShapeRef subshape = makeModCMSD(cmsdShapes, tm, mtl))
				shapes.push_back(subshape);
		}

		for (int i = 0; i < otherShapes.Count(); i++) {
			INode *tnode = otherShapes[i];
			if (tnode != NULL && (!tnode->IsHidden() || mExportHidden))
			{
				bhkShapeRef subshape = makeCollisionShape(tnode, tm, body, mtl);
				if (subshape)
					shapes.push_back(subshape);
			}
		}
		shape->SetSubShapes(shapes);

		if (shapes.size() == 1) // ignore the list when only one object is present
		{
			return shapes[0];
		}
		else if (!shapes.empty())
		{
			return bhkShapeRef(shape);
		}
	}
	return bhkShapeRef();
}

bhkShapeRef Exporter::makeProxyShape(INode *node, Object *obj, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd, };  // pblock ID

	bhkShapeRef shape;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		int bvType = bv_type_none;
		pblock2->GetValue(PB_BOUND_TYPE, 0, bvType, FOREVER, 0);
		if (bvType != bv_type_none)
		{
			if (TriObject *triObj = (TriObject *)obj->ConvertToType(0, triObjectClassID))
			{
				Mesh& mesh = triObj->GetMesh();
				switch (bvType)
				{
				case bv_type_box:
					shape = makeProxyBoxShape(node, obj, mesh, tm, mtlDefault);
					break;

					//case bv_type_sphere:
					//	shape = makeProxySphereShape(node, obj, mesh, tm);
					//	break;

				case bv_type_shapes:
					shape = makeProxyTriStripShape(node, obj, mesh, tm, mtlDefault);
					break;

				case bv_type_packed:
					shape = makeProxyPackedTriStripShape(node, obj, mesh, tm, mtlDefault);
					break;

				case bv_type_convex:
					shape = makeProxyConvexShape(node, obj, mesh, tm, mtlDefault);
					break;

				case bv_type_capsule:
					shape = makeProxyCapsuleShape(node, obj, mesh, tm, mtlDefault);
					break;

				case bv_type_obb:
					shape = makeProxyOBBShape(node, obj, mesh, tm, mtlDefault);
					break;

				case bv_type_cmsd:
					shape = makeProxyCMSD(node, obj, mesh, tm, mtlDefault);
					break;
				}
			}
		}
	}
	return shape;
}

bhkShapeRef	Exporter::makeProxyBoxShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef retval;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		Box3 box; box.Init();
		CalcAxisAlignedBox(mesh, box, NULL);

		float length = 0, width = 0, height = 0;
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);

		bhkBoxShapeRef shape = new bhkBoxShape();
		Vector3 dim(box.Max().x - box.Min().x, box.Max().y - box.Min().y, box.Max().z - box.Min().z);
		dim /= (bhkAppScaleFactor * 2);

		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);
		shape->SetDimensions(dim);

		Matrix3 ltm = /*GetLocalTM(node) * */TransMatrix(box.Center()) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}


bhkShapeRef	Exporter::makeProxyOBBShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef retval;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		Matrix3 rtm(true);
		Point3 center;
		float udim, vdim, ndim;
		CalcOrientedBox(mesh, udim, vdim, ndim, center, rtm);

		float length = 0, width = 0, height = 0;
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);

		bhkBoxShapeRef shape = new bhkBoxShape();
		Vector3 dim(udim, vdim, ndim);
		dim /= (bhkAppScaleFactor * 2);
		shape->SetDimensions(dim);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = rtm * tm; // Translation already done in CalcOrientedBox().
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}


bhkShapeRef	Exporter::makeProxyCapsuleShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef retval;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		Point3 center = Point3::Origin;
		Point3 pt1 = Point3::Origin;
		Point3 pt2 = Point3::Origin;
		Point3 trans_pt1 = Point3::Origin;
		Point3 trans_pt2 = Point3::Origin;
		float r1 = 0.0f;
		float r2 = 0.0f;
		CalcCapsule(mesh, pt1, pt2, r1, r2); // Both R's are the same.
		center = (pt1 + pt2) / 2;
		trans_pt1 = pt1 - center;
		trans_pt2 = pt2 - center;

		if (bhkCapsuleShapeRef shape = new bhkCapsuleShape())
		{
			shape->SetRadius(r1 / bhkAppScaleFactor);
			shape->SetRadius1(r1 / bhkAppScaleFactor);
			shape->SetRadius2(r1 / bhkAppScaleFactor);
			shape->SetFirstPoint(TOVECTOR3(trans_pt1 / bhkAppScaleFactor));
			shape->SetSecondPoint(TOVECTOR3(trans_pt2 / bhkAppScaleFactor));
			shape->SetMaterial(material);
			shape->SetSkyrimMaterial(skyrimMaterial);

			Matrix3 ltm = TransMatrix(center) * tm;
			if (ltm.IsIdentity())
			{
				retval = StaticCast<bhkShape>(shape);
			}
			else
			{
				ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

				bhkTransformShapeRef transform = new bhkTransformShape();
				transform->SetTransform(TOMATRIX4(ltm).Transpose());
				transform->SetShape(shape);
				transform->SetMaterial(material);
				transform->SetSkyrimMaterial(skyrimMaterial);
				retval = StaticCast<bhkShape>(transform);
			}
		}
	}
	return retval;
}


bhkShapeRef	Exporter::makeProxySphereShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef shape;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		//Matrix3 tm = GetLocalTM(node) * TransMatrix(box.Center());

	}
	return shape;
}

bhkShapeRef	Exporter::makeProxyConvexShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef shape;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		if (bhkConvexVerticesShapeRef convShape = makeConvexShape(mesh, tm, mtlDefault))
		{
			int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
			HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
			GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
			convShape->SetMaterial(material);
			convShape->SetSkyrimMaterial(skyrimMaterial);
			shape = StaticCast<bhkShape>(convShape);
		}
	}
	return shape;
}

bhkShapeRef	Exporter::makeProxyTriStripShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef shape;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);

		// Transform location
		Mesh localmesh(mesh);
		MNMesh tmpMesh(localmesh);
		tmpMesh.Transform(tm);
		tmpMesh.buildNormals();
		tmpMesh.OutToTri(localmesh);
		localmesh.buildNormals();

		Matrix3 ident(true);
		bhkNiTriStripsShapeRef trishape = makeTriStripsShape(localmesh, ident, mtl);
		trishape->SetMaterial(material);
		trishape->SetSkyrimMaterial(skyrimMaterial);

		shape = StaticCast<bhkShape>(trishape);
	}
	return shape;
}

bhkShapeRef	Exporter::makeProxyPackedTriStripShape(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { list_params, bv_mesh, };  // pblock2 ID
	enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, };
	enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef shape;
	if (IParamBlock2* pblock2 = obj->GetParamBlockByID(list_params))
	{
		int mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);

		int layer = NP_DEFAULT_HVK_LAYER;
		int filter = NP_DEFAULT_HVK_FILTER;

		// Transform location
		Mesh localmesh(mesh);
		MNMesh tmpMesh(localmesh);
		tmpMesh.Transform(tm);
		tmpMesh.buildNormals();
		tmpMesh.OutToTri(localmesh);
		localmesh.buildNormals();

		Matrix3 ident(true);
		bhkPackedNiTriStripsShapeRef trishape = makePackedTriStripsShape(localmesh, ident, mtlDefault, layer, filter);
		shape = StaticCast<bhkShape>(makeTreeShape(trishape, mtl));
	}
	return shape;
}

bhkShapeRef	Exporter::makeModifierShape(INode *node, Object* obj, Modifier* mod, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

	bhkShapeRef shape;

	const Mesh* mesh = NULL;
	int type = bv_type_none;

	node->EvalWorldState(0);
	if (bhkHelperInterface* bhkHelp = (bhkHelperInterface*)mod->GetInterface(BHKHELPERINTERFACE_DESC))
	{
		mesh = bhkHelp->GetMesh();
	}
	else
	{
		if (TriObject *tri = (TriObject *)obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)))
		{
			mesh = &tri->GetMesh();
		}
	}
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtlDefault = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		type = GetPBValue(pblock2, PB_BOUND_TYPE, type);
	}
	switch (type)
	{
	default:
	case bv_type_none:
		break;

	case bv_type_box:
		shape = makeModBoxShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_sphere:
		shape = makeModSphereShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_capsule:
		shape = makeModCapsuleShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_obb:
		shape = makeModOBBShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_shapes:
		shape = makeModTriStripShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_convex:
		shape = makeModConvexShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_packed:
		shape = makeModPackedTriStripShape(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;

	case bv_type_cmsd:
		shape = makeModCMSD(node, mod, const_cast<Mesh&>(*mesh), tm, mtlDefault);
		break;
	}
	return shape;
}

bhkShapeRef	Exporter::makeModBoxShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID
	int mtl = mtlDefault;

	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	bhkShapeRef retval;
	if (bhkBoxShapeRef shape = new bhkBoxShape())
	{
		Box3 box; box.Init();
		CalcAxisAlignedBox(mesh, box, NULL);

		Vector3 dim(box.Max().x - box.Min().x, box.Max().y - box.Min().y, box.Max().z - box.Min().z);
		dim /= (bhkAppScaleFactor * 2);
		shape->SetDimensions(dim);

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = TransMatrix(box.Center()) * node->GetNodeTM(0) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}

bhkShapeRef	Exporter::makeModSphereShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

	int mtl = mtlDefault;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	bhkShapeRef retval;

	Point3 center = Point3::Origin;
	float radius = 0.0f;
	CalcCenteredSphere(mesh, center, radius);

	if (bhkSphereShapeRef shape = new bhkSphereShape())
	{
		shape->SetRadius(radius / bhkAppScaleFactor);

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = TransMatrix(center) * node->GetObjTMAfterWSM(0) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}

bhkShapeRef	Exporter::makeModCapsuleShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

	node->EvalWorldState(0);

	int mtl = mtlDefault;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	bhkShapeRef retval;

	Point3 center = Point3::Origin;
	Point3 pt1 = Point3::Origin;
	Point3 pt2 = Point3::Origin;
	Point3 trans_pt1 = Point3::Origin;
	Point3 trans_pt2 = Point3::Origin;
	float r1 = 0.0f;
	float r2 = 0.0f;
	CalcCapsule(mesh, pt1, pt2, r1, r2); // Both R's are the same.
	center = (pt1 + pt2) / 2;
	trans_pt1 = pt1 - center;
	trans_pt2 = pt2 - center;

	if (bhkCapsuleShapeRef shape = new bhkCapsuleShape())
	{
		shape->SetRadius(r1 / bhkAppScaleFactor);
		shape->SetRadius1(r1 / bhkAppScaleFactor);
		shape->SetRadius2(r1 / bhkAppScaleFactor);
		shape->SetFirstPoint(TOVECTOR3(trans_pt1 / bhkAppScaleFactor));
		shape->SetSecondPoint(TOVECTOR3(trans_pt2 / bhkAppScaleFactor));

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = TransMatrix(center) * node->GetObjTMAfterWSM(0) * tm;
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(HavokMaterial(material));
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}

bhkShapeRef	Exporter::makeModConvexShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

	int mtl = mtlDefault;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	Matrix3 ltm = node->GetObjTMAfterWSM(0) * tm;

	bhkShapeRef shape;
	if (bhkConvexVerticesShapeRef convShape = makeConvexShape(mesh, ltm, mtl))
	{
		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		convShape->SetMaterial(material);
		convShape->SetSkyrimMaterial(skyrimMaterial);

		shape = StaticCast<bhkShape>(convShape);
	}
	return shape;
}

bhkShapeRef	Exporter::makeModTriStripShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, bv_type_cmsd };  // pblock ID

	int mtl = mtlDefault;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	Matrix3 ltm = node->GetObjTMAfterWSM(0) * tm;

	bhkShapeRef shape;
	if (bhkNiTriStripsShapeRef trishape = makeTriStripsShape(mesh, ltm, mtl))
	{
		shape = StaticCast<bhkShape>(trishape);
	}
	return shape;
}

bhkShapeRef	Exporter::makeModPackedTriStripShape(INode *tnode, Matrix3& tm, int mtlDefault)
{
	INodeTab map;
	AccumulateNodesFromGroup(tnode, map);
	return makeModPackedTriStripShape(map, tm, mtlDefault);
}

bhkShapeRef	Exporter::makeModPackedTriStripShape(INodeTab &map, Matrix3& tm, int mtlDefault)
{
	// Need to separate the vertices based on material.  
	typedef vector<Triangle> Triangles;

	// setup shape data
	vector<Vector3> verts;
	vector<Vector3> norms;
	Triangles		tris;
	int voff = 0;

	int mtl = mtlDefault;

	vector<OblivionSubShape> subshapes;

	for (int i = 0; i < map.Count(); ++i) {

		INode *node = map[i];

		// skip group heads
		if (node->IsGroupHead() || (node->IsHidden() && !mExportHidden))
			continue;

		ObjectState os = node->EvalWorldState(0);

		int layer = NP_DEFAULT_HVK_LAYER;
		int filter = NP_DEFAULT_HVK_FILTER;
		Mesh *mesh = NULL;

		bool bNodeTransform = true;
		if (os.obj->ClassID() == BHKPROXYOBJECT_CLASS_ID)
		{
			enum { list_params, bv_mesh, };  // pblock2 ID
			enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
			enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

			int type = bv_type_none;
			if (IParamBlock2* pblock2 = os.obj->GetParamBlockByID(list_params))
			{
				pblock2->GetValue(PB_BOUND_TYPE, 0, type, FOREVER, 0);
				if (type == bv_type_packed || type == bv_type_cmsd)
				{
					mtlDefault = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
					pblock2->GetValue(PB_FILTER, 0, filter, FOREVER, 0);
					pblock2->GetValue(PB_LAYER, 0, layer, FOREVER, 0);
					if (TriObject *tri = (TriObject *)os.obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)))
						mesh = const_cast<Mesh*>(&tri->GetMesh());
					bNodeTransform = false;
				}
			}
		}
		else if (Modifier* mod = GetbhkCollisionModifier(node))
		{
			enum { havok_params, opt_params, clone_params, subshape_params };  // pblock ID
			enum { PB_BOUND_TYPE, PB_MATERIAL, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
			enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

			if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params)) {
				pblock2->GetValue(PB_FILTER, INFINITE, filter, FOREVER, 0);
				pblock2->GetValue(PB_LAYER, INFINITE, layer, FOREVER, 0);
			}
			if (bhkHelperInterface* bhkHelp = (bhkHelperInterface*)mod->GetInterface(BHKHELPERINTERFACE_DESC))
				mesh = const_cast<Mesh*>(bhkHelp->GetMesh());
		}
		else
		{
			if (TriObject *tri = (TriObject *)os.obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)))
				mesh = const_cast<Mesh*>(&tri->GetMesh());
		}
		if (mesh == NULL)
			continue;

		Matrix3 ltm = (node->GetObjTMAfterWSM(0) * tm);
		int vi[3];
		if (TMNegParity(ltm)) {
			vi[0] = 2; vi[1] = 1; vi[2] = 0;
		}
		else {
			vi[0] = 0; vi[1] = 1; vi[2] = 2;
		}

		int nvert = mesh->getNumVerts();
		int nface = mesh->getNumFaces();
		mesh->buildNormals();

		for (int i = 0; i < nvert; ++i)
		{
			Point3 vert = (mesh->getVert(i) * ltm) / bhkAppScaleFactor;
			verts.push_back(TOVECTOR3(vert));
		}
		for (int i = 0; i < nface; ++i)
		{
			Point3 norm = (mesh->getFaceNormal(i) * ltm) / bhkAppScaleFactor;
			norms.push_back(TOVECTOR3(mesh->getFaceNormal(i)));

			Triangle tri;
			Face& face = mesh->faces[i];
			tri[0] = (USHORT)face.getVert(0) + voff;
			tri[1] = (USHORT)face.getVert(1) + voff;
			tri[2] = (USHORT)face.getVert(2) + voff;
			tris.push_back(tri);
		}
		voff += nvert;

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);

		OblivionLayer obv_layer; SkyrimLayer sky_layer;
		GetHavokLayersFromIndex(layer, (int*)&obv_layer, (int*)&sky_layer);

		OblivionSubShape subshape;
		subshape.layer = obv_layer;
		subshape.material = material;
		subshape.colFilter = filter;
		subshape.numVertices = nvert;
		subshapes.push_back(subshape);
	}

	hkPackedNiTriStripsDataRef data = new hkPackedNiTriStripsData();
	data->SetNumFaces(tris.size());
	data->SetVertices(verts);
	data->SetTriangles(tris);
	data->SetNormals(norms);

	// setup shape
	bhkPackedNiTriStripsShapeRef shape = new bhkPackedNiTriStripsShape();
	shape->SetData(data);

	shape->SetSubShapes(subshapes);
	data->SetSubShapes(subshapes);

	return StaticCast<bhkShape>(makeTreeShape(shape, mtl));
}

bhkShapeRef	Exporter::makeModPackedTriStripShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params, opt_params, clone_params, subshape_params };  // pblock ID
	enum { PB_BOUND_TYPE, PB_MATERIAL, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, };  // pblock ID

	int mtl = mtlDefault;
	int layer = NP_DEFAULT_HVK_LAYER;
	int filter = NP_DEFAULT_HVK_FILTER;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		pblock2->GetValue(PB_FILTER, INFINITE, filter, FOREVER, 0);
		pblock2->GetValue(PB_LAYER, INFINITE, layer, FOREVER, 0);
	}

	Matrix3 ltm = node->GetObjTMAfterWSM(0) * tm;

	bhkShapeRef shape;
	if (bhkPackedNiTriStripsShapeRef trishape = makePackedTriStripsShape(mesh, ltm, mtl, layer, filter))
	{
		shape = StaticCast<bhkShape>(trishape);
	}
	return shape;
}
bhkShapeRef	Exporter::makeModOBBShape(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, };  // pblock ID
	int mtl = mtlDefault;

	node->EvalWorldState(0);
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
	}

	bhkShapeRef retval;
	if (bhkBoxShapeRef shape = new bhkBoxShape())
	{
		Matrix3 rtm(true);
		Point3 center;
		float udim, vdim, ndim;
		CalcOrientedBox(mesh, udim, vdim, ndim, center, rtm);

		Vector3 dim(udim, vdim, ndim);
		dim /= (bhkAppScaleFactor * 2);
		shape->SetDimensions(dim);

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);
		shape->SetMaterial(material);
		shape->SetSkyrimMaterial(skyrimMaterial);

		Matrix3 ltm = rtm * node->GetNodeTM(0) * tm; // Translation already done in CalcOrientedBox().
		if (ltm.IsIdentity())
		{
			retval = StaticCast<bhkShape>(shape);
		}
		else
		{
			ltm.SetTrans(ltm.GetTrans() / bhkAppScaleFactor);

			bhkTransformShapeRef transform = new bhkTransformShape();
			transform->SetTransform(TOMATRIX4(ltm).Transpose());
			transform->SetShape(shape);
			transform->SetMaterial(material);
			transform->SetSkyrimMaterial(skyrimMaterial);
			retval = StaticCast<bhkShape>(transform);
		}
	}
	return retval;
}

bhkShapeRef Exporter::makeProxyCMSD(INode *node, Object *obj, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	return makeProxyPackedTriStripShape(node, obj, mesh, tm, mtlDefault);
}

bhkShapeRef Exporter::makeModCMSD(INode *node, Modifier* mod, Mesh& mesh, Matrix3& tm, int mtlDefault)
{
	enum { havok_params, opt_params, clone_params, subshape_params };  // pblock ID
	enum { PB_BOUND_TYPE, PB_MATERIAL, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
	enum { bv_type_none, bv_type_box, bv_type_sphere, bv_type_capsule, bv_type_shapes, bv_type_convex, bv_type_packed, bv_type_obb, };  // pblock ID

	int mtl = mtlDefault;
	int layer = NP_DEFAULT_HVK_LAYER;
	int filter = NP_DEFAULT_HVK_FILTER;
	if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params))
	{
		mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
		pblock2->GetValue(PB_FILTER, INFINITE, filter, FOREVER, 0);
		pblock2->GetValue(PB_LAYER, INFINITE, layer, FOREVER, 0);
	}

	Matrix3 ltm = node->GetObjTMAfterWSM(0) * tm;

	bhkShapeRef shape;
	if (bhkCompressedMeshShapeRef trishape = makeCompressedMeshShape(mesh, ltm, mtl, layer, filter))
	{
		shape = StaticCast<bhkShape>(makeTreeShape(trishape, mtl));
	}
	return shape;
}

bhkShapeRef Exporter::makeModCMSD(INode *tnode, Matrix3& tm, int mtlDefault)
{
	return makeModPackedTriStripShape(tnode, tm, mtlDefault);
}

struct bhkCMSDMaterial_equal : std::unary_function<bhkCMSDMaterial, bool>
{
	const bhkCMSDMaterial &idx_;
	bhkCMSDMaterial_equal(const bhkCMSDMaterial &idx) : idx_(idx) {}
	bool operator()(const bhkCMSDMaterial &arg) const { 
		return arg.skyrimMaterial == idx_.skyrimMaterial && arg.skyrimLayer == idx_.skyrimLayer; 
	}
};

bhkShapeRef Exporter::makeModCMSD(INodeTab &map, Matrix3& tm, int mtlDefault)
{
	// Need to separate the vertices based on material.  
	typedef vector<Triangle> Triangles;

	// setup shape
	vector<bhkCMSDChunk> subshapes; subshapes.reserve(map.Count());
	vector<bhkCMSDTransform> transforms; transforms.reserve(map.Count());
	vector<bhkCMSDMaterial> materials; materials.reserve(map.Count());
	Box3 bounds;

	for (int i = 0; i < map.Count(); ++i) {

		INode *node = map[i];

		// skip group heads
		if (node->IsGroupHead() || (node->IsHidden() && !mExportHidden))
			continue;

		ObjectState os = node->EvalWorldState(0);

		int mtl = mtlDefault;
		int layer = NP_DEFAULT_HVK_LAYER;
		int filter = NP_DEFAULT_HVK_FILTER;
		Mesh *mesh = NULL;

		bool bNodeTransform = true;
		if (os.obj->ClassID() == BHKPROXYOBJECT_CLASS_ID)
		{
			enum { list_params, bv_mesh, };  // pblock2 ID
			enum { PB_MATERIAL, PB_MESHLIST, PB_BOUND_TYPE, PB_CENTER, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
			enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

			int type = bv_type_none;
			if (IParamBlock2* pblock2 = os.obj->GetParamBlockByID(list_params))
			{
				pblock2->GetValue(PB_BOUND_TYPE, 0, type, FOREVER, 0);
				if (type == bv_type_cmsd)
				{
					mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
					pblock2->GetValue(PB_FILTER, INFINITE, filter, FOREVER, 0);
					pblock2->GetValue(PB_LAYER, INFINITE, layer, FOREVER, 0);
					if (TriObject *tri = (TriObject *)os.obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)))
						mesh = const_cast<Mesh*>(&tri->GetMesh());
					bNodeTransform = false;
				}
			}
		}
		else if (Modifier* mod = GetbhkCollisionModifier(node))
		{
			enum { havok_params, opt_params, clone_params, subshape_params };  // pblock ID
			enum { PB_BOUND_TYPE, PB_MATERIAL, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
			enum { bv_type_none, bv_type_box, bv_type_shapes, bv_type_packed, bv_type_convex, bv_type_capsule, bv_type_obb, bv_type_cmsd };  // pblock ID

			if (IParamBlock2* pblock2 = mod->GetParamBlockByID(havok_params)) {
				mtl = GetPBValue(pblock2, PB_MATERIAL, mtlDefault);
				pblock2->GetValue(PB_FILTER, INFINITE, filter, FOREVER, 0);
				pblock2->GetValue(PB_LAYER, INFINITE, layer, FOREVER, 0);
			}
			if (bhkHelperInterface* bhkHelp = (bhkHelperInterface*)mod->GetInterface(BHKHELPERINTERFACE_DESC))
				mesh = const_cast<Mesh*>(bhkHelp->GetMesh());
		}
		else
		{
			if (TriObject *tri = (TriObject *)os.obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)))
				mesh = const_cast<Mesh*>(&tri->GetMesh());
		}
		if (mesh == NULL)
			continue;

		// Need to separate the vertices based on material.  
		typedef vector<Triangle> Triangles;

		// setup shape data
		vector<Vector3> verts;
		vector<Vector3> norms;
		Triangles		tris;

		Matrix3 ltm = (node->GetObjTMAfterWSM(0) * tm);
		int vi[3];
		if (TMNegParity(ltm)) {
			vi[0] = 2; vi[1] = 1; vi[2] = 0;
		}
		else {
			vi[0] = 0; vi[1] = 1; vi[2] = 2;
		}

		int nvert = mesh->getNumVerts();
		int nface = mesh->getNumFaces();
		mesh->buildNormals();
		mesh->buildBoundingBox();
		Box3 aabb = mesh->getBoundingBox();
		Vector3 ptoffset = TOVECTOR3(aabb.Min()) / bhkAppScaleFactor;
		bounds += aabb;

		tris.resize(nface);
		verts.resize(nvert);
		norms.resize(nface);
		for (int i = 0; i < nvert; ++i) {
			verts[i] = TOVECTOR3((mesh->getVert(i) * ltm) / bhkAppScaleFactor);
		}
		for (int i = 0; i < nface; ++i)
		{
			Triangle& tri = tris[i];
			norms[i] = TOVECTOR3(mesh->getFaceNormal(i));
			Face& face = mesh->faces[i];
			tri[0] = (USHORT)face.getVert(0);
			tri[1] = (USHORT)face.getVert(1);
			tri[2] = (USHORT)face.getVert(2);
		}
		TriStrips strips;
		strippify(strips, verts, norms, tris);
		vector<unsigned short > indices;
		indices.reserve(nface * 3);
		for (auto itr = strips.begin(); itr != strips.end(); ++itr) {
			auto& strip = (*itr);
			for (int j = 0; j < strip.size(); ++j) {
				indices.push_back(strip[j]);
			}
		}

		HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
		GetHavokMaterialsFromIndex(mtl, (int*)&material, (int*)&skyrimMaterial);

		OblivionLayer oblivlayer; SkyrimLayer skyrimLayer;
		GetHavokLayersFromIndex(layer, (int*)&oblivlayer, (int*)&skyrimLayer);
		
		auto subshapeitr = subshapes.insert(subshapes.end(), bhkCMSDChunk());
		bhkCMSDChunk& subshape = (*subshapeitr);

		//auto transitr = transforms.insert(transforms.end(), bhkCMSDTransform());
		//bhkCMSDTransform& subtrans = (*transitr);

		subshape.transformIndex = 0;

		bhkCMSDMaterial cmsdMaterial;
		cmsdMaterial.skyrimMaterial = skyrimMaterial;
		cmsdMaterial.skyrimLayer = skyrimLayer;
		const auto& itr = std::find_if(materials.begin(), materials.end(), bhkCMSDMaterial_equal(cmsdMaterial));
		if (itr == materials.end()) {
			subshape.materialIndex = materials.size();
			materials.push_back(cmsdMaterial);

			bhkCMSDTransform trans;
			trans.translation = TOVECTOR4(Point3());
			trans.rotation = TOQUATXYZW(Quat());
			transforms.push_back(trans);
			subshape.transformIndex = subshape.materialIndex;
		} else {
			subshape.materialIndex = std::distance(materials.begin(), itr);
			subshape.transformIndex = subshape.materialIndex;
		}

		subshape.translation = ptoffset;
		subshape.numVertices = verts.size();
		subshape.vertices.reserve(subshape.numVertices * 3);
		for (auto itr = verts.begin(); itr != verts.end(); ++itr) {
			const auto& v = (*itr);
			subshape.vertices.push_back(static_cast<unsigned short>((v.x - ptoffset.x) * 1000.0f));
			subshape.vertices.push_back(static_cast<unsigned short>((v.y - ptoffset.y) * 1000.0f));
			subshape.vertices.push_back(static_cast<unsigned short>((v.z - ptoffset.z) * 1000.0f));
		}
		subshape.numIndices = indices.size();
		subshape.indices = indices;
		subshape.numStrips = strips.size();
		subshape.strips.reserve(subshape.numStrips);
		for (auto itr = strips.begin(); itr != strips.end(); ++itr)
			subshape.strips.push_back((*itr).size());
	}

	bhkCompressedMeshShapeRef shape = new bhkCompressedMeshShape();
	bhkCompressedMeshShapeDataRef data = new bhkCompressedMeshShapeData();
	data->SetChunkTransforms(transforms);
	data->SetChunkMaterials(materials);
	data->SetChunks(subshapes);
	data->SetBoundsMin(TOVECTOR4(bounds.Min(), 0.0f) / bhkAppScaleFactor);
	data->SetBoundsMax(TOVECTOR4(bounds.Max(), bhkAppScaleFactor) / bhkAppScaleFactor);
	shape->SetData(data);

	return StaticCast<bhkShape>(makeTreeShape(shape, mtlDefault));
}

// returns partially construted Compessed Mesh Shape.  Will be passed to NifMopp later to fully compress with MOPP
bhkCompressedMeshShapeRef Exporter::makeCompressedMeshShape(Mesh& mesh, Matrix3& sm, int mtlDefault, int lyrDefault, int colFilter)
{
	// Need to separate the vertices based on material.  
	typedef vector<Triangle> Triangles;

	// setup shape data
	vector<Vector3> verts;
	vector<Vector3> norms;
	Triangles		tris;

	int vi[3];
	if (TMNegParity(sm)) {
		vi[0] = 2; vi[1] = 1; vi[2] = 0;
	}
	else {
		vi[0] = 0; vi[1] = 1; vi[2] = 2;
	}

	int nvert = mesh.getNumVerts();
	int nface = mesh.getNumFaces();
	mesh.buildNormals();
	mesh.buildBoundingBox();
	Box3 aabb = mesh.getBoundingBox();
	Vector3 ptoffset = TOVECTOR3(aabb.Min()) / bhkAppScaleFactor;

	tris.resize(nface);
	verts.resize(nvert);
	norms.resize(nface);
	for (int i = 0; i < nvert; ++i) {
		verts[i] = TOVECTOR3((mesh.getVert(i) * sm) / bhkAppScaleFactor);
	}
	for (int i = 0; i < nface; ++i)
	{
		Triangle& tri = tris[i];
		norms[i] = TOVECTOR3(mesh.getFaceNormal(i));
		Face& face = mesh.faces[i];
		tri[0] = (USHORT)face.getVert(0);
		tri[1] = (USHORT)face.getVert(1);
		tri[2] = (USHORT)face.getVert(2);
	}
	TriStrips strips;
	strippify(strips, verts, norms, tris);
	vector<unsigned short > indices;
	indices.reserve(nface * 3);
	for (auto itr = strips.begin(); itr != strips.end(); ++itr) {
		auto& strip = (*itr);
		for (int j = 0; j < strip.size(); ++j) {
			indices.push_back(strip[j]);
		}
	}
	

	bhkCompressedMeshShapeDataRef data = new bhkCompressedMeshShapeData();
	HavokMaterial material; SkyrimHavokMaterial skyrimMaterial;
	GetHavokMaterialsFromIndex(mtlDefault, (int*)&material, (int*)&skyrimMaterial);

	OblivionLayer layer; SkyrimLayer skyrimLayer;
	GetHavokLayersFromIndex(lyrDefault, (int*)&layer, (int*)&skyrimLayer);

	// setup shape
	bhkCompressedMeshShapeRef shape = new bhkCompressedMeshShape();
	shape->SetData(data);

	vector<bhkCMSDChunk> subshapes; subshapes.resize(1);
	vector<bhkCMSDTransform> transforms; transforms.resize(1);
	vector<bhkCMSDMaterial> materials; materials.resize(1);
	bhkCMSDChunk& subshape = subshapes.front();
	bhkCMSDTransform& subtrans = transforms.front();
	bhkCMSDMaterial& submat = materials.front();
	subtrans.translation = ptoffset;
	subtrans.rotation = TOQUATXYZW(Quat());

	submat.skyrimMaterial = skyrimMaterial;
	submat.skyrimLayer = skyrimLayer;

	data->SetChunkTransforms(transforms);

	subshape.numVertices = verts.size();
	subshape.vertices.reserve(subshape.numVertices * 3);
	for (auto itr = verts.begin(); itr != verts.end(); ++itr) {
		const auto& v = (*itr);
		subshape.vertices.push_back(static_cast<unsigned short>((v.x - ptoffset.x) * 1000.0f));
		subshape.vertices.push_back(static_cast<unsigned short>((v.y - ptoffset.y) * 1000.0f));
		subshape.vertices.push_back(static_cast<unsigned short>((v.z - ptoffset.z) * 1000.0f));
	}
	subshape.numIndices = indices.size();
	subshape.indices = indices;
	subshape.numStrips = strips.size();
	subshape.strips.reserve(subshape.numStrips);
	for (auto itr = strips.begin(); itr != strips.end(); ++itr)
		subshape.strips.push_back(strips.size());
	data->SetChunks(subshapes);
	return shape;
}

void Exporter::updateRigidBody(bhkRigidBodyRef& ref)
{
	TheHavokCode.UpdateRigidBody(ref);

	// recursively update root node/targets
	updateRootTarget(ref->GetShape(), mNiRoot);
}


void Exporter::updateRootTarget(Ref<bhkShape> ref, NiNodeRef ni_node)
{
	if (ref == nullptr) return;

	if (bhkMoppBvTreeShapeRef treeShape = DynamicCast<bhkMoppBvTreeShape>(ref)) {
		updateRootTarget(treeShape->GetShape(), ni_node);
	}
	else if (bhkListShapeRef listShape = DynamicCast<bhkListShape>(ref)) {
		const auto& shapes = listShape->GetSubShapes();
		for (auto itr = shapes.begin(); itr != shapes.end(); ++itr) {
			updateRootTarget(*itr, ni_node);
		}
	}
	else if ( bhkCompressedMeshShapeRef cmsr = DynamicCast<bhkCompressedMeshShape>(ref)) {
		cmsr->SetTarget(mNiRoot);
	}
}