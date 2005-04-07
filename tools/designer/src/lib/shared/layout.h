/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LAYOUT_H
#define LAYOUT_H

#include "shared_global.h"

#include <layoutinfo.h>

#include <QtCore/QPointer>
#include <QtCore/QObject>
#include <QtCore/QMap>

#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QWidget>

class AbstractFormWindow;

void add_to_box_layout(QBoxLayout *box, QWidget *widget);
void insert_into_box_layout(QBoxLayout *box, int index, QWidget *widget);
void add_to_grid_layout(QGridLayout *grid, QWidget *widget, int r, int c, int rs, int cs, Qt::Alignment align = 0);

class QT_SHARED_EXPORT Layout : public QObject
{
    Q_OBJECT
public:
    Layout(const QList<QWidget*> &wl, QWidget *p, AbstractFormWindow *fw, QWidget *lb, bool splitter = false);
    virtual ~Layout();

    int margin() const;
    int spacing() const;

    virtual void setup();

    virtual void doLayout() = 0;
    virtual void undoLayout();
    virtual void breakLayout();
    virtual bool prepareLayout(bool &needMove, bool &needReparent);
    virtual void finishLayout(bool needMove, QLayout *layout);

protected:
    QList<QWidget*> widgets;
    QWidget *m_parentWidget;
    QPoint startPoint;
    QMap<QPointer<QWidget>, QRect> geometries;
    QWidget *layoutBase;
    AbstractFormWindow *formWindow;
    QRect oldGeometry;
    bool isBreak;
    bool useSplitter;


protected slots:
    void widgetDestroyed();

};

class QT_SHARED_EXPORT HorizontalLayout : public Layout
{
public:
    HorizontalLayout(const QList<QWidget*> &wl, QWidget *p, AbstractFormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();
};

class QT_SHARED_EXPORT VerticalLayout : public Layout
{
public:
    VerticalLayout(const QList<QWidget*> &wl, QWidget *p, AbstractFormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();
};

class QT_SHARED_EXPORT StackedLayout : public Layout
{
public:
    StackedLayout(const QList<QWidget*> &wl, QWidget *p, AbstractFormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();
};


class Grid;

class QT_SHARED_EXPORT GridLayout : public Layout
{
public:
    GridLayout(const QList<QWidget*> &wl, QWidget *p, AbstractFormWindow *fw, QWidget *lb, const QSize &res);
    ~GridLayout();

    void doLayout();

protected:
    void setup();
    QWidget *widgetAt(QGridLayout *layout, int row, int column) const;

protected:
    void buildGrid();
    QSize resolution;
    Grid* grid;

};

class QT_SHARED_EXPORT WidgetVerticalSorter
{
public:
    bool operator()(const QWidget *a, const QWidget *b) const
    { return a->y() < b->y(); }
};

class QT_SHARED_EXPORT WidgetHorizontalSorter
{
public:
    bool operator()(const QWidget *a, const QWidget *b) const
    { return a->x() < b->x(); }
};

class VerticalLayoutList: public QList<QWidget*>
{
public:
    VerticalLayoutList(const QList<QWidget*> &l)
        : QList<QWidget*>(l) {}

    static bool lessThan(const QWidget *a, const QWidget *b)
    {  return a->y() < b->y(); }

    void sort()
    { qSort(this->begin(), this->end(), WidgetVerticalSorter()); }
};

class HorizontalLayoutList : public QList<QWidget*>
{
public:
    HorizontalLayoutList(const QList<QWidget*> &l)
        : QList<QWidget*>(l) {}

    static bool hLessThan(const QWidget *a, const QWidget *b)
    { return a->x() < b->x(); }

    void sort()
    { qSort(this->begin(), this->end(), WidgetHorizontalSorter()); }
};

namespace Utils // ### fix the namespace
{

inline int indexOfWidget(QLayout *layout, QWidget *widget)
{
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        if (item->widget() == widget)
            return index;

        ++index;
    }

    return -1;
}

} // namespace Utils

#endif // LAYOUT_H
