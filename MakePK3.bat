@ECHO OFF

rem ****************
rem VARIABLE DEFINES
rem ****************

set PK3DLL=OpenRP
set PK3CONTENT=OpenRP_Content
set ASSETSFOLDER=OpenRP

rem ***************
rem START OF SCRIPT 
rem ***************

ECHO.
ECHO ===========
ECHO Making Pk3s 
ECHO ===========

ECHO Y | DEL %PK3DLL%.pk3
ECHO Y | DEL %PK3CONTENT%.pk3
7za.exe a -tzip %ASSETSFOLDER%\%PK3CONTENT%.pk3 .\%ASSETSFOLDER%\* -mx9
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