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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerActionEditorInterface>

#include <QPointer>

class QDesignerPropertyEditorInterface;
class QListWidget;
class QListWidgetItem;
class QSplitter;

namespace qdesigner_internal {

class ActionRepository;

class QDESIGNER_SHARED_EXPORT ActionEditor: public QDesignerActionEditorInterface
{
    Q_OBJECT
public:
    ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ActionEditor();

    QDesignerFormWindowInterface *formWindow() const;
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow);

    virtual QDesignerFormEditorInterface *core() const;

    QAction *actionNew() const;
    QAction *actionDelete() const;

    QString filter() const;

public slots:
    void setFilter(const QString &filter);

private slots:
    void slotItemChanged(QListWidgetItem *item);
    void slotActionChanged();
    void slotNewAction();
    void slotDeleteAction();
    void slotNotImplemented();

private:
    QListWidgetItem *createListWidgetItem(QAction *action);

private:
    QDesignerFormEditorInterface *m_core;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QSplitter *splitter;
    QListWidget *m_actionGroups;
    ActionRepository *m_actionRepository;
    QAction *m_actionNew;
    QAction *m_actionDelete;
    QString m_filter;
    QWidget *m_filterWidget;
};

} // namespace qdesigner_internal

#endif // ACTIONEDITOR_H
