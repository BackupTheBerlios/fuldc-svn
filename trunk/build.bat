@ECHO OFF
rem CONFIG START
rem ----------------------------------------------------
set NET="C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe"
set OPTS=dcplusplus.sln /build release
set CLEAN=dcplusplus.sln /clean release

rem set RAR=C:\Progra~1\Winrar\rar.exe
rem set RAROPS=a -r -idp -inul -m3 ..\fulDC.rar *.*

rem CONFIG END
rem ----------------------------------------------------

ECHO Cleaning Solution...
%NET% %CLEAN%
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
rem ECHO Rarring...
rem del fulDC.rar
rem cd App
rem %RAR% %RAROPS%
ECHO ------------------------------
ECHO finished!
pause