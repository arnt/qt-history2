#include <qcheckbox.h>
#include <qactiveqt.h>

class QAxWidget1 : public QWidget, public QActiveQt
{
    Q_OBJECT
public:
    QAxWidget1( QWidget *parent = 0, const char *name = 0, WFlags f = 0 )
	: QWidget( parent, name, f )
    {
    }
};

QT_ACTIVEX( QAxWidget1, 
	  "{1D9928BD-4453-4bdd-903D-E525ED17FDE5}",
	  "{99F6860E-2C5A-42ec-87F2-43396F4BE389}",
	  "{0A3E9F27-E4F1-45bb-9E47-63099BCCD0E3}",
	  "{98DE28B6-6CD3-4e08-B9FA-3D1DB43F1D2F}",
	  "{05828915-AD1C-47ab-AB96-D6AD1E25F0E2}" )
