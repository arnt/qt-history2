#include "qdesigner_promotedwidget.h"

#include <qextensionmanager.h>
#include <abstractwidgetdatabase.h>

#include <QEvent>
#include <QIcon>

#include <qdebug.h>

PromotedWidgetPropertySheet::PromotedWidgetPropertySheet(QDesignerPromotedWidget *promoted,
                                QExtensionManager *extension_manager, QObject *parent)
    : QObject(parent)
{
    m_promoted = promoted;
    QWidget *child = promoted->child();
    m_sheet = qt_extension<IPropertySheet*>(extension_manager, child);
}

PromotedWidgetPropertySheet::~PromotedWidgetPropertySheet()
{
}

int PromotedWidgetPropertySheet::count() const
{
    return m_sheet->count();
}

int PromotedWidgetPropertySheet::indexOf(const QString &name) const
{
    return m_sheet->indexOf(name);
}

QString PromotedWidgetPropertySheet::propertyName(int index) const
{
    return m_sheet->propertyName(index);
}

QString PromotedWidgetPropertySheet::propertyGroup(int index) const
{
    return m_sheet->propertyGroup(index);
}

void PromotedWidgetPropertySheet::setPropertyGroup(int index, const QString &group)
{
    m_sheet->setPropertyGroup(index, group);
}

bool PromotedWidgetPropertySheet::hasReset(int index) const
{
    return m_sheet->hasReset(index);
}

void PromotedWidgetPropertySheet::reset(int index)
{
    m_sheet->reset(index);
}

bool PromotedWidgetPropertySheet::isVisible(int index) const
{
    return m_sheet->isVisible(index);
}

void PromotedWidgetPropertySheet::setVisible(int index, bool b)
{
    return m_sheet->setVisible(index, b);
}

bool PromotedWidgetPropertySheet::isAttribute(int index) const
{
    return m_sheet->isAttribute(index);
}

void PromotedWidgetPropertySheet::setAttribute(int index, bool b)
{
    m_sheet->setAttribute(index, b);
}

QVariant PromotedWidgetPropertySheet::property(int index) const
{
    QVariant result;

    QString name = propertyName(index);
    if (name == QLatin1String("geometry")) {
        result = m_promoted->geometry();
    } else {
        result = m_sheet->property(index);
    }
    
    return result;
}

void PromotedWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    QString name = propertyName(index);
    if (name == QLatin1String("geometry")) {
        if (value.type() == QVariant::Rect)
            m_promoted->setGeometry(value.toRect());
    } else {
        m_sheet->setProperty(index, value);
    }
}

bool PromotedWidgetPropertySheet::isChanged(int index) const
{
    return m_sheet->isChanged(index);
}

void PromotedWidgetPropertySheet::setChanged(int index, bool changed)
{
    m_sheet->setChanged(index, changed);
}

PromotedWidgetPropertySheetFactory::PromotedWidgetPropertySheetFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *PromotedWidgetPropertySheetFactory::createExtension(QObject *object,
                                            const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(IPropertySheet))
        return 0;
    QDesignerPromotedWidget *promoted = qt_cast<QDesignerPromotedWidget*>(object);
    if (promoted == 0)
        return 0;
    return new PromotedWidgetPropertySheet(promoted,
                                qt_cast<QExtensionManager*>(this->parent()),
                                parent);
}

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
    
private:
    QString m_name;
    QString m_include;
};

QDesignerPromotedWidget::QDesignerPromotedWidget(const QString &class_name, const QString &include_file,
                                                    QWidget *child, AbstractFormEditor *core,
                                                    QWidget *parent)
    : QWidget(parent)
{
    m_child = child;
    m_child_inserted = false;
    m_include_file = include_file;
    m_custom_class_name = class_name.toLatin1();

    AbstractWidgetDataBase *db = core->widgetDataBase();
    int idx = core->widgetDataBase()->indexOfClassName(class_name);
    if (idx == -1) {
        PromotedWidgetDataBaseItem *item = new PromotedWidgetDataBaseItem(class_name, include_file);
        db->append(item);
    } else {
        AbstractWidgetDataBaseItem *item = db->item(idx);
        item->setIncludeFile(include_file);
    }
}

QDesignerPromotedWidget::~QDesignerPromotedWidget()
{
}

void QDesignerPromotedWidget::resizeEvent(QResizeEvent*)
{
    if (m_child_inserted)
        m_child->setGeometry(rect());
}

void QDesignerPromotedWidget::childEvent(QChildEvent *e)
{
    if (e->child() != m_child)
        return;

    switch (e->type()) {
        case QEvent::ChildPolished:
            m_child_inserted = true;
            m_child->setGeometry(rect());
            break;
        case QEvent::ChildRemoved:
            m_child_inserted = false;
            break;
        default:
            break;
    }
}

