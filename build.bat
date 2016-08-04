@echo on
pushd "%~dp0"
setlocal
IF EXIST "setbuildenv.bat" call setbuildenv.bat

REM using delayed expansion ! instead of % as it just works better with ProgramFiles(x86) env variable
IF EXIST "%ProgramFiles(x86)%" set Program_32=%ProgramFiles(x86)%
IF EXIST "%ProgramFiles(x86)%" set Program_64=%ProgramFiles%
IF NOT EXIST "%ProgramFiles(x86)%" set Program_32=%ProgramFiles%
IF NOT EXIST "%ProgramFiles(x86)%" set Program_64=

if NOT "%ADSK_3DSMAX_SDK_2012%" == "" set MAXINSTALLPATH2012=%ADSK_3DSMAX_SDK_2012%
if NOT "%ADSK_3DSMAX_SDK_2013%" == "" set MAXINSTALLPATH2013=%ADSK_3DSMAX_SDK_2013%
if NOT "%ADSK_3DSMAX_SDK_2014%" == "" set MAXINSTALLPATH2014=%ADSK_3DSMAX_SDK_2014%
if NOT "%ADSK_3DSMAX_SDK_2015%" == "" set MAXINSTALLPATH2015=%ADSK_3DSMAX_SDK_2015%
if NOT "%ADSK_3DSMAX_SDK_2016%" == "" set MAXINSTALLPATH2016=%ADSK_3DSMAX_SDK_2016%

if "%GMAXINSTALLPATH12%"  == "" set GMAXINSTALLPATH12=%SystemDrive%\gmax12
if "%MAXINSTALLPATH40%"   == "" set MAXINSTALLPATH40=%SystemDrive%\3dsmax4
if "%MAXINSTALLPATH42%"   == "" set MAXINSTALLPATH42=%SystemDrive%\3dsmax42
if "%MAXINSTALLPATH50%"   == "" set MAXINSTALLPATH50=%SystemDrive%\3dsmax5
if "%MAXINSTALLPATH60%"   == "" set MAXINSTALLPATH60=%SystemDrive%\3dsmax6
if "%MAXINSTALLPATH70%"   == "" set MAXINSTALLPATH70=%SystemDrive%\3dsmax7
if "%MAXINSTALLPATH80%"   == "" set MAXINSTALLPATH80=%SystemDrive%\3dsmax8
if "%MAXINSTALLPATH90%"   == "" set MAXINSTALLPATH90=%Program_32%\AutoDesk\3ds Max 9
if "%MAXINSTALLPATH2008%" == "" set MAXINSTALLPATH2008=%Program_32%\AutoDesk\3ds Max 2008
if "%MAXINSTALLPATH2009%" == "" set MAXINSTALLPATH2009=%Program_32%\AutoDesk\3ds Max 2009
if "%MAXINSTALLPATH2010%" == "" set MAXINSTALLPATH2010=%Program_32%\AutoDesk\3ds Max 2010
if "%MAXINSTALLPATH2011%" == "" set MAXINSTALLPATH2011=%Program_32%\AutoDesk\Autodesk 3ds Max 2011 SDK
if "%MAXINSTALLPATH2012%" == "" set MAXINSTALLPATH2012=%Program_32%\AutoDesk\3ds Max 2012 SDK
if "%MAXINSTALLPATH2013%" == "" set MAXINSTALLPATH2013=%Program_64%\AutoDesk\3ds Max 2013 SDK
if "%MAXINSTALLPATH2014%" == "" set MAXINSTALLPATH2014=%Program_64%\AutoDesk\3ds Max 2014 SDK
if "%MAXINSTALLPATH2015%" == "" set MAXINSTALLPATH2015=%Program_64%\AutoDesk\3ds Max 2015 SDK
if "%MAXINSTALLPATH2016%" == "" set MAXINSTALLPATH2016=%Program_64%\AutoDesk\3ds Max 2016 SDK
 
REM svn update
set SUPPRESS_BUILD_CONFIG=0
call makeconfig.bat
set SUPPRESS_BUILD_CONFIG=1

IF EXIST "%GMAXINSTALLPATH12%"  msbuild NifPlugins.sln "/p:Configuration=Release - gmax" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH42%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 4.2" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH50%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 5" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH60%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 6" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH70%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 7" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH80%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 8" /p:Platform=Win32
IF EXIST "%MAXINSTALLPATH90%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 9" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 9" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2008%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 2008" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2008" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2009%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 2009" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2009" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2010%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 2010" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2010" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2011%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 2011" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2011" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2012%" (
    msbuild NifPlugins.sln "/p:Configuration=Release - Max 2012" /p:Platform=Win32
    IF EXIST "%Program_64%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2012" /p:Platform=x64
)
IF EXIST "%MAXINSTALLPATH2013%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2013" /p:Platform=x64
IF EXIST "%MAXINSTALLPATH2014%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2014" /p:Platform=x64
IF EXIST "%MAXINSTALLPATH2015%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2015" /p:Platform=x64
IF EXIST "%MAXINSTALLPATH2016%" msbuild NifPlugins.sln "/p:Configuration=Release - Max 2016" /p:Platform=x64

endlocal
popd