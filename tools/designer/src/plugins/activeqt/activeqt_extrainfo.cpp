/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "activeqt_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include "ui4_p.h"

QAxWidgetExtraInfo::QAxWidgetExtraInfo(QActiveXPluginObject *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *QAxWidgetExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *QAxWidgetExtraInfo::core() const
{ return m_core; }

bool QAxWidgetExtraInfo::saveUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool QAxWidgetExtraInfo::loadUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool QAxWidgetExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);

    foreach (DomProperty *p, ui_widget->elementProperty())
        p->setAttributeStdset(false);

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

    if (QActiveXPluginObject *w = qobject_cast<QActiveXPluginObject*>(object))
        return new QAxWidgetExtraInfo(w, m_core, parent);

    return 0;
}
