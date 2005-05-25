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

#include "activeqt_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/ui4.h>

#include <Qt3Support/QAxWidget>

QAxWidgetExtraInfo::QAxWidgetExtraInfo(QAxWidget *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *QAxWidgetExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *QAxWidgetExtraInfo::core() const
{ return m_core; }

bool QAxWidgetExtraInfo::saveUiExtraInfo(DomUi *ui)
{ Q_UNUSED(ui); return false; }

bool QAxWidgetExtraInfo::loadUiExtraInfo(DomUi *ui)
{ Q_UNUSED(ui); return false; }

bool QAxWidgetExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);

    foreach (DomProperty *p, ui_widget->elementProperty())
        p->setAttributeStdSet(false);

    return true;
}

bool QAxWidgetExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);
    return false;
}

QAxWidgetExtraInfoFactory::QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *QAxWidgetExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (QAxWidget *w = qobject_cast<QAxWidget*>(object))
        return new QAxWidgetExtraInfo(w, m_core, parent);

    return 0;
}
