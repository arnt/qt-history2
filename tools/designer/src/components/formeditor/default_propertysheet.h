#ifndef DEFAULT_PROPERTYSHEET_H
#define DEFAULT_PROPERTYSHEET_H

#include <propertysheet.h>
#include <default_extensionfactory.h>
#include <qpair.h>

class QDesignerPropertySheet: public QObject, public IPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(IPropertySheet)
public:
    QDesignerPropertySheet(QObject *object, QObject *parent = 0);
    virtual ~QDesignerPropertySheet();

    virtual int indexOf(const QString &name) const;

    virtual int count() const;
    virtual QString propertyName(int index) const;

    virtual QString propertyGroup(int index) const;
    virtual void setPropertyGroup(int index, const QString &group);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);
    virtual bool isChanged(int index) const;
    virtual void setChanged(int index, bool changed);
    
    void createFakeProperty(const QString &propertyName, const QVariant &value = QVariant());
    
protected:
    bool isFakeProperty(int index) const;
    QVariant resolvePropertyValue(const QVariant &value) const;
    QVariant metaProperty(int index) const;
    void setFakeProperty(int index, const QVariant &value);

protected:
    QObject *m_object;
    const QMetaObject *meta;

    struct Info
    {
        QString group;
        uint changed: 1;
        uint visible: 1;

        inline Info()
            : changed(0), visible(1) {}
    };

    QHash<int, Info> m_info;
    QList< QPair<QString, QVariant> > m_fakeProperties;
};

class QDesignerPropertySheetFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    QDesignerPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // DEFAULT_PROPERTYSHEET_H
