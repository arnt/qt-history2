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

#include "default_layoutdecoration.h"
#include "formwindow.h"
#include "qdesigner_widget.h"

#include <QLayout>
#include <qdebug.h>

// ---- QDesignerLayoutDecoration ----
QDesignerLayoutDecoration::QDesignerLayoutDecoration(QLayoutWidget *widget, QObject *parent)
    : QObject(parent), 
      m_layoutWidget(widget), 
      m_layoutSupport(0)
{
}

QDesignerLayoutDecoration::QDesignerLayoutDecoration(FormWindow *formWindow, QWidget *widget, QObject *parent)
    : QObject(parent),
      m_layoutWidget(0),
      m_layoutSupport(new QLayoutSupport(formWindow, widget, this))
{
}
    
int QDesignerLayoutDecoration::findItemAt(const QPoint &pos) const
{
    if (m_layoutWidget)
        return m_layoutWidget->findItemAt(pos);
        
    return m_layoutSupport->findItemAt(pos);
}

void QDesignerLayoutDecoration::adjustIndicator(const QPoint &pos, int index)
{
    if (m_layoutWidget)
        m_layoutWidget->adjustIndicator(pos, index);
    else
        m_layoutSupport->adjustIndicator(pos, index);
}

// ---- QDesignerLayoutDecorationFactory ----
QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(ILayoutDecoration))
        return 0;
        
    if (QLayoutWidget *widget = qt_cast<QLayoutWidget*>(object)) {
        return new QDesignerLayoutDecoration(widget, parent);
    } else if (QWidget *widget = static_cast<QWidget*>(object)) {
        if (FormWindow *fw = FormWindow::findFormWindow(widget)) {
            AbstractMetaDataBaseItem *item = fw->core()->metaDataBase()->item(widget->layout());
            return item ? new QDesignerLayoutDecoration(fw, widget, parent) : 0;
        }
    } 
    
    return 0;
}
