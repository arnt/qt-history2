// LicenseKeyGenerator.h : Declaration of the CLicenseKeyGenerator

#ifndef __LICENSEKEYGENERATOR_H_
#define __LICENSEKEYGENERATOR_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLicenseKeyGenerator
class ATL_NO_VTABLE CLicenseKeyGenerator : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLicenseKeyGenerator, &CLSID_LicenseKeyGenerator>,
	public IDispatchImpl<ILicenseKeyGenerator, &IID_ILicenseKeyGenerator, &LIBID_LICENSEKEYSLib>
{
public:
	CLicenseKeyGenerator()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LICENSEKEYGENERATOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLicenseKeyGenerator)
	COM_INTERFACE_ENTRY(ILicenseKeyGenerator)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ILicenseKeyGenerator
public:
	STDMETHOD(unlocksWindows)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksUnix)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksMac)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksEmbedded)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksExtra1)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksExtra2)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksEnterprise)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(unlocksUS)(BSTR key,/*[out,retval]*/int* valid);
	STDMETHOD(decodeExpiryDate)(BSTR key,/*[out,retval]*/BSTR* expDate);
	STDMETHOD(newKey)(BSTR keyHome, BSTR expiryDate,int us, int enterprise, int windows,int unix,int embedded, int mac, int extra1, int extra2, /*[out,retval]*/BSTR* key);
	STDMETHOD(reset)(BSTR keyHome);
	STDMETHOD(dummy)(/*[out,retval]*/BSTR* str);
};

#endif //__LICENSEKEYGENERATOR_H_
