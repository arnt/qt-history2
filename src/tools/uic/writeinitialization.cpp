/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "globaldefs.h"

#include <qtextstream.h>
#include <qdebug.h>

#include <limits.h>

WriteInitialization::WriteInitialization(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option()),
      m_defaultMargin(INT_MIN), m_defaultSpacing(INT_MIN),
      refreshOut(&m_delayedInitialization, QIODevice::WriteOnly),
      actionOut(&m_delayedActionInitialization, QIODevice::WriteOnly)
{
    this->uic = uic;
}

void WriteInitialization::acceptUI(DomUI *node)
{
    m_actionGroupChain.push(0);
    m_widgetChain.push(0);
    m_layoutChain.push(0);

    acceptLayoutDefault(node->elementLayoutDefault());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

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

    if(uic->isMainWindow(widgetClassName) &&
       (node->elementWidget() && !node->elementWidget()->elementWidget().isEmpty())) {
        QString centralWidget = driver->unique(QLatin1String("__qt_center_widget"));
        output << option.indent << "QWidget *" << centralWidget << "= new QWidget(" << varName << ");\n";
        output << option.indent << centralWidget << "->setObjectName(" << fixString(centralWidget) << ");\n";
        output << option.indent << varName << "->setCentralWidget(" << centralWidget << ");\n";
    }

    acceptWidget(node->elementWidget());

    for (int i=0; i<m_buddies.size(); ++i) {
        const Buddy &b = m_buddies.at(i);

        if (!m_registeredWidgets.contains(b.objName)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.objName.toLatin1().data());
            continue;
        } else if (!m_registeredWidgets.contains(b.buddy)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.buddy.toLatin1().data());
            continue;
        }


        output << option.indent << b.objName << "->setBuddy(" << b.buddy << ");\n";
    }

    if (node->elementTabStops())
        acceptTabStops(node->elementTabStops());

    if (m_delayedActionInitialization.size())
        output << "\n" << m_delayedActionInitialization;

    output << option.indent << "retranslateUi(" << varName << ");\n";

    if (node->elementConnections())
        acceptConnections(node->elementConnections());

    if (option.autoConnection)
        output << "\n" << option.indent << "QMetaObject::connectSlotsByName(" << varName << ");\n";

    output << "}\n\n";

    output << "inline void " << className << "::retranslateUi(" << widgetClassName << " *" << varName << ")\n"
           << "{\n"
           << m_delayedInitialization
           << "}\n\n";

    m_layoutChain.pop();
    m_widgetChain.pop();
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptWidget(DomWidget *node)
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

    if (uic->isContainer(parentClass))
        parentWidget.clear();

    if (uic->isMainWindow(parentClass) && !uic->isStatusBar(className)
            && !uic->isToolBar(className) && !uic->isMenuBar(className)) {
        if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QMainWindow")))
            parentWidget += QLatin1String("->centralWidget()");
        else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow")))
            parentWidget += QLatin1String("->centralWidget()");
    }

    if (m_widgetChain.size() != 1)
        output << option.indent << varName << " = new " << className << "(" << parentWidget << ");\n";

    parentWidget = savedParentWidget;

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QListBox"))) {
        initializeListBox(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QComboBox"))) {
        initializeListBox(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))) {
        initializeListView(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QIconView"))) {
        initializeIconView(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QTable"))) {
        initializeTable(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QDataTable"))) {
        initializeSqlDataTable(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QDataBrowser"))) {
        initializeSqlDataBrowser(node);
    }

    if (uic->isButton(className)) {
        QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());
        if (attributes.contains(QLatin1String("buttonGroup"))) {
            DomProperty *prop = attributes.value(QLatin1String("buttonGroup"));
            QString groupName = toString(prop->elementString());
            if (!m_buttonGroups.contains(groupName)) {
                m_buttonGroups.insert(groupName, driver->findOrInsertName(groupName));
                QString g = m_buttonGroups.value(groupName);
                output << option.indent << "QButtonGroup *" << g << " = new QButtonGroup(" << m_generatedClass << ");\n";
            }

            QString g = m_buttonGroups.value(groupName);
            output << option.indent << g << "->addButton(" << varName << ");\n";
        }
    }

    writeProperties(varName, className, node->elementProperty());

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QMenu")) && parentWidget.size()) {
        initializeMenu(node, parentWidget);
    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::acceptWidget(node);
    m_layoutChain.pop();
    m_widgetChain.pop();

    QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());

    QString title = QLatin1String("Page");
    if (attributes.contains(QLatin1String("title")))
        title = toString(attributes.value(QLatin1String("title"))->elementString());

    QString label = QLatin1String("Page");
    if (attributes.contains(QLatin1String("label")))
        label = toString(attributes.value(QLatin1String("label"))->elementString());

    int id = -1;
    if (attributes.contains(QLatin1String("id")))
        id = attributes.value(QLatin1String("id"))->elementNumber();

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"))
        && !uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow"))) {
        output << option.indent << parentWidget << "->setMenuBar(" << varName <<");\n";
    }

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QToolBar"))) {
        output << option.indent << parentWidget << "->addToolBar(" << varName << ");\n";
    }

    if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QStackedWidget"))) {
        output << option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBar"))) {
        output << option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QWidgetStack"))) {
        output << option.indent << parentWidget << "->addWidget(" << varName << ", " << id << ");\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBox"))) {
        output << option.indent << parentWidget << "->addItem(" << varName << ", " << trCall(label, className) << ");\n";

        refreshOut << option.indent << parentWidget << "->setItemText("
                   << parentWidget << "->indexOf(" << varName << "), " << trCall(label, className) << ");\n";

    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QTabWidget"))) {
        output << option.indent << parentWidget << "->addTab(" << varName << ", " << trCall(title, className) << ");\n";

        refreshOut << option.indent << parentWidget << "->setTabText("
                   << parentWidget << "->indexOf(" << varName << "), " << trCall(title, className) << ");\n";

    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QWizard"))) {
        output << option.indent << parentWidget << "->addPage(" << varName << ", " << trCall(title, className) << ");\n";

        refreshOut << option.indent << parentWidget << "->setTitle("
                   << varName << ", " << trCall(title, className) << ");\n";

    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.pop();
}

void WriteInitialization::acceptLayout(DomLayout *node)
{
    QString className = node->attributeClass();
    QString varName = driver->findOrInsertLayout(node);

    QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    bool isGroupBox = false;
    bool isMainWindow = false;

    if (m_widgetChain.top()) {
        QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3GroupBox"))
                        || uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3ButtonGroup")))) {
            QString parent = driver->findOrInsertWidget(m_widgetChain.top());

            isGroupBox = true;

            // special case for group box

            int margin = m_defaultMargin;
            int spacing = m_defaultSpacing;

            if (properties.contains(QLatin1String("margin")))
                margin = properties.value(QLatin1String("margin"))->elementNumber();

            if (properties.contains(QLatin1String("spacing")))
                spacing = properties.value(QLatin1String("spacing"))->elementNumber();

            output << option.indent << parent << "->setColumnLayout(0, Qt::Vertical);\n";

            if (spacing != INT_MIN)
                output << option.indent << parent << "->layout()->setSpacing(" << spacing << ");\n";

            if (margin != INT_MIN)
                output << option.indent << parent << "->layout()->setMargin(" << margin << ");\n";
        } else if (uic->isMainWindow(parentWidget)) {
            isMainWindow = true;
        }
    }

    output << option.indent << varName << " = new " << className << "(";

    if (isMainWindow) {
        output << driver->findOrInsertWidget(m_widgetChain.top()) << "->centralWidget()";
    } else if (isGroupBox) {
        output << driver->findOrInsertWidget(m_widgetChain.top()) << "->layout()";
    } else if (!m_layoutChain.top()) {
        output << driver->findOrInsertWidget(m_widgetChain.top());
    }

    output << ");\n";

    if (isGroupBox)
        output << option.indent << varName << "->setAlignment(Qt::AlignTop);\n";
    else {
        if (!m_layoutChain.top() && !properties.contains(QLatin1String("margin")) && m_defaultMargin != INT_MIN)
                output << option.indent << varName << "->setMargin(" << m_defaultMargin << ");\n";

        if (!properties.contains(QLatin1String("spacing")) && m_defaultSpacing != INT_MIN)
            output << option.indent << varName << "->setSpacing(" << m_defaultSpacing << ");\n";
    }

    writeProperties(varName, className, node->elementProperty());

    m_layoutChain.push(node);
    TreeWalker::acceptLayout(node);
    m_layoutChain.pop();
}

void WriteInitialization::acceptSpacer(DomSpacer *node)
{
    QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());
    QString varName = driver->findOrInsertSpacer(node);

    DomSize *sizeHint = properties.contains(QLatin1String("sizeHint"))
        ? properties.value(QLatin1String("sizeHint"))->elementSize() : 0;

    QString sizeType = properties.contains(QLatin1String("sizeType"))
        ? properties.value(QLatin1String("sizeType"))->elementEnum()
        : QString::fromLatin1("Expanding");

    QString orientation = properties.contains(QLatin1String("orientation"))
        ? properties.value(QLatin1String("orientation"))->elementEnum() : QString();

    bool isVspacer = orientation == QLatin1String("Qt::Vertical")
        || orientation == QLatin1String("Vertical");

    output << option.indent << varName << " = new QSpacerItem(";

    if (sizeHint)
        output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";

    if (sizeType.startsWith(QLatin1String("QSizePolicy::")) == false)
        sizeType.prepend(QLatin1String("QSizePolicy::"));

    if (isVspacer)
        output << "QSizePolicy::Minimum, " << sizeType << ");\n";
    else
        output << sizeType << ", QSizePolicy::Minimum);\n";

    TreeWalker::acceptSpacer(node);
}

void WriteInitialization::acceptLayoutItem(DomLayoutItem *node)
{
    TreeWalker::acceptLayoutItem(node);

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

        opt = QString::fromLatin1(", %1, %2, %3, %4").arg(row).arg(col).arg(rowSpan).arg(colSpan);
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

void WriteInitialization::acceptActionGroup(DomActionGroup *node)
{
    QString actionName = driver->findOrInsertActionGroup(node);
    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = driver->findOrInsertActionGroup(m_actionGroupChain.top());

    output << option.indent << actionName << " = new QActionGroup(" << varName << ");\n";
    writeProperties(actionName, QLatin1String("QActionGroup"), node->elementProperty());

    m_actionGroupChain.push(node);
    TreeWalker::acceptActionGroup(node);
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptAction(DomAction *node)
{
    if (node->hasAttributeMenu())
        return;

    QString actionName = driver->findOrInsertAction(node);
    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = driver->findOrInsertActionGroup(m_actionGroupChain.top());

    output << option.indent << actionName << " = new QAction(" << varName << ");\n";
    writeProperties(actionName, QLatin1String("QAction"), node->elementProperty());
}

void WriteInitialization::acceptActionRef(DomActionRef *node)
{
    QString actionName = node->attributeName();
    bool isSeparator = actionName == QLatin1String("separator");
    bool isMenu = false;

    if (actionName.isEmpty() || !m_widgetChain.top()) {
        return;
    } else if (driver->actionGroupByName(actionName)) {
        return;
    } else if (DomWidget *w = driver->widgetByName(actionName)) {
        isMenu = uic->isMenu(w->attributeClass());
    } else if (!(driver->actionByName(actionName) || isSeparator)) {
        fprintf(stderr, "Warning: action `%s' not declared\n",
            actionName.toLatin1().data());
        return;
    }

    QString varName = driver->findOrInsertWidget(m_widgetChain.top());

    if (m_widgetChain.top() && isSeparator) {
        // separator is always reserved!
        actionOut << option.indent << varName << "->addSeparator();\n";
        return;
    }

    if (isMenu)
        actionName += QLatin1String("->menuAction()");

    actionOut << option.indent << varName << "->addAction(" << actionName << ");\n";
}

void WriteInitialization::writeProperties(const QString &varName,
                                          const QString &className,
                                          const QList<DomProperty*> &lst)
{
    bool isTopLevel = m_widgetChain.count() == 1;

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
        QHash<QString, DomProperty*> properties = propertyMap(lst);
        if (properties.contains(QLatin1String("control"))) {
            DomProperty *p = properties.value(QLatin1String("control"));
            output << option.indent << varName << "->setControl(QString::fromUtf8("
                   << fixString(toString(p->elementString())) << "));\n";
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
                    && m_widgetChain.top()
                    && uic->customWidgetsInfo()->extends(m_widgetChain.top()->attributeClass(), QLatin1String("Q3ButtonGroup"))) {
            output << option.indent << driver->findOrInsertWidget(m_widgetChain.top()) << "->insert("
                   << varName << ", " << p->elementNumber() << ");\n";
            continue;
        } else if (propertyName == QLatin1String("control") // ActiveQt support
                    && uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
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
            propertyValue = QString::fromLatin1("QColor(%1, %2, %3)")
                  .arg(c->elementRed())
                  .arg(c->elementGreen())
                  .arg(c->elementBlue()); }
            break;
        case DomProperty::Cstring:
            if (propertyName == QLatin1String("buddy") && uic->customWidgetsInfo()->extends(className, QLatin1String("QLabel"))) {
                m_buddies.append(Buddy(varName, p->elementCstring()));
            } else {
                propertyValue = fixString(p->elementCstring());
            }
            break;
        case DomProperty::Cursor:
            propertyValue = QString::fromLatin1("QCursor(static_cast<Qt::CursorShape(%1))")
                            .arg(p->elementCursor());
            break;
        case DomProperty::Enum:
            propertyValue = p->elementEnum();
            if (!propertyValue.contains(QLatin1String("::")))
                propertyValue.prepend(className + QLatin1String(QLatin1String("::")));
            break;
        case DomProperty::Set:
            propertyValue = p->elementSet();
            break;
        case DomProperty::Font: {
            DomFont *f = p->elementFont();
            QString fontName = driver->unique(QLatin1String("font"));
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
                << (f->elementStrikeOut() ? "true" : "false") << ");\n";
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
            QString paletteName = driver->unique(QLatin1String("palette"));
            output << option.indent << "QPalette " << paletteName << ";\n";

            writeColorGroup(pal->elementActive(), QLatin1String("QPalette::Active"), paletteName);
            writeColorGroup(pal->elementInactive(), QLatin1String("QPalette::Inactive"), paletteName);
            writeColorGroup(pal->elementDisabled(), QLatin1String("QPalette::Disabled"), paletteName);

            propertyValue = paletteName;
            break;
        }
        case DomProperty::Point: {
            DomPoint *po = p->elementPoint();
            propertyValue = QString::fromLatin1("QPoint(%1, %2)")
                            .arg(po->elementX()).arg(po->elementY());
            break;
        }
        case DomProperty::Rect: {
            DomRect *r = p->elementRect();
            propertyValue = QString::fromLatin1("QRect(%1, %2, %3, %4)")
                            .arg(r->elementX()).arg(r->elementY())
                            .arg(r->elementWidth()).arg(r->elementHeight());
            break;
        }
        case DomProperty::SizePolicy: {
            DomSizePolicy *sp = p->elementSizePolicy();
            QString spName = driver->unique(QLatin1String("sizePolicy"));
            output << option.indent << "QSizePolicy " << spName << QString::fromLatin1(
                "((QSizePolicy::Policy)%1, (QSizePolicy::Policy)%2);\n")
                            .arg(sp->elementHSizeType())
                            .arg(sp->elementVSizeType());
            output << option.indent << spName << ".setHorizontalStretch("
                << sp->elementHorStretch() << ");\n";
            output << option.indent << spName << ".setVerticalStretch("
                << sp->elementVerStretch() << ");\n";
            output << option.indent << spName << QString::fromLatin1(
                ".setHeightForWidth(%1->sizePolicy().hasHeightForWidth());\n")
                .arg(varName);

            propertyValue = spName;
            break;
        }
        case DomProperty::Size: {
             DomSize *s = p->elementSize();
              propertyValue = QString::fromLatin1("QSize(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::String: {
            if (p->elementString()->hasAttributeNotr()
                    && toBool(p->elementString()->attributeNotr())) {
                propertyValue = QLatin1String("QString::fromUtf8(")
                        + fixString(p->elementString()->text())
                        + QLatin1String(")");
            } else {
                propertyValue = trCall(p->elementString(), className);
            }
            break;
        }
        case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;
        case DomProperty::Date: {
            DomDate *d = p->elementDate();
            propertyValue = QString::fromLatin1("QDate(%1, %2, %3)")
                            .arg(d->elementYear())
                            .arg(d->elementMonth())
                            .arg(d->elementDay());
            break;
        }
        case DomProperty::Time: {
            DomTime *t = p->elementTime();
            propertyValue = QString::fromLatin1("QTime(%1, %2, %3)")
                            .arg(t->elementHour())
                            .arg(t->elementMinute())
                            .arg(t->elementSecond());
            break;
        }
        case DomProperty::DateTime: {
            DomDateTime *dt = p->elementDateTime();
            propertyValue = QString::fromLatin1("QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))")
                            .arg(dt->elementYear())
                            .arg(dt->elementMonth())
                            .arg(dt->elementDay())
                            .arg(dt->elementHour())
                            .arg(dt->elementMinute())
                            .arg(dt->elementSecond());
            break;
        }
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
            QTextStream *o = &output;

            if (p->kind() == DomProperty::String
                    && (!p->elementString()->hasAttributeNotr() || !toBool(p->elementString()->attributeNotr())))
                o = &refreshOut;

            (*o) << option.indent << varName << setFunction << propertyValue;
            if (!stdset)
                (*o) << ")";
            (*o) << ");\n";
        }
    }
}

QString WriteInitialization::domColor2QString(DomColor *c)
{
    return QString::fromLatin1("QColor(%1, %2, %3)")
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

void WriteInitialization::acceptCustomWidget(DomCustomWidget *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptCustomWidgets(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptTabStops(DomTabStops *tabStops)
{
    QString lastName;

    QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        if (!m_registeredWidgets.contains(name)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", name.toLatin1().data());
            continue;
        }

        if (i == 0) {
            lastName = name;
            continue;
        } else if (name.isEmpty() || lastName.isEmpty()) {
            continue;
        }

        output << option.indent << "QWidget::setTabOrder(" << lastName << ", " << name << ");\n";

        lastName = name;
    }
}

QString WriteInitialization::translate(const QString &text, const QString &className) const
{
    if (option.translateFunction.size())
        return option.translateFunction + QLatin1String("(") + text + QLatin1String(")");

    Q_UNUSED(className);

    return QLatin1String("QApplication::translate(\"") + m_generatedClass + QLatin1String("\", ") + text + QLatin1String(")");
}

void WriteInitialization::acceptLayoutDefault(DomLayoutDefault *node)
{
    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;

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

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << "->clear();\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        refreshOut << option.indent << varName << "->insertItem(";
        if (pixmap) {
            refreshOut << pixCall(pixmap->elementPixmap());

            if (text)
                refreshOut << ", ";
        }
        refreshOut << trCall(text->elementString(), className) << ");\n";
    }
}

void WriteInitialization::initializeIconView(DomWidget *w)
{
    QString varName = driver->findOrInsertWidget(w);
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << "->clear();\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << "QIconViewItem *" << itemName << " = new QIconViewItem(" << varName << ");\n";

        if (pixmap) {
            refreshOut << option.indent << itemName << "->setPixmap(" << pixCall(pixmap->elementPixmap()) << ");\n";
        }

        if (text) {
            refreshOut << option.indent << itemName << "->setText(" << trCall(text->elementString(), className) << ");\n";
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
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        DomProperty *clickable = properties.value(QLatin1String("clickable"));
        DomProperty *resizable = properties.value(QLatin1String("resizable"));

        QString txt = trCall(text->elementString(), className);
        output << option.indent << varName << "->addColumn(" << txt << ");\n";
        refreshOut << option.indent << varName << "->header()->setLabel(" << i << ", " << txt << ");\n";

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

    if (w->elementItem().size()) {
        refreshOut << option.indent << varName << "->clear();\n";

        initializeListViewItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeListViewItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << "Q3ListViewItem *" << itemName << " = new Q3ListViewItem(" << varName << ");\n";

        int textCount = 0, pixCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << "->setText(" << textCount++ << ", "
                           << trCall(p->elementString(), className) << ");\n";

            if (p->attributeName() == QLatin1String("pixmap"))
                refreshOut << option.indent << itemName << "->setPixmap(" << pixCount++ << ", "
                           << pixCall(p->elementPixmap()) << ");\n";
        }

        if (item->elementItem().size()) {
            refreshOut << option.indent << itemName << "->setOpen(true);\n";
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
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << option.indent << varName << "->horizontalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap->elementPixmap()) << ", ";
        }
        refreshOut << trCall(text->elementString(), className) << ");\n";
    }

    // rows
    QList<DomRow*> rows = w->elementRow();
    refreshOut << option.indent << varName << "->setNumRows(" << rows.size() << ");\n";

    for (int i=0; i<rows.size(); ++i) {
        DomRow *row = rows.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << option.indent << varName << "->verticalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap->elementPixmap()) << ", ";
        }
        refreshOut << trCall(text->elementString(), className) << ");\n";
    }


    //initializeTableItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeTableItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    Q_UNUSED(className);
    Q_UNUSED(varName);
    Q_UNUSED(items);
}

QString WriteInitialization::pixCall(DomResourcePixmap *r) const
{
    QString pix = r->text();
    QString s = pix;

    bool declaredPix = driver->containsPixmap(pix);
    if (s.isEmpty() || uic->hasExternalPixmap() || uic->pixmapFunction().size() || !declaredPix) {
        QString pixFunc = uic->pixmapFunction();

        if (pixFunc.isEmpty() && !s.isEmpty())
            pixFunc = QLatin1String("QString::fromUtf8");

        if (uic->hasExternalPixmap() || !declaredPix)
			s = fixString(s);
            
        if (pixFunc.isEmpty() && s == QLatin1String("\"\""))
            s.clear();

        if (!pixFunc.isEmpty())
            s = pixFunc + QLatin1String("(") + s + QLatin1String(")");

        s = QLatin1String("QPixmap(") + s + QLatin1String(")");

        return s;
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

    DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QLatin1String("database"), 0);
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

    DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QLatin1String("database"), 0);
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

void WriteInitialization::initializeMenu(DomWidget *w, const QString &/*parentWidget*/)
{
    QString menuName = driver->findOrInsertWidget(w);
    QString menuAction = menuName + QLatin1String("Action");

    DomAction *action = driver->actionByName(menuAction);
    if (action && action->hasAttributeMenu()) {
        output << option.indent << menuAction << " = " << menuName << "->menuAction();\n";
    }
}

QString WriteInitialization::trCall(const DomString *str, const QString &className) const
{
    return trCall(toString(str), className);
}

void WriteInitialization::acceptConnection(DomConnection *connection)
{
    if (!m_registeredWidgets.contains(connection->elementSender())
            || !m_registeredWidgets.contains(connection->elementReceiver()))
        return;

    output << option.indent << "QObject::connect("
        << connection->elementSender()
        << ", "
        << "SIGNAL(" << connection->elementSignal() << ")"
        << ", "
        << connection->elementReceiver()
        << ", "
        << "SLOT(" << connection->elementSlot() << ")"
        << ");\n";
}
