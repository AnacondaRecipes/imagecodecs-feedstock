rem this old code-base assume there is a libdeflate.lib file to be found.
rem it was renamed by upstream venture to deflate.lib, which we fix here.
copy/b %LIBRARY_LIB%\deflate.lib %LIBRARY_LIB%\libdeflate.lib

set openjpeg=2.5

%PYTHON% -m pip install . --no-deps --no-build-isolation -vv

rem remove created file from PREFIX, so that this copy doesn't end up being
rem part of the resulting package

del %LIBRARY_LIB%\libdeflate.lib
