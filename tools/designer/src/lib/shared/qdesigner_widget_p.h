/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#ifndef QDESIGNER_WIDGET_H
#define QDESIGNER_WIDGET_H

#include "shared_global_p.h"
#include <QtGui/QDialog>
#include <QtGui/QLabel>

class QDesignerFormWindowInterface;
class QPaintEvent;

namespace qdesigner_internal {

// Paint the designer grid on a top-level widget.
QDESIGNER_SHARED_EXPORT void paintGrid(QWidget *widget, QDesignerFormWindowInterface *formWindow, QPaintEvent *e, bool needFrame = false);
QDESIGNER_SHARED_EXPORT void paintGrid(QWidget *widget, const QPoint &grid, QPaintEvent *e, bool needFrame = false);

}


class QDESIGNER_SHARED_EXPORT QDesignerWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent = 0);
    virtual ~QDesignerWidget();

    inline QDesignerFormWindowInterface* formWindow() const
    { return m_formWindow; }

    void updatePixmap();

    virtual QSize minimumSizeHint() const
    { return QWidget::minimumSizeHint().expandedTo(QSize(16, 16)); }

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);

private:
    QDesignerFormWindowInterface* m_formWindow;
};

class QDESIGNER_SHARED_EXPORT QDesignerDialog : public QDialog
{
    Q_OBJECT
public:
    QDesignerDialog(QDesignerFormWindowInterface *fw, QWidget *parent)
        : QDialog(parent), m_formWindow(fw) {}

    virtual QSize minimumSizeHint() const
    { return QWidget::minimumSizeHint().expandedTo(QSize(16, 16)); }

protected:
    void paintEvent(QPaintEvent *e);

private:
    QDesignerFormWindowInterface *m_formWindow;
};

class QDESIGNER_SHARED_EXPORT QDesignerLabel : public QLabel
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

class QDESIGNER_SHARED_EXPORT Line : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
public:
    Line(QWidget *parent) : QFrame(parent)
    { setAttribute(Qt::WA_MouseNoMask); setFrameStyle(HLine | Sunken); }

    inline void setOrientation(Qt::Orientation orient)
    { setFrameShape(orient == Qt::Horizontal ? HLine : VLine); }

    inline Qt::Orientation orientation() const
    { return frameShape() == HLine ? Qt::Horizontal : Qt::Vertical; }
};

#endif // QDESIGNER_WIDGET_H
