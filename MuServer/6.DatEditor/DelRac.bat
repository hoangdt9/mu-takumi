@echo off
REM Delete Build Data
for /r "%~dp0" %%f in (*.nlp) do del "%%f"
for /r "%~dp0" %%f in (*.dll) do del "%%f"
for /r "%~dp0" %%f in (*.pdb) do del "%%f"

echo Successfully!!!
exit