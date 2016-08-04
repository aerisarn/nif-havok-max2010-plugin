/**********************************************************************
*<
FILE: BSSubIndexModifier.cpp

CREATED BY: 

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

#define SEL_OBJECT  0
#define SEL_FACE	3
#define SEL_POLY	4
#define SEL_ELEMENT 5

//#define CHECKHEAP() {HeapValidate(GetProcessHeap(), 0, NULL);}
#define CHECKHEAP()

static EnumLookupType DefaultBodyPartFlags[] =
{
	{ 0xffffffff, TEXT("---------------") },
	{ 0,  NULL },
};

static NameValueList BSSubIndexMaterials;
static EnumLookupType *BodyPartFlagsCache;
EnumLookupType *BodyPartFlags = DefaultBodyPartFlags;

class PartSubObjType : public ISubObjType {
	TSTR name;
public:
	PartSubObjType() {}
	PartSubObjType(const PartSubObjType& rhs) { name = rhs.name; }
	PartSubObjType& operator=(const PartSubObjType& rhs) { name = rhs.name; return *this; }
	void SetName(const TCHAR *nm) { name = nm; }
#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
	TSTR& GetNameRef() { return name; }
	TCHAR *GetName() { return name; }
#else
	TSTR& GetNameRef() { return name; }
	const MSTR& GetName() { return name; }
#endif
	MaxIcon *GetIcon() { return NULL; }
};


// Named selection set levels:
const DWORD WM_UPDATE_CACHE = (WM_USER + 0x2000);

// Image list used for mesh sub-object toolbar in Edit Mesh, Mesh Select:
class BSSIImageHandler {
public:
	HIMAGELIST images;
	BSSIImageHandler() { images = NULL; }
	~BSSIImageHandler() { if (images) ImageList_Destroy(images); }
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

class BSSIPartImageHandler {
public:
	HIMAGELIST images;
	BSSIPartImageHandler() { images = NULL; }
	~BSSIPartImageHandler() { if (images) ImageList_Destroy(images); }
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
class BSSILockImageHandler {
public:
	HIMAGELIST images;
	BSSILockImageHandler() { images = NULL; }
	~BSSILockImageHandler() { if (images) ImageList_Destroy(images); }
	HIMAGELIST LoadImages() {
		if (images) return images;
		images = ImageList_Create(14, 14, ILC_COLOR32 | ILC_MASK, 4, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_LOCK));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_LOCKMASK));
		ImageList_Add(images, hBitmap, hMask);
		DeleteObject(hBitmap), DeleteObject(hMask);
		return images;
	}
};
// Local static instance.
static BSSIImageHandler theBSSIImageHandler;
static BSSIPartImageHandler theBSSIPartImageHandler;
static BSSILockImageHandler theBSSILockImageHandler;

#define IDC_SELFACE 0x3262
#define IDC_SELPOLY 0x3263
#define IDC_SELELEMENT 0x3264
extern int *meshSubTypeToolbarIDs;

// BSSIModifier partitions:
#define MS_DISP_END_RESULT 0x01

// BSSIModifier References:
#define REF_PBLOCK 0

class BSSIModifier : public Modifier, public IBSSubIndexModifier, public FlagUser {
public:
	IParamBlock2 *pblock;
	static IObjParam *ip;
	static BSSIModifier *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;
	Tab<PartSubObjType*> segments;
	BOOL inLocalDataChanged;

	BSSIModifier();
	~BSSIModifier()
	{
		for (int i = segments.Count() - 1; i >= 0; --i) {
			if (PartSubObjType *subobj = segments[i]) delete subobj;
			segments.Delete(i, 1);
		}
	}

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_RB_BSSIMODIFIER); }
	virtual Class_ID ClassID() { return BSSIMODIFIER_CLASS_ID; }
	RefTargetHandle Clone(RemapDir& remap /*= DefaultRemapDir()*/);
#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
	TCHAR *                 GetObjectName() { return _T(GetString(IDS_RB_BSSIMODIFIER)); }
#else
	const MCHAR*             GetObjectName() { return GetString(IDS_RB_BSSIMODIFIER); }
#endif
	void* GetInterface(ULONG id) {
		return (id == I_BSSUBINDEXSKINMODIFIER) ? static_cast<IBSSubIndexModifier*>(this) : Modifier::GetInterface(id);
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

public: //IBSSubIndexModifier

   /*! \remarks This method must be called when the <b>LocalModData</b> of
   the modifier is changed. Developers can use the methods of
   <b>IMeshSelectData</b> to get and set the actual selection for vertex, face
   and edge. When a developers does set any of these selection sets this
   method must be called when done. */
	virtual void LocalDataChanged();

	/*! \remarks Gets all of the mod contexts related to this modifier. */
	virtual Tab<IBSSubIndexModifierData*> GetModifierData() {
		Tab<IBSSubIndexModifierData*> data;
		SelectModContextEnumProc selector;
		this->EnumModContexts(&selector);
		for (int i = 0; i < selector.mcList.Count(); ++i) {
			LocalModData *md = selector.mcList[i]->localData;
			if (!md) continue;
			if (IBSSubIndexModifierData* p = (IBSSubIndexModifierData*)md->GetInterface(I_BSSUBINDEXMODIFIERDATA))
				data.Append(1, &p);
		}
		return data;
	}

	// Helper Methods

	IBSSubIndexModifierData* GetFirstModifierData()
	{
		SelectModContextEnumProc selector;
		this->EnumModContexts(&selector);
		for (int i = 0; i < selector.mcList.Count(); ++i) {
			LocalModData *md = selector.mcList[i]->localData;
			if (!md) continue;
			if (IBSSubIndexModifierData* p = (IBSSubIndexModifierData*)md->GetInterface(I_BSSUBINDEXMODIFIERDATA))
				return p;
		}
		return NULL;
	}

	int GetNumPartitions()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetNumPartitions();
		}
		return -1;
	}

	int GetActivePartition()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetActivePartition();
		}
		return -1;
	}

	BSSubIndexData& GetCurrentPartition()
	{
		static BSSubIndexData empty;
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetPartition(p->GetActivePartition());
		}
		return empty;
	}

	DWORD GetSelectionLevel()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			BSSubIndexData &flags = p->GetPartition(p->GetActivePartition());
			return flags.selLevel;
		}
		return SEL_OBJECT;
	}

	void SetSelectionLevel(DWORD level)
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			BSSubIndexData &flags = p->GetPartition(p->GetActivePartition());
			flags.selLevel = level;

			UpdateSelLevelDisplay();
			SetEnableStates();
			SetNumSelLabel();

			NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
			ip->PipeSelLevelChanged();
			NotifyDependents(FOREVER, SELECT_CHANNEL | DISP_ATTRIB_CHANNEL | SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
		}
	}

	int GetActiveSubPartition()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetActiveSubPartition();
		}
		return -1;
	}


	int GetSubPartitionCount()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetActivePartitionSubCount();
		}
		return -1;
	}

	BOOL GetPartitionEditEnabled()
	{
		if (IBSSubIndexModifierData* p = GetFirstModifierData()) {
			return p->GetPartitionEditEnabled();
		}
		return FALSE;
	}
};

class BSSIData : public LocalModData, public IBSSubIndexModifierData
{
private:
	// Temp data used for soft selections, adjacent edge / face lists.
	MeshTempData *temp;

public:
	// LocalModData
	void* GetInterface(ULONG id) {
		return (id == I_BSSUBINDEXMODIFIERDATA) ? (IBSSubIndexModifierData*)this : LocalModData::GetInterface(id);
	}

	// Selection sets
	BOOL enablePartitionEdit;
	DWORD activePartition;
	DWORD activeSubPartition;
	vector<BSSubIndexData> partitions;
	TSTR ssfFile;

	// Lists of named selection sets
	GenericNamedSelSetList fselSet;
	BOOL held;
	Mesh *mesh;

	BSSIData(Mesh &mesh);
	BSSIData();
	~BSSIData() { BSSIData::FreeCache(); }

	DWORD getID(DWORD partition, DWORD subPartition) { return ((partition & 0xFF) << 8) + (subPartition & 0xFF); }
	LocalModData *Clone();
	Mesh *GetMesh() { return mesh; }
	AdjEdgeList *GetAdjEdgeList();
	AdjFaceList *GetAdjFaceList();
	void SetCache(Mesh &mesh);
	void FreeCache();
	void SynchBitArrays();

	// From IMeshSelectData:
	BitArray& GetFaceSel() { return GetFaceSel(activePartition, activeSubPartition); }

	void SetFaceSel(BitArray &set, BSSIModifier *imod, TimeValue t) {
		SetFaceSel(activePartition, activeSubPartition, set, imod, t);
	}

	BOOL EnablePartitionEdit(BOOL value) override {
		BOOL result = value;
		enablePartitionEdit = value;
		return result;
	}

	BOOL GetPartitionEditEnabled() const override {
		return enablePartitionEdit;
	}

	/*! \remarks Returns the number of partitions for the modifier. */
	DWORD GetNumPartitions() override {
		return partitions.size();
	}

	/*! \remarks Adds the specified partition and returns its index. */
	DWORD AddPartition() override {
		DWORD index = GetNumPartitions();
		SetActivePartition(index);
		SelectUnused();
		return index;
	}

	/*! \remarks Removes the specified partition from the modifier. */
	void RemovePartition(DWORD partition) override;

	// IBSSubIndexModifierData
   /*! \remarks Returns the current level of selection for the modifier. */
	DWORD GetActivePartition() override { return activePartition; }

	/*! \remarks Sets the currently selected partition level of the modifier. */
	void SetActivePartition(DWORD partition) override;

	/*! \remarks Returns the number of sub partitions for the modifier. */
	DWORD GetNumSubPartitions() override { return partitions[activePartition].materials.size(); }

	/*! \remarks Adds the specified sub partition and returns its index. */
	DWORD AddSubPartition() override;

	/*! \remarks Rempves the specified sub partition from the modifier. */
	void RemoveSubPartition(DWORD partition) override;

	/*! \remarks Returns the current level of selection for the modifier. */
	DWORD GetActiveSubPartition() override { return activeSubPartition; }

	/*! \remarks Sets the currently selected partition level of the modifier. */
	void SetActiveSubPartition(DWORD partition) override;

	DWORD GetActivePartitionSubCount() override;


	BitArray& GetFaceSel(int index, int subIndex) override {
		int id = getID(index, subIndex);
		BitArray* set = fselSet.GetSet(id);
		if (!set) {
			fselSet.InsertSet(BitArray(), id);
			set = fselSet.GetSet(id);
		}
		return *set;
	}

	void SetFaceSel(int index, int subIndex, BitArray &set, IBSSubIndexModifier *imod, TimeValue t) override;

	BSSubIndexData & GetPartition(int i) override { return partitions[i]; }

	GenericNamedSelSetList & GetFaceSelList() override { return fselSet; }

	void FixDuplicates();
	void SelectUnused();

	DWORD GetSelectionLevel() {
		return partitions[activePartition].selLevel;
	}

	TSTR GetSSF() const override {
		return ssfFile;
	}

	void SetSSF(const TSTR& ssf_file) override {
		ssfFile = ssf_file;
	}

};

class BSSISelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	BSSIModifier *mod;
	BSSIData *d;
	int level;

	BSSISelectRestore(BSSIModifier *m, BSSIData *d);
	BSSISelectRestore(BSSIModifier *m, BSSIData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() { d->held = FALSE; }
	TSTR Description() { return TSTR(TEXT("SelectRestore")); }
};

//--- ClassDescriptor and class vars ---------------------------------



IObjParam *BSSIModifier::ip = NULL;
BSSIModifier *BSSIModifier::editMod = NULL;
SelectModBoxCMode *BSSIModifier::selectMode = NULL;
BOOL BSSIModifier::updateCachePosted = FALSE;

class BSSIClassDesc :public ClassDesc2 {
public:
	BSSIClassDesc();
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BSSIModifier; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BSSIMODIFIER); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return BSSIMODIFIER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR * InternalName() { return TEXT("BSSIModifier"); }
	HINSTANCE HInstance() { return hInstance; }
};

class BSSIModifierMainDlgProc : public ParamMap2UserDlgProc {
public:
	BSSIModifier *mod;
	BSSIModifierMainDlgProc() { mod = NULL; }
	void SetEnables(HWND hWnd);
	void UpdateSelLevelDisplay(HWND hWnd);
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void UpdateSubIndexSelection(IBSSubIndexModifierData* mod);

	NpComboBox mCbMaterial;
	ICustEdit *p_ssf_edit;
	ISpinnerControl *mActivePart;
	ISpinnerControl *mEdMaterialID;
	ICustButton *mAddPart;
	ICustButton *mDelPart;
	ICustButton *mLockPart;

	ISpinnerControl *mActiveSubPart;
	ICustButton *mAddSubPart;
	ICustButton *mDelSubPart;
	ICustButton *mLockSubPart;
};

static BSSIModifierMainDlgProc theBSSIModifierMainDlgProc;

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

static ParamBlockDesc2 BSSI_desc(ms_pblock,
	_T("BSSubIndexModifierDescription"),
	IDS_MS_SOFTSEL, NULL,
	P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP,
	REF_PBLOCK,
	//rollout descriptions
	1,
	ms_map_main, IDD_BSSI_SELECT, IDS_MS_PARAMS, 0, 0, NULL,

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

static BSSIClassDesc BSSIDesc;
ClassDesc2* GetBSSIModifierDesc() { return &BSSIDesc; }
BSSIClassDesc::BSSIClassDesc() {
	BSSI_desc.SetClassDesc(this);
}

//--- BSSI mod methods -------------------------------

BSSIModifier::BSSIModifier() {
	SetAFlag(A_PLUGIN1);
	pblock = NULL;
	inLocalDataChanged = FALSE;
	BSSIDesc.MakeAutoParamBlocks(this);
	assert(pblock);
}

RefTargetHandle BSSIModifier::Clone(RemapDir& remap) {
	BSSIModifier *mod = new BSSIModifier();
	mod->ReplaceReference(0, remap.CloneRef(pblock));
	BaseClone(this, mod, remap);
	return mod;
}

Interval BSSIModifier::LocalValidity(TimeValue t)
{
	// aszabo|feb.05.02 If we are being edited, return NEVER 
	  // to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
}

Interval BSSIModifier::GetValidity(TimeValue t) {
	Interval ret = FOREVER;
	return ret;
}

void BSSIModifier::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf(triObjectClassID)) return;
	TriObject *tobj = (TriObject*)os->obj;
	BSSIData *d = (BSSIData*)mc.localData;
	if (!d) mc.localData = d = new BSSIData(tobj->GetMesh());

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

void BSSIModifier::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((BSSIData*)mc->localData)->FreeCache();
	if (!ip || (editMod != this) || updateCachePosted) return;

	if (!BSSIDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSSIDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	TimeValue t = ip->GetTime();
	PostMessage(hWnd, WM_UPDATE_CACHE, (WPARAM)t, 0);
	updateCachePosted = TRUE;
}

void BSSIModifier::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t, t), PART_GEOM | SELECT_CHANNEL | PART_SUBSEL_TYPE |
		PART_DISPLAY | PART_TOPO, REFMSG_MOD_EVAL);
	updateCachePosted = FALSE;
}

void BSSIModifier::UpdateSelLevelDisplay() {
	if (theBSSIModifierMainDlgProc.mod != this) return;
	if (!BSSIDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSSIDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theBSSIModifierMainDlgProc.UpdateSelLevelDisplay(hWnd);
}

static int butIDs[] = { 0, 0, 0, IDC_SELFACE, IDC_SELPOLY, IDC_SELELEMENT };
void BSSIModifierMainDlgProc::UpdateSelLevelDisplay(HWND hWnd) {
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

	int nActive = mod->GetActivePartition();
	int nParts = mod->GetNumPartitions();
	int nSubActive = mod->GetActiveSubPartition();
	int nSubParts = mod->GetSubPartitionCount();
	if (mActivePart) {
		mActivePart->SetLimits(0, nParts - 1, TRUE);
		mActivePart->SetValue(nActive, 0);
	}
	if (mActiveSubPart) {
		mActiveSubPart->SetLimits(0, nSubParts - 1, TRUE);
		mActiveSubPart->SetValue(nSubActive, 0);
	}
	tstring text = FormatString(TEXT("%d Total Partitions"), nParts);
	SetWindowText(GetDlgItem(hWnd, IDC_MS_PART_COUNT), text.c_str());

	text = FormatString(TEXT("%d Sub Partitions"), nSubParts);
	SetWindowText(GetDlgItem(hWnd, IDC_MS_SI_PART_COUNT), text.c_str());

	BSSubIndexData& partition = mod->GetCurrentPartition();
	if (nSubActive >= 0) {
		auto& mat = partition.materials[nSubActive];
		mCbMaterial.select(EnumToIndex(mat.materialHash, BodyPartFlags));
		//CheckDlgButton(hWnd, IDC_CBO_SI_VISIBLE, mat.visible ? BST_CHECKED : BST_UNCHECKED);

		if (mEdMaterialID) {
			mEdMaterialID->SetValue((int)mat.id, FALSE);
		}
	}
	int lockIdx = mod->GetPartitionEditEnabled() ? 0 : 1;
	mLockPart->SetImage(theBSSILockImageHandler.LoadImages(), lockIdx, 2 + lockIdx, lockIdx, 2 + lockIdx, 16, 16);


	auto* data = mod->GetFirstModifierData();
	if (data && p_ssf_edit) p_ssf_edit->SetText(data->GetSSF());

	CHECKHEAP();
	UpdateWindow(hWnd);
}

static bool oldShowEnd;

void BSSIModifier::BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev) {
	this->ip = ip;
	editMod = this;

	// Use our classdesc2 to put up our parammap2 maps:
	BSSIDesc.BeginEditParams(ip, this, flags, prev);
	theBSSIModifierMainDlgProc.mod = this;
	BSSIDesc.SetUserDlgProc(&BSSI_desc, ms_map_main, &theBSSIModifierMainDlgProc);

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
	CHECKHEAP();

}

void BSSIModifier::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	BSSIDesc.EndEditParams(ip, this, flags, next);
	theBSSIModifierMainDlgProc.mod = NULL;

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
	CHECKHEAP();

}

int BSSIModifier::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {

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

	if (!mc->localData || !((BSSIData*)mc->localData)->GetMesh()) return 0;

	DWORD selLevel = GetSelectionLevel();
	DWORD hitFlags;
	if (selByVert) {
		hitFlags = SUBHIT_VERTS;
		if (selLevel > SEL_FACE) hitFlags |= SUBHIT_USEFACESEL;
	}
	else {
		hitFlags = hitLevel[selLevel];
	}

	Mesh &mesh = *((BSSIData*)mc->localData)->GetMesh();

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
	CHECKHEAP();

	return res;
}

int BSSIModifier::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
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
	BSSIData *modData = (BSSIData *)(IBSSubIndexModifierData *)md->GetInterface(I_BSSUBINDEXMODIFIERDATA);
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
	CHECKHEAP();
	return 0;
}

void BSSIModifier::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip->GetShowEndResult() || !mc->localData) return;
	BSSIData *modData = (BSSIData *)(IBSSubIndexModifierData *)mc->localData->GetInterface(I_BSSUBINDEXMODIFIERDATA);
	if (!modData) return;
	DWORD selLevel = modData->GetSelectionLevel();
	if (!selLevel) return;

	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = inode->GetObjectTM(t);
	box = mesh->getBoundingBox(&tm);
}

void BSSIModifier::GetSubObjectCenters(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc) {
	if (!mc->localData) return;
	BSSIData *modData = (BSSIData *)(IBSSubIndexModifierData *)mc->localData->GetInterface(I_BSSUBINDEXMODIFIERDATA);
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

void BSSIModifier::GetSubObjectTMs(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc) {
	if (!mc->localData) return;
	BSSIData *modData = (BSSIData *)(IBSSubIndexModifierData *)mc->localData->GetInterface(I_BSSUBINDEXMODIFIERDATA);
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

void BSSIModifier::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	if (IBSSubIndexModifierData* p = GetFirstModifierData())
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
		CHECKHEAP();
	}
}

void BSSIModifier::LocalDataChanged() {
	if (inLocalDataChanged)
		return;
	inLocalDataChanged = TRUE;
	try
	{
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		if (ip && editMod == this) {
			SetNumSelLabel();

			// Setup subobjects
			if (IBSSubIndexModifierData* p = GetFirstModifierData())
			{
				bool changed = false;
				changed = (segments.Count() != p->GetNumPartitions());
				//segments.Resize(p->GetNumPartitions());
				for (int i = 0; i < p->GetNumPartitions(); ++i) {
					TSTR name = FormatText(TEXT("%d"), i);
					if (i >= segments.Count())
					{
						PartSubObjType *subobj = new PartSubObjType(); 
						subobj->SetName(name);
						segments.Append(1, &subobj);
						changed = true;
					}
					else
					{
						PartSubObjType *subobj = segments[i];
						if (!strmatch(name, subobj->GetName()))
						{
							subobj->SetName(name);
							changed = true;
						}
					}
				}
				while ( segments.Count() > p->GetNumPartitions() )
				{
					int i = segments.Count() - 1;
					PartSubObjType *subobj = segments[i];
					delete subobj;
					segments.Delete(i, 1);
				}

				SetEnableStates();
				UpdateSelLevelDisplay();
				SetNumSelLabel();

				NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
				ip->PipeSelLevelChanged();
				NotifyDependents(FOREVER, SELECT_CHANNEL | DISP_ATTRIB_CHANNEL | SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);

				if (changed)
					NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				CHECKHEAP();
			}
		}
	}
	catch(...)
	{}
	inLocalDataChanged = FALSE;
}

void BSSIModifier::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	BSSIData *d = NULL, *od = NULL;
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

	Tab<IBSSubIndexModifierData*> mcList = GetModifierData();
	for (nd = 0; nd < mcList.Count(); nd++) {
		d = (BSSIData *)mcList[nd];
		if (d == NULL) continue;

		if (!d->GetPartitionEditEnabled()) continue;


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

void BSSIModifier::ClearSelection(int selLevel) {
	BOOL changed = FALSE;
	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (d == NULL) continue;
		if (!d->GetPartitionEditEnabled()) continue;

		changed = TRUE;
		// Check if we have anything selected first:
		switch (d->GetSelectionLevel()) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			if (!d->GetFaceSel().NumberSet()) continue;
			else break;
		}

		if (theHold.Holding() && !d->held) theHold.Put(new BSSISelectRestore(this, d));
		d->SynchBitArrays();
		switch (d->GetSelectionLevel()) {
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->GetFaceSel().ClearAll();
			break;
		}
	}
	if (changed)
		LocalDataChanged();
}

void BSSIModifier::SelectAll(int selLevel) {
	ModContextList context_list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(context_list, nodes);
	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new BSSISelectRestore(this, d));
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

void BSSIModifier::InvertSelection(int selLevel) {
	ModContextList context_list;
	INodeTab nodes;
	if (!ip) return;
	ip->GetModContexts(context_list, nodes);
	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new BSSISelectRestore(this, d));
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

void BSSIModifier::SelectByMatID(int id) {
	if (!ip) return;
	BOOL add = GetKeyState(VK_CONTROL) < 0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU) < 0 ? TRUE : FALSE;
	theHold.Begin();
	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);
	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSSISelectRestore(this, d));
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

void BSSIModifier::SelectFrom(int from) {
	if (!ip) return;
	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);
	BSSIData *d;
	int ignoreVisEdge;
	float planarThresh;
	pblock->GetValue(ms_ignore_visible, TimeValue(0), ignoreVisEdge, FOREVER);
	pblock->GetValue(ms_planar_threshold, TimeValue(0), planarThresh, FOREVER);

	theHold.Begin();
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;

		if (!d->held) theHold.Put(new BSSISelectRestore(this, d));
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
#define CURSEL_CHUNK			0x2848
#define PARTFLAGS_CHUNK			0x2849

#define SSF_FILE_CHUNK          0x2850

static int namedSelID[] = { NAMEDVSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK };

IOResult BSSIModifier::Save(ISave *isave) {
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

IOResult BSSIModifier::Load(ILoad *iload) {
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

IOResult BSSIModifier::SaveLocalData(ISave *isave, LocalModData *ld) {
	BSSIData *d = (BSSIData*)ld;

	if (d->fselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->fselSet.Save(isave);
		isave->EndChunk();
	}

	ULONG nb;
	IOResult res;
	isave->BeginChunk(CURSEL_CHUNK);
	res = isave->Write(&d->enablePartitionEdit, sizeof(BOOL), &nb);
	res = isave->Write(&d->activePartition, sizeof(int), &nb);
	res = isave->Write(&d->activeSubPartition, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(PARTFLAGS_CHUNK);
	int n = d->partitions.size();
	res = isave->Write(&n, sizeof(int), &nb);
	for (int i = 0; i < n; ++i) {
		BSSubIndexData& sub_index_data = d->partitions[i];
		res = isave->Write(&sub_index_data.id, sizeof(int), &nb);
		res = isave->Write(&sub_index_data.selLevel, sizeof(int), &nb);
		int matSize = sub_index_data.materials.size();
		res = isave->Write(&matSize, sizeof(int), &nb);
		for (int j = 0; j < matSize; ++j)
		{
			BSSubIndexMaterial& material = sub_index_data.materials[j];
			res = isave->Write(&material.id, sizeof(int), &nb);
			res = isave->Write(&material.materialHash, sizeof(int), &nb);
			int sizeData = material.data.size();
			res = isave->Write(&sizeData, sizeof(int), &nb);
			for (int k = 0; k < sizeData; ++k)
			{
				res = isave->Write(&material.data[k], sizeof(float), &nb);
			}
		}
	}
	isave->EndChunk();

	isave->BeginChunk(SSF_FILE_CHUNK);
	isave->WriteWString(d->ssfFile);
	isave->EndChunk();

	return IO_OK;
}

IOResult BSSIModifier::LoadLocalData(ILoad *iload, LocalModData **pld) {
	USES_CONVERSION;
	BSSIData *d = new BSSIData;
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
			iload->Read(&d->enablePartitionEdit, sizeof(d->enablePartitionEdit), &nb);
			iload->Read(&d->activePartition, sizeof(d->activePartition), &nb);
			iload->Read(&d->activeSubPartition, sizeof(d->activeSubPartition), &nb);
			break;

		case PARTFLAGS_CHUNK:
		{
			int n = 0;
			iload->Read(&n, sizeof(n), &nb);
			d->partitions.resize(n);
			for (int i = 0; i < n; ++i) {
				BSSubIndexData& sub_index_data = d->partitions[i];
				res = iload->Read(&sub_index_data.id, sizeof(int), &nb);
				res = iload->Read(&sub_index_data.selLevel, sizeof(int), &nb);
				int matSize = 0;
				res = iload->Read(&matSize, sizeof(int), &nb);
				for (int j = 0; j < matSize; ++j)
				{
					BSSubIndexMaterial material;
					res = iload->Read(&material.id, sizeof(int), &nb);
					res = iload->Read(&material.materialHash, sizeof(int), &nb);
					int sizeData = 0;
					res = iload->Read(&sizeData, sizeof(int), &nb);
					material.data.resize(sizeData);
					for (int k = 0; k < sizeData; ++k)
					{
						res = iload->Read(&material.data[k], sizeof(float), &nb);
					}
					sub_index_data.materials.push_back(material);
				}

			}
		} break;

		case SSF_FILE_CHUNK:
		{
			LPWSTR ptr = nullptr;
			res = iload->ReadWStringChunk(&ptr);
			if (res == IO_OK)
				d->ssfFile = W2T(ptr);
		}break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}
	LocalDataChanged();
	return IO_OK;
}


// Window Procs ------------------------------------------------------

void BSSIModifier::SetEnableStates() {
	if (!BSSIDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSSIDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theBSSIModifierMainDlgProc.SetEnables(hWnd);
}

void BSSIModifierMainDlgProc::UpdateSubIndexSelection(IBSSubIndexModifierData* mod)
{
	if (mActiveSubPart != nullptr) {
		mActiveSubPart->SetLimits(0, mod->GetActivePartitionSubCount() - 1, TRUE);
		mActiveSubPart->SetValue((int)mod->GetActiveSubPartition(), 0);
	}
}

void BSSIModifierMainDlgProc::SetEnables(HWND hParams) {
	if (!mod) return;
	int selLevel = mod->GetSelectionLevel();
	ICustButton *but;
	ISpinnerControl *spin;

	int nSub = mod->ip->GetSubObjectLevel();
	bool subSel = (nSub != 0);

	bool poly = (selLevel == SEL_POLY) ? true : false;
	bool face = (selLevel == SEL_FACE) || (selLevel == SEL_ELEMENT) || poly ? TRUE : FALSE;

	BOOL editEnabled = mod->GetPartitionEditEnabled();
	
	int numParts = mod->GetNumPartitions();
	int numSubParts = mod->GetSubPartitionCount();
	if (mActivePart) mActivePart->Enable(numParts >= 0);
	if (mAddPart) mAddPart->Enable(editEnabled && numParts >= 0);
	if (mDelPart) mDelPart->Enable(editEnabled && numParts > 1);

	if (mActiveSubPart) mActiveSubPart->Enable(numSubParts >= 0);
	if (mAddSubPart) mAddSubPart->Enable(editEnabled && numSubParts >= 0);
	if (mDelSubPart) mDelSubPart->Enable(editEnabled && numSubParts > 1);


	EnableWindow(mCbMaterial.mWnd, editEnabled);
	//EnableWindow(GetDlgItem(hParams, IDC_CBO_SI_VISIBLE), editEnabled);
	if (p_ssf_edit) p_ssf_edit->Enable(editEnabled);
	EnableWindow(GetDlgItem(hParams, IDC_BTN_SSF_LOAD), editEnabled);

	EnableWindow(GetDlgItem(hParams, IDC_MS_SEL_BYVERT), editEnabled && subSel && face);
	//EnableWindow (GetDlgItem (hParams, IDC_MS_IGNORE_BACKFACES), subSel&&(edge||face));
	EnableWindow(GetDlgItem(hParams, IDC_MS_IGNORE_VISEDGE), editEnabled && subSel && poly);
	EnableWindow(GetDlgItem(hParams, IDC_MS_PLANAR_TEXT), editEnabled && subSel && poly);
	spin = GetISpinner(GetDlgItem(hParams, IDC_MS_PLANARSPINNER));
	spin->Enable(editEnabled && subSel && poly);
	ReleaseISpinner(spin);

	EnableWindow(GetDlgItem(hParams, IDC_MS_IGNORE_BACKFACES), editEnabled && subSel);

	EnableWindow(GetDlgItem(hParams, IDC_MS_SELBYMAT_BOX), editEnabled && subSel && face);
	EnableWindow(GetDlgItem(hParams, IDC_MS_SELBYMAT_TEXT), editEnabled && subSel && face);
	but = GetICustButton(GetDlgItem(hParams, IDC_MS_SELBYMAT));
	but->Enable(editEnabled && subSel && face);
	ReleaseICustButton(but);

	spin = GetISpinner(GetDlgItem(hParams, IDC_MS_MATIDSPIN));
	spin->Enable(editEnabled && subSel && face);
	ReleaseISpinner(spin);

	but = GetICustButton(GetDlgItem(hParams, IDC_MS_SELUNUSED));
	but->Enable(editEnabled && subSel);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hParams, IDC_MS_FIXDUPL));
	but->Enable(editEnabled && subSel);
	ReleaseICustButton(but);

}

INT_PTR BSSIModifierMainDlgProc::DlgProc(TimeValue t, IParamMap2 *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	if (!mod) return FALSE;
	ICustToolbar *iToolbar;
	ICustButton *but;
	int matid;
	int nParts;
	int nActive;
	int nSubActive;
	int nSubParts;
	int lockIdx;

	switch (msg) {
	case WM_INITDIALOG:
	{
		mCbMaterial.init(GetDlgItem(hWnd, IDC_CB_MATERIAL));
		p_ssf_edit = GetICustEdit(GetDlgItem(hWnd, IDC_ED_SSF_FILE));

		if (BodyPartFlagsCache == nullptr)
		{
			TCHAR iniFile[MAX_PATH];
			GetIniFileName(iniFile);
			PathRemoveFileSpec(iniFile);
			PathAppend(iniFile, TEXT("MaxNifStrings.ini"));
			NameValueList settings;
			if ( ReadIniSectionAsList(TEXT("BSSubIndexMaterials"), iniFile, BSSubIndexMaterials) ){
				if (BSSubIndexMaterials.size() > 0) {
					int i = 0;
					BodyPartFlagsCache = static_cast<EnumLookupType*>(calloc(BSSubIndexMaterials.size()+1, sizeof(EnumLookupType)));
					for (NameValueList::const_iterator itr = BSSubIndexMaterials.begin(), end = BSSubIndexMaterials.end(); itr != end; ++itr, ++i) {
						BodyPartFlagsCache[i].value = (int)_tcstoul(itr->first.c_str(), nullptr, 0);
						BodyPartFlagsCache[i].name = itr->second.c_str();
					}
					BodyPartFlags = BodyPartFlagsCache;
				}				
			}
		}

		for (const EnumLookupType* bpFlag = BodyPartFlags; bpFlag->name != NULL; ++bpFlag) {
			mCbMaterial.add(bpFlag->name);
		}

		nParts = mod->GetNumPartitions();
		nActive = mod->GetActivePartition();
		if (nActive < 0) {
			if (IBSSubIndexModifierData* p = mod->GetFirstModifierData())
				p->SetActivePartition(0);
			nParts = mod->GetNumPartitions();
			nActive = mod->GetActivePartition();
		}

		
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
		mAddPart->SetImage(theBSSIPartImageHandler.LoadImages(), 0, 2, 0, 2, 16, 16);

		mDelPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_DELPART));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mDelPart->SetTooltip(TRUE, "Delete Active Partition");
#endif
		mDelPart->SetImage(theBSSIPartImageHandler.LoadImages(), 1, 3, 1, 3, 16, 16);

		mLockPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_LOCKEDIT));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mLockPart->SetTooltip(TRUE, "Lock Selection Edits");
#endif
		lockIdx = mod->GetPartitionEditEnabled() ? 0 : 1;
		mLockPart->SetImage(theBSSILockImageHandler.LoadImages(), lockIdx, 2 + lockIdx, lockIdx, 2 + lockIdx, 16, 16);


		/////////////////////////
		nSubActive = mod->GetActiveSubPartition();
		nSubParts = mod->GetSubPartitionCount();

		EnableWindow(GetDlgItem(hWnd, IDC_MS_SIPARTITION), TRUE);
		mActiveSubPart = GetISpinner(GetDlgItem(hWnd, IDC_MS_SIPARTSPIN));
		mActiveSubPart->LinkToEdit(GetDlgItem(hWnd, IDC_MS_SIPARTITION), EDITTYPE_POS_INT);
		mActiveSubPart->SetValue(nSubActive, 0);
		mActiveSubPart->SetLimits(0, nSubParts - 1, TRUE);
		mActiveSubPart->SetAutoScale(FALSE);
		mActiveSubPart->SetScale(1);
		mActiveSubPart->SetResetValue(0);

		EnableWindow(GetDlgItem(hWnd, IDC_MS_SIPARTITION_ID), TRUE);
		mEdMaterialID = GetISpinner(GetDlgItem(hWnd, IDC_MS_SIPART_ID_SPIN));
		mEdMaterialID->LinkToEdit(GetDlgItem(hWnd, IDC_MS_SIPARTITION_ID), EDITTYPE_POS_INT);
		mEdMaterialID->SetValue(nActive, 0);
		mEdMaterialID->SetLimits(0, 200, TRUE);
		mEdMaterialID->SetAutoScale(FALSE);
		mEdMaterialID->SetScale(1);
		mEdMaterialID->SetResetValue(0);
        
		mAddSubPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_SIADDPART));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mAddSubPart->SetTooltip(TRUE, "Create New SubPartition");
#endif
		mAddSubPart->SetImage(theBSSIPartImageHandler.LoadImages(), 0, 2, 0, 2, 16, 16);

		mDelSubPart = GetICustButton(GetDlgItem(hWnd, IDC_MS_SI_DELPART));
#if VERSION_3DSMAX < ((10000<<16)+(24<<8)+0) // Version 7
		mDelPart->SetTooltip(TRUE, "Delete Active SubPartition");
#endif
		mDelSubPart->SetImage(theBSSIPartImageHandler.LoadImages(), 1, 3, 1, 3, 16, 16);

		BSSubIndexData& si_data = mod->GetCurrentPartition();
		if (nSubActive >= 0) {
			BSSubIndexMaterial& si_mat = si_data.materials[nSubActive];
			mCbMaterial.select(EnumToIndex(si_mat.materialHash, BodyPartFlags));
			//CheckDlgButton(hWnd, IDC_CBO_SI_VISIBLE, si_mat.visible ? BST_CHECKED : BST_UNCHECKED);
			mEdMaterialID->Enable();
		} else {
			mCbMaterial.select(0);
			//CheckDlgButton(hWnd, IDC_CBO_SI_VISIBLE, BST_UNCHECKED);
			mEdMaterialID->Disable();
		}

		auto* data = mod->GetFirstModifierData();
		if (data && p_ssf_edit) p_ssf_edit->SetText(data->GetSSF());

		iToolbar = GetICustToolbar(GetDlgItem(hWnd, IDC_MS_SELTYPE));
		iToolbar->SetImage(theBSSIImageHandler.LoadImages());
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
	} break;

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
		if (mActiveSubPart) {
			ReleaseISpinner(mActiveSubPart);
			mActiveSubPart = NULL;
		}
		if (mAddSubPart) {
			ReleaseICustButton(mAddSubPart);
			mAddSubPart = NULL;
		}
		if (mDelSubPart) {
			ReleaseICustButton(mDelSubPart);
			mDelSubPart = NULL;
		}
		if (p_ssf_edit) {
			ReleaseICustEdit(p_ssf_edit);
			p_ssf_edit = nullptr;
		}
		break;

	case WM_UPDATE_CACHE:
		mod->UpdateCache((TimeValue)wParam);
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_MS_PARTSPIN:
			if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActivePart->GetIVal();
				p->SetActivePartition(idx);
				mod->ip->SetSubObjectLevel(idx + 1);
				mod->LocalDataChanged();
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
			}
			UpdateWindow(hWnd);
			break;
		case IDC_MS_SIPARTSPIN:
			if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActiveSubPart->GetIVal();
				p->SetActiveSubPartition(idx);
				mod->ip->SetSubObjectLevel(p->GetActivePartition()+1);
				mod->LocalDataChanged();
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
			}
			UpdateWindow(hWnd);
			break;
		}
		CHECKHEAP();
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_MS_PARTITION:
			if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActivePart->GetIVal();
				p->SetActivePartition(idx);
				mod->ip->SetSubObjectLevel(idx + 1);
			}
			break;
		case IDC_MS_SIPARTITION:
			if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
				int idx = mActiveSubPart->GetIVal();
				p->SetActiveSubPartition(idx);
				mod->ip->SetSubObjectLevel(p->GetActivePartition() + 1);
			}
			break;

		case IDC_ED_SSF_FILE:
			if (p_ssf_edit) {
				auto* data = mod->GetFirstModifierData();
				if (data && p_ssf_edit) {
					TCHAR buffer[120];
					p_ssf_edit->GetText(buffer, _countof(buffer));
					data->SetSSF(buffer);
				}
			} break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_CB_MATERIAL:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int nActive = p->GetActivePartition();
					int nSubActive = p->GetActiveSubPartition();
					int sel = mCbMaterial.selection();
					mod->GetCurrentPartition().materials[nSubActive].materialHash = BodyPartFlags[sel].value;
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;
		case IDC_CBO_SI_VISIBLE:
			{ 
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int nActive = p->GetActivePartition();
					int nSubActive = p->GetActiveSubPartition();
					int sel = mCbMaterial.selection();
					mod->GetCurrentPartition().materials[nSubActive].visible = IsDlgButtonChecked(hWnd, IDC_CBO_SI_VISIBLE) == BST_CHECKED;
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			} break;

		case IDC_MS_SIPARTITION_ID:
			if (mEdMaterialID)
			{
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int nActive = p->GetActivePartition();
					int nSubActive = p->GetActiveSubPartition();
					int sel = mCbMaterial.selection();
					auto& mat = mod->GetCurrentPartition().materials[nSubActive];
					mat.id = mEdMaterialID->GetIVal();
				}				
			} break;

		case IDC_MS_ADDPART:
			if (mActivePart) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int idx = p->AddPartition();
					nParts = mod->GetNumPartitions();
					mod->ip->SetSubObjectLevel(idx + 1);
					mActivePart->SetLimits(0, nParts - 1, TRUE);
					mActivePart->SetValue(idx, TRUE);
					UpdateSubIndexSelection(p);
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_DELPART:
			if (mActivePart) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int idx = mActivePart->GetIVal();
					p->RemovePartition(idx);
					mod->ip->SetSubObjectLevel(p->GetActivePartition() + 1);
					nParts = mod->GetNumPartitions();
					mActivePart->SetLimits(0, nParts, TRUE);
					mActivePart->SetValue((int)p->GetActivePartition(), TRUE);
					UpdateSubIndexSelection(p);
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_LOCKEDIT:
			if (mLockPart) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					p->EnablePartitionEdit(!p->GetPartitionEditEnabled());
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_SIADDPART:
			if (mActiveSubPart) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int idx = p->AddSubPartition();
					nParts = mod->GetSubPartitionCount();
					mod->ip->SetSubObjectLevel(p->GetActivePartition() + 1);
					mActiveSubPart->SetLimits(0, nParts - 1, TRUE);
					mActiveSubPart->SetValue(idx, TRUE);
					mod->LocalDataChanged();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
				}
			}
			break;

		case IDC_MS_SI_DELPART:
			if (mActiveSubPart) {
				if (IBSSubIndexModifierData* p = mod->GetFirstModifierData()) {
					int idx = mActiveSubPart->GetIVal();
					p->RemovePartition(idx);
					mod->ip->SetSubObjectLevel(p->GetActivePartition() + 1);
					nParts = mod->GetSubPartitionCount();
					mActiveSubPart->SetLimits(0, nParts, TRUE);
					mActiveSubPart->SetValue((int)p->GetActiveSubPartition(), TRUE);
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

		case IDC_BTN_SSF_LOAD:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR tmp[MAX_PATH];
				Interface *gi = GetCOREInterface();

				TCHAR filter[64], *pfilter = filter;
				pfilter = _tcscpy(pfilter, TEXT("SSF Files (*.SSF)"));
				pfilter += _tcslen(pfilter), *pfilter++ = '\0';
				_tcscpy(pfilter, TEXT("*.SSF"));
				pfilter += _tcslen(pfilter), *pfilter++ = '\0';
				*pfilter++ = '\0';

				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = gi->GetMAXHWnd();
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = tmp;
				ofn.nMaxFile = _countof(tmp);
				ofn.lpstrTitle = TEXT("Browse for SSF File...");
				ofn.lpstrDefExt = TEXT("SSF");
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					for (LPCTSTR filepart = tmp; filepart != nullptr; filepart = PathFindNextComponent(filepart)) {
						if (wildmatch(TEXT("materials\\*"), filepart)) {
							auto* data = mod->GetFirstModifierData();
							if (data && p_ssf_edit) {
								p_ssf_edit->SetText(const_cast<LPTSTR>(filepart));
								data->SetSSF(filepart);
							}
							break;
						}
					}
				}
			}
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
		case IDC_MS_SIPARTITION_ID:
			lpttt->lpszText = GetString(IDS_SIPARTITION_ID);
			break;
		case IDC_MS_LOCKEDIT:
			lpttt->lpszText = GetString(IDS_MS_LOCKEDIT);
			break;
		}
		break;

	default: return FALSE;
	}
	return TRUE;
}

// BSSIData -----------------------------------------------------

LocalModData *BSSIData::Clone() {
	BSSIData *d = new BSSIData;
	d->activePartition = activePartition;
	d->activeSubPartition = activeSubPartition;
	d->partitions = partitions;
	d->fselSet = fselSet;
	d->enablePartitionEdit = enablePartitionEdit;
	return d;
}

BSSIData::BSSIData(Mesh &mesh) {
	held = 0; this->mesh = NULL; temp = NULL;
	activeSubPartition = 0;
	enablePartitionEdit = FALSE;
	SetActivePartition(0);
	GetFaceSel() = mesh.faceSel;
}

BSSIData::BSSIData()
{
	held = 0; mesh = NULL; temp = NULL;
	activeSubPartition = 0;
	enablePartitionEdit = FALSE;
	SetActivePartition(0);
}

void BSSIData::SynchBitArrays() {
	if (!mesh) return;
	if (GetFaceSel().GetSize() != mesh->getNumFaces())
		GetFaceSel().SetSize(mesh->getNumFaces(), TRUE);
}

AdjEdgeList *BSSIData::GetAdjEdgeList() {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh(mesh);
	return temp->AdjEList();
}

AdjFaceList *BSSIData::GetAdjFaceList() {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh(mesh);
	return temp->AdjFList();
}

void BSSIData::SetCache(Mesh &mesh) {
	if (this->mesh) delete this->mesh;
	this->mesh = new Mesh(mesh);
	if (temp) temp->Invalidate(PART_TOPO);
	SynchBitArrays();
}

void BSSIData::FreeCache() {
	if (mesh) delete mesh;
	mesh = NULL;
	if (temp) delete temp;
	temp = NULL;
}

void BSSIData::SetFaceSel(int index, int subIndex, BitArray &set, IBSSubIndexModifier *imod, TimeValue t) {
	BSSIModifier *mod = (BSSIModifier *)imod;
	if (theHold.Holding()) theHold.Put(new BSSISelectRestore(mod, this, SEL_FACE));
	GetFaceSel(index, subIndex) = set;
	if (mesh) mesh->faceSel = set;
}

void BSSIData::SetActivePartition(DWORD partition)
{
	if (activePartition != partition)
		activeSubPartition = 0;
	activePartition = partition;

	// ensure partitions are created for each partition
	while (partitions.size() <= partition) {
		BSSubIndexData si_data;
		si_data.selLevel = SEL_FACE;
		si_data.id = partitions.size();
		partitions.push_back(si_data);
	}
	// ensure bit arrays are constructed for partitions
	BitArray empty;
	for (int i = 0; i < partition; ++i) {
		BSSubIndexData &si_data = partitions[i];
		// ensure at least one sub partition per partition
		if (si_data.materials.size() == 0) {
			si_data.materials.resize(1);
		}
		DWORD id = getID(partition, 0);
		BitArray *ba = fselSet.GetSet(id);
		if (ba == nullptr) fselSet.AppendSet(empty, id);
	}
}

void BSSIData::RemovePartition(DWORD partition)
{
	int nPartitions = partitions.size();
	if (partition < partitions.size() && partitions.size() > 1)
	{
		Tab<DWORD> &ids = fselSet.ids;
		// purge all sub sets (in reverse order
		for (int i = 0, n = ids.Count(); i < n; ++i)
		{
			DWORD id = *ids.Addr(n-i-1);
			DWORD partID = (id & 0xFF00) >> 8;
			//DWORD subPartID = (id & 0xFF);
			if (partID == partition)
				fselSet.RemoveSet(id);
		}
		//BSSubIndexData &flag = partitions[partition];
		//for (int i = 0; i < flag.subIndexCount; ++i) {
		//	DWORD id = getID(partition, i);
		//	fselSet.RemoveSet(id);
		//}
		auto& itr = partitions.begin();
		advance(itr, partition);
		partitions.erase(itr);

		// rename subsequent ids
		for (int i = 0, n = ids.Count(); i < n; ++i)
		{
			DWORD* id = ids.Addr(i);
			DWORD partID = (*id & 0xFF00) >> 8;
			DWORD subPartID = (*id & 0xFF);
			if (partID > partition)
				*id = getID(partID-1, subPartID);
		}
		if (partition >= partitions.size())
			SetActivePartition(partitions.size()-1);
	}
}

/*! \remarks Adds the specified sub partition and returns its index. */
DWORD BSSIData::AddSubPartition() {
	BSSubIndexData& data = partitions[activePartition];
	BSSubIndexMaterial si_mat;
	si_mat.id = data.materials.size();
	si_mat.materialHash = 0xFFFFFFFF;
	si_mat.visible = false;
	data.materials.push_back(si_mat);
	
	BitArray empty;
	DWORD id = getID(activePartition, activeSubPartition);
	BitArray *ba = fselSet.GetSet(id);
	if (ba == nullptr) fselSet.AppendSet(empty, id);
	return activeSubPartition;
}

/*! \remarks Rempves the specified sub partition from the modifier. */
void BSSIData::RemoveSubPartition(DWORD subPartition)
{
	BSSubIndexData& data = partitions[activePartition];
	DWORD subIndexCount = data.materials.size();
	if (subIndexCount > 1 && subPartition < subIndexCount)
	{
		DWORD id = getID(activePartition, subPartition);
		fselSet.RemoveSet(id);

		auto itr = data.materials.begin();
		std::advance(itr, subPartition);
		data.materials.erase(itr);

		// rename subsequent ids
		Tab<DWORD> &ids = fselSet.ids;
		for (int i = 0, n = ids.Count(); i < n; ++i)
		{
			DWORD* id = ids.Addr(i);
			DWORD partID = (*id & 0xFF00) >> 8;
			DWORD subPartID = (*id & 0xFF);
			if (partID == activePartition && subPartID >= subPartition)
				*id = getID(partID, subPartID-1);
		}

		if (activeSubPartition >= subIndexCount)
			SetActiveSubPartition(subIndexCount - 1);
	}
}

/*! \remarks Sets the currently selected partition level of the modifier. */
void BSSIData::SetActiveSubPartition(DWORD subPartition)
{
	if (subPartition == 0xFFFFFFFF)
		subPartition = 0;

	activeSubPartition = subPartition;
	SetActivePartition(activePartition); // should be nop

	// ensure bit arrays are constructed for partitions
	BitArray empty;
	BSSubIndexData &si_data = partitions[activePartition];
	if (subPartition >= si_data.materials.size()) {
		for (DWORD i = si_data.materials.size(); i <= subPartition; ++i)
		{
			DWORD id = getID(activePartition, i);
			BitArray *ba = fselSet.GetSet(id);
			if (ba == nullptr) fselSet.AppendSet(empty, id);

			BSSubIndexMaterial si_mat;
			si_mat.id = i;
			si_mat.materialHash = 0xFFFFFFFF;
			si_mat.visible = false;
			si_data.materials.push_back(si_mat);
		}
	}
}

DWORD BSSIData::GetActivePartitionSubCount()
{
	if (activePartition >= 0 && activePartition <partitions.size()) {
		return partitions[activePartition].materials.size();
	}
	return 0;
}

void BSSIData::SelectUnused()
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
void BSSIData::FixDuplicates()
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
// SelectRestore --------------------------------------------------

BSSISelectRestore::BSSISelectRestore(BSSIModifier *m, BSSIData *data) {
	mod = m;
	level = data->GetSelectionLevel();
	d = data;
	d->held = TRUE;
	usel = d->GetFaceSel();
}

BSSISelectRestore::BSSISelectRestore(BSSIModifier *m, BSSIData *data, int sLevel) {
	mod = m;
	level = sLevel;
	d = data;
	d->held = TRUE;
	usel = d->GetFaceSel();
}

void BSSISelectRestore::Restore(int isUndo) {
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

void BSSISelectRestore::Redo() {
	switch (level) {
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		d->GetFaceSel() = rsel; break;
	}
	mod->LocalDataChanged();
}


//--- Named selection sets -----------------------------------------


void BSSIModifier::ActivateSubSelSet(int index) {
	if (index < 0 || !ip) return;

	theHold.Begin();
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);

	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		BSSIData *meshData = (BSSIData *)list[i];
		if (!meshData) continue;
		if (theHold.Holding() && !meshData->held) theHold.Put(new BSSISelectRestore(this, meshData));
		meshData->SetActivePartition(index);
	}

	nodes.DisposeTemporary();
	LocalDataChanged();
	theHold.Accept(GetString(IDS_DS_SELECT));
	ip->RedrawViews(ip->GetTime());
}


void BSSIModifier::SetNumSelLabel() {
	TSTR buf;
	int num = 0;
	int which = 0;

	if (!BSSIDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSSIDesc.GetParamMap(ms_map_main);
	if (!pmap) return;
	HWND hParams = pmap->GetHWnd();
	if (!hParams) return;

	ModContextList context_list;
	INodeTab nodes;
	ip->GetModContexts(context_list, nodes);

	DWORD selLevel = SEL_FACE;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		BSSIData *meshData = (BSSIData *)list[i];
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


	//TODO: count partitions
	//buf.printf(GetString(IDS_RB_NUMFACESELP), num);
	//SetDlgItemText(hParams, IDC_MS_PART_COUNT, buf);
	
	nodes.DisposeTemporary();
}

#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
RefResult BSSIModifier::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
#else
RefResult BSSIModifier::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
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

int BSSIModifier::NumSubObjTypes()
{
	return segments.Count();
}

ISubObjType *BSSIModifier::GetSubObjType(int i)
{
	if (i >= segments.Count())
		return NULL;
	if (i < 0) {
		if (GetActivePartition() > 0)
			return GetSubObjType(GetActivePartition() - 1);
		return NULL;
	}
	return segments[i];
}

void BSSIModifier::InvalidateDialogElement(int elem) {
	if (!pblock) return;
	if (!BSSIDesc.NumParamMaps()) return;
	IParamMap2 *pmap = BSSIDesc.GetParamMap(ms_map_main);
	if (pmap) pmap->Invalidate(elem);
}

void BSSIModifier::FixDuplicates()
{
	theHold.Begin();

	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSSISelectRestore(this, d));
		d->FixDuplicates();
	}
	theHold.Accept(GetString(IDS_RB_FIXDUPL));

	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

void BSSIModifier::SelectUnused()
{
	theHold.Begin();

	BSSIData *d;
	Tab<IBSSubIndexModifierData*> list = GetModifierData();
	for (int i = 0; i < list.Count(); i++) {
		d = (BSSIData *)list[i];
		if (!d) continue;
		if (!d->held) theHold.Put(new BSSISelectRestore(this, d));
		d->SelectUnused();
	}
	theHold.Accept(GetString(IDS_RB_SELUNUSED));

	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

// Get or Create the Skin Modifier
Modifier *GetOrCreateBSSubIndexModifier(INode *node)
{
	Modifier *skinMod = GetBSSubIndexModifier(node);
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
	skinMod = (Modifier*)CreateInstance(OSM_CLASS_ID, BSSIMODIFIER_CLASS_ID);
	dobj->SetAFlag(A_LOCK_TARGET);
	dobj->AddModifier(skinMod);
	dobj->ClearAFlag(A_LOCK_TARGET);
	node->SetObjectRef(dobj);
	return skinMod;
}

Modifier *GetBSSubIndexModifier(INode *node)
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
			if (mod->ClassID() == BSSIMODIFIER_CLASS_ID)
				return mod;
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	return NULL;
}
