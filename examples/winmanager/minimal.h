
#include "qwsmanager_qws.h"

class QPixmap;
class QWidget;
class QRegion;

class QWSMinimalDecoration : public QWSDecoration
{
public:
    QWSMinimalDecoration();
    virtual ~QWSMinimalDecoration();
    
    virtual QRegion region(const QWidget *, const QRect &, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

protected:
    QPixmap *closePixmap;
    QPixmap *minimizePixmap;
    QPixmap *maximizePixmap;
    QPixmap *normalizePixmap;
};

