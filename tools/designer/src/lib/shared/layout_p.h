/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef LAYOUT_H
#define LAYOUT_H

#include "shared_global_p.h"
#include "layoutinfo_p.h"

#include <QtCore/QPointer>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QHash>

#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QWidget>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

void QDESIGNER_SHARED_EXPORT add_to_box_layout(QBoxLayout *box, QWidget *widget);
void QDESIGNER_SHARED_EXPORT insert_into_box_layout(QBoxLayout *box, int index, QWidget *widget);
void QDESIGNER_SHARED_EXPORT add_to_grid_layout(QGridLayout *grid, QWidget *widget, int r, int c, int rs, int cs, Qt::Alignment align = 0);

class QDESIGNER_SHARED_EXPORT Layout : public QObject
{
    Q_OBJECT
public:
    Layout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter = false);
    virtual ~Layout();

    int margin() const;
    int spacing() const;

    virtual void sort() = 0;
    virtual void doLayout() = 0;

    virtual void setup();
    virtual void undoLayout();
    virtual void breakLayout();
    virtual bool prepareLayout(bool &needMove, bool &needReparent);
    virtual void finishLayout(bool needMove, QLayout *layout);

    QList<QWidget*> widgets() const { return m_widgets; }
    QWidget *parentWidget() const { return m_parentWidget; }
    QWidget *layoutBaseWidget() const { return layoutBase; }

protected:
    QList<QWidget*> m_widgets;
    QWidget *m_parentWidget;
    QPoint startPoint;
    QHash<QWidget *, QRect> geometries;
    QWidget *layoutBase;
    QDesignerFormWindowInterface *formWindow;
    QRect oldGeometry;
    bool isBreak;
    bool useSplitter;


protected slots:
    void widgetDestroyed();

};

class QDESIGNER_SHARED_EXPORT HorizontalLayout : public Layout
{
public:
    HorizontalLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter = false);

    virtual void doLayout();
    virtual void sort();
};

class QDESIGNER_SHARED_EXPORT VerticalLayout : public Layout
{
public:
    VerticalLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter = false);

    virtual void doLayout();
    virtual void sort();
};

class QDESIGNER_SHARED_EXPORT StackedLayout : public Layout
{
public:
    StackedLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter = false);

    virtual void doLayout();
    virtual void sort();
};


class Grid;

class QDESIGNER_SHARED_EXPORT GridLayout : public Layout
{
public:
    GridLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, const QSize &res);
    ~GridLayout();

    virtual void doLayout();
    virtual void sort();

protected:
    QWidget *widgetAt(QGridLayout *layout, int row, int column) const;

protected:
    void buildGrid();
    QSize resolution;
    Grid* grid;

};

class QDESIGNER_SHARED_EXPORT WidgetVerticalSorter
{
public:
    bool operator()(const QWidget *a, const QWidget *b) const
    { return a->y() < b->y(); }
};

class QDESIGNER_SHARED_EXPORT WidgetHorizontalSorter
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

namespace Utils
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

} // namespace qdesigner_internal

#endif // LAYOUT_H
