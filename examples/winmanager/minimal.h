
#include "qwsmanager.h"

class QPixmap;
class QWidget;
class QRegion;

class QWSMinimalDecorator : public QWSDecorator
{
public:
    QWSMinimalDecorator();
    virtual ~QWSMinimalDecorator();
    
    virtual QRegion region(const QWidget *, const QRect &, QWSManager::Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, QWSManager::Region, int state);

protected:
    QPixmap *closePixmap;
    QPixmap *minimizePixmap;
    QPixmap *maximizePixmap;
    QPixmap *normalizePixmap;
};

