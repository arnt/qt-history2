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

    void moveDockWidget( QDockWidget *w, const QPoint &globalPos, const QRect &rect, bool swap );
    void removeDockWidget( QDockWidget *w, bool makeFloating, bool swap );

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

