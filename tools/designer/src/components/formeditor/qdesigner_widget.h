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

#ifndef QDESIGNER_WIDGET_H
#define QDESIGNER_WIDGET_H

#include "formeditor_global.h"
#include "layoutdecoration.h"

#include <abstractmetadatabase.h>

#include <QGridLayout>
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPointer>
#include <qpair.h>

class AbstractFormWindow;
class QAction;
class QLayoutItem;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

class QT_FORMEDITOR_EXPORT QDesignerWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerWidget(AbstractFormWindow* formWindow, QWidget *parent = 0);
    virtual ~QDesignerWidget();

    inline AbstractFormWindow* formWindow() const
    { return m_formWindow; }

    void updatePixmap();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);

private:
    AbstractFormWindow* m_formWindow;
    uint need_frame : 1;
    QPixmap grid;
};

class QT_FORMEDITOR_EXPORT QLayoutSupport: public QObject
{
    Q_OBJECT
public:
    QLayoutSupport(AbstractFormWindow *formWindow, QWidget *widget, QObject *parent = 0);
    virtual ~QLayoutSupport();

    inline QWidget *widget() const
    { return m_widget; }

    inline QLayout *layout() const
    { return widget()->layout(); }

    inline QGridLayout *gridLayout() const
    { return qt_cast<QGridLayout*>(layout()); }

    inline AbstractFormWindow *formWindow() const
    { return m_formWindow; }

    AbstractFormEditor *core() const;

    inline int currentIndex() const
    { return m_currentIndex; }

    inline ILayoutDecoration::InsertMode currentInsertMode() const
    { return m_currentInsertMode; }

    inline QPair<int, int> currentCell() const
    { return m_currentCell; }

    int findItemAt(const QPoint &pos) const;
    QRect itemInfo(int index) const;
    int indexOf(QWidget *widget) const;
    int indexOf(QLayoutItem *item) const;

    void adjustIndicator(const QPoint &pos, int index);

    void insertWidget(QWidget *widget, const QPair<int, int> &cell);
    void removeWidget(QWidget *widget);

    QList<QWidget*> widgets(QLayout *layout);

    void simplifyLayout();

    void insertRow(int row);
    void insertColumn(int column);

    void removeRow(int row);
    void removeColumn(int column);

    QRect extendedGeometry(int index) const;

//
// QGridLayout helpers
//
    int findItemAt(int row, int column) const;

    static int findItemAt(QGridLayout *, int row, int column);
    static void createEmptyCells(QGridLayout *&gridLayout);

    void computeGridLayout(QHash<QLayoutItem*, QRect> *layout);
    void rebuildGridLayout(QHash<QLayoutItem*, QRect> *layout);
    void insertWidget(int index, QWidget *widget);

    inline bool isEmptyItem(QLayoutItem *item) const
    { return item->spacerItem() != 0; }

protected:
    void tryRemoveRow(int row);
    void tryRemoveColumn(int column);

private:
    AbstractFormWindow *m_formWindow;
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_indicatorLeft;
    QPointer<QWidget> m_indicatorTop;
    QPointer<QWidget> m_indicatorRight;
    QPointer<QWidget> m_indicatorBottom;
    int m_currentIndex;
    ILayoutDecoration::InsertMode m_currentInsertMode;
    QPair<int, int> m_currentCell;
};

class QT_FORMEDITOR_EXPORT QLayoutWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int margin READ layoutMargin WRITE setLayoutMargin DESIGNABLE true)
    Q_PROPERTY(int spacing READ layoutSpacing WRITE setLayoutSpacing DESIGNABLE true)
public:
    QLayoutWidget(AbstractFormWindow *formWindow, QWidget *parent = 0);

    int layoutMargin() const;
    void setLayoutMargin(int layoutMargin);

    int layoutSpacing() const;
    void setLayoutSpacing(int layoutSpacing);

    inline AbstractFormWindow *formWindow() const
    { return m_formWindow; }

    inline QLayoutSupport *support() const
    { return const_cast<QLayoutSupport*>(&m_support); }

    inline int findItemAt(const QPoint &pos) const
    { return m_support.findItemAt(pos); }

    inline QRect itemInfo(int index) const
    { return m_support.itemInfo(index); }

    inline void adjustIndicator(const QPoint &pos, int index)
    { m_support.adjustIndicator(pos, index); }

    inline void insertWidget(QWidget *widget, const QPair<int, int> &cell)
    { m_support.insertWidget(widget, cell); }

    inline void removeWidget(QWidget *widget)
    { m_support.removeWidget(widget); }

protected:
    virtual bool event(QEvent *e);
    virtual void paintEvent(QPaintEvent *e);

    void updateMargin();

    inline QList<QWidget*> widgets(QLayout *layout)
    { return m_support.widgets(layout); }

    inline void insertRow(int row)
    { m_support.insertRow(row); }

    inline void insertColumn(int column)
    { m_support.insertColumn(column); }

    inline void computeGridLayout(QHash<QLayoutItem*, QRect> *layout)
    { m_support.computeGridLayout(layout); }

    inline void rebuildGridLayout(QHash<QLayoutItem*, QRect> *layout)
    { m_support.rebuildGridLayout(layout); }

private:
    AbstractFormWindow *m_formWindow;
    QLayoutSupport m_support;
};

class QLayoutWidgetItem: public QWidgetItem
{
public:
    QLayoutWidgetItem(QWidget *widget): QWidgetItem(widget) {}

    virtual void setGeometry(const QRect &r)
    { widget()->setGeometry(r); }

    virtual QSize sizeHint() const
    { return theLayout()->sizeHint(); }

    virtual QSize minimumSize() const
    {return theLayout()->minimumSize(); }

    virtual QSize maximumSize() const
    { return theLayout()->maximumSize(); }

    virtual QSizePolicy::ExpandData expanding() const
    { return theLayout()->expanding(); }

    void addTo(QLayout *layout);
    void removeFrom(QLayout *layout);

protected:
    inline QLayoutWidgetItem *me() const
    { return const_cast<QLayoutWidgetItem*>(this); }

    inline QLayout *theLayout() const
    { Q_ASSERT(me()->widget()); return me()->widget()->layout(); }
};

class QT_FORMEDITOR_EXPORT QDesignerDialog : public QDialog
{
    Q_OBJECT
public:
    QDesignerDialog(AbstractFormWindow *fw, QWidget *parent)
        : QDialog(parent), m_formWindow(fw) {}

protected:
    void paintEvent(QPaintEvent *e);

private:
    AbstractFormWindow *m_formWindow;
};

class QT_FORMEDITOR_EXPORT QDesignerLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString buddy READ buddyWidget WRITE setBuddyWidget)
public:
    QDesignerLabel(QWidget *parent = 0)
        : QLabel(parent) {}

    inline void setBuddyWidget(const QString &b)
    {
        myBuddy = b;
        updateBuddy();
    }

    inline QString buddyWidget() const
    { return myBuddy; }

protected:
    void showEvent(QShowEvent *e)
    {
        QLabel::showEvent(e);
        updateBuddy();
    }

private:
    void updateBuddy();

    QString myBuddy;
};

class QT_FORMEDITOR_EXPORT Line : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_OVERRIDE(int frameWidth DESIGNABLE false)
    Q_OVERRIDE(Shape frameShape DESIGNABLE false)
    Q_OVERRIDE(QRect frameRect DESIGNABLE false)
    Q_OVERRIDE(QRect contentsRect DESIGNABLE false)
public:
    Line(QWidget *parent) : QFrame(parent, Qt::WMouseNoMask)
    { setFrameStyle(HLine | Sunken); }

    inline void setOrientation(Qt::Orientation orient)
    { setFrameShape(orient == Qt::Horizontal ? HLine : VLine); }

    inline Qt::Orientation orientation() const
    { return frameShape() == HLine ? Qt::Horizontal : Qt::Vertical; }
};

#endif // QDESIGNER_WIDGET_H
