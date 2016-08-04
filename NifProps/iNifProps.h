#pragma once

class StdMat2;
struct BGSMFile;
struct BGEMFile;
const Class_ID BSDSMODIFIER_CLASS_ID(0xe9a0a68e, 0xb091bd48);
const Class_ID BSSIMODIFIER_CLASS_ID(0x32bee15a, 0xb60e422e);

class IBSDismemberSkinModifierData;
class IBSSubIndexModifierData;
class IFO4ShaderData;
struct IFileResolver;

// Selection levels:
#define IMESHSEL_OBJECT 0
#define IMESHSEL_VERTEX 1
#define IMESHSEL_FACE 2
#define IMESHSEL_EDGE 3

enum DismemberBodyPartType {
	BP_TORSO = 0, /*!< Torso */
	BP_HEAD = 1, /*!< Head */
	BP_HEAD2 = 2, /*!< Head 2 */
	BP_LEFTARM = 3, /*!< Left Arm */
	BP_LEFTARM2 = 4, /*!< Left Arm 2 */
	BP_RIGHTARM = 5, /*!< Right Arm */
	BP_RIGHTARM2 = 6, /*!< Right Arm 2 */
	BP_LEFTLEG = 7, /*!< Left Leg */
	BP_LEFTLEG2 = 8, /*!< Left Leg 2 */
	BP_LEFTLEG3 = 9, /*!< Left Leg 3 */
	BP_RIGHTLEG = 10, /*!< Right Leg */
	BP_RIGHTLEG2 = 11, /*!< Right Leg 2 */
	BP_RIGHTLEG3 = 12, /*!< Right Leg 3 */
	BP_BRAIN = 13, /*!< Brain */

	SBP_30_HEAD = 30, /*!< Skyrim, Head(Human), Body(Atronachs,Beasts), Mask(Dragonpriest) */
	SBP_31_HAIR = 31, /*!< Skyrim, Hair(human), Far(Dragon), Mask2(Dragonpriest),SkinnedFX(Spriggan) */
	SBP_32_BODY = 32, /*!< Skyrim, Main body, extras(Spriggan) */
	SBP_33_HANDS = 33, /*!< Skyrim, Hands L/R, BodyToo(Dragonpriest), Legs(Draugr), Arms(Giant) */
	SBP_34_FOREARMS = 34, /*!< Skyrim, Forearms L/R, Beard(Draugr) */
	SBP_35_AMULET = 35, /*!< Skyrim, Amulet */
	SBP_36_RING = 36, /*!< Skyrim, Ring */
	SBP_37_FEET = 37, /*!< Skyrim, Feet L/R */
	SBP_38_CALVES = 38, /*!< Skyrim, Calves L/R */
	SBP_39_SHIELD = 39, /*!< Skyrim, Shield */
	SBP_40_TAIL = 40, /*!< Skyrim, Tail(Argonian/Khajiit), Skeleton01(Dragon), FX01(AtronachStorm),FXMist (Dragonpriest), Spit(Chaurus,Spider),SmokeFins(IceWraith) */
	SBP_41_LONGHAIR = 41, /*!< Skyrim, Long Hair(Human), Skeleton02(Dragon),FXParticles(Dragonpriest) */
	SBP_42_CIRCLET = 42, /*!< Skyrim, Circlet(Human, MouthFireEffect(Dragon) */
	SBP_43_EARS = 43, /*!< Skyrim, Ears */
	SBP_44_DRAGON_BLOODHEAD_OR_MOD_MOUTH = 44, /*!< Skyrim, Bloodied dragon head, or NPC face/mouth */
	SBP_45_DRAGON_BLOODWINGL_OR_MOD_NECK = 45, /*!< Skyrim, Left Bloodied dragon wing, Saddle(Horse), or NPC cape, scarf, shawl, neck-tie, etc. */
	SBP_46_DRAGON_BLOODWINGR_OR_MOD_CHEST_PRIMARY = 46, /*!< Skyrim, Right Bloodied dragon wing, or NPC chest primary or outergarment */
	SBP_47_DRAGON_BLOODTAIL_OR_MOD_BACK = 47, /*!< Skyrim, Bloodied dragon tail, or NPC backpack/wings/... */
	SBP_48_MOD_MISC1 = 48, /*!< Anything that does not fit in the list */
	SBP_49_MOD_PELVIS_PRIMARY = 49, /*!< Pelvis primary or outergarment */
	SBP_50_DECAPITATEDHEAD = 50, /*!< Skyrim, Decapitated Head */
	SBP_51_DECAPITATE = 51, /*!< Skyrim, Decapitate, neck gore */
	SBP_52_MOD_PELVIS_SECONDARY = 52, /*!< Pelvis secondary or undergarment */
	SBP_53_MOD_LEG_RIGHT = 53, /*!< Leg primary or outergarment or right leg */
	SBP_54_MOD_LEG_LEFT = 54, /*!< Leg secondary or undergarment or left leg */
	SBP_55_MOD_FACE_JEWELRY = 55, /*!< Face alternate or jewelry */
	SBP_56_MOD_CHEST_SECONDARY = 56, /*!< Chest secondary or undergarment */
	SBP_57_MOD_SHOULDER = 57, /*!< Shoulder */
	SBP_58_MOD_ARM_LEFT = 58, /*!< Arm secondary or undergarment or left arm */
	SBP_59_MOD_ARM_RIGHT = 59, /*!< Arm primary or outergarment or right arm */
	SBP_60_MOD_MISC2 = 60, /*!< Anything that does not fit in the list */
	SBP_61_FX01 = 61, /*!< Skyrim, FX01(Humanoid) */

	SBP_130_HEAD = 130, /*!< Skyrim, Head slot, use on full-face helmets */
	SBP_131_HAIR = 131, /*!< Skyrim, Hair slot 1, use on hoods */
	SBP_141_LONGHAIR = 141, /*!< Skyrim, Hair slot 2, use for longer hair */
	SBP_142_CIRCLET = 142, /*!< Skyrim, Circlet slot 1, use for circlets */
	SBP_143_EARS = 143, /*!< Skyrim, Ear slot */
	SBP_150_DECAPITATEDHEAD = 150, /*!< Skyrim, neck gore on head side */

	SBP_230_HEAD = 230, /*!< Skyrim, Head slot, use for neck on character head */

	BP_SECTIONCAP_HEAD = 101, /*!< Section Cap | Head */
	BP_SECTIONCAP_HEAD2 = 102, /*!< Section Cap | Head 2 */
	BP_SECTIONCAP_LEFTARM = 103, /*!< Section Cap | Left Arm */
	BP_SECTIONCAP_LEFTARM2 = 104, /*!< Section Cap | Left Arm 2 */
	BP_SECTIONCAP_RIGHTARM = 105, /*!< Section Cap | Right Arm */
	BP_SECTIONCAP_RIGHTARM2 = 106, /*!< Section Cap | Right Arm 2 */
	BP_SECTIONCAP_LEFTLEG = 107, /*!< Section Cap | Left Leg */
	BP_SECTIONCAP_LEFTLEG2 = 108, /*!< Section Cap | Left Leg 2 */
	BP_SECTIONCAP_LEFTLEG3 = 109, /*!< Section Cap | Left Leg 3 */
	BP_SECTIONCAP_RIGHTLEG = 110, /*!< Section Cap | Right Leg */
	BP_SECTIONCAP_RIGHTLEG2 = 111, /*!< Section Cap | Right Leg 2 */
	BP_SECTIONCAP_RIGHTLEG3 = 112, /*!< Section Cap | Right Leg 3 */
	BP_SECTIONCAP_BRAIN = 113, /*!< Section Cap | Brain */

	BP_TORSOCAP_HEAD = 201, /*!< Torso Cap | Head */
	BP_TORSOCAP_HEAD2 = 202, /*!< Torso Cap | Head 2 */
	BP_TORSOCAP_LEFTARM = 203, /*!< Torso Cap | Left Arm */
	BP_TORSOCAP_LEFTARM2 = 204, /*!< Torso Cap | Left Arm 2 */
	BP_TORSOCAP_RIGHTARM = 205, /*!< Torso Cap | Right Arm */
	BP_TORSOCAP_RIGHTARM2 = 206, /*!< Torso Cap | Right Arm 2 */
	BP_TORSOCAP_LEFTLEG = 207, /*!< Torso Cap | Left Leg */
	BP_TORSOCAP_LEFTLEG2 = 208, /*!< Torso Cap | Left Leg 2 */
	BP_TORSOCAP_LEFTLEG3 = 209, /*!< Torso Cap | Left Leg 3 */
	BP_TORSOCAP_RIGHTLEG = 210, /*!< Torso Cap | Right Leg */
	BP_TORSOCAP_RIGHTLEG2 = 211, /*!< Torso Cap | Right Leg 2 */
	BP_TORSOCAP_RIGHTLEG3 = 212, /*!< Torso Cap | Right Leg 3 */
	BP_TORSOCAP_BRAIN = 213, /*!< Torso Cap | Brain */

	BP_TORSOSECTION_HEAD = 1000, /*!< Torso Section | Head */
	BP_TORSOSECTION_HEAD2 = 2000, /*!< Torso Section | Head 2 */
	BP_TORSOSECTION_LEFTARM = 3000, /*!< Torso Section | Left Arm */
	BP_TORSOSECTION_LEFTARM2 = 4000, /*!< Torso Section | Left Arm 2 */
	BP_TORSOSECTION_RIGHTARM = 5000, /*!< Torso Section | Right Arm */
	BP_TORSOSECTION_RIGHTARM2 = 6000, /*!< Torso Section | Right Arm 2 */
	BP_TORSOSECTION_LEFTLEG = 7000, /*!< Torso Section | Left Leg */
	BP_TORSOSECTION_LEFTLEG2 = 8000, /*!< Torso Section | Left Leg 2 */
	BP_TORSOSECTION_LEFTLEG3 = 9000, /*!< Torso Section | Left Leg 3 */
	BP_TORSOSECTION_RIGHTLEG = 10000, /*!< Torso Section | Right Leg */
	BP_TORSOSECTION_RIGHTLEG2 = 11000, /*!< Torso Section | Right Leg 2 */
	BP_TORSOSECTION_RIGHTLEG3 = 12000, /*!< Torso Section | Right Leg 3 */
	BP_TORSOSECTION_BRAIN = 13000, /*!< Torso Section | Brain */
};

struct BSDSPartitionData
{
	BSDSPartitionData() : bodyPart(::BP_TORSO), partFlag(1), selLevel(0) {}
	DismemberBodyPartType bodyPart;
	DWORD partFlag;
	DWORD selLevel;
};

class SelectModContextEnumProc : public ModContextEnumProc {
public:
	virtual BOOL proc(ModContext *mc) {
		mcList.Append(1, &mc);
		return TRUE;
	}
	ModContextList mcList;
};

const ULONG I_BSDISMEMBERSKINMODIFIER = I_USERINTERFACE + 0x0000E271;
const ULONG I_BSDISMEMBERSKINMODIFIERDATA = I_USERINTERFACE + 0x0000E272;

const ULONG I_BSSUBINDEXSKINMODIFIER = I_USERINTERFACE + 0x0000E281;
const ULONG I_BSSUBINDEXMODIFIERDATA = I_USERINTERFACE + 0x0000E282;

const ULONG I_BSSHADERDATA = I_USERINTERFACE + 0x0000E291;

class IBSDismemberSkinModifier
	   //: public MaxHeapOperators 
{
public:
	/*! \remarks This method must be called when the <b>LocalModData</b> of
	the modifier is changed. Developers can use the methods of
	<b>IMeshSelectData</b> to get and set the actual selection for vertex, face
	and edge. When a developers does set any of these selection sets this
	method must be called when done. */
	virtual void LocalDataChanged() = 0;

	/*! \remarks Gets all of the mod contexts related to this modifier. */
	virtual Tab<IBSDismemberSkinModifierData*> GetModifierData() = 0;
};


class IBSDismemberSkinModifierData
	   //: public MaxHeapOperators 
{
public:

	/*! \remarks Returns the number of partitions for the modifier. */
	virtual DWORD GetNumPartitions() = 0;

	/*! \remarks Adds the specified partition and returns its index. */
	virtual DWORD AddPartition() = 0;

	/*! \remarks Rempves the specified partition from the modifier. */
	virtual void RemovePartition(DWORD partition) = 0;

	/*! \remarks Returns the current level of selection for the modifier. */
	virtual DWORD GetActivePartition() = 0;

	/*! \remarks Sets the currently selected partition level of the modifier. */
	virtual void SetActivePartition(DWORD partition) = 0;

	virtual BitArray& GetFaceSel(int index) = 0;
	virtual void SetFaceSel(int index, BitArray &set, IBSDismemberSkinModifier *imod, TimeValue t) = 0;
	virtual Tab<BSDSPartitionData> & GetPartitionFlags() = 0;
	virtual GenericNamedSelSetList & GetFaceSelList() = 0;
};


extern Modifier *GetOrCreateBSDismemberSkin(INode *node);
extern Modifier *GetBSDismemberSkin(INode *node);


////////////////////////////////////////////////
struct BSSubIndexMaterial
{
	uint32_t id; // unique id per each subindex
	uint32_t materialHash;
	bool visible; 
	vector<float> data;
};

struct BSSubIndexData
{
	BSSubIndexData() : selLevel(0), id(0) {}
	DWORD selLevel; // Current Selection Type
	int id;
	// DWORD subIndexCount;
	vector<BSSubIndexMaterial> materials;
};


class IBSSubIndexModifier
{
public:
	/*! \remarks This method must be called when the <b>LocalModData</b> of
	the modifier is changed. Developers can use the methods of
	<b>IMeshSelectData</b> to get and set the actual selection for vertex, face
	and edge. When a developers does set any of these selection sets this
	method must be called when done. */
	virtual void LocalDataChanged() = 0;

	/*! \remarks Gets all of the mod contexts related to this modifier. */
	virtual Tab<IBSSubIndexModifierData*> GetModifierData() = 0;
};

class IBSSubIndexModifierData
	//: public MaxHeapOperators 
{
public:
	/*! \remarks Returns the number of partitions for the modifier. */
	virtual DWORD GetNumPartitions() = 0;

	/*! \remarks Adds the specified partition and returns its index. */
	virtual DWORD AddPartition() = 0;

	/*! \remarks Rempves the specified partition from the modifier. */
	virtual void RemovePartition(DWORD partition) = 0;

	/*! \remarks Returns the current level of selection for the modifier. */
	virtual DWORD GetActivePartition() = 0;

	/*! \remarks Sets the currently selected partition level of the modifier. */
	virtual void SetActivePartition(DWORD partition) = 0;


	/*! \remarks Returns the number of sub partitions for the modifier. */
	virtual DWORD GetNumSubPartitions() = 0;

	/*! \remarks Adds the specified sub partition and returns its index. */
	virtual DWORD AddSubPartition() = 0;

	/*! \remarks Rempves the specified sub partition from the modifier. */
	virtual void RemoveSubPartition(DWORD partition) = 0;

	/*! \remarks Returns the current level of selection for the modifier. */
	virtual DWORD GetActiveSubPartition() = 0;

	/*! \remarks Sets the currently selected partition level of the modifier. */
	virtual void SetActiveSubPartition(DWORD partition) = 0;

	/*! \remarks Get the number of subpartitions in active partition */
	virtual DWORD GetActivePartitionSubCount() = 0;

	virtual BOOL EnablePartitionEdit(BOOL value) = 0;
	virtual BOOL GetPartitionEditEnabled() const = 0;

	virtual BitArray& GetFaceSel(int index, int subIndex) = 0;
	virtual void SetFaceSel(int index, int subIndex, BitArray &set, IBSSubIndexModifier *imod, TimeValue t) = 0;
	
	virtual BSSubIndexData& GetPartition(int index) = 0;
	virtual GenericNamedSelSetList & GetFaceSelList() = 0;

	virtual TSTR GetSSF() const = 0;
	virtual void SetSSF(const TSTR& ssf_file) = 0;
};

class IBSShaderMaterialData
	//: public MaxHeapOperators 
{
public:
	virtual LPCTSTR GetMaterialName() const = 0;
	virtual LPCTSTR GetFileName() const = 0;
	virtual void SetMaterialName(LPCTSTR name) = 0;
	virtual void SetFileName(LPCTSTR path) = 0;

	virtual BOOL HasBGSM() const = 0;
	virtual BGSMFile* GetBGSMData() const = 0;
	virtual BOOL LoadBGSM(BGSMFile&) = 0;

	virtual BOOL HasBGEM() const = 0;
	virtual BGEMFile* GetBGEMData() const = 0;
	virtual BOOL LoadBGEM(BGEMFile&) = 0;

	virtual BOOL UpdateMaterial(StdMat2* mtl) = 0;
	virtual BOOL LoadMaterial(StdMat2* mtl, IFileResolver* resolver) = 0;
};
extern Modifier *GetOrCreateBSSubIndexModifier(INode *node);
extern Modifier *GetBSSubIndexModifier(INode *node);


// interface for resolving files
struct IFileResolver
{
public:
	enum FileType { FT_Unknown, FT_Texture, FT_Material, FT_Mesh };
	virtual bool FindFile(const tstring& name, tstring& resolved_name) const = 0;
	virtual bool FindFileByType(const tstring& name, FileType type, tstring& resolved_name) const = 0;
	virtual bool GetRelativePath(tstring& name, FileType type) const = 0;
protected:
	~IFileResolver() {}
};
