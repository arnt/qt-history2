#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qprocess.h>

class QPushButton;

class QDocMainWindow : public QDialog
{
    Q_OBJECT
public:
    QDocMainWindow( const QString &qtdir, QStringList defines,
		    QWidget* parent=0, const char* name=0 );
    ~QDocMainWindow();

public slots:
    void readOutput();
    void activateEditor( QListViewItem* );
    void populateListView();
    void editorFinished();
    void finished();

private slots:
    void changeVersion();
    void find();
    void findNext();
    void timeout();

private:
    void setEditor();
    void updateTitle();

    QVBoxLayout *vb;
    QListView *classList;
    QProcess *proc;
    QProcess *procper;
    QProcess *procedit;
    QString outputText;
    QString qtdirenv;
    QLabel *statusBar;
    QPushButton *findButton;
    QPushButton *redo;
    QPushButton *commercial;
    QPushButton *version;
    QPushButton *stop;
    QString findText;
    int msgCount;
    QString editor;
    int warnings;
    int prevWarnings;
    QStringList _defines;
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

