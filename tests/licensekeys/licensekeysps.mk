
licensekeysps.dll: dlldata.obj licensekeys_p.obj licensekeys_i.obj
	link /dll /out:licensekeysps.dll /def:licensekeysps.def /entry:DllMain dlldata.obj licensekeys_p.obj licensekeys_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del licensekeysps.dll
	@del licensekeysps.lib
	@del licensekeysps.exp
	@del dlldata.obj
	@del licensekeys_p.obj
	@del licensekeys_i.obj
