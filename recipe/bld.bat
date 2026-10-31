rem this old code-base assume there is a libdeflate.lib file to be found.
rem it was renamed by upstream venture to deflate.lib, which we fix here.
copy/b %LIBRARY_LIB%\deflate.lib %LIBRARY_LIB%\libdeflate.lib
if errorlevel 1 exit 1

set OPENJPEG=2.5

%PYTHON% -m pip install . --no-deps --no-build-isolation -vv
if errorlevel 1 exit 1

rem TEMPORARY (remove before merge): LZW decode_size repro on win-arm64.
rem Output is prefixed LZW_REPRO: and never fails the build.
if "%target_platform%"=="win-arm64" call :lzw_repro

rem remove created file from PREFIX, so that this copy doesn't end up being
rem part of the resulting package

del %LIBRARY_LIB%\libdeflate.lib
exit /b 0

:lzw_repro
setlocal
set "LZW_DIR=%SRC_DIR%\lzw_repro_tmp"
mkdir "%LZW_DIR%"
copy /y "%RECIPE_DIR%\lzw_repro\t.c" "%LZW_DIR%\" >nul
copy /y "%SRC_DIR%\imagecodecs\imcd.c" "%LZW_DIR%\" >nul
copy /y "%SRC_DIR%\imagecodecs\imcd.h" "%LZW_DIR%\" >nul
pushd "%LZW_DIR%"
echo LZW_REPRO: compiler banner
cl 2>&1 | findstr /i "Version"
call :lzw_variant O2_GL "/O2 /GL"
call :lzw_variant O2 "/O2"
call :lzw_variant O1 "/O1"
call :lzw_variant Od "/Od"
popd
endlocal
exit /b 0

:lzw_variant
echo LZW_REPRO: ===== variant %~1 [%~2] =====
cl /nologo %~2 /DNDEBUG /MD /c imcd.c /Foimcd_%~1.obj
cl /nologo /Od /MD /c t.c /Fot_%~1.obj
link /nologo t_%~1.obj imcd_%~1.obj /OUT:t_%~1.exe
if exist t_%~1.exe (t_%~1.exe) else (echo LZW_REPRO: build of variant %~1 failed)
exit /b 0
