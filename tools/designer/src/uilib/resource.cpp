
#include "resource.h"
#include "ui4.h"

#include <QAction>
#include <QActionGroup>
#include <QGridLayout>
#include <QDomDocument>
#include <QVariant>
#include <QWidget>
#include <QMetaProperty>
#include <QDateTime>
#include <QIcon>
#include <QPixmap>
// containers
#include <QToolBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>

#include <qdebug.h>


#include <limits.h>

class FriendlyLayout: public QLayout
{
public:
    inline FriendlyLayout() { Q_ASSERT(0); }

    friend class Resource;
};

Resource::Resource()
{
    m_idToSizeType.insert("Fixed", QSizePolicy::Fixed);
    m_idToSizeType.insert("Minimum", QSizePolicy::Minimum);
    m_idToSizeType.insert("Maximum", QSizePolicy::Maximum);
    m_idToSizeType.insert("Preferred", QSizePolicy::Preferred);
    m_idToSizeType.insert("MinimumExpanding", QSizePolicy::MinimumExpanding);
    m_idToSizeType.insert("Expanding", QSizePolicy::Expanding);
    m_idToSizeType.insert("Ignored", QSizePolicy::Ignored);

    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;
}

Resource::~Resource()
{
}

QWidget *Resource::load(QIODevice *dev, QWidget *parentWidget)
{
    QDomDocument doc;
    if (!doc.setContent(dev))
        return 0;

    QDomElement root = doc.firstChild().toElement();
    DomUI ui;
    ui.read(root); /// ### check the result

    QWidget *w = create(&ui, parentWidget);
    DomConnections *connections = ui.elementConnections();
    if (connections != 0)
        createConnections(connections, w);
    
    return w;
}

QWidget *Resource::create(DomUI *ui, QWidget *parentWidget)
{
    if (DomLayoutDefault *def = ui->elementLayoutDefault()) {
        m_defaultMargin = def->hasAttributeMargin() ? def->attributeMargin() : INT_MIN;
        m_defaultSpacing = def->hasAttributeSpacing() ? def->attributeSpacing() : INT_MIN;
    }

    DomWidget *ui_widget = ui->elementWidget();
    if (!ui_widget)
        return 0;

    QWidget *widget = create(ui_widget, parentWidget);
    applyTabStops(widget, ui->elementTabStops());
    return widget;
}

QWidget *Resource::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QWidget *w = createWidget(ui_widget->attributeClass(), parentWidget, 0);
    if (!w)
        return 0;

    applyProperties(w, ui_widget->elementProperty());

#if 0 // ### implement me
    if (Q3GroupBox *g = qt_cast<Q3GroupBox*>(w)) {
        g->setColumnLayout(0, Qt::Vertical);

        int margin, spacing;
        layoutInfo(ui_widget, parentWidget, &margin, &spacing);

        if (margin != INT_MIN)
            g->layout()->setSpacing(margin);

        if (spacing != INT_MIN)
            g->layout()->setMargin(spacing);
    }
#endif

    foreach (DomAction *ui_action, ui_widget->elementAction()) {
        QAction *child_action = create(ui_action, w);
        Q_UNUSED( child_action );
    }
    
    foreach (DomActionGroup *ui_action_group, ui_widget->elementActionGroup()) {
        QActionGroup *child_action_group = create(ui_action_group, w);
        Q_UNUSED( child_action_group );
    }

    foreach (DomWidget *ui_child, ui_widget->elementWidget()) {
        QWidget *child_w = create(ui_child, w);
        Q_UNUSED( child_w );
    }

    foreach (DomLayout *ui_lay, ui_widget->elementLayout()) {
        QLayout *child_lay = create(ui_lay, 0, w);
        Q_UNUSED( child_lay );
    }
        
    foreach (DomActionRef *ui_action_ref, ui_widget->elementAddAction()) {
        QString name = ui_action_ref->attributeName();
        if (name == QLatin1String("separator")) {
            QAction *sep = new QAction(w);
            sep->setSeparator(true);
            w->addAction(sep);
        } else if (QAction *a = m_actions.value(name)) {
            w->addAction(a);
        } else if (QActionGroup *g = m_actionGroups.value(name)) {
            foreach (QAction *a, g->actions()) {
                w->addAction(a);
            }
        }
    }

    addItem(ui_widget, w, parentWidget);

    return w;
}

QAction *Resource::create(DomAction *ui_action, QObject *parent)
{
    QAction *a = createAction(parent, ui_action->attributeName());
    if (!a)
        return 0;
        
    applyProperties(a, ui_action->elementProperty());
    return a;
}

QActionGroup *Resource::create(DomActionGroup *ui_action_group, QObject *parent)
{
    QActionGroup *a = createActionGroup(parent, ui_action_group->attributeName());
    if (!a)
        return 0;
        
    applyProperties(a, ui_action_group->elementProperty());
    
    foreach (DomAction *ui_action, ui_action_group->elementAction()) {
        QAction *child_action = create(ui_action, a);
        Q_UNUSED( child_action );
    }
    
    foreach (DomActionGroup *g, ui_action_group->elementActionGroup()) {
        QActionGroup *child_action_group = create(g, a);
        Q_UNUSED( child_action_group );
    }
    return a;
}


bool Resource::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    QHash<QString, DomProperty*> attributes = propertyMap(ui_widget->elementAttribute());

    QString title = QLatin1String("Page");
    if (attributes.contains("title"))
        title = toString(attributes.value("title")->elementString());

    QString label = QLatin1String("Page");
    if (attributes.contains("label"))
        label = toString(attributes.value("label")->elementString());

    if (QTabWidget *tabWidget = qt_cast<QTabWidget*>(parentWidget)) {
        tabWidget->addTab(widget, title);
    } else if (QStackedWidget *stackedWidget = qt_cast<QStackedWidget*>(parentWidget)) {
        stackedWidget->addWidget(widget);
    } else if (QToolBox *toolBox = qt_cast<QToolBox*>(parentWidget)) {
        toolBox->addItem(widget, label);
    } else if (QMainWindow *mainWindow = qt_cast<QMainWindow*>(parentWidget)) {
        if (QToolBar *tb = qt_cast<QToolBar*>(widget)) {
            mainWindow->addToolBar(tb);
        }
    } else if (QMenuBar *mb = qt_cast<QMenuBar*>(parentWidget)) {
        if (QMenu *menu = qt_cast<QMenu*>(widget)) {
            mb->addMenu(menu);
        }
    } else if (QMenu *parentMenu = qt_cast<QMenu*>(parentWidget)) {
        if (QMenu *menu = qt_cast<QMenu*>(widget)) {
            parentMenu->addMenu(menu);
        }
    } else {
        return false;
    } 
    
    return true;
}

void Resource::layoutInfo(DomWidget *ui_widget, QObject *parent, int *margin, int *spacing)
{
    Q_UNUSED(parent);

    QHash<QString, DomProperty*> properties = propertyMap(ui_widget->elementProperty());

    if (margin)
        *margin = properties.contains("margin")
            ? properties.value("margin")->elementNumber()
            : m_defaultMargin;

    if (spacing)
        *spacing = properties.contains("spacing")
            ? properties.value("spacing")->elementNumber()
            : m_defaultSpacing;
}

void Resource::layoutInfo(DomLayout *ui_layout, QObject *parent, int *margin, int *spacing)
{
    QHash<QString, DomProperty*> properties = propertyMap(ui_layout->elementProperty());

    if (margin)
        *margin = properties.contains("margin")
            ? properties.value("margin")->elementNumber()
            : m_defaultMargin;

    if (spacing)
        *spacing = properties.contains("spacing")
            ? properties.value("spacing")->elementNumber()
            : m_defaultSpacing;

    if (margin && m_defaultMargin == INT_MIN) {
        Q_ASSERT(parent);
        if (qstrcmp(parent->metaObject()->className(), "QLayoutWidget") == 0)
            *margin = INT_MIN;
    }
}

QLayout *Resource::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    QObject *p = layout
            ? static_cast<QObject*>(layout)
            : static_cast<QObject*>(parentWidget);

#if 0 // ### enable me
    Q3GroupBox *g = qt_cast<Q3GroupBox*>(parentWidget);
    QLayout *lay = createLayout(ui_layout->attributeClass(), g ? g->layout() : p, 0);
#else
    QLayout *lay = createLayout(ui_layout->attributeClass(), p, 0);
#endif
    if (!lay)
        return 0;

    QObject *parent = parentWidget;
    if (!parent)
        parent = layout;

    int margin, spacing;
    layoutInfo(ui_layout, parent, &margin, &spacing);

    if (margin != INT_MIN) {
        // qDebug() << "setMargin:" << margin;
        lay->setMargin(margin);
    }

    if (spacing != INT_MIN) {
        // qDebug() << "setSpacing:" << spacing;
        lay->setSpacing(spacing);
    }

    applyProperties(lay, ui_layout->elementProperty());

    foreach (DomLayoutItem *ui_item, ui_layout->elementItem()) {
        QLayoutItem *item = create(ui_item, lay, parentWidget);

        if (item)
            addItem(ui_item, item, lay);
    }

    return lay;
}

bool Resource::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    if (item->widget()) {
        static_cast<FriendlyLayout*>(layout)->addChildWidget(item->widget());
    } else if (item->layout()) {
        static_cast<FriendlyLayout*>(layout)->addChildLayout(item->layout ());
    }

    if (QGridLayout *grid = qt_cast<QGridLayout*>(layout)) {
        int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
        int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
        grid->addItem(item, ui_item->attributeRow(), ui_item->attributeColumn(),
                        rowSpan, colSpan, item->alignment());
    } else {
        layout->addItem(item);
    }
    
    return true;
}

QLayoutItem *Resource::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    switch (ui_layoutItem->kind()) {
    case DomLayoutItem::Widget:
        return new QWidgetItem(create(ui_layoutItem->elementWidget(), parentWidget));

    case DomLayoutItem::Spacer: {
        QSize size(0, 0);
        QSizePolicy::SizeType sizeType = QSizePolicy::Expanding;
        bool isVspacer = false;

        DomSpacer *ui_spacer = ui_layoutItem->elementSpacer();

        foreach (DomProperty *p, ui_spacer->elementProperty()) {
            QVariant v = toVariant(&QObject::staticMetaObject, p); // ###  remove me
            if (v.isNull())
                continue;

            if (p->attributeName() == QLatin1String("sizeHint") && p->kind() == DomProperty::Size)
                size = v.toSize();  // ###  remove me
            else if (p->attributeName() == QLatin1String("sizeType") && p->kind() == DomProperty::Enum) {
                sizeType = m_idToSizeType.value(p->elementEnum(), QSizePolicy::Expanding);
            } else if (p->attributeName() == QLatin1String("orientation") && p->kind() == DomProperty::Enum)
                isVspacer = isVertical(p->elementEnum());
        }

        QSpacerItem *spacer = 0;
        if (isVspacer)
            spacer = new QSpacerItem(size.width(), size.height(), QSizePolicy::Minimum, sizeType);
        else
            spacer = new QSpacerItem(size.width(), size.height(), sizeType, QSizePolicy::Minimum);
        return spacer; }

    case DomLayoutItem::Layout:
        return create(ui_layoutItem->elementLayout(), layout, parentWidget);

    default:
        break;
    }

    return 0;
}

bool Resource::isVertical(const QString &str)
{
    return str == QLatin1String("Qt::Vertical")
        || str == QLatin1String("Vertical"); // ### compat
}

void Resource::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    foreach (DomProperty *p, properties) {
        QVariant v = toVariant(o->metaObject(), p);
        if (!v.isNull())
            o->setProperty(p->attributeName(), v);
    }
}

QVariant Resource::toVariant(const QMetaObject *meta, DomProperty *p)
{
    QVariant v;

    switch(p->kind()) {
    case DomProperty::Bool: {
        v = toBool(p->elementBool());
    } break;

    case DomProperty::Cstring: {
        v = p->elementCstring();
    } break;

    case DomProperty::Point: {
        DomPoint *point = p->elementPoint();
        QPoint pt(point->elementX(), point->elementY());
        v = QVariant(pt);
    } break;

    case DomProperty::Size: {
        DomSize *size = p->elementSize();
        QSize sz(size->elementWidth(), size->elementHeight());
        v = QVariant(sz);
    } break;

    case DomProperty::Rect: {
        DomRect *rc = p->elementRect();
        QRect g(rc->elementX(), rc->elementY(), rc->elementWidth(), rc->elementHeight());
        v = QVariant(g);
    } break;

    case DomProperty::String: {
        int index = meta->indexOfProperty(p->attributeName().toLatin1());
        if (index != -1 && meta->property(index).type() == QVariant::KeySequence)
            v = QKeySequence(p->elementString()->text());
        else
            v = p->elementString()->text();
    } break;

    case DomProperty::Number: {
        v = p->elementNumber();
    } break;

    case DomProperty::Color: {
        DomColor *color = p->elementColor();
        QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
        v = QVariant(c);
    } break;

    case DomProperty::Font: {
        DomFont *font = p->elementFont();

        QFont f(font->elementFamily(), font->elementPointSize(), font->elementWeight(), font->elementItalic());
        f.setBold(font->elementBold());
        f.setUnderline(font->elementUnderline());
        f.setStrikeOut(font->elementStrikeOut());
        v = QVariant(f);
    } break;

    case DomProperty::Date: {
        DomDate *date = p->elementDate();

        QDate d(date->elementYear(), date->elementMonth(), date->elementDay());
        v = QVariant(d);
    } break;

    case DomProperty::Time: {
        DomTime *t = p->elementTime();

        QTime tm(t->elementHour(), t->elementMinute(), t->elementSecond());
        v = QVariant(tm);
    } break;

    case DomProperty::DateTime: {
        DomDateTime *dateTime = p->elementDateTime();
        QDate d(dateTime->elementYear(), dateTime->elementMonth(), dateTime->elementDay());
        QTime tm(dateTime->elementHour(), dateTime->elementMinute(), dateTime->elementSecond());

        QDateTime dt(d, tm);
        v = QVariant(dt);
    } break;

    case DomProperty::IconSet: {
        DomResourcePixmap *pix = p->elementIconSet();
        QIcon icon(pix->text());
        v = QVariant(icon);
    } break;

    case DomProperty::Palette: {
        DomPalette *dom = p->elementPalette();
        QPalette palette;

        if (dom->elementActive()) {
            palette.setCurrentColorGroup(QPalette::Active);
            setupColorGroup(palette, dom->elementActive());
        }

        if (dom->elementInactive()) {
            palette.setCurrentColorGroup(QPalette::Inactive);
            setupColorGroup(palette, dom->elementInactive());
        }

        if (dom->elementDisabled()) {
            palette.setCurrentColorGroup(QPalette::Disabled);
            setupColorGroup(palette, dom->elementDisabled());
        }

        palette.setCurrentColorGroup(QPalette::Active);
        v = QVariant(palette);
    } break;

    case DomProperty::Cursor: {
        v = QCursor(static_cast<Qt::CursorShape>(p->elementCursor()));
    } break;

    case DomProperty::Set: {
        qDebug() << "set not implemented yet!"; // ### implement me
    } break;

    case DomProperty::Enum: {
        QByteArray pname = p->attributeName().toLatin1();
        int index = meta->indexOfProperty(pname);
        QMetaEnum e = meta->property(index).enumerator();

        QByteArray key = p->elementEnum().toLatin1();
        int idx = key.lastIndexOf("::");
        if (idx != -1)
            key = key.mid(idx + 2);

        v = e.keyToValue(key);
    } break;

    case DomProperty::SizePolicy: {
        DomSizePolicy *sizep = p->elementSizePolicy();

        QSizePolicy sizePolicy;
        sizePolicy.setHorizontalStretch(sizep->elementHorStretch());
        sizePolicy.setVerticalStretch(sizep->elementVerStretch());

        sizePolicy.setHorizontalData((QSizePolicy::SizeType) sizep->elementHSizeType());
        sizePolicy.setVerticalData((QSizePolicy::SizeType) sizep->elementVSizeType());
        v = sizePolicy;
    } break;

    default:
        qDebug() << "Resource::toVariant:" << p->kind() << " not implemented yet!";
        break;
    }

    return v;
}

void Resource::setupColorGroup(QPalette &palette, DomColorGroup *group)
{
    QList<DomColor*> colors = group->elementColor();
    for (int role = 0; role < colors.size(); ++role) {
        DomColor *color = colors.at(role);
        QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
        palette.setColor(QPalette::ColorRole(role), c); // ### TODO: support the QPalette::ColorRole as string
    }
}

DomColorGroup *Resource::saveColorGroup(const QPalette &palette)
{
    DomColorGroup *group = new DomColorGroup();
    QList<DomColor*> colors;

    for (int role = QPalette::Foreground; role < QPalette::NColorRoles; ++role) {
        QColor c = palette.color(QPalette::ColorRole(role));

        DomColor *color = new DomColor();
        color->setElementRed(c.red());
        color->setElementGreen(c.green());
        color->setElementBlue(c.blue());
        colors.append(color);
    }

    group->setElementColor(colors);
    return group;
}

QWidget *Resource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    Q_UNUSED(widgetName);
    Q_UNUSED(parentWidget);
    Q_UNUSED(name);
    return 0;
}

QLayout *Resource::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    Q_UNUSED(layoutName);
    Q_UNUSED(parent);
    Q_UNUSED(name);
    return 0;
}

QAction *Resource::createAction(QObject *parent, const QString &name)
{
    QAction *action = new QAction(parent);
    m_actions.insert(name, action);
    return action;
}

QActionGroup *Resource::createActionGroup(QObject *parent, const QString &name)
{
    QActionGroup *g = new QActionGroup(parent);
    m_actionGroups.insert(name, g);
    return g;
}

void Resource::save(QIODevice *dev, QWidget *widget)
{
    DomWidget *ui_widget = createDom(widget, 0);
    Q_ASSERT( ui_widget != 0 );

    DomUI *ui = new DomUI();
    ui->setAttributeVersion(QLatin1String("4.0"));
    ui->setElementClass(widget->objectName());
    ui->setElementWidget(ui_widget);
    ui->setElementConnections(saveConnections());
    ui->setElementCustomWidgets(saveCustomWidgets());
    ui->setElementTabStops(saveTabStops());
    
    QDomDocument doc;
    doc.appendChild(ui->write(doc));
    QByteArray bytes = doc.toString().toUtf8();
    dev->write(bytes, bytes.size());

    m_laidout.clear();

    delete ui;
}

DomConnections *Resource::saveConnections()
{
    return new DomConnections; // ### return 0; ??
}

DomWidget *Resource::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    DomWidget *ui_widget = new DomWidget();
    ui_widget->setAttributeClass(widget->metaObject()->className());
    ui_widget->setElementProperty(computeProperties(widget));

    if (!recursive)
        return ui_widget;

    QList<QObject*> children = widget->children();

    // widgets
    QList<DomWidget*> ui_widgets;

    if (widget->layout()) {
        DomLayout *ui_layout = createDom(widget->layout(), 0, ui_parentWidget);

        // layouts
        QList<DomLayout*> ui_layouts;
        QListMutableIterator<QObject*> lay_it(children);
        while (lay_it.hasNext()) {
            QObject *obj = lay_it.next();
            if (!qt_cast<QLayout*>(obj))
                continue;

            DomLayout *ui_child = createDom(static_cast<QLayout*>(obj), ui_layout, ui_widget);
            if (!ui_child) {
                // ### error message
            } else {
                ui_layouts.append(ui_child);
            }
        }
        ui_widget->setElementLayout(ui_layouts);
    }

    QListIterator<QObject*> it(children);
    while (it.hasNext()) {
        QObject *obj = it.next();
        if (!obj->isWidgetType() || m_laidout.contains(obj))
            continue;

        DomWidget *ui_child = createDom(static_cast<QWidget*>(obj), ui_widget);
        if (ui_child)
            ui_widgets.append(ui_child);
    }
    ui_widget->setElementWidget(ui_widgets);

    return ui_widget;
}

DomLayout *Resource::createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_layout)
    DomLayout *lay = new DomLayout();
    lay->setAttributeClass(layout->metaObject()->className());
    lay->setElementProperty(computeProperties(layout));

    QList<DomLayoutItem*> ui_items;

    for (int idx=0; layout->itemAt(idx); ++idx) {
        QLayoutItem *item = layout->itemAt(idx);

        DomLayoutItem *ui_item = createDom(item, lay, ui_parentWidget);
        if (ui_item)
            ui_items.append(ui_item);
    }

    lay->setElementItem(ui_items);

    return lay;
}

DomLayoutItem *Resource::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    DomLayoutItem *ui_item = new DomLayoutItem();

    if (item->widget())  {
        ui_item->setElementWidget(createDom(item->widget(), ui_parentWidget));
        m_laidout.insert(item->widget(), true);
    } else if (item->layout()) {
        ui_item->setElementLayout(createDom(item->layout(), ui_layout, ui_parentWidget));
    } else if (item->spacerItem()) {
        ui_item->setElementSpacer(createDom(item->spacerItem(), ui_layout, ui_parentWidget));
    }

    return ui_item;
}

DomSpacer *Resource::createDom(QSpacerItem *spacer, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_layout);
    Q_UNUSED(ui_parentWidget);

    DomSpacer *ui_spacer = new DomSpacer();
    QList<DomProperty*> properties;

    DomProperty *prop = 0;

    // sizeHint property
    prop = new DomProperty();
    prop->setAttributeName("sizeHint");
    prop->setElementSize(new DomSize());
    prop->elementSize()->setElementWidth(spacer->sizeHint().width());
    prop->elementSize()->setElementHeight(spacer->sizeHint().height());
    properties.append(prop);

    // orientation property
    prop = new DomProperty();
    prop->setAttributeName("orientation");
    prop->setElementEnum(spacer->expanding() == QSizePolicy::Vertically ? QLatin1String("Qt::Vertical") : QLatin1String("Qt::Horizontal"));
    properties.append(prop);

#if 0 /// ### implement me
    // sizeType property
    QSizePolicy::SizeType sizeType = QSizePolicy::Expanding;
    prop = new DomProperty();
    prop->setAttributeName("sizeType");
    if (isVspacer)
        prop->setElementEnum(m_idToSizeType.find(spacer->expanding().verData()).key());
    else
        prop->setElementEnum(m_idToSizeType.find(spacer->expanding().horData()).key());
    properties.append(prop);
#endif

    ui_spacer->setElementProperty(properties);
    return ui_spacer;
}

DomProperty *Resource::createProperty(QObject *obj, const QString &pname, const QVariant &v)
{
    if (!checkProperty(obj, pname)) {
        return 0;
    }

    DomProperty *dom_prop = new DomProperty();

    switch (v.type()) {
        case QVariant::String: {
            DomString *str = new DomString();
            str->setText(v.toString());

            if (pname == QLatin1String("objectName"))
                str->setAttributeNotr("true");

            dom_prop->setElementString(str);
        } break;

        case QVariant::ByteArray: {
            dom_prop->setElementCstring(v.toByteArray());
        } break;

#if 0
        case QVariant::Int: {
            if (prop.isFlagType())
                qWarning("flags property not supported yet!!");

            if (prop.isEnumType()) {
                QString scope = QString::fromUtf8(prop.enumerator().scope());
                if (scope.size())
                    scope += QString::fromUtf8("::");
                QString e = prop.enumerator().valueToKey(v.toInt());
                if (e.size())
                    dom_prop->setElementEnum(scope + e);
            } else
                dom_prop->setElementNumber(v.toInt());
        } break;
#else
        case QVariant::Int: {
            dom_prop->setElementNumber(v.toInt());
        } break;
#endif

        case QVariant::UInt: {
            dom_prop->setElementNumber(v.toUInt());
        } break;

        case QVariant::Bool: {
            dom_prop->setElementBool(v.toBool() ? QLatin1String("true") : QLatin1String("false"));
        } break;

        case QVariant::Point: {
            DomPoint *pt = new DomPoint();
            QPoint point = v.toPoint();
            pt->setElementX(point.x());
            pt->setElementY(point.y());
            dom_prop->setElementPoint(pt);
        } break;

        case QVariant::Size: {
            DomSize *sz = new DomSize();
            QSize size = v.toSize();
            sz->setElementWidth(size.width());
            sz->setElementHeight(size.height());
            dom_prop->setElementSize(sz);
        } break;

        case QVariant::Rect: {
            DomRect *rc = new DomRect();
            QRect rect = v.toRect();
            rc->setElementX(rect.x());
            rc->setElementY(rect.y());
            rc->setElementWidth(rect.width());
            rc->setElementHeight(rect.height());
            dom_prop->setElementRect(rc);
        } break;

        case QVariant::Font: {
            DomFont *fnt = new DomFont();
            QFont font = v.toFont();
            fnt->setElementBold(font.bold());
            fnt->setElementFamily(font.family());
            fnt->setElementItalic(font.italic());
            fnt->setElementPointSize(font.pointSize());
            fnt->setElementStrikeOut(font.strikeOut());
            fnt->setElementUnderline(font.underline());
            fnt->setElementWeight(font.weight());
            dom_prop->setElementFont(fnt);
        } break;

        case QVariant::Cursor: {
            dom_prop->setElementCursor(v.toCursor().shape());
        } break;

        case QVariant::KeySequence: {
            DomString *s = new DomString();
            s->setText(v.toKeySequence());
            dom_prop->setElementString(s);
        } break;

        case QVariant::Palette: {
            DomPalette *dom = new DomPalette();
            QPalette palette = v.toPalette();

            palette.setCurrentColorGroup(QPalette::Active);
            dom->setElementActive(saveColorGroup(palette));

            palette.setCurrentColorGroup(QPalette::Inactive);
            dom->setElementInactive(saveColorGroup(palette));

            palette.setCurrentColorGroup(QPalette::Disabled);
            dom->setElementDisabled(saveColorGroup(palette));

            dom_prop->setElementPalette(dom);
        } break;
        
        case QVariant::SizePolicy: {
            DomSizePolicy *dom = new DomSizePolicy();
            QSizePolicy sizePolicy = v.toSizePolicy();
            
            dom->setElementHorStretch(sizePolicy.horizontalStretch());
            dom->setElementVerStretch(sizePolicy.verticalStretch());
            
            dom->setElementHSizeType(sizePolicy.horizontalData());
            dom->setElementVSizeType(sizePolicy.verticalData());
            
            dom_prop->setElementSizePolicy(dom);
        } break;

        default: {
            qWarning("support for property `%s' of type `%d' not implemented yet!!",
                pname.latin1(), v.type());
        } break;
    }

    dom_prop->setAttributeName(pname);
    // ### dom_prop->setAttributeStdset(true);

    if (dom_prop->kind() == DomProperty::Unknown) {
        delete dom_prop;
        dom_prop = 0;
    }

    return dom_prop;
}

QList<DomProperty*> Resource::computeProperties(QObject *obj)
{
    QList<DomProperty*> lst;

    const QMetaObject *meta = obj->metaObject();

    QHash<QByteArray, bool> properties;
    for(int i=0; i<meta->propertyCount(); ++i)
        properties.insert(meta->property(i).name(), true);

    QList<QByteArray> propertyNames = properties.keys();

    for(int i=0; i<propertyNames.size(); ++i) {
        QString pname = propertyNames.at(i);
        QMetaProperty prop = meta->property(meta->indexOfProperty(pname));

        if (!prop.isWritable() || !checkProperty(obj, prop.name()))
            continue;

        QVariant v = prop.read(obj);
        
        DomProperty *dom_prop = 0;
        if (v.type() == QVariant::Int) {
            dom_prop = new DomProperty();
            
            if (prop.isFlagType())
                qWarning("flags property not supported yet!!");

            if (prop.isEnumType()) {
                QString scope = QString::fromUtf8(prop.enumerator().scope());
                if (scope.size())
                    scope += QString::fromUtf8("::");
                QString e = prop.enumerator().valueToKey(v.toInt());
                if (e.size())
                    dom_prop->setElementEnum(scope + e);
            } else
                dom_prop->setElementNumber(v.toInt());
            dom_prop->setAttributeName(pname);
        } else {
            dom_prop = createProperty(obj, pname, v);
        }
        
        if (!dom_prop || dom_prop->kind() == DomProperty::Unknown)
            delete dom_prop;
        else
            lst.append(dom_prop);
    }

    return lst;
}

bool Resource::toBool(const QString &str)
{
    return str.toLower() == QLatin1String("true");
}

QHash<QString, DomProperty*> Resource::propertyMap(const QList<DomProperty*> &properties)
{
    QHash<QString, DomProperty*> map;

    foreach (DomProperty *p, properties)
        map.insert(p->attributeName(), p);

    return map;
}

bool Resource::checkProperty(QObject *obj, const QString &prop) const
{
    Q_UNUSED(obj);
    Q_UNUSED(prop);

    return true;
}

QString Resource::toString(const DomString *str)
{
    return str ? str->text() : QString::null;
}

void Resource::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
    if (!tabStops)
        return;

    QWidget *lastWidget = 0;

    QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        QWidget *child = qFindChild<QWidget*>(widget, name);
        if (!child) {
            qWarning("'%s' isn't a valid widget\n", name.latin1());
            continue;
        }

        if (i == 0) {
            lastWidget = qFindChild<QWidget*>(widget, name);
            continue;
        } else if (!child || !lastWidget) {
            continue;
        }

        QWidget::setTabOrder(lastWidget, child);

        lastWidget = qFindChild<QWidget*>(widget, name);
    }
}

DomCustomWidgets *Resource::saveCustomWidgets()
{
    return 0;
}

DomTabStops *Resource::saveTabStops()
{
    return 0;
}

