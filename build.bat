:: 2.1
@echo off
setlocal enabledelayedexpansion

set TOP=%~dp0
set SRC=%TOP%hss\
set BIN=%TOP%bin\
set TAR=%BIN%hss.n
IF NOT "%1"=="" goto %1

:all
	IF NOT EXIST "%BIN%" md "%BIN%"
	nekoml "%SRC%Ast.nml" "%SRC%Lexer.nml" "%SRC%Parser.nml" "%SRC%Rules.nml" "%SRC%Utils.nml" "%SRC%Main.nml"
	nekoc -link "%TAR%" "%SRC%Main"
	nekoc -z "%TAR%"
	nekotools boot "%TAR%"
	del "%TOP%*.n" "%SRC%*.n"
	goto monitor

:clean
	del "%TOP%*.n" "%SRC%*.n" "%TAR%" "%TAR:.n=.exe%" 2>nul
	goto end

:monitor
	where msbuild >nul 2>&1

	if NOT %ERRORLEVEL% == 0 (

		if exist "%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (

			for /F "tokens=* USEBACKQ" %%F in (`"%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
				set INSTALLPATH=%%F
			)
			if exist "!INSTALLPATH!\Common7\Tools\VsDevCmd.bat" (
				call "!INSTALLPATH!\Common7\Tools\VsDevCmd.bat" -arch=x64 -app_platform=Desktop -no_logo
			)
		)
	)

	msbuild "monitor\windows\monitor.vcxproj" /p:Configuration=Release /p:Platform=x64

	pause

	goto end
:end
