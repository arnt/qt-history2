#include <qwidget.h>



#include <qlist.h>

class QBoxLayout;

class QButtonRow : public QWidget
{
    Q_OBJECT
public:
    QButtonRow( QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
protected:
    //    QHBox( bool horizontal, QWidget *parent=0, const char *name=0 );
    virtual void childEvent( QChildEvent * );
    virtual void layoutEvent( QEvent * );
private:
    void recalc();
    QBoxLayout *lay;
    QSize prefSize;
    bool first;
public slots:
    void dump();
};



