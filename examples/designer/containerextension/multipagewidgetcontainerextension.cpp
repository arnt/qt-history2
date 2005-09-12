#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/ui4.h>

#include "multipagewidgetcontainerextension.h"
#include "multipagewidget.h"

MultiPageWidgetContainerExtension::MultiPageWidgetContainerExtension(MultiPageWidget *widget,
                                                                     QObject *parent)
    :QObject(parent)
{
    myWidget = widget;
}

void MultiPageWidgetContainerExtension::addWidget(QWidget *widget)
{
    myWidget->addPage(widget);
}

int MultiPageWidgetContainerExtension::count() const
{
    return myWidget->count();
}

int MultiPageWidgetContainerExtension::currentIndex() const
{
    return myWidget->currentIndex();
}

void MultiPageWidgetContainerExtension::insertWidget(int index, QWidget *widget)
{
    myWidget->insertPage(index, widget);
}

void MultiPageWidgetContainerExtension::remove(int index)
{
    myWidget->removePage(index);
}

void MultiPageWidgetContainerExtension::setCurrentIndex(int index)
{
    myWidget->setCurrentIndex(index);
}

QWidget* MultiPageWidgetContainerExtension::widget(int index) const
{
    return myWidget->widget(index);
}
