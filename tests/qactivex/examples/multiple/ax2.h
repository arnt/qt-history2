#include <qradiobutton.h>
#include <qactiveqt.h>

class QAxWidget2 : public QWidget, public QActiveQt
{
    Q_OBJECT
public:
    QAxWidget2( QWidget *parent = 0, const char *name = 0, WFlags f = 0 )
	: QWidget( parent, name, f )
    {
    }
};

QT_ACTIVEX( QAxWidget2, 
	  "{58139D56-6BE9-4b17-937D-1B1EDEDD5B71}",
	  "{B66280AB-08CC-4dcc-924F-58E6D7975B7D}",
	  "{D72BACBA-03C4-4480-B4BB-DE4FE3AA14A0}",
	  "{98DE28B6-6CD3-4e08-B9FA-3D1DB43F1D2F}",
	  "{05828915-AD1C-47ab-AB96-D6AD1E25F0E2}" )
