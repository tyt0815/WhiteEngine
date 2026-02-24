@echo off
set /p opt="Enter format (default BC7_UNORM): " || set opt=BC7_UNORM

echo Starting conversion...
echo.

for %%f in (*.jpg) do (
    echo Converting: %%f
    texconv.exe "%%f" -f %opt% -y
)

echo.
echo Conversion Complete!
pause