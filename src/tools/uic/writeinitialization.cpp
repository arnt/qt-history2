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
#include "uic.h"
#include "databaseinfo.h"

#include <qtextstream.h>
#include <qdebug.h>

WriteInitialization::WriteInitialization(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option()),
      m_defaultMargin(0), m_defaultSpacing(0), m_externPixmap(true)
{
    this->uic = uic;
}

void WriteInitialization::accept(DomUI *node)
{
    m_actionGroupChain.push(0);
    m_widgetChain.push(0);
    m_layoutChain.push(0);

    m_pixmapFunction = node->elementPixmapFunction();

    m_externPixmap = node->elementImages() == 0;
    if (m_externPixmap)
        m_pixmapFunction = QLatin1String("QPixmap::fromMimeSource");

    accept(node->elementLayoutDefault());

    if (node->elementCustomWidgets())
        TreeWalker::accept(node->elementCustomWidgets());

    if (option.generateImplemetation)
        output << "#include <" << driver->headerFileName() << ">\n\n";

    m_stdsetdef = true;
    if (node->hasAttributeStdSetDef())
        m_stdsetdef = node->attributeStdSetDef();

    QString className = node->elementClass() + option.postfix;
    m_generatedClass = className;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    m_registeredWidgets.insert(varName, node->elementWidget()); // register the main widget

    QString widgetClassName = node->elementWidget()->attributeClass();

    output << "inline void " << className << "::setupUi(" << widgetClassName << " *" << varName << ")\n"
           << "{\n";

    QStringList connections = uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        QString varConn = connection + QLatin1String("Connection");
        output << option.indent << varConn << " = QSqlDatabase::database(" << fixString(connection) << ");\n";
    }

    accept(node->elementWidget());

    for (int i=0; i<m_buddies.size(); ++i) {
        const Buddy &b = m_buddies.at(i);

        if (!m_registeredWidgets.contains(b.objName)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.objName.latin1());
            continue;
        } else if (!m_registeredWidgets.contains(b.buddy)) {
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
    m_actionGroupChain.pop();
}

void WriteInitialization::accept(DomWidget *node)
{
    QString className = node->attributeClass();
    QString varName = driver->findOrInsertWidget(node);
    m_registeredWidgets.insert(varName, node); // register the current widget

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

    if (m_widgetChain.size() != 1)
        output << option.indent << varName << " = new " << className << "(" << parentWidget << ");\n";

    parentWidget = savedParentWidget;

    if (className == QLatin1String("QListBox")) {
        initializeListBox(node);
    } else if (className == QLatin1String("QIconView")) {
        initializeIconView(node);
    } else if (className == QLatin1String("QTable")) {
        initializeTable(node);
    } else if (className.mid(1) == QLatin1String("ComboBox")) {
        initializeListBox(node);
    } else if (className == QLatin1String("Q3ListView")) {
        initializeListView(node);
    } else if (className == QLatin1String("QDataTable")) {
        initializeSqlDataTable(node);
    } else if (className == QLatin1String("QDataBrowser")) {
        initializeSqlDataBrowser(node);
    }

    writeProperties(varName, className, node->elementProperty());

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::accept(node);
    m_layoutChain.pop();
    m_widgetChain.pop();

    QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());

    QString title = QLatin1String("Page");
    if (attributes.contains("title"))
        title = attributes.value("title")->elementString();

    QString label = QLatin1String("Page");
    if (attributes.contains("label"))
        label = attributes.value("label")->elementString();

    int id = -1;
    if (attributes.contains("id"))
        id = attributes.value("id")->elementNumber();

    if (parentClass == QLatin1String("QStackedBox"))
        output << option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    else if (parentClass == QLatin1String("QWidgetStack"))
        output << option.indent << parentWidget << "->addWidget(" << varName << ", " << id << ");\n";
    else if (parentClass == QLatin1String("QToolBox"))
        output << option.indent << parentWidget << "->addItem(" << varName << ", " << trCall(label, className) << ");\n";
    else if (parentClass == QLatin1String("QTabWidget"))
        output << option.indent << parentWidget << "->addTab(" << varName << ", " << trCall(title, className) << ");\n";
    else if (parentClass == QLatin1String("QWizard"))
        output << option.indent << parentWidget << "->addPage(" << varName << ", " << trCall(title, className) << ");\n";
    else if (parentClass == QLatin1String("QMenuBar")
            || parentClass == QLatin1String("QMenu") && className == QLatin1String("QMenu"))
        output << option.indent << parentWidget << "->addMenu(" << trCall(title, className) << ", " << varName << ");\n";

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
    QString centerWidget;

    if (m_widgetChain.top()) {
        QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (parentWidget == QLatin1String("Q3GroupBox") || parentWidget == QLatin1String("Q3ButtonGroup"))) {
            QString parent = driver->findOrInsertWidget(m_widgetChain.top());

            isGroupBox = true;

            // special case for group box
            output << option.indent << parent << "->setColumnLayout(0, Qt::Vertical);\n";
            output << option.indent << parent << "->layout()->setSpacing(" << spacing << ");\n";
            output << option.indent << parent << "->layout()->setMargin(" << margin << ");\n";
        } else if (m_widgetChain.top()->attributeClass() == QLatin1String("QMainWindow") ||
                m_widgetChain.top()->attributeClass() == QLatin1String("Q3MainWindow")) {
            QString parent = driver->findOrInsertWidget(m_widgetChain.top());

            isMainWindow = true;
            centerWidget = driver->unique("__qt_center_widget");
            output << option.indent << "QWidget *" << centerWidget << "= new QWidget(" << parent << ");\n";
            output << option.indent << centerWidget << "->setObjectName(" << fixString(centerWidget) << ");\n";

            if (m_widgetChain.top()->attributeClass() == QLatin1String("Q3MainWindow")) {
                output << option.indent << parent << "->setCentralWidget(" << centerWidget << ");\n";
            } else {
                output << option.indent << parent << "->setCenterWidget(" << centerWidget << ");\n";
            }
        }
    }

    //bool isMainWindow = m_widgetChain.top()->attributeClass() == QLatin1String("QMainWindow");

    output << option.indent << varName << " = new " << className << "(";

    if (isMainWindow) {
        output << centerWidget;
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

    writeProperties(varName, className, node->elementProperty());

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

void WriteInitialization::accept(DomActionGroup *node)
{
    QString actionName = driver->findOrInsertActionGroup(node);
    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = driver->findOrInsertActionGroup(m_actionGroupChain.top());

    output << option.indent << actionName << " = new QActionGroup(" << varName << ");\n";
    output << option.indent << actionName << "->setExclusive(false);\n";
    writeProperties(actionName, "QActionGroup", node->elementProperty());

    m_actionGroupChain.push(node);
    TreeWalker::accept(node);
    m_actionGroupChain.pop();
}

void WriteInitialization::accept(DomAction *node)
{
    QString actionName = driver->findOrInsertAction(node);
    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = driver->findOrInsertActionGroup(m_actionGroupChain.top());

    output << option.indent << actionName << " = new QAction(" << varName << ");\n";
    writeProperties(actionName, "QAction", node->elementProperty());
}

void WriteInitialization::accept(DomActionRef *node)
{
    QString actionName = node->attributeName();
    if (actionName.isEmpty() || !m_widgetChain.top())
        return;

    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_widgetChain.top() && actionName == QLatin1String("separator")) {
        QString parentClass = m_widgetChain.top()->attributeClass();
        if (parentClass == QLatin1String("QMenuBar")) {
            // ### separator in menubar are not supported!
            return;
        } else {
            // separator is always reserved!
            output << option.indent << varName << "->addSeparator();\n";
            return;
        }
    }

    output << option.indent << varName << "->addAction(" << node->attributeName() << ");\n";
}

void WriteInitialization::writeProperties(const QString &varName, const QString &className,
                                     const QList<DomProperty*> &lst)
{
    bool isTopLevel = m_widgetChain.count() == 1;

    if (className == QLatin1String("QAxWidget")) {
        QHash<QString, DomProperty*> properties = propertyMap(lst);
        if (properties.contains("control")) {
            DomProperty *p = properties.value("control");
            output << option.indent << varName << "->setControl("
                   << fixString(p->elementString()) << ");\n";
        }
    }

    for (int i=0; i<lst.size(); ++i) {
        DomProperty *p = lst.at(i);
        QString propertyName = p->attributeName();
        QString propertyValue;

        // special case for the property `geometry'
        if (isTopLevel && propertyName == QLatin1String("geometry") && p->elementRect()) {
            DomRect *r = p->elementRect();
            int w = r->elementWidth();
            int h = r->elementHeight();
            output << option.indent << varName << "->resize(QSize(" << w << ", " << h << ").expandedTo("
                << varName << "->minimumSizeHint()));\n";
            continue;
        } else if (propertyName == QLatin1String("buttonGroupId") // Q3ButtonGroup support
                    && p->elementNumber()
                    && m_widgetChain.top() && m_widgetChain.top()->attributeClass() == QLatin1String("Q3ButtonGroup")) {
            output << option.indent << driver->findOrInsertWidget(m_widgetChain.top()) << "->insert("
                   << varName << ", " << p->elementNumber() << ");\n";
            continue;
        } else if (propertyName == QLatin1String("control") // ActiveQt support
                    && className == QLatin1String("QAxWidget")) {
            // already done ;)
            continue;
        } else if (propertyName == QLatin1String("database")
                    && p->elementStringList()) {
            // Sql support
            continue;
        } else if (propertyName == QLatin1String("frameworkCode")
                    && p->kind() == DomProperty::Bool) {
            // Sql support
            continue;
        }

        bool stdset = m_stdsetdef;
        if (p->hasAttributeStdset())
            stdset = p->attributeStdset();

        QString setFunction = stdset
                ? QLatin1String("->set") + propertyName.left(1).toUpper() + propertyName.mid(1) + QLatin1String("(")
                : QLatin1String("->setProperty(\"") + propertyName + QLatin1String("\", QVariant(");

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
            if (propertyName == QLatin1String("buddy") && className == QLatin1String("QLabel")) {
                m_buddies.append(Buddy(varName, p->elementCstring()));
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
                            .arg(className)
                            .arg(p->elementEnum());
            break;
        case DomProperty::Font: {
            DomFont *f = p->elementFont();
            QString fontName = driver->unique("font");
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
        case DomProperty::IconSet:
            propertyValue = pixCall(p->elementIconSet());
            break;

        case DomProperty::Pixmap:
            propertyValue = pixCall(p->elementPixmap());
            break;

        case DomProperty::Palette: {
            DomPalette *pal = p->elementPalette();
            QString paletteName = driver->unique("palette");
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
                propertyValue += className + "::" + lst.value(i);
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
                            .arg(varName);
            break;
        }
        case DomProperty::Size: {
             DomSize *s = p->elementSize();
              propertyValue = QString("QSize(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::String: {
            propertyValue = trCall(p->elementString(), className);
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
        case DomProperty::Shortcut:
            propertyValue = QString("QKeySequence(%1)")
                            .arg(fixString(p->elementShortcut()));
            break;

        case DomProperty::StringList:
            propertyValue = QLatin1String("QStringList()");
            if (p->elementStringList()->elementString().size()) {
                QStringList lst = p->elementStringList()->elementString();
                for (int i=0; i<lst.size(); ++i) {
                    propertyValue += QLatin1String(" << ") + fixString(lst.at(i));
                }
            }
            break;

        case DomProperty::Unknown:
            break;
        }

        if (propertyValue.size()) {
            output << option.indent << varName << setFunction << propertyValue;
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

        if (!m_registeredWidgets.contains(name)) {
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

    Q_UNUSED(className);

    return QLatin1String("qApp->translate(\"") + m_generatedClass + QLatin1String("\", ") + text + QLatin1String(")");
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
        DomProperty *pixmap = properties.value("pixmap");
        if (!(text || pixmap))
            continue;

        output << option.indent << varName << "->insertItem(";
        if (pixmap) {
            output << pixCall(pixmap->elementPixmap());

            if (text)
                output << ", ";
        }
        output << trCall(text->elementString(), className) << ");\n";
    }
}

void WriteInitialization::initializeIconView(DomWidget *w)
{
    QString varName = driver->findOrInsertWidget(w);
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value("text");
        DomProperty *pixmap = properties.value("pixmap");
        if (!(text || pixmap))
            continue;

        QString itemName = driver->unique("__item");
        output << "\n";
        output << option.indent << "QIconViewItem *" << itemName << " = new QIconViewItem(" << varName << ");\n";

        if (pixmap) {
            output << option.indent << itemName << "->setPixmap(" << pixCall(pixmap->elementPixmap()) << ");\n";
        }

        if (text) {
            output << option.indent << itemName << "->setText(" << trCall(text->elementString(), className) << ");\n";
        }
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

        QString txt = trCall(text->elementString(), className);
        output << option.indent << varName << "->addColumn(" << txt << ");\n";

        if (pixmap) {
            output << option.indent << varName << "->header()->setLabel("
                   << varName << "->header()->count() - 1, " << pixCall(pixmap->elementIconSet()) << ", " << txt << ");\n";
        }

        if (!clickable) {
            output << option.indent << varName << "->header()->setClickEnabled(false, " << varName << "->header()->count() - 1);\n";
        }

        if (!resizable) {
            output << option.indent << varName << "->header()->setResizeEnabled(false, " << varName << "->header()->count() - 1);\n";
        }
    }

    initializeListViewItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeListViewItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->unique("__item");
        output << "\n";
        output << option.indent << "Q3ListViewItem *" << itemName << " = new Q3ListViewItem(" << varName << ");\n";

        int textCount = 0, pixCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                output << option.indent << itemName << "->setText(" << textCount++ << ", "
                       << trCall(p->elementString(), className) << ");\n";

            if (p->attributeName() == QLatin1String("pixmap"))
                output << option.indent << itemName << "->setPixmap(" << pixCount++ << ", "
                       << pixCall(p->elementPixmap()) << ");\n";
        }

        if (item->elementItem().size()) {
            output << option.indent << itemName << "->setOpen(true);\n";
            initializeListViewItems(className, itemName, item->elementItem());
        }
    }
}

void WriteInitialization::initializeTable(DomWidget *w)
{
    QString varName = driver->findOrInsertWidget(w);
    QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();
    output << option.indent << varName << "->setNumCols(" << columns.size() << ");\n";

    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value("text");
        DomProperty *pixmap = properties.value("pixmap");

        output << option.indent << varName << "->horizontalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            output << pixCall(pixmap->elementPixmap()) << ", ";
        }
        output << trCall(text->elementString(), className) << ");\n";
    }

    // rows
    QList<DomRow*> rows = w->elementRow();
    output << option.indent << varName << "->setNumRows(" << rows.size() << ");\n";

    for (int i=0; i<rows.size(); ++i) {
        DomRow *row = rows.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        DomProperty *text = properties.value("text");
        DomProperty *pixmap = properties.value("pixmap");

        output << option.indent << varName << "->verticalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            output << pixCall(pixmap->elementPixmap()) << ", ";
        }
        output << trCall(text->elementString(), className) << ");\n";
    }


    //initializeTableItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeTableItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    Q_UNUSED(className);
    Q_UNUSED(varName);
    Q_UNUSED(items);
}

QString WriteInitialization::pixCall(const QString &pix) const
{
    QString s = pix;

    bool declaredPix = driver->containsPixmap(pix);
    if (s.isEmpty() || m_externPixmap || m_pixmapFunction.size() || !declaredPix) {
        if (m_externPixmap || !declaredPix)
            s = "\"" + s + "\"";

        if (m_pixmapFunction.size())
            s = m_pixmapFunction + "(" + s + ")";

        return QString("QPixmap(%1)").arg(s);
    }

    return QLatin1String("icon(") + s + QLatin1String("_ID)");
}

QString WriteInitialization::trCall(const QString &str, const QString &className) const
{
    return translate(fixString(str), className);
}

void WriteInitialization::initializeSqlDataTable(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value("frameworkCode", 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value("database", 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = driver->findOrInsertWidget(w);

    output << option.indent << "if (!" << varName << "->sqlCursor()) {\n";

    output << option.indent << option.indent << varName << "->setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        output << "new QSqlCursor(" << fixString(table) << "), false, true);\n";
    } else {
        output << "new QSqlCursor(" << fixString(table) << ", true, " << connection << "Connection" << "), false, true);\n";
    }
    output << option.indent << option.indent << varName << "->refresh(QDataTable::RefreshAll);\n";
    output << option.indent << "}\n";
}

void WriteInitialization::initializeSqlDataBrowser(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value("frameworkCode", 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value("database", 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = driver->findOrInsertWidget(w);

    output << option.indent << "if (!" << varName << "->sqlCursor()) {\n";

    output << option.indent << option.indent << varName << "->setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        output << "new QSqlCursor(" << fixString(table) << "), true);\n";
    } else {
        output << "new QSqlCursor(" << fixString(table) << ", true, " << connection << "Connection" << "), false, true);\n";
    }
    output << option.indent << option.indent << varName << "->refresh();\n";
    output << option.indent << "}\n";
}


