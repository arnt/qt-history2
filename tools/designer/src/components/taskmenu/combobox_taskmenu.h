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

#ifndef COMBOBOX_TASKMENU_H
#define COMBOBOX_TASKMENU_H

#include <QtGui/QComboBox>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ComboBoxTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit ComboBoxTaskMenu(QComboBox *button, QObject *parent = 0);
    virtual ~ComboBoxTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editItems();
    void updateSelection();

private:
    QComboBox *m_comboBox;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

class ComboBoxTaskMenuFactory : public ExtensionFactory<QDesignerTaskMenuExtension, QComboBox, ComboBoxTaskMenu>
{
public:
    explicit ComboBoxTaskMenuFactory(const QString &iid, QExtensionManager *extensionManager);

private:
    virtual QComboBox *checkObject(QObject *qObject) const;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // COMBOBOX_TASKMENU_H
