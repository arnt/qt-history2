/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "abstractformbuilder.h"
#include "ui4_p.h"

#include <QtCore/QVariant>
#include <QtCore/QMetaProperty>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QQueue>
#include <QtCore/QUrl>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QIcon>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QPixmap>
#include <QtGui/QStatusBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>
#include <QtGui/QSplitter>

#include <QtXml/QDomDocument>

#include <QtGui/QDialog>

// containers
#include <QtGui/QToolBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QMenuBar>
#include <QtGui/QDockWidget>

#include <QtCore/qdebug.h>

#include <limits.h>
#include <private/qfont_p.h>

#ifdef QFORMINTERNAL_NAMESPACE
using namespace QFormInternal;
#endif

class QFriendlyLayout: public QLayout
{
public:
    inline QFriendlyLayout() { Q_ASSERT(0); }

#ifdef QFORMINTERNAL_NAMESPACE
    friend class QFormInternal::QAbstractFormBuilder;
#else
    friend class QAbstractFormBuilder;
#endif
};

// This class exists to provide meta information
// for enumerations only.
class QAbstractFormBuilderGadget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ fakeOrientation)
    Q_PROPERTY(QSizePolicy::Policy sizeType READ fakeSizeType)
    Q_PROPERTY(QPalette::ColorRole colorRole READ fakeColorRole)
    Q_PROPERTY(QPalette::ColorGroup colorGroup READ fakeColorGroup)
    Q_PROPERTY(QFont::StyleStrategy styleStrategy READ fakeStyleStrategy)
    Q_PROPERTY(Qt::CursorShape cursorShape READ fakeCursorShape)
    Q_PROPERTY(Qt::BrushStyle brushStyle READ fakeBrushStyle)
    Q_PROPERTY(Qt::ToolBarArea toolBarArea READ fakeToolBarArea)
    Q_PROPERTY(QGradient::Type gradientType READ fakeGradientType)
    Q_PROPERTY(QGradient::Spread gradientSpread READ fakeGradientSpread)
    Q_PROPERTY(QGradient::CoordinateMode gradientCoordinate READ fakeGradientCoordinate)
public:
    QAbstractFormBuilderGadget() { Q_ASSERT(0); }

    Qt::Orientation fakeOrientation() const     { Q_ASSERT(0); return Qt::Horizontal; }
    QSizePolicy::Policy fakeSizeType() const    { Q_ASSERT(0); return QSizePolicy::Expanding; }
    QPalette::ColorGroup fakeColorGroup() const { Q_ASSERT(0); return static_cast<QPalette::ColorGroup>(0); }
    QPalette::ColorRole fakeColorRole() const   { Q_ASSERT(0); return static_cast<QPalette::ColorRole>(0); }
    QFont::StyleStrategy fakeStyleStrategy() const     { Q_ASSERT(0); return QFont::PreferDefault; }
    Qt::CursorShape fakeCursorShape() const     { Q_ASSERT(0); return Qt::ArrowCursor; }
    Qt::BrushStyle fakeBrushStyle() const       { Q_ASSERT(0); return Qt::NoBrush; }
    Qt::ToolBarArea fakeToolBarArea() const {  Q_ASSERT(0); return Qt::NoToolBarArea; }
    QGradient::Type fakeGradientType() const    { Q_ASSERT(0); return QGradient::NoGradient; }
    QGradient::Spread fakeGradientSpread() const  { Q_ASSERT(0); return QGradient::PadSpread; }
    QGradient::CoordinateMode fakeGradientCoordinate() const  { Q_ASSERT(0); return QGradient::LogicalMode; }
};

namespace {
    // convert key to value for a given QMetaEnum
    template <class EnumType>
        inline static EnumType enumKeyToValue(const QMetaEnum &metaEnum,const char *key, const EnumType* = 0) {
            return static_cast<EnumType>(metaEnum.keyToValue(key));
        }

    // Access meta enumeration object of a qobject
    template <class QObjectType>
        inline QMetaEnum metaEnum(const char *name, const QObjectType* = 0) {
            const int e_index = QObjectType::staticMetaObject.indexOfProperty(name);
            Q_ASSERT(e_index != -1);
            return QObjectType::staticMetaObject.property(e_index).enumerator();
        }

    // convert key to value for a enumeration by name
    template <class QObjectType, class EnumType>
        EnumType enumKeyOfObjectToValue(const char *enumName, const char *key, const QObjectType* = 0, const EnumType* = 0) {
            const QMetaEnum me = metaEnum<QObjectType>(enumName);
            return enumKeyToValue<EnumType>(me, key);
        }
}

/*!
    \class QAbstractFormBuilder

    \brief The QAbstractFormBuilder class provides a default
    implementation for classes that create user interfaces at
    run-time.

    \inmodule QtDesigner

    QAbstractFormBuilder provides a standard interface and a default
    implementation for constructing forms from user interface
    files. It is not intended to be instantiated directly. Use the
    QFormBuilder class to create user interfaces from \c{.ui} files at
    run-time. For example:

    \code
        MyForm::MyForm(QWidget *parent)
            : QWidget(parent)
        {
            QFormBuilder builder;
            QFile file(":/forms/myWidget.ui");
            file.open(QFile::ReadOnly);
            QWidget *myWidget = builder.load(&file, this);
            file.close();

            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(myWidget);
            setLayout(layout);
        }
    \endcode

    To override certain aspects of the form builder's behavior,
    subclass QAbstractFormBuilder and reimplement the relevant virtual
    functions:

    \list
    \o load() handles reading of \c{.ui} format files from arbitrary
       QIODevices, and construction of widgets from the XML data
       that they contain.
    \o save() handles saving of widget details in \c{.ui} format to
       arbitrary QIODevices.
    \o workingDirectory() and setWorkingDirectory() control the
       directory in which forms are held. The form builder looks for
       other resources on paths relative to this directory.
    \endlist

    The QFormBuilder class is typically used by custom components and
    applications that embed \QD. Standalone applications that need to
    dynamically generate user interfaces at run-time use the
    QUiLoader, found in the QtUiTools module.

    \sa {QtUiTools Module}
*/

/*!
    Constructs a new form builder.*/
QAbstractFormBuilder::QAbstractFormBuilder()
{
    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;
}

/*!
    Destroys the form builder.*/
QAbstractFormBuilder::~QAbstractFormBuilder()
{
}

/*!
    \fn QWidget *QAbstractFormBuilder::load(QIODevice *device, QWidget *parent)

    Loads an XML representation of a widget from the given \a device,
    and constructs a new widget with the specified \a parent.

    \sa save()
*/
QWidget *QAbstractFormBuilder::load(QIODevice *dev, QWidget *parentWidget)
{
    QDomDocument doc;
    if (!doc.setContent(dev))
        return 0;

    QDomElement root = doc.firstChild().toElement();
    DomUI ui;
    ui.read(root); /// ### check the result

    return create(&ui, parentWidget);
}

/*!
    \internal
*/
QWidget *QAbstractFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
    if (const DomLayoutDefault *def = ui->elementLayoutDefault()) {
        m_defaultMargin = def->hasAttributeMargin() ? def->attributeMargin() : INT_MIN;
        m_defaultSpacing = def->hasAttributeSpacing() ? def->attributeSpacing() : INT_MIN;
    }

    DomWidget *ui_widget = ui->elementWidget();
    if (!ui_widget)
        return 0;

    createCustomWidgets(ui->elementCustomWidgets());

    if (QWidget *widget = create(ui_widget, parentWidget)) {
        createConnections(ui->elementConnections(), widget);
        createResources(ui->elementResources());
        applyTabStops(widget, ui->elementTabStops());
        reset();

        return widget;
    }

    return 0;
}

/*!
    \internal
*/
QWidget *QAbstractFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QWidget *w = createWidget(ui_widget->attributeClass(), parentWidget, ui_widget->attributeName());
    if (!w)
        return 0;

    applyProperties(w, ui_widget->elementProperty());

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
        const QString name = ui_action_ref->attributeName();
        if (name == QLatin1String("separator")) {
            QAction *sep = new QAction(w);
            sep->setSeparator(true);
            w->addAction(sep);
            addMenuAction(sep);
        } else if (QAction *a = m_actions.value(name)) {
            w->addAction(a);
        } else if (QActionGroup *g = m_actionGroups.value(name)) {
            w->addActions(g->actions());
        } else if (QMenu *menu = qFindChild<QMenu*>(w, name)) {
            w->addAction(menu->menuAction());
            addMenuAction(menu->menuAction());
        }
    }

    loadExtraInfo(ui_widget, w, parentWidget);
    addItem(ui_widget, w, parentWidget);

    if (qobject_cast<QDialog *>(w) && parentWidget)
        w->setAttribute(Qt::WA_Moved, false); // So that QDialog::setVisible(true) will center it

    return w;
}

/*!
    \internal
*/
QAction *QAbstractFormBuilder::create(DomAction *ui_action, QObject *parent)
{
    QAction *a = createAction(parent, ui_action->attributeName());
    if (!a)
        return 0;

    applyProperties(a, ui_action->elementProperty());
    return a;
}

/*!
    \internal
*/
QActionGroup *QAbstractFormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
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
        QActionGroup *child_action_group = create(g, parent);
        Q_UNUSED( child_action_group );
    }

    return a;
}

// figure out the toolbar area of a DOM attrib list.
// By legacy, it is stored as an integer. As of 4.3.0, it is the enumeration value.
Qt::ToolBarArea QAbstractFormBuilder::toolbarAreaFromDOMAttributes(const DomPropertyHash &attributes) {
    const DomProperty *attr = attributes.value(QLatin1String("toolBarArea"));
    if (!attr)
        return Qt::TopToolBarArea;
    switch(attr->kind()) {
    case DomProperty::Number:
        return static_cast<Qt::ToolBarArea>(attr->elementNumber());
    case DomProperty::Enum:
        return enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::ToolBarArea>("toolBarArea",  attr->elementEnum().toLatin1());
    default:
        break;
    }
    return Qt::TopToolBarArea;
}

/*!
    \internal
*/
bool QAbstractFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());

    QString title = QLatin1String("Page");
    if (const DomProperty *ptitle = attributes.value(QLatin1String("title"))) {
        title = toString(ptitle->elementString());
    }

    QString label = QLatin1String(QLatin1String("Page"));
    if (const DomProperty *plabel = attributes.value(QLatin1String("label"))) {
        label = toString(plabel->elementString());
    }

    if (QMainWindow *mw = qobject_cast<QMainWindow*>(parentWidget)) {

        // the menubar
        if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
            mw->setMenuBar(menuBar);
            return true;
        }

        // apply the toolbar's attributes
        else if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
            mw->addToolBar(toolbarAreaFromDOMAttributes(attributes), toolBar);
            // check break
            if (const DomProperty *attr = attributes.value(QLatin1String("toolBarBreak")))
                if (attr->elementBool() == QLatin1String("true"))
                    mw->insertToolBarBreak (toolBar);

            return true;
        }

        // statusBar
        else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
            mw->setStatusBar(statusBar);
            return true;
        }

        // apply the dockwidget's attributes
        else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(widget)) {
            if (const DomProperty *attr = attributes.value(QLatin1String("dockWidgetArea"))) {
                Qt::DockWidgetArea area = static_cast<Qt::DockWidgetArea>(attr->elementNumber());
                if (!dockWidget->isAreaAllowed(area)) {
                    if (dockWidget->isAreaAllowed(Qt::LeftDockWidgetArea))
                        area = Qt::LeftDockWidgetArea;
                    else if (dockWidget->isAreaAllowed(Qt::RightDockWidgetArea))
                        area = Qt::RightDockWidgetArea;
                    else if (dockWidget->isAreaAllowed(Qt::TopDockWidgetArea))
                        area = Qt::TopDockWidgetArea;
                    else if (dockWidget->isAreaAllowed(Qt::BottomDockWidgetArea))
                        area = Qt::BottomDockWidgetArea;
                }
                mw->addDockWidget(area, dockWidget);
            } else {
                mw->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
            }
            return true;
        }

        else if (! mw->centralWidget()) {
            mw->setCentralWidget(widget);
            return true;
        }
    }

    else if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(parentWidget)) {
        widget->setParent(0);

        const int tabIndex = tabWidget->count();
        tabWidget->addTab(widget, title);

        if (DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            tabWidget->setTabIcon(tabIndex, qvariant_cast<QIcon>(toVariant(0, picon)));
        }

        if (const DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            tabWidget->setTabToolTip(tabIndex, toString(ptoolTip->elementString()));
        }

        return true;
    }

    else if (QToolBox *toolBox = qobject_cast<QToolBox*>(parentWidget)) {
        const int tabIndex = toolBox->count();
        toolBox->addItem(widget, label);

        if (DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            toolBox->setItemIcon(tabIndex, qvariant_cast<QIcon>(toVariant(0, picon)));
        }

        if (const DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            toolBox->setItemToolTip(tabIndex, toString(ptoolTip->elementString()));
        }

        return true;
    }

    else if (QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(parentWidget)) {
        stackedWidget->addWidget(widget);
        return true;
    }

    else if (QSplitter *splitter = qobject_cast<QSplitter*>(parentWidget)) {
        splitter->addWidget(widget);
        return true;
    }

    else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget)) {
        dockWidget->setWidget(widget);
        return true;
    }

    return false;
}

/*!
    \internal
*/
void QAbstractFormBuilder::layoutInfo(DomLayout *ui_layout, QObject *parent, int *margin, int *spacing)
{
    const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

    if (margin)
        *margin = properties.contains(QLatin1String("margin"))
            ? properties.value(QLatin1String("margin"))->elementNumber()
            : m_defaultMargin;

    if (spacing)
        *spacing = properties.contains(QLatin1String("spacing"))
            ? properties.value(QLatin1String("spacing"))->elementNumber()
            : m_defaultSpacing;

    if (margin && m_defaultMargin == INT_MIN) {
        Q_ASSERT(parent);
        if (qstrcmp(parent->metaObject()->className(), "QLayoutWidget") == 0)
            *margin = INT_MIN;
    }
}

/*!
    \internal
*/
QLayout *QAbstractFormBuilder::create(DomLayout *ui_layout, QLayout *parentLayout, QWidget *parentWidget)
{
    QObject *p = parentLayout;

    if (p == 0)
        p = parentWidget;

    Q_ASSERT(p != 0);

    bool tracking = false;

    if (p == parentWidget && parentWidget->layout()) {
        tracking = true;
        p = parentWidget->layout();
    }

    QLayout *layout = createLayout(ui_layout->attributeClass(), p, QString());

    if (layout == 0)
        return 0;

    if (tracking && layout->parent() == 0) {
        QBoxLayout *box = qobject_cast<QBoxLayout*>(parentWidget->layout());
        Q_ASSERT(box != 0); // only QBoxLayout is supported
        box->addLayout(layout);
    }

    int margin = INT_MIN, spacing = INT_MIN;
    layoutInfo(ui_layout, p, &margin, &spacing);

    if (margin != INT_MIN)
       layout->setMargin(margin);

    if (spacing != INT_MIN)
        layout->setSpacing(spacing);

    applyProperties(layout, ui_layout->elementProperty());

    foreach (DomLayoutItem *ui_item, ui_layout->elementItem()) {
        if (QLayoutItem *item = create(ui_item, layout, parentWidget)) {
            addItem(ui_item, item, layout);
        }
    }

    return layout;
}

/*!
    \internal
*/
bool QAbstractFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    if (item->widget()) {
        static_cast<QFriendlyLayout*>(layout)->addChildWidget(item->widget());
    } else if (item->layout()) {
        static_cast<QFriendlyLayout*>(layout)->addChildLayout(item->layout());
    } else if (item->spacerItem()) {
        // nothing to do
    } else {
        return false;
    }

    if (QGridLayout *grid = qobject_cast<QGridLayout*>(layout)) {
        const int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
        const int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
        grid->addItem(item, ui_item->attributeRow(), ui_item->attributeColumn(),
                        rowSpan, colSpan, item->alignment());
    } else {
        layout->addItem(item);
    }

    return true;
}

/*!
    \internal
*/
QLayoutItem *QAbstractFormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    switch (ui_layoutItem->kind()) {
    case DomLayoutItem::Widget:
        return new QWidgetItem(create(ui_layoutItem->elementWidget(), parentWidget));

    case DomLayoutItem::Spacer: {
        QSize size(0, 0);
        QSizePolicy::Policy sizeType = QSizePolicy::Expanding;
        bool isVspacer = false;

        const DomSpacer *ui_spacer = ui_layoutItem->elementSpacer();

        const QMetaEnum sizePolicy_enum  = metaEnum<QAbstractFormBuilderGadget>("sizeType");
        const QMetaEnum orientation_enum =  metaEnum<QAbstractFormBuilderGadget>("orientation");

        foreach (DomProperty *p, ui_spacer->elementProperty()) {
            const QVariant v = toVariant(&QAbstractFormBuilderGadget::staticMetaObject, p); // ### remove me
            if (v.isNull())
                continue;

            if (p->attributeName() == QLatin1String("sizeHint") && p->kind() == DomProperty::Size) {
                size = v.toSize();  // ###  remove me
            } else if (p->attributeName() == QLatin1String("sizeType") && p->kind() == DomProperty::Enum) {
                sizeType = enumKeyToValue<QSizePolicy::Policy>(sizePolicy_enum, p->elementEnum().toUtf8());
            } else if (p->attributeName() == QLatin1String("orientation") && p->kind() == DomProperty::Enum) {
                const Qt::Orientation o = enumKeyToValue<Qt::Orientation>(orientation_enum, p->elementEnum().toUtf8());
                isVspacer = (o == Qt::Vertical);
            }
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

/*!
    \internal
*/
void QAbstractFormBuilder::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    foreach (DomProperty *p, properties) {
        QVariant v = toVariant(o->metaObject(), p);
        if (!v.isNull())
            o->setProperty(p->attributeName().toUtf8(), v);
    }
}

/*!
    \internal
*/

QVariant QAbstractFormBuilder::toVariant(const QMetaObject *meta, DomProperty *p)
{
    // requires non-const virtual nameToIcon, etc.
    switch(p->kind()) {
    case DomProperty::Bool:
        return QVariant(toBool(p->elementBool()));

    case DomProperty::Cstring:
        return QVariant(p->elementCstring());

    case DomProperty::Point: {
        const DomPoint *point = p->elementPoint();
        return QVariant(QPoint(point->elementX(), point->elementY()));
    }

    case DomProperty::PointF: {
        const DomPointF *pointf = p->elementPointF();
        return QVariant(QPointF(pointf->elementX(), pointf->elementY()));
    }

    case DomProperty::Size: {
        const DomSize *size = p->elementSize();
        return QVariant(QSize(size->elementWidth(), size->elementHeight()));
    }

    case DomProperty::SizeF: {
        const DomSizeF *sizef = p->elementSizeF();
        return QVariant(QSizeF(sizef->elementWidth(), sizef->elementHeight()));
    }

    case DomProperty::Rect: {
        const DomRect *rc = p->elementRect();
        const QRect g(rc->elementX(), rc->elementY(), rc->elementWidth(), rc->elementHeight());
        return QVariant(g);
    }

    case DomProperty::RectF: {
        const DomRectF *rcf = p->elementRectF();
        const QRectF g(rcf->elementX(), rcf->elementY(), rcf->elementWidth(), rcf->elementHeight());
        return QVariant(g);
    }

    case DomProperty::String: {
        const int index = meta->indexOfProperty(p->attributeName().toUtf8());
        if (index != -1 && meta->property(index).type() == QVariant::KeySequence)
            return qVariantFromValue(QKeySequence(p->elementString()->text()));
        else
            return QVariant(p->elementString()->text());
    }

    case DomProperty::Number:
        return QVariant(p->elementNumber());

    case DomProperty::UInt:
        return QVariant(p->elementUInt());

    case DomProperty::LongLong:
        return QVariant(p->elementLongLong());

    case DomProperty::ULongLong:
        return QVariant(p->elementULongLong());

    case DomProperty::Double:
        return QVariant(p->elementDouble());


    case DomProperty::Char: {
        const DomChar *character = p->elementChar();
        const QChar c(character->elementUnicode());
        return qVariantFromValue(c);
    }

    case DomProperty::Color: {
        const DomColor *color = p->elementColor();
        const QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
        return qVariantFromValue(c);
    }

    case DomProperty::Font: {
        const DomFont *font = p->elementFont();

        QFont f;
        if (font->hasElementFamily() && !font->elementFamily().isEmpty())
            f.setFamily(font->elementFamily());
        if (font->hasElementPointSize() && font->elementPointSize() > 0)
            f.setPointSize(font->elementPointSize());
        if (font->hasElementWeight() && font->elementWeight() > 0)
            f.setWeight(font->elementWeight());
        if (font->hasElementItalic())
            f.setItalic(font->elementItalic());
        if (font->hasElementBold())
            f.setBold(font->elementBold());
        if (font->hasElementUnderline())
            f.setUnderline(font->elementUnderline());
        if (font->hasElementStrikeOut())
            f.setStrikeOut(font->elementStrikeOut());
        if (font->hasElementKerning())
            f.setKerning(font->elementKerning());
        if (font->hasElementAntialiasing())
            f.setStyleStrategy(font->elementAntialiasing() ? QFont::PreferDefault : QFont::NoAntialias);
        if (font->hasElementStyleStrategy()) {
            f.setStyleStrategy(enumKeyOfObjectToValue<QAbstractFormBuilderGadget, QFont::StyleStrategy>("styleStrategy", font->elementStyleStrategy().toLatin1()));
        }
        return qVariantFromValue(f);
    }

    case DomProperty::Date: {
        const DomDate *date = p->elementDate();
        return QVariant(QDate(date->elementYear(), date->elementMonth(), date->elementDay()));
    }

    case DomProperty::Time: {
        const DomTime *t = p->elementTime();
        return QVariant(QTime(t->elementHour(), t->elementMinute(), t->elementSecond()));
    }

    case DomProperty::DateTime: {
        const DomDateTime *dateTime = p->elementDateTime();
        const QDate d(dateTime->elementYear(), dateTime->elementMonth(), dateTime->elementDay());
        const QTime tm(dateTime->elementHour(), dateTime->elementMinute(), dateTime->elementSecond());
        return QVariant(QDateTime(d, tm));
    }

    case DomProperty::Url: {
        const DomUrl *url = p->elementUrl();
        return QVariant(QUrl(url->elementString()->text()));
    }

    case DomProperty::Pixmap: {
        const DomResourcePixmap * dpx = domPixmap(p);
        return qVariantFromValue(dpx ? domPropertyToPixmap(p) : QPixmap());
    }

    case DomProperty::IconSet: {
        const DomResourcePixmap * dpx = domPixmap(p);
        return qVariantFromValue(dpx ? domPropertyToIcon(p) : QIcon());
    }


    case DomProperty::Palette: {
        const DomPalette *dom = p->elementPalette();
        QPalette palette;

        if (dom->elementActive())
            setupColorGroup(palette, QPalette::Active, dom->elementActive());

        if (dom->elementInactive())
            setupColorGroup(palette, QPalette::Inactive, dom->elementInactive());

        if (dom->elementDisabled())
            setupColorGroup(palette, QPalette::Disabled, dom->elementDisabled());

        palette.setCurrentColorGroup(QPalette::Active);
        return qVariantFromValue(palette);
    }

    case DomProperty::Cursor:
        return qVariantFromValue(QCursor(static_cast<Qt::CursorShape>(p->elementCursor())));

    case DomProperty::CursorShape:
        return qVariantFromValue(QCursor(enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::CursorShape>("cursorShape", p->elementCursorShape().toLatin1())));

    case DomProperty::Set: {
        const QByteArray pname = p->attributeName().toUtf8();
        const int index = meta->indexOfProperty(pname);
        if (index == -1) {
            qWarning() << "property" << pname << "is not supported";
            return QVariant();
        }

        const QMetaEnum e = meta->property(index).enumerator();
        Q_ASSERT(e.isFlag() == true);
        return QVariant(e.keysToValue(p->elementSet().toUtf8()));
    }

    case DomProperty::Enum: {
        const QByteArray pname = p->attributeName().toUtf8();
        const int index = meta->indexOfProperty(pname);
        if (index == -1) {
            // ### special-casing for Line (QFrame) -- fix for 4.2
            if (!qstrcmp(meta->className(), "QFrame")
                && (pname == QLatin1String("orientation"))) {
                return QVariant((p->elementEnum() == QLatin1String("Qt::Horizontal")) ? QFrame::HLine : QFrame::VLine);
            } else {
                qWarning() << "property" << pname << "is not supported";
                return QVariant();
            }
        }

        const QMetaEnum e = meta->property(index).enumerator();
        return QVariant(e.keyToValue(p->elementEnum().toUtf8()));
    }

    case DomProperty::SizePolicy: {
        const DomSizePolicy *sizep = p->elementSizePolicy();

        QSizePolicy sizePolicy;
        sizePolicy.setHorizontalStretch(sizep->elementHorStretch());
        sizePolicy.setVerticalStretch(sizep->elementVerStretch());

        const QMetaEnum sizeType_enum = metaEnum<QAbstractFormBuilderGadget>("sizeType");

        if (sizep->hasElementHSizeType()) {
            sizePolicy.setHorizontalPolicy((QSizePolicy::Policy) sizep->elementHSizeType());
        } else if (sizep->hasAttributeHSizeType()) {
            const QSizePolicy::Policy sp = enumKeyToValue<QSizePolicy::Policy>(sizeType_enum, sizep->attributeHSizeType().toLatin1());
            sizePolicy.setHorizontalPolicy(sp);
        }

        if (sizep->hasElementVSizeType()) {
            sizePolicy.setVerticalPolicy((QSizePolicy::Policy) sizep->elementVSizeType());
        } else if (sizep->hasAttributeVSizeType()) {
            const  QSizePolicy::Policy sp = enumKeyToValue<QSizePolicy::Policy>(sizeType_enum, sizep->attributeVSizeType().toLatin1());
            sizePolicy.setVerticalPolicy(sp);
        }

        return qVariantFromValue(sizePolicy);
    }

    case DomProperty::StringList:
        return QVariant(p->elementStringList()->elementString());

    default:
        qWarning() << "QAbstractFormBuilder::toVariant:" << p->kind() << " not implemented yet!";
        break;

    }

    return QVariant();
}

/*!
    \internal
*/
void QAbstractFormBuilder::setupColorGroup(QPalette &palette, QPalette::ColorGroup colorGroup,
            DomColorGroup *group)
{
    // old format
    const QList<DomColor*> colors = group->elementColor();
    for (int role = 0; role < colors.size(); ++role) {
        const DomColor *color = colors.at(role);
        const QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
        palette.setColor(colorGroup, QPalette::ColorRole(role), c);
    }

    // new format
    const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

    const QList<DomColorRole*> colorRoles = group->elementColorRole();
    for (int role = 0; role < colorRoles.size(); ++role) {
        const DomColorRole *colorRole = colorRoles.at(role);

        if (colorRole->hasAttributeRole()) {
            const int r = colorRole_enum.keyToValue(colorRole->attributeRole().toLatin1());
            if (r != -1) {
                const QBrush br = setupBrush(colorRole->elementBrush());
                palette.setBrush(colorGroup, static_cast<QPalette::ColorRole>(r), br);
            }
        }
    }
}

/*!
    \internal
*/
DomColorGroup *QAbstractFormBuilder::saveColorGroup(const QPalette &palette)
{

    const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

    DomColorGroup *group = new DomColorGroup();
    QList<DomColorRole*> colorRoles;

    const uint mask = palette.resolve();
    for (int role = QPalette::WindowText; role < QPalette::NColorRoles; ++role) {
        if (mask & (1 << role)) {
            QBrush br = palette.brush(QPalette::ColorRole(role));

            DomColorRole *colorRole = new DomColorRole();
            colorRole->setElementBrush(saveBrush(br));
            colorRole->setAttributeRole(colorRole_enum.valueToKey(role));
            colorRoles.append(colorRole);
        }
    }

    group->setElementColorRole(colorRoles);
    return group;
}

/*!
    \internal
*/
QBrush QAbstractFormBuilder::setupBrush(DomBrush *brush)
{
    QBrush br;
    if (!brush->hasAttributeBrushStyle())
        return br;

    const Qt::BrushStyle style = enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::BrushStyle>("brushStyle", brush->attributeBrushStyle().toLatin1());

    if (style == Qt::LinearGradientPattern ||
            style == Qt::RadialGradientPattern ||
            style == Qt::ConicalGradientPattern) {
        const QMetaEnum gradientType_enum = metaEnum<QAbstractFormBuilderGadget>("gradientType");
        const QMetaEnum gradientSpread_enum = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
        const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

        const DomGradient *gradient = brush->elementGradient();
        const QGradient::Type type = enumKeyToValue<QGradient::Type>(gradientType_enum, gradient->attributeType().toLatin1());


        QGradient *gr = 0;

        if (type == QGradient::LinearGradient) {
            gr = new QLinearGradient(QPointF(gradient->attributeStartX(), gradient->attributeStartY()),
                            QPointF(gradient->attributeEndX(), gradient->attributeEndY()));
        } else if (type == QGradient::RadialGradient) {
            gr = new QRadialGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
                            gradient->attributeRadius(),
                            QPointF(gradient->attributeFocalX(), gradient->attributeFocalY()));
        } else if (type == QGradient::ConicalGradient) {
            gr = new QConicalGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
                            gradient->attributeAngle());
        }
        if (!gr)
            return br;

        const QGradient::Spread spread = enumKeyToValue<QGradient::Spread>(gradientSpread_enum, gradient->attributeSpread().toLatin1());
        gr->setSpread(spread);

        const QGradient::CoordinateMode coord = enumKeyToValue<QGradient::CoordinateMode>(gradientCoordinate_enum, gradient->attributeCoordinateMode().toLatin1());
        gr->setCoordinateMode(coord);

        const QList<DomGradientStop *> stops = gradient->elementGradientStop();
        QListIterator<DomGradientStop *> it(stops);
        while (it.hasNext()) {
            const DomGradientStop *stop = it.next();
            const DomColor *color = stop->elementColor();
            gr->setColorAt(stop->attributePosition(), QColor::fromRgb(color->elementRed(),
                            color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
        }
        br = QBrush(*gr);
        delete gr;
    } else if (style == Qt::TexturePattern) {
        const DomProperty *texture = brush->elementTexture();
        if (texture && texture->kind() == DomProperty::Pixmap) {
            br.setTexture(domPropertyToPixmap(texture));
        }
    } else {
        const DomColor *color = brush->elementColor();
        br.setColor(QColor::fromRgb(color->elementRed(),
                            color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
        br.setStyle((Qt::BrushStyle)style);
    }
    return br;
}

/*!
    \internal
*/
DomBrush *QAbstractFormBuilder::saveBrush(const QBrush &br)
{
    const QMetaEnum brushStyle_enum = metaEnum<QAbstractFormBuilderGadget>("brushStyle");

    DomBrush *brush = new DomBrush();
    const Qt::BrushStyle style = br.style();
    brush->setAttributeBrushStyle(brushStyle_enum.valueToKey(style));
    if (style == Qt::LinearGradientPattern ||
                style == Qt::RadialGradientPattern ||
                style == Qt::ConicalGradientPattern) {
        const QMetaEnum gradientType_enum = metaEnum<QAbstractFormBuilderGadget>("gradientType");
        const QMetaEnum gradientSpread_enum = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
        const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

        DomGradient *gradient = new DomGradient();
        const QGradient *gr = br.gradient();
        const QGradient::Type type = gr->type();
        gradient->setAttributeType(gradientType_enum.valueToKey(type));
        gradient->setAttributeSpread(gradientSpread_enum.valueToKey(gr->spread()));
        gradient->setAttributeCoordinateMode(gradientCoordinate_enum.valueToKey(gr->coordinateMode()));
        QList<DomGradientStop *> stops;
        QGradientStops st = gr->stops();
        QVectorIterator<QPair<qreal, QColor> > it(st);
        while (it.hasNext()) {
            const QPair<qreal, QColor> pair = it.next();
            DomGradientStop *stop = new DomGradientStop();
            stop->setAttributePosition(pair.first);
            DomColor *color = new DomColor();
            color->setElementRed(pair.second.red());
            color->setElementGreen(pair.second.green());
            color->setElementBlue(pair.second.blue());
            color->setAttributeAlpha(pair.second.alpha());
            stop->setElementColor(color);
            stops.append(stop);
        }
        gradient->setElementGradientStop(stops);
        if (type == QGradient::LinearGradient) {
            QLinearGradient *lgr = (QLinearGradient *)(gr);
            gradient->setAttributeStartX(lgr->start().x());
            gradient->setAttributeStartY(lgr->start().y());
            gradient->setAttributeEndX(lgr->finalStop().x());
            gradient->setAttributeEndY(lgr->finalStop().y());
        } else if (type == QGradient::RadialGradient) {
            QRadialGradient *rgr = (QRadialGradient *)(gr);
            gradient->setAttributeCentralX(rgr->center().x());
            gradient->setAttributeCentralY(rgr->center().y());
            gradient->setAttributeFocalX(rgr->focalPoint().x());
            gradient->setAttributeFocalY(rgr->focalPoint().y());
            gradient->setAttributeRadius(rgr->radius());
        } else if (type == QGradient::ConicalGradient) {
            QConicalGradient *cgr = (QConicalGradient *)(gr);
            gradient->setAttributeCentralX(cgr->center().x());
            gradient->setAttributeCentralY(cgr->center().y());
            gradient->setAttributeAngle(cgr->angle());
        }

        brush->setElementGradient(gradient);
    } else if (style == Qt::TexturePattern) {
        const QPixmap pixmap = br.texture();
        if (!pixmap.isNull()) {
            DomProperty *p = new DomProperty;
            setPixmapProperty(*p,  pixmapPaths(pixmap));
            brush->setElementTexture(p);
        }
    } else {
        QColor c = br.color();
        DomColor *color = new DomColor();
        color->setElementRed(c.red());
        color->setElementGreen(c.green());
        color->setElementBlue(c.blue());
        color->setAttributeAlpha(c.alpha());
        brush->setElementColor(color);
    }
    return brush;
}

/*!
    \internal
*/
QWidget *QAbstractFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    Q_UNUSED(widgetName);
    Q_UNUSED(parentWidget);
    Q_UNUSED(name);
    return 0;
}

/*!
    \internal
*/
QLayout *QAbstractFormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    Q_UNUSED(layoutName);
    Q_UNUSED(parent);
    Q_UNUSED(name);
    return 0;
}

/*!
    \internal
*/
QAction *QAbstractFormBuilder::createAction(QObject *parent, const QString &name)
{
    QAction *action = new QAction(parent);
    action->setObjectName(name);
    m_actions.insert(name, action);

    return action;
}

/*!
    \internal
*/
QActionGroup *QAbstractFormBuilder::createActionGroup(QObject *parent, const QString &name)
{
    QActionGroup *g = new QActionGroup(parent);
    g->setObjectName(name);
    m_actionGroups.insert(name, g);

    return g;
}

/*!
    \fn void QAbstractFormBuilder::save(QIODevice *device, QWidget *widget)

    Saves an XML representation of the given \a widget to the
    specified \a device in the standard \c{.ui} file format.

    \sa load()*/
void QAbstractFormBuilder::save(QIODevice *dev, QWidget *widget)
{
    DomWidget *ui_widget = createDom(widget, 0);
    Q_ASSERT( ui_widget != 0 );

    DomUI *ui = new DomUI();
    ui->setAttributeVersion(QLatin1String("4.0"));
    ui->setElementWidget(ui_widget);

    saveDom(ui, widget);

    QDomDocument doc;
    doc.appendChild(ui->write(doc));
    QByteArray bytes = doc.toString().toUtf8();
    dev->write(bytes, bytes.size());

    m_laidout.clear();

    delete ui;
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveDom(DomUI *ui, QWidget *widget)
{
    ui->setElementClass(widget->objectName());

    if (DomConnections *ui_connections = saveConnections()) {
        ui->setElementConnections(ui_connections);
    }

    if (DomCustomWidgets *ui_customWidgets = saveCustomWidgets()) {
        ui->setElementCustomWidgets(ui_customWidgets);
    }

    if (DomTabStops *ui_tabStops = saveTabStops()) {
        ui->setElementTabStops(ui_tabStops);
    }

    if (DomResources *ui_resources = saveResources()) {
        ui->setElementResources(ui_resources);
    }
}

/*!
    \internal
*/
DomConnections *QAbstractFormBuilder::saveConnections()
{
    return new DomConnections;
}

/*!
    \internal
*/
DomWidget *QAbstractFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    DomWidget *ui_widget = new DomWidget();
    ui_widget->setAttributeClass(QLatin1String(widget->metaObject()->className()));
    ui_widget->setElementProperty(computeProperties(widget));

    if (recursive) {
        if (QLayout *layout = widget->layout()) {
            if (DomLayout *ui_layout = createDom(layout, 0, ui_parentWidget)) {
                QList<DomLayout*> ui_layouts;
                ui_layouts.append(ui_layout);

                ui_widget->setElementLayout(ui_layouts);
            }
        }
    }

    // widgets, actions and action groups
    QList<DomWidget*> ui_widgets;
    QList<DomAction*> ui_actions;
    QList<DomActionGroup*> ui_action_groups;

    QList<QObject*> children;

    // splitters need to store their children in the order specified by child indexes,
    // not the order of the child list.
    if (QSplitter *splitter = qobject_cast<QSplitter*>(widget)) {
        for (int i = 0; i < splitter->count(); ++i)
            children.append(splitter->widget(i));
    } else {
        children = widget->children();
    }

    foreach (QObject *obj, children) {
        if (QWidget *childWidget = qobject_cast<QWidget*>(obj)) {
            if (m_laidout.contains(childWidget) || recursive == false)
                continue;

            if (QMenu *menu = qobject_cast<QMenu *>(childWidget)) {
                QList<QAction *> actions = menu->parentWidget()->actions();
                QListIterator<QAction *> it(actions);
                bool found = false;
                while (it.hasNext()) {
                    if (it.next()->menu() == menu)
                        found = true;
                }
                if (!found)
                    continue;
            }

            if (DomWidget *ui_child = createDom(childWidget, ui_widget)) {
                ui_widgets.append(ui_child);
            }
        } else if (QAction *childAction = qobject_cast<QAction*>(obj)) {
            if (childAction->actionGroup() != 0) {
                // it will be added later.
                continue;
            }

            if (DomAction *ui_action = createDom(childAction)) {
                ui_actions.append(ui_action);
            }
        } else if (QActionGroup *childActionGroup = qobject_cast<QActionGroup*>(obj)) {
            if (DomActionGroup *ui_action_group = createDom(childActionGroup)) {
                ui_action_groups.append(ui_action_group);
            }
        }
    }

    // add-action
    QList<DomActionRef*> ui_action_refs;
    foreach (QAction *action, widget->actions()) {
        if (DomActionRef *ui_action_ref = createActionRefDom(action)) {
            ui_action_refs.append(ui_action_ref);
        }
    }

    if (recursive)
        ui_widget->setElementWidget(ui_widgets);

    ui_widget->setElementAction(ui_actions);
    ui_widget->setElementActionGroup(ui_action_groups);
    ui_widget->setElementAddAction(ui_action_refs);

    saveExtraInfo(widget, ui_widget, ui_parentWidget);

    return ui_widget;
}

/*!
    \internal
*/
DomActionRef *QAbstractFormBuilder::createActionRefDom(QAction *action)
{
    QString name = action->objectName();

    if (action->menu() != 0)
        name = action->menu()->objectName();

    DomActionRef *ui_action_ref = new DomActionRef();
    if (action->isSeparator())
        ui_action_ref->setAttributeName(QLatin1String("separator"));
    else
        ui_action_ref->setAttributeName(name);

    return ui_action_ref;
}

/*!
    \internal
*/
DomLayout *QAbstractFormBuilder::createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_layout)
    DomLayout *lay = new DomLayout();
    lay->setAttributeClass(QLatin1String(layout->metaObject()->className()));
    lay->setElementProperty(computeProperties(layout));

    QList<DomLayoutItem*> ui_items;

    QMap<QObject *, QLayoutItem *> objectToItem;
    QList<QLayoutItem *> spacerItems;
    QList<QLayoutItem *> newList;

    for (int idx=0; layout->itemAt(idx); ++idx) {
        QLayoutItem *item = layout->itemAt(idx);
        if (item->widget())
            objectToItem[item->widget()] = item;
        else if (item->layout())
            objectToItem[item->layout()] = item;
        else if (item->spacerItem())
            spacerItems.append(item);
        newList.append(item);
    }

    if (qobject_cast<QGridLayout *>(layout)) {
        newList.clear();
        QList<QObject *> childrenList = layout->parentWidget()->children();
        foreach (QObject *o, childrenList) {
            if (objectToItem.contains(o))
                newList.append(objectToItem[o]);
        }
        newList += spacerItems;
    }

    foreach (QLayoutItem *item, newList) {
        DomLayoutItem *ui_item = createDom(item, lay, ui_parentWidget);
        if (ui_item)
            ui_items.append(ui_item);
    }

    lay->setElementItem(ui_items);

    return lay;
}

/*!
    \internal
*/
DomLayoutItem *QAbstractFormBuilder::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
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

/*!
    \internal
*/
DomSpacer *QAbstractFormBuilder::createDom(QSpacerItem *spacer, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_layout);
    Q_UNUSED(ui_parentWidget);

    DomSpacer *ui_spacer = new DomSpacer();
    QList<DomProperty*> properties;

    DomProperty *prop = 0;

    // sizeHint property
    prop = new DomProperty();
    prop->setAttributeName(QLatin1String("sizeHint"));
    prop->setElementSize(new DomSize());
    prop->elementSize()->setElementWidth(spacer->sizeHint().width());
    prop->elementSize()->setElementHeight(spacer->sizeHint().height());
    properties.append(prop);

    // orientation property
    prop = new DomProperty(); // ### we don't implemented the case where expandingDirections() is both Vertical and Horizontal
    prop->setAttributeName(QLatin1String("orientation"));
    prop->setElementEnum((spacer->expandingDirections() & Qt::Horizontal) ? QLatin1String("Qt::Horizontal") : QLatin1String("Qt::Vertical"));
    properties.append(prop);

    ui_spacer->setElementProperty(properties);
    return ui_spacer;
}

/*!
    \internal
*/
DomProperty *QAbstractFormBuilder::createProperty(QObject *obj, const QString &pname, const QVariant &v)
{
    if (!checkProperty(obj, pname)) {
        return 0;
    }

    DomProperty *dom_prop = new DomProperty();

    const QMetaObject *meta = obj->metaObject();
    const int pindex = meta->indexOfProperty(pname.toLatin1());
    if (pindex != -1) {
        QMetaProperty meta_property = meta->property(pindex);
        if (!meta_property.hasStdCppSet())
            dom_prop->setAttributeStdset(0);
    }

    switch (v.type()) {
        case QVariant::String: {
            DomString *str = new DomString();
            str->setText(v.toString());

            if (pname == QLatin1String("objectName"))
                str->setAttributeNotr(QLatin1String("true"));

            dom_prop->setElementString(str);
        } break;

        case QVariant::ByteArray: {
            dom_prop->setElementCstring(QString::fromUtf8(v.toByteArray()));
        } break;

        case QVariant::Int: {
            dom_prop->setElementNumber(v.toInt());
        } break;

        case QVariant::UInt: {
            dom_prop->setElementUInt(v.toUInt());
        } break;

        case QVariant::LongLong: {
            dom_prop->setElementLongLong(v.toLongLong());
        } break;

        case QVariant::ULongLong: {
            dom_prop->setElementULongLong(v.toULongLong());
        } break;

        case QVariant::Double: {
            dom_prop->setElementDouble(v.toDouble());
        } break;

        case QVariant::Bool: {
            dom_prop->setElementBool(v.toBool() ? QLatin1String("true") : QLatin1String("false"));
        } break;

        case QVariant::Char: {
            DomChar *ch = new DomChar();
            const QChar character = v.toChar();
            ch->setElementUnicode(character.unicode());
            dom_prop->setElementChar(ch);
        } break;

        case QVariant::Point: {
            DomPoint *pt = new DomPoint();
            const QPoint point = v.toPoint();
            pt->setElementX(point.x());
            pt->setElementY(point.y());
            dom_prop->setElementPoint(pt);
        } break;

        case QVariant::PointF: {
            DomPointF *ptf = new DomPointF();
            const QPointF pointf = v.toPointF();
            ptf->setElementX(pointf.x());
            ptf->setElementY(pointf.y());
            dom_prop->setElementPointF(ptf);
        } break;

        case QVariant::Color: {
            DomColor *clr = new DomColor();
            const QColor color = qvariant_cast<QColor>(v);
            clr->setElementRed(color.red());
            clr->setElementGreen(color.green());
            clr->setElementBlue(color.blue());
            dom_prop->setElementColor(clr);
        } break;

        case QVariant::Size: {
            DomSize *sz = new DomSize();
            const QSize size = v.toSize();
            sz->setElementWidth(size.width());
            sz->setElementHeight(size.height());
            dom_prop->setElementSize(sz);
        } break;

        case QVariant::SizeF: {
            DomSizeF *szf = new DomSizeF();
            const QSizeF sizef = v.toSizeF();
            szf->setElementWidth(sizef.width());
            szf->setElementHeight(sizef.height());
            dom_prop->setElementSizeF(szf);
        } break;

        case QVariant::Rect: {
            DomRect *rc = new DomRect();
            const QRect rect = v.toRect();
            rc->setElementX(rect.x());
            rc->setElementY(rect.y());
            rc->setElementWidth(rect.width());
            rc->setElementHeight(rect.height());
            dom_prop->setElementRect(rc);
        } break;

        case QVariant::RectF: {
            DomRectF *rcf = new DomRectF();
            const QRectF rectf = v.toRectF();
            rcf->setElementX(rectf.x());
            rcf->setElementY(rectf.y());
            rcf->setElementWidth(rectf.width());
            rcf->setElementHeight(rectf.height());
            dom_prop->setElementRectF(rcf);
        } break;

        case QVariant::Font: {
            DomFont *fnt = new DomFont();
            const QFont font = qvariant_cast<QFont>(v);
            const uint mask = font.resolve();
            if (mask & QFontPrivate::Weight) {
                fnt->setElementBold(font.bold());
                fnt->setElementWeight(font.weight());
            }
            if (mask & QFontPrivate::Family)
                fnt->setElementFamily(font.family());
            if (mask & QFontPrivate::Style)
                fnt->setElementItalic(font.italic());
            if (mask & QFontPrivate::Size)
                fnt->setElementPointSize(font.pointSize());
            if (mask & QFontPrivate::StrikeOut)
                fnt->setElementStrikeOut(font.strikeOut());
            if (mask & QFontPrivate::Underline)
                fnt->setElementUnderline(font.underline());
            if (mask & QFontPrivate::Kerning)
                fnt->setElementKerning(font.kerning());
            if (mask & QFontPrivate::StyleStrategy) {
                const QMetaEnum styleStrategy_enum = metaEnum<QAbstractFormBuilderGadget>("styleStrategy");
                fnt->setElementStyleStrategy(styleStrategy_enum.valueToKey(font.styleStrategy()));
            }
            dom_prop->setElementFont(fnt);
        } break;

        case QVariant::Cursor: {
            const QMetaEnum cursorShape_enum = metaEnum<QAbstractFormBuilderGadget>("cursorShape");
            dom_prop->setElementCursorShape(cursorShape_enum.valueToKey(qvariant_cast<QCursor>(v).shape()));
        } break;

        case QVariant::KeySequence: {
            DomString *s = new DomString();
            s->setText(qvariant_cast<QKeySequence>(v).toString(QKeySequence::PortableText));
            dom_prop->setElementString(s);
        } break;

        case QVariant::Palette: {
            DomPalette *dom = new DomPalette();
            QPalette palette = qvariant_cast<QPalette>(v);

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
            const QSizePolicy sizePolicy = qvariant_cast<QSizePolicy>(v);

            dom->setElementHorStretch(sizePolicy.horizontalStretch());
            dom->setElementVerStretch(sizePolicy.verticalStretch());

            const QMetaEnum sizeType_enum = metaEnum<QAbstractFormBuilderGadget>("sizeType");

            dom->setAttributeHSizeType(sizeType_enum.valueToKey(sizePolicy.horizontalPolicy()));
            dom->setAttributeVSizeType(sizeType_enum.valueToKey(sizePolicy.verticalPolicy()));

            dom_prop->setElementSizePolicy(dom);
        } break;

        case QVariant::Date: {
            DomDate *dom = new DomDate();
            const QDate date = qvariant_cast<QDate>(v);

            dom->setElementYear(date.year());
            dom->setElementMonth(date.month());
            dom->setElementDay(date.day());

            dom_prop->setElementDate(dom);
        } break;

        case QVariant::Time: {
            DomTime *dom = new DomTime();
            const QTime time = qvariant_cast<QTime>(v);

            dom->setElementHour(time.hour());
            dom->setElementMinute(time.minute());
            dom->setElementSecond(time.second());

            dom_prop->setElementTime(dom);
        } break;

        case QVariant::DateTime: {
            DomDateTime *dom = new DomDateTime();
            const QDateTime dateTime = qvariant_cast<QDateTime>(v);

            dom->setElementHour(dateTime.time().hour());
            dom->setElementMinute(dateTime.time().minute());
            dom->setElementSecond(dateTime.time().second());
            dom->setElementYear(dateTime.date().year());
            dom->setElementMonth(dateTime.date().month());
            dom->setElementDay(dateTime.date().day());

            dom_prop->setElementDateTime(dom);
        } break;

        case QVariant::Url: {
            DomUrl *dom = new DomUrl();
            const QUrl url = v.toUrl();

            DomString *str = new DomString();
            str->setText(url.toString());
            dom->setElementString(str);

            dom_prop->setElementUrl(dom);
        } break;

        case QVariant::Pixmap:
            setPixmapProperty(*dom_prop, pixmapPaths(qvariant_cast<QPixmap>(v)));
            break;
        case QVariant::Icon:
            setIconProperty(*dom_prop, iconPaths(qvariant_cast<QIcon>(v)));
            break;

        case QVariant::StringList: {
            DomStringList *sl = new DomStringList;
            sl->setElementString(qvariant_cast<QStringList>(v));
            dom_prop->setElementStringList(sl);
        } break;

        default: {
            qWarning("support for property `%s' of type `%d' not implemented yet!!",
                pname.toUtf8().data(), v.type());
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

/*!
    \internal
*/
QList<DomProperty*> QAbstractFormBuilder::computeProperties(QObject *obj)
{
    QList<DomProperty*> lst;

    const QMetaObject *meta = obj->metaObject();

    QHash<QByteArray, bool> properties;
    for(int i=0; i<meta->propertyCount(); ++i)
        properties.insert(meta->property(i).name(), true);

    const QList<QByteArray> propertyNames = properties.keys();

    for(int i=0; i<propertyNames.size(); ++i) {
        const QString pname = QString::fromUtf8(propertyNames.at(i));
        const QMetaProperty prop = meta->property(meta->indexOfProperty(pname.toUtf8()));

        if (!prop.isWritable() || !checkProperty(obj, QLatin1String(prop.name())))
            continue;

        const QVariant v = prop.read(obj);

        DomProperty *dom_prop = 0;
        if (v.type() == QVariant::Int) {
            dom_prop = new DomProperty();

            if (prop.isFlagType())
                qWarning("flags property not supported yet!!");

            if (prop.isEnumType()) {
                QString scope = QString::fromUtf8(prop.enumerator().scope());
                if (scope.size())
                    scope += QString::fromUtf8("::");
                const QString e = QString::fromUtf8(prop.enumerator().valueToKey(v.toInt()));
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

/*!
    \internal
*/
bool QAbstractFormBuilder::toBool(const QString &str)
{
    return str.toLower() == QLatin1String("true");
}

/*!
    \internal
*/
QAbstractFormBuilder::DomPropertyHash QAbstractFormBuilder::propertyMap(const QList<DomProperty*> &properties)
{
    DomPropertyHash map;

    foreach (DomProperty *p, properties)
        map.insert(p->attributeName(), p);

    return map;
}

/*!
    \internal
*/
bool QAbstractFormBuilder::checkProperty(QObject *obj, const QString &prop) const
{
    Q_UNUSED(obj);
    Q_UNUSED(prop);

    return true;
}

/*!
    \internal
*/
QString QAbstractFormBuilder::toString(const DomString *str)
{
    return str ? str->text() : QString();
}

/*!
    \internal
*/
void QAbstractFormBuilder::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
    if (!tabStops)
        return;

    QWidget *lastWidget = 0;

    const QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        QWidget *child = qFindChild<QWidget*>(widget, name);
        if (!child) {
            qWarning("'%s' isn't a valid widget\n", name.toUtf8().data());
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

/*!
    \internal
*/
DomCustomWidgets *QAbstractFormBuilder::saveCustomWidgets()
{
    return 0;
}

/*!
    \internal
*/
DomTabStops *QAbstractFormBuilder::saveTabStops()
{
    return 0;
}

/*!
    \internal
*/
DomResources *QAbstractFormBuilder::saveResources()
{
    return 0;
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveTreeWidgetExtraInfo(QTreeWidget *treeWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    QList<DomColumn*> columns;

    // save the header
    for (int c = 0; c<treeWidget->columnCount(); ++c) {
        DomColumn *column = new DomColumn;

        QList<DomProperty*> properties;

        // property text
        DomProperty *ptext = new DomProperty;
        DomString *str = new DomString;
        str->setText(treeWidget->headerItem()->text(c));
        ptext->setAttributeName(QLatin1String("text"));
        ptext->setElementString(str);
        properties.append(ptext);

        if ( DomProperty *p = iconToDomProperty(treeWidget->headerItem()->icon(c)))
            properties.append(p);

        column->setElementProperty(properties);
        columns.append(column);
    }

    ui_widget->setElementColumn(columns);

    QList<DomItem *> items = ui_widget->elementItem();

    QQueue<QPair<QTreeWidgetItem *, DomItem *> > pendingQueue;
    for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
        pendingQueue.enqueue(qMakePair(treeWidget->topLevelItem(i), (DomItem *)0));

    while (!pendingQueue.isEmpty()) {
        const QPair<QTreeWidgetItem *, DomItem *> pair = pendingQueue.dequeue();
        QTreeWidgetItem *item = pair.first;
        DomItem *parentDomItem = pair.second;

        DomItem *currentDomItem = new DomItem;

        QList<DomProperty*> properties;
        for (int c = 0; c < treeWidget->columnCount(); c++) {
            DomProperty *ptext = new DomProperty;
            DomString *str = new DomString;
            str->setText(item->text(c));
            ptext->setAttributeName(QLatin1String("text"));
            ptext->setElementString(str);
            properties.append(ptext);

            if (DomProperty *p = iconToDomProperty(item->icon(c)))
                properties.append(p);
        }
        currentDomItem->setElementProperty(properties);

        if (parentDomItem) {
            QList<DomItem *> childrenItems = parentDomItem->elementItem();
            childrenItems.append(currentDomItem);
            parentDomItem->setElementItem(childrenItems);
        } else
            items.append(currentDomItem);

        for (int i = 0; i < item->childCount(); i++)
            pendingQueue.enqueue(qMakePair(item->child(i), currentDomItem));
    }

    ui_widget->setElementItem(items);
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveTableWidgetExtraInfo(QTableWidget *tableWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    // save the horizontal header
    QList<DomColumn*> columns;
    for (int c = 0; c < tableWidget->columnCount(); c++) {
        DomColumn *column = new DomColumn;
        QList<DomProperty*> properties;
        QTableWidgetItem *item = tableWidget->horizontalHeaderItem(c);
        if (item) {
            // property text
            DomProperty *ptext = new DomProperty;
            DomString *str = new DomString;
            str->setText(item->text());
            ptext->setAttributeName(QLatin1String("text"));
            ptext->setElementString(str);
            properties.append(ptext);

            if (DomProperty *p = iconToDomProperty(item->icon()))
                properties.append(p);
        }

        column->setElementProperty(properties);
        columns.append(column);
    }
    ui_widget->setElementColumn(columns);

    // save the vertical header
    QList<DomRow*> rows;
    for (int r = 0; r < tableWidget->rowCount(); r++) {
        DomRow *row = new DomRow;
        QList<DomProperty*> properties;
        QTableWidgetItem *item = tableWidget->verticalHeaderItem(r);
        if (item) {
            // property text
            DomProperty *ptext = new DomProperty;
            DomString *str = new DomString;
            str->setText(item->text());
            ptext->setAttributeName(QLatin1String("text"));
            ptext->setElementString(str);
            properties.append(ptext);

            if (DomProperty *p = iconToDomProperty(item->icon()))
                properties.append(p);
        }

        row->setElementProperty(properties);
        rows.append(row);
    }
    ui_widget->setElementRow(rows);

    QList<DomItem *> items = ui_widget->elementItem();

    for (int r = 0; r < tableWidget->rowCount(); r++)
        for (int c = 0; c < tableWidget->columnCount(); c++) {
            QTableWidgetItem *item = tableWidget->item(r, c);
            if (item) {
                DomItem *domItem = new DomItem;
                domItem->setAttributeRow(r);
                domItem->setAttributeColumn(c);
                QList<DomProperty*> properties;

                DomProperty *ptext = new DomProperty;

                DomString *str = new DomString;
                str->setText(item->text());
                ptext->setAttributeName(QLatin1String("text"));
                ptext->setElementString(str);
                properties.append(ptext);

                if (DomProperty *p = iconToDomProperty(item->icon()))
                    properties.append(p);

                domItem->setElementProperty(properties);
                items.append(domItem);
            }
        }

    ui_widget->setElementItem(items);
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveListWidgetExtraInfo(QListWidget *listWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    QList<DomItem*> ui_items = ui_widget->elementItem();

    for (int i=0; i<listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        DomItem *ui_item = new DomItem();

        QList<DomProperty*> properties;

        // text
        DomString *str = new DomString;
        str->setText(item->text());

        DomProperty *p = 0;

        p = new DomProperty;
        p->setAttributeName(QLatin1String("text"));
        p->setElementString(str);
        properties.append(p);

        if (DomProperty *p = iconToDomProperty(item->icon()))
            properties.append(p);

        ui_item->setElementProperty(properties);
        ui_items.append(ui_item);
    }

    ui_widget->setElementItem(ui_items);
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveComboBoxExtraInfo(QComboBox *comboBox, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    QList<DomItem*> ui_items = ui_widget->elementItem();

    for (int i=0; i<comboBox->count(); ++i) {
        DomItem *ui_item = new DomItem();

        QList<DomProperty*> properties;

        // text
        DomString *str = new DomString;
        str->setText(comboBox->itemText(i));

        DomProperty *p = 0;

        p = new DomProperty;
        p->setAttributeName(QLatin1String("text"));
        p->setElementString(str);
        properties.append(p);

        if (DomProperty *p = iconToDomProperty(qVariantValue<QIcon>(comboBox->itemData(i))))
                properties.append(p);

        ui_item->setElementProperty(properties);
        ui_items.append(ui_item);
    }

    ui_widget->setElementItem(ui_items);
}

/*!
    \internal
*/
void QAbstractFormBuilder::saveExtraInfo(QWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    if (QListWidget *listWidget = qobject_cast<QListWidget*>(widget)) {
        saveListWidgetExtraInfo(listWidget, ui_widget, ui_parentWidget);
    } else if (QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(widget)) {
        saveTreeWidgetExtraInfo(treeWidget, ui_widget, ui_parentWidget);
    } else if (QTableWidget *tableWidget = qobject_cast<QTableWidget*>(widget)) {
        saveTableWidgetExtraInfo(tableWidget, ui_widget, ui_parentWidget);
    } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
        if (!qobject_cast<QFontComboBox*>(widget))
            saveComboBoxExtraInfo(comboBox, ui_widget, ui_parentWidget);
    }
}

/*!
    \internal
*/
void QAbstractFormBuilder::loadListWidgetExtraInfo(DomWidget *ui_widget, QListWidget *listWidget, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    foreach (DomItem *ui_item, ui_widget->elementItem()) {
        const DomPropertyHash properties = propertyMap(ui_item->elementProperty());
        QListWidgetItem *item = new QListWidgetItem(listWidget);

        DomProperty *p = properties.value(QLatin1String("text"));
        if (p && p->kind() == DomProperty::String) {
            item->setText(p->elementString()->text());
        }

        p = properties.value(QLatin1String("icon"));
        if (p && p->kind() == DomProperty::IconSet) {
            item->setIcon(domPropertyToIcon(p));
        }
    }

    DomProperty *currentRow = propertyMap(ui_widget->elementProperty()).value("currentRow");
    if (currentRow)
        listWidget->setCurrentRow(currentRow->elementNumber());
}

/*!
    \internal
*/
void QAbstractFormBuilder::loadTreeWidgetExtraInfo(DomWidget *ui_widget, QTreeWidget *treeWidget, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    const QList<DomColumn*> columns = ui_widget->elementColumn();

    treeWidget->setColumnCount(columns.count());

    for (int i = 0; i<columns.count(); ++i) {
        const DomColumn *c = columns.at(i);
        const DomPropertyHash properties = propertyMap(c->elementProperty());

        DomProperty *ptext = properties.value(QLatin1String("text"));
        DomProperty *picon = properties.value(QLatin1String("icon"));

        if (ptext != 0 && ptext->elementString())
            treeWidget->headerItem()->setText(i, ptext->elementString()->text());

        if (picon && picon->kind() == DomProperty::IconSet) {
            treeWidget->headerItem()->setIcon(i, domPropertyToIcon(picon));
        }
    }

    QQueue<QPair<DomItem *, QTreeWidgetItem *> > pendingQueue;
    foreach (DomItem *ui_item, ui_widget->elementItem())
        pendingQueue.enqueue(qMakePair(ui_item, (QTreeWidgetItem *)0));

    while (!pendingQueue.isEmpty()) {
        const QPair<DomItem *, QTreeWidgetItem *> pair = pendingQueue.dequeue();
        const DomItem *domItem = pair.first;
        QTreeWidgetItem *parentItem = pair.second;

        QTreeWidgetItem *currentItem = 0;

        if (parentItem)
            currentItem = new QTreeWidgetItem(parentItem);
        else
            currentItem = new QTreeWidgetItem(treeWidget);

        const QList<DomProperty *> properties = domItem->elementProperty();
        int col = 0;
        foreach (DomProperty *property, properties) {
            if (property->attributeName() == QLatin1String("text") &&
                        property->elementString()) {
                currentItem->setText(col, property->elementString()->text());
                col++;
            } else if (property->attributeName() == QLatin1String("icon") &&
                        property->kind() == DomProperty::IconSet && col > 0) {
                currentItem->setIcon(col - 1, domPropertyToIcon(property));
            }
        }


        foreach (DomItem *childItem, domItem->elementItem())
            pendingQueue.enqueue(qMakePair(childItem, currentItem));

    }
}

/*!
    \internal
*/
void QAbstractFormBuilder::loadTableWidgetExtraInfo(DomWidget *ui_widget, QTableWidget *tableWidget, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    const QList<DomColumn*> columns = ui_widget->elementColumn();
    tableWidget->setColumnCount(columns.count());
    for (int i = 0; i< columns.count(); i++) {
        DomColumn *c = columns.at(i);
        const DomPropertyHash properties = propertyMap(c->elementProperty());

        const DomProperty *ptext = properties.value(QLatin1String("text"));
        const DomProperty *picon = properties.value(QLatin1String("icon"));

        if (ptext || picon) {
            QTableWidgetItem *item = new QTableWidgetItem;
            if (ptext != 0 && ptext->elementString()) {
                item->setText(ptext->elementString()->text());
            }

            if (picon && picon->kind() == DomProperty::IconSet) {
                item->setIcon(domPropertyToIcon(picon));
            }
            tableWidget->setHorizontalHeaderItem(i, item);
        }
    }

    const QList<DomRow*> rows = ui_widget->elementRow();
    tableWidget->setRowCount(rows.count());
    for (int i = 0; i< rows.count(); i++) {
        const DomRow *r = rows.at(i);
        const DomPropertyHash properties = propertyMap(r->elementProperty());

        const DomProperty *ptext = properties.value(QLatin1String("text"));
        const DomProperty *picon = properties.value(QLatin1String("icon"));

        if (ptext || picon) {
            QTableWidgetItem *item = new QTableWidgetItem;
            if (ptext != 0 && ptext->elementString()) {
                item->setText(ptext->elementString()->text());
            }

            if (picon && picon->kind() == DomProperty::IconSet) {
                item->setIcon(domPropertyToIcon(picon));
            }
            tableWidget->setVerticalHeaderItem(i, item);
        }
    }

    foreach (DomItem *ui_item, ui_widget->elementItem()) {
        if (ui_item->hasAttributeRow() && ui_item->hasAttributeColumn()) {
            QTableWidgetItem *item = new QTableWidgetItem;
            foreach (DomProperty *property, ui_item->elementProperty()) {
                if (property->attributeName() == QLatin1String("text") &&
                        property->elementString()) {
                    item->setText(property->elementString()->text());
                } else if (property->attributeName() == QLatin1String("icon") &&
                        property->kind() == DomProperty::IconSet) {
                    item->setIcon(domPropertyToIcon(property));
                }

            }
            tableWidget->setItem(ui_item->attributeRow(), ui_item->attributeColumn(), item);
        }
    }
}

/*!
    \internal
*/
void QAbstractFormBuilder::loadComboBoxExtraInfo(DomWidget *ui_widget, QComboBox *comboBox, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    foreach (DomItem *ui_item, ui_widget->elementItem()) {
        const DomPropertyHash properties = propertyMap(ui_item->elementProperty());
        QString text;
        QIcon icon;

        DomProperty *p = 0;

        p = properties.value(QLatin1String("text"));
        if (p && p->elementString()) {
            text = p->elementString()->text();
        }

        p = properties.value(QLatin1String("icon"));
        if (p && p->kind() == DomProperty::IconSet) {
             icon = domPropertyToIcon(p);
        }

        comboBox->addItem(icon, text);
        comboBox->setItemData((comboBox->count()-1), icon);
    }

    DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value("currentIndex");
    if (currentIndex)
        comboBox->setCurrentIndex(currentIndex->elementNumber());
}

/*!
    \internal
*/
void QAbstractFormBuilder::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (QListWidget *listWidget = qobject_cast<QListWidget*>(widget)) {
        loadListWidgetExtraInfo(ui_widget, listWidget, parentWidget);
    } else if (QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(widget)) {
        loadTreeWidgetExtraInfo(ui_widget, treeWidget, parentWidget);
    } else if (QTableWidget *tableWidget = qobject_cast<QTableWidget*>(widget)) {
        loadTableWidgetExtraInfo(ui_widget, tableWidget, parentWidget);
    } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
        if (!qobject_cast<QFontComboBox *>(widget))
            loadComboBoxExtraInfo(ui_widget, comboBox, parentWidget);
    } else if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(widget)) {
        DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value("currentIndex");
        if (currentIndex)
            tabWidget->setCurrentIndex(currentIndex->elementNumber());
    } else if (QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(widget)) {
        DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value("currentIndex");
        if (currentIndex)
            stackedWidget->setCurrentIndex(currentIndex->elementNumber());
    } else if (QToolBox *toolBox = qobject_cast<QToolBox*>(widget)) {
        DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value("currentIndex");
        if (currentIndex)
            toolBox->setCurrentIndex(currentIndex->elementNumber());
    }
}

/*!
    \internal
*/
QIcon QAbstractFormBuilder::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    Q_UNUSED(qrcPath);
    const QFileInfo fileInfo(workingDirectory(), filePath);
    return QIcon(fileInfo.absoluteFilePath());
}

/*!
    \internal
*/
QString QAbstractFormBuilder::iconToFilePath(const QIcon &pm) const
{
    Q_UNUSED(pm);
    return QString();
}

/*!
    \internal
*/
QString QAbstractFormBuilder::iconToQrcPath(const QIcon &pm) const
{
    Q_UNUSED(pm);
    return QString();
}

/*!
    \internal
*/
QPixmap QAbstractFormBuilder::nameToPixmap(const QString &filePath, const QString &qrcPath)
{
    Q_UNUSED(qrcPath);
    const QFileInfo fileInfo(workingDirectory(), filePath);
    return QPixmap(fileInfo.absoluteFilePath());
}

/*!
    \internal
*/
QString QAbstractFormBuilder::pixmapToFilePath(const QPixmap &pm) const
{
    Q_UNUSED(pm);
    return QString();
}

/*!
    \internal
*/
QString QAbstractFormBuilder::pixmapToQrcPath(const QPixmap &pm) const
{
    Q_UNUSED(pm);
    return QString();
}

/*!
    Returns the current working directory of the form builder.

    \sa setWorkingDirectory() */
QDir QAbstractFormBuilder::workingDirectory() const
{
    return m_workingDirectory;
}

/*!
    Sets the current working directory of the form builder to the
    specified \a directory.

    \sa workingDirectory()*/
void QAbstractFormBuilder::setWorkingDirectory(const QDir &directory)
{
    m_workingDirectory = directory;
}

/*!
    \internal
*/
DomAction *QAbstractFormBuilder::createDom(QAction *action)
{
    if (action->parentWidget() == action->menu() || action->isSeparator())
        return 0;

    DomAction *ui_action = new DomAction;
    ui_action->setAttributeName(action->objectName());

    const QList<DomProperty*> properties = computeProperties(action);
    ui_action->setElementProperty(properties);

    return ui_action;
}

/*!
    \internal
*/
DomActionGroup *QAbstractFormBuilder::createDom(QActionGroup *actionGroup)
{
    DomActionGroup *ui_action_group = new DomActionGroup;
    ui_action_group->setAttributeName(actionGroup->objectName());

    QList<DomProperty*> properties = computeProperties(actionGroup);
    ui_action_group->setElementProperty(properties);

    QList<DomAction*> ui_actions;

    foreach (QAction *action, actionGroup->actions()) {
        if (DomAction *ui_action = createDom(action)) {
            ui_actions.append(ui_action);
        }
    }

    ui_action_group->setElementAction(ui_actions);

    return ui_action_group;
}

/*!
    \internal
*/
void QAbstractFormBuilder::addMenuAction(QAction *action)
{
    Q_UNUSED(action);
}

/*!
    \internal
*/
void QAbstractFormBuilder::reset()
{
    m_laidout.clear();
    m_actions.clear();
    m_actionGroups.clear();
    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;
}
/*!
    \internal
    Access meta enumeration for Qt::ToolBarArea
*/

QMetaEnum QAbstractFormBuilder::toolBarAreaMetaEnum()
{
    return metaEnum<QAbstractFormBuilderGadget>("toolBarArea");
}

namespace {
    // set forward slashes in image path.
    inline void fixImagePath(QString &p)    {
        p.replace(QLatin1Char('\\'), QLatin1Char('/'));
    }
}

/*!
    \internal
    Return paths of an icon.
*/

QAbstractFormBuilder::IconPaths QAbstractFormBuilder::iconPaths(const QIcon &icon) const
{
    IconPaths rc(iconToFilePath(icon), iconToQrcPath(icon));
    fixImagePath(rc.first);
    fixImagePath(rc.second);
    return rc;
}

/*!
    \internal
    Return paths of a pixmap.
*/

QAbstractFormBuilder::IconPaths QAbstractFormBuilder::pixmapPaths(const QPixmap &pixmap) const
{
    IconPaths rc(pixmapToFilePath(pixmap), pixmapToQrcPath(pixmap));
    fixImagePath(rc.first);
    fixImagePath(rc.second);
    return rc;
}

/*!
    \internal
    Set up a DOM property with icon.
*/

void QAbstractFormBuilder::setIconProperty(DomProperty &p, const IconPaths &ip) const
{
    DomResourcePixmap *pix = new DomResourcePixmap;
    if (!ip.second.isEmpty())
        pix->setAttributeResource(ip.second);

    pix->setText(ip.first);

    p.setAttributeName(QLatin1String("icon"));
    p.setElementIconSet(pix);
}
 
/*!
    \internal
    Set up a DOM property with pixmap.
*/

void QAbstractFormBuilder::setPixmapProperty(DomProperty &p, const IconPaths &ip) const
{
    DomResourcePixmap *pix = new DomResourcePixmap;
    if (!ip.second.isEmpty())
        pix->setAttributeResource(ip.second);

    pix->setText(ip.first);

    p.setAttributeName(QLatin1String("pixmap"));
    p.setElementPixmap(pix);
}

/*!
    \internal
    Convenience. Return DOM property for icon; 0 if icon.isNull(). 
*/

DomProperty* QAbstractFormBuilder::iconToDomProperty(const QIcon &icon) const
{
    if (icon.isNull())
        return 0;
    DomProperty *p = new DomProperty;
    setIconProperty(*p, iconPaths(icon));
    return p;
}

/*!
    \internal
    Return the appropriate DOM pixmap for an image dom property.
*/

const DomResourcePixmap *QAbstractFormBuilder::domPixmap(const DomProperty* p) {
    switch (p->kind()) {
    case DomProperty::IconSet:
        return p->elementIconSet();
    case DomProperty::Pixmap:
        return p->elementPixmap();
    default:
        break;
    }
    return 0;
}

/*!
    \internal
    Create icon from DOM.
*/

QIcon QAbstractFormBuilder::domPropertyToIcon(const DomResourcePixmap *icon)
{
    const QString iconPath = icon->text();
    const QString qrcPath = icon->attributeResource();
    return nameToIcon(iconPath, qrcPath);
}

/*!
    \internal
    Create icon from DOM. Assert if !domPixmap
*/

QIcon QAbstractFormBuilder::domPropertyToIcon(const DomProperty* p)
{
    const DomResourcePixmap *px = domPixmap(p);
    Q_ASSERT(px);
    return domPropertyToIcon(px);
}


/*!
    \internal 
    Create pixmap from DOM.
*/

QPixmap QAbstractFormBuilder::domPropertyToPixmap(const DomResourcePixmap* pixmap)
{
    const QString iconPath = pixmap->text();
    const QString qrcPath = pixmap->attributeResource();
    return nameToPixmap(iconPath, qrcPath);
}


/*!
    \internal
    Create pixmap from DOM. Assert if !domPixmap
*/

QPixmap QAbstractFormBuilder::domPropertyToPixmap(const DomProperty* p)
{
    const DomResourcePixmap *px = domPixmap(p);
    Q_ASSERT(px);
    return domPropertyToPixmap(px);
}

/*!
    \fn void QAbstractFormBuilder::createConnections ( DomConnections *, QWidget * )
    \internal
*/

/*!
    \fn void QAbstractFormBuilder::createCustomWidgets ( DomCustomWidgets * )
    \internal
*/

/*!
    \fn void QAbstractFormBuilder::createResources ( DomResources * )
    \internal
*/

#include "abstractformbuilder.moc"
