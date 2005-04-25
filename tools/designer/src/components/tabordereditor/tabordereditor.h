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

#ifndef TABORDEREDITOR_H
#define TABORDEREDITOR_H

#include "tabordereditor_global.h"

#include <QtCore/QPointer>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>
#include <QtGui/QRegion>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

class QtUndoStack;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QT_TABORDEREDITOR_EXPORT TabOrderEditor : public QWidget
{
    Q_OBJECT

public:
    TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const;

public slots:
    void setBackground(QWidget *background);
    void updateBackground();
    void widgetRemoved(QWidget*);
    void initTabOrder();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    QRect indicatorRect(int index) const;
    int widgetIndexAt(const QPoint &pos) const;
    bool skipWidget(QWidget *w) const;

    QPointer<QDesignerFormWindowInterface> m_form_window;

    QList<QWidget*> m_tab_order_list;

    QWidget *m_bg_widget;
    QtUndoStack *m_undo_stack;
    QPixmap m_bg_pixmap;
    QRegion m_indicator_region;

    QFontMetrics m_font_metrics;
    int m_current_index;
};

}  // namespace qdesigner_internal

#endif
