#include "qdesigner_promotedwidget.h"

#include <qextensionmanager.h>

#include <QEvent>
#include <QVBoxLayout>

#include <qdebug.h>

PromotedWidgetPropertySheet::PromotedWidgetPropertySheet(QDesignerPromotedWidget *promoted,
                                QExtensionManager *extension_manager, QObject *parent)
    : QObject(parent)
{
    m_promoted = promoted;
    QWidget *child = promoted->child();
    m_sheet = qt_extension<IPropertySheet*>(extension_manager, child);
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

void PromotedWidgetPropertySheet::reset(int index)
{
    m_sheet->reset(index);
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
    QVariant result;

    QString name = propertyName(index);
    if (name == QLatin1String("geometry")) {
        result = m_promoted->geometry();
    } else {
        result = m_sheet->property(index);
    }
    
    return result;
}

void PromotedWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    QString name = propertyName(index);
    if (name == QLatin1String("geometry")) {
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
    : DefaultExtensionFactory(parent)
{
}

QObject *PromotedWidgetPropertySheetFactory::createExtension(QObject *object,
                                            const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(IPropertySheet))
        return 0;
    QDesignerPromotedWidget *promoted = qt_cast<QDesignerPromotedWidget*>(object);
    if (promoted == 0)
        return 0;
    return new PromotedWidgetPropertySheet(promoted,
                                qt_cast<QExtensionManager*>(this->parent()),
                                parent);
}

QDesignerPromotedWidget::QDesignerPromotedWidget(AbstractWidgetDataBaseItem *item,
                                                    QWidget *child,
                                                    QWidget *parent)
    : QWidget(parent)
{
    (new QVBoxLayout(this))->setMargin(0);

    m_child = child;
    m_item = item;
    m_custom_class_name = item->name().toLatin1();

    setSizePolicy(m_child->sizePolicy());
    m_child->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
}

QDesignerPromotedWidget::~QDesignerPromotedWidget()
{
}

void QDesignerPromotedWidget::childEvent(QChildEvent *e)
{
    if (e->child() != m_child)
        return;

    switch (e->type()) {
        case QEvent::ChildAdded:
            layout()->addWidget(m_child);
            break;
        case QEvent::ChildRemoved:
            layout()->removeWidget(m_child);
            break;
        default:
            break;
    }
}

QSize QDesignerPromotedWidget::sizeHint() const
{
    return m_child->sizeHint();
}

QSize QDesignerPromotedWidget::minimumSizeHint() const
{
    return m_child->minimumSizeHint();
}

