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

#ifndef LABEL_TASKMENU_H
#define LABEL_TASKMENU_H

#include <QtGui/QLabel>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu.h>
#include <default_extensionfactory.h>

class QLineEdit;
class AbstractFormWindow;

class LabelTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    LabelTaskMenu(QLabel *button, QObject *parent = 0);
    virtual ~LabelTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editText();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();

private:
    QLabel *m_label;
    QPointer<AbstractFormWindow> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editTextAction;
};

class LabelTaskMenuFactory: public DefaultExtensionFactory
{
    Q_OBJECT
public:
    LabelTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // LABEL_TASKMENU_H
