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

class QtUndoStack;
class AbstractFormWindow;

class QT_TABORDEREDITOR_EXPORT TabOrderEditor : public QWidget
{
    Q_OBJECT

public:
    TabOrderEditor(AbstractFormWindow *form, QWidget *parent);

    AbstractFormWindow *formWindow() const;

public slots:
    void setBackground(QWidget *background);
    void updateBackground();
    void widgetRemoved(QWidget*);
    void initTabOrder();

protected:    
    virtual void paintEvent(QPaintEvent *e);

private:
    QPointer<AbstractFormWindow> m_form_window;

    QList<QWidget*> m_tab_order_list;
    
    QWidget *m_bg_widget;
    QtUndoStack *m_undo_stack;
    QPixmap m_bg_pixmap;
};

#endif
