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

#ifndef ABSTRACTFORMWINDOWMANAGER_H
#define ABSTRACTFORMWINDOWMANAGER_H

#include <QtDesigner/sdk_global.h>
#include <QtDesigner/abstractformwindow.h>

#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QAction;
class QActionGroup;
class QDesignerFormEditorInterface;
class DomUI;
class QWidget;
class QDesignerDnDItemInterface;

class QDesignerFormWindowManagerInterfacePrivate;

class QDESIGNER_SDK_EXPORT QDesignerFormWindowManagerInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerFormWindowManagerInterface(QObject *parent = 0);
    virtual ~QDesignerFormWindowManagerInterface();

    virtual QAction *actionCut() const;
    virtual QAction *actionCopy() const;
    virtual QAction *actionPaste() const;
    virtual QAction *actionDelete() const;
    virtual QAction *actionSelectAll() const;
    virtual QAction *actionLower() const;
    virtual QAction *actionRaise() const;
    virtual QAction *actionUndo() const;
    virtual QAction *actionRedo() const;

    virtual QAction *actionHorizontalLayout() const;
    virtual QAction *actionVerticalLayout() const;
    virtual QAction *actionSplitHorizontal() const;
    virtual QAction *actionSplitVertical() const;
    virtual QAction *actionGridLayout() const;
    virtual QAction *actionBreakLayout() const;
    virtual QAction *actionAdjustSize() const;
    QAction *actionSimplifyLayout() const;

    virtual QDesignerFormWindowInterface *activeFormWindow() const;

    virtual int formWindowCount() const;
    virtual QDesignerFormWindowInterface *formWindow(int index) const;

    virtual QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = 0, Qt::WindowFlags flags = 0);

    virtual QDesignerFormEditorInterface *core() const;

    virtual void dragItems(const QList<QDesignerDnDItemInterface*> &item_list) = 0;

Q_SIGNALS:
    void formWindowAdded(QDesignerFormWindowInterface *formWindow);
    void formWindowRemoved(QDesignerFormWindowInterface *formWindow);
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

public Q_SLOTS:
    virtual void addFormWindow(QDesignerFormWindowInterface *formWindow);
    virtual void removeFormWindow(QDesignerFormWindowInterface *formWindow);
    virtual void setActiveFormWindow(QDesignerFormWindowInterface *formWindow);

protected:
    void setActionSimplifyLayout(QAction *action);

private:
    Q_DECLARE_PRIVATE(QDesignerFormWindowManagerInterface)

    QDesignerFormWindowManagerInterface(const QDesignerFormWindowManagerInterface &other);
    QDesignerFormWindowManagerInterface &operator = (const QDesignerFormWindowManagerInterface &other);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTFORMWINDOWMANAGER_H
