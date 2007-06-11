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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerActionEditorInterface>

#include <QtCore/QPointer>

class QDesignerPropertyEditorInterface;
class QMenu;
class QActionGroup;
class QSignalMapper;
class QItemSelection;
class QListWidget;

namespace qdesigner_internal {

class ActionView;
class ResourceMimeData;

class QDESIGNER_SHARED_EXPORT ActionEditor: public QDesignerActionEditorInterface
{
    Q_OBJECT
public:
    explicit ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ActionEditor();

    QDesignerFormWindowInterface *formWindow() const;
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow);

    virtual QDesignerFormEditorInterface *core() const;

    QAction *actionNew() const;
    QAction *actionDelete() const;

    QString filter() const;

    virtual void manageAction(QAction *action);
    virtual void unmanageAction(QAction *action);

    static QString actionTextToName(const QString &text, const QString &prefix = QLatin1String("action"));

    // ActionView::ViewMode
    int viewMode() const;
    void setViewMode(int lm);

public slots:
    void setFilter(const QString &filter);
    void mainContainerChanged();

private slots:
    void slotCurrentItemChanged(QAction *item);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void editAction(QAction *item);
    void editCurrentAction();
    void slotActionChanged();
    void slotNewAction();
    void slotDelete();
    void slotNotImplemented();
    void resourceImageDropped(const ResourceMimeData &data, QAction *action);
    void slotContextMenuRequested(QContextMenuEvent *, QAction *);
    void slotViewMode(QAction *a);
    void slotSelectAssociatedWidget(QWidget *w);
    void slotCopy();
    void slotCut();
    void slotPaste();

signals:
    void itemActivated(QAction *item);
    // Context menu for item or global menu if item == 0.
    void contextMenuRequested(QMenu *menu, QAction *item);

private:
    typedef QList<QAction *> ActionList;
    void deleteActions(QDesignerFormWindowInterface *formWindow, const ActionList &);
    void copyActions(QDesignerFormWindowInterface *formWindow, const ActionList &);

    void updateViewModeActions();

    QDesignerFormEditorInterface *m_core;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QListWidget *m_actionGroups;

    ActionView *m_actionView;

    QAction *m_actionNew;
    QAction *m_actionEdit;
    QAction *m_actionCopy;
    QAction *m_actionCut;
    QAction *m_actionPaste;
    QAction *m_actionSelectAll;
    QAction *m_actionDelete;

    QActionGroup *m_viewModeGroup;
    QAction *m_iconViewAction;
    QAction *m_listViewAction;

    QString m_filter;
    QWidget *m_filterWidget;
    QSignalMapper *m_selectAssociatedWidgetsMapper;
};

} // namespace qdesigner_internal

#endif // ACTIONEDITOR_H
