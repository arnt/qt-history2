#include <qvbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qfiledialog.h>
#include <qtextstream.h>

class ControlCentral : public QVBox
{
    Q_OBJECT

public:
    ControlCentral();
    ~ControlCentral();

    QSize sizeHint() const;

signals:

public slots:
    void show( QStringList* files=0 );
    void showSource();
    void showParseProtocol();
    void showErrorProtocol();
    void showTree();
    void incrementalParse();

private:
    void parse( const QString& filename, const QString& incrementalSteps=QString::null );

    QListView* lview;
    QLineEdit* incSteps;
    QPushButton* quit;
    QFile *parseProtocolFile;
    QTextStream *parseProtocolTS;
    QFile *parsePerformanceFile;
    QTextStream *parsePerformanceTS;
};
