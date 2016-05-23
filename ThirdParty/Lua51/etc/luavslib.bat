@rem Script to build LuaJIT under "Visual Studio .NET Command Prompt".
@rem Do not run from this directory; run it from the toplevel: etc\luavs.bat .
@rem It creates lua51.dll, lua51.lib and luajit.exe in src.
@rem (contributed by David Manura and Mike Pall)

@setlocal
@set MYCOMPILE=cl  /nologo /MD  /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I . /I ..\dynasm
@set MYCOMPILED=cl /nologo /MDd /Od /W3 /c  /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /D "LUAJIT_ASSERT"  /D "_CRT_SECURE_NO_DEPRECATE" /FD /EHsc /RTC1 /I . /I ..\dynasm
@set MYLINK=lib /nologo

cd src

%MYCOMPILE% l*.c
del lua.obj luac.obj
%MYLINK% /out:..\bin\lua51.lib l*.obj
del *.obj *.manifest

%MYCOMPILED% l*.c
del lua.obj luac.obj
%MYLINK% /out:..\bin\lua51D.lib l*.obj
del *.obj *.manifest

cd ..
