#include <io.h>
#include <string.h>
#include <tchar.h>
#include "AppSettings.h"
#include "IniSection.h"

AppSettingsMap TheAppSettings;
static bool TheAppSettingsInitialized = false;
bool AppSettings::Initialized()
{
	return TheAppSettingsInitialized;
}

void AppSettings::Initialize(Interface *gi)
{
	TCHAR iniName[MAX_PATH];
	GetIniFileName(iniName);
	if (-1 != _taccess(iniName, 0)) {
		bool reparse = GetIniValue<bool>(TEXT("System"), TEXT("Reparse"), false, iniName);
		if (reparse || TheAppSettings.empty()) {
			TheAppSettings.clear();
		}
		TheAppSettingsInitialized = true;

		tstring Applications = GetIniValue<tstring>(TEXT("System"), TEXT("KnownApplications"), TEXT(""), iniName);
		tstringlist apps = TokenizeString(Applications.c_str(), TEXT(";"));
		apps.push_back(tstring(TEXT("User"))); // always ensure that user is present
		for (tstringlist::iterator appstr = apps.begin(); appstr != apps.end(); ++appstr) {
			AppSettings* setting = FindAppSetting(*appstr);
			if (nullptr == setting) {
				AppSettingsMap::iterator itr = TheAppSettings.insert(TheAppSettings.end(), AppSettings(*appstr));
				(*itr).ReadSettings(iniName);
			}
		}
	}
}

void AppSettings::ReadSettings(tstring iniFile)
{
	NameValueCollection settings = ReadIniSection(Name.c_str(), iniFile.c_str());

	// expand indirect values first
	for (NameValueCollection::iterator itr = settings.begin(), end = settings.end(); itr != end; ++itr)
		itr->second = GetIndirectValue(itr->second.c_str());

	// next expand qualifiers
	for (NameValueCollection::iterator itr = settings.begin(), end = settings.end(); itr != end; ++itr)
		itr->second = ExpandQualifiers(itr->second.c_str(), settings);

	// finally expand environment variables, last because it clobbers my custom qualifier expansion
	for (NameValueCollection::iterator itr = settings.begin(), end = settings.end(); itr != end; ++itr)
		itr->second = ExpandEnvironment(itr->second);

	std::swap(Environment, settings);

	NiVersion = GetSetting<tstring>(TEXT("NiVersion"), TEXT("20.0.0.5"));
	NiUserVersion = GetSetting<int>(TEXT("NiUserVersion"), 0);
	NiUserVersion2 = GetSetting<int>(TEXT("NiUserVersion2"), 0);

	rootPath = GetSetting<tstring>(TEXT("RootPath"));
	rootPaths = TokenizeString(GetSetting<tstring>(TEXT("RootPaths")).c_str(), TEXT(";"));
	searchPaths = TokenizeString(GetSetting<tstring>(TEXT("TextureSearchPaths")).c_str(), TEXT(";"));
	extensions = TokenizeString(GetSetting<tstring>(TEXT("TextureExtensions")).c_str(), TEXT(";"));
	textureRootPaths = TokenizeString(GetSetting<tstring>(TEXT("TextureRootPaths")).c_str(), TEXT(";"));
	materialRootPaths = TokenizeString(GetSetting<tstring>(TEXT("MaterialPaths")).c_str(), TEXT(";"));

	Skeleton = GetSetting<tstring>(TEXT("Skeleton"));
	useSkeleton = GetSetting<bool>(TEXT("UseSkeleton"), useSkeleton);
	skeletonSearchPaths = TokenizeString(GetSetting<tstring>(TEXT("SkeletonSearchPaths"), TEXT(".")).c_str(), TEXT(";"));

	goToSkeletonBindPosition = GetSetting<bool>(TEXT("GoToSkeletonBindPosition"), goToSkeletonBindPosition);
	disableCreateNubsForBones = GetSetting<bool>(TEXT("DisableCreateNubsForBones"), disableCreateNubsForBones);
	applyOverallTransformToSkinAndBones = GetSetting<int>(TEXT("ApplyOverallTransformToSkinAndBones"), -1);
	textureUseFullPath = GetSetting<int>(TEXT("TextureUseFullPath"), textureUseFullPath);

	dummyNodeMatches = TokenizeString(GetSetting<tstring>(TEXT("DummyNodeMatches")).c_str(), TEXT(";"));
	rotate90Degrees = TokenizeString(GetSetting<tstring>(TEXT("Rotate90Degrees")).c_str(), TEXT(";"));
	supportPrnStrings = GetSetting<bool>(TEXT("SupportPrnStrings"), supportPrnStrings);
	doNotReuseExistingBones = GetSetting<bool>(TEXT("DoNotReuseExistingBones"), doNotReuseExistingBones);

	skeletonCheck = GetSetting<tstring>(TEXT("SkeletonCheck"));
}

void AppSettings::WriteSettings(Interface *gi)
{
	TCHAR iniName[MAX_PATH];
	GetIniFileName(iniName);
	if (-1 != _taccess(iniName, 0))
	{
		SetIniValue(Name.c_str(), TEXT("NiVersion"), NiVersion.c_str(), iniName);
		SetIniValue(Name.c_str(), TEXT("NiUserVersion"), FormatString(TEXT("%d"), NiUserVersion).c_str(), iniName);
		SetIniValue(Name.c_str(), TEXT("NiUserVersion2"), FormatString(TEXT("%d"), NiUserVersion2).c_str(), iniName);
	}
}


void AppSettings::CacheImages()
{
	if (!parsedImages) {
		FindImages(imgTable, rootPath, searchPaths, extensions);
		parsedImages = true;
	}
}

tstring AppSettings::FindImage(const tstring& fname) const {
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(fname.c_str())) {
		if (-1 != _taccess(fname.c_str(), 0))
			return fname;
	}

	// Test if its relative and in one of the specified root paths
	for (tstringlist::const_iterator itr = textureRootPaths.begin(), end = textureRootPaths.end(); itr != end; ++itr) {
		PathCombine(buffer, itr->c_str(), fname.c_str());
		if (-1 != _taccess(buffer, 0)) {
			return tstring(buffer);
		}
	}

	// Hit the directories to find out whats out there
	const_cast<AppSettings*>(this)->CacheImages();

	// Search my filename for our texture
	_tcscpy(buffer, PathFindFileName(fname.c_str()));
	PathRemoveExtension(buffer);
	NameValueCollection::const_iterator nmitr = imgTable.find(buffer);
	if (nmitr != imgTable.end()) {
		if (!rootPath.empty()) {
			_tcscpy(buffer, rootPath.c_str());
			PathCombine(buffer, rootPath.c_str(), ((*nmitr).second).c_str());
			return tstring(buffer);
		}
		else {
			return (*nmitr).second;
		}
	}
	return fname;
}

tstring AppSettings::FindMaterial(const tstring& fname) const {
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(fname.c_str())) {
		if (-1 != _taccess(fname.c_str(), 0))
			return fname;
	}

	// Test if its relative and in one of the specified root paths
	for (tstringlist::const_iterator itr = materialRootPaths.begin(), end = materialRootPaths.end(); itr != end; ++itr) {
		PathCombine(buffer, itr->c_str(), fname.c_str());
		if (-1 != _taccess(buffer, 0)) {
			return tstring(buffer);
		}
	}

	for (LPCTSTR filepart = PathFindNextComponent(fname.c_str()); filepart != nullptr; filepart = PathFindNextComponent(filepart)) {
		if (wildmatch(TEXT("materials\\*"), filepart)) {
			return FindMaterial(fname);
		}
	}
	return fname;
}

bool AppSettings::FindFile(const tstring& fname, tstring& resolved_name) const
{
	TCHAR buffer[MAX_PATH];

	// Simply check for fully qualified path
	if (!PathIsRelative(fname.c_str())) {
		if (-1 != _taccess(fname.c_str(), 0)) {
			resolved_name = fname;
			return true;
		}
	}

	// Test if its relative and in one of the specified root paths
	for (tstringlist::const_iterator itr = rootPaths.begin(), end = rootPaths.end(); itr != end; ++itr) {
		PathCombine(buffer, itr->c_str(), fname.c_str());
		if (-1 != _taccess(buffer, 0)) {
			resolved_name = tstring(buffer);
			return true;
		}
	}
	resolved_name = fname;
	return false;
}

// Check whether the given file is a child of the root paths
bool AppSettings::IsFileInRootPaths(const tstring& fname) const
{
	TCHAR root[MAX_PATH];
	TCHAR file[MAX_PATH];
	GetFullPathName(fname.c_str(), _countof(file), file, nullptr);
	PathMakePretty(file);

	for (tstringlist::const_iterator itr = rootPaths.begin(), end = rootPaths.end(); itr != end; ++itr) {
		GetFullPathName((*itr).c_str(), _countof(root), root, nullptr);
		PathAddBackslash(root);
		PathMakePretty(root);
		if (-1 != _taccess(root, 0)) {
			auto len = _tcslen(root);
			if (0 == _tcsnicmp(root, file, len))
				return true;
		}
	}
	return false;
}

// Return the Relative Texture Path for filename or empty
tstring AppSettings::GetRelativeTexPath(const tstring& fname, const tstring& prefix) const
{
	return GetRelativeTexPath(fname.c_str(), prefix.c_str());
}

tstring AppSettings::GetRelativeTexPath(LPCTSTR fname, LPCTSTR prefix) const
{
	TCHAR buffer[MAX_PATH];
	if (textureUseFullPath == 1) // full path name
	{
		GetFullPathName(fname, _countof(buffer), buffer, nullptr);
		return tstring(buffer);
	}
	else if (textureUseFullPath == -1) // only filename
	{
		return tstring(PathFindFileName(fname));
	}
	if (!PathIsRelative(fname))
	{
		TCHAR root[MAX_PATH];
		TCHAR file[MAX_PATH];
		GetFullPathName(fname, _countof(file), file, nullptr);
		PathMakePretty(file);

		for (tstringlist::const_iterator itr = textureRootPaths.begin(), end = textureRootPaths.end(); itr != end; ++itr) {
			GetFullPathName((*itr).c_str(), _countof(root), root, nullptr);
			PathAddBackslash(root);
			PathMakePretty(root);
			if (-1 != _taccess(root, 0)) {
				size_t len = _tcslen(root);
				if (0 == _tcsnicmp(root, file, len))
					return tstring(file + len);
			}
		}
	}
	else // Test if its relative to one of the specified root paths just return the texture 
	{
		for (tstringlist::const_iterator itr = textureRootPaths.begin(), end = textureRootPaths.end(); itr != end; ++itr) {
			PathCombine(buffer, itr->c_str(), fname);
			if (-1 != _taccess(buffer, 0)) {
				return fname;
			}
		}
	}

	// check if prefix is in place if so then just return fname as is
	for (LPCTSTR path = fname; path != nullptr; path = PathFindNextComponent(path))
	{
		if (_tcsnicmp(path, prefix, _tcslen(prefix)) == 0)
			return tstring(path);
	}

	// Now just combine prefix with file portion of the name
	PathCombine(buffer, prefix, PathFindFileName(fname));
	return tstring(buffer);
}
