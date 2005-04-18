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

class QDesignerFormWindowToolInterface;

namespace qdesigner { namespace components { namespace formeditor {

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QWidget
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QWidget *parent = 0);
    virtual ~FormWindowWidgetStack();

    int count() const;
    QDesignerFormWindowToolInterface *tool(int index) const;
    QDesignerFormWindowToolInterface *currentTool() const;
    int currentIndex() const;
    int indexOf(QDesignerFormWindowToolInterface *tool) const;

signals:
    void currentToolChanged(int index);

public slots:
    void addTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(int index);
    void setSenderAsCurrentTool();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    QList<QDesignerFormWindowToolInterface*> m_tools;
    int m_current_index;
};

} } } // namespace qdesigner::components::formeditor

#endif // FORMWINDOW_WIDGETSTACK_H
