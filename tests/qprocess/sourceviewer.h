#include <qvbox.h>
#include <qdir.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextedit.h>

class SourceViewer : public QVBox
{
    Q_OBJECT

public:
    SourceViewer(  const QDir &directory, QWidget *parent=0, const char *name=0, WFlags f=0, bool allowLines=TRUE );
    ~SourceViewer();

signals:
    void closed();

public slots:
    void close();
    void setSource( const QString & file );

private:
    void init( const QDir &directory );

private:
    QDir dir;
    QComboBox *sourceSel;
    QTextEdit *sourceCode;
    QPushButton *closeButton;
};
