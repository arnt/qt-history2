#ifndef __LICENSEDLGIMPL_H__
#define __LICENSEDLGIMPL_H__

#include "licensedlg.h"

class LicenseDialogImpl : public LicenseDialog
{
    Q_OBJECT

public:
    LicenseDialogImpl( QWidget* parent = 0 );
    bool showLicense( bool licenseUs );
};

#endif /* __LICENSEDLGIMPL_H__ */
