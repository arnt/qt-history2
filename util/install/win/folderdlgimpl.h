#include "folderdlg.h"

class WinShell;

class FolderDlgImpl : public FolderDlg
{
    Q_OBJECT
public:
    FolderDlgImpl( QWidget* parent = NULL, const char* name = NULL, bool modal = false, WFlags f = 0 );

    void setup( QString, QString );

    virtual void expandedDir( QListViewItem* );
    virtual void collapsedDir( QListViewItem* );
    virtual void selectedDir( QListViewItem* );

    QString getFolderName();
private:
    void ScanFolder( QString folderPath, QListViewItem* parent );
};
