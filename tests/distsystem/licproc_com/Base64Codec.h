// Base64Codec.h : Declaration of the CBase64Codec

#ifndef __BASE64CODEC_H_
#define __BASE64CODEC_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBase64Codec
class ATL_NO_VTABLE CBase64Codec : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBase64Codec, &CLSID_Base64Codec>,
	public IDispatchImpl<IBase64Codec, &IID_IBase64Codec, &LIBID_LICPROC_COMLib>
{
public:
	CBase64Codec()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BASE64CODEC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBase64Codec)
	COM_INTERFACE_ENTRY(IBase64Codec)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IBase64Codec
public:
	STDMETHOD(get_DecodedData)(/*[out, retval]*/ BYTE* *pVal);
	STDMETHOD(put_DecodedData)(/*[in]*/ BYTE* newVal);
	STDMETHOD(get_EncodedData)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_EncodedData)(/*[in]*/ BSTR newVal);
	STDMETHOD(Decode)( int numBytes );
	STDMETHOD(Encode)( int numBytes );
};

#endif //__BASE64CODEC_H_
