#include <qpushbutton.h>

class QuickButton : public QPushButton
{
    Q_OBJECT

public:
    QuickButton( QWidget *parent, const char *name=0 );
    QuickButton( const QString &text, QWidget *parent, const char* name=0 );
    QuickButton( const QIconSet& icon, const QString &text, QWidget *parent, const char* name=0 );

signals:
    void rightClick();

protected:
    void mouseReleaseEvent( QMouseEvent * );
};
