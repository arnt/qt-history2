
AtlTestps.dll: dlldata.obj AtlTest_p.obj AtlTest_i.obj
	link /dll /out:AtlTestps.dll /def:AtlTestps.def /entry:DllMain dlldata.obj AtlTest_p.obj AtlTest_i.obj kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib 

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL $<

clean:
	@del AtlTestps.dll
	@del AtlTestps.lib
	@del AtlTestps.exp
	@del dlldata.obj
	@del AtlTest_p.obj
	@del AtlTest_i.obj
