#include <qvbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qfiledialog.h>

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

private:
    void parse( const QString& filename );

    QListView* lview;
    QPushButton* quit;
};
