#include "generatordlg.h"

class GeneratorDlgImpl : public GeneratorDlg
{
    Q_OBJECT
public:
    GeneratorDlgImpl( const QString &dest, QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );

    virtual void clickedSourceButton();
    virtual void clickedDestButton();
    virtual void clickedGenerate();
public slots:
    virtual void updateProgress( const QString& );
};
