#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#include <qwidget.h>
#include <qlist.h>
#include "qdockwidget.h"

class QSplitter;
class QBoxLayout;
class QToolLayout;

class QDockArea : public QWidget
{
    Q_OBJECT

public:
    QDockArea( Orientation o, QWidget *parent = 0, const char *name = 0 );
    ~QDockArea();
    
    void moveDockWidget( QDockWidget *w, const QPoint &globalPos, const QRect &rect, bool swap );
    void removeDockWidget( QDockWidget *w, bool makeFloating, bool swap );

    Orientation orientation() const { return orient; }

    bool eventFilter( QObject *, QEvent * );

private:
    int findDockWidget( QDockWidget *w );
    void updateLayout();

private:
    Orientation orient;
    QList<QDockWidget> *dockWidgets;
    QToolLayout *layout;
    
};

#endif

