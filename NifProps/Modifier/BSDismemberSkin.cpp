/**********************************************************************
*<
FILE: BSDismemberSkin.cpp

CREATED BY: tazpn (Theo)

HISTORY:

*>   Copyright (c) 2008, All Rights Reserved.
**********************************************************************/
#pragma warning( disable:4800 )
#include <max.h>
#include "MAX_Mem.h"
#include <custcont.h>
#include "..\NifProps\NifProps.h"

//#include "mods.h"
#include <iparamm2.h>
#include <MeshDLib.h>
#include <namesel.h>
#include "NifGui.h"
#include "..\NifProps\iNifProps.h"

#ifndef MESHSELECTCONVERT_INTERFACE 
#define MESHSELECTCONVERT_INTERFACE Interface_ID(0x3da7dd5, 0x7ecf0391)
#endif
//////////////////////////////////////////////////////////////////////////

const EnumLookupType BodyPartFlags[] =
{
   {BP_TORSO, TEXT("Torso")},
   {BP_HEAD, TEXT("Head")},
   {BP_HEAD2, TEXT("Head 2")},
   {BP_LEFTARM, TEXT("Left Arm")},
   {BP_LEFTARM2, TEXT("Left Arm 2")},
   {BP_RIGHTARM, TEXT("Right Arm")},
   {BP_RIGHTARM2, TEXT("Right Arm 2")},
   {BP_LEFTLEG, TEXT("Left Leg")},
   {BP_LEFTLEG2, TEXT("Left Leg 2")},
   {BP_LEFTLEG3, TEXT("Left Leg 3")},
   {BP_RIGHTLEG, TEXT("Right Leg")},
   {BP_RIGHTLEG2, TEXT("Right Leg 2")},
   {BP_RIGHTLEG3, TEXT("Right Leg 3")},
   {BP_BRAIN, TEXT("Brain")},



   { SBP_30_HEAD, TEXT("Skyrim, Head(Human)") },
   { SBP_31_HAIR, TEXT("Skyrim, Hair(human)") },
   { SBP_32_BODY, TEXT("Skyrim, Main body") },
   { SBP_33_HANDS, TEXT("Skyrim, Hands L/R") },
   { SBP_34_FOREARMS, TEXT("Skyrim, Forearms L/R") },
   { SBP_35_AMULET, TEXT("Skyrim, Amulet") },
   { SBP_36_RING, TEXT("Skyrim, Ring") },
   { SBP_37_FEET, TEXT("Skyrim, Feet L/R") },
   { SBP_38_CALVES, TEXT("Skyrim, Calves L/R") },
   { SBP_39_SHIELD, TEXT("Skyrim, Shield") },
   { SBP_40_TAIL, TEXT("Skyrim, Tail") },
   { SBP_41_LONGHAIR, TEXT("Skyrim, Long Hair") },
   { SBP_42_CIRCLET, TEXT("Skyrim, Circlet") },
   { SBP_43_EARS, TEXT("Skyrim, Ears") },
   { SBP_44_DRAGON_BLOODHEAD_OR_MOD_MOUTH, TEXT("Skyrim, NPC face/mouth") },
   { SBP_45_DRAGON_BLOODWINGL_OR_MOD_NECK, TEXT("Skyrim, NPC neck") },
   { SBP_46_DRAGON_BLOODWINGR_OR_MOD_CHEST_PRIMARY, TEXT("Skyrim, NPC chest") },
   { SBP_47_DRAGON_BLOODTAIL_OR_MOD_BACK, TEXT("Skyrim, NPC backpack") },
   { SBP_48_MOD_MISC1, TEXT("Skyrim, Miscellaneous 1") },
   { SBP_49_MOD_PELVIS_PRIMARY, TEXT("Skyrim, Pelvis primary") },
   { SBP_50_DECAPITATEDHEAD, TEXT("Skyrim, Decapitated Head") },
   { SBP_51_DECAPITATE, TEXT("Skyrim, Decapitate, neck gore") },
   { SBP_52_MOD_PELVIS_SECONDARY, TEXT("Skyrim, Pelvis secondary") },
   { SBP_53_MOD_LEG_RIGHT, TEXT("Skyrim, right leg") },
   { SBP_54_MOD_LEG_LEFT, TEXT("Skyrim, left leg") },
   { SBP_55_MOD_FACE_JEWELRY, TEXT("Skyrim, Face jewelry") },
   { SBP_56_MOD_CHEST_SECONDARY, TEXT("Skyrim, Chest secondary") },
   { SBP_57_MOD_SHOULDER, TEXT("Skyrim, Shoulder") },
   { SBP_58_MOD_ARM_LEFT, TEXT("Skyrim, left arm") },
   { SBP_59_MOD_ARM_RIGHT, TEXT("Skyrim, right arm") },
   { SBP_60_MOD_MISC2, TEXT("Skyrim, Miscellaneous 2") },
   { SBP_61_FX01, TEXT("Skyrim, FX01(Humanoid)") },
   { SBP_130_HEAD, TEXT("Skyrim, Head slot") },
   { SBP_131_HAIR, TEXT("Skyrim, Hair slot") },
   { SBP_141_LONGHAIR, TEXT("Skyrim, Long hair slot") },
   { SBP_142_CIRCLET, TEXT("Skyrim, Circlet slot") },
   { SBP_143_EARS, TEXT("Skyrim, Ear slot") },
   { SBP_150_DECAPITATEDHEAD, TEXT("Skyrim, neck gore") },
   { SBP_230_HEAD, TEXT("Skyrim, Head/neck slot") },

   {BP_SECTIONCAP_HEAD, TEXT("Section Cap | Head")},
   {BP_SECTIONCAP_HEAD2, TEXT("Section Cap | Head 2")},
   {BP_SECTIONCAP_LEFTARM, TEXT("Section Cap | Left Arm")},
   {BP_SECTIONCAP_LEFTARM2, TEXT("Section Cap | Left Arm 2")},
   {BP_SECTIONCAP_RIGHTARM, TEXT("Section Cap | Right Arm")},
   {BP_SECTIONCAP_RIGHTARM2, TEXT("Section Cap | Right Arm 2")},
   {BP_SECTIONCAP_LEFTLEG, TEXT("Section Cap | Left Leg")},
   {BP_SECTIONCAP_LEFTLEG2, TEXT("Section Cap | Left Leg 2")},
   {BP_SECTIONCAP_LEFTLEG3, TEXT("Section Cap | Left Leg 3")},
   {BP_SECTIONCAP_RIGHTLEG, TEXT("Section Cap | Right Leg")},
   {BP_SECTIONCAP_RIGHTLEG2, TEXT("Section Cap | Right Leg 2")},
   {BP_SECTIONCAP_RIGHTLEG3, TEXT("Section Cap | Right Leg 3")},
   {BP_SECTIONCAP_BRAIN, TEXT("Section Cap | Brain")},
   {BP_TORSOCAP_HEAD, TEXT("Body Cap | Head")},
   {BP_TORSOCAP_HEAD2, TEXT("Body Cap | Head 2")},
   {BP_TORSOCAP_LEFTARM, TEXT("Body Cap | Left Arm")},
   {BP_TORSOCAP_LEFTARM2, TEXT("Body Cap | Left Arm 2")},
   {BP_TORSOCAP_RIGHTARM, TEXT("Body Cap | Right Arm")},
   {BP_TORSOCAP_RIGHTARM2, TEXT("Body Cap | Right Arm 2")},
   {BP_TORSOCAP_LEFTLEG, TEXT("Body Cap | Left Leg")},
   {BP_TORSOCAP_LEFTLEG2, TEXT("Body Cap | Left Leg 2")},
   {BP_TORSOCAP_LEFTLEG3, TEXT("Body Cap | Left Leg 3")},
   {BP_TORSOCAP_RIGHTLEG, TEXT("Body Cap | Right Leg")},
   {BP_TORSOCAP_RIGHTLEG2, TEXT("Body Cap | Right Leg 2")},
   {BP_TORSOCAP_RIGHTLEG3, TEXT("Body Cap | Right Leg 3")},
   {BP_TORSOCAP_BRAIN, TEXT("Body Cap | Brain")},
   {BP_TORSOSECTION_HEAD, TEXT("Body | Head")},
   {BP_TORSOSECTION_HEAD2, TEXT("Body | Head 2")},
   {BP_TORSOSECTION_LEFTARM, TEXT("Body | Left Arm")},
   {BP_TORSOSECTION_LEFTARM2, TEXT("Body | Left Arm 2")},
   {BP_TORSOSECTION_RIGHTARM, TEXT("Body | Right Arm")},
   {BP_TORSOSECTION_RIGHTARM2, TEXT("Body | Right Arm 2")},
   {BP_TORSOSECTION_LEFTLEG, TEXT("Body | Left Leg")},
   {BP_TORSOSECTION_LEFTLEG2, TEXT("Body | Left Leg 2")},
   {BP_TORSOSECTION_LEFTLEG3, TEXT("Body | Left Leg 3")},
   {BP_TORSOSECTION_RIGHTLEG, TEXT("Body | Right Leg")},
   {BP_TORSOSECTION_RIGHTLEG2, TEXT("Body | Right Leg 2")},
   {BP_TORSOSECTION_RIGHTLEG3, TEXT("Body | Right Leg 3")},
   {BP_TORSOSECTION_BRAIN, TEXT("Body | Brain")},
   {0,  NULL},
};

#define SEL_OBJECT  0
#define SEL_FACE	3
#define SEL_POLY	4
#define SEL_ELEMENT 5

class PartSubObjType : public ISubObjType {
	TSTR name;
	int mIdx;
public:
	TSTR& GetNameRef() { return name; }
	void SetName(const TCHAR *nm) { name = nm; }
#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
	TCHAR *GetName() { return name; }
#else
	const MSTR& GetName() { return name; }
#endif
	MaxIcon *GetIcon() { return NULL; }
};


// Named selection set levels:
const DWORD WM_UPDATE_CACHE = (WM_USER + 0x2000);

// Image list used for mesh sub-object toolbar in Edit Mesh, Mesh Select:
class BSDSImageHandler {
public:
	HIMAGELIST images;
	BSDSImageHandler() { images = NULL; }
	~BSDSImageHandler() { if (images) ImageList_Destroy(images); }
	// MeshSelImageHandler methods.
	// Note: these are also used by meshsel.cpp (and declared in mods.h).
	HIMAGELIST LoadImages() {
		if (images) return images;
		images = ImageList_Create(24, 23, ILC_COLOR | ILC_MASK, 10, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_SELTYPES));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_SELMASK));
		ImageList_Add(images, hBitmap, hMask);
		DeleteObject(hBitmap), DeleteObject(hMask);
		return images;
	}
};

class BSDSPartImageHandler {
public:
	HIMAGELIST images;
	BSDSPartImageHandler() { images = NULL; }
	~BSDSPartImageHandler() { if (images) ImageList_Destroy(images); }
	HIMAGELIST LoadImages() {
		if (images) return images;
		images = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_ADDDELPART));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_ADDDELMASK));
		ImageList_Add(images, hBitmap, hMask);
		DeleteObject(hBitmap), DeleteObject(hMask);
		return images;
	}
};
// Local static instance.
static BSDSImageHandler theBSDSImageHandler;
static BSDSPartImageHandler theBSDSPartImageHandler;

#define IDC_SELFACE 0x3262
#define IDC_SELPOLY 0x3263
#define IDC_SELELEMENT 0x3264
extern int *meshSubTypeToolbarIDs;

// BSDSModifier flags:
#define MS_DISP_END_RESULT 0x01

// BSDSModifier References:
#define REF_PBLOCK 0

class BSDSModifier : public Modifier, public IBSDismemberSkinModifier, public FlagUser {
public:
	IParamBlock2 *pblock;
	static IObjParam *ip;
	static BSDSModifier *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;
	std::vector<PartSubObjType> partitions;

	BSDSModifier();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_RB_BSDSMODIFIER); }
	virtual Class_ID ClassID() { return BSDSMODIFIER_CLASS_ID; }
	RefTargetHandle Clone(RemapDir& remap /*= DefaultRemapDir()*/);
#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
	TCHAR *                 GetObjectName() { return _T(GetString(IDS_RB_BSDSMODIFIER)); }
#else
	const MCHAR*             GetObjectName() { return GetString(IDS_RB_BSDSMODIFIER); }
#endif
	void* GetInterface(ULONG id) {
		return (id == I_BSDISMEMBERSKINMODIFIER) ? (IBSDismemberSkinModifier*)this : Modifier::GetInterface(id);
	}


	// From modifier
	ChannelMask ChannelsUsed() { return PART_GEOM | PART_TOPO; }
	ChannelMask ChannelsChanged() { return PART_SELECT | PART_SUBSEL_TYPE | PART_TOPO | PART_GEOM; } // RB 5/27/97: Had to include topo channel because in edge select mode this modifier turns on hidden edges -- which may cause the edge list to be rebuilt, which is effectively a change in topology since the edge list is now part of the topo channel.
	Class_ID InputType() { return triObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity(TimeValue t);
	BOOL DependOnTopology(ModContext &mc) { return TRUE; }

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; }
	void BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void GetSubObjectCenters(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc);

	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert = FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	void ShowEndResultChanged(BOOL showEndResult) { NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE); }

	BOOL SupportsNamedSubSels() { return FALSE; }

	void ActivateSubSelSet(int index);

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumParamBlocks() { return 1; }
	IParamBlock2 *GetParamBlock(int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID(short id) { return (pblock->ID() == id) ? pblock : NULL; }

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }
	void SetReference(int i, RefTargetHandle rtarg) { pblock = (IParamBlock2 *)rtarg; }

	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) { return GetString(IDS_RB_PARAMETERS); }

#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
#else
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
#endif

	void UpdateSelLevelDisplay();
	void SelectFrom(int from);
	void SetEnableStates();
	void SelectOpenEdges();
	void SelectByMatID(int id);
	void SetNumSelLabel();
	void UpdateCache(TimeValue t);
	void InvalidateDialogElement(int elem);

	void FixDuplicates();
	void SelectUnused();

public: //IBSDismemberSkinModifier

   /*! \remarks This method must be called when the <b>LocalModData</b> of
   the modifier is changed. Developers can use the methods of
   <b>IMeshSelectData</b> to get and set the actual selection for vertex, face
   and edge. When a developers does set any of these selection sets this
   method must be called when done. */
	virtual void LocalDataChanged();

	/*! \remarks Gets all of the mod contexts related to this modifier. */
	virtual Tab<IBSDismemberSkinModifierData*> GetModifierData() {
		Tab<IBSDismemberSkinModifierData*> data;
		SelectModContextEnumProc selector;
		this->EnumModContexts(&selector);
		for (int i = 0; i < selector.mcList.Count(); ++i) {
			LocalModData *md = selector.mcList[i]->localData;
			if (!md) continue;
			if (IBSDismemberSkinModifierData* p = (IBSDismemberSkinModifierData*)md->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA))
				data.Append(1, &p);
		}
		return data;
	}

	// Helper Methods

	IBSDismemberSkinModifierData* GetFirstModifierData()
	{
		SelectModContextEnumProc selector;
		this->EnumModContexts(&selector);
		for (int i = 0; i < selector.mcList.Count(); ++i) {
			LocalModData *md = selector.mcList[i]->localData;
			if (!md) continue;
			if (IBSDismemberSkinModifierData* p = (IBSDismemberSkinModifierData*)md->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA))
				return p;
		}
		return NULL;
	}

	int GetNumPartitions()
	{
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData()) {
			return p->GetNumPartitions();
		}
		return 0;
	}

	int GetActivePartition()
	{
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData()) {
			return p->GetActivePartition();
		}
		return -1;
	}

	BSDSPartitionData& GetFlags()
	{
		static BSDSPartitionData empty;
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData()) {
			Tab<BSDSPartitionData> &flags = p->GetPartitionFlags();
			return flags[p->GetActivePartition()];
		}
		return empty;
	}

	DWORD GetSelectionLevel()
	{
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData()) {
			Tab<BSDSPartitionData> &flags = p->GetPartitionFlags();
			return flags[p->GetActivePartition()].selLevel;
		}
		return SEL_OBJECT;
	}

	void SetSelectionLevel(DWORD level)
	{
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData()) {
			Tab<BSDSPartitionData> &flags = p->GetPartitionFlags();
			flags[p->GetActivePartition()].selLevel = level;

			UpdateSelLevelDisplay();
			SetEnableStates();
			SetNumSelLabel();

			NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
			ip->PipeSelLevelChanged();
			NotifyDependents(FOREVER, SELECT_CHANNEL | DISP_ATTRIB_CHANNEL | SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
		}
	}


};

class BSDSData : public LocalModData, public IBSDismemberSkinModifierData
{
private:
	// Temp data used for soft selections, adjacent edge / face lists.
	MeshTempData *temp;

public:
	// LocalModData
	void* GetInterface(ULONG id) {
		return (id == I_BSDISMEMBERSKINMODIFIERDATA) ? (IBSDismemberSkinModifierData*)this : LocalModData::GetInterface(id);
	}

	// Selection sets
	DWORD activePartition;
	Tab<BSDSPartitionData> flags;

	// Lists of named selection sets
	GenericNamedSelSetList fselSet;

	BOOL held;
	Mesh *mesh;

	BSDSData(Mesh &mesh);
	BSDSData();
	~BSDSData() { FreeCache(); }
	LocalModData *Clone();
	Mesh *GetMesh() { return mesh; }
	AdjEdgeList *GetAdjEdgeList();
	AdjFaceList *GetAdjFaceList();
	void SetCache(Mesh &mesh);
	void FreeCache();
	void SynchBitArrays();

	// From IMeshSelectData:
	BitArray& GetFaceSel() { return GetFaceSel(activePartition); }

	void SetFaceSel(BitArray &set, BSDSModifier *imod, TimeValue t) {
		SetFaceSel(activePartition, set, imod, t);
	}


	/*! \remarks Returns the number of partitions for the modifier. */
	virtual DWORD GetNumPartitions() {
		return flags.Count();
	}

	/*! \remarks Adds the specified partition and returns its index. */
	virtual DWORD AddPartition() {
		DWORD index = GetNumPartitions();
		SetActivePartition(index);
		SelectUnused();
		return index;
	}

	/*! \remarks Removes the specified partition from the modifier. */
	virtual void RemovePartition(DWORD partition);

	// IBSDismemberSkinModifierData
   /*! \remarks Returns the current level of selection for the modifier. */
	virtual DWORD GetActivePartition() { return activePartition; }

	/*! \remarks Sets the currently selected partition level of the modifier. */
	virtual void SetActivePartition(DWORD partition);

	BitArray& GetFaceSel(int index) {
		BitArray* set = fselSet.GetSetByIndex(index);
		if (!set) {
			fselSet.InsertSet(index, BitArray());
			set = fselSet.GetSetByIndex(index);
		}
		return *set;
	}

	virtual void SetFaceSel(int index, BitArray &set, IBSDismemberSkinModifier *imod, TimeValue t);

	virtual Tab<BSDSPartitionData> & GetPartitionFlags() { return flags; }

	GenericNamedSelSetList & GetFaceSelList() { return fselSet; }

	void FixDuplicates();
	void SelectUnused();

	DWORD GetSelectionLevel() {
		return flags[activePartition].selLevel;
	}
};

class BSDSSelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	BSDSModifier *mod;
	BSDSData *d;
	int level;

	BSDSSelectRestore(BSDSModifier *m, BSDSData *d);
	BSDSSelectRestore(BSDSModifier *m, BSDSData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() { d->held = FALSE; }
	TSTR Description() { return TSTR(TEXT("BSDSSelectRestore")); }
};

//--- ClassDescriptor and class vars ---------------------------------



IObjParam *BSDSModifier::ip = NULL;
BSDSModifier *BSDSModifier::editMod = NULL;
SelectModBoxCMode *BSDSModifier::selectMode = NULL;
BOOL BSDSModifier::updateCachePosted = FALSE;

class BSDSClassDesc :public ClassDesc2 {
public:
	BSDSClassDesc();
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BSDSModifier; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BSDSMODIFIER); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return BSDSMODIFIER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR * InternalName() { return TEXT("BSDSModifier"); }
	HINSTANCE HInstance() { return hInstance; }
};

class BSDSModifierMainDlgProc : public ParamMap2UserDlgProc {
public:
	BSDSModifier *mod;
	BSDSModifierMainDlgProc() { mod = NULL; }
	void SetEnables(HWND hWnd);
	void UpdateSelLevelDisplay(HWND hWnd);
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }

	NpComboBox mCbMaterial;
	ISpinnerControl *mActivePart;
	ICustButton *mAddPart;
	ICustButton *mDelPart;
};

static BSDSModifierMainDlgProc theBSDSModifierMainDlgProc;

// Table to convert selLevel values to mesh selLevel flags.
const int meshLevel[] = { MESH_OBJECT, MESH_VERTEX, MESH_EDGE,
MESH_FACE, MESH_FACE, MESH_FACE };

// Get display flags based on selLevel.
const DWORD levelDispFlags[] = { 0,DISP_VERTTICKS | DISP_SELVERTS,
DISP_SELEDGES,DISP_SELFACES,DISP_SELPOLYS,DISP_SELPOLYS };

// For hit testing...
const int hitLevel[] = { 0,SUBHIT_VERTS,SUBHIT_EDGES,SUBHIT_FACES,SUBHIT_FACES,SUBHIT_FACES };

// Parameter block IDs:
// Blocks themselves:
enum { ms_pblock };
// Parameter maps:
enum { ms_map_main };
// Parameters in first block:
enum {
	ms_by_vertex, ms_ignore_backfacing,
	ms_matid, ms_ignore_visible, ms_planar_threshold,
};

static ParamBlockDesc2 BSDS_desc(ms_pblock,
	_T("BSDismemberSkinDescription"),
	IDS_MS_SOFTSEL, NULL,
	P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP,
	REF_PBLOCK,
	//rollout descriptions
	1,
	ms_map_main, IDD_MESH_SELECT, IDS_MS_PARAMS, 0, 0, NULL,

	// params
	ms_by_vertex, _T("byVertex"), TYPE_BOOL, P_RESET_DEFAULT, IDS_BY_VERTEX,
	p_default, false,
	p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_SEL_BYVERT,
	p_end,

	ms_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_IGNORE_BACKFACING,
	p_default, false,
	p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_IGNORE_BACKFACES,
	p_end,

	ms_matid, _T("materialID"), TYPE_INT, P_TRANSIENT | P_RESET_DEFAULT, IDS_RB_MATERIALID,
	p_default, 1,
	p_range, 1, 65535,
	p_ui, ms_map_main, TYPE_SPINNER, EDITTYPE_INT, IDC_MS_MATID, IDC_MS_MATIDSPIN, .5f,
	p_end,

	ms_ignore_visible, _T("ignoreVisibleEdges"), TYPE_BOOL, P_RESET_DEFAULT, IDS_IGNORE_VISIBLE,
	p_default, false,
	p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_IGNORE_VISEDGE,
	p_end,

	ms_planar_threshold, _T("planarThreshold"), TYPE_ANGLE, P_RESET_DEFAULT, IDS_RB_THRESHOLD,
	p_default, PI / 4.0f,	// Default value for angles has to be in radians.
	p_range, 0.0f, 180.0f,	// but range given in degrees.
	p_ui, ms_map_main, TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_MS_PLANAR, IDC_MS_PLANARSPINNER, .1f,
	p_end,
	p_end
	);

static BSDSClassDesc BSDSDesc;
ClassDesc2* GetBSDSModifierDesc() { return &BSDSDesc; }
BSDSClassDesc::BSDSClassDesc() {
	BSDS_desc.SetClassDesc(this);
}

//--- BSDS mod methods -------------------------------

BSDSModifier::BSDSModifier() {
	SetAFlag(A_PLUGIN1);
	pblock = NULL;
	BSDSDesc.MakeAutoParamBlocks(this);
	assert(pblock);
}

RefTargetHandle BSDSModifier::Clone(RemapDir& remap) {
	BSDSModifier *mod = new BSDSModifier();
	mod->ReplaceReference(0, remap.CloneRef(pblock));
	BaseClone(this, mod, remap);
	return mod;
}

Interval BSDSModifier::LocalValidity(TimeValue t)
{
	// aszabo|feb.05.02 If we are being edited, return NEVER 
	  // to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
}

Interval BSDSModifier::GetValidity(TimeValue t) {
	Interval ret = FOREVER;
	return ret;
}

void BSDSModifier::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf(triObjectClassID)) return;
	TriObject *tobj = (TriObject*)os->obj;
	BSDSData *d = (BSDSData*)mc.localData;
	if (!d) mc.localData = d = new BSDSData(tobj->GetMesh());

	int level = 0;
	if (ip) level = ip->GetSubObjectLevel();

	BitArray& faceSel = d->GetFaceSel();
	faceSel.SetSize(tobj->GetMesh().getNumFaces(), TRUE);
	tobj->GetMesh().faceSel = faceSel;
	tobj->GetMesh().selLevel = level == 0 ? SEL_OBJECT : GetSelectionLevel();

	Interval outValid;
	outValid = tobj->ChannelValidity(t, SELECT_CHAN_NUM);

	tobj->GetMesh().ClearVSelectionWeights();

	// Update the cache used for display, hit-testing:
	if (!d->GetMesh()) d->SetCache(tobj->GetMesh());
	else *(d->GetMesh()) = tobj->GetMesh();

	// Set display flags - but be sure to turn off vertex display in stack result if
	// "Show End Result" is turned on - we want the user to just see the Mesh Select
	// level vertices (from the Display method).
	tobj->GetMesh().dispFlags = 0;
	if (level != 0)
	{
		if (!ip || !ip->GetShowEndResult())
			tobj->GetMesh().SetDispFlag(levelDispFlags[GetSelectionLevel()]);
	}
	tobj->SetChannelValidity(SELECT_CHAN_NUM, outValid);

	LocalDataChanged();
	//NotifyDependents(FOREVER,PART_ALL,REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
}

void BSDSModifier::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((BSDSData*)mc->localData)->FreeCache();
	if (!ip || (editMod != this) || updateCachePosted) return;

	if (!BSDSDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSDSDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	TimeValue t = ip->GetTime();
	PostMessage(hWnd, WM_UPDATE_CACHE, (WPARAM)t, 0);
	updateCachePosted = TRUE;
}

void BSDSModifier::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t, t), PART_GEOM | SELECT_CHANNEL | PART_SUBSEL_TYPE |
		PART_DISPLAY | PART_TOPO, REFMSG_MOD_EVAL);
	updateCachePosted = FALSE;
}

void BSDSModifier::UpdateSelLevelDisplay() {
	if (theBSDSModifierMainDlgProc.mod != this) return;
	if (!BSDSDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSDSDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theBSDSModifierMainDlgProc.UpdateSelLevelDisplay(hWnd);
}

static int butIDs[] = { 0, 0, 0, IDC_SELFACE, IDC_SELPOLY, IDC_SELELEMENT };
void BSDSModifierMainDlgProc::UpdateSelLevelDisplay(HWND hWnd) {
	if (!mod) return;
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hWnd, IDC_MS_SELTYPE));
	ICustButton *but;
	DWORD level = mod->ip->GetSubObjectLevel();
	DWORD selLevel = mod->GetSelectionLevel();
	for (int i = 3; i < 6; i++) {
		but = iToolbar->GetICustButton(butIDs[i]);
		but->SetCheck(level != 0 && (DWORD)i == selLevel);
		ReleaseICustButton(but);
	}
	ReleaseICustToolbar(iToolbar);

	if (mActivePart) {
		int nParts = mod->GetNumPartitions();
		int nActive = mod->GetActivePartition();
		mActivePart->SetLimits(0, nParts - 1, TRUE);
		mActivePart->SetValue(nActive, 0);
	}
	mCbMaterial.select(EnumToIndex(mod->GetFlags().bodyPart, BodyPartFlags));

	UpdateWindow(hWnd);
}

static bool oldShowEnd;

void BSDSModifier::BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;
	editMod = this;

	// Use our classdesc2 to put up our parammap2 maps:
	BSDSDesc.BeginEditParams(ip, this, flags, prev);
	theBSDSModifierMainDlgProc.mod = this;
	BSDSDesc.SetUserDlgProc(&BSDS_desc, ms_map_main, &theBSDSModifierMainDlgProc);

	selectMode = new SelectModBoxCMode(this, ip);

	// Assign the selection level.
	// ip->SetSubObjectLevel( GetActivePartition() + 1 );

	SetEnableStates();
	UpdateSelLevelDisplay();
	SetNumSelLabel();

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult(GetFlag(MS_DISP_END_RESULT));

	NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
}

void BSDSModifier::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	BSDSDesc.EndEditParams(ip, this, flags, next);
	theBSDSModifierMainDlgProc.mod = NULL;

	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	this->ip = NULL;
	// Reset show end result
	SetFlag(MS_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
	ip->SetShowEndResult(oldShowEnd);

	editMod = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

int BSDSModifier::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {

#if VERSION_3DSMAX >= (15000<<16) // Version 15 (2013)
	if (!vpt || !vpt->IsAlive())
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}
#endif

	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;

	int selByVert, ignoreBackfaces;
	pblock->GetValue(ms_by_vertex, t, selByVert, FOREVER);
	pblock->GetValue(ms_ignore_backfacing, t, ignoreBackfaces, FOREVER);

	// Setup GW
	MakeHitRegion(hr, type, crossing, 4, p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfaces) gw->setRndLimits(gw->getRndLimits() | GW_BACKCULL);
	else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();

	SubObjHitList hitList;
	MeshSubHitRec *rec;

	if (!mc->localData || !((BSDSData*)mc->localData)->GetMesh()) return 0;

	DWORD selLevel = GetSelectionLevel();
	DWORD hitFlags;
	if (selByVert) {
		hitFlags = SUBHIT_VERTS;
		if (selLevel > SEL_FACE) hitFlags |= SUBHIT_USEFACESEL;
	}
	else {
		hitFlags = hitLevel[selLevel];
	}

	Mesh &mesh = *((BSDSData*)mc->localData)->GetMesh();

	// cache backfacing vertices as hidden:
	BitArray oldHide;
	if ((hitFlags & SUBHIT_VERTS) && ignoreBackfaces) {
		BOOL flip = mat.Parity();
		oldHide = mesh.vertHide;
		BitArray faceBack;
		faceBack.SetSize(mesh.getNumFaces());
		faceBack.ClearAll();
		for (int i = 0; i < mesh.getNumFaces(); i++) {
			DWORD *vv = mesh.faces[i].v;
			IPoint3 A[3];
			for (int j = 0; j < 3; j++) gw->wTransPoint(&(mesh.verts[vv[j]]), &(A[j]));
			IPoint3 d1 = A[1] - A[0];
			IPoint3 d2 = A[2] - A[0];
			if (flip) {
				if ((d1^d2).z > 0) continue;
			}
			else {
				if ((d1^d2).z < 0) continue;
			}
			for (int j = 0; j < 3; j++) mesh.vertHide.Set(vv[j]);
			faceBack.Set(i);
		}
		for (int i = 0; i < mesh.getNumFaces(); i++) {
			if (faceBack[i]) continue;
			DWORD *vv = mesh.faces[i].v;
			for (int j = 0; j < 3; j++) mesh.vertHide.Clear(vv[j]);
		}
		mesh.vertHide |= oldHide;
	}

	res = mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr, flags | hitFlags, hitList);

	if ((hitFlags & SUBHIT_VERTS) && ignoreBackfaces) mesh.vertHide = oldHide;

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode, mc, rec->dist, rec->index, NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);
	return res;
}

int BSDSModifier::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
#if VERSION_3DSMAX >= (15000<<16) // Version 15 (2013)
	if (!vpt || !vpt->IsAlive())
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}
#endif
	if (!ip->GetShowEndResult()) return 0;
	if (!mc->localData) return 0;

	LocalModData *md = mc->localData;
	if (!md) return 0;
	BSDSData *modData = (BSDSData *)(IBSDismemberSkinModifierData *)md->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA);
	if (!modData)
		return 0;

	if (ip->GetSubObjectLevel() == 0)
		return 0;

	DWORD selLevel = modData->GetSelectionLevel();
	if (!selLevel) return 0;

	Mesh *mesh = modData->GetMesh();
	if (!mesh) return 0;

	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	// We need to draw a "gizmo" version of the mesh:
	Point3 colSel = GetSubSelColor();
	Point3 colTicks = GetUIColor(COLOR_VERT_TICKS);
	Point3 colGiz = GetUIColor(COLOR_GIZMOS);
	Point3 colGizSel = GetUIColor(COLOR_SEL_GIZMOS);
	gw->setColor(LINE_COLOR, colGiz);

	AdjEdgeList *ae = modData->GetAdjEdgeList();
	Point3 rp[3];
	int i, ect = ae->edges.Count();

#ifdef MESH_CAGE_BACKFACE_CULLING
	// Figure out backfacing from frontfacing.
	BitArray fBackfacing, vBackfacing;
	bool backCull = (savedLimits & GW_BACKCULL) ? true : false;
	if (backCull)
	{
		fBackfacing.SetSize(mesh->numFaces);
		vBackfacing.SetSize(mesh->numVerts);
		vBackfacing.SetAll();
		BitArray nonIsoVerts;
		nonIsoVerts.SetSize(mesh->numVerts);

		mesh->checkNormals(false);	// Allocates rVerts.
		BOOL gwFlipped = gw->getFlipped();

		for (i = 0; i < mesh->numVerts; i++) {
			mesh->getRVert(i).rFlags = (mesh->getRVert(i).rFlags & ~(GW_PLANE_MASK | RND_MASK | RECT_MASK)) |
				gw->hTransPoint(&(mesh->verts[i]), (IPoint3 *)mesh->getRVert(i).pos);
		}
		for (i = 0; i < mesh->numFaces; i++)
		{
			Face & f = mesh->faces[i];
			fBackfacing.Set(i, hIsFacingBack(mesh->getRVert(f.v[0]).pos, mesh->getRVert(f.v[1]).pos, mesh->getRVert(f.v[2]).pos, gwFlipped));
			for (int j = 0; j < 3; j++) nonIsoVerts.Set(f.v[j]);
			if (fBackfacing[i]) continue;
			for (int j = 0; j < 3; j++) vBackfacing.Clear(f.v[j]);
		}
		vBackfacing &= nonIsoVerts;	// so isolated vertices aren't labeled as backfacing.
	}
#endif

	int es[3];
	gw->startSegments();
	for (i = 0; i < ect; i++) {
		MEdge & me = ae->edges[i];
		if (me.Hidden(mesh->faces)) continue;
		if (me.Visible(mesh->faces)) {
			es[0] = GW_EDGE_VIS;
		}
		else {
			if (selLevel > SEL_FACE) continue;
			es[0] = GW_EDGE_INVIS;
		}

#ifdef MESH_CAGE_BACKFACE_CULLING
		if (backCull && fBackfacing[me.f[0]] && ((me.f[1] == UNDEFINED) || fBackfacing[me.f[1]]))
			continue;
#endif

		if (selLevel >= SEL_FACE) {
			if (ae->edges[i].AFaceSelected(modData->GetFaceSel())) gw->setColor(LINE_COLOR, colGizSel);
			else gw->setColor(LINE_COLOR, colGiz);
		}
		rp[0] = mesh->verts[me.v[0]];
		rp[1] = mesh->verts[me.v[1]];
		//gw->polyline (2, rp, NULL, NULL, FALSE, es);
		if (es[0] == GW_EDGE_VIS)
			gw->segment(rp, 1);
		else gw->segment(rp, 0);

	}
	gw->endSegments();
	gw->setRndLimits(savedLimits);
	return 0;
}

void BSDSModifier::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip->GetShowEndResult() || !mc->localData) return;
	BSDSData *modData = (BSDSData *)(IBSDismemberSkinModifierData *)mc->localData->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA);
	if (!modData) return;
	DWORD selLevel = modData->GetSelectionLevel();
	if (!selLevel) return;

	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = inode->GetObjectTM(t);
	box = mesh->getBoundingBox(&tm);
}

void BSDSModifier::GetSubObjectCenters(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc) {
	if (!mc->localData) return;
	BSDSData *modData = (BSDSData *)(IBSDismemberSkinModifierData *)mc->localData->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA);
	if (!modData) return;

	if (ip->GetSubObjectLevel() == 0) return;

	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i = 0; i < mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	cb->Center(box.Center(), 0);
}

void BSDSModifier::GetSubObjectTMs(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc) {
	if (!mc->localData) return;
	BSDSData *modData = (BSDSData *)(IBSDismemberSkinModifierData *)mc->localData->GetInterface(I_BSDISMEMBERSKINMODIFIERDATA);
	if (!modData) return;

	if (ip->GetSubObjectLevel() == 0) return;

	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i = 0; i < mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	Matrix3 ctm(1);
	ctm.SetTrans(box.Center());
	cb->TM(ctm, 0);
}

void BSDSModifier::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	if (IBSDismemberSkinModifierData* p = GetFirstModifierData())
	{
		// Fill in modes with our sub-object modes
		if (level > 0) {
			p->SetActivePartition(level - 1);
			modes = XFormModes(NULL, NULL, NULL, NULL, NULL, selectMode);
		}

		// Update UI
		UpdateSelLevelDisplay();
		SetEnableStates();
		SetNumSelLabel();

		NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
		ip->PipeSelLevelChanged();
		NotifyDependents(FOREVER, SELECT_CHANNEL | DISP_ATTRIB_CHANNEL | SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
	}
}

void BSDSModifier::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod == this) {
		SetNumSelLabel();

		// Setup subobjects
		if (IBSDismemberSkinModifierData* p = GetFirstModifierData())
		{
			Tab<BSDSPartitionData> &flags = p->GetPartitionFlags();

			bool changed = false;
			changed = (partitions.size() != flags.Count());
			partitions.resize(flags.Count());
			for (int i = 0; i < partitions.size(); ++i) {
				TSTR name = EnumToString(flags[i].bodyPart, BodyPartFlags);
				changed |= (name != partitions[i].GetNameRef()) ? true : false;
				partitions[i].SetName(name);
			}

			SetEnableStates();
			UpdateSelLevelDisplay();
			SetNumSelLabel();

			NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
			ip->PipeSelLevelChanged();
			NotifyDependents(FOREVER, SELECT_CHANNEL | DISP_ATTRIB_CHANNEL | SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);

			if (changed)
				NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
		}
	}
}

void BSDSModifier::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	BSDSData *d = NULL, *od = NULL;
	if (!ip) return;
	ip->ClearCurNamedSelSet();

	int selByVert, ignoreVisEdge;
	float planarThresh;
	TimeValue t = ip->GetTime();
	pblock->GetValue(ms_by_vertex, t, selByVert, FOREVER);
	pblock->GetValue(ms_ignore_visible, t, ignoreVisEdge, FOREVER);
	pblock->GetValue(ms_planar_threshold, t, planarThresh, FOREVER);

	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);
	int nd;
	BitArray nsel;
	AdjEdgeList *ae = NULL;
	AdjFaceList *af = NULL;
	Mesh *mesh;

	Tab<IBSDismemberSkinModifierData*> mcList = GetModifierData();
	for (nd = 0; nd < mcList.Count(); nd++) {
		d = (BSDSData *)mcList[nd];
		if (d == NULL) continue;
		HitRecord *hr = hitRec;
		if (!all && (hr->modContext->localData != d)) continue;
		for (; hr != NULL; hr = hr->Next()) if (hr->modContext->localData == d) break;
		if (hr == NULL) continue;

		mesh = d->GetMesh();
		if (!mesh) continue;
		BaseInterface *msci = mesh->GetInterface(MESHSELECTCONVERT_INTERFACE);

		if (selByVert) {
			ae = d->GetAdjEdgeList();
			if (!ae) continue;
		}

		DWORD selLevel = d->GetSelectionLevel();
		switch (selLevel) {
		case SEL_FACE:
			if (msci && selByVert) {
				// Use new improved selection conversion:
				BitArray vhit;
				vhit.SetSize(mesh->numVerts);
				for (hr = hitRec; hr != NULL; hr = hr->Next()) {
					if (d != hr->modContext->localData) continue;
					vhit.Set(hr->hitInfo);
					if (!all) break;
				}
#if VERSION_3DSMAX >= ((5000<<16)+(15<<8)+0)
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				pConv->VertexToFace(*mesh, vhit, nsel);
				if (invert) nsel ^= d->GetFaceSel();
				else {
					if (selected) nsel |= d->GetFaceSel();
					else nsel = d->GetFaceSel() & ~nsel;
				}
#endif
			}
			else {
				BitArray& faceSel = d->GetFaceSel();
				nsel = d->GetFaceSel();
				for (; hr != NULL; hr = hr->Next()) {
					if (d != hr->modContext->localData) continue;
					if (selByVert) {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i = 0; i < list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j = 0; j < 2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								nsel.Set(me.f[j], invert ? !faceSel[me.f[j]] : selected);
							}
						}
					}
					else {
						nsel.Set(hr->hitInfo, invert ? !faceSel[hr->hitInfo] : selected);
					}
					if (!all) break;
				}
			}
			d->SetFaceSel(nsel, this, t);
			break;

		case SEL_POLY:
		case SEL_ELEMENT:
			af = d->GetAdjFaceList();
			if (msci) {
				// Use new improved selection conversion:
#if VERSION_3DSMAX >= ((5000<<16)+(15<<8)+0)
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				if (selByVert) {
					BitArray vhit;
					vhit.SetSize(mesh->numVerts);
					for (hr = hitRec; hr != NULL; hr = hr->Next()) {
						if (d != hr->modContext->localData) continue;
						vhit.Set(hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SEL_ELEMENT) pConv->VertexToElement(*mesh, af, vhit, nsel);
					else pConv->VertexToPolygon(*mesh, af, vhit, nsel, planarThresh, ignoreVisEdge ? true : false);
				}
				else {
					BitArray fhit;
					fhit.SetSize(mesh->numFaces);
					for (hr = hitRec; hr != NULL; hr = hr->Next()) {
						if (d != hr->modContext->localData) continue;
						fhit.Set(hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SEL_ELEMENT) pConv->FaceToElement(*mesh, af, fhit, nsel);
					else pConv->FaceToPolygon(*mesh, af, fhit, nsel, planarThresh, ignoreVisEdge ? true : false);
				}
#endif
			}
			else {
				// Otherwise we'll take the old approach of converting faces to polygons or elements as we go.
				nsel.SetSize(mesh->numFaces);
				nsel.ClearAll();
				for (; hr != NULL; hr = hr->Next()) {
					if (d != hr->modContext->localData) continue;
					if (!selByVert) {
						if (selLevel == SEL_ELEMENT)
							mesh->ElementFromFace(hr->hitInfo, nsel, af);
						else
							mesh->PolyFromFace(hr->hitInfo, nsel, planarThresh, ignoreVisEdge, af);
					}
					else {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i = 0; i < list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j = 0; j < 2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								if (selLevel == SEL_ELEMENT)
									mesh->ElementFromFace(me.f[j], nsel, af);
								else
									mesh->PolyFromFace(me.f[j], nsel, planarThresh, ignoreVisEdge, af);
							}
						}
					}
					if (!all) break;
				}
			}

			if (invert) nsel ^= d->GetFaceSel();
			else if (selected) nsel |= d->GetFaceSel();
			else nsel = d->GetFaceSel() & ~nsel;

			d->SetFaceSel(nsel, this, t);
			break;
		}
	}

	nodes.DisposeTemporary();
	LocalDataChanged();
}

void BSDSModifier::ClearSelection(int selLevel) {
	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (d == NULL) continue;

		// Check if we have anything selected first:
		switch (d->GetSelectionLevel()) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			if (!d->GetFaceSel().NumberSet()) continue;
			else break;
		}

		if (theHold.Holding() && !d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SynchBitArrays();
		switch (d->GetSelectionLevel()) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->GetFaceSel().ClearAll();
			break;
		}
	}
	LocalDataChanged();
}

void BSDSModifier::SelectAll(int selLevel) {
	ModContextList context_list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(context_list, nodes);
	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->GetFaceSel().SetAll();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged();
}

void BSDSModifier::InvertSelection(int selLevel) {
	ModContextList context_list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(context_list, nodes);
	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->GetFaceSel().Reverse();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged();
}

void BSDSModifier::SelectByMatID(int id) {
	if (!ip) return;
	BOOL add = GetKeyState(VK_CONTROL) < 0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU) < 0 ? TRUE : FALSE;
	theHold.Begin();
	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);
	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SynchBitArrays();
		if (!add && !sub) d->GetFaceSel().ClearAll();
		Mesh *mesh = d->GetMesh();
		if (!mesh) continue;
		for (int i = 0; i < mesh->numFaces; i++) {
			if (mesh->faces[i].getMatID() == (MtlID)id) {
				if (sub) d->GetFaceSel().Clear(i);
				else d->GetFaceSel().Set(i);
			}
		}
	}
	nodes.DisposeTemporary();
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));

	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

void BSDSModifier::SelectFrom(int from) {
	if (!ip) return;
	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);
	BSDSData *d;
	int ignoreVisEdge;
	float planarThresh;
	pblock->GetValue(ms_ignore_visible, TimeValue(0), ignoreVisEdge, FOREVER);
	pblock->GetValue(ms_planar_threshold, TimeValue(0), planarThresh, FOREVER);

	theHold.Begin();
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;

		if (!d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SynchBitArrays();
	}
	theHold.Accept(GetString(IDS_DS_SELECT));
	nodes.DisposeTemporary();
	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

// old-style parameter block for loading old scenes:
#define NUM_OLDVERSIONS	0

#define SELLEVEL_CHUNKID		0x0100
#define FLAGS_CHUNKID			0x0240
#define VERSION_CHUNKID			0x0230
static int currentVersion = 1;

#define NAMEDVSEL_NAMES_CHUNK	0x2805
#define NAMEDFSEL_NAMES_CHUNK	0x2806
#define NAMEDESEL_NAMES_CHUNK	0x2807
#define NAMEDSEL_STRING_CHUNK	0x2809
#define NAMEDSEL_ID_CHUNK		0x2810

#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847
#define CURSEL_CHUNK			   0x2848
#define PARTFLAGS_CHUNK			0x2849

static int namedSelID[] = { NAMEDVSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK };

IOResult BSDSModifier::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write(&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	DWORD flags = ExportFlags();
	isave->BeginChunk(FLAGS_CHUNKID);
	res = isave->Write(&flags, sizeof(DWORD), &nb);
	isave->EndChunk();

	return res;
}

IOResult BSDSModifier::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	int version = 1;
	DWORD flags;

	Modifier::Load(iload);

	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case VERSION_CHUNKID:
			iload->Read(&version, sizeof(version), &nb);
			break;

		case FLAGS_CHUNKID:
			iload->Read(&flags, sizeof(DWORD), &nb);
			ImportFlags(flags);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}
	return IO_OK;
}

IOResult BSDSModifier::SaveLocalData(ISave *isave, LocalModData *ld) {
	BSDSData *d = (BSDSData*)ld;

	if (d->fselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->fselSet.Save(isave);
		isave->EndChunk();
	}

	ULONG nb;
	IOResult res;
	isave->BeginChunk(CURSEL_CHUNK);
	res = isave->Write(&d->activePartition, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(PARTFLAGS_CHUNK);
	int n = d->flags.Count();
	res = isave->Write(&n, sizeof(int), &nb);
	for (int i = 0; i < n; ++i) {
		BSDSPartitionData& flag = d->flags[i];
		res = isave->Write(&flag.partFlag, sizeof(int), &nb);
		res = isave->Write((DWORD*)&flag.bodyPart, sizeof(int), &nb);
		res = isave->Write(&flag.selLevel, sizeof(int), &nb);
	}
	isave->EndChunk();

	return IO_OK;
}

IOResult BSDSModifier::LoadLocalData(ILoad *iload, LocalModData **pld) {
	BSDSData *d = new BSDSData;
	ULONG nb = 0;
	*pld = d;
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case FSELSET_CHUNK:
			while (d->fselSet.Count() > 0)
				d->fselSet.DeleteSet(0);
			d->fselSet.SetSize(0);
			res = d->fselSet.Load(iload);
			break;
		case CURSEL_CHUNK:
			iload->Read(&d->activePartition, sizeof(d->activePartition), &nb);
			break;

		case PARTFLAGS_CHUNK:
		{
			int n = 0;
			iload->Read(&n, sizeof(n), &nb);
			d->flags.SetCount(n);
			for (int i = 0; i < n; ++i) {
				BSDSPartitionData& flag = d->flags[i];
				res = iload->Read(&flag.partFlag, sizeof(int), &nb);
				res = iload->Read((DWORD*)&flag.bodyPart, sizeof(int), &nb);
				res = iload->Read(&flag.selLevel, sizeof(int), &nb);
			}
		}
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}
	LocalDataChanged();
	return IO_OK;
}


// Window Procs ------------------------------------------------------

void BSDSModifier::SetEnableStates() {
	if (!BSDSDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSDSDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theBSDSModifierMainDlgProc.SetEnables(hWnd);
}

void BSDSModifierMainDlgProc::SetEnables(HWND hParams) {
	if (!mod) return;
	int selLevel = mod->GetSelectionLevel();
	ICustButton *but;
	ISpinnerControl *spin;

	int nSub = mod->ip->GetSubObjectLevel();
	bool subSel = (nSub != 0);

	bool poly = (selLevel == SEL_POLY) ? true : false;
	bool face = (selLevel == SEL_FACE) || (selLevel == SEL_ELEMENT) || poly ? TRUE : FALSE;

	EnableWindow(GetDlgItem(hParams, IDC_MS_SEL_BYVERT), subSel && face);
	//EnableWindow (GetDlgItem (hParams, IDC_MS_IGNORE_BACKFACES), subSel&&(edge||face));
	EnableWindow(GetDlgItem(hParams, IDC_MS_IGNORE_VISEDGE), subSel&&poly);
	EnableWindow(GetDlgItem(hParams, IDC_MS_PLANAR_TEXT), subSel&&poly);
	spin = GetISpinner(GetDlgItem(hParams, IDC_MS_PLANARSPINNER));
	spin->Enable(subSel&&poly);
	ReleaseISpinner(spin);

	EnableWindow(GetDlgItem(hParams, IDC_MS_IGNORE_BACKFACES), subSel);

	EnableWindow(GetDlgItem(hParams, IDC_MS_SELBYMAT_BOX), subSel&&face);
	EnableWindow(GetDlgItem(hParams, IDC_MS_SELBYMAT_TEXT), subSel&&face);
	but = GetICustButton(GetDlgItem(hParams, IDC_MS_SELBYMAT));
	but->Enable(subSel&&face);
	ReleaseICustButton(but);

	spin = GetISpinner(GetDlgItem(hParams, IDC_MS_MATIDSPIN));
	spin->Enable(subSel&&face);
	ReleaseISpinner(spin);

	int npart = mod->GetNumPartitions();
	mDelPart->Enable((npart > 1) ? TRUE : FALSE);

	but = GetICustButton(GetDlgItem(hParams, IDC_MS_SELUNUSED));
	but->Enable(subSel);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hParams, IDC_MS_FIXDUPL));
	but->Enable(subSel);
	ReleaseICustButton(but);

}

INT_PTR BSDSModifierMainDlgProc::DlgProc(TimeValue t, IParamMap2 *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) return FALSE;
	ICustToolbar *iToolbar;
	ICustButton *but;
	int matid;
	int nParts;
	int nActive;
	int objLevel;

	switch (msg) {
	case WM_INITDIALOG:

		mCbMaterial.init(GetDlgItem(hWnd, IDC_CB_MATERIAL));
		for (const EnumLookupType* bpFlag = BodyPartFlags; bpFlag->name != NULL; ++bpFlag) {
			mCbMaterial.add(bpFlag->name);
		}
		mCbMaterial.select(EnumToIndex(mod->GetFlags().bodyPart, BodyPartFlags));

		nParts = mod->GetNumPartitions();
		nActive = mod->GetActivePartition();
		objLevel = mod->GetSubObjectLevel();
		EnableWindow(GetDlgItem(hWnd, IDC_MS_PARTITION), TRUE);
		mActivePart = GetISpinner(GetDlgItem(hWnd, IDC_MS_PARTSPIN));
		mActivePart->LinkToEdit(GetDlgItem(hWnd, IDC_MS_PARTITION), EDITTYPE_POS_INT);
		mActivePart->SetValue(nActive, 0);
		mActivePart->SetLimits(0, nParts - 1, TRUE);
		mActivePart->SetAutoScale(FALSE);
		mActivePart->SetScale(1);
		mActivePart->SetResetValue(0);

		mAddPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_ADDPART));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mAddPart->SetTooltip(TRUE, "Create New Partition");
#endif
		mAddPart->SetImage(theBSDSPartImageHandler.LoadImages(), 0, 2, 0, 2, 16, 16);

		mDelPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_DELPART));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mDelPart->SetTooltip(TRUE, "Delete Active Partition");
#endif
		mDelPart->SetImage(theBSDSPartImageHandler.LoadImages(), 1, 3, 1, 3, 16, 16);

		iToolbar = GetICustToolbar(GetDlgItem(hWnd, IDC_MS_SELTYPE));
		iToolbar->SetImage(theBSDSImageHandler.LoadImages());
		//iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		//iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 2, 7, 2, 7, 24, 23, 24, 23, IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 3, 8, 3, 8, 24, 23, 24, 23, IDC_SELPOLY));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 4, 9, 4, 9, 24, 23, 24, 23, IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);

#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 11+
		but = GetICustButton(GetDlgItem(hWnd, IDC_MS_SELUNUSED));
		but->SetTooltip(TRUE, "Select unused faces into active partition");
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd, IDC_MS_FIXDUPL));
		but->SetTooltip(TRUE, "Remove duplicate faces from partitions");
		ReleaseICustButton(but);
#endif

		UpdateSelLevelDisplay(hWnd);
		SetEnables(hWnd);
		break;

	case WM_NCDESTROY:
		if (mActivePart) {
			ReleaseISpinner(mActivePart);
			mActivePart = NULL;
		}
		if (mAddPart) {
			ReleaseICustButton(mAddPart);
			mAddPart = NULL;
		}
		if (mDelPart) {
			ReleaseICustButton(mDelPart);
			mDelPart = NULL;
		}
		break;

	case WM_UPDATE_CACHE:
		mod->UpdateCache((TimeValue)wParam);
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_MS_PARTSPIN:
			if (IBSDismemberSkinModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActivePart->GetIVal();
				p->SetActivePartition(idx);
				mod->ip->SetSubObjectLevel(idx + 1);
			}
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_MS_PARTITION:
			if (IBSDismemberSkinModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActivePart->GetIVal();
				p->SetActivePartition(idx);
				mod->ip->SetSubObjectLevel(idx + 1);
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_CB_MATERIAL:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int sel = mCbMaterial.selection();
				mod->GetFlags().bodyPart = DismemberBodyPartType(BodyPartFlags[sel].value);
				mod->LocalDataChanged();
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
			}
			break;

		case IDC_MS_ADDPART:
			if (mActivePart) {
				if (IBSDismemberSkinModifierData* p = mod->GetFirstModifierData()) {
					int idx = p->AddPartition();
					nParts = mod->GetNumPartitions();
					mod->ip->SetSubObjectLevel(idx + 1);
					mActivePart->SetLimits(0, nParts - 1, TRUE);
					mActivePart->SetValue(idx, TRUE);
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_DELPART:
			if (mActivePart) {
				if (IBSDismemberSkinModifierData* p = mod->GetFirstModifierData()) {
					int idx = mActivePart->GetIVal();
					p->RemovePartition(idx);
					mod->ip->SetSubObjectLevel(p->GetActivePartition() + 1);
					nParts = mod->GetNumPartitions();
					mActivePart->SetLimits(0, nParts, TRUE);
					mActivePart->SetValue((int)p->GetActivePartition(), TRUE);
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_FIXDUPL:
			mod->FixDuplicates();
			break;

		case IDC_MS_SELUNUSED:
			mod->SelectUnused();
			break;

		case IDC_MS_SELBYMAT:
			mod->pblock->GetValue(ms_matid, t, matid, FOREVER);
			mod->SelectByMatID(matid - 1);
			break;

		case IDC_SELFACE:
			if (mod->GetSelectionLevel() != SEL_FACE) {
				mod->SetSelectionLevel(SEL_FACE);
				mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			else {
				if (mod->ip->GetSubObjectLevel() != 0)
					mod->ip->SetSubObjectLevel(0);
				else
					mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			UpdateWindow(hWnd);
			break;
		case IDC_SELPOLY:
			if (mod->GetSelectionLevel() != SEL_POLY) {
				mod->SetSelectionLevel(SEL_POLY);
				mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			else {
				if (mod->ip->GetSubObjectLevel() != 0)
					mod->ip->SetSubObjectLevel(0);
				else
					mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			UpdateWindow(hWnd);
			break;
		case IDC_SELELEMENT:
			if (mod->GetSelectionLevel() != SEL_ELEMENT) {
				mod->SetSelectionLevel(SEL_ELEMENT);
				mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			else {
				if (mod->ip->GetSubObjectLevel() != 0)
					mod->ip->SetSubObjectLevel(0);
				else
					mod->ip->SetSubObjectLevel(mod->GetActivePartition() + 1);
			}
			UpdateWindow(hWnd);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;
		switch (lpttt->hdr.idFrom) {
			//case IDC_SELVERTEX:
			//	lpttt->lpszText = GetString (IDS_RB_VERTEX);
			//	break;
			//case IDC_SELEDGE:
			//	lpttt->lpszText = GetString (IDS_RB_EDGE);
			//	break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_RB_FACE);
			break;
		case IDC_SELPOLY:
			lpttt->lpszText = GetString(IDS_EM_POLY);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_EM_ELEMENT);
			break;
		}
		break;

	default: return FALSE;
	}
	return TRUE;
}

// BSDSData -----------------------------------------------------

LocalModData *BSDSData::Clone() {
	BSDSData *d = new BSDSData;
	d->activePartition = activePartition;
	d->flags = flags;
	d->fselSet = fselSet;
	return d;
}

BSDSData::BSDSData(Mesh &mesh) {
	held = 0; this->mesh = NULL; temp = NULL;
	SetActivePartition(0);
	GetFaceSel() = mesh.faceSel;
}

BSDSData::BSDSData()
{
	held = 0; mesh = NULL; temp = NULL;
	SetActivePartition(0);
}

void BSDSData::SynchBitArrays() {
	if (!mesh) return;
	if (GetFaceSel().GetSize() != mesh->getNumFaces())
		GetFaceSel().SetSize(mesh->getNumFaces(), TRUE);
}

AdjEdgeList *BSDSData::GetAdjEdgeList() {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh(mesh);
	return temp->AdjEList();
}

AdjFaceList *BSDSData::GetAdjFaceList() {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh(mesh);
	return temp->AdjFList();
}

void BSDSData::SetCache(Mesh &mesh) {
	if (this->mesh) delete this->mesh;
	this->mesh = new Mesh(mesh);
	if (temp) temp->Invalidate(PART_TOPO);
	SynchBitArrays();
}

void BSDSData::FreeCache() {
	if (mesh) delete mesh;
	mesh = NULL;
	if (temp) delete temp;
	temp = NULL;
}

void BSDSData::SetFaceSel(int index, BitArray &set, IBSDismemberSkinModifier *imod, TimeValue t) {
	BSDSModifier *mod = (BSDSModifier *)imod;
	if (theHold.Holding()) theHold.Put(new BSDSSelectRestore(mod, this, SEL_FACE));
	GetFaceSel(index) = set;
	if (mesh) mesh->faceSel = set;
}

void BSDSData::SetActivePartition(DWORD partition)
{
	activePartition = partition;
	while (flags.Count() <= partition) {
		BSDSPartitionData flag;
		flag.bodyPart = BP_TORSO;
		flag.partFlag = 1;
		flag.selLevel = SEL_FACE;
		flags.Append(1, &flag);
	}
	BitArray empty;
	while (fselSet.Count() <= partition) {
		fselSet.AppendSet(empty);
	}
}

void BSDSData::RemovePartition(DWORD partition)
{
	if (partition < flags.Count() && flags.Count() > 1)
	{
		flags.Delete(partition, 1);
		fselSet.DeleteSet(partition);
		if (partition >= flags.Count())
			SetActivePartition(0);
	}
}

void BSDSData::SelectUnused()
{
	SynchBitArrays();
	Mesh *mesh = GetMesh();
	if (!mesh) return;
	int nList = fselSet.Count();
	BitArray& activeFSel = GetFaceSel();
	for (int i = 0; i < mesh->numFaces; i++) {
		bool found = false;
		for (int j = nList - 1; j >= 0; --j) {
			if (fselSet[j][i]) {
				found = true;
			}
		}
		if (!found)
			activeFSel.Set(i);
	}
}

//////////////////////////////////////////////////////////////////////////
// Remove duplicate faces
//  If Ctrl pressed then active partition is greedy and gets the face 
//    if already selected in partition regardless of other partitions
void BSDSData::FixDuplicates()
{
	BOOL add = GetKeyState(VK_CONTROL) < 0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU) < 0 ? TRUE : FALSE;

	Mesh *mesh = GetMesh();
	if (!mesh) return;
	SynchBitArrays();

	int nList = fselSet.Count();
	BitArray& activeFSel = GetFaceSel();
	for (int i = 0; i < mesh->numFaces; i++) {
		bool found = false;
		bool active = activeFSel[i];
		if (add) found = active;
		for (int j = nList - 1; j >= 0; --j) {
			if (fselSet[j][i]) {
				if (found)
					fselSet[j].Clear(i);
				found = true;
			}
		}
		if (add) activeFSel.Set(i, active);
	}
}
// BSDSSelectRestore --------------------------------------------------

BSDSSelectRestore::BSDSSelectRestore(BSDSModifier *m, BSDSData *data) {
	mod = m;
	level = data->GetSelectionLevel();
	d = data;
	d->held = TRUE;
	usel = d->GetFaceSel();
}

BSDSSelectRestore::BSDSSelectRestore(BSDSModifier *m, BSDSData *data, int sLevel) {
	mod = m;
	level = sLevel;
	d = data;
	d->held = TRUE;
	usel = d->GetFaceSel();
}

void BSDSSelectRestore::Restore(int isUndo) {
	if (isUndo) {
		switch (level) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			rsel = d->GetFaceSel(); break;
		}
	}
	switch (level) {
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		d->GetFaceSel() = usel; break;
	}
	mod->LocalDataChanged();
}

void BSDSSelectRestore::Redo() {
	switch (level) {
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		d->GetFaceSel() = rsel; break;
	}
	mod->LocalDataChanged();
}


//--- Named selection sets -----------------------------------------


void BSDSModifier::ActivateSubSelSet(int index) {
	if (index < 0 || !ip) return;

	theHold.Begin();
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);

	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		BSDSData *meshData = (BSDSData *)list[i];
		if (!meshData) continue;
		if (theHold.Holding() && !meshData->held) theHold.Put(new BSDSSelectRestore(this, meshData));
		meshData->SetActivePartition(index);
	}

	nodes.DisposeTemporary();
	LocalDataChanged();
	theHold.Accept(GetString(IDS_DS_SELECT));
	ip->RedrawViews(ip->GetTime());
}


void BSDSModifier::SetNumSelLabel() {
	TSTR buf;
	int num = 0;
	int which = 0;

	if (!BSDSDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSDSDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hParams = pmap->GetHWnd();
	if (!hParams) return;

	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);

	DWORD selLevel = SEL_FACE;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		BSDSData *meshData = (BSDSData *)list[i];
		if (!meshData) continue;

		selLevel = meshData->GetSelectionLevel();
		switch (selLevel) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			num += meshData->GetFaceSel().NumberSet();
			if (meshData->GetFaceSel().NumberSet() == 1) {
				for (which = 0; which < meshData->GetFaceSel().GetSize(); which++) if (meshData->GetFaceSel()[which]) break;
			}
			break;
		}
	}

	switch (selLevel) {
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		if (num == 1) buf.printf(GetString(IDS_EM_WHICHFACESEL), which + 1);
		else buf.printf(GetString(IDS_RB_NUMFACESELP), num);
		break;
	}

	SetDlgItemText(hParams, IDC_MS_NUMBER_SEL, buf);

	nodes.DisposeTemporary();
}

#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
RefResult BSDSModifier::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
#else
RefResult BSDSModifier::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
#endif
{
	if ((message == REFMSG_CHANGE) && (hTarget == pblock)) {
		// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
		// will contain ID to update, else it will be -1 => inval whole rollout
		ParamID pid = pblock->LastNotifyParamID();
		InvalidateDialogElement(pid);
	}
	return(REF_SUCCEED);
}

int BSDSModifier::NumSubObjTypes()
{
	return partitions.size();
}

ISubObjType *BSDSModifier::GetSubObjType(int i)
{
	if (i >= partitions.size())
		return NULL;
	if (i < 0) {
		if (GetActivePartition() > 0)
			return GetSubObjType(GetActivePartition() - 1);
		return NULL;
	}
	return &partitions[i];
}

void BSDSModifier::InvalidateDialogElement(int elem) {
	if (!pblock) return;
	if (!BSDSDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSDSDesc.GetParamMap(ms_map_main);
	if (pmap) pmap->Invalidate(elem);
}

void BSDSModifier::FixDuplicates()
{
	theHold.Begin();

	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->FixDuplicates();
	}
	theHold.Accept(GetString(IDS_RB_FIXDUPL));

	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

void BSDSModifier::SelectUnused()
{
	theHold.Begin();

	BSDSData *d;
	Tab<IBSDismemberSkinModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSDSData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSDSSelectRestore(this, d));
		d->SelectUnused();
	}
	theHold.Accept(GetString(IDS_RB_SELUNUSED));

	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

// Get or Create the Skin Modifier
Modifier *GetOrCreateBSDismemberSkin(INode *node)
{
	Modifier *skinMod = GetBSDismemberSkin(node);
	if (skinMod)
		return skinMod;

	Object *pObj = node->GetObjectRef();
	IDerivedObject *dobj = nullptr;
	if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		dobj = static_cast<IDerivedObject*>(pObj);
	else {
		dobj = CreateDerivedObject(pObj);
	}
	//create a skin modifier and add it
	skinMod = (Modifier*)CreateInstance(OSM_CLASS_ID, BSDSMODIFIER_CLASS_ID);
	dobj->SetAFlag(A_LOCK_TARGET);
	dobj->AddModifier(skinMod);
	dobj->ClearAFlag(A_LOCK_TARGET);
	node->SetObjectRef(dobj);
	return skinMod;
}

Modifier *GetBSDismemberSkin(INode *node)
{
	Object* pObj = node->GetObjectRef();
	if (!pObj) return NULL;
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);
			if (mod->ClassID() == BSDSMODIFIER_CLASS_ID)
				return mod;
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	return NULL;
}
