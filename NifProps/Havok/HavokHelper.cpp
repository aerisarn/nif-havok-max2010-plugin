#include "HavokHelper.h"
#include <meshadj.h>

//Havok
#include <ContentTools/Max/MaxFpInterfaces/Physics/Shape/hctShapeModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/RigidBody/hctRigidBodyModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Objects/TaperedCapsule/hctTaperedCapsuleObjectInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctBallAndSocketConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctHingeConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctPrismaticConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctRagdollConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctStiffSpringConstraintModifierInterface.h>
#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctWheelConstraintModifierInterface.h>



INode* HavokImport::ImportHCTSphere(bhkSphereShapeRef shape, INode * parent, INode* ragdollParent, Matrix3 & tm)
{
	USES_CONVERSION;
	bhkShapeRef retval;

	if (SimpleObject *obj = (SimpleObject *)ni.gi->CreateInstance(GEOMOBJECT_CLASS_ID, HK_TAPEREDCAPSULE_CLASS_ID)) {

		if (IParamBlock2* pblock2 = obj->GetParamBlockByID(PB_TAPEREDCAPSULE_OBJ_PBLOCK))
		{
			float radius = shape->GetRadius();
			int mtl = GetHavokIndexFromMaterials(ni.IsSkyrim() ? -1 : shape->GetMaterial(), ni.IsSkyrim() ? shape->GetSkyrimMaterial() : -1);
			//float radius1 = shape->GetRadius1() * ni.bhkScaleFactor;
			//float radius2 = shape->GetRadius2();
			//Vector3 pt1 = shape->GetFirstPoint();
			//Vector3 pt2 = shape->GetSecondPoint();
			float len = radius * ni.bhkScaleFactor;

			//Point3 center = (TOPOINT3(pt2 + pt1) / 2.0f) * ni.bhkScaleFactor;
			//Point3 norm = Normalize(TOPOINT3(len));
			//Matrix3 mat;
			//MatrixFromNormal(norm, mat);
			//Matrix3 newTM = tm * mat * TransMatrix(center);

			pblock2->SetValue(PA_TAPEREDCAPSULE_OBJ_RADIUS, 0, len, 0);
			//			pblock2->SetValue(PA_TAPEREDCAPSULE_OBJ_HEIGHT, 0, 1.0, 0);

			if (INode *n = ni.CreateImportNode(A2T(shape->GetType().GetTypeName().c_str()), obj, ragdollParent)) {
				//				ImportBase(body, shape, parent, n, newTM);
				const int MaxChar = 512;
				char buffer[MaxChar];
				//TSTR name(A2THelper(buffer, parent->GetName().c_str(), _countof(buffer)));
				n->SetName(FormatText(TEXT("Ragdoll_%s"), parent->GetName()));
				PosRotScale prs = prsDefault;

				//n->SetObjOffsetScale(ScaleValue(Point3(1, 1, 1)));

				Point3 pos = tm.GetTrans();
				Quat rot(tm);
				PosRotScaleNode(n, pos, rot, 1.0, prsDefault);

				Object *pObj = n->GetObjectRef();
				IDerivedObject *dobj = nullptr;
				if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
					dobj = static_cast<IDerivedObject*>(pObj);
				else {
					dobj = CreateDerivedObject(pObj);
				}


				Modifier* shapeMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_SHAPE_MODIFIER_CLASS_ID);
				if (IParamBlock2* shapeParameters = shapeMod->GetParamBlockByID(PB_SHAPE_MOD_PBLOCK)) {
					shapeParameters->SetValue(PA_SHAPE_MOD_SHAPE_TYPE, 0, 1, 0);
				}

				dobj->SetAFlag(A_LOCK_TARGET);
				dobj->AddModifier(shapeMod);
				dobj->ClearAFlag(A_LOCK_TARGET);
				n->SetObjectRef(dobj);

				//AddShape(rbody, n);
				return n;
			}
		}
	}
	return ragdollParent;
}

INode* HavokImport::ImportHCTCapsule(bhkCapsuleShapeRef shape, INode *parent, INode *ragdollParent, Matrix3& tm)
{
	USES_CONVERSION;

	bhkShapeRef retval;
	if (SimpleObject *obj = (SimpleObject *)ni.gi->CreateInstance(GEOMOBJECT_CLASS_ID, HK_TAPEREDCAPSULE_CLASS_ID)) {

		if (IParamBlock2* pblock2 = obj->GetParamBlockByID(PB_TAPEREDCAPSULE_OBJ_PBLOCK))
		{
			float radius = shape->GetRadius();
			int mtl = GetHavokIndexFromMaterials(ni.IsSkyrim() ? -1 : shape->GetMaterial(), ni.IsSkyrim() ? shape->GetSkyrimMaterial() : -1);
			float radius1 = shape->GetRadius1() * ni.bhkScaleFactor;
			float radius2 = shape->GetRadius2();
			Vector3 pt1 = shape->GetFirstPoint();
			Vector3 pt2 = shape->GetSecondPoint();
			float len = (pt2 - pt1).Magnitude() * ni.bhkScaleFactor;

			Point3 center = (TOPOINT3(pt2 + pt1) / 2.0f) * ni.bhkScaleFactor;
			Point3 norm = Normalize(TOPOINT3(pt2 - pt1));
			Matrix3 mat;
			MatrixFromNormal(norm, mat);

			pblock2->SetValue(PA_TAPEREDCAPSULE_OBJ_RADIUS, 0, radius1, 0);
			pblock2->SetValue(PA_TAPEREDCAPSULE_OBJ_HEIGHT, 0, len, 0);

			if (INode *n = ni.CreateImportRagdollNode(A2T(shape->GetType().GetTypeName().c_str()), obj, ragdollParent)) {

				PosRotScale prs = prsDefault;

				n->SetObjOffsetPos(parent->GetObjOffsetPos() + center);
				n->SetObjOffsetRot(parent->GetObjOffsetRot()*mat);

				Point3 pos = tm.GetTrans();
				Quat rot(tm);
				PosRotScaleNode(n, pos, rot, 1.0, prsDefault);

				Object *pObj = n->GetObjectRef();
				IDerivedObject *dobj = nullptr;
				if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
					dobj = static_cast<IDerivedObject*>(pObj);
				else {
					dobj = CreateDerivedObject(pObj);
				}

				//Havok shape
				Modifier* shapeMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_SHAPE_MODIFIER_CLASS_ID);
				if (IParamBlock2* shapeParameters = shapeMod->GetParamBlockByID(PB_SHAPE_MOD_PBLOCK)) {
					shapeParameters->SetValue(PA_SHAPE_MOD_SHAPE_TYPE, 0, 2, 0);
				}

				dobj->SetAFlag(A_LOCK_TARGET);
				dobj->AddModifier(shapeMod);
				dobj->ClearAFlag(A_LOCK_TARGET);
				n->SetObjectRef(dobj);

				return n;
			}
		}
	}
	return ragdollParent;
#if 0
	if (SimpleObject *ob = (SimpleObject *)ni.gi->CreateInstance(GEOMOBJECT_CLASS_ID, SCUBA_CLASS_ID)) {
		float radius = shape->GetRadius();
		float radius1 = shape->GetRadius1();
		float radius2 = shape->GetRadius2();
		Point3 pt1 = TOPOINT3(shape->GetFirstPoint());
		Point3 pt2 = TOPOINT3(shape->GetSecondPoint());
		float height = Length(pt1 - pt2);
		int heighttype = 1;

		RefTargetHandle t = ob->GetReference(0);
		if (IParamBlock2* pblock2 = ob->GetParamBlockByID(0))
		{
			pblock2->SetValue(CAPSULE_RADIUS, 0, radius);
			pblock2->SetValue(CAPSULE_HEIGHT, 0, height);
			pblock2->SetValue(CAPSULE_CENTERS, 0, heighttype);
		}

		if (INode *n = ni.CreateImportNode(shape->GetType().GetTypeName().c_str(), ob, parent)) {
			// Need to "Affect Pivot Only" and "Center to Object" first
			//n->CenterPivot(0, FALSE);

			// Need to reposition the Capsule so that caps are rotated correctly for pts given

			int mtlIdx = GetHavokIndexFromMaterial(shape->GetMaterial());
			int lyrIdx = GetHavokIndexFromLayer(OL_UNIDENTIFIED);
			CreatebhkCollisionModifier(n, bv_type_capsule, mtlIdx, lyrIdx, 0);
			ImportBase(body, shape, parent, n, tm);
			AddShape(rbody, n);
			return true;
		}
	}
	return true;
#endif
}


void HavokImport::HandleRagdollOnNonAccum(INode* accumChild, INode* ragdollParent)
{
	const int MaxChar = 512;
	char buffer[MaxChar];

	//Fix for nonaccumNodes

	//TSTR name(A2THelper(buffer, parent->GetName().c_str(), _countof(buffer)));
	ragdollParent->SetName(FormatText(TEXT("Ragdoll_%s"), accumChild->GetName()));

	Object *Obj = ragdollParent->GetObjectRef();

	if (Obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		while (Obj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		
			IDerivedObject *DerObj = static_cast<IDerivedObject *> (Obj);
			const int nMods = DerObj->NumModifiers();
			for (int i = 0; i < nMods; i++)
			{
				Modifier *Mod = DerObj->GetModifier(i);
				if (Mod->ClassID() == HK_RIGIDBODY_MODIFIER_CLASS_ID)
				{
					ICustAttribContainer* cc = Mod->GetCustAttribContainer();
					if (cc)
					{
					//reset
						Mod->DeleteCustAttribContainer();

					}
					Mod->AllocCustAttribContainer();
					cc = Mod->GetCustAttribContainer();
					CustAttrib* c = (CustAttrib*)CreateInstance(CUST_ATTRIB_CLASS_ID, Class_ID(0x6e663460, 0x32682c72));
					IParamBlock2* custModParameters = c->GetParamBlock(0);
					custModParameters->SetValue(0, 0, accumChild, 0);

					cc->InsertCustAttrib(0, c);
				}
			}
			Obj = DerObj->GetObjRef();
		}
	}
}

void HavokImport::createRagdollRigidBody(INode* n, INode* parent, INode* ragdollParent, bhkRigidBodyRef rbody) {

	const int MaxChar = 512;
	char buffer[MaxChar];

	//TSTR name(A2THelper(buffer, parent->GetName().c_str(), _countof(buffer)));
	n->SetName(FormatText(TEXT("Ragdoll_%s"), parent->GetName()));

	Object *pObj = n->GetObjectRef();
	IDerivedObject *dobj = nullptr;
	if (n->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		dobj = static_cast<IDerivedObject*>(pObj);
	else {
		dobj = CreateDerivedObject(pObj);
	}

	MotionSystem msys = rbody->GetMotionSystem(); //?
	MotionQuality qtype = rbody->GetQualityType();
	float mass = rbody->GetMass();
	float lindamp = rbody->GetLinearDamping();
	float angdamp = rbody->GetAngularDamping();
	float frict = rbody->GetFriction();
	float resti = rbody->GetRestitution();
	float maxlinvel = rbody->GetMaxLinearVelocity();
	float maxangvel = rbody->GetMaxAngularVelocity();
	float pendepth = rbody->GetPenetrationDepth();
	InertiaMatrix im = rbody->GetInertia();

	Modifier* rbMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_RIGIDBODY_MODIFIER_CLASS_ID);
	if (IParamBlock2* rbParameters = rbMod->GetParamBlockByID(PB_RB_MOD_PBLOCK)) {
		//These are fundamental parameters
		rbParameters->SetValue(PA_RB_MOD_MASS, 0, mass, 0);
		rbParameters->SetValue(PA_RB_MOD_RESTITUTION, 0, resti, 0);
		rbParameters->SetValue(PA_RB_MOD_FRICTION, 0, frict, 0);
		rbParameters->SetValue(PA_RB_MOD_INERTIA_TENSOR, 0, Point3(im[0][0],im[1][1],im[2][2]), 0);

		rbParameters->SetValue(PA_RB_MOD_LINEAR_DAMPING, 0, lindamp, 0);
		rbParameters->SetValue(PA_RB_MOD_CHANGE_ANGULAR_DAMPING, 0, angdamp, 0);

		rbParameters->SetValue(PA_RB_MOD_MAX_LINEAR_VELOCITY, 0, maxlinvel, 0);
		rbParameters->SetValue(PA_RB_MOD_MAX_ANGULAR_VELOCITY, 0, maxangvel, 0);

		rbParameters->SetValue(PA_RB_MOD_ALLOWED_PENETRATION_DEPTH, 0, pendepth, 0);


		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_FIXED, 0);
		rbParameters->SetValue(PA_RB_MOD_SOLVER_DEACTIVATION, 0, SD_LOW, 0);
		rbParameters->SetValue(PA_RB_MOD_DEACTIVATOR_TYPE, 0, DT_LOW, 0);

		/*body->SetMotionSystem(MotionSystem::MO_SYS_BOX);
		body->SetDeactivatorType(DeactivatorType::DEACTIVATOR_NEVER);
		body->SetSolverDeactivation(SolverDeactivation::SOLVER_DEACTIVATION_LOW);
		body->SetQualityType(MO_QUAL_FIXED);*/


		/*switch (qtype) {
		case MO_QUAL_INVALID:
			break;
		case MO_QUAL_FIXED:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_FIXED, 0);
			break;
		case MO_QUAL_KEYFRAMED:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_KEYFRAMED, 0);
			break;
		case MO_QUAL_DEBRIS:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_DEBRIS, 0);
			break;
		case MO_QUAL_MOVING:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_MOVING, 0);
			break;
		case MO_QUAL_CRITICAL:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_CRITICAL, 0);
			break;
		case MO_QUAL_BULLET:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_BULLET, 0);
			break;
		case MO_QUAL_USER:
			break;
		case MO_QUAL_CHARACTER:
			break;
		case MO_QUAL_KEYFRAMED_REPORT:
			rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_KEYFRAMED_REPORTING, 0);
			break;
		}*/


	}

	//Link Rigid Body to parent Rigid Body
	ICustAttribContainer* cc = rbMod->GetCustAttribContainer();

	if (!cc)
	{
		rbMod->AllocCustAttribContainer();
		cc = rbMod->GetCustAttribContainer();
	}
	CustAttrib* c = (CustAttrib*)CreateInstance(CUST_ATTRIB_CLASS_ID, Class_ID(0x6e663460, 0x32682c72));
	IParamBlock2* custModParameters = c->GetParamBlock(0);
	custModParameters->SetValue(0, 0, parent, 0);

	cc->InsertCustAttrib(0, c);

	Modifier* constraintMod = nullptr;

	vector< bhkSerializableRef > constraints = rbody->GetConstraints();
	//Rigid Body constraints
	if (ragdollParent) {

		for (vector< bhkSerializableRef >::iterator it = constraints.begin(); it != constraints.end(); ) {
			bhkConstraintRef constraint = bhkConstraintRef(*it);
			if (constraint->IsDerivedType(bhkLimitedHingeConstraint::TYPE)) {
				bhkLimitedHingeConstraintRef limitedHingeConstraint = bhkLimitedHingeConstraintRef(*it);
				LimitedHingeDescriptor lh = limitedHingeConstraint->GetLimitedHinge();
				constraintMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_CONSTRAINT_HINGE_CLASS_ID);
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_NODE, 0, ragdollParent, 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_ROTATION_LOCK, 0, 0, 0);

					Point3 origin(0, 0, 0);

					Matrix3 parentRotation(TOPOINT3(lh.perp2AxleInA1), TOPOINT3(lh.perp2AxleInA2), TOPOINT3(lh.axleA), origin);
					Matrix3 childRotation(TOPOINT3(lh.perp2AxleInB1), TOPOINT3(lh.perp2AxleInB2), TOPOINT3(lh.axleB), origin);

					//Matrix3 parentRotation(true);
					//MatrixFromNormal(TOPOINT3(lh.axleA), parentRotation);
					//Matrix3 childRotation(true);
					//MatrixFromNormal(TOPOINT3(lh.axleB), childRotation);

					constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, 0);
				}
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_HINGE_MOD_PBLOCK)) {
					constraintParameters->SetValue(PA_HINGE_MOD_IS_LIMITED, 0, 1, 0);
					constraintParameters->SetValue(PA_HINGE_MOD_LIMIT_MIN, 0, TODEG(lh.minAngle), 0);
					constraintParameters->SetValue(PA_HINGE_MOD_LIMIT_MAX, 0, TODEG(lh.maxAngle), 0);
					constraintParameters->SetValue(PA_HINGE_MOD_MAX_FRICTION_TORQUE, 0, lh.maxFriction, 0);
					//	constraintParameters->SetValue(PA_HINGE_MOD_MOTOR_TYPE, 0, lh.motor., 0);
				}
			}
			else if (constraint->IsDerivedType(bhkRagdollConstraint::TYPE)) {
				bhkRagdollConstraintRef ragdollConstraint = bhkRagdollConstraintRef(*it);
				RagdollDescriptor rag = ragdollConstraint->GetRagdoll();
				constraintMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_CONSTRAINT_RAGDOLL_CLASS_ID);
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_NODE, 0, ragdollParent, 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_ROTATION_LOCK, 0, 0, 0);
					
					//TOVECTOR3(rag.twistA);
					//MatrixFromNormal(TOPOINT3(rag.twistA), parentRotation);
					Point3 origin(0,0,0);
					Matrix3 parentRotation(TOPOINT3(rag.planeA),TOPOINT3(rag.motorA),TOPOINT3(rag.twistA),origin);
					
					//TOVECTOR3(rag.twistB);
					//MatrixFromNormal(TOPOINT3(rag.twistB), childRotation);
					Matrix3 childRotation(TOPOINT3(rag.planeB), TOPOINT3(rag.motorB), TOPOINT3(rag.twistB), origin);

					constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_TRANSLATION, 0, TOPOINT3(rag.pivotB), 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_TRANSLATION, 0, TOPOINT3(rag.pivotA), 0);
					constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, 0);
				}
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_RAGDOLL_MOD_PBLOCK)) {
					constraintParameters->SetValue(PA_RAGDOLL_MOD_CONE_ANGLE, 0, TODEG(rag.coneMaxAngle), 0);
					constraintParameters->SetValue(PA_RAGDOLL_MOD_PLANE_MIN, 0, TODEG(rag.planeMinAngle), 0);
					constraintParameters->SetValue(PA_RAGDOLL_MOD_PLANE_MAX, 0, TODEG(rag.planeMaxAngle), 0);
					constraintParameters->SetValue(PA_RAGDOLL_MOD_TWIST_MIN, 0, TODEG(rag.twistMinAngle), 0);
					constraintParameters->SetValue(PA_RAGDOLL_MOD_TWIST_MAX, 0, TODEG(rag.twistMaxAngle), 0);
					constraintParameters->SetValue(PA_RAGDOLL_MOD_MAX_FRICTION_TORQUE, 0, rag.maxFriction, 0);
				}
			}
			else if (constraint->IsDerivedType(bhkMalleableConstraint::TYPE)) {
				bhkMalleableConstraintRef malleableConstraint = bhkMalleableConstraintRef(*it);
				if (malleableConstraint->GetConstraintType() == (unsigned int)2) {
					LimitedHingeDescriptor lh = malleableConstraint->GetLimitedHinge();
					constraintMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_CONSTRAINT_HINGE_CLASS_ID);
					if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_NODE, 0, ragdollParent, 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_ROTATION_LOCK, 0, 0, 0);
						Point3 origin(0, 0, 0);

						Matrix3 parentRotation(TOPOINT3(lh.perp2AxleInA1), TOPOINT3(lh.perp2AxleInA2), TOPOINT3(lh.axleA), origin);
						Matrix3 childRotation(TOPOINT3(lh.perp2AxleInB1), TOPOINT3(lh.perp2AxleInB2), TOPOINT3(lh.axleB), origin);

						//Matrix3 parentRotation(true);
						//MatrixFromNormal(TOPOINT3(lh.axleA), parentRotation);
						//Matrix3 childRotation(true);
						//MatrixFromNormal(TOPOINT3(lh.axleB), childRotation);

						constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, 0);
					}
					if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_HINGE_MOD_PBLOCK)) {
						constraintParameters->SetValue(PA_HINGE_MOD_IS_LIMITED, 0, 1, 0);
						constraintParameters->SetValue(PA_HINGE_MOD_LIMIT_MIN, 0, TODEG(lh.minAngle), 0);
						constraintParameters->SetValue(PA_HINGE_MOD_LIMIT_MAX, 0, TODEG(lh.maxAngle), 0);
						constraintParameters->SetValue(PA_HINGE_MOD_MAX_FRICTION_TORQUE, 0, lh.maxFriction, 0);
						//	constraintParameters->SetValue(PA_HINGE_MOD_MOTOR_TYPE, 0, lh.motor., 0);
					}
				}
				else if (malleableConstraint->GetConstraintType() == (unsigned int)7) {
					RagdollDescriptor rag = malleableConstraint->GetRagdoll();
					constraintMod = (Modifier*)CreateInstance(OSM_CLASS_ID, HK_CONSTRAINT_RAGDOLL_CLASS_ID);
					if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_NODE, 0, ragdollParent, 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_ROTATION_LOCK, 0, 0, 0);
						//TOVECTOR3(rag.twistA);
						//MatrixFromNormal(TOPOINT3(rag.twistA), parentRotation);
						Point3 origin(0, 0, 0);
						Matrix3 parentRotation(TOPOINT3(rag.planeA), TOPOINT3(rag.motorA), TOPOINT3(rag.twistA), origin);

						//TOVECTOR3(rag.twistB);
						//MatrixFromNormal(TOPOINT3(rag.twistB), childRotation);
						Matrix3 childRotation(TOPOINT3(rag.planeB), TOPOINT3(rag.motorB), TOPOINT3(rag.twistB), origin);


						constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_TRANSLATION, 0, TOPOINT3(rag.pivotB), 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_TRANSLATION, 0, TOPOINT3(rag.pivotA), 0);
						constraintParameters->SetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, 0);
					}
					if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_RAGDOLL_MOD_PBLOCK)) {
						constraintParameters->SetValue(PA_RAGDOLL_MOD_CONE_ANGLE, 0, TODEG(rag.coneMaxAngle), 0);
						constraintParameters->SetValue(PA_RAGDOLL_MOD_PLANE_MIN, 0, TODEG(rag.planeMinAngle), 0);
						constraintParameters->SetValue(PA_RAGDOLL_MOD_PLANE_MAX, 0, TODEG(rag.planeMaxAngle), 0);
						constraintParameters->SetValue(PA_RAGDOLL_MOD_TWIST_MIN, 0, TODEG(rag.twistMinAngle), 0);
						constraintParameters->SetValue(PA_RAGDOLL_MOD_TWIST_MAX, 0, TODEG(rag.twistMaxAngle), 0);
						constraintParameters->SetValue(PA_RAGDOLL_MOD_MAX_FRICTION_TORQUE, 0, rag.maxFriction, 0);
					}
				}
			}
			++it;
		}
	}

	dobj->SetAFlag(A_LOCK_TARGET);
	dobj->AddModifier(rbMod);
	if (constraintMod)
		dobj->AddModifier(constraintMod);
	dobj->ClearAFlag(A_LOCK_TARGET);
	n->SetObjectRef(dobj);

}

void HavokExport::makeHavokRigidBody(NiNodeRef parent, INode *ragdollParent, float scale) {

	this->scale = scale;

	Object *Obj = ragdollParent->GetObjectRef();

	Modifier* rbMod = nullptr;
	Modifier* shapeMod = nullptr;
	Modifier* constraintMod = nullptr;

	SimpleObject* havokTaperCapsule = nullptr;

	//get modifiers
	

	while (Obj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		IDerivedObject *DerObj = static_cast<IDerivedObject *> (Obj);
		const int nMods = DerObj->NumModifiers(); //it is really the last modifier on the stack, and not the total number of modifiers

		for (int i = 0; i < nMods; i++)
		{
			Modifier *Mod = DerObj->GetModifier(i);
			if (Mod->ClassID() == HK_RIGIDBODY_MODIFIER_CLASS_ID) {
				rbMod = Mod;
			}
			if (Mod->ClassID() == HK_SHAPE_MODIFIER_CLASS_ID) {
				shapeMod = Mod;
			}
			if (Mod->ClassID() == HK_CONSTRAINT_RAGDOLL_CLASS_ID || Mod->ClassID() == HK_CONSTRAINT_HINGE_CLASS_ID) {
				constraintMod = Mod;
			}
		}
		if (Obj->SuperClassID() == GEOMOBJECT_CLASS_ID) {
			havokTaperCapsule = (SimpleObject*)Obj;
		}
		Obj = DerObj->GetObjRef();
	}

	
	if (!rbMod) {
		throw exception(FormatText("No havok rigid body modifier found on %s", ragdollParent->GetName()));
	}
	if (!shapeMod) {
		throw exception(FormatText("No havok shape modifier found on %s", ragdollParent->GetName()));
	}

//	Object* taper = ragdollParent->GetObjectRef();
	IParamBlock2* taperParameters = Obj->GetParamBlockByID(PB_TAPEREDCAPSULE_OBJ_PBLOCK);
	float radius;
	enum
	{
		// GENERAL PROPERTIES ROLLOUT
		PA_TAPEREDCAPSULE_OBJ_RADIUS = 0,
		PA_TAPEREDCAPSULE_OBJ_TAPER,
		PA_TAPEREDCAPSULE_OBJ_HEIGHT,
		PA_TAPEREDCAPSULE_OBJ_VERSION_INTERNAL,
	};
	taperParameters->GetValue(PA_TAPEREDCAPSULE_OBJ_RADIUS, 0, radius, FOREVER);
	

	int shapeType;
	if (IParamBlock2* shapeParameters = shapeMod->GetParamBlockByID(PB_SHAPE_MOD_PBLOCK)) {
		shapeParameters->GetValue(PA_SHAPE_MOD_SHAPE_TYPE,0,shapeType,FOREVER);
	}

	//Havok Shape
	bhkShapeRef shape;

	if (shapeType == 2) {

		// Capsule
		bhkCapsuleShapeRef capsule = new bhkCapsuleShape();
		capsule->SetRadius(radius/scale);
		capsule->SetRadius1(radius/scale);
		capsule->SetRadius2(radius/scale);
		float length; 
		taperParameters->GetValue(PA_TAPEREDCAPSULE_OBJ_HEIGHT, 0, length, FOREVER);
		//get the normal
		Matrix3 axis(true);
		ragdollParent->GetObjOffsetRot().MakeMatrix(axis);
		Point3 normalAx = axis.GetRow(2);
		//capsule center
		Point3 center = ragdollParent->GetObjOffsetPos();
		//min and max points
		Point3 pt1 = center - normalAx*(length/2);
		Point3 pt2 = center + normalAx*(length/2);

		capsule->SetFirstPoint(TOVECTOR3(pt1)/scale);
		capsule->SetSecondPoint(TOVECTOR3(pt2)/scale);
		capsule->SetMaterial(HAV_MAT_SKIN);

		shape = StaticCast<bhkShape>(capsule);
		
	}
	else {
		// Sphere
		//CalcBoundingSphere(node, tm.GetTrans(), radius, 0);

		bhkSphereShapeRef sphere = new bhkSphereShape();
		sphere->SetRadius(radius/scale);
		sphere->SetMaterial(HAV_MAT_SKIN);
		shape = StaticCast<bhkShape>(sphere);
	}

	bhkRigidBodyRef body;

	if (shape)
	{
		bhkBlendCollisionObjectRef blendObj = new bhkBlendCollisionObject();
		body = new bhkRigidBody();

		Matrix3 tm = ragdollParent->GetObjTMAfterWSM(0);
		
		//Calculate Object Offset Matrix
		Matrix3 otm(1);
		Point3 pos = ragdollParent->GetObjOffsetPos();
		otm.PreTranslate(pos);
		Quat quat = ragdollParent->GetObjOffsetRot();
		PreRotateMatrix(otm, quat);
		Matrix3 otmInvert = otm;
		otmInvert.Invert();

		//correct object tm
		Matrix3 tmbhk = otmInvert * tm;

		//set geometric parameters
		body->SetRotation(TOQUATXYZW(Quat(tmbhk).Invert()));
		body->SetTranslation(TOVECTOR4(tmbhk.GetTrans() / scale));
		body->SetCenter(TOVECTOR4(ragdollParent->GetObjOffsetPos())/scale);

		//set physics
		if (IParamBlock2* rbParameters = rbMod->GetParamBlockByID(PB_RB_MOD_PBLOCK)) {
			//These are fundamental parameters

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
			Point3 InertiaTensor;


			rbParameters->GetValue(PA_RB_MOD_MASS, 0, mass, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_RESTITUTION, 0, resti, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_FRICTION, 0, frict, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_INERTIA_TENSOR, 0, InertiaTensor, FOREVER);


			rbParameters->GetValue(PA_RB_MOD_LINEAR_DAMPING, 0, lindamp, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_CHANGE_ANGULAR_DAMPING, 0, angdamp, FOREVER);

			rbParameters->GetValue(PA_RB_MOD_MAX_LINEAR_VELOCITY, 0, maxlinvel, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_MAX_ANGULAR_VELOCITY, 0, maxangvel, FOREVER);

			rbParameters->GetValue(PA_RB_MOD_ALLOWED_PENETRATION_DEPTH, 0, pendepth, FOREVER);
			rbParameters->GetValue(PA_RB_MOD_QUALITY_TYPE, 0, qtype, FOREVER);

			body->SetMass(mass);
			body->SetRestitution(resti);
			body->SetFriction(frict);
			body->SetLinearDamping(lindamp);
			body->SetMaxLinearVelocity(maxlinvel);
			body->SetMaxAngularVelocity(maxangvel);
			body->SetPenetrationDepth(pendepth);
			InertiaMatrix im;
			im[0][0] = InertiaTensor[0];
			im[1][1] = InertiaTensor[1];
			im[2][2] = InertiaTensor[2];

			body->SetInertia(im);

			/*switch (qtype) {
			case QT_FIXED:
				body->SetQualityType(MO_QUAL_FIXED);
				break;
			case QT_KEYFRAMED:
				body->SetQualityType(MO_QUAL_KEYFRAMED);
				break;
			case QT_DEBRIS:
				body->SetQualityType(MO_QUAL_DEBRIS);
				break;
			case QT_MOVING:
				body->SetQualityType(MO_QUAL_MOVING);
				break;
			case QT_CRITICAL:
				body->SetQualityType(MO_QUAL_CRITICAL);
				break;
			case QT_BULLET:
				body->SetQualityType(MO_QUAL_BULLET);
				break;
			case QT_KEYFRAMED_REPORTING:
				body->SetQualityType(MO_QUAL_KEYFRAMED_REPORT);
				break;
			}*/

			body->SetSkyrimLayer(SkyrimLayer::SKYL_BIPED);
			body->SetSkyrimLayerCopy(SkyrimLayer::SKYL_BIPED);

			body->SetMotionSystem(MotionSystem::MO_SYS_BOX);
			body->SetDeactivatorType(DeactivatorType::DEACTIVATOR_NEVER);
			body->SetSolverDeactivation(SolverDeactivation::SOLVER_DEACTIVATION_LOW);
			body->SetQualityType(MO_QUAL_FIXED);

		}
		
		if (constraintMod && ragdollParent->GetParentNode() && parent->GetParent()) {
			if (constraintMod->ClassID() == HK_CONSTRAINT_RAGDOLL_CLASS_ID) {
				bhkRagdollConstraintRef ragdollConstraint = new bhkRagdollConstraint();
				
				//entities
				ragdollConstraint->AddEntity(body);
				NiNodeRef parentRef = parent->GetParent();
				bhkRigidBodyRef nifParentRigidBody;
				while (parentRef) {
					if (parentRef->GetCollisionObject()) {
						nifParentRigidBody = StaticCast<bhkRigidBody>(StaticCast<bhkBlendCollisionObject>(parentRef->GetCollisionObject())->GetBody());
						break;
					}
					parentRef = parentRef->GetParent();
				}
				if (!nifParentRigidBody)
					throw exception(FormatText("Unable to find NIF constraint parent for ragdoll node %s", ragdollParent->GetName()));
				ragdollConstraint->AddEntity(nifParentRigidBody);

				RagdollDescriptor desc;
				//parameters
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
					Point3 pivotA;
					Matrix3 parentRotation;
					Point3 pivotB;
					Matrix3 childRotation;
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_TRANSLATION, 0, pivotB, FOREVER);
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, FOREVER);
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_TRANSLATION, 0, pivotA, FOREVER);
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, FOREVER);
					
					desc.pivotA = TOVECTOR4(pivotA);
					desc.pivotB = TOVECTOR4(pivotB);
					desc.planeA = TOVECTOR4(parentRotation.GetRow(0));
					desc.motorA = TOVECTOR4(parentRotation.GetRow(1));
					desc.twistA = TOVECTOR4(parentRotation.GetRow(2));
					desc.planeB = TOVECTOR4(childRotation.GetRow(0));
					desc.motorB = TOVECTOR4(childRotation.GetRow(1));
					desc.twistB = TOVECTOR4(childRotation.GetRow(2));
					
				}
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_RAGDOLL_MOD_PBLOCK)) {
					float coneMaxAngle;
					float planeMinAngle;
					float planeMaxAngle;
					float coneMinAngle;
					float twistMinAngle;
					float maxFriction;

					constraintParameters->GetValue(PA_RAGDOLL_MOD_CONE_ANGLE, 0, coneMaxAngle, FOREVER);
					constraintParameters->GetValue(PA_RAGDOLL_MOD_PLANE_MIN, 0, planeMinAngle, FOREVER);
					constraintParameters->GetValue(PA_RAGDOLL_MOD_PLANE_MAX, 0, planeMaxAngle, FOREVER);
					constraintParameters->GetValue(PA_RAGDOLL_MOD_TWIST_MIN, 0, coneMinAngle, FOREVER);
					constraintParameters->GetValue(PA_RAGDOLL_MOD_TWIST_MAX, 0, twistMinAngle, FOREVER);
					constraintParameters->GetValue(PA_RAGDOLL_MOD_MAX_FRICTION_TORQUE, 0, maxFriction, FOREVER);

					desc.coneMaxAngle = TORAD(coneMaxAngle);
					desc.planeMinAngle = TORAD(planeMinAngle);
					desc.planeMaxAngle = TORAD(planeMaxAngle);
					desc.coneMaxAngle = TORAD(coneMinAngle);
					desc.twistMinAngle = TORAD(twistMinAngle);
					desc.maxFriction = maxFriction;


				}
				ragdollConstraint->SetRagdoll(desc);
				body->AddConstraint(ragdollConstraint);
			}
			else if (constraintMod->ClassID() == HK_CONSTRAINT_HINGE_CLASS_ID) {
				bhkLimitedHingeConstraintRef limitedHingeConstraint = new bhkLimitedHingeConstraint();

				//entities
				limitedHingeConstraint->AddEntity(body);
				NiNodeRef parentRef = parent->GetParent();
				bhkRigidBodyRef nifParentRigidBody;
				while (parentRef) {
					if (parentRef->GetCollisionObject()) {
						nifParentRigidBody = StaticCast<bhkRigidBody>(StaticCast<bhkBlendCollisionObject>(parentRef->GetCollisionObject())->GetBody());
						break;
					}
					parentRef = parentRef->GetParent();
				}
				if (!nifParentRigidBody)
					throw exception(FormatText("Unable to find NIF constraint parent for limited hinge node %s", ragdollParent->GetName()));
				limitedHingeConstraint->AddEntity(nifParentRigidBody);

				LimitedHingeDescriptor lh;

				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS)) {
					Matrix3 parentRotation;
					Matrix3 childRotation;
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION, 0, childRotation, FOREVER);
					constraintParameters->GetValue(PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION, 0, parentRotation, FOREVER);

					lh.perp2AxleInA1 = TOVECTOR4(parentRotation.GetRow(0));
					lh.perp2AxleInA2 = TOVECTOR4(parentRotation.GetRow(1));
					lh.axleA = TOVECTOR4(parentRotation.GetRow(2));
					lh.perp2AxleInB1 = TOVECTOR4(childRotation.GetRow(0));
					lh.perp2AxleInB2 = TOVECTOR4(childRotation.GetRow(1));
					lh.axleB = TOVECTOR4(childRotation.GetRow(2));
					
				}
				if (IParamBlock2* constraintParameters = constraintMod->GetParamBlockByID(PB_HINGE_MOD_PBLOCK)) {
					float minAngle;
					float maxAngle;
					float maxFriction;

					constraintParameters->GetValue(PA_HINGE_MOD_LIMIT_MIN, 0, minAngle, FOREVER);
					constraintParameters->GetValue(PA_HINGE_MOD_LIMIT_MAX, 0, maxAngle, FOREVER);
					constraintParameters->GetValue(PA_HINGE_MOD_MAX_FRICTION_TORQUE, 0, maxFriction, FOREVER);
					//	constraintParameters->SetValue(PA_HINGE_MOD_MOTOR_TYPE, 0, lh.motor., 0);

					lh.minAngle = TORAD(minAngle);
					lh.maxAngle = TORAD(maxAngle);
					lh.maxAngle = maxFriction;

				}
				limitedHingeConstraint->SetLimitedHinge(lh);
				body->AddConstraint(limitedHingeConstraint);
			}
		}


		//InitializeRigidBody(body, node);
		body->SetShape(shape);
		blendObj->SetBody(StaticCast<NiObject>(body));
		parent->SetCollisionObject(StaticCast<NiCollisionObject>(blendObj));
	}

	////rigid body parameters
	//	// get data from node
	//int lyr = NP_DEFAULT_HVK_LAYER;
	//int mtl = NP_DEFAULT_HVK_MATERIAL;
	//int msys = NP_DEFAULT_HVK_MOTION_SYSTEM;
	//int qtype = NP_DEFAULT_HVK_QUALITY_TYPE;
	//float mass = NP_DEFAULT_HVK_MASS;
	//float lindamp = NP_DEFAULT_HVK_LINEAR_DAMPING;
	//float angdamp = NP_DEFAULT_HVK_ANGULAR_DAMPING;
	//float frict = NP_DEFAULT_HVK_FRICTION;
	//float maxlinvel = NP_DEFAULT_HVK_MAX_LINEAR_VELOCITY;
	//float maxangvel = NP_DEFAULT_HVK_MAX_ANGULAR_VELOCITY;
	//float resti = NP_DEFAULT_HVK_RESTITUTION;
	//float pendepth = NP_DEFAULT_HVK_PENETRATION_DEPTH;
	//BOOL transenable = TRUE;

	//if (IParamBlock2* rbParameters = rbMod->GetParamBlockByID(PB_SHAPE_MOD_PBLOCK))
	//{
	//	//These are fundamental parameters
	//	rbParameters->GetValue(PA_RB_MOD_MASS, 0, mass, FOREVER);
	//	rbParameters->GetValue(PA_RB_MOD_RESTITUTION, 0, resti, FOREVER);
	//	rbParameters->GetValue(PA_RB_MOD_FRICTION, 0, frict, FOREVER);

	//	rbParameters->GetValue(PA_RB_MOD_LINEAR_DAMPING, 0, lindamp, FOREVER);
	//	rbParameters->GetValue(PA_RB_MOD_CHANGE_ANGULAR_DAMPING, 0, angdamp, FOREVER);

	//	rbParameters->GetValue(PA_RB_MOD_MAX_LINEAR_VELOCITY, 0, maxlinvel, FOREVER);
	//	rbParameters->GetValue(PA_RB_MOD_MAX_ANGULAR_VELOCITY, 0, maxangvel, FOREVER);

	//	rbParameters->GetValue(PA_RB_MOD_ALLOWED_PENETRATION_DEPTH, 0, pendepth, FOREVER);

	//	rbParameters->GetValue(PA_RB_MOD_QUALITY_TYPE, 0, qtype, FOREVER);


	//	switch (qtype) {
	//	case MO_QUAL_INVALID:
	//		break;
	//	case QT_FIXED:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, MO_QUAL_FIXED, 0);
	//		break;
	//	case MO_QUAL_KEYFRAMED:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_KEYFRAMED, 0);
	//		break;
	//	case MO_QUAL_DEBRIS:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_DEBRIS, 0);
	//		break;
	//	case MO_QUAL_MOVING:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_MOVING, 0);
	//		break;
	//	case MO_QUAL_CRITICAL:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_CRITICAL, 0);
	//		break;
	//	case MO_QUAL_BULLET:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_BULLET, 0);
	//		break;
	//	case MO_QUAL_USER:
	//		break;
	//	case MO_QUAL_CHARACTER:
	//		break;
	//	case MO_QUAL_KEYFRAMED_REPORT:
	//		rbParameters->SetValue(PA_RB_MOD_QUALITY_TYPE, 0, QT_KEYFRAMED_REPORTING, 0);
	//		break;
	//	}

	//	// setup body
	//	bhkRigidBodyRef body = transenable ? new bhkRigidBodyT() : new bhkRigidBody();

	//	OblivionLayer obv_layer; SkyrimLayer sky_layer;
	//	GetHavokLayersFromIndex(lyr, (int*)&obv_layer, (int*)&sky_layer);
	//	body->SetLayer(obv_layer);
	//	body->SetLayerCopy(obv_layer);
	//	body->SetSkyrimLayer(sky_layer);

	//	body->SetMotionSystem(MotionSystem(msys));
	//	body->SetQualityType(MotionQuality(qtype));
	//	body->SetMass(mass);
	//	body->SetLinearDamping(lindamp);
	//	body->SetAngularDamping(angdamp);
	//	body->SetFriction(frict);
	//	body->SetRestitution(resti);
	//	body->SetMaxLinearVelocity(maxlinvel);
	//	body->SetMaxAngularVelocity(maxangvel);
	//	body->SetPenetrationDepth(pendepth);
	//	body->SetCenter(center);
	//	QuaternionXYZW q; q.x = q.y = q.z = 0; q.w = 1.0f;
	//	body->SetRotation(q);
	//}
}
