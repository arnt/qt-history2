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
    void editorFinished();
    void finished();
private:
    QListView* classList;
    QProcess* proc;
    QProcess* procper;
    QProcess* procedit;
    QString outputText;
    QString qtdirenv;
    QString filename;
    QLabel* waitText;
    QString editText;
};

