@ECHO OFF
rem CONFIG START
rem ----------------------------------------------------
set NET=C:\Progra~1\Micros~1.NET\Common7\IDE\devenv.exe
set OPTS=dcplusplus.sln /build release
set CLEAN=dcplusplus.sln /clean release

set RAR=C:\Progra~1\Winrar\rar.exe
set RAROPS=a -r -idp -inul -m3 ..\fulDC.rar *.*

rem CONFIG END
rem ----------------------------------------------------

ECHO Cleaning Solution...
%NET% %CLEAN%
rmdir App /S /Q
ECHO ------------------------------
ECHO Compiling Solution...
%NET% %OPTS%
ECHO Done!

ECHO ------------------------------
ECHO copying files...
md App\icons
xcopy /Q /Y res\*.* App\icons\
xcopy /Q /Y help.xml App\
xcopy /Q /Y changelog.txt App\
xcopy /Q /Y example.xml App\

del App\icons\dcplusplus.manifest
del App\dcplusplus.map

ECHO ------------------------------
ECHO Rarring...
del fulDC.rar
cd App
%RAR% %RAROPS%
ECHO ------------------------------
ECHO finished!
pause