/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LAYOUT_H
#define LAYOUT_H

#include "formeditor_global.h"

#include <layoutinfo.h>

#include <QWidget>
#include <QMap>
#include <QPointer>
#include <QObject>
#include <QLayout>
#include <QGridLayout>
#include <QMap>

class FormWindow;

class Layout : public QObject
{
    Q_OBJECT
public:
    Layout(const QList<QWidget*> &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool splitter = false);
    virtual ~Layout();

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
    FormWindow *formWindow;
    QRect oldGeometry;
    bool isBreak;
    bool useSplitter;


protected slots:
    void widgetDestroyed();

};

class HorizontalLayout : public Layout
{
public:
    HorizontalLayout(const QList<QWidget*> &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();

};

class VerticalLayout : public Layout
{
public:
    VerticalLayout(const QList<QWidget*> &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();

};

class StackedLayout : public Layout
{
public:
    StackedLayout(const QList<QWidget*> &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool splitter = false);

    void doLayout();

protected:
    void setup();

};


class Grid;

class GridLayout : public Layout
{
public:
    GridLayout(const QList<QWidget*> &wl, QWidget *p, FormWindow *fw, QWidget *lb, const QSize &res);
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

class VerticalLayoutList: public QList<QWidget*>
{
public:
    VerticalLayoutList(const QList<QWidget*> &l)
        : QList<QWidget*>(l) {}

    static bool lessThan(const QWidget *a, const QWidget *b)
    {  return a->y() < b->y(); }

    void sort()
    { qSort(*this, lessThan); }
};

class HorizontalLayoutList : public QList<QWidget*>
{
public:
    HorizontalLayoutList(const QList<QWidget*> &l)
        : QList<QWidget*>(l) {}

    static bool hLessThan(const QWidget *a, const QWidget *b)
    { return a->x() < b->x(); }

    void sort()
    { qSort(*this, hLessThan); }
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

