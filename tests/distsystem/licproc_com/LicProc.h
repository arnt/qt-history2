// LicProc.h : Declaration of the LicProc

#ifndef __LICPROC_H_
#define __LICPROC_H_

#include "resource.h"       // main symbols

class QSqlDatabase;
/////////////////////////////////////////////////////////////////////////////
// LicProc
class ATL_NO_VTABLE LicProc : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<LicProc, &CLSID_LicProc>,
    public IDispatchImpl<ILicProc, &IID_ILicProc, &LIBID_LICPROC_COMLib>
{
public:
    LicProc()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_LICPROC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(LicProc)
    COM_INTERFACE_ENTRY(ILicProc)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

private:
    QSqlDatabase* distDb;
// ILicProc
public:
    STDMETHOD(updateDb)(DWORD licenseId, BSTR versionTag, BSTR companyId);
    STDMETHOD(setServer)(BSTR srv, BSTR user, BSTR password, BSTR db);
    STDMETHOD(publishLine)(DWORD licenseId, BSTR itemId, DWORD usLicense, BSTR companyId);
    STDMETHOD(publishLicense)(DWORD licenseId, BSTR custId, BSTR licensee, BSTR licenseeEmail, BSTR login, BSTR password, BSTR expiryDate, BSTR companyId);
    STDMETHOD(dummy)(DWORD bar);
};

#endif //__LICPROC_H_
