/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_promotedwidget_p.h"

#include <QtDesigner/QExtensionManager>

#include <QtGui/QVBoxLayout>
#include <QtCore/QVariant>

namespace qdesigner_internal {

PromotedWidgetPropertySheet::PromotedWidgetPropertySheet(QDesignerPromotedWidget *promoted,
                                QExtensionManager *extension_manager, QObject *parent)
    : QObject(parent),
      m_promoted(promoted),
      m_sheet(qt_extension<QDesignerPropertySheetExtension*>(extension_manager, promoted->child()))
{
}

PromotedWidgetPropertySheet::~PromotedWidgetPropertySheet()
{
}

int PromotedWidgetPropertySheet::count() const
{
    return m_sheet->count();
}

int PromotedWidgetPropertySheet::indexOf(const QString &name) const
{
    return m_sheet->indexOf(name);
}

QString PromotedWidgetPropertySheet::propertyName(int index) const
{
    return m_sheet->propertyName(index);
}

QString PromotedWidgetPropertySheet::propertyGroup(int index) const
{
    return m_sheet->propertyGroup(index);
}

void PromotedWidgetPropertySheet::setPropertyGroup(int index, const QString &group)
{
    m_sheet->setPropertyGroup(index, group);
}

bool PromotedWidgetPropertySheet::hasReset(int index) const
{
    return m_sheet->hasReset(index);
}

bool PromotedWidgetPropertySheet::reset(int index)
{
    return m_sheet->reset(index);
}

bool PromotedWidgetPropertySheet::isVisible(int index) const
{
    return m_sheet->isVisible(index);
}

void PromotedWidgetPropertySheet::setVisible(int index, bool b)
{
    m_sheet->setVisible(index, b);
}

bool PromotedWidgetPropertySheet::isAttribute(int index) const
{
    return m_sheet->isAttribute(index);
}

void PromotedWidgetPropertySheet::setAttribute(int index, bool b)
{
    m_sheet->setAttribute(index, b);
}

QVariant PromotedWidgetPropertySheet::property(int index) const
{

    if (propertyName(index) == QLatin1String("geometry")) 
        return m_promoted->geometry();
    return m_sheet->property(index);
}

void PromotedWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    if (propertyName(index) == QLatin1String("geometry")) {
        if (value.type() == QVariant::Rect)
            m_promoted->setGeometry(value.toRect());
    } else {
        m_sheet->setProperty(index, value);
    }
}

bool PromotedWidgetPropertySheet::isChanged(int index) const
{
    return m_sheet->isChanged(index);
}

void PromotedWidgetPropertySheet::setChanged(int index, bool changed)
{
    m_sheet->setChanged(index, changed);
}

PromotedWidgetPropertySheetFactory::PromotedWidgetPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *PromotedWidgetPropertySheetFactory::createExtension(QObject *object,
                                            const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;
    QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(object);
    if (promoted == 0)
        return 0;
    return new PromotedWidgetPropertySheet(promoted,
                                qobject_cast<QExtensionManager*>(this->parent()),
                                parent);
}

QDesignerPromotedWidget::QDesignerPromotedWidget(const QDesignerWidgetDataBaseItemInterface *item, 
                                                 QWidget *parent)
    : QWidget(parent),
      m_item(item),
      m_custom_class_name(item->name().toUtf8()),
      m_child(0)
{
    (new QVBoxLayout(this))->setMargin(0);
}

QDesignerPromotedWidget::~QDesignerPromotedWidget()
{
}


void QDesignerPromotedWidget::setChildWidget(QWidget *widget)
{
    if (m_child != 0) {
        layout()->removeWidget(m_child);
        m_child->setSizePolicy(sizePolicy());
        m_child->setParent(0);
    }

    m_child = widget;

    if (m_child != 0) {
        m_child->setParent(this);
        setSizePolicy(m_child->sizePolicy());
        m_child->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
        layout()->addWidget(m_child);
    }
}

QSize QDesignerPromotedWidget::sizeHint() const
{
    if (m_child) 
        return m_child->sizeHint();
    return QSize();
}

QSize QDesignerPromotedWidget::minimumSizeHint() const
{
    if (m_child) 
        return m_child->minimumSizeHint();
    return QSize();
}
    
} // namespace qdesigner_internal
