#ifndef QDESIGNER_PROMOTED_WIDGET_h
#define QDESIGNER_PROMOTED_WIDGET_H

#include <QWidget>
#include <QIcon>

#include "shared_global.h"
#include "propertysheet.h"
#include <default_extensionfactory.h>
#include <abstractformeditor.h>
#include <abstractwidgetdatabase.h>

class QDesignerPromotedWidget;
class QExtensionManager;

class QT_SHARED_EXPORT PromotedWidgetPropertySheet: public QObject, public IPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(IPropertySheet)
public:
    PromotedWidgetPropertySheet(QDesignerPromotedWidget *promoted, QExtensionManager *extension_manager, QObject *parent);
    virtual ~PromotedWidgetPropertySheet();

    virtual int count() const;

    virtual int indexOf(const QString &name) const;

    virtual QString propertyName(int index) const;
    virtual QString propertyGroup(int index) const;
    virtual void setPropertyGroup(int index, const QString &group);

    virtual bool hasReset(int index) const;
    virtual bool reset(int index);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual bool isAttribute(int index) const;
    virtual void setAttribute(int index, bool b);

    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);

    virtual bool isChanged(int index) const;
    virtual void setChanged(int index, bool changed);
private:
    QDesignerPromotedWidget *m_promoted;
    IPropertySheet *m_sheet;
};

class QT_SHARED_EXPORT PromotedWidgetPropertySheetFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    PromotedWidgetPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

class QT_SHARED_EXPORT QDesignerPromotedWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerPromotedWidget(AbstractWidgetDataBaseItem *item,
                            QWidget *child, QWidget *parent = 0);
    ~QDesignerPromotedWidget();

    QWidget *child() const { return m_child; }
    AbstractWidgetDataBaseItem *item() const { return m_item; }
    const char *customClassName() { return m_custom_class_name.constData(); }

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    virtual void childEvent(QChildEvent *e);

private:
    AbstractWidgetDataBaseItem *m_item;
    QByteArray m_custom_class_name;
    QWidget *m_child;
};

#endif // QDESIGNER_PROMOTED_WIDGET_H
