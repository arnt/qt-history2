
QGraphps.dll: dlldata.obj QGraph_p.obj QGraph_i.obj
	link /dll /out:QGraphps.dll /def:QGraphps.def /entry:DllMain dlldata.obj QGraph_p.obj QGraph_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del QGraphps.dll
	@del QGraphps.lib
	@del QGraphps.exp
	@del dlldata.obj
	@del QGraph_p.obj
	@del QGraph_i.obj
