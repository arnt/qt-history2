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
#include <abstractmetadatabase.h>

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPointer>

class FormWindow;
class QAction;
class QLayoutItem;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

class QT_FORMEDITOR_EXPORT QDesignerWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerWidget(FormWindow* formWindow, QWidget *parent = 0);
    virtual ~QDesignerWidget();

    void updatePixmap();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);

private:
    FormWindow* m_formWindow;
    uint need_frame : 1;
    QPixmap grid;
};

class QT_FORMEDITOR_EXPORT QLayoutSupport: public QObject
{
    Q_OBJECT
public:
    QLayoutSupport(FormWindow *formWindow, QWidget *widget, QObject *parent = 0);
    virtual ~QLayoutSupport();
          
    inline QWidget *widget() const
    { return m_widget; }
    
    inline QLayout *layout() const
    { return widget()->layout(); }
    
    inline FormWindow *formWindow() const
    { return m_formWindow; }

    int findItemAt(const QPoint &pos) const;    
    QRect itemInfo(int index) const;
    int indexOf(QWidget *widget) const;

    void adjustIndicator(const QPoint &pos, int index);
    
    void insertWidget(QWidget *widget);
    void removeWidget(QWidget *widget);

    QList<QWidget*> widgets(QLayout *layout);
    
    void insertRow(QGridLayout *gridLayout, int row);
    void insertColumn(QGridLayout *gridLayout, int column);
    
    static void createEmptyCells(QGridLayout *gridLayout);
    void computeGridLayout(QGridLayout *gridLayout, QHash<QLayoutItem*, QRect> *layout);
    void rebuildGridLayout(QGridLayout *gridLayout, const QHash<QLayoutItem*, QRect> &layout);
    
private:
    FormWindow *m_formWindow;
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_indicatorLeft;
    QPointer<QWidget> m_indicatorTop;
    QPointer<QWidget> m_indicatorRight;
    QPointer<QWidget> m_indicatorBottom;
};

class QT_FORMEDITOR_EXPORT QLayoutWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int margin READ layoutMargin WRITE setLayoutMargin DESIGNABLE true)
    Q_PROPERTY(int spacing READ layoutSpacing WRITE setLayoutSpacing DESIGNABLE true)
public:
    QLayoutWidget(FormWindow *formWindow, QWidget *parent = 0);
          
    QSizePolicy sizePolicy() const;
    void updateSizePolicy();

    int layoutMargin() const;
    void setLayoutMargin(int layoutMargin);

    int layoutSpacing() const;
    void setLayoutSpacing(int layoutSpacing);

    inline FormWindow *formWindow() const
    { return m_formWindow; }
    
    inline int findItemAt(const QPoint &pos) const
    { return m_support.findItemAt(pos); }
    
    inline QRect itemInfo(int index) const
    { return m_support.itemInfo(index); }

    inline void adjustIndicator(const QPoint &pos, int index)
    { m_support.adjustIndicator(pos, index); }
    
    inline void insertWidget(QWidget *widget)
    { m_support.insertWidget(widget); }
    
    inline void removeWidget(QWidget *widget)
    { m_support.removeWidget(widget); }

protected:
    virtual bool event(QEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    
    void updateMargin();
    
    inline QList<QWidget*> widgets(QLayout *layout)
    { return m_support.widgets(layout); }
    
    inline void insertRow(QGridLayout *gridLayout, int row)
    { m_support.insertRow(gridLayout, row); }
    
    inline void insertColumn(QGridLayout *gridLayout, int column)
    { m_support.insertColumn(gridLayout, column); }
    
    inline void computeGridLayout(QGridLayout *gridLayout, QHash<QLayoutItem*, QRect> *layout)
    { m_support.computeGridLayout(gridLayout, layout); }
    
    inline void rebuildGridLayout(QGridLayout *gridLayout, const QHash<QLayoutItem*, QRect> &layout)
    { m_support.rebuildGridLayout(gridLayout, layout); }
    
private:
    FormWindow *m_formWindow;
    QSizePolicy sp;    
    QLayoutSupport m_support;
};

class QT_FORMEDITOR_EXPORT QDesignerDialog : public QDialog
{
    Q_OBJECT
public:
    QDesignerDialog(FormWindow *fw, QWidget *parent)
        : QDialog(parent), m_formWindow(fw) {}

protected:
    void paintEvent(QPaintEvent *e);

private:
    FormWindow *m_formWindow;
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
