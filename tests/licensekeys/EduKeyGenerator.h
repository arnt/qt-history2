// EduKeyGenerator.h : Declaration of the CEduKeyGenerator

#ifndef __EDUKEYGENERATOR_H_
#define __EDUKEYGENERATOR_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CEduKeyGenerator
class ATL_NO_VTABLE CEduKeyGenerator : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEduKeyGenerator, &CLSID_EduKeyGenerator>,
	public IDispatchImpl<IEduKeyGenerator, &IID_IEduKeyGenerator, &LIBID_LICENSEKEYSLib>
{
public:
	CEduKeyGenerator()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EDUKEYGENERATOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEduKeyGenerator)
	COM_INTERFACE_ENTRY(IEduKeyGenerator)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IEduKeyGenerator
public:
	STDMETHOD(newKey)(BSTR institute,/*[out,retval]*/BSTR* key );
};

#endif //__EDUKEYGENERATOR_H_
