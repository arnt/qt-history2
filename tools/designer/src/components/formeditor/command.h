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

#ifndef COMMAND_H
#define COMMAND_H

#include <qtundo.h>
#include <ui4.h>
#include <layoutinfo.h>
#include "layoutdecoration.h"

#include <QPointer>
#include <QVariant>
#include <qpair.h>

class FormEditor;
class FormWindowManager;
class FormWindow;
class Layout;
class QToolBox;
class QTabWidget;
class QStackedWidget;

struct IPropertySheet;
struct AbstractMetaDataBaseItem;

class FormEditorCommand: public QtCommand
{
    Q_OBJECT
public:
    FormEditorCommand(const QString &description, FormEditor *core);

    FormEditor *core() const;

private:
    QPointer<FormEditor> m_core;
};

class FormWindowManagerCommand: public QtCommand
{
    Q_OBJECT
public:
    FormWindowManagerCommand(const QString &description, FormWindowManager *formWindowManager);

    FormWindowManager *formWindowManager() const;

private:
    QPointer<FormWindowManager> m_formWindowManager;
};

class FormWindowCommand: public QtCommand
{
    Q_OBJECT
public:
    FormWindowCommand(const QString &description, FormWindow *formWindow);

    FormWindow *formWindow() const;

protected:
    void checkObjectName(QWidget *widget);
    void checkSelection(QWidget *widget);
    void checkParent(QWidget *widget, QWidget *parentWidget);
    bool hasLayout(QWidget *widget) const;

private:
    QPointer<FormWindow> m_formWindow;
};

class SetPropertyCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    SetPropertyCommand(FormWindow *formWindow);

    void init(QWidget *widget, const QString &propertyName, const QVariant &newValue);

    QWidget *widget() const;
    QWidget *parentWidget() const;

    inline QString propertyName() const
    { return m_propertyName; }

    inline QVariant oldValue() const
    { return m_oldValue; }

    inline void setOldValue(const QVariant &oldValue)
    { m_oldValue = oldValue; }

    inline QVariant newValue() const
    { return m_newValue; }

    inline void setNewValue(const QVariant &newValue)
    { m_newValue = newValue; }

    virtual void redo();
    virtual void undo();

protected:
    virtual bool mergeMeWith(QtCommand *other);

private:
    QString m_propertyName;
    int m_index;
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_parentWidget;
    IPropertySheet *m_propertySheet;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_changed;
};

class InsertWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    InsertWidgetCommand(FormWindow *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    ILayoutDecoration::InsertMode m_insertMode;
    QPair<int, int> m_cell;
};

class RaiseWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    RaiseWidgetCommand(FormWindow *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
};

class LowerWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    LowerWidgetCommand(FormWindow *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
};

class DeleteWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    DeleteWidgetCommand(FormWindow *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_parentWidget;
    QRect m_geometry;
    LayoutInfo::Type m_layoutType;
    int m_index;
    int m_row, m_col;
    int m_rowspan, m_colspan;
};

class ReparentWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    ReparentWidgetCommand(FormWindow *formWindow);

    void init(QWidget *widget, QWidget *parentWidget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    QPoint m_oldPos;
    QPoint m_newPos;
    QPointer<QWidget> m_oldParentWidget;
    QPointer<QWidget> m_newParentWidget;
};

class TabOrderCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    TabOrderCommand(FormWindow *formWindow);

    void init(const QList<QWidget*> &newTabOrder);

    inline QList<QWidget*> oldTabOrder() const
    { return m_oldTabOrder; }

    inline QList<QWidget*> newTabOrder() const
    { return m_newTabOrder; }

    virtual void redo();
    virtual void undo();

private:
    AbstractMetaDataBaseItem *m_widgetItem;
    QList<QWidget*> m_oldTabOrder;
    QList<QWidget*> m_newTabOrder;
};

class LayoutCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    LayoutCommand(FormWindow *formWindow);
    virtual ~LayoutCommand();

    inline QList<QWidget*> widgets() const
    { return m_widgets; }

    void init(QWidget *parentWidget, const QList<QWidget*> &widgets, LayoutInfo::Type layoutType,
        QWidget *layoutBase = 0, bool splitter = false);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_parentWidget;
    QList<QWidget*> m_widgets;
    QPointer<QWidget> m_layoutBase;
    QPointer<Layout> m_layout;
};

class BreakLayoutCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    BreakLayoutCommand(FormWindow *formWindow);
    virtual ~BreakLayoutCommand();

    inline QList<QWidget*> widgets() const
    { return m_widgets; }

    void init(const QList<QWidget*> &widgets, QWidget *layoutBase);

    virtual void redo();
    virtual void undo();

private:
    QList<QWidget*> m_widgets;
    QPointer<QWidget> m_layoutBase;
    QPointer<Layout> m_layout;
    int m_margin;
    int m_spacing;
};

class ToolBoxCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    ToolBoxCommand(FormWindow *formWindow);
    virtual ~ToolBoxCommand();

    virtual void init(QToolBox *toolBox);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QToolBox> m_toolBox;
    QPointer<QWidget> m_widget;
    int m_index;
    QString m_itemText;
    QIcon m_itemIcon;
};

class DeleteToolBoxPageCommand: public ToolBoxCommand
{
    Q_OBJECT
public:
    DeleteToolBoxPageCommand(FormWindow *formWindow);
    virtual ~DeleteToolBoxPageCommand();

    virtual void init(QToolBox *toolBox);

    virtual void redo();
    virtual void undo();
};

class AddToolBoxPageCommand: public ToolBoxCommand
{
    Q_OBJECT
public:
    AddToolBoxPageCommand(FormWindow *formWindow);
    virtual ~AddToolBoxPageCommand();

    virtual void init(QToolBox *toolBox);

    virtual void redo();
    virtual void undo();
};

class TabWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    TabWidgetCommand(FormWindow *formWindow);
    virtual ~TabWidgetCommand();

    virtual void init(QTabWidget *tabWidget);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QTabWidget> m_tabWidget;
    QPointer<QWidget> m_widget;
    int m_index;
    QString m_itemText;
    QIcon m_itemIcon;
};

class DeleteTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    DeleteTabPageCommand(FormWindow *formWindow);
    virtual ~DeleteTabPageCommand();

    virtual void init(QTabWidget *tabWidget);

    virtual void redo();
    virtual void undo();
};

class AddTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    AddTabPageCommand(FormWindow *formWindow);
    virtual ~AddTabPageCommand();

    virtual void init(QTabWidget *tabWidget);

    virtual void redo();
    virtual void undo();
};

class MoveTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    MoveTabPageCommand(FormWindow *formWindow);
    virtual ~MoveTabPageCommand();

    virtual void init(QTabWidget *tabWidget, QWidget *page,
                      const QIcon &icon, const QString &label,
                      int index, int newIndex);

    virtual void redo();
    virtual void undo();

private:
    int m_newIndex;
    int m_oldIndex;
    QPointer<QWidget> m_page;
    QString m_label;
    QIcon m_icon;
};

class StackedWidgetCommand: public FormWindowCommand
{
    Q_OBJECT
public:
    StackedWidgetCommand(FormWindow *formWindow);
    virtual ~StackedWidgetCommand();

    virtual void init(QStackedWidget *stackedWidget);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QStackedWidget> m_stackedWidget;
    QPointer<QWidget> m_widget;
    int m_index;
};

class DeleteStackedWidgetPageCommand: public StackedWidgetCommand
{
    Q_OBJECT
public:
    DeleteStackedWidgetPageCommand(FormWindow *formWindow);
    virtual ~DeleteStackedWidgetPageCommand();

    virtual void init(QStackedWidget *stackedWidget);

    virtual void redo();
    virtual void undo();
};

class AddStackedWidgetPageCommand: public StackedWidgetCommand
{
    Q_OBJECT
public:
    AddStackedWidgetPageCommand(FormWindow *formWindow);
    virtual ~AddStackedWidgetPageCommand();

    virtual void init(QStackedWidget *stackedWidget);

    virtual void redo();
    virtual void undo();
};

#endif // COMMAND_H
