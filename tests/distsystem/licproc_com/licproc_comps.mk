
licproc_comps.dll: dlldata.obj licproc_com_p.obj licproc_com_i.obj
	link /dll /out:licproc_comps.dll /def:licproc_comps.def /entry:DllMain dlldata.obj licproc_com_p.obj licproc_com_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del licproc_comps.dll
	@del licproc_comps.lib
	@del licproc_comps.exp
	@del dlldata.obj
	@del licproc_com_p.obj
	@del licproc_com_i.obj
