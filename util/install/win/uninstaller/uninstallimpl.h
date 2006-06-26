#include "uninstall.h"

class UninstallDlgImpl : public UninstallDlg
{
    Q_OBJECT
public:
    UninstallDlgImpl( QWidget* parent = 0, const char* name = 0, bool modal = false, WindowFlags fl = 0 );
    ~UninstallDlgImpl();
public slots:
    void cleanRegistry();
private:
    void cleanRegistryHelper( const QString& key );
    
};
