#pragma once

#include "niutils.h"
#include "obj\bhkCollisionObject.h"
#include "obj\bhkBlendCollisionObject.h"
#include "obj\bhkRigidBody.h"
#include "obj\bhkShape.h"
#include "obj\bhkSphereShape.h"
#include "obj\bhkCapsuleShape.h"
#include "obj\bhkConvexVerticesShape.h"
#include "obj\bhkMoppBvTreeShape.h"
#include "obj\bhkPackedNiTriStripsShape.h"
#include "obj\hkPackedNiTriStripsData.h"
#include "obj\bhkListShape.h"
#include "obj\bhkTransformShape.h"
#include "obj/bhkCompressedMeshShape.h"
#include "obj/bhkCompressedMeshShapeData.h"
#include "..\NifProps\bhkRigidBodyInterface.h"

#include "obj\bhkConstraint.h"
#include "obj\bhkLimitedHingeConstraint.h"
#include "obj\bhkMalleableConstraint.h"
#include "obj\bhkHingeConstraint.h"
#include "obj\bhkRagdollConstraint.h"

#include "NifPlugins.h"
#include "nifqhull.h"
#include "../NifProps/bhkHelperFuncs.h"
#include "ICustAttribContainer.h"
#include "../max_nif_plugin/NifImport/NIFImporter.h"

//#include "../max_nif_plugin/NifExport/Exporter.h"

//using namespace Niflib;

struct HavokImport {

	HavokImport(NifImporter& parent) : ni(parent) {}

	NifImporter &ni;

	INode* ImportHCTCapsule(Niflib::bhkCapsuleShapeRef shape, INode * parent, INode* ragdollParent, Matrix3 & tm);
	INode* ImportHCTSphere(Niflib::bhkSphereShapeRef shape, INode * parent, INode* ragdollParent, Matrix3 & tm);
	void HandleRagdollOnNonAccum(INode* accumChild, INode* ragdollParent);
	void createRagdollRigidBody(INode * n, INode * parent, INode * ragdollParent, bhkRigidBodyRef rbody);
};

struct HavokExport {

	float scale = 1.0;

	void makeHavokRigidBody(NiNodeRef parent, INode * ragdollParent, float scale = 1.0);
};