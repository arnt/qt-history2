#ifndef QTOOLBAR_P_H
#define QTOOLBAR_P_H

#include <qaction.h>
#include <qframe.h>
#include <qtoolbar.h>

#include <private/qframe_p.h>

/*
    internal class to associate a widget with an action
*/
class QToolBarWidgetAction : public QAction
{
    Q_OBJECT

    QWidget *_widget;

public:
    inline QToolBarWidgetAction(QWidget *widget, QWidget *parent)
        : QAction(parent), _widget(widget)
    {
        setText(_widget->objectName().isEmpty()
                ? _widget->metaObject()->className()
                : _widget->objectName());
    }

    inline QWidget *widget() const
    { return _widget; }
};

struct QToolBarItem {
    QAction *action;
    QWidget *widget;
    uint hidden : 1; // toolbar too small to show this item
};

class QToolBarExtension;
class QToolBarHandle;

class QToolBarPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QToolBar);

public:
    inline QToolBarPrivate()
        : movable(true), allowedAreas(Qt::AllToolBarAreas), area(Qt::ToolBarAreaTop),
          handle(0), extension(0), ignoreActionAddedEvent(false)
    { }

    void init();
    void actionTriggered();
    QToolBarItem createItem(QAction *action);
    int indexOf(QAction *action) const;

    bool movable;
    QSize old_size;
    Qt::ToolBarAreas allowedAreas;
    Qt::ToolBarArea area;
    QToolBarHandle *handle;
    QToolBarExtension *extension;

    QList<QToolBarItem> items;
    bool ignoreActionAddedEvent;
};

static inline QCOORD pick(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Horizontal ? p.x() : p.y(); }

static inline QCOORD pick(Qt::Orientation o, const QSize &s)
{ return o == Qt::Horizontal ? s.width() : s.height(); }

#endif // QTOOLBAR_P_H
