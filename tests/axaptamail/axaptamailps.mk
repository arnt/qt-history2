
axaptamailps.dll: dlldata.obj axaptamail_p.obj axaptamail_i.obj
	link /dll /out:axaptamailps.dll /def:axaptamailps.def /entry:DllMain dlldata.obj axaptamail_p.obj axaptamail_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del axaptamailps.dll
	@del axaptamailps.lib
	@del axaptamailps.exp
	@del dlldata.obj
	@del axaptamail_p.obj
	@del axaptamail_i.obj
