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

#ifndef QDESIGNER_COMMAND_H
#define QDESIGNER_COMMAND_H

#include "shared_global_p.h"
#include "qtundo_p.h"
#include "layoutinfo_p.h"

#include <QtDesigner/layoutdecoration.h>

#include <QtCore/QPointer>
#include <QtCore/QPair>

#include <QtCore/QVariant>

class QDesignerFormEditorInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerFormWindowInterface;
class QDesignerWidgetDataBaseItemInterface;

class QDesignerContainerExtension;
class QDesignerPropertySheetExtension;
class QDesignerMetaDataBaseItemInterface;

class QToolBox;
class QTabWidget;
class QTableWidget;
class QListWidget;
class QComboBox;
class QStackedWidget;
class QDockWidget;
class QMainWindow;

namespace qdesigner_internal {

class QDesignerPromotedWidget;
class Layout;

class QDESIGNER_SHARED_EXPORT QDesignerFormEditorCommand: public QtCommand
{
    Q_OBJECT
public:
    QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core);

    QDesignerFormEditorInterface *core() const;

private:
    QPointer<QDesignerFormEditorInterface> m_core;
};

class QDESIGNER_SHARED_EXPORT QDesignerFormWindowManagerCommand: public QtCommand
{
    Q_OBJECT
public:
    QDesignerFormWindowManagerCommand(const QString &description, QDesignerFormWindowManagerInterface *formWindowManager);

    QDesignerFormWindowManagerInterface *formWindowManager() const;

private:
    QPointer<QDesignerFormWindowManagerInterface> m_formWindowManager;
};

class QDESIGNER_SHARED_EXPORT QDesignerFormWindowCommand: public QtCommand
{
    Q_OBJECT
public:
    QDesignerFormWindowCommand(const QString &description, QDesignerFormWindowInterface *formWindow);

    QDesignerFormWindowInterface *formWindow() const;
    QDesignerFormEditorInterface *core() const;

protected:
    void checkObjectName(QObject *object);
    void updateBuddies(const QString &old_name, const QString &new_name);
    void checkSelection(QWidget *widget);
    void checkParent(QWidget *widget, QWidget *parentWidget);
    bool hasLayout(QWidget *widget) const;

private:
    QPointer<QDesignerFormWindowInterface> m_formWindow;
};

class QDESIGNER_SHARED_EXPORT SetPropertyCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    SetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    void init(QObject *object, const QString &propertyName, const QVariant &newValue);

    QObject *object() const;

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
    QPointer<QObject> m_object;
    QPointer<QWidget> m_parentWidget;
    QDesignerPropertySheetExtension *m_propertySheet;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_changed;
};

class QDESIGNER_SHARED_EXPORT ResetPropertyCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ResetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget, const QString &propertyName);

    QWidget *widget() const;
    QWidget *parentWidget() const;

    inline QString propertyName() const
    { return m_propertyName; }

    inline QVariant oldValue() const
    { return m_oldValue; }

    inline void setOldValue(const QVariant &oldValue)
    { m_oldValue = oldValue; }

    virtual void redo();
    virtual void undo();

protected:
    virtual bool mergeMeWith(QtCommand *other) { Q_UNUSED(other); return false; }

private:
    QString m_propertyName;
    int m_index;
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_parentWidget;
    QDesignerPropertySheetExtension *m_propertySheet;
    QVariant m_oldValue;
    bool m_changed;
};

class QDESIGNER_SHARED_EXPORT InsertWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    InsertWidgetCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    QDesignerLayoutDecorationExtension::InsertMode m_insertMode;
    QPair<int, int> m_cell;
};

class QDESIGNER_SHARED_EXPORT RaiseWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    RaiseWidgetCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
};

class QDESIGNER_SHARED_EXPORT LowerWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    LowerWidgetCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
};

class QDESIGNER_SHARED_EXPORT AdjustWidgetSizeCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    AdjustWidgetSizeCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    QRect m_geometry;
};

class QDESIGNER_SHARED_EXPORT DeleteWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    DeleteWidgetCommand(QDesignerFormWindowInterface *formWindow);

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
    QDesignerMetaDataBaseItemInterface *m_formItem;
    int m_tabOrderIndex;
};

class QDESIGNER_SHARED_EXPORT ReparentWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ReparentWidgetCommand(QDesignerFormWindowInterface *formWindow);

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

class QDESIGNER_SHARED_EXPORT ChangeLayoutItemGeometry: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ChangeLayoutItemGeometry(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget, int row, int column, int rowspan, int colspan);

    virtual void redo();
    virtual void undo();

protected:
    void changeItemPosition(const QRect &g);

private:
    QPointer<QWidget> m_widget;
    QRect m_oldInfo;
    QRect m_newInfo;
};

class QDESIGNER_SHARED_EXPORT InsertRowCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    InsertRowCommand(QDesignerFormWindowInterface *formWindow);

    void init(QWidget *widget, int row);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    int m_row;
};


class QDESIGNER_SHARED_EXPORT TabOrderCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    TabOrderCommand(QDesignerFormWindowInterface *formWindow);

    void init(const QList<QWidget*> &newTabOrder);

    inline QList<QWidget*> oldTabOrder() const
    { return m_oldTabOrder; }

    inline QList<QWidget*> newTabOrder() const
    { return m_newTabOrder; }

    virtual void redo();
    virtual void undo();

private:
    QDesignerMetaDataBaseItemInterface *m_widgetItem;
    QList<QWidget*> m_oldTabOrder;
    QList<QWidget*> m_newTabOrder;
};

class QDESIGNER_SHARED_EXPORT PromoteToCustomWidgetCommand : public QDesignerFormWindowCommand
{
public:
    PromoteToCustomWidgetCommand(QDesignerFormWindowInterface *formWindow);
    void init(QDesignerWidgetDataBaseItemInterface *item, QWidget *widget);
    virtual void redo();
    virtual void undo();
private:
    QWidget *m_widget;
    QDesignerPromotedWidget *m_promoted;
    friend class DemoteFromCustomWidgetCommand;
};

class QDESIGNER_SHARED_EXPORT DemoteFromCustomWidgetCommand : public QDesignerFormWindowCommand
{
public:
    DemoteFromCustomWidgetCommand(QDesignerFormWindowInterface *formWindow);
    void init(QDesignerPromotedWidget *promoted);
    virtual void redo();
    virtual void undo();
private:
    PromoteToCustomWidgetCommand *m_promote_cmd;
};

class QDESIGNER_SHARED_EXPORT LayoutCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    LayoutCommand(QDesignerFormWindowInterface *formWindow);
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

class QDESIGNER_SHARED_EXPORT BreakLayoutCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    BreakLayoutCommand(QDesignerFormWindowInterface *formWindow);
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

class QDESIGNER_SHARED_EXPORT ToolBoxCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ToolBoxCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~ToolBoxCommand();

    void init(QToolBox *toolBox);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QToolBox> m_toolBox;
    QPointer<QWidget> m_widget;
    int m_index;
    QString m_itemText;
    QIcon m_itemIcon;
};

class QDESIGNER_SHARED_EXPORT DeleteToolBoxPageCommand: public ToolBoxCommand
{
    Q_OBJECT
public:
    DeleteToolBoxPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~DeleteToolBoxPageCommand();

    void init(QToolBox *toolBox);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT AddToolBoxPageCommand: public ToolBoxCommand
{
    Q_OBJECT
public:
    enum InsertionMode {
        InsertBefore,
        InsertAfter
    };
    AddToolBoxPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~AddToolBoxPageCommand();

    void init(QToolBox *toolBox);
    void init(QToolBox *toolBox, InsertionMode mode);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT TabWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    TabWidgetCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~TabWidgetCommand();

    void init(QTabWidget *tabWidget);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QTabWidget> m_tabWidget;
    QPointer<QWidget> m_widget;
    int m_index;
    QString m_itemText;
    QIcon m_itemIcon;
};

class QDESIGNER_SHARED_EXPORT DeleteTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    DeleteTabPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~DeleteTabPageCommand();

    void init(QTabWidget *tabWidget);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT AddTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    enum InsertionMode {
        InsertBefore,
        InsertAfter
    };
    AddTabPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~AddTabPageCommand();

    void init(QTabWidget *tabWidget);
    void init(QTabWidget *tabWidget, InsertionMode mode);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT MoveTabPageCommand: public TabWidgetCommand
{
    Q_OBJECT
public:
    MoveTabPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~MoveTabPageCommand();

    void init(QTabWidget *tabWidget, QWidget *page,
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

class QDESIGNER_SHARED_EXPORT StackedWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    StackedWidgetCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~StackedWidgetCommand();

    void init(QStackedWidget *stackedWidget);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QStackedWidget> m_stackedWidget;
    QPointer<QWidget> m_widget;
    int m_index;
};

class QDESIGNER_SHARED_EXPORT DeleteStackedWidgetPageCommand: public StackedWidgetCommand
{
    Q_OBJECT
public:
    DeleteStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~DeleteStackedWidgetPageCommand();

    void init(QStackedWidget *stackedWidget);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT AddStackedWidgetPageCommand: public StackedWidgetCommand
{
    Q_OBJECT
public:
    enum InsertionMode {
        InsertBefore,
        InsertAfter
    };
    AddStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~AddStackedWidgetPageCommand();

    void init(QStackedWidget *stackedWidget);
    void init(QStackedWidget *stackedWidget, InsertionMode mode);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT DockWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    DockWidgetCommand(const QString &description, QDesignerFormWindowInterface *formWindow);
    virtual ~DockWidgetCommand();

    void init(QDockWidget *dockWidget);

protected:
    QPointer<QDockWidget> m_dockWidget;
};

class QDESIGNER_SHARED_EXPORT SetDockWidgetCommand: public DockWidgetCommand
{
    Q_OBJECT
public:
    SetDockWidgetCommand(QDesignerFormWindowInterface *formWindow);

    void init(QDockWidget *dockWidget, QWidget *widget);

    virtual void undo();
    virtual void redo();

private:
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_oldWidget;
};

class QDESIGNER_SHARED_EXPORT AddDockWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    AddDockWidgetCommand(QDesignerFormWindowInterface *formWindow);

    void init(QMainWindow *mainWindow, QDockWidget *dockWidget);

    virtual void undo();
    virtual void redo();

private:
    QPointer<QMainWindow> m_mainWindow;
    QPointer<QDockWidget> m_dockWidget;
};

class QDESIGNER_SHARED_EXPORT ContainerWidgetCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ContainerWidgetCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~ContainerWidgetCommand();

    QDesignerContainerExtension *containerExtension() const;

    void init(QWidget *containerWidget);

    virtual void removePage();
    virtual void addPage();

protected:
    QPointer<QWidget> m_containerWidget;
    QPointer<QWidget> m_widget;
    int m_index;
};

class QDESIGNER_SHARED_EXPORT DeleteContainerWidgetPageCommand: public ContainerWidgetCommand
{
    Q_OBJECT
public:
    DeleteContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~DeleteContainerWidgetPageCommand();

    void init(QWidget *containerWidget);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT AddContainerWidgetPageCommand: public ContainerWidgetCommand
{
    Q_OBJECT
public:
    enum InsertionMode {
        InsertBefore,
        InsertAfter
    };
    AddContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~AddContainerWidgetPageCommand();

    void init(QWidget *containerWidget);
    void init(QWidget *containerWidget, InsertionMode mode);

    virtual void redo();
    virtual void undo();
};

class QDESIGNER_SHARED_EXPORT ChangeTableContentsCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ChangeTableContentsCommand(QDesignerFormWindowInterface *formWindow);

    void init(QTableWidget *tableWidget, QTableWidget *fromTableWidget);
    virtual void redo();
    virtual void undo();
private:
    void changeContents(QTableWidget *tableWidget,
        int rowCount, int columnCount,
        const QMap<int, QPair<QString, QIcon> > &horizontalHeaderState,
        const QMap<int, QPair<QString, QIcon> > &verticalHeaderState,
        const QMap<QPair<int, int>, QPair<QString, QIcon> > &itemsState) const;

    QPointer<QTableWidget> m_tableWidget;
    QMap<QPair<int, int>, QPair<QString, QIcon> > m_oldItemsState;
    QMap<QPair<int, int>, QPair<QString, QIcon> > m_newItemsState;
    QMap<int, QPair<QString, QIcon> > m_oldHorizontalHeaderState;
    QMap<int, QPair<QString, QIcon> > m_newHorizontalHeaderState;
    QMap<int, QPair<QString, QIcon> > m_oldVerticalHeaderState;
    QMap<int, QPair<QString, QIcon> > m_newVerticalHeaderState;
    int m_oldColumnCount;
    int m_newColumnCount;
    int m_oldRowCount;
    int m_newRowCount;
};

class QT_SHARED_EXPORT ChangeListContentsCommand: public QDesignerFormWindowCommand
{
    Q_OBJECT
public:
    ChangeListContentsCommand(QDesignerFormWindowInterface *formWindow);

    void init(QListWidget *listWidget, const QList<QPair<QString, QIcon> > &items);
    void init(QComboBox *comboBox, const QList<QPair<QString, QIcon> > &items);
    virtual void redo();
    virtual void undo();
private:
    void changeContents(QListWidget *listWidget,
        const QList<QPair<QString, QIcon> > &itemsState) const;
    void changeContents(QComboBox *comboBox,
        const QList<QPair<QString, QIcon> > &itemsState) const;

    QPointer<QListWidget> m_listWidget;
    QPointer<QComboBox> m_comboBox;
    QList<QPair<QString, QIcon> > m_oldItemsState;
    QList<QPair<QString, QIcon> > m_newItemsState;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_COMMAND_H
