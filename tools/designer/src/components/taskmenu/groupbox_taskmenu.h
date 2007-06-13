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

#ifndef GROUPBOX_TASKMENU_H
#define GROUPBOX_TASKMENU_H

#include <QtGui/QGroupBox>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class InPlaceEditor;

class GroupBoxTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent = 0);
    virtual ~GroupBoxTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editTitle();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();

private:
    QGroupBox *m_groupbox;
    QPointer<InPlaceEditor> m_editor;

    QAction *m_editTitleAction;
    QList<QAction*> m_taskActions;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QGroupBox, GroupBoxTaskMenu>  GroupBoxTaskMenuFactory;
}  // namespace qdesigner_internal

#endif // GROUPBOX_TASKMENU_H
