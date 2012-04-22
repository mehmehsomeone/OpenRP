@ECHO OFF

rem ****************
rem VARIABLE DEFINES
rem ****************

set PK3DLL=OpenRP
set PK3ASSETS=OpenRP_stuff
set ASSETSFOLDER=OpenRP

rem ***************
rem START OF SCRIPT 
rem ***************

ECHO.
ECHO ===========
ECHO Making Pk3s
ECHO ===========

ECHO Y | DEL %PK3DLL%
ECHO Y | DEL %PK3ASSETS%
7za.exe a -tzip %ASSETSFOLDER%\%PK3ASSETS%.pk3 .\%ASSETSFOLDER%\* -xr!.screenshots\ -xr!.svn\ -x!*.dll -x!*.so -x!.\%ASSETSFOLDER%\*.* -xr!*.nav -mx9
IF ERRORLEVEL 1 GOTO ERROR
7za.exe a -tzip %ASSETSFOLDER%\%PK3DLL%.pk3 .\%ASSETSFOLDER%\*.dll -mx9

IF ERRORLEVEL 1 GOTO ERROR
ECHO.
ECHO ====================
ECHO Finished Making Pk3s
ECHO ====================
GOTO END


:ERROR
ECHO ================================================
ECHO Encountered Error While Zipping PK3S!  ABORTING!
ECHO ================================================
GOTO END


:END


