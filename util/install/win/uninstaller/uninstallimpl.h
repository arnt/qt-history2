#include "uninstall.h"

class UninstallDlgImpl : public UninstallDlg
{
    Q_OBJECT
public:
    UninstallDlgImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UninstallDlgImpl();
public slots:
    void cleanRegistry();
private:
    void cleanRegistryHelper( const QString& key );
    
};