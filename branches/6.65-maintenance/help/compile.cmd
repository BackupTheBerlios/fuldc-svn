@echo off
REM generate changelog.html
if exist "gen_changelog.py" goto generate
cd ..\help

:generate
gen_changelog.py

REM  this isn't pretty. - Todd
if exist "%ProgramFiles%\HTML Help Workshop\hhc.exe" goto compile
if exist "%ProgramFiles% (x86)\HTML Help Workshop\hhc.exe" goto compile64
echo.
echo HTML Help Workshop not detected.  Please install it from:
echo http://msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp
echo.
echo or.. enter the correct path in help/compile.cmd
echo.
exit 1

:compile
copy ..\res\users.bmp .
"%ProgramFiles%\HTML Help Workshop\hhc.exe" DCPlusPlus.hhp
if errorlevel 1 goto okay

:compile64
copy ..\res\users.bmp .
"%ProgramFiles% (x86)\HTML Help Workshop\hhc.exe" DCPlusPlus.hhp
if errorlevel 1 goto okay

:bad
del users.bmp
exit 1

:okay
del users.bmp
exit 0
