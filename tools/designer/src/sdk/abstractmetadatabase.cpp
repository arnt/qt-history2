
// sdk
#include "abstractmetadatabase.h"
#include "abstractformeditor.h"

// extension
#include <qextensionmanager.h>
#include <propertysheet.h>

// Qt
#include <qdebug.h>

AbstractMetaDataBase::AbstractMetaDataBase(QObject *parent)
    : QObject(parent)
{
}

AbstractMetaDataBase::~AbstractMetaDataBase()
{
}

void AbstractMetaDataBase::setPropertyChanged(QObject *o, const QString &propertyName, bool changed)
{
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core()->extensionManager(), o)) {
        sheet->setChanged(sheet->indexOf(propertyName), changed);
    }
}

bool AbstractMetaDataBase::isPropertyChanged(QObject *o, const QString &propertyName) const
{
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core()->extensionManager(), o)) {
        return sheet->isChanged(sheet->indexOf(propertyName));
    }        
    
    return false;
}

