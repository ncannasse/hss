:: 1.7
@echo off
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
	goto end

:clean
	del "%TOP%*.n" "%SRC%*.n" "%TAR%" "%TAR:.n=.exe%" 2>nul
	goto end

:end
