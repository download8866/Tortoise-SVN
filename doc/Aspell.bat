@echo off
rem parameters
rem %1 = path to spellcheck executable
rem %2 = language
rem %3 = name of the file to check
rem %4 = name of the logfile
echo --- %3 >> %4
%1 --mode=sgml --encoding=utf-8 --add-extra-dicts=./ASpell/%2.pws --add-extra-dicts=./ASpell/Temp.pws --lang=%2 list check < %3 >> %4
