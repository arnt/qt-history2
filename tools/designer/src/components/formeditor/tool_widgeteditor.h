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

#ifndef TOOL_WIDGETEDITOR_H
#define TOOL_WIDGETEDITOR_H

#include <abstractformwindowtool.h>

#include <QtGui/qevent.h>

class FormWindow;

class ToolWidgetEditor: public AbstractFormWindowTool
{
    Q_OBJECT
public:
    ToolWidgetEditor(FormWindow *formWindow);
    virtual ~ToolWidgetEditor();

    virtual AbstractFormEditor *core() const;
    virtual AbstractFormWindow *formWindow() const;
    virtual QWidget *createEditor() const;

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    bool handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e);
    bool handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
    bool handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
    bool handlePaintEvent(QWidget *widget, QWidget *managedWidget, QPaintEvent *e);

private:
    FormWindow *m_formWindow;
};

#endif // TOOL_WIDGETEDITOR_H
