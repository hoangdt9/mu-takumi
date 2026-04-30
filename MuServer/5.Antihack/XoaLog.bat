color 0A
del *.log /s /q
del *log*.txt /s/q
REM del *Screen*.jpg /s/q
del *2022*.txt /s/q
del *MuError*.dmp /s/q
del *2021*.txt /s/q
del *2020*.txt /s/q
del *PSGG*.txt /s/q
del *.rar /s /q
del Thumbs.db /s/q
RMDIR "LOG" /s/q
for /f "tokens=* delims=" %%i in ('dir /s /b /a:d *.svn') do (
rd /s /q "%%i"
)
exit