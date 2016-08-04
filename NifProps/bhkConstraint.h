#pragma once
#ifndef __BHKCONSTRAINT__H
#define __BHKCONSTRAINT__H

//! The unique instance of the rigid body interface
extern CoreExport FPInterfaceDesc gbhkConstraintDesc;



#define BHKCONSTRAINTCLASS_DESC Class_ID(0x5ba11e05, 0x132165a1)
const Interface_ID BHKCONSTRAINTINTERFACE_DESC(0x6cd3215f, 0x24601fa0);

extern FPInterfaceDesc* GetbhkConstraintInterfaceDesc();

class bhkRigidBodyInterface : public FPMixinInterface
{
}

#endif