#include "setupwizard.h"
#include "installthread.h"

class SetupWizardImpl : public SetupWizard
{
    Q_OBJECT;
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0 );

    virtual void clickedPath();
    virtual void clickedSystem( int );
    virtual void licenseAccepted();
    virtual void showPage( QWidget* );

protected:
    virtual void customEvent( QCustomEvent* );
private:
    int sysID;
    QByteArray tmpPath;

    InstallThread installer;
};