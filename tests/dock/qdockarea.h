#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#include <qwidget.h>
#include <qlist.h>

class QDockWidgetData;
class QDockWidget;

class QDockArea : public QWidget
{
    Q_OBJECT

public:
    QDockArea( Orientation o, QWidget *parent = 0, const char *name = 0 );

    void moveDockWidget( QDockWidget *w /*some other args will come here*/ );
    void removeDockWidget( QDockWidget *w, bool makeFloating );

    Orientation orientation() const { return orient; }
    
private:
    int findDockWidget( QDockWidget *w );
    
private:
    Orientation orient;
    QList<QDockWidgetData> dockWidgets;
    
};

class QDockWidgetData
{
    friend class QDockArea;
private:
    QDockWidget *dockWidget;
};
#endif

