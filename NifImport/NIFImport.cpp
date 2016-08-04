/**********************************************************************
*<
FILE: ImporterCore.cpp

DESCRIPTION:	Core Import helper routines

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "stdafx.h"
#include "MaxNifImport.h"

using namespace Niflib;

// Define the standard section names used in the ini file
LPCTSTR NifImportSection = TEXT("MaxNifImport");
LPCTSTR SystemSection = TEXT("System");
LPCTSTR BipedImportSection = TEXT("BipedImport");
LPCTSTR AnimImportSection = TEXT("AnimationImport");
LPCTSTR CollisionSection = TEXT("Collision");

class IBipMaster;
IBipMaster * (_cdecl * Max8CreateNewBiped)(float, float, class Point3 const &, int, int, int, int, int, int, int, int, int, int, int, int, float, int, int, int, int, int, int, int, int) = 0;
IBipMaster * (_cdecl * Max7CreateNewBiped)(float, float, class Point3 const &, int, int, int, int, int, int, int, int, int, int, int, int, float, int, int, int, int) = 0;

NifImporter::NifImporter(const TCHAR *Name, ImpInterface *I, Interface *GI, BOOL SuppressPrompts)
	: BaseImporter()
{
	BaseInit(Name, I, GI, SuppressPrompts);
}

NifImporter::NifImporter()
{
}

INode* NifImporter::CreateImportNode(const TCHAR *name, Object *obj, INode* parent)
{
#if USE_IMPORTNODE
	ImpNode* impNode = i->CreateNode();
	impNode->Reference(obj);
	if (INode *n = impNode->GetINode()) {
		n->SetName(const_cast<TCHAR*>(name));
		n->SetObjectRef(obj);
		i->AddNodeToScene(impNode);
		this->RegisterNode(name, n);
		if (parent)
		{
			parent->AttachChild(impNode->GetINode());

			ASSERT(parent == n->GetParentNode());

		}
	}
	return impNode->GetINode();
#else
	if (INode* n = gi->CreateObjectNode(obj))
	{
		this->SetNodeName(n, name);
		if (parent)
		{
			parent->AttachChild(n);
			ASSERT(parent == n->GetParentNode());
		}
		return n;
	}
	return nullptr;

#endif
}

INode* NifImporter::CreateImportRagdollNode(const TCHAR *name, Object *obj, INode* parent)
{
	if (INode* n = gi->CreateObjectNode(obj))
	{
//		this->SetNodeName(n, name);
//		if (parent)
//		{
//			parent->AttachChild(n);
//			ASSERT(parent == n->GetParentNode());
//		}
		return n;
	}
	return nullptr;
}

void NifImporter::ReadBlocks()
{
	Niflib::NifInfo info;
	blocks = ReadNifList(T2AString(name), &info);
	//root = ReadNifTree(T2AString(name), &info);
	root = SelectFirstObjectOfType<NiObject>(blocks);
	nifVersion = info.version;
	userVersion = info.userVersion;
	userVersion2 = info.userVersion2;
	this->unnamedCounter = 0;
	BuildNodes();
}


void NifImporter::BuildNodes(NiNodeRef object, vector<NiNodeRef>& nodes)
{
	if (!object)
		return;

	// Handle nodes without names by setting before importing.  Really a hack but so is editing in nifskope and not settings names
	string name = object->GetName();
	if (name.empty())
	{
		name = FormatString("noname:%d", ++unnamedCounter);
		object->SetName(name);
	}
	nodes.push_back(object);
	vector<NiNodeRef> links = DynamicCast<NiNode>(object->GetChildren());
	for (vector<NiNodeRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
		BuildNodes(*itr, nodes);
}

void NifImporter::BuildNodes()
{
	BuildNodes(root, nodes);
	std::sort(nodes.begin(), nodes.end(), NodeEquivalence());
}

void NifImporter::Initialize()
{
	// Apply post processing checks after reading blocks
	if (isValid()) {
		if (goToSkeletonBindPosition && !nodes.empty() && importBones)
			GoToSkeletonBindPosition(nodes);

		// Only support biped if CreateNewBiped can be found.
		useBiped &= (Max8CreateNewBiped != nullptr || Max7CreateNewBiped != nullptr);

		hasSkeleton = HasSkeleton();
		isBiped = IsBiped();
		skeleton = GetSkeleton(appSettings);
		importSkeleton = hasSkeleton;
		importSkeleton &= !isBiped;
		bool skeletonExists = (-1 != _taccess(skeleton.c_str(), 0));

		// Guess that the skeleton is the same one in the current directory
		if (importSkeleton && !defaultSkeletonName.empty()) {
			TCHAR buffer[MAX_PATH], fullname[MAX_PATH];

			// preserve last selected setting if file not found
			importSkeleton = ((appSettings != nullptr) ? appSettings->useSkeleton : false);

			for (tstringlist::iterator itr = appSettings->skeletonSearchPaths.begin(),
				end = appSettings->skeletonSearchPaths.end(); itr != end; ++itr) {

				if ( PathIsRoot((*itr).c_str()) ) {
					GetFullPathName((*itr).c_str(), _countof(buffer), buffer, nullptr);
					PathRemoveFileSpec(buffer);
					PathAddBackslash(buffer);
					PathAppend(buffer, (*itr).c_str());
				} else {
					GetFullPathName(name.c_str(), _countof(buffer), buffer, nullptr);
					PathRemoveFileSpec(buffer);
					PathAddBackslash(buffer);
					PathAppend(buffer, (*itr).c_str());
				}
				PathAddBackslash(buffer);
				PathAppend(buffer, defaultSkeletonName.c_str());
				GetFullPathName(buffer, _countof(fullname), fullname, nullptr);
				bool defaultSkeletonExists = (-1 != _taccess(fullname, 0));
				if (defaultSkeletonExists) {
					importSkeleton = (defaultSkeletonExists || skeletonExists);
					skeleton = fullname;
					break;
				}
			}
		}
		else {
			hasSkeleton = false;
		}

	}
}

tstring NifImporter::GetSkeleton(AppSettings *appSettings)
{
	tstring skeleton = (appSettings != nullptr) ? appSettings->Skeleton : TEXT("");
	// Guess that the skeleton is the same one in the current directory
	if (importSkeleton && !defaultSkeletonName.empty()) {
		TCHAR buffer[MAX_PATH];
		GetFullPathName(name.c_str(), _countof(buffer), buffer, nullptr);
		PathRemoveFileSpec(buffer);
		PathAddBackslash(buffer);
		PathAppend(buffer, defaultSkeletonName.c_str());
		if (-1 != _taccess(buffer, 0))
			skeleton = buffer;
	}
	return skeleton;
}

void NifImporter::LoadIniSettings()
{
	TCHAR iniName[MAX_PATH];
	GetIniFileName(iniName);
	this->iniFileName = iniName;
	iniFileValid = (-1 != _taccess(iniName, 0));

	// Locate which application to use. If Auto, find first app where this file appears in the root path list
	appSettings = nullptr;
	tstring curapp = GetIniValue<tstring>(NifImportSection, TEXT("CurrentApp"), TEXT("AUTO"));
	tstring lastselapp = GetIniValue<tstring>(NifImportSection, TEXT("LastSelectedApp"), TEXT(""));
	if (0 == _tcsicmp(curapp.c_str(), TEXT("AUTO"))) {
		autoDetect = true;
		// Scan Root paths
		bool versionmatch = false;
		int version = GetNifVersion(T2AString(this->name));
		for (AppSettingsMap::iterator itr = TheAppSettings.begin(), end = TheAppSettings.end(); itr != end; ++itr) {
			if ((*itr).IsFileInRootPaths(this->name)) {
				appSettings = &(*itr);
				break;
			}
			else if (!versionmatch && ParseVersionString(T2AString((*itr).NiVersion)) == version) {
				// Version matching is an ok fit but we want the other if possible. And we want the first match if possible.
				appSettings = &(*itr);
				versionmatch = true;
			}
		}
	}
	else {
		autoDetect = false;
		appSettings = FindAppSetting(curapp);
	}
	if (appSettings == nullptr && !lastselapp.empty())
		appSettings = FindAppSetting(lastselapp);
	if (appSettings == nullptr && !TheAppSettings.empty()) {
		appSettings = &TheAppSettings.front();
	}

	// General System level
	useBiped = GetIniValue(NifImportSection, TEXT("UseBiped"), false);
	skeletonCheck = GetIniValue<tstring>(NifImportSection, TEXT("SkeletonCheck"), TEXT("Bip*"));
	showTextures = GetIniValue(NifImportSection, TEXT("ShowTextures"), true);
	removeIllegalFaces = GetIniValue(NifImportSection, TEXT("RemoveIllegalFaces"), true);
	removeDegenerateFaces = GetIniValue(NifImportSection, TEXT("RemoveDegenerateFaces"), true);
	enableAutoSmooth = GetIniValue(NifImportSection, TEXT("EnableAutoSmooth"), true);
	autoSmoothAngle = GetIniValue(NifImportSection, TEXT("AutoSmoothAngle"), 30.0f);
	flipUVTextures = GetIniValue(NifImportSection, TEXT("FlipUVTextures"), true);
	enableSkinSupport = GetIniValue(NifImportSection, TEXT("EnableSkinSupport"), true);
	enableCollision = GetIniValue(NifImportSection, TEXT("EnableCollision"), true);
	enableLights = GetIniValue(NifImportSection, TEXT("Lights"), false);
	enableCameras = GetIniValue(NifImportSection, TEXT("Cameras"), false);
	vertexColorMode = GetIniValue<int>(NifImportSection, TEXT("VertexColorMode"), 1);
	useNiftoolsShader = GetIniValue<int>(NifImportSection, TEXT("UseNiftoolsShader"), 1);
	mergeNonAccum = GetIniValue(NifImportSection, TEXT("MergeNonAccum"), true);
	importUPB = GetIniValue(NifImportSection, TEXT("ImportUPB"), true);
	ignoreRootNode = GetIniValue(NifImportSection, TEXT("IgnoreRootNode"), true);
	weldVertices = GetIniValue(NifImportSection, TEXT("WeldVertices"), false);
	weldVertexThresh = GetIniValue(NifImportSection, TEXT("WeldVertexThresh"), 0.01f);
	dummyBonesAsLines = GetIniValue(NifImportSection, TEXT("DummyBonesAsLines"), false);
	importBonesAsDummy = GetIniValue(NifImportSection, TEXT("ImportBonesAsDummy"), false);
	disableBSDismemberSkinModifier = GetIniValue(NifImportSection, TEXT("DisableBSDismemberSkinModifier"), false);

	// Biped
	importBones = GetIniValue(BipedImportSection, TEXT("ImportBones"), true);
	bipedHeight = GetIniValue(BipedImportSection, TEXT("BipedHeight"), 131.90f);
	bipedAngle = GetIniValue(BipedImportSection, TEXT("BipedAngle"), 90.0f);
	bipedAnkleAttach = GetIniValue(BipedImportSection, TEXT("BipedAnkleAttach"), 0.2f);
	bipedTrianglePelvis = GetIniValue(BipedImportSection, TEXT("BipedTrianglePelvis"), false);
	removeUnusedImportedBones = GetIniValue(BipedImportSection, TEXT("RemoveUnusedImportedBones"), false);
	forceRotation = GetIniValue(BipedImportSection, TEXT("ForceRotation"), true);
	browseForSkeleton = GetIniValue(BipedImportSection, TEXT("BrowseForSkeleton"), true);
	defaultSkeletonName = GetIniValue<tstring>(BipedImportSection, TEXT("DefaultSkeletonName"), TEXT("Skeleton.Nif"));
	minBoneWidth = GetIniValue(BipedImportSection, TEXT("MinBoneWidth"), 0.5f);
	maxBoneWidth = GetIniValue(BipedImportSection, TEXT("MaxBoneWidth"), 3.0f);
	boneWidthToLengthRatio = GetIniValue(BipedImportSection, TEXT("BoneWidthToLengthRatio"), 0.25f);
	createNubsForBones = GetIniValue(BipedImportSection, TEXT("CreateNubsForBones"), true);
	dummyNodeMatches = TokenizeString(GetIniValue<tstring>(BipedImportSection, TEXT("DummyNodeMatches"), TEXT("")).c_str(), TEXT(";"));
	convertBillboardsToDummyNodes = GetIniValue(BipedImportSection, TEXT("ConvertBillboardsToDummyNodes"), true);
	uncontrolledDummies = GetIniValue(BipedImportSection, TEXT("UncontrolledDummies"), true);

	// Animation
	replaceTCBRotationWithBezier = GetIniValue(AnimImportSection, TEXT("ReplaceTCBRotationWithBezier"), true);
	enableAnimations = GetIniValue(AnimImportSection, TEXT("EnableAnimations"), true);
	requireMultipleKeys = GetIniValue(AnimImportSection, TEXT("RequireMultipleKeys"), true);
	applyOverallTransformToSkinAndBones = GetIniValue(AnimImportSection, TEXT("ApplyOverallTransformToSkinAndBones"), true);
	clearAnimation = GetIniValue(AnimImportSection, TEXT("ClearAnimation"), true);
	addNoteTracks = GetIniValue(AnimImportSection, TEXT("AddNoteTracks"), true);
	addTimeTags = GetIniValue(AnimImportSection, TEXT("AddTimeTags"), true);

	rotate90Degrees = TokenizeString(GetIniValue<tstring>(NifImportSection, TEXT("Rotate90Degrees"), TEXT("")).c_str(), TEXT(";"));

	// Collision
	bhkScaleFactor = GetIniValue<float>(CollisionSection, TEXT("bhkScaleFactor"), 6.9969f);
	ApplyAppSettings(true);
}
void NifImporter::ApplyAppSettings(bool initialize)
{
	goToSkeletonBindPosition = false;
	// Override specific settings
	if (appSettings) {
		if (appSettings->disableCreateNubsForBones)
			createNubsForBones = false;
		goToSkeletonBindPosition = appSettings->goToSkeletonBindPosition;
		if (!appSettings->dummyNodeMatches.empty())
			dummyNodeMatches = appSettings->dummyNodeMatches;
		if (appSettings->applyOverallTransformToSkinAndBones != -1)
			applyOverallTransformToSkinAndBones = appSettings->applyOverallTransformToSkinAndBones ? true : false;
		if (!appSettings->rotate90Degrees.empty())
			rotate90Degrees = appSettings->rotate90Degrees;
		supportPrnStrings = appSettings->supportPrnStrings;
		doNotReuseExistingBones = appSettings->doNotReuseExistingBones;
		if (!appSettings->skeletonCheck.empty())
			skeletonCheck = appSettings->skeletonCheck;
		bhkScaleFactor = appSettings->GetSetting(TEXT("bhkScaleFactor"), bhkScaleFactor);
	}
}

void NifImporter::SaveIniSettings()
{
	SetIniValue(NifImportSection, TEXT("UseBiped"), useBiped);
	SetIniValue(NifImportSection, TEXT("EnableSkinSupport"), enableSkinSupport);
	SetIniValue(NifImportSection, TEXT("VertexColorMode"), vertexColorMode);
	SetIniValue(NifImportSection, TEXT("EnableCollision"), enableCollision);
	SetIniValue(NifImportSection, TEXT("Lights"), enableLights);
	SetIniValue(NifImportSection, TEXT("Cameras"), enableCameras);

	//SetIniValue(NifImportSection, TEXT("EnableFurniture"), enableAnimations);

	SetIniValue(NifImportSection, TEXT("FlipUVTextures"), flipUVTextures);
	SetIniValue(NifImportSection, TEXT("ShowTextures"), showTextures);
	SetIniValue(NifImportSection, TEXT("EnableAutoSmooth"), enableAutoSmooth);
	SetIniValue(NifImportSection, TEXT("RemoveIllegalFaces"), removeIllegalFaces);
	SetIniValue(NifImportSection, TEXT("RemoveDegenerateFaces"), removeDegenerateFaces);
	SetIniValue(NifImportSection, TEXT("ImportUPB"), importUPB);
	SetIniValue(NifImportSection, TEXT("IgnoreRootNode"), ignoreRootNode);

	SetIniValue(BipedImportSection, TEXT("ImportBones"), importBones);
	SetIniValue(BipedImportSection, TEXT("RemoveUnusedImportedBones"), removeUnusedImportedBones);

	SetIniValue(AnimImportSection, TEXT("EnableAnimations"), enableAnimations);
	SetIniValue(AnimImportSection, TEXT("ClearAnimation"), clearAnimation);
	SetIniValue(AnimImportSection, TEXT("AddNoteTracks"), addNoteTracks);
	SetIniValue(AnimImportSection, TEXT("AddTimeTags"), addTimeTags);
	SetIniValue(NifImportSection, TEXT("WeldVertices"), weldVertices);
	SetIniValue(NifImportSection, TEXT("WeldVertexThresh"), weldVertexThresh);
	SetIniValue(NifImportSection, TEXT("DummyBonesAsLines"), dummyBonesAsLines);
	SetIniValue(NifImportSection, TEXT("ImportBonesAsDummy"), importBonesAsDummy);
	SetIniValue(NifImportSection, TEXT("DisableBSDismemberSkinModifier"), disableBSDismemberSkinModifier);

	SetIniValue<tstring>(NifImportSection, TEXT("CurrentApp"), autoDetect ? TEXT("AUTO") : appSettings->Name);
	SetIniValue<tstring>(NifImportSection, TEXT("LastSelectedApp"), appSettings->Name);
}

void NifImporter::RegisterNode(Niflib::NiObjectNETRef node, INode* inode)
{
	nodeMap[node] = inode;
}

INode* NifImporter::FindNode(Niflib::NiObjectNETRef node)
{
	// may want to make this a map if its hit a lot
	if (nullptr == node) return nullptr;

	NodeToNodeMap::iterator itr = nodeMap.find(node);
	if (itr != nodeMap.end())
		return (*itr).second;

	//return gi->GetINodeByName(node->GetName().c_str());
	return GetNode(node->GetName());
}

void NifImporter::SetNodeName(INode* inode, const LPCTSTR name)
{
	if (name == nullptr || name[0] == 0)
	{
		TSTR str;
		str.printf(TEXT("noname:%d"), ++unnamedCounter);
		SetNodeName(inode, str);
		inode->SetUserPropBool(NP_NONAME, TRUE);
	}
	else if (wildmatch(TEXT("noname*"), name))
	{
		inode->SetUserPropBool(NP_NONAME, TRUE);
	}
	inode->SetName(const_cast<LPTSTR>(name));
	this->RegisterNode(name, inode);
}

INode *NifImporter::GetNode(Niflib::NiNodeRef node)
{
	return FindNode(node);
}

INode *NifImporter::GetNode(Niflib::NiObjectNETRef obj)
{
	if (obj->IsDerivedType(NiNode::TYPE)) {
		NiNodeRef node = StaticCast<NiNode>(obj);
		if (INode *n = GetNode(node)) {
			return n;
		}
	}
	//return gi->GetINodeByName(obj->GetName().c_str());
	return GetNode(obj->GetName());
}

void NifImporter::RegisterNode(const string& name, INode* inode) {
	nodeNameMap[A2TString(name)] = inode;
}
void NifImporter::RegisterNode(const wstring& name, INode* inode) {
	nodeNameMap[W2TString(name)] = inode;
}
INode *NifImporter::GetNode(const string& name) {
	tstring tname = A2TString(name);
	NameToNodeMap::iterator itr = nodeNameMap.find(tname);
	if (itr != nodeNameMap.end())
		return (*itr).second;

	INode *node = gi->GetINodeByName(tname.c_str());
	if (node != nullptr) {
		nodeNameMap[tname] = node;
	}
	return node;
}
INode *NifImporter::GetNode(const wstring& name) {
	tstring tname = W2TString(name);
	NameToNodeMap::iterator itr = nodeNameMap.find(tname);
	if (itr != nodeNameMap.end())
		return (*itr).second;

	INode *node = gi->GetINodeByName(tname.c_str());
	if (node != nullptr) {
		nodeNameMap[tname] = node;
	}
	return node;
}

INode *NifImporter::GetNode(const TSTR& name) {
	return GetNode(tstring(name.data()));
}


bool NifImporter::FindFile(const tstring& name, tstring& resolved_name) const
{
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(name.c_str())) {
		if (-1 != _taccess(name.c_str(), 0)) {
			resolved_name = tstring(name);
			return true;
		}
	}
	if (!path.empty()) {
		PathCombine(buffer, path.c_str(), name.c_str()); // try as-is
		if (-1 != _taccess(buffer, 0)) {
			resolved_name = tstring(buffer);
			return true;
		}

		// try only filename in nif directory
		PathCombine(buffer, path.c_str(), PathFindFileName(name.c_str()));
		if (-1 != _taccess(buffer, 0)) {
			resolved_name = tstring(buffer);
			return true;
		}
	}
	if (appSettings != nullptr) {
		return appSettings->FindFile(name, resolved_name);
	}
	resolved_name = name;
	return false;
}

bool NifImporter::FindFileByType(const tstring& name, FileType type, tstring& resolved_name) const
{
	switch(type)
	{
	case FT_Texture: resolved_name = FindImage(name); return true;
	case FT_Material: resolved_name = FindMaterial(name); return true;
	}
	return FindFile(name, resolved_name);
}

bool NifImporter::GetRelativePath(tstring& name, FileType type) const
{
	if (type == FT_Texture) {
		if (appSettings != nullptr) {
			name = appSettings->GetRelativeTexPath(name, TEXT("textures"));
			return true;
		}
	}
	return false;
}

bool NifImporter::DoImport()
{
	bool ok = true;
	if (!suppressPrompts)
	{
		if (!ShowDialog())
			return true;

		ApplyAppSettings(false);
		SaveIniSettings();
	}

	unnamedCounter = 0;
	vector<string> importedBones;
	if (!isBiped && importSkeleton && importBones)
	{
		if (importSkeleton && !skeleton.empty()) {
			try
			{
				NifImporter skelImport(skeleton.c_str(), i, gi, suppressPrompts);
				if (skelImport.isValid())
				{
					// Enable Skeleton specific items
					skelImport.isBiped = true;
					skelImport.importBones = true;
					// Disable undesirable skeleton items
					skelImport.enableCollision = false;
					skelImport.enableAnimations = false;
					skelImport.suppressPrompts = true;
					skelImport.doNotReuseExistingBones = true; // ignore dupes while importing skeleton regardless of other settings
					skelImport.DoImport();
					if (!skelImport.useBiped && removeUnusedImportedBones)
						importedBones = GetNamesOfNodes(skelImport.nodes);
				}
			}
			catch (RuntimeError &)
			{
				// ignore import errors and continue
			}
		}
	}
	else if (hasSkeleton && useBiped && importBones) {
		ImportBipeds(nodes);
	}

	if (isValid()) {

		if (root->IsDerivedType(NiNode::TYPE))
		{
			NiNodeRef rootNode = root;

			if (importBones) {
				if (ignoreRootNode || strmatch(rootNode->GetName(), "Scene Root")) {
					//Oblivion skeleton is like this
					RegisterNode(root, gi->GetRootNode());
					ImportBonesWithRagdoll(DynamicCast<NiNode>(rootNode->GetChildren()), NULL);
				}
				else {
					ImportBonesWithRagdoll(rootNode, NULL);
				}
			}


			if (enableLights) {
				ok = ImportLights(rootNode);
			}

			ok = ImportMeshes(rootNode);

			// Import Havok Collision Data surrounding node
			if (enableCollision) {
				ImportCollision(rootNode);
			}

			if (importSkeleton && removeUnusedImportedBones) {
				vector<string> importedNodes = GetNamesOfNodes(nodes);
				sort(importedBones.begin(), importedBones.end());
				sort(importedNodes.begin(), importedNodes.end());
				vector<string> results;
				results.resize(importedBones.size());
				vector<string>::iterator end = set_difference(
					importedBones.begin(), importedBones.end(),
					importedNodes.begin(), importedNodes.end(), results.begin());
				for (vector<string>::iterator itr = results.begin(); itr != end; ++itr) {
					if (INode *node = gi->GetINodeByName(A2TString(*itr).c_str())) {
						gi->DeleteNode(node, FALSE);
					}
				}
			}
		}
		else if (root->IsDerivedType(NiTriShape::TYPE))
		{
			ok |= ImportMesh(NiTriShapeRef(root));
		}
		else if (root->IsDerivedType(NiTriStrips::TYPE))
		{
			ok |= ImportMesh(NiTriStripsRef(root));
		}
	}

	ClearAnimation();
	ImportAnimation();
	return true;
}
bool NifImporter::IsSkyrim() const {
	return (nifVersion == 0x14020007 && userVersion == 12 && userVersion2 < 130);
}
bool NifImporter::IsFallout3() const {
	return (nifVersion == 0x14020007 && userVersion == 11);
}
bool NifImporter::IsFallout4() const {
	return (nifVersion == 0x14020007 && userVersion == 12 && userVersion2 == 130);
}
bool NifImporter::IsOblivion() const {
	return ((nifVersion == 0x14000004 || nifVersion == 0x14000005) && (userVersion == 11 || userVersion == 10));
}
bool NifImporter::IsMorrowind() const {
	return ((nifVersion == 0x04000002) && (userVersion == 11 || userVersion == 10));
}


