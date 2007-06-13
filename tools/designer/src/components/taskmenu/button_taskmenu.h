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

#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <QtGui/QAbstractButton>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class InPlaceEditor;

class ButtonTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit ButtonTaskMenu(QAbstractButton *button, QObject *parent = 0);
    virtual ~ButtonTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editText();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();

private:
    QAbstractButton *m_button;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<InPlaceEditor> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_preferredEditAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QAbstractButton, ButtonTaskMenu>  ButtonTaskMenuFactory;
}  // namespace qdesigner_internal

#endif // BUTTON_TASKMENU_H
