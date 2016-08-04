#include "pch.h"
#include "AppSettings.h"
#include "niutils.h"
#include  <io.h>
#include "obj/BSFadeNode.h"
#include "obj/NiControllerManager.h"
#include "obj/NiTimeController.h"
#include "obj/NiControllerSequence.h"
#include "ObjectRegistry.h"
#include "Hyperlinks.h"
using namespace Niflib;

#define NifExport_CLASS_ID	Class_ID(0xa57ff0a4, 0xa0374ffb)

LPCTSTR NifExportSection = TEXT("MaxNifExport");
TSTR shortDescription;

class NifExport : public SceneExport
{
public:

	static HWND		hParams;
	int				mDlgResult;
	TSTR iniFileName;
	TSTR fileVersion;
	TSTR webSite;
	TSTR wikiSite;

	int				ExtCount();					// Number of extensions supported
	const TCHAR		*Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR		*LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR		*ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR		*AuthorName();				// ASCII Author name
	const TCHAR		*CopyrightMessage();			// ASCII Copyright message
	const TCHAR		*OtherMessage1();			// Other message #1
	const TCHAR		*OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	BOOL			SupportsOptions(int ext, DWORD options);
	int				DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts = FALSE, DWORD options = 0);
	int				DoExportInternal(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options);

	//SDK_RESERVED_METHOD(1); // russom 02/26/01
	//SDK_RESERVED_METHOD(2); // russom 02/26/01
	//SDK_RESERVED_METHOD(3); // russom 02/26/01
	//SDK_RESERVED_METHOD(4); // russom 02/26/01
	//SDK_RESERVED_METHOD(5); // russom 02/26/01
	//SDK_RESERVED_METHOD(6); // russom 02/26/01

	NifExport();
	~NifExport();
};



class NifExportClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return TRUE; }
	void			*Create(BOOL loading = FALSE) { return new NifExport(); }
	const TCHAR		*ClassName() { return GetString(IDS_NIF_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return NifExport_CLASS_ID; }
	const TCHAR		*Category() { return GetString(IDS_CATEGORY); }

	const TCHAR		*InternalName() { return _T("NifExport"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle


};

static NifExportClassDesc NifExportDesc;
ClassDesc2* GetNifExportDesc() { return &NifExportDesc; }

extern list<NiObjectRef> GetAllObjectsByType(NiObjectRef const & root, const Type & type);

void NifExportUpdateStatusDlg(NifExport* imp, HWND hwnd);

INT_PTR CALLBACK NifExportOptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	USES_CONVERSION;
	static NifExport *imp = nullptr;
	TCHAR buffer[256];
	static bool setCursor = false;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		imp = (NifExport *)lParam;

		// Append file version to dialog
		if (!imp->fileVersion.isNull()) {
			GetWindowText(hWnd, buffer, _countof(buffer));
			_tcscat(buffer, TEXT(" "));
			_tcscat(buffer, imp->fileVersion);
			SetWindowText(hWnd, buffer);
		}

		CenterWindow(hWnd, GetParent(hWnd));
		CheckDlgButton(hWnd, IDC_CHK_STRIPS, Exporter::mTriStrips);
		CheckDlgButton(hWnd, IDC_CHK_HIDDEN, Exporter::mExportHidden);
		CheckDlgButton(hWnd, IDC_CHK_FURN, Exporter::mExportFurn);
		CheckDlgButton(hWnd, IDC_CHK_LIGHTS, Exporter::mExportLights);
		CheckDlgButton(hWnd, IDC_CHK_VCOLORS, Exporter::mVertexColors);
		SetDlgItemText(hWnd, IDC_ED_TEXPREFIX, Exporter::mTexPrefix.c_str());
		CheckDlgButton(hWnd, IDC_CHK_COLL, Exporter::mExportCollision);
		CheckDlgButton(hWnd, IDC_CHK_REMAP, Exporter::mRemapIndices);

		CheckDlgButton(hWnd, IDC_CHK_EXTRA, Exporter::mExportExtraNodes);
		CheckDlgButton(hWnd, IDC_CHK_UPB, Exporter::mUserPropBuffer);
		CheckDlgButton(hWnd, IDC_CHK_HIER, Exporter::mFlattenHierarchy);
		CheckDlgButton(hWnd, IDC_CHK_REM_BONES, Exporter::mRemoveUnreferencedBones);
		CheckDlgButton(hWnd, IDC_CHK_SORTNODES, Exporter::mSortNodesToEnd);
		CheckDlgButton(hWnd, IDC_CHK_SKEL_ONLY, Exporter::mSkeletonOnly);
		CheckDlgButton(hWnd, IDC_CHK_CAMERA, Exporter::mExportCameras);
		CheckDlgButton(hWnd, IDC_CHK_BONE_COLL, Exporter::mGenerateBoneCollision);
		CheckDlgButton(hWnd, IDC_CHK_TANGENTS, Exporter::mTangentAndBinormalExtraData);
		CheckDlgButton(hWnd, IDC_CHK_COLLAPSE_TRANS, Exporter::mCollapseTransforms);
		CheckDlgButton(hWnd, IDC_CHK_ZERO_TRANS, Exporter::mZeroTransforms);

		tstring selection = Exporter::mGameName;
		tstring version = Exporter::mNifVersion;
		tstring userVer = FormatString(TEXT("%d"), Exporter::mNifUserVersion);
		tstring userVer2 = FormatString(TEXT("%d"), Exporter::mNifUserVersion2);
		for (AppSettingsMap::iterator itr = TheAppSettings.begin(), end = TheAppSettings.end(); itr != end; ++itr)
			SendDlgItemMessage(hWnd, IDC_CB_GAME, CB_ADDSTRING, 0, LPARAM(itr->Name.c_str()));
		SendDlgItemMessage(hWnd, IDC_CB_GAME, CB_SELECTSTRING, WPARAM(-1), LPARAM(selection.c_str()));
		SendDlgItemMessage(hWnd, IDC_CB_VERSION, WM_SETTEXT, 0, LPARAM(version.c_str()));
		SendDlgItemMessage(hWnd, IDC_CB_USER_VERSION, WM_SETTEXT, 0, LPARAM(userVer.c_str()));
		SendDlgItemMessage(hWnd, IDC_CB_USER_VERSION2, WM_SETTEXT, 0, LPARAM(userVer2.c_str()));
		CheckDlgButton(hWnd, IDC_CHK_AUTO_DETECT, Exporter::mAutoDetect);

		// Populate Type options
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::NIF_WO_ANIM, LPARAM(TEXT("NIF w/o Animation")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::NIF_WO_KF, LPARAM(TEXT("NIF with Animation")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::SINGLE_KF_WITH_NIF, LPARAM(TEXT("Single KF with NIF")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::SINGLE_KF_WO_NIF, LPARAM(TEXT("Single KF w/o NIF")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::MULTI_KF_WITH_NIF, LPARAM(TEXT("Multi KF with NIF")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::MULTI_KF_WO_NIF, LPARAM(TEXT("Multi KF w/o NIF")));
		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_ADDSTRING, Exporter::NIF_WITH_MGR, LPARAM(TEXT("NIF w/ Manager")));

		SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_SETCURSEL, WPARAM(Exporter::mExportType), 0);

		CheckDlgButton(hWnd, IDC_CHK_TRANSFORMS2, Exporter::mExportTransforms);
		SetDlgItemText(hWnd, IDC_ED_PRIORITY2, FormatText(TEXT("%.1f"), Exporter::mDefaultPriority));
		//CheckDlgButton(hWnd, IDC_CHK_USE_TIME_TAGS, Exporter::mUseTimeTags);           

		// Skin
		CheckDlgButton(hWnd, IDC_CHK_SKIN, Exporter::mExportSkin);
		CheckDlgButton(hWnd, IDC_CHK_SKINPART, Exporter::mMultiplePartitions);
		SetDlgItemText(hWnd, IDC_ED_BONES_PART, FormatText(TEXT("%d"), Exporter::mBonesPerPartition));
		SetDlgItemText(hWnd, IDC_ED_BONES_VERTEX, FormatText(TEXT("%d"), Exporter::mBonesPerVertex));

		CheckDlgButton(hWnd, IDC_CHK_ALLOW_ACCUM, Exporter::mAllowAccum);

		TSTR tmp;
		tmp.printf(TEXT("%.4f"), Exporter::mWeldThresh);
		SetDlgItemText(hWnd, IDC_ED_WELDTHRESH2, tmp);

		CheckDlgButton(hWnd, IDC_CHK_START_NIFSKOPE, Exporter::mStartNifskopeAfterStart);

		ConvertStaticToHyperlink(hWnd, IDC_LBL_LINK);
		ConvertStaticToHyperlink(hWnd, IDC_LBL_WIKI);

		CheckDlgButton(hWnd, IDC_CHK_PARTSTRIPS, Exporter::mTriPartStrips);


		//EEDOCAN: FIXED BY INI SETTING
		// Populate Type options
		//int i = 0;
		//for (tstringlist::iterator itr = Exporter::mRootTypes.begin(); itr != Exporter::mRootTypes.end(); ++itr)
		//	SendDlgItemMessage(hWnd, IDC_CBO_ROOT_TYPE, CB_ADDSTRING, i++, LPARAM((*itr).c_str()));
		//LRESULT lIndex = SendDlgItemMessage(hWnd, IDC_CBO_ROOT_TYPE, CB_FINDSTRINGEXACT, -1, LPARAM(Exporter::mRootType.c_str()));
		//SendDlgItemMessage(hWnd, IDC_CBO_ROOT_TYPE, CB_SETCURSEL, WPARAM(lIndex), 0);

		NifExportUpdateStatusDlg(imp, hWnd);
		imp->mDlgResult = IDCANCEL;
	}
	return TRUE;

	case WM_CLOSE:
		EndDialog(hWnd, imp->mDlgResult);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			TCHAR tmp[MAX_PATH], *end;
			bool close = false;
			switch (LOWORD(wParam))
			{
			case IDOK:
				// Validity Check
				GetDlgItemText(hWnd, IDC_CB_VERSION, tmp, MAX_PATH);
				if (tmp[0] != 0)
				{
					int nifVersion = ParseVersionString(T2A(tmp));
					if (!IsSupportedVersion(nifVersion))
					{
						if (IDNO == MessageBox(hWnd, FormatString(TEXT("Version '%s' is not a known version. Do you wish to continue?"), tmp).c_str(), TEXT("NifExport"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP))
							return FALSE;
					}
				}

				Exporter::mTriStrips = IsDlgButtonChecked(hWnd, IDC_CHK_STRIPS);
				Exporter::mExportHidden = IsDlgButtonChecked(hWnd, IDC_CHK_HIDDEN);
				Exporter::mExportFurn = IsDlgButtonChecked(hWnd, IDC_CHK_FURN);
				Exporter::mExportLights = IsDlgButtonChecked(hWnd, IDC_CHK_LIGHTS);
				Exporter::mVertexColors = IsDlgButtonChecked(hWnd, IDC_CHK_VCOLORS);
				Exporter::mExportCollision = IsDlgButtonChecked(hWnd, IDC_CHK_COLL);
				Exporter::mRemapIndices = IsDlgButtonChecked(hWnd, IDC_CHK_REMAP);

				Exporter::mExportExtraNodes = IsDlgButtonChecked(hWnd, IDC_CHK_EXTRA);
				Exporter::mUserPropBuffer = IsDlgButtonChecked(hWnd, IDC_CHK_UPB);
				Exporter::mFlattenHierarchy = IsDlgButtonChecked(hWnd, IDC_CHK_HIER);
				Exporter::mRemoveUnreferencedBones = IsDlgButtonChecked(hWnd, IDC_CHK_REM_BONES);
				Exporter::mSortNodesToEnd = IsDlgButtonChecked(hWnd, IDC_CHK_SORTNODES);
				Exporter::mSkeletonOnly = IsDlgButtonChecked(hWnd, IDC_CHK_SKEL_ONLY);
				Exporter::mExportCameras = IsDlgButtonChecked(hWnd, IDC_CHK_CAMERA);
				Exporter::mGenerateBoneCollision = IsDlgButtonChecked(hWnd, IDC_CHK_BONE_COLL);
				Exporter::mTangentAndBinormalExtraData = IsDlgButtonChecked(hWnd, IDC_CHK_TANGENTS);
				Exporter::mCollapseTransforms = IsDlgButtonChecked(hWnd, IDC_CHK_COLLAPSE_TRANS);
				Exporter::mZeroTransforms = IsDlgButtonChecked(hWnd, IDC_CHK_ZERO_TRANS);

				Exporter::mExportTransforms = IsDlgButtonChecked(hWnd, IDC_CHK_TRANSFORMS2);
				//Exporter::mUseTimeTags = IsDlgButtonChecked(hWnd, IDC_CHK_USE_TIME_TAGS);           

				Exporter::mExportType = Exporter::ExportType(SendDlgItemMessage(hWnd, IDC_CBO_ANIM_TYPE, CB_GETCURSEL, 0, 0));
				GetDlgItemText(hWnd, IDC_ED_PRIORITY2, tmp, MAX_PATH);
				Exporter::mDefaultPriority = (float)_ttof(tmp);

				GetDlgItemText(hWnd, IDC_ED_TEXPREFIX, tmp, MAX_PATH);
				Exporter::mTexPrefix = tmp;

				GetDlgItemText(hWnd, IDC_ED_WELDTHRESH2, tmp, MAX_PATH);
				Exporter::mWeldThresh = (float)_ttof(tmp);

				Exporter::mAllowAccum = IsDlgButtonChecked(hWnd, IDC_CHK_ALLOW_ACCUM);

				// Skin
				Exporter::mExportSkin = IsDlgButtonChecked(hWnd, IDC_CHK_SKIN);
				Exporter::mMultiplePartitions = IsDlgButtonChecked(hWnd, IDC_CHK_SKINPART);
				GetDlgItemText(hWnd, IDC_ED_BONES_PART, tmp, MAX_PATH);
				Exporter::mBonesPerPartition = _ttoi(tmp);
				GetDlgItemText(hWnd, IDC_ED_BONES_VERTEX, tmp, MAX_PATH);
				Exporter::mBonesPerVertex = _ttoi(tmp);

				GetDlgItemText(hWnd, IDC_CB_GAME, tmp, MAX_PATH);
				if (AppSettings *appSettings = FindAppSetting(tmp))
				{
					Exporter::mGameName = appSettings->Name;
					GetDlgItemText(hWnd, IDC_CB_VERSION, tmp, MAX_PATH);
					Exporter::mNifVersion = tmp;
					GetDlgItemText(hWnd, IDC_CB_USER_VERSION, tmp, MAX_PATH);
					Exporter::mNifUserVersion = _tcstol(tmp, &end, 0);
					GetDlgItemText(hWnd, IDC_CB_USER_VERSION2, tmp, MAX_PATH);
					Exporter::mNifUserVersion2 = _tcstol(tmp, &end, 0);
				}
				Exporter::mAutoDetect = IsDlgButtonChecked(hWnd, IDC_CHK_AUTO_DETECT);
				Exporter::mStartNifskopeAfterStart = IsDlgButtonChecked(hWnd, IDC_CHK_START_NIFSKOPE);
				Exporter::mTriPartStrips = IsDlgButtonChecked(hWnd, IDC_CHK_PARTSTRIPS);

				//this was not working, I don't know why
				//{
					//LRESULT idx = SendDlgItemMessage(hWnd, IDC_CBO_ROOT_TYPE, CB_GETCURSEL, 0, 0);
					//Exporter::mRootType = SendDlgItemMessage(hWnd, IDC_CBO_ROOT_TYPE, CB_GETCURSEL, 0, 0);//(idx >= 0) ? Exporter::mRootTypes[idx] : TEXT("NiNode");
					//SendMessage(hWnd, CB_GETLBTEXT, idx, (LPARAM)tmp);
					//Exporter::mRootType = tmp;
				//}

				EndDialog(hWnd, imp->mDlgResult = IDOK);
				close = true;
				break;

			case IDCANCEL:
				EndDialog(hWnd, imp->mDlgResult = IDCANCEL);
				close = true;
				break;

			case IDC_LBL_LINK:
				ShellExecute(hWnd, TEXT("open"), imp->webSite, nullptr, nullptr, SW_SHOWNORMAL);
				break;

			case IDC_LBL_WIKI:
				ShellExecute(hWnd, TEXT("open"), imp->wikiSite, nullptr, nullptr, SW_SHOWNORMAL);
				break;
			}

			if (close)
				SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (LOWORD(wParam) == IDC_CB_GAME)
			{
				TCHAR tmp[MAX_PATH];
				GetDlgItemText(hWnd, IDC_CB_GAME, tmp, MAX_PATH);
				if (AppSettings *appSettings = FindAppSetting(tmp))
				{
					tstring version = appSettings->NiVersion;
					tstring userVer = FormatString(TEXT("%d"), appSettings->NiUserVersion);
					tstring userVer2 = FormatString(TEXT("%d"), appSettings->NiUserVersion2);
					SendDlgItemMessage(hWnd, IDC_CB_VERSION, WM_SETTEXT, 0, LPARAM(version.c_str()));
					SendDlgItemMessage(hWnd, IDC_CB_USER_VERSION, WM_SETTEXT, 0, LPARAM(userVer.c_str()));
					SendDlgItemMessage(hWnd, IDC_CB_USER_VERSION2, WM_SETTEXT, 0, LPARAM(userVer2.c_str()));
				}

				NifExportUpdateStatusDlg(imp, hWnd);
			}
		}
		break;
	}
	return FALSE;
}
void NifExportUpdateStatusDlg(NifExport* imp, HWND hWnd)
{
	TCHAR tmp[MAX_PATH];
	GetDlgItemText(hWnd, IDC_CB_GAME, tmp, MAX_PATH);
	if (AppSettings *appSettings = FindAppSetting(tmp))
	{
		bool isNotFallout4 = (!strmatch(appSettings->Name, TEXT("Fallout 4")));
		EnableWindow(GetDlgItem(hWnd, IDC_CHK_COLL), isNotFallout4);
		EnableWindow(GetDlgItem(hWnd, IDC_CHK_STRIPS), isNotFallout4);
		EnableWindow(GetDlgItem(hWnd, IDC_CHK_SKINPART), isNotFallout4);
		EnableWindow(GetDlgItem(hWnd, IDC_ED_BONES_PART), isNotFallout4);
		EnableWindow(GetDlgItem(hWnd, IDC_ED_BONES_VERTEX), isNotFallout4);
		EnableWindow(GetDlgItem(hWnd, IDC_CHK_PARTSTRIPS), isNotFallout4);
	}
}

//--- NifExport -------------------------------------------------------
NifExport::NifExport()
{
	Interface *gi = GetCOREInterface();
	TCHAR iniName[MAX_PATH];
	GetIniFileName(iniName);
	iniFileName = iniName;
	shortDescription = GetIniValue<TSTR>(TEXT("System"), TEXT("ShortDescription"), TEXT("Netimmerse/Gamebryo"), iniFileName);
	fileVersion = GetFileVersion(LPCTSTR(nullptr));

	webSite = GetIniValue<TSTR>(TEXT("System"), TEXT("Website"), TEXT("http://niftools.sourceforge.net"), iniFileName);
	wikiSite = GetIniValue<TSTR>(TEXT("System"), TEXT("Wiki"), TEXT("http://niftools.sourceforge.net/wiki/3ds_Max"), iniFileName);
}

NifExport::~NifExport()
{

}

int NifExport::ExtCount()
{
	return 2;
}

const TCHAR *NifExport::Ext(int n)
{
	switch (n)
	{
	case 0: return _T("KF");
	case 1: return _T("NIF");
	}
	return nullptr;
}

const TCHAR *NifExport::LongDesc()
{
	return _T("Gamebryo File");
}

const TCHAR *NifExport::ShortDesc()
{
	return shortDescription;
}

const TCHAR *NifExport::AuthorName()
{
	return _T("Alexander \"Gundalf\" Stillich / The Niftools Team");
}

const TCHAR *NifExport::CopyrightMessage()
{
	return _T("http://niftools.sourceforge.net");
}

const TCHAR *NifExport::OtherMessage1()
{
	return _T("http://niftools.sourceforge.net");
}

const TCHAR *NifExport::OtherMessage2()
{
	return _T("http://niftools.sourceforge.net");
}

unsigned int NifExport::Version()
{
	return Exporter::mVersion;
}

void NifExport::ShowAbout(HWND hWnd)
{

}

BOOL NifExport::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}

static DWORD WINAPI dummyProgress(LPVOID arg) {
	return(0);
}

int	NifExport::DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options)
{
	try
	{
		TSTR title = FormatText(TEXT("Exporting '%s'..."), PathFindFileName(name));
		i->PushPrompt(title);
		if (!suppressPrompts)
			i->ProgressStart(title, TRUE, dummyProgress, nullptr);
		DoExportInternal(name, ei, i, suppressPrompts, options);
	}
	catch (Exporter::CancelExporterException&)
	{
		// Special user cancellation exception
	}
	catch (exception &e)
	{
		if (!suppressPrompts)
			MessageBoxA(nullptr, e.what(), "Export Error", MB_OK);
	}
	catch (...)
	{
		if (!suppressPrompts)
			MessageBox(nullptr, TEXT("Unknown error."), TEXT("Export Error"), MB_OK);
	}
	try
	{
		i->PopPrompt();
		if (!suppressPrompts)
			i->ProgressEnd();
	}
	catch (...)
	{
	}
	return true;
}

int NifExport::DoExportInternal(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options)
{
	USES_CONVERSION;
	TCHAR path[MAX_PATH];
	TCHAR filename[MAX_PATH];
	_tcscpy(filename, PathFindFileName(name));
	PathRemoveExtension(filename);
	GetFullPathName(name, MAX_PATH, path, nullptr);
	PathRenameExtension(path, TEXT(".nif"));

	Exporter::mSelectedOnly = (options&SCENE_EXPORT_SELECTED) != 0;

	// read application settings
	AppSettings::Initialize(i);

	TCHAR iniName[MAX_PATH];
	GetIniFileName(iniName);
	bool iniNameIsValid = (-1 != _taccess(iniName, 0));

	// Set whether Config should use registry or not
	Exporter::mUseRegistry = !iniNameIsValid || GetIniValue<bool>(NifExportSection, TEXT("UseRegistry"), false, iniName);
	// read config from registry
	Exporter::readConfig(i);
	// read config from root node
	Exporter::readConfig(i->GetRootNode());

	// locate the "default" app setting
	tstring fname = path;
	AppSettings *appSettings = Exporter::importAppSettings(fname);

	Exporter::mSuppressPrompts = suppressPrompts;
	if (!suppressPrompts)
	{
		if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_NIF_PANEL), GetActiveWindow(), NifExportOptionsDlgProc, (LPARAM)this) != IDOK)
			return true;

		// write config to registry
		Exporter::writeConfig(i);
		// write config to root node
		Exporter::writeConfig(i->GetRootNode());

		appSettings = Exporter::exportAppSettings();
		appSettings->WriteSettings(i);
	}

	int nifVersion = VER_20_0_0_5;
	int nifUserVer = Exporter::mNifUserVersion;
	int nifUserVer2 = Exporter::mNifUserVersion2;
	if (!Exporter::mNifVersion.empty())
	{
		nifVersion = ParseVersionString(T2AString(Exporter::mNifVersion));
		if (!IsSupportedVersion(nifVersion))
		{
			string tmp = FormatVersionString(nifVersion);
			if (IDNO == MessageBox(GetActiveWindow(), FormatString(TEXT("Version '%s' is not a known version. Do you wish to continue?"), tmp.c_str()).c_str(), TEXT("NifExport"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP))
				return FALSE;
		}
	}
	Exporter::mNifVersionInt = nifVersion;

	Exporter::ExportType exportType = Exporter::mExportType;

	// Hack so MW exports cleaner. Basically write tree without NiControllerManager
	if (nifVersion <= VER_10_0_1_0
		&& (Exporter::mExportType != Exporter::NIF_WO_ANIM && Exporter::mExportType != Exporter::NIF_WITH_MGR)
		)
	{
		Exporter::mExportType = Exporter::NIF_WO_KF;
	}
	Niflib::NifInfo info(nifVersion, nifUserVer, nifUserVer2);
	info.endian = ENDIAN_LITTLE;
	info.creator = T2AString(Exporter::mCreatorName);
	info.exportInfo1 = T2A(FormatText(TEXT("Niftools 3ds Max Plugins %s"), fileVersion.data()));

	Exporter exp(i, appSettings);

	//i->Log()->LogEntry(SYSLOG_WARN, DISPLAY_DIALOG, T2AString("RootType"), _M("%s"), Exporter::mRootType.c_str());
	Ref<NiNode> root = DynamicCast<NiNode>(Niflib::ObjectRegistry::CreateObject(T2AString(Exporter::mRootType)));
	if (root == nullptr) {
		//i->Log()->LogEntry(SYSLOG_WARN, DISPLAY_DIALOG, T2AString("NULL"), T2AString("root is null"));
		if (Exporter::mSkeletonOnly)
			root = new BSFadeNode();
		else
			root = new NiNode();
	}
	if (exp.IsFallout3() || exp.IsSkyrim() || exp.IsFallout4())
		root->SetFlags(14);

	Exporter::Result result = exp.doExport(root, i->GetRootNode());

	if (result != Exporter::Ok && result != Exporter::Skip)
		throw exception("Unknown error.");

	if (exp.IsFallout4())
		root->SetName(T2A(filename));

	if (exportType == Exporter::NIF_WO_ANIM || exportType == Exporter::NIF_WITH_MGR)
	{
		WriteNifTree(T2AString(path), NiObjectRef(root), info);
	}
	else
	{
		Niflib::ExportOptions export_type = EXPORT_NIF;
		switch (exportType) {
		case Exporter::SINGLE_KF_WITH_NIF: export_type = EXPORT_NIF_KF;       break;
		case Exporter::SINGLE_KF_WO_NIF:   export_type = EXPORT_KF;           break;
		case Exporter::MULTI_KF_WITH_NIF:  export_type = EXPORT_NIF_KF_MULTI; break;
		case Exporter::MULTI_KF_WO_NIF:    export_type = EXPORT_KF_MULTI;     break;
		}

		Niflib::NifGame game = KF_MW;
		if (appSettings->Name == TEXT("Dark Age of Camelot")) {
			game = KF_DAOC;
		}
		else if (nifVersion <= VER_4_0_0_2) {
			game = KF_MW;
		}
		else if (nifVersion <= VER_20_0_0_4) {
			game = KF_FFVT3R;
		}
		else {
			game = KF_CIV4;
		}

		WriteFileGroup(T2AString(path), StaticCast<NiObject>(root), info, export_type, game);
	}

	if (Exporter::mStartNifskopeAfterStart)
	{
		//string nifskope = Exporter::mNifskopeDir + TEXT("\\Nifskope.exe");
		//if (_access(nifskope.c_str(), 04) != -1) {
		//   char buffer[260*2];
		//   sprintf(buffer, "\"%s\" \"%s\"", nifskope.c_str(), path);
		//   STARTUPINFO si; PROCESS_INFORMATION pi;
		//   memset(&si, 0, sizeof(si)); memset(&pi, 0, sizeof(pi));
		//   si.cb = sizeof(si);
		//   si.dwFlags = STARTF_USESHOWWINDOW;
		//   si.wShowWindow = SW_SHOWNORMAL;        
		//   if ( CreateProcess(nullptr, buffer, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, Exporter::mNifskopeDir.c_str(), &si, &pi) )
		//      CloseHandle( pi.hThread ), CloseHandle( pi.hProcess );
		//}
		tstring nifskope = Exporter::mNifskopeDir;
		tstring::size_type idx = nifskope.find(TEXT("%1"));
		if (idx != tstring::npos) {
			nifskope.replace(idx, 2, path);
			WinExec(T2A(nifskope.c_str()), SW_SHOWNORMAL);
		}
	}
	return true;
}


