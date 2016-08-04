; -- MaxNifUtils.iss --
;

[Setup]
AppName=NIF Utilities for 3ds Max
AppVerName=NIF Utilities {code:GetParam|fullver} for 3ds Max
AppPublisher=NIF File Format Library and Tools
AppCopyright=Copyright © 2015, NIF File Format Library and Tools
OutputBaseFilename=niftools-max-plugins
DisableProgramGroupPage=yes
Compression=lzma2/ultra64
SolidCompression=yes
DirExistsWarning=no
EnableDirDoesntExistWarning=yes
UsePreviousAppDir=yes
DefaultDirName={win}{\}Installer\NifTools
UninstallFilesDir={win}{\}Installer\NifTools
Uninstallable=yes
DisableDirPage=yes
ArchitecturesInstallIn64BitMode=x64
SourceDir=.
OutputDir=.\Output
;UninstallDisplayIcon={app}{\}..\Oblivion.exe

[Types]
;Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "max2016"; Description: "3ds Max 2016"; Types: custom;
Name: "max2015"; Description: "3ds Max 2015"; Types: custom;
Name: "max2014"; Description: "3ds Max 2014"; Types: custom;
Name: "max2013"; Description: "3ds Max 2013"; Types: custom;
Name: "max2012"; Description: "3ds Max 2012 (Win32)"; Types: custom;
Name: "max2012x64"; Description: "3ds Max 2012 (x64)"; Types: custom;
Name: "max2011"; Description: "3ds Max 2011 (Win32)"; Types: custom;
Name: "max2011x64"; Description: "3ds Max 2011 (x64)"; Types: custom;
Name: "max2010"; Description: "3ds Max 2010 (Win32)"; Types: custom;
Name: "max2010x64"; Description: "3ds Max 2010 (x64)"; Types: custom;
Name: "max2009"; Description: "3ds Max 2009 (Win32)"; Types: custom;
Name: "max2009x64"; Description: "3ds Max 2009 (x64)"; Types: custom;
Name: "max2008"; Description: "3ds Max 2008 (Win32)"; Types: custom;
Name: "max2008x64"; Description: "3ds Max 2008 (x64)"; Types: custom;
Name: "max9"; Description: "3ds Max 9 (Win32)"; Types: custom;
Name: "max9x64"; Description: "3ds Max 9 (x64)"; Types: custom;
Name: "max8"; Description: "3ds Max 8"; Types: custom;
Name: "max7"; Description: "3ds Max 7"; Types: custom;
Name: "max6"; Description: "3ds Max 6"; Types: custom;
Name: "max5"; Description: "3ds Max 5"; Types: custom;
Name: "gmax12"; Description: "gmax 1.2"; Types: custom;
;Name: "max4"; Description: "3ds Max 4"; Types: custom;
;Name: "max42"; Description: "3ds Max 4.2"; Types: custom;
;Name: "src"; Description: "Program Source";

[Files]
Source: "License.txt"; DestDir: "{app}"; Flags: isreadme ignoreversion;
Source: "ChangeLog.txt"; DestDir: "{app}"; Flags: isreadme ignoreversion;
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme ignoreversion;

;Source: "Staging\Win32\Release - gmax\Readme.txt"; DestName: "NifPlugins_Readme.txt"; DestDir: "{code:InstallPath|gmax12}"; Components: "gmax12"; Flags: isreadme ignoreversion;
Source: "..\gmax12\winmm.dll"; DestDir: "{code:InstallPath|gmax12}"; Components: "gmax12"; Flags: ignoreversion;
Source: "Staging\Win32\Release - gmax\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|gmax12}{\}plugins"; Components: "gmax12"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|gmax12}{\}plugcfg"; Components: "gmax12"; Flags: ignoreversion; AfterInstall: FixPathInINI('gmax12');
Source: "MaxNifStrings.ini"; DestDir: "{code:InstallPath|gmax12}{\}plugcfg"; Components: "gmax12"; Flags: ignoreversion; AfterInstall: FixPathInINI('gmax12');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|gmax12}{\}plugins"; Components: "gmax12"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|gmax12}{\}plugins"; Components: "gmax12"; Flags: ignoreversion;

;Source: "Staging\Win32\Release - Max 4\Readme.txt"; DestName: "NifPlugins_Readme.txt"; DestDir: "{code:InstallPath|max4}"; Components: "max4"; Flags: isreadme ignoreversion;
;Source: "Staging\Win32\Release - Max 4\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max4}{\}plugins"; Components: "max4"; Flags: ignoreversion;
;Source: "Staging\Win32\Release - Max 4\MaxNifTools.ini"; DestDir: "{code:InstallPath|max4}{\}plugcfg"; Components: "max4"; Flags: ignoreversion;

;Source: "Staging\Win32\Release - Max 4.2\Readme.txt"; DestName: "NifPlugins_Readme.txt"; DestDir: "{code:InstallPath|max42}"; Components: "max42"; Flags: isreadme ignoreversion;
;Source: "Staging\Win32\Release - Max 4.2\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max42}{\}plugins"; Components: "max42"; Flags: ignoreversion;
;Source: "Staging\Win32\Release - Max 4.2\MaxNifTools.ini"; DestDir: "{code:InstallPath|max42}{\}plugcfg"; Components: "max42"; Flags: ignoreversion;
;Source: "..\NifMopp\Win32\NifMopp.dll" DestDir: "{code:InstallPath|max42}{\}plugins"; Components: "gmax12"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 5\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max5}{\}plugins"; Components: "max5"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max5}{\}plugcfg"; Components: "max5"; Flags: ignoreversion; AfterInstall: FixPathInINI('max5');
Source: "MaxNifStrings.ini"; DestDir: "{code:InstallPath|max5}{\}plugcfg"; Components: "max5"; Flags: ignoreversion; AfterInstall: FixPathInINI('max5');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max5}{\}plugins"; Components: "max5"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max5}{\}plugins"; Components: "max5"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 6\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max6}{\}plugins"; Components: "max6"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max6}{\}plugcfg"; Components: "max6"; Flags: ignoreversion; AfterInstall: FixPathInINI('max6');
Source: "MaxNifStrings.ini"; DestDir: "{code:InstallPath|max6}{\}plugcfg"; Components: "max6"; Flags: ignoreversion; AfterInstall: FixPathInINI('max6');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max6}{\}plugins"; Components: "max6"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max6}{\}plugins"; Components: "max6"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 7\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max7}{\}plugins"; Components: "max7"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max7}{\}plugcfg"; Components: "max7"; Flags: ignoreversion; AfterInstall: FixPathInINI('max7');
Source: "MaxNifStrings.ini"; DestDir: "{code:InstallPath|max7}{\}plugcfg"; Components: "max7"; Flags: ignoreversion; AfterInstall: FixPathInINI('max7');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max7}{\}plugins"; Components: "max7"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max7}{\}plugins"; Components: "max7"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 8\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max8}{\}plugins"; Components: "max8"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max8}{\}plugcfg"; Components: "max8"; Flags: ignoreversion; AfterInstall: FixPathInINI('max8');
Source: "MaxNifStrings.ini"; DestDir: "{code:InstallPath|max8}{\}plugcfg"; Components: "max8"; Flags: ignoreversion; AfterInstall: FixPathInINI('max8');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max8}{\}plugins"; Components: "max8"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max8}{\}plugins"; Components: "max8"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 9\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max9}{\}plugins"; Components: "max9"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max9}{\}plugcfg"; Components: "max9"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\9 - 32bit\enu\plugcfg"; Components: "max9"; Flags: ignoreversion; AfterInstall: FixPathInINI('max9');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\9 - 32bit\enu\plugcfg"; Components: "max9"; Flags: ignoreversion; AfterInstall: FixPathInINI('max9');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max9}{\}plugins"; Components: "max9"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max9}{\}plugins"; Components: "max9"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 9\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max9x64}{\}plugins"; Components: "max9x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max9x64}{\}plugcfg"; Components: "max9x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\9 - 64bit\enu\plugcfg"; Components: "max9x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max9x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\9 - 64bit\enu\plugcfg"; Components: "max9x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max9x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max9x64}{\}plugins"; Components: "max9x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max9x64}{\}plugins"; Components: "max9x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max9x64}{\}plugins"; Components: "max9x64"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 2008\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2008}{\}plugins"; Components: "max2008"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2008}{\}plugcfg"; Components: "max2008"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2008 - 32bit\enu\plugcfg"; Components: "max2008"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2008');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2008 - 32bit\enu\plugcfg"; Components: "max2008"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2008');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max2008}{\}plugins"; Components: "max2008"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max2008}{\}plugins"; Components: "max2008"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2008\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2008x64}{\}plugins"; Components: "max2008x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2008x64}{\}plugcfg"; Components: "max2008x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2008 - 64bit\enu\plugcfg"; Components: "max2008x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2008x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2008 - 64bit\enu\plugcfg"; Components: "max2008x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2008x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2008x64}{\}plugins"; Components: "max2008x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max2008x64}{\}plugins"; Components: "max2008x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max2008x64}{\}plugins"; Components: "max2008x64"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 2009\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2009}{\}plugins"; Components: "max2009"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2009}{\}plugcfg"; Components: "max2009"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2009 - 32bit\enu\plugcfg"; Components: "max2009"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2009');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2009 - 32bit\enu\plugcfg"; Components: "max2009"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2009');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max2009}{\}plugins"; Components: "max2009"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max2009}{\}plugins"; Components: "max2009"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2009\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2009x64}{\}plugins"; Components: "max2009x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2009x64}{\}plugcfg"; Components: "max2009x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2009 - 64bit\enu\plugcfg"; Components: "max2009x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2009x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2009 - 64bit\enu\plugcfg"; Components: "max2009x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2009x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2009x64}{\}plugins"; Components: "max2009x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max2009x64}{\}plugins"; Components: "max2009x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max2009x64}{\}plugins"; Components: "max2009x64"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 2010\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2010}{\}plugins"; Components: "max2010"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2010}{\}plugcfg"; Components: "max2010"; Flags: ignoreversion
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2010 - 32bit\enu\plugcfg"; Components: "max2010"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2010');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2010 - 32bit\enu\plugcfg"; Components: "max2010"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2010');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max2010}{\}plugins"; Components: "max2010"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max2010}{\}plugins"; Components: "max2010"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2010\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2010x64}{\}plugins"; Components: "max2010x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2010x64}{\}plugcfg"; Components: "max2010x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2010 - 64bit\enu\plugcfg"; Components: "max2010x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2010x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2010 - 64bit\enu\plugcfg"; Components: "max2010x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2010x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2010x64}{\}plugins"; Components: "max2010x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max2010x64}{\}plugins"; Components: "max2010x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max2010x64}{\}plugins"; Components: "max2010x64"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 2011\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2011}{\}plugins"; Components: "max2011"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2011}{\}plugcfg"; Components: "max2011"; Flags: ignoreversion
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2011 - 32bit\enu\plugcfg"; Components: "max2011"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2011');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2011 - 32bit\enu\plugcfg"; Components: "max2011"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2011');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max2011}{\}plugins"; Components: "max2011"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max2011}{\}plugins"; Components: "max2011"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2011\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2011x64}{\}plugins"; Components: "max2011x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2011x64}{\}plugcfg"; Components: "max2011x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2011 - 64bit\enu\plugcfg"; Components: "max2011x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2011x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2011 - 64bit\enu\plugcfg"; Components: "max2011x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2011x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2011x64}{\}plugins"; Components: "max2011x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max2011x64}{\}plugins"; Components: "max2011x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max2011x64}{\}plugins"; Components: "max2011x64"; Flags: ignoreversion;

Source: "Staging\Win32\Release - Max 2012\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2012}{\}plugins"; Components: "max2012"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2012}{\}plugcfg"; Components: "max2012"; Flags: ignoreversion
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2012 - 32bit\enu\plugcfg"; Components: "max2012"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2012');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2012 - 32bit\enu\plugcfg"; Components: "max2012"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2012');
Source: "..\NifMopp\Win32\NifMopp.dll"; DestDir: "{code:InstallPath|max2012}{\}plugins"; Components: "max2012"; Flags: ignoreversion;
Source: "..\NifMagic\Win32\NifMagic.dll"; DestDir: "{code:InstallPath|max2012}{\}plugins"; Components: "max2012"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2012\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2012x64}{\}plugins"; Components: "max2012x64"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2012x64}{\}plugcfg"; Components: "max2012x64"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2012 - 64bit\enu\plugcfg"; Components: "max2012x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2012x64');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2012 - 64bit\enu\plugcfg"; Components: "max2012x64"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2012x64');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2012x64}{\}plugins"; Components: "max2012x64"; Flags: ignoreversion;
Source: "..\NifMopp\x64\Release\NifMopp.dll"; DestDir: "{code:InstallPath|max2012x64}{\}plugins"; Components: "max2012x64"; Flags: ignoreversion;
Source: "..\NifMopp\Release - Exe\NifMopp.exe"; DestDir: "{code:InstallPath|max2012x64}{\}plugins"; Components: "max2012x64"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2013\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2013}{\}plugins"; Components: "max2013"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2013}{\}plugcfg"; Components: "max2013"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2013 - 64bit\enu\en-US\plugcfg"; Components: "max2013"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2013');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2013 - 64bit\enu\en-US\plugcfg"; Components: "max2013"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2013');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2013}{\}plugins"; Components: "max2013"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.dll"; DestDir: "{code:InstallPath|max2013}{\}plugins"; Components: "max2013"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.exe"; DestDir: "{code:InstallPath|max2013}{\}plugins"; Components: "max2013"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2014\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2014}{\}plugins"; Components: "max2014"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2014}{\}plugcfg"; Components: "max2014"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2014 - 64bit\enu\en-US\plugcfg"; Components: "max2014"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2014');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2014 - 64bit\enu\en-US\plugcfg"; Components: "max2014"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2014');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2014}{\}plugins"; Components: "max2014"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.dll"; DestDir: "{code:InstallPath|max2014}{\}plugins"; Components: "max2014"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.exe"; DestDir: "{code:InstallPath|max2014}{\}plugins"; Components: "max2014"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2015\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2015}{\}plugins"; Components: "max2015"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2015}{\}plugcfg"; Components: "max2015"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2015 - 64bit\enu\en-US\plugcfg"; Components: "max2015"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2015');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2015 - 64bit\enu\en-US\plugcfg"; Components: "max2015"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2015');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2015}{\}plugins"; Components: "max2015"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.dll"; DestDir: "{code:InstallPath|max2015}{\}plugins"; Components: "max2015"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.exe"; DestDir: "{code:InstallPath|max2015}{\}plugins"; Components: "max2015"; Flags: ignoreversion;

Source: "Staging\x64\Release - Max 2016\NifPlugins\NifPlugins.dlu"; DestDir: "{code:InstallPath|max2016}{\}plugins"; Components: "max2016"; Flags: ignoreversion;
;Source: "MaxNifTools.ini"; DestDir: "{code:InstallPath|max2016}{\}plugcfg"; Components: "max2016"; Flags: ignoreversion;
Source: "MaxNifTools.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2016 - 64bit\enu\en-US\plugcfg"; Components: "max2016"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2016');
Source: "MaxNifStrings.ini"; DestDir: "{localappdata}{\}Autodesk\3dsmax\2016 - 64bit\enu\en-US\plugcfg"; Components: "max2016"; Flags: ignoreversion; AfterInstall: FixPathInINI('max2016');
Source: "..\NifMagic\x64\NifMagic.dll"; DestDir: "{code:InstallPath|max2016}{\}plugins"; Components: "max2016"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.dll"; DestDir: "{code:InstallPath|max2016}{\}plugins"; Components: "max2016"; Flags: ignoreversion;
Source: "..\NifMopp\x64\NifMopp.exe"; DestDir: "{code:InstallPath|max2016}{\}plugins"; Components: "max2016"; Flags: ignoreversion;

;[InstallDelete]
;Type: files; Name: "{code:InstallPath|max9}{\}plugcfg{\}MaxNifTools.ini";
;Type: files; Name: "{code:InstallPath|max9x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2008}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2008x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2009}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2009x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2010}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2010x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2011}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2011x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2012}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2012x64}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2013}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2014}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2015}{\}plugcfg{\}MaxNifTools.ini"; 
;Type: files; Name: "{code:InstallPath|max2016}{\}plugcfg{\}MaxNifTools.ini"; 


[Code]
#include "NifCommon\build.isi"

var i: Integer;
    UsagePage: TInputOptionWizardPage;
    OBDataDirPage: TInputDirWizardPage;
    OBSIDataDirPage: TInputDirWizardPage;
    MWDataDirPage: TInputDirWizardPage;
    Civ4DataDirPage: TInputDirWizardPage;
    DAoCDataDirPage: TInputDirWizardPage;
    FFDataDirPage: TInputDirWizardPage;
    FF3RDataDirPage: TInputDirWizardPage;
    BCDataDirPage: TInputDirWizardPage;
    F3DataDirPage: TInputDirWizardPage;
    FNVDataDirPage: TInputDirWizardPage;
    F4DataDirPage: TInputDirWizardPage;
    SKDataDirPage: TInputDirWizardPage;

function InitializeSetup(): Boolean;
begin
  Result := True;
end;

function RegSoftware32(): String;
begin
  if ( IsWin64() ) then begin
      Result := 'SOFTWARE\Wow6432Node\';
  end else begin
      Result := 'SOFTWARE\';
  end;
end;

function RegSoftware64(): String;
begin
  Result := 'SOFTWARE\';
end;

function GetOptionString(value: Boolean): String;
begin
  if value then begin
    Result := 'True';
  end else begin
    Result := 'False';
  end;
end;

function GetOptionBool(value: String): Boolean;
begin
  if value = 'True' then begin
    Result := True;
  end else begin
    Result := False;
  end;
end;

function GetPrevDataBool(key: String; value: Boolean): Boolean;
begin
    Result := GetOptionBool(GetPreviousData(key, GetOptionString(value)));
end;

function SetPrevDataString(PreviousDataKey: Integer; key:String; value: Boolean): Boolean;
begin
    Result := SetPreviousData(PreviousDataKey, key, GetOptionString(value));
end;

function DataDirPage_ShouldSkipPage(Page: TWizardPage): Boolean;
begin
    Result := True;
    case Page.ID of
      F4DataDirPage.ID: Result    := not UsagePage.Values[0];
      SKDataDirPage.ID: Result    := not UsagePage.Values[1];
      F3DataDirPage.ID: Result    := not UsagePage.Values[2];
      FNVDataDirPage.ID: Result    := not UsagePage.Values[3];
      OBDataDirPage.ID: Result    := not UsagePage.Values[4];
      OBSIDataDirPage.ID: Result  := not UsagePage.Values[5];
      MWDataDirPage.ID: Result    := not UsagePage.Values[6];
      Civ4DataDirPage.ID: Result  := not UsagePage.Values[7];
      DAoCDataDirPage.ID: Result  := not UsagePage.Values[8];
      FFDataDirPage.ID: Result    := not UsagePage.Values[9];
      FF3RDataDirPage.ID: Result  := not UsagePage.Values[10];
      BCDataDirPage.ID: Result    := not UsagePage.Values[11];
      
    end;
end;

procedure InitializeWizard;
begin
  { Create the pages }

  { Add each supported game to option page }
  UsagePage := CreateInputOptionPage(wpSelectComponents,
    'Custom Directories', 'Select Custom Directories for supported Games',
    'Please specify which games you wish to add custom directories for, then click Next.',
    False, False);
  UsagePage.Add('Fallout 4');
  UsagePage.Add('Skyrim');
  UsagePage.Add('Fallout 3');
  UsagePage.Add('Fallout NV');
  UsagePage.Add('Oblivion');
  UsagePage.Add('Oblivion: Shivering Isles');
  UsagePage.Add('Morrowind');
  UsagePage.Add('Civilization 4');
  UsagePage.Add('Dark Age of Camelot');
  UsagePage.Add('Freedom Force');
  UsagePage.Add('Freedom Force vs. the 3rd Reich');
  UsagePage.Add('Star Trek: Bridge Commander');

  { Create pages for each Game texture locations }
  F4DataDirPage := CreateInputDirPage(UsagePage.ID,
    'Select Fallout 4 Data Directory', 'Where are the extracted Fallout 4 data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  F4DataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  F4DataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  F4DataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');
  F4DataDirPage.Add('Extracted Materials Directory (e.g. root directory containing the Materials directory)');

  SKDataDirPage := CreateInputDirPage(F4DataDirPage.ID,
    'Select Skyrim Data Directory', 'Where are the extracted Skyrim data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  SKDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  SKDataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  SKDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  F3DataDirPage := CreateInputDirPage(SKDataDirPage.ID,
    'Select Fallout 3 Data Directory', 'Where are the extracted Fallout 3 data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  F3DataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  F3DataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  F3DataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  FNVDataDirPage := CreateInputDirPage(F3DataDirPage.ID,
    'Select Fallout New Vegas Data Directory', 'Where are the extracted Fallout New Vegas data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  FNVDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  FNVDataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  FNVDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  OBDataDirPage := CreateInputDirPage(FNVDataDirPage.ID,
    'Select Oblivion Data Directory', 'Where are the extracted Oblivion data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  OBDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  OBDataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  OBDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  OBSIDataDirPage := CreateInputDirPage(OBDataDirPage.ID,
    'Select Oblivion Shivering Isles Data Directory', 'Where are the extracted Oblivion data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  OBSIDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  OBSIDataDirPage.Add('Extracted Model Directory (e.g. root directory containing the Meshes directory)');
  OBSIDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  MWDataDirPage := CreateInputDirPage(OBSIDataDirPage.ID,
    'Select Morrowind Data Directory', 'Where are the extracted Morrowind data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  MWDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  MWDataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  MWDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  Civ4DataDirPage := CreateInputDirPage(MWDataDirPage.ID,
    'Select Civilization 4 Data Directory', 'Where are the extracted Civilization 4 data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  Civ4DataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  Civ4DataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  Civ4DataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  DAoCDataDirPage := CreateInputDirPage(Civ4DataDirPage.ID,
    'Select Dark Age of Camelot Data Directory', 'Where are the extracted Dark Age of Camelot data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  DAoCDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  DAoCDataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  DAoCDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  FFDataDirPage := CreateInputDirPage(DAoCDataDirPage.ID,
    'Select Freedom Force Data Directory', 'Where are the extracted Freedom Force data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  FFDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  FFDataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  FFDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  FF3RDataDirPage := CreateInputDirPage(FFDataDirPage.ID,
    'Select Freedom Force vs. the 3rd Reich Data Directory', 'Where are the extracted Freedom Force vs. the 3rd Reich data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  FF3RDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  FF3RDataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  FF3RDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');

  BCDataDirPage := CreateInputDirPage(FF3RDataDirPage.ID,
    'Select Star Trek: Bridge Commander Data Directory', 'Where are the extracted Star Trek: Bridge Commander data files located?',
    'Select the folders in which 3ds Max should look for files, then click Next.',
    False, '');
  BCDataDirPage.OnShouldSkipPage := @DataDirPage_ShouldSkipPage;
  BCDataDirPage.Add('Extracted Model Directory (e.g. root directory where NIF files are located)');
  BCDataDirPage.Add('Extracted Textures Directory (e.g. root directory containing the Textures directory)');
  
  { Set default values, using settings that were stored last time if possible }
  UsagePage.Values[0] := GetPrevDataBool('bF4', False);
  UsagePage.Values[1] := GetPrevDataBool('bSK', False);
  UsagePage.Values[2] := GetPrevDataBool('bF3', False);
  UsagePage.Values[3] := GetPrevDataBool('bFNV', False);
  UsagePage.Values[4] := GetPrevDataBool('bOB', False);
  UsagePage.Values[5] := GetPrevDataBool('bOBSI', False);
  UsagePage.Values[6] := GetPrevDataBool('bMW', False);
  UsagePage.Values[7] := GetPrevDataBool('bCiv4', False);
  UsagePage.Values[8] := GetPrevDataBool('bDAoC', False);
  UsagePage.Values[9] := GetPrevDataBool('bFF', False);
  UsagePage.Values[10] := GetPrevDataBool('bFF3R', False);
  UsagePage.Values[11] := GetPrevDataBool('bBC', False);

  OBDataDirPage.Values[0] := GetPreviousData('OBModelDir', '');
  OBDataDirPage.Values[1] := GetPreviousData('OBTexDir', '');
  OBSIDataDirPage.Values[0] := GetPreviousData('OBSIModelDir', '');
  OBSIDataDirPage.Values[1] := GetPreviousData('OBSITexDir', '');
  MWDataDirPage.Values[0] := GetPreviousData('MWModelDir', '');
  MWDataDirPage.Values[1] := GetPreviousData('MWTexDir', '');
  Civ4DataDirPage.Values[0] := GetPreviousData('Civ4ModelDir', '');
  Civ4DataDirPage.Values[1] := GetPreviousData('Civ4TexDir', '');
  DAoCDataDirPage.Values[0] := GetPreviousData('DAoCModelDir', '');
  DAoCDataDirPage.Values[1] := GetPreviousData('DAoCTexDir', '');
  FFDataDirPage.Values[0] := GetPreviousData('FFModelDir', '');
  FFDataDirPage.Values[1] := GetPreviousData('FFTexDir', '');
  FF3RDataDirPage.Values[0] := GetPreviousData('FF3RModelDir', '');
  FF3RDataDirPage.Values[1] := GetPreviousData('FF3RTexDir', '');
  BCDataDirPage.Values[0] := GetPreviousData('BCModelDir', '');
  BCDataDirPage.Values[1] := GetPreviousData('BCTexDir', '');
  F3DataDirPage.Values[0] := GetPreviousData('F3ModelDir', '');
  F3DataDirPage.Values[1] := GetPreviousData('F3TexDir', '');
  SKDataDirPage.Values[0] := GetPreviousData('SKModelDir', '');
  SKDataDirPage.Values[1] := GetPreviousData('SKTexDir', '');
  FNVDataDirPage.Values[0] := GetPreviousData('FNVModelDir', '');
  FNVDataDirPage.Values[1] := GetPreviousData('FNVTexDir', '');
  F4DataDirPage.Values[0] := GetPreviousData('F4ModelDir', '');
  F4DataDirPage.Values[1] := GetPreviousData('F4TexDir', '');
  F4DataDirPage.Values[2] := GetPreviousData('F4MtlDir', '');

end;


procedure RegisterPreviousData(PreviousDataKey: Integer);
var
  UsageMode: String;
begin
  { Store the settings so we can restore them next time }
  SetPrevDataString(PreviousDataKey, 'bF4',   UsagePage.Values[0]);
  SetPrevDataString(PreviousDataKey, 'bSK',   UsagePage.Values[1]);
  SetPrevDataString(PreviousDataKey, 'bF3',   UsagePage.Values[2]);
  SetPrevDataString(PreviousDataKey, 'bFNV',   UsagePage.Values[3]);
  SetPrevDataString(PreviousDataKey, 'bOB',   UsagePage.Values[4]);
  SetPrevDataString(PreviousDataKey, 'bOBSI', UsagePage.Values[5]);
  SetPrevDataString(PreviousDataKey, 'bMW',   UsagePage.Values[6]);
  SetPrevDataString(PreviousDataKey, 'bCiv4', UsagePage.Values[7]);
  SetPrevDataString(PreviousDataKey, 'bDAoC', UsagePage.Values[8]);
  SetPrevDataString(PreviousDataKey, 'bFF',   UsagePage.Values[9]);
  SetPrevDataString(PreviousDataKey, 'bFF3R', UsagePage.Values[10]);
  SetPrevDataString(PreviousDataKey, 'bBC',   UsagePage.Values[11]);

  SetPreviousData(PreviousDataKey, 'OBModelDir', OBDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'OBTexDir', OBDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'OBSIModelDir', OBSIDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'OBSITexDir', OBSIDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'MWModelDir', MWDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'MWTexDir', MWDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'Civ4ModelDir', Civ4DataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'Civ4TexDir', Civ4DataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'DAoCModelDir', DAoCDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'DAoCTexDir', DAoCDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'FFModelDir', FFDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'FFTexDir', FFDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'FF3RModelDir', FF3RDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'FF3RTexDir', FF3RDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'BCModelDir', BCDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'BCTexDir', BCDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'F3ModelDir', F3DataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'F3TexDir', F3DataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'SKModelDir', SKDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'SKTexDir', SKDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'FNVModelDir', FNVDataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'FNVTexDir', FNVDataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'F4ModelDir', F4DataDirPage.Values[0]);
  SetPreviousData(PreviousDataKey, 'F4TexDir', F4DataDirPage.Values[1]);
  SetPreviousData(PreviousDataKey, 'F4MtlDir', F4DataDirPage.Values[2]);

end;

function InstallPath(Param: String): String;
  var
    Names: TArrayOfString;
    I: Integer;
begin
  Result := '';
  case Param of
    'gmax12':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\gmax', 'uninstallpath', Result) then
          Result := ExpandConstant('{sd}{\}gmax12');
    'max4':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 4', 'uninstallpath', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax4');
    'max42':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 4', 'uninstallpath', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax42');
    'max5':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 5', 'uninstallpath', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax5');
    'max6':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\6.0', 'InstallDir', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax6');
    'max7':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\7.0', 'InstallDir', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax7');
    'max8':
        if not RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\8.0', 'InstallDir', Result) then
          Result := ExpandConstant('{sd}{\}3dsmax8');
    'max9': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\9.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\9.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 9{\}');
        end;
    'max9x64':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\9.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\9.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 9{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2008': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\10.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\10.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 2008{\}');
        end;
    'max2008x64':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\10.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\10.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2008{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2009': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\11.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\11.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 2009{\}');
        end;
    'max2009x64':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\11.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\11.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2009{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2010': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\12.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\12.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 2010{\}');
        end;
    'max2010x64':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\12.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\12.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2010{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2011': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\13.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\13.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 2011{\}');
        end;
    'max2011x64':
        begin
          if IsWin64() then begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\13.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\13.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2011{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2012': 
        begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\14.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\14.0\' + Names[I], 'Installdir', Result) then begin
                break;
              end;
            end;
          end;
          if (Length(Result) = 0) then
            Result := ExpandConstant('{pf32}{\}AutoDesk\3ds Max 2012{\}');
        end;
    'max2012x64':
        begin
          if IsWin64() then begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\14.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\14.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2012{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2013':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\15.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\15.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2013{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2014':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\16.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\16.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2014{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2015':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\17.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\17.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2015{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    'max2016':
        begin
          If IsWin64() Then Begin
          if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\18.0', Names) then begin
            for I := 0 to GetArrayLength(Names)-1 do begin
              if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\18.0\' + Names[I], 'InstallDir', Result) then
                break;              
            end;
          end;
          if Length(Result) = 0 then
              Result := ExpandConstant('{pf64}{\}AutoDesk\3ds Max 2016{\}');
          end else begin
              Result := GetTempDir();
          end;
        end;
    else
      Result := '';
  end;
end;

function RegInstallPath(Param: String): String;
  var
    Names: TArrayOfString;
    I: Integer;
begin
  Result := '';
  case Param of
    'gmax 1.2':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\gmax', 'uninstallpath', Result);
    '3ds Max 4':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 4', 'uninstallpath', Result);
    '3ds Max 4.2':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 4', 'uninstallpath', Result);
    '3ds Max 5':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Microsoft\Windows\CurrentVersion\Uninstall\3ds max 5', 'uninstallpath', Result);
    '3ds Max 6':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\6.0', 'InstallDir', Result);
    '3ds Max 7':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\7.0', 'InstallDir', Result);
    '3ds Max 8':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\8.0', 'InstallDir', Result);
    '3ds Max 9 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\9.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\9.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 9 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\9.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\9.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2008 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\10.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\10.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2008 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\10.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\10.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2009 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\11.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\11.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2009 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\11.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\11.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2010 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\12.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\12.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2010 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\12.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\12.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2011 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\13.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\13.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2011 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\13.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\13.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2012 (Win32)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\14.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware32() + 'Autodesk\3dsMax\14.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2012 (x64)':
        if RegGetSubkeyNames(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\14.0', Names) then begin
          for I := 0 to GetArrayLength(Names)-1 do begin
            if RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\14.0\' + Names[I], 'InstallDir', Result) then
              break;              
          end;
        end;
    '3ds Max 2013':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\15.0', 'InstallDir', Result);
    '3ds Max 2014':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\16.0', 'InstallDir', Result);
    '3ds Max 2015':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\17.0', 'InstallDir', Result);
    '3ds Max 2016':
        RegQueryStringValue(HKEY_LOCAL_MACHINE, RegSoftware64() + 'Autodesk\3dsMax\18.0', 'InstallDir', Result);
    else
      Result := '';
  end;
end;

function IsInstalled(Param: String): Boolean;
begin
  Result := Length(RegInstallPath(Param)) <> 0;
  if Result then begin
    if not IsWin64() and (Param = '3ds Max 9 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2008 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2009 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2010 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2011 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2012 (x64)') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2013') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2014') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2015') then
       Result := False;
    if not IsWin64() and (Param = '3ds Max 2016') then
       Result := False;
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageId = wpSelectComponents then
    for i := 0 to WizardForm.ComponentsList.Items.Count - 1 do
       WizardForm.ComponentsList.Checked[i] := IsInstalled(WizardForm.ComponentsList.ItemCaption[i]);
end;

procedure FixPathInINI(component: String);
  var
    iniFile: String;
begin
  if IsComponentSelected(component) then begin
      iniFile := CurrentFileName();
      iniFile := ExpandConstant(iniFile);
      iniFile := ExpandFileName(iniFile);

      if UsagePage.Values[0] then begin
        SetIniString('Fallout 4', 'MeshRootPath', F4DataDirPage.Values[0], iniFile);
        SetIniString('Fallout 4', 'TextureRootPath', F4DataDirPage.Values[1], iniFile);
        SetIniString('Fallout 4', 'MaterialRootPath', F4DataDirPage.Values[2], iniFile);
      end;
      if UsagePage.Values[1] then begin
        SetIniString('Skyrim', 'MeshRootPath', SKDataDirPage.Values[0], iniFile);
        SetIniString('Skyrim', 'TextureRootPath', SKDataDirPage.Values[1], iniFile);
      end;
      if UsagePage.Values[2] then begin
        SetIniString('Fallout 3', 'MeshRootPath', F3DataDirPage.Values[0], iniFile);
        SetIniString('Fallout 3', 'TextureRootPath', F3DataDirPage.Values[1], iniFile);
      end;
      if UsagePage.Values[3] then begin
        SetIniString('Fallout NV', 'MeshRootPath', FNVDataDirPage.Values[0], iniFile);
        SetIniString('Fallout NV', 'TextureRootPath', FNVDataDirPage.Values[1], iniFile);
      end;
      if UsagePage.Values[4] then begin {Oblivion}
        SetIniString('Oblivion', 'MeshRootPath', OBDataDirPage.Values[0], iniFile);
        SetIniString('Oblivion', 'TextureRootPath', OBDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[5] then begin
        SetIniString('Oblivion', 'IslesMeshRootPath', OBSIDataDirPage.Values[0], iniFile);
        SetIniString('Oblivion', 'IslesTextureRootPath', OBSIDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[6] then begin
        SetIniString('Morrowind', 'MeshRootPath', MWDataDirPage.Values[0], iniFile);
        SetIniString('Morrowind', 'TextureRootPath', MWDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[7] then begin
        SetIniString('Civilization 4', 'MeshRootPath', Civ4DataDirPage.Values[0], iniFile);
        SetIniString('Civilization 4', 'TextureRootPath', Civ4DataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[8] then begin
        SetIniString('Dark Age of Camelot', 'MeshRootPath', DAoCDataDirPage.Values[0], iniFile);
        SetIniString('Dark Age of Camelot', 'TextureRootPath', DAoCDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[9] then begin
        SetIniString('Freedom Force', 'MeshRootPath', FFDataDirPage.Values[0], iniFile);
        SetIniString('Freedom Force', 'TextureRootPath', FFDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[10] then begin
        SetIniString('Freedom Force vs. the 3rd Reich', 'MeshRootPath', FF3RDataDirPage.Values[0], iniFile);
        SetIniString('Freedom Force vs. the 3rd Reich', 'TextureRootPath', FF3RDataDirPage.Values[1], iniFile);
      end
      if UsagePage.Values[11] then begin
        SetIniString('Star Trek: Bridge Commander', 'MeshRootPath', BCDataDirPage.Values[0], iniFile);
        SetIniString('Star Trek: Bridge Commander', 'TextureRootPath', BCDataDirPage.Values[1], iniFile);
      end;
  end;
end;
