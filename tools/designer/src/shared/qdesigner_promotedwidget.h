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
    virtual void reset(int index);

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

struct PromotedWidgetDataBaseItem : public AbstractWidgetDataBaseItem
{
    PromotedWidgetDataBaseItem(const QString &name = QString(), const QString &include = QString())
        : m_name(name), m_include(include) {}

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString group() const { return QObject::tr("Promoted Widgets"); }
    void setGroup(const QString &) {}

    QString toolTip() const { return QString(); }
    void setToolTip(const QString &) {}

    QString whatsThis() const { return QString(); }
    void setWhatsThis(const QString &) {}

    QString includeFile() const { return m_include; }
    void setIncludeFile(const QString &include) { m_include = include; }

    QIcon icon() const { return QIcon(); }
    void setIcon(const QIcon &) {}

    bool isCompat() const{ return false; }
    void setCompat(bool) {}
    
    bool isContainer() const { return false; }
    void setContainer(bool) {}

    bool isForm() const { return false; }
    void setForm(bool) {}

    bool isCustom() const { return true; }
    void setCustom(bool) {}

    QString pluginPath() const { return QString(); }
    void setPluginPath(const QString &) {}

    bool isPromoted() const { return true; }
    void setPromoted(bool) {}

    QString extends() const { return m_extends; }
    void setExtends(const QString &s) { m_extends = s; }
    
private:
    QString m_name;
    QString m_include;
    QString m_extends;
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
        
protected:
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    AbstractWidgetDataBaseItem *m_item;
    QByteArray m_custom_class_name;
    QWidget *m_child;
    bool m_child_inserted;
};

#endif // QDESIGNER_PROMOTED_WIDGET_H
