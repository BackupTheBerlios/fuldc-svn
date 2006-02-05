@ECHO OFF
rem CONFIG START
rem ----------------------------------------------------

set NET="devenv.exe"
set OPTS=dcplusplus.sln /build release
set CLEAN=dcplusplus.sln /clean release

set RAR="C:\Program Files\Winrar\rar.exe"
set RAROPS=a -r -idp -inul -m3 ..\fulDC.rar *.*
set RARSRCOPS=a -r -idp -inul -m3 -x@exclude.txt fulDC-src.rar *.*

rem CONFIG END
rem ----------------------------------------------------

ECHO Cleaning solution...
%NET% %CLEAN%
ECHO ------------------------------
ECHO Compiling solution in release mode...
%NET% %OPTS%
ECHO Done!

ECHO ------------------------------
ECHO updating Geo ip database...
updateGeoIP.py

ECHO ------------------------------
ECHO copying files...

md Temp
md Temp\icons
md Temp\icons\32bpp
md Temp\icons\24bpp
xcopy /Q /Y res\*.* Temp\icons\
xcopy /Q /Y res\32bpp\*.* Temp\icons\32bpp\
xcopy /Q /Y res\24bpp\*.* Temp\icons\24bpp\
xcopy /Q /Y changelog.txt Temp\
xcopy /Q /Y example.xml Temp\
xcopy /Q /Y App\Dcplusplus.exe Temp\
xcopy /Q /Y App\DCPlusPlus.pdb Temp\
xcopy /Q /Y App\DCPlusPlus.chm Temp\
xcopy /Q /Y GeoIPCountryWhois.csv Temp\
xcopy /Q /Y "C:\Program Files\Microsoft Visual Studio 8\Common7\IDE\dbghelp.dll" Temp\

del Temp\icons\dcplusplus.manifest


ECHO ------------------------------
ECHO packing executable...
del fulDC.rar
cd Temp
%RAR% %RAROPS%
cd ..

ECHO ------------------------------
ECHO Removing temp directory...
rmdir /S /Q Temp\ 

ECHO ------------------------------
ECHO packing source...
del fulDc-src.rar
%RAR% %RARSRCOPS%

ECHO ------------------------------
ECHO finished!
pause