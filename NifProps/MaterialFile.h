#pragma once

#include "../NifProps/NifProps.h"
#include "../MtlUtils/mtldefine.h"

const Class_ID MTLFILE_CLASS_ID(0x8efac6e7, 0x86cf438a);
const Class_ID BGSMFILE_CLASS_ID(0x4b9416f7, 0x771e4e55);
const Class_ID BGEMFILE_CLASS_ID(0xb79b2136, 0x07df5403);
const SClass_ID MATERIALFILE_CLASS_ID = 0xC001;

enum MaterialFileType {
	MFT_None,
	MFT_BGSM,
	MFT_BGEM,
};

class MaterialReference : public ReferenceTarget
{
public:
	MaterialFileType file_type;
	TSTR materialName;
	TSTR materialFileName;

	MaterialReference() {
		file_type = MFT_None;
	}

	Class_ID ClassID() override { return MTLFILE_CLASS_ID; }
	SClass_ID SuperClassID() override { return MATERIALFILE_CLASS_ID; }
	static TSTR GetName() { return GetString(IDS_SH_NAME); }
	void GetClassName(TSTR& s) override { s = GetName(); }

	static IOResult LoadMaterialChunk(BaseMaterial &mtl, ILoad* iload);
	static IOResult SaveMaterialChunk(BaseMaterial &mtl, ISave* isave);

	void SetFileName(const TSTR& name, const TSTR& path) {
		SetName(name);
		SetFileName(path);
	}

	virtual BaseMaterial *GetBaseMaterial() = 0;

	void SetName(const TSTR& name) {
		materialName = name;
	}
	void SetFileName(const TSTR& path) {
		materialFileName = path;
	}

#if VERSION_3DSMAX < (17000<<16) // Version 17 (2015)
	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) override { return(REF_SUCCEED); }
#else
	RefResult	NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override { return(REF_SUCCEED); }
#endif
protected:
	~MaterialReference() {}
};

class BGSMFileReference : public MaterialReference
{
public:
	BGSMFile file;

	BGSMFileReference() {
		file_type = MFT_BGSM;
		InitialzeBGSM(file);
	}
	Class_ID ClassID() override { return BGSMFILE_CLASS_ID; }
	void DeleteThis() override { delete this; }

	IOResult Load(ILoad* iload) override;
	IOResult Save(ISave* isave) override;

	IOResult LoadMaterialChunk(ILoad* iload);
	IOResult SaveMaterialChunk(ISave* isave);

	BaseMaterial *GetBaseMaterial() override { return &file;  }


	ReferenceTarget* Clone(RemapDir& remap) override
	{
		ReferenceTarget* result = new BGSMFileReference();
		BaseClone(this, result, remap);
		return result;
	}
};


class BGEMFileReference : public MaterialReference
{
public:
	BGEMFile file;

	BGEMFileReference() {
		file_type = MFT_BGEM;
		InitialzeBGEM(file);
	}
	Class_ID ClassID() override { return BGEMFILE_CLASS_ID; }
	void DeleteThis() override { delete this; }

	IOResult Load(ILoad* iload) override;
	IOResult Save(ISave* isave) override;

	IOResult LoadMaterialChunk(ILoad* iload);
	IOResult SaveMaterialChunk(ISave* isave);

	BaseMaterial *GetBaseMaterial() override { return &file; }

	ReferenceTarget* Clone(RemapDir& remap) override
	{
		ReferenceTarget* result = new BGEMFileReference();
		BaseClone(this, result, remap);
		return result;
	}

};