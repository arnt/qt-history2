#include <qobject.h>
#include <qdialog.h>

class Dynamic : public QObject
{
    Q_OBJECT
public:
    void setParent( QDialog *parent );
public slots:
    void setAmount();
private:
    QDialog *p;
};
