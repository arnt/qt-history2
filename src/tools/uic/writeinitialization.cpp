/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "writeinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"

#include <qtextstream.h>
#include <qdebug.h>

static QString fixString(const QString &str, bool encode=false)
{
    QString s;
    if (!encode) {
        s = str;
        s.replace("\\", "\\\\");
        s.replace("\"", "\\\"");
        s.replace("\r", "");
        s.replace("\n", "\\n\"\n\"");
    } else {
        QByteArray utf8 = str.utf8();
        const int l = utf8.length();
        for (int i = 0; i < l; ++i)
            s += "\\x" + QString::number((uchar)utf8[i], 16);
    }

    return "\"" + s + "\"";
}


WriteInitialization::WriteInitialization(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option()),
      m_defaultMargin(0), m_defaultSpacing(0)
{
}

void WriteInitialization::accept(DomUI *node)
{
    m_widgetChain.push(node->elementWidget());
    m_layoutChain.push(0);

    accept(node->elementLayoutDefault());

    if (node->elementCustomWidgets())
        TreeWalker::accept(node->elementCustomWidgets());

    if (option.generateImplemetation)
        output << "#include <" << driver->headerFileName() << ">\n\n";

    m_stdsetdef = true;
    if (node->hasAttributeStdSetDef())
        m_stdsetdef = node->attributeStdSetDef();

    QString className = node->elementClass() + option.postfix;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    m_registerdWidgets.insert(varName, node->elementWidget()); // register the main widget

    QString widgetClassName = node->elementWidget()->attributeClass();

    output << "inline void " << className << "::setupUI(" << widgetClassName << " *" << varName << ")\n"
           << "{\n";

    accept(node->elementWidget());

    for (int i=0; i<m_buddies.size(); ++i) {
        const Buddy &b = m_buddies.at(i);

        if (!m_registerdWidgets.contains(b.objName)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.objName.latin1());
            continue;
        } else if (!m_registerdWidgets.contains(b.buddy)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.buddy.latin1());
            continue;
        }


        output << option.indent << b.objName << "->setBuddy(" << b.buddy << ");\n";
    }

    if (node->elementTabStops())
        accept(node->elementTabStops());

    if (option.autoConnection)
        output << "\n" << option.indent << "QMetaObject::connectSlotsByName(" << varName << ");\n";

    output << "}\n\n";

    m_layoutChain.pop();
    m_widgetChain.pop();
}

void WriteInitialization::accept(DomWidget *node)
{
    QString className = node->attributeClass();
    QString varName = driver->findOrInsertWidget(node);
    m_registerdWidgets.insert(varName, node); // register the current widget

    QString parentWidget, parentClass;
    if (m_widgetChain.top()) {
        parentWidget = driver->findOrInsertWidget(m_widgetChain.top());
        parentClass = m_widgetChain.top()->attributeClass();
    }

    QString savedParentWidget = parentWidget;

    if (parentClass == QLatin1String("QStackedBox") ||
            parentClass == QLatin1String("QToolBox") ||
            parentClass == QLatin1String("QTabWidget") ||
            parentClass == QLatin1String("QWidgetStack") ||
            parentClass == QLatin1String("QWizard"))
        parentWidget.clear();

    if (node != m_widgetChain.top())
        output << option.indent << varName << " = new " << className << "(" << parentWidget << ");\n";

    parentWidget = savedParentWidget;

    writePropertiesImpl(varName, className, node->elementProperty());

    if (className == QLatin1String("QListBox")) {
        initializeListBox(node);
    } else if (className.mid(1) == QLatin1String("ComboBox")) {
        initializeListBox(node);
    } else if (className.mid(1) == QLatin1String("ListView")) {
        initializeListView(node);
    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::accept(node);
    m_layoutChain.pop();
    m_widgetChain.pop();

    QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());

    QString title = fixString(QLatin1String("Page"));
    if (attributes.contains("title"))
        title = fixString(attributes.value("title")->elementString());

    QString label = fixString(QLatin1String("Page"));
    if (attributes.contains("label"))
        title = fixString(attributes.value("label")->elementString());

    int id = -1;
    if (attributes.contains("id"))
        id = attributes.value("id")->elementNumber();

    if (parentClass == QLatin1String("QStackedBox"))
        output << option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    else if (parentClass == QLatin1String("QWidgetStack"))
        output << option.indent << parentWidget << "->addWidget(" << varName << ", " << id << ");\n";
    else if (parentClass == QLatin1String("QToolBox"))
        output << option.indent << parentWidget << "->addItem(" << varName << ", " << translate(label, className) << ");\n";
    else if (parentClass == QLatin1String("QTabWidget"))
        output << option.indent << parentWidget << "->addTab(" << varName << ", " << translate(title, className) << ");\n";
    else if (parentClass == QLatin1String("QWizard"))
        output << option.indent << parentWidget << "->addPage(" << varName << ", " << translate(title, className) << ");\n";

    if (node->elementLayout().isEmpty())
        m_layoutChain.pop();
}

void WriteInitialization::accept(DomLayout *node)
{
    QString className = node->attributeClass(); // ### check if className is a valid layout
    QString varName = driver->findOrInsertLayout(node);

    QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    int margin = m_defaultMargin;
    int spacing = m_defaultSpacing;

    if (properties.contains("margin"))
        margin = properties.value("margin")->elementNumber();

    if (properties.contains("spacing"))
        spacing = properties.value("spacing")->elementNumber();

    bool isGroupBox = false;
    bool isMainWindow = false;
    QString centralWidget;

    if (m_widgetChain.top()) {
        QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (parentWidget == QLatin1String("Q3GroupBox") || parentWidget == QLatin1String("Q3ButtonGroup"))) {
            QString parent = driver->findOrInsertWidget(m_widgetChain.top());

            isGroupBox = true;

            // special case for group box
            output << option.indent << parent << "->setColumnLayout(0, Qt::Vertical);\n";
            output << option.indent << parent << "->layout()->setSpacing(" << spacing << ");\n";
            output << option.indent << parent << "->layout()->setMargin(" << margin << ");\n";
        } else if (m_widgetChain.top()->attributeClass() == QLatin1String("QMainWindow")) {
            QString parent = driver->findOrInsertWidget(m_widgetChain.top());

            isMainWindow = true;
            centralWidget = driver->findOrInsertName("__qt_central_widget");
            output << option.indent << "QWidget *" << centralWidget << "= new QWidget(" << parent << ");\n";
            output << option.indent << centralWidget << "->setObjectName(" << fixString(centralWidget) << ");\n";
            output << option.indent << parent << "->setCentralWidget(" << centralWidget << ");\n";
        }
    }

    //bool isMainWindow = m_widgetChain.top()->attributeClass() == QLatin1String("QMainWindow");

    output << option.indent << varName << " = new " << className << "(";

    if (isMainWindow) {
        output << centralWidget;
    } else if (isGroupBox) {
        output << driver->findOrInsertWidget(m_widgetChain.top()) << "->layout()";
    } else if (m_layoutChain.top()) {
        output << driver->findOrInsertLayout(m_layoutChain.top());
    } else {
        output << driver->findOrInsertWidget(m_widgetChain.top());
    }

    output << ");\n";

    if (isGroupBox)
        output << option.indent << varName << "->setAlignment(Qt::AlignTop);\n";

    if (!properties.contains("margin"))
        output << option.indent << varName << "->setMargin(" << m_defaultMargin << ");\n";

    if (!properties.contains("spacing"))
        output << option.indent << varName << "->setSpacing(" << m_defaultMargin << ");\n";

    writePropertiesImpl(varName, className, node->elementProperty());

    m_layoutChain.push(node);
    TreeWalker::accept(node);
    m_layoutChain.pop();
}

void WriteInitialization::accept(DomSpacer *node)
{
    QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());
    QString varName = driver->findOrInsertSpacer(node);

    DomSize *sizeHint = properties.contains("sizeHint") ? properties.value("sizeHint")->elementSize() : 0;
    QString sizeType = properties.contains("sizeType") ? properties.value("sizeType")->elementEnum() : QLatin1String("Expanding");
    bool isVspacer = properties.contains("orientation") ? properties.value("orientation")->elementEnum().toLower() == QLatin1String("vertical") : false;

    output << option.indent << varName << " = new QSpacerItem(";

    if (sizeHint)
        output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";

    if (isVspacer)
        output << "QSizePolicy::Minimum, QSizePolicy::" << sizeType << ");\n";
    else
        output << "QSizePolicy::" << sizeType << ", QSizePolicy::Minimum);\n";

    TreeWalker::accept(node);
}

QHash<QString, DomProperty *> WriteInitialization::propertyMap(const QList<DomProperty *> &properties)
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

void WriteInitialization::accept(DomLayoutItem *node)
{
    TreeWalker::accept(node);

    DomLayout *layout = m_layoutChain.top();

    if (!layout)
        return;

    QString varName = driver->findOrInsertLayoutItem(node);
    QString layoutName = driver->findOrInsertLayout(layout);

    QString opt;
    if (layout->attributeClass() == QLatin1String("QGridLayout")) {
        int row = node->attributeRow();
        int col = node->attributeColumn();

        int rowSpan = 1;
        if (node->hasAttributeRowSpan())
            rowSpan = node->attributeRowSpan();

        int colSpan = 1;
        if (node->hasAttributeColSpan())
            colSpan = node->attributeColSpan();

        opt = QString(", %1, %2, %3, %4").arg(row).arg(col).arg(rowSpan).arg(colSpan);
    }

    QString method = QLatin1String("addItem");
    switch (node->kind()) {
        case DomLayoutItem::Widget:
            method = QLatin1String("addWidget");
            break;
        case DomLayoutItem::Layout:
            method = QLatin1String("addLayout");
            break;
        case DomLayoutItem::Spacer:
            method = QLatin1String("addItem");
            break;
        case DomLayoutItem::Unknown:
            Q_ASSERT( 0 );
            break;
    }

    output << "\n" << option.indent << layoutName << "->" << method << "(" << varName << opt << ");\n\n";
}


void WriteInitialization::writePropertiesImpl(const QString &objName, const QString &objClass,
                                     const QList<DomProperty*> &lst)
{
    QString propertyName;
    QString propertyValue;
    bool stdset;
    bool isTopLevel = m_widgetChain.count() == 1;

    QString setFunction;

    for (int i=0; i<lst.size(); ++i) {
        DomProperty *p = lst.at(i);
        propertyName = p->attributeName();

        // special case for the property `geometry'
        if (isTopLevel && propertyName == QLatin1String("geometry") && p->elementRect()) {
            DomRect *r = p->elementRect();
            int w = r->elementWidth();
            int h = r->elementHeight();
            output << option.indent << objName << "->resize(QSize(" << w << ", " << h << ").expandedTo("
                << objName << "->minimumSizeHint()));\n";
            continue;
        }

        stdset = m_stdsetdef;
        if (p->hasAttributeStdset())
            stdset = p->attributeStdset();

        if (stdset)
            setFunction = "->set" + propertyName.left(1).toUpper()
                          + propertyName.mid(1) + "(";
        else
            setFunction = "->setProperty(\"" + propertyName + "\", QVariant(";

        switch (p->kind()) {
        case DomProperty::Bool: {
            propertyValue = p->elementBool();
            break;
        }
        case DomProperty::Color: {
            DomColor *c = p->elementColor();
            propertyValue = QString("QColor(%1, %2, %3)")
                  .arg(c->elementRed())
                  .arg(c->elementGreen())
                  .arg(c->elementBlue()); }
            break;
        case DomProperty::Cstring:
            if (propertyName == QLatin1String("buddy") && objClass == "QLabel") {
                m_buddies.append(Buddy(objName, p->elementCstring()));
            } else {
                propertyValue = fixString(p->elementCstring());
            }
            break;
        case DomProperty::Cursor:
            propertyValue = QString("QCursor(%1)")
                            .arg(p->elementCursor());
            break;
        case DomProperty::Enum:
            propertyValue = QString("%1::%2")
                            .arg(objClass)
                            .arg(p->elementEnum());
            break;
        case DomProperty::Font: {
            DomFont *f = p->elementFont();
            QString fontName = driver->findOrInsertName("font");
            output << option.indent << "QFont " << fontName << ";\n";
            output << option.indent << fontName << ".setFamily(" << fixString(f->elementFamily())
                << ");\n";
            output << option.indent << fontName << ".setPointSize(" << f->elementPointSize()
                << ");\n";
            output << option.indent << fontName << ".setBold("
                << (f->elementBold() ? "true" : "false") << ");\n";
            output << option.indent << fontName << ".setItalic("
                <<  (f->elementItalic() ? "true" : "false") << ");\n";
            output << option.indent << fontName << ".setUnderline("
                << (f->elementUnderline() ? "true" : "false") << ");\n";
            output << option.indent << fontName << ".setWeight("
                << f->elementWeight() << ");" << endl;
            output << option.indent << fontName << ".setStrikeOut("
                << (f->elementStrikeOut() ? "true" : "false") << ");\b";
            propertyValue = fontName;
            break;
        }
        case DomProperty::IconSet: {
            break;
        }
        case DomProperty::Palette: {
            DomPalette *pal = p->elementPalette();
            QString paletteName = driver->findOrInsertName("palette");
            output << option.indent << "QPalette " << paletteName << ";\b";

            writeColorGroup(pal->elementActive(), QLatin1String("QPalette::Active"), paletteName);
            writeColorGroup(pal->elementInactive(), QLatin1String("QPalette::Inactive"), paletteName);
            writeColorGroup(pal->elementDisabled(), QLatin1String("QPalette::Disabled"), paletteName);

            propertyValue = paletteName;
            break;
        }
        case DomProperty::Point: {
            DomPoint *po = p->elementPoint();
            propertyValue = QString("QPoint(%1, %2)")
                            .arg(po->elementX()).arg(po->elementY());
            break;
        }
        case DomProperty::Rect: {
            DomRect *r = p->elementRect();
            propertyValue = QString("QRect(%1, %2, %3, %4)")
                            .arg(r->elementX()).arg(r->elementY())
                            .arg(r->elementWidth()).arg(r->elementHeight());
            break;
        }
        case DomProperty::Set: {
            QString keys = p->elementSet();
            QStringList lst = keys.split('|');
            propertyValue = "int(";
            QStringList::ConstIterator it = lst.begin();
            for (int i = 0; i < lst.count(); ++i ) {
                propertyValue += objClass + "::" + lst.value(i);
                if (i != lst.count()-1)
                    propertyValue += " | ";
            }
            propertyValue += ")";
            break;
        }
        case DomProperty::SizePolicy: {
            DomSizePolicy *sp = p->elementSizePolicy();
            propertyValue = QString("QSizePolicy((QSizePolicy::SizeType)%1, "
                                    "(QSizePolicy::SizeType)%2, %3, %4, "
                                    "%5->sizePolicy().hasHeightForWidth())")
                            .arg(sp->elementHSizeType())
                            .arg(sp->elementVSizeType())
                            .arg(sp->elementHorStretch())
                            .arg(sp->elementVerStretch())
                            .arg(objName);
            break;
        }
        case DomProperty::Size: {
             DomSize *s = p->elementSize();
              propertyValue = QString("QSize(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::String: {
            propertyValue = translate(fixString(p->elementString()), objClass);
            break;
        }
        case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;
        case DomProperty::Date: {
            DomDate *d = p->elementDate();
            propertyValue = QString("QDate(%1, %2, %3)")
                            .arg(d->elementYear())
                            .arg(d->elementMonth())
                            .arg(d->elementDay());
            break;
        }
        case DomProperty::Time: {
            DomTime *t = p->elementTime();
            propertyValue = QString("QTime(%1, %2, %3)")
                            .arg(t->elementHour())
                            .arg(t->elementMinute())
                            .arg(t->elementSecond());
            break;
        }
        case DomProperty::DateTime: {
            DomDateTime *dt = p->elementDateTime();
            propertyValue = QString("QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))")
                            .arg(dt->elementYear())
                            .arg(dt->elementMonth())
                            .arg(dt->elementDay())
                            .arg(dt->elementHour())
                            .arg(dt->elementMinute())
                            .arg(dt->elementSecond());
            break;
        }
        default:
            continue;
        }

        if (propertyValue.size()) {
            output << option.indent << objName << setFunction << propertyValue;
            if (!stdset)
                output << ")";
            output << ");\n";
        }
    }
}

QString WriteInitialization::domColor2QString(DomColor *c)
{
    return QString("QColor(%1, %2, %3)")
        .arg(c->elementRed())
        .arg(c->elementGreen())
        .arg(c->elementBlue());
}

void WriteInitialization::writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName)
{
    if (!colorGroup)
        return;

    QList<DomColor*> colors = colorGroup->elementColor();
    for (int i=0; i<colors.size(); ++i) {
        DomColor *color = colors.at(i);

        output << option.indent << paletteName << ".setColor(" << group
            << ", " << "static_cast<QPalette::ColorRole>(" << i << ")"
            << ", " << domColor2QString(color)
            << ");\n";
    }
}

void WriteInitialization::accept(DomCustomWidget *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::accept(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::accept(DomTabStops *tabStops)
{
    QString lastName;

    QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        if (!m_registerdWidgets.contains(name)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", name.latin1());
            continue;
        }

        if (lastName.size())
            output << option.indent << "QWidget::setTabOrder(" << name << ", " << lastName << ");\n";

        lastName = name;
    }
}

QString WriteInitialization::translate(const QString &text, const QString &className) const
{
    if (option.translateFunction.size())
        return option.translateFunction + "(" + text + ")";

    Q_ASSERT( className.size() != 0 );

    return QLatin1String("(qApp->translate(\"") + className + QLatin1String("\", ") + text + QLatin1String("))");
}

void WriteInitialization::accept(DomLayoutDefault *node)
{
    m_defaultMargin = 6;
    m_defaultSpacing = 11;

    if (!node)
        return;

    if (node->hasAttributeMargin())
        m_defaultMargin = node->attributeMargin();
    if (node->hasAttributeSpacing())
        m_defaultSpacing = node->attributeSpacing();
}

void WriteInitialization::initializeListBox(DomWidget *w)
{
    QString varName = driver->findOrInsertWidget(w);
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value("text");
        if (!text)
            continue;

        output << option.indent << varName << "->insertItem("
            << translate(fixString(text->elementString()), className) << ");\n";
    }
}

void WriteInitialization::initializeListView(DomWidget *w)
{
    QString varName = driver->findOrInsertWidget(w);
    QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value("text");
        DomProperty *pixmap = properties.value("pixmap");
        DomProperty *clickable = properties.value("clickable");
        DomProperty *resizable = properties.value("resizable");

        QString txt = translate(fixString(text->elementString()), className);
        output << option.indent << varName << "->addColumn(" << txt << ");\n";

        if (pixmap) {
            output << option.indent << varName << "->header()->setLabel("
                   << varName << "->header()->count() - 1, " << pixCall(pixmap->elementIconSet()) << ", " << txt << ");\n";
        }

        if (!clickable)
            output << option.indent << varName << "->header()->setClickEnabled(false, " << varName << "->header()->count() - 1);\n";
        if (!resizable)
            output << option.indent << varName << "->header()->setResizeEnabled(false, " << varName << "->header()->count() - 1);\n";
    }

    // items
    QList<DomItem*> items = w->elementItem();
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->findOrInsertName("__item");
        output << "\n";
        output << option.indent << "QListViewItem *" << itemName << " = new QListViewItem(" << varName << ");\n";

        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                output << option.indent << itemName << "->setText(" << i << ", "
                       << trCall(p->elementString(), className) << ");\n";
        }
    }
}

QString WriteInitialization::pixCall(const QString &pix) const
{
    QString s = pix;

    if (!m_pixmapFunction.isEmpty()) {
        s.prepend(m_pixmapFunction + "(\"");
        s.append("\")");
    } else {
        s = QString("icon(%1_ID)").arg(s);
    }

    return QString("QIconSet(%1)").arg(s);
}

QString WriteInitialization::trCall(const QString &str, const QString &className) const
{
    return translate(fixString(str), className);
}

