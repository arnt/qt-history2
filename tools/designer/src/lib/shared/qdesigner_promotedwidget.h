#ifndef QDESIGNER_PROMOTED_WIDGET_h
#define QDESIGNER_PROMOTED_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QIcon>

#include "shared_global.h"
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/default_extensionfactory.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetdatabase.h>

class QDesignerPromotedWidget;
class QExtensionManager;

class QT_SHARED_EXPORT PromotedWidgetPropertySheet: public QObject, public QDesignerPropertySheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
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
    QDesignerPropertySheetExtension *m_sheet;
};

class QT_SHARED_EXPORT PromotedWidgetPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    PromotedWidgetPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

class QT_SHARED_EXPORT QDesignerPromotedWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerPromotedWidget(QDesignerWidgetDataBaseItemInterface *item,
                            QWidget *child, QWidget *parent = 0);
    ~QDesignerPromotedWidget();

    QWidget *child() const { return m_child; }
    QDesignerWidgetDataBaseItemInterface *item() const { return m_item; }
    const char *customClassName() { return m_custom_class_name.constData(); }

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    virtual void childEvent(QChildEvent *e);

private:
    QDesignerWidgetDataBaseItemInterface *m_item;
    QByteArray m_custom_class_name;
    QWidget *m_child;
};

#endif // QDESIGNER_PROMOTED_WIDGET_H
