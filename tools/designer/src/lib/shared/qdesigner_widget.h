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

#include "shared_global.h"
#include "layoutdecoration.h"

#include <abstractmetadatabase.h>

#include <QtCore/QPointer>
#include <QtCore/QPair>

#include <QtGui/QGridLayout>
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>

class AbstractFormWindow;
class QAction;
class QLayoutItem;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

class QT_SHARED_EXPORT QDesignerWidget : public QWidget
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

class QT_SHARED_EXPORT QDesignerDialog : public QDialog
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

class QT_SHARED_EXPORT QDesignerLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QByteArray buddy READ buddy WRITE setBuddy)
public:
    QDesignerLabel(QWidget *parent = 0);

    inline void setBuddy(const QByteArray &b)
    {
        myBuddy = b;
        updateBuddy();
    }

    inline QByteArray buddy() const
    { return myBuddy; }

    void setBuddy(QWidget *widget);

protected:
    void showEvent(QShowEvent *e)
    {
        QLabel::showEvent(e);
        updateBuddy();
    }

private:
    void updateBuddy();

    QByteArray myBuddy;
};

class QT_SHARED_EXPORT Line : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_OVERRIDE(int frameWidth DESIGNABLE false)
    Q_OVERRIDE(Shape frameShape DESIGNABLE false)
    Q_OVERRIDE(QRect frameRect DESIGNABLE false)
    Q_OVERRIDE(QRect contentsRect DESIGNABLE false)
public:
    Line(QWidget *parent) : QFrame(parent)
    { setAttribute(Qt::WA_MouseNoMask); setFrameStyle(HLine | Sunken); }

    inline void setOrientation(Qt::Orientation orient)
    { setFrameShape(orient == Qt::Horizontal ? HLine : VLine); }

    inline Qt::Orientation orientation() const
    { return frameShape() == HLine ? Qt::Horizontal : Qt::Vertical; }
};

#endif // QDESIGNER_WIDGET_H
