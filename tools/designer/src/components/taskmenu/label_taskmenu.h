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

#ifndef LABEL_TASKMENU_H
#define LABEL_TASKMENU_H

#include <QtGui/QLabel>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class InPlaceEditor;

class LabelTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    LabelTaskMenu(QLabel *button, QObject *parent = 0);
    virtual ~LabelTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editRichText();
    void editPlainText();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();
private:
    QLabel *m_label;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<InPlaceEditor> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editRichTextAction;
    QAction *m_editPlainTextAction;
};

class LabelTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    LabelTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // LABEL_TASKMENU_H
