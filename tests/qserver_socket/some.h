#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

class Some : public QVBox
{
    Q_OBJECT
public:
    Some();
    ~Some();

public slots:
    void quit();
    void startServer();
    void startClient();

signals:
    void quitted();

private:
    QLineEdit *hostEdit;
    QLineEdit *portEdit;
};
