
#include "qdesigner_customwidget.h"

QDesignerCustomWidget::QDesignerCustomWidget(FormWindow *formWindow, QWidget *parent)
    : QDesignerWidget(formWindow, parent),
      m_widgetClassName(QLatin1String("QWidget")),
      m_compat(false)
{
}

QDesignerCustomWidget::~QDesignerCustomWidget()
{
}

QString QDesignerCustomWidget::widgetClassName() const
{
    return m_widgetClassName;
}

void QDesignerCustomWidget::setWidgetClassName(const QString &widgetClassName)
{
    m_widgetClassName = widgetClassName;
}

bool QDesignerCustomWidget::isCompat() const
{
    return m_compat;
}

void QDesignerCustomWidget::setCompat(bool compat)
{
    m_compat = compat;
    update();
}
