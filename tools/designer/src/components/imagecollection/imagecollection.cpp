
#include "imagecollection.h"

ImageCollection::ImageCollection(AbstractFormEditor *core, QObject *parent)
    : AbstractImageCollection(parent), m_core(core)
{
}

ImageCollection::~ImageCollection()
{
}

AbstractFormEditor *ImageCollection::core() const
{
    return m_core;
}

QString ImageCollection::fileName() const
{
    return QString::null;
}

QString ImageCollection::prefix() const
{
    return QString::null;
}

int ImageCollection::count() const
{
    return 0;
}

QString ImageCollection::item(int index) const
{
    Q_UNUSED(index);
    return QString::null;
}

