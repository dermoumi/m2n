@ECHO OFF
for /R extlibs\libs-%1 %%f in (*.dll) do xcopy /Q /C /R /Y /s "%%f" bin > NUL