#include <qprocess.h>
#include <qmainwindow.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qlabel.h>

class QDocMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    QDocMainWindow( QWidget* parent=0, const char* name=0 );
    ~QDocMainWindow();
    void init();
public slots:
    void readOutput();
    void activateEditor( QListViewItem* );
    void finished();
    void saveFile();
private:
    QListView* classList;
    QMultiLineEdit* fileText;
    QProcess* proc;
    QProcess* procper;
    QString outputText;
    QString qtdirenv;
    QString filename;
    QLabel* waitText;
};

