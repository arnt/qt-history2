#include <qvbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qfiledialog.h>

class PlayDisplay : public QVBox
{
    Q_OBJECT

public:
    PlayDisplay();
    ~PlayDisplay();

    QSize sizeHint() const;

signals:

public slots:
    void show( const QString& filename );

private:
    QListView* lview;
    QPushButton* quit;
};
