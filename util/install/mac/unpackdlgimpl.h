#include "unpackdlg.h"

class UnpackDlgImpl : public UnpackDlg
{
    Q_OBJECT;
public:
    UnpackDlgImpl( QString key = QString::null, 
		   QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );

    virtual void clickedDestButton();
    virtual void clickedUnpack();
    virtual bool copyFile( const QString& src, const QString& dest );
public slots:
    virtual void updateProgress( const QString& );
    virtual void updateProgress( int );
    virtual void licenseKeyChanged();
    virtual void reject();
};
