
ActiveQtEXEps.dll: dlldata.obj ActiveQtEXE_p.obj ActiveQtEXE_i.obj
	link /dll /out:ActiveQtEXEps.dll /def:ActiveQtEXEps.def /entry:DllMain dlldata.obj ActiveQtEXE_p.obj ActiveQtEXE_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del ActiveQtEXEps.dll
	@del ActiveQtEXEps.lib
	@del ActiveQtEXEps.exp
	@del dlldata.obj
	@del ActiveQtEXE_p.obj
	@del ActiveQtEXE_i.obj
