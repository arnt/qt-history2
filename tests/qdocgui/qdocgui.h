#include <qprocess.h>
#include <qmainwindow.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qlabel.h>

class QPushButton;

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
    void populateListView();
    void editorFinished();
    void finished();
private:
    QVBoxLayout* vb;
    QListView* classList;
    QProcess* proc;
    QProcess* procper;
    QProcess* procedit;
    QString outputText;
    QString qtdirenv;
    QLabel* waitText;
    QString editText;
    QPushButton* redo;
};

class QDocListItem : public QListViewItem
{
public:
    QDocListItem( QListViewItem* after, QString text, QString lineNumber );
    ~QDocListItem();
    QString key( int column, bool ascending ) const;
private:
    QString line;
};

