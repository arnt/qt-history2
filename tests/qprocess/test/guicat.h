#include <qvbox.h>
#include <qcstring.h>

class GuiCat : public QVBox
{
    Q_OBJECT

public:
    GuiCat( QWidget *p=0 );
    ~GuiCat() { };

public slots:
    void readOneByte();
    void readMuch();

    void writeStdout();
    void writeStderr();

    void closeStdin();
    void closeStdout();
    void closeStderr();

private:
    QByteArray buf;
};
