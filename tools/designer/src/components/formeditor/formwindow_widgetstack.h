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

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include "formeditor_global.h"

#include <QtGui/QWidget>

class AbstractFormWindowTool;

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QWidget
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QWidget *parent = 0);
    virtual ~FormWindowWidgetStack();

    int count() const;
    AbstractFormWindowTool *tool(int index) const;
    AbstractFormWindowTool *currentTool() const;
    int currentIndex() const;
    int indexOf(AbstractFormWindowTool *tool) const;

signals:
    void currentToolChanged(int index);

public slots:
    void addTool(AbstractFormWindowTool *tool);
    void setCurrentTool(AbstractFormWindowTool *tool);
    void setCurrentTool(int index);
    void setSenderAsCurrentTool();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    QList<AbstractFormWindowTool*> m_tools;
    int m_current_index;
};

#endif // FORMWINDOW_WIDGETSTACK_H
