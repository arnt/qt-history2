#ifndef SPACER_PROPERTYSHEET_H
#define SPACER_PROPERTYSHEET_H

#include "default_propertysheet.h"

class Spacer;

class SpacerPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(IPropertySheet)
public:
    SpacerPropertySheet(Spacer *object, QObject *parent = 0);
    virtual ~SpacerPropertySheet();

    virtual void setProperty(int index, const QVariant &value);
};

class SpacerPropertySheetFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    SpacerPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // SPACER_PROPERTYSHEET_H
