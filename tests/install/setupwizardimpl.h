#include "setupwizard.h"

class SetupWizardImpl : public SetupWizard
{
    Q_OBJECT;
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0 );

    virtual void clickedPath();
    virtual void clickedSystem( int );

public slots:
    virtual void changedPage( const QString& );

private:
    int sysID;
};