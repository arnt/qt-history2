#include "unpackdlg.h"

class UnpackDlgImpl : public UnpackDlg
{
    Q_OBJECT;
public:
    UnpackDlgImpl( QString key = QString::null, 
		   QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );

    virtual void clickedDestButton();
    virtual void clickedUnpack();
public slots:
    virtual void updateProgress( const QString& );
};
