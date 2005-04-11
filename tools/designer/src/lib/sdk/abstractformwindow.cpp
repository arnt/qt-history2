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

#include "abstractformwindow.h"

#include <QtGui/QTabBar>
#include <QtGui/QSizeGrip>
#include <QtGui/QAbstractButton>
#include <QtGui/QToolBox>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>

QDesignerFormWindowInterface::QDesignerFormWindowInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

QDesignerFormWindowInterface::~QDesignerFormWindowInterface()
{
}

QDesignerFormEditorInterface *QDesignerFormWindowInterface::core() const
{
    return 0;
}

static bool isPassiveInteractor(QWidget *o)
{
    if (qobject_cast<QTabBar*>(o))
        return true;
    else if (qobject_cast<QSizeGrip*>(o))
        return true;
    else if (qobject_cast<QAbstractButton*>(o)
            && (qobject_cast<QTabBar*>(o->parent()) || qobject_cast<QToolBox*>(o->parent())))
        return true;
    else if (qobject_cast<QMenuBar*>(o) && qobject_cast<QMainWindow*>(o->parent()))
        return true;
    else if (qstrcmp(o->metaObject()->className(), "QDockSeparator") == 0)
        return true;
    else if (qstrcmp(o->metaObject()->className(), "QDockWindowSeparator") == 0)
        return true;
    else if (o->objectName() == QLatin1String("designer_wizardstack_button"))
        return true;
    else if (o->objectName().startsWith("__qt__passive_"))
        return true;

    return false;
}

QDesignerFormWindowInterface *QDesignerFormWindowInterface::findFormWindow(QWidget *w)
{
    while (w) {
        if (QDesignerFormWindowInterface *fw = qobject_cast<QDesignerFormWindowInterface*>(w)) {
            return fw;
        } else if (isPassiveInteractor(w)) {
            break;
        } else if (w->isWindow()) {
            break;
        }

        w = w->parentWidget();
    }

    return 0;
}
