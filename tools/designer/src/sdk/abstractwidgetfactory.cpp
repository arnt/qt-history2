
#include "abstractwidgetfactory.h"
#include "abstractformeditor.h"
#include "abstractwidgetdatabase.h"

AbstractWidgetFactory::AbstractWidgetFactory(QObject *parent)
    : QObject(parent)
{
}

AbstractWidgetFactory::~AbstractWidgetFactory()
{
}
