#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#include <qwidget.h>
#include <qlist.h>

class QDockWidget;
class QSplitter;
class QBoxLayout;

struct QDockWidgetData
{
    QDockWidget *dockWidget;
    int section;
    int offset;
};

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
    void setupLayout();
    void setupHorizontalLayout();
    void setupVerticalLayout();

private:
    Orientation orient;
    QList<QDockWidgetData> dockWidgets;
    QList<QWidget> insertedSplitters;
    QBoxLayout *layout;
    int sections;

};

#endif

