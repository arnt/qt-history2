// LicProc.h : Declaration of the LicProc

#ifndef __LICPROC_H_
#define __LICPROC_H_

#include "resource.h"       // main symbols

#include "qapplication.h"
#include "qstring.h"

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
	int i( 0 );

	app = new QApplication( i, NULL );
    }

DECLARE_REGISTRY_RESOURCEID(IDR_LICPROC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(LicProc)
    COM_INTERFACE_ENTRY(ILicProc)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

private:
    QApplication* app;
    QString srvName;
// ILicProc
public:
	STDMETHOD(deleteLicense)(BSTR licenseId, BSTR companyId);
    STDMETHOD(deleteFilemap)(BSTR tag, BSTR itemId, BSTR fileName, BSTR companyId);
    STDMETHOD(deleteVersionTag)(BSTR tag, BSTR companyId);
    STDMETHOD(clearFilemap)(BSTR companyId);
    STDMETHOD(clearVersionTags)(BSTR companyId);
    STDMETHOD(publishFilemap)(BSTR tag, BSTR itemId, BSTR fileName, BSTR fileDesc, BSTR companyId);
    STDMETHOD(publishVersionTag)(BSTR tag, BSTR versionString, BSTR subDir, BSTR companyId);
    STDMETHOD(updateDb)(DWORD licenseId, BSTR versionTag, BSTR companyId);
    STDMETHOD(setServer)(BSTR srv, BSTR user, BSTR password, BSTR db);
    STDMETHOD(publishLine)(DWORD licenseId, BSTR itemId, DWORD usLicense, BSTR companyId);
    STDMETHOD(publishLicense)(DWORD licenseId, BSTR custId, BSTR licensee, BSTR licenseeEmail, BSTR login, BSTR password, BSTR expiryDate, BSTR companyId);
    STDMETHOD(dummy)(DWORD bar);
};

#endif //__LICPROC_H_
