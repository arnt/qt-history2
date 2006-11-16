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

#include "cppwriteinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"
#include "databaseinfo.h"
#include "globaldefs.h"

#include <QTextStream>
#include <QtDebug>

#include <limits.h>

namespace {
    // Fixup an enumeration name from class Qt.
    // They are currently stored as "BottomToolBarArea" instead of "Qt::BottomToolBarArea".
    // due to MO issues. This might be fixed in the future.
    void fixQtEnumerationName(QString& name) {
        static const QLatin1String prefix("Qt::");
        if (name.indexOf(prefix) != 0) 
            name.prepend(prefix);
    }
    // figure out the toolbar area of a DOM attrib list.
    // By legacy, it is stored as an integer. As of 4.3.0, it is the enumeration value.
    QString toolBarAreaStringFromDOMAttributes(const QHash<QString, DomProperty*> &attributes) {
        const DomProperty *pstyle = attributes.value(QLatin1String("toolBarArea"));
        if (!pstyle)
            return QString();
        
        switch (pstyle->kind()) {
        case DomProperty::Number: {
            QString area = QLatin1String("static_cast<Qt::ToolBarArea>(");
            area += QString::number(pstyle->elementNumber());
            area += "), ";
            return area;
        }
        case DomProperty::Enum: {
            QString area = pstyle->elementEnum();
            fixQtEnumerationName(area);
            area += ", ";
            return area;
        }
        default:
            break;
        }
        return QString();
    }
}

namespace CPP {

WriteInitialization::WriteInitialization(Uic *uic)
    : m_uic(uic),
      m_driver(uic->driver()), m_output(uic->output()), m_option(uic->option()),
      m_defaultMargin(INT_MIN), m_defaultSpacing(INT_MIN),
      delayedOut(&m_delayedInitialization, QIODevice::WriteOnly),
      refreshOut(&m_refreshInitialization, QIODevice::WriteOnly),
      actionOut(&m_delayedActionInitialization, QIODevice::WriteOnly),
      resizeOut(&m_delayedResize, QIODevice::WriteOnly)
{
}

void WriteInitialization::acceptUI(DomUI *node)
{
    m_registeredImages.clear();
    m_actionGroupChain.push(0);
    m_widgetChain.push(0);
    m_layoutChain.push(0);

    acceptLayoutDefault(node->elementLayoutDefault());
    acceptLayoutFunction(node->elementLayoutFunction());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    if (node->elementImages())
        TreeWalker::acceptImages(node->elementImages());

    if (m_option.generateImplemetation)
        m_output << "#include <" << m_driver->headerFileName() << ">\n\n";

    m_stdsetdef = true;
    if (node->hasAttributeStdSetDef())
        m_stdsetdef = node->attributeStdSetDef();

    const QString className = node->elementClass() + m_option.postfix;
    m_generatedClass = className;

    const QString varName = m_driver->findOrInsertWidget(node->elementWidget());
    m_registeredWidgets.insert(varName, node->elementWidget()); // register the main widget

    const QString widgetClassName = node->elementWidget()->attributeClass();

    m_output << m_option.indent << "void " << "setupUi(" << widgetClassName << " *" << varName << ")\n"
           << m_option.indent << "{\n";

    const QStringList connections = m_uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        const QString varConn = connection + QLatin1String("Connection");
        m_output << m_option.indent << varConn << " = QSqlDatabase::database(" << fixString(connection, m_option.indent) << ");\n";
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

        m_output << m_option.indent << b.objName << "->setBuddy(" << b.buddy << ");\n";
    }

    if (node->elementTabStops())
        acceptTabStops(node->elementTabStops());

    if (m_delayedActionInitialization.size())
        m_output << "\n" << m_delayedActionInitialization;

    m_output << "\n" << m_option.indent << "retranslateUi(" << varName << ");\n";

    if (!m_delayedResize.isEmpty())
        m_output << "\n" << m_delayedResize << "\n";

    if (node->elementConnections())
        acceptConnections(node->elementConnections());

    if (!m_delayedInitialization.isEmpty())
        m_output << "\n" << m_delayedInitialization << "\n";

    if (m_option.autoConnection)
        m_output << "\n" << m_option.indent << "QMetaObject::connectSlotsByName(" << varName << ");\n";

    m_output << m_option.indent << "} // setupUi\n\n";

    if (m_delayedActionInitialization.isEmpty()) {
        m_refreshInitialization += m_option.indent + QLatin1String("Q_UNUSED(") + varName + QLatin1String(");\n");
    }

    m_output << m_option.indent << "void " << "retranslateUi(" << widgetClassName << " *" << varName << ")\n"
           << m_option.indent << "{\n"
           << m_refreshInitialization
           << m_option.indent << "} // retranslateUi\n\n";

    m_layoutChain.pop();
    m_widgetChain.pop();
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptWidget(DomWidget *node)
{
    const QString className = node->attributeClass();
    const QString varName = m_driver->findOrInsertWidget(node);
    m_registeredWidgets.insert(varName, node); // register the current widget

    QString parentWidget, parentClass;
    if (m_widgetChain.top()) {
        parentWidget = m_driver->findOrInsertWidget(m_widgetChain.top());
        parentClass = m_widgetChain.top()->attributeClass();
    }

    const QString savedParentWidget = parentWidget;

    if (m_uic->isContainer(parentClass) || m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3ToolBar")))
        parentWidget.clear();

    if (m_widgetChain.size() != 1)
        m_output << m_option.indent << varName << " = new " << m_uic->customWidgetsInfo()->realClassName(className) << "(" << parentWidget << ");\n";

    parentWidget = savedParentWidget;

    if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QComboBox"))) {
        initializeComboBox(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QListWidget"))) {
        initializeListWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QTreeWidget"))) {
        initializeTreeWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QTableWidget"))) {
        initializeTableWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListBox"))) {
        initializeQ3ListBox(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))) {
        initializeQ3ListView(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3IconView"))) {
        initializeQ3IconView(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3Table"))) {
        initializeQ3Table(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3DataTable"))) {
        initializeQ3SqlDataTable(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3DataBrowser"))) {
        initializeQ3SqlDataBrowser(node);
    }

    if (m_uic->isButton(className)) {
        const QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());
        if (const DomProperty *prop = attributes.value(QLatin1String("buttonGroup"))) {
            const QString groupName = toString(prop->elementString());
            if (!m_buttonGroups.contains(groupName)) {
                m_buttonGroups.insert(groupName, m_driver->findOrInsertName(groupName));
                const QString g = m_buttonGroups.value(groupName);
                m_output << m_option.indent << "QButtonGroup *" << g << " = new QButtonGroup(" << m_generatedClass << ");\n";
            }

            const QString g = m_buttonGroups.value(groupName);
            m_output << m_option.indent << g << "->addButton(" << varName << ");\n";
        }
    }

    writeProperties(varName, className, node->elementProperty());

    if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QMenu")) && parentWidget.size()) {
        initializeMenu(node, parentWidget);
    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::acceptWidget(node);
    m_layoutChain.pop();
    m_widgetChain.pop();

    const QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());

    QString title = QLatin1String("Page");
    if (const DomProperty *ptitle = attributes.value(QLatin1String("title"))) {
        title = toString(ptitle->elementString());
    }

    QString label = QLatin1String("Page");
    if (const DomProperty *plabel = attributes.value(QLatin1String("label"))) {
        label = toString(plabel->elementString());
    }

    int id = -1;
    if (const DomProperty *pid = attributes.value(QLatin1String("id"))) {
        id = pid->elementNumber();
    }

    if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QMainWindow"))
            || m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow"))) {

        if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"))) {
            if (!m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow")))
                m_output << m_option.indent << parentWidget << "->setMenuBar(" << varName <<");\n";
        } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QToolBar"))) {
            m_output << m_option.indent << parentWidget << "->addToolBar(" 
                     << toolBarAreaStringFromDOMAttributes(attributes) << varName << ");\n";
            
            if (const DomProperty *pbreak = attributes.value(QLatin1String("toolBarBreak"))) {
                if (pbreak->elementBool() == QLatin1String("true")) {
                    m_output << m_option.indent << parentWidget << "->insertToolBarBreak(" <<  varName << ");\n";          
                }
            }

        } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QDockWidget"))) {
            QString area;
            if (DomProperty *pstyle = attributes.value(QLatin1String("dockWidgetArea"))) {
                area += QLatin1String("static_cast<Qt::DockWidgetArea>(");
                area += QString::number(pstyle->elementNumber());
                area += "), ";
            }

            m_output << m_option.indent << parentWidget << "->addDockWidget(" << area << varName << ");\n";
        } else if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QStatusBar"))) {
            m_output << m_option.indent << parentWidget << "->setStatusBar(" << varName << ");\n";
        } else if (className == QLatin1String("QWidget")) {
            m_output << m_option.indent << parentWidget << "->setCentralWidget(" << varName << ");\n";
        }
    }

    if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QStackedWidget"))) {
        m_output << m_option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBar"))) {
        m_output << m_option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3WidgetStack"))) {
        m_output << m_option.indent << parentWidget << "->addWidget(" << varName << ", " << id << ");\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QDockWidget"))) {
        m_output << m_option.indent << parentWidget << "->setWidget(" << varName << ");\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QSplitter"))) {
        m_output << m_option.indent << parentWidget << "->addWidget(" << varName << ");\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBox"))) {
        QString icon;
        if (const DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            icon += QLatin1String(", ") + pixCall(picon);
        }

        m_output << m_option.indent << parentWidget << "->addItem(" << varName << icon << ", " << trCall(label) << ");\n";

        refreshOut << m_option.indent << parentWidget << "->setItemText("
                   << parentWidget << "->indexOf(" << varName << "), " << trCall(label) << ");\n";

#ifndef QT_NO_TOOLTIP
        if (DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            refreshOut << m_option.indent << parentWidget << "->setItemToolTip("
                       << parentWidget << "->indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ");\n";
        }
#endif // QT_NO_TOOLTIP
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QTabWidget"))) {
        QString icon;
        if (const DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            icon += QLatin1String(", ") + pixCall(picon);
        }

        m_output << m_option.indent << parentWidget << "->addTab(" << varName << icon << ", " << trCall(title) << ");\n";

        refreshOut << m_option.indent << parentWidget << "->setTabText("
                   << parentWidget << "->indexOf(" << varName << "), " << trCall(title) << ");\n";

#ifndef QT_NO_TOOLTIP
        if (const DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            refreshOut << m_option.indent << parentWidget << "->setTabToolTip("
                       << parentWidget << "->indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ");\n";
        }
#endif // QT_NO_TOOLTIP
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3Wizard"))) {
        m_output << m_option.indent << parentWidget << "->addPage(" << varName << ", " << trCall(title) << ");\n";

        refreshOut << m_option.indent << parentWidget << "->setTitle("
                   << varName << ", " << trCall(title) << ");\n";

    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.pop();
}

void WriteInitialization::acceptLayout(DomLayout *node)
{
    const QString className = node->attributeClass();
    const QString varName = m_driver->findOrInsertLayout(node);

    const QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    bool isGroupBox = false;

    if (m_widgetChain.top()) {
        const QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (m_uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3GroupBox"))
                        || m_uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3ButtonGroup")))) {
            const QString parent = m_driver->findOrInsertWidget(m_widgetChain.top());

            isGroupBox = true;

            // special case for group box

            int margin = m_defaultMargin;
            int spacing = m_defaultSpacing;

            if (properties.contains(QLatin1String("margin")))
                margin = properties.value(QLatin1String("margin"))->elementNumber();

            if (properties.contains(QLatin1String("spacing")))
                spacing = properties.value(QLatin1String("spacing"))->elementNumber();

            m_output << m_option.indent << parent << "->setColumnLayout(0, Qt::Vertical);\n";

            if (spacing != INT_MIN) {
                QString value = QString::number(spacing);
                if (!m_spacingFunction.isEmpty() && spacing == m_defaultSpacing)
                    value = m_spacingFunction + QLatin1String("()");

                m_output << m_option.indent << parent << "->layout()->setSpacing(" << value << ");\n";
            }

            if (margin != INT_MIN) {
                QString value = QString::number(margin);
                if (!m_marginFunction.isEmpty() && margin == m_defaultMargin)
                    value = m_marginFunction + QLatin1String("()");

                m_output << m_option.indent << parent << "->layout()->setMargin(" << value << ");\n";
            }
        }
    }

    m_output << m_option.indent << varName << " = new " << className << "(";

    if (isGroupBox) {
        m_output << m_driver->findOrInsertWidget(m_widgetChain.top()) << "->layout()";
    } else if (!m_layoutChain.top()) {
        m_output << m_driver->findOrInsertWidget(m_widgetChain.top());
    }

    m_output << ");\n";

    QList<DomProperty*> layoutProperties = node->elementProperty();

    if (isGroupBox) {
        m_output << m_option.indent << varName << "->setAlignment(Qt::AlignTop);\n";
    } else {
        int margin = m_defaultMargin;
        int spacing = m_defaultSpacing;

        if (properties.contains(QLatin1String("margin"))) {
            DomProperty *p = properties.value(QLatin1String("margin"));
            Q_ASSERT(p != 0);

            margin = properties.value(QLatin1String("margin"))->elementNumber();
            layoutProperties.removeAt(layoutProperties.indexOf(p));
        }

        if (properties.contains(QLatin1String("spacing"))) {
            DomProperty *p = properties.value(QLatin1String("spacing"));
            Q_ASSERT(p != 0);

            spacing = properties.value(QLatin1String("spacing"))->elementNumber();
            layoutProperties.removeAt(layoutProperties.indexOf(p));
        }

        if (spacing != INT_MIN) {
            QString value = QString::number(spacing);
            if (!m_spacingFunction.isEmpty() && spacing == m_defaultSpacing)
                value = m_spacingFunction + QLatin1String("()");

            m_output << m_option.indent << varName << "->setSpacing(" << value << ");\n";
        }

        if (margin != INT_MIN) {
            QString value = QString::number(margin);
            if (!m_marginFunction.isEmpty() && margin == m_defaultMargin)
                value = m_marginFunction + QLatin1String("()");

            m_output << m_option.indent << varName << "->setMargin(" << value << ");\n";
        }
    }

    writeProperties(varName, className, layoutProperties);

    m_layoutChain.push(node);
    TreeWalker::acceptLayout(node);
    m_layoutChain.pop();
}

void WriteInitialization::acceptSpacer(DomSpacer *node)
{
    const QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());
    const QString varName = m_driver->findOrInsertSpacer(node);

    const DomSize *sizeHint = properties.contains(QLatin1String("sizeHint"))
        ? properties.value(QLatin1String("sizeHint"))->elementSize() : 0;

    QString sizeType = properties.contains(QLatin1String("sizeType"))
        ? properties.value(QLatin1String("sizeType"))->elementEnum()
        : QString::fromLatin1("Expanding");

    const QString orientation = properties.contains(QLatin1String("orientation"))
        ? properties.value(QLatin1String("orientation"))->elementEnum() : QString();

    const bool isVspacer = orientation == QLatin1String("Qt::Vertical")
        || orientation == QLatin1String("Vertical");

    m_output << m_option.indent << varName << " = new QSpacerItem(";

    if (sizeHint)
        m_output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";

    if (sizeType.startsWith(QLatin1String("QSizePolicy::")) == false)
        sizeType.prepend(QLatin1String("QSizePolicy::"));

    if (isVspacer)
        m_output << "QSizePolicy::Minimum, " << sizeType << ");\n";
    else
        m_output << sizeType << ", QSizePolicy::Minimum);\n";

    TreeWalker::acceptSpacer(node);
}

void WriteInitialization::acceptLayoutItem(DomLayoutItem *node)
{
    TreeWalker::acceptLayoutItem(node);

    DomLayout *layout = m_layoutChain.top();

    if (!layout)
        return;

    const QString varName = m_driver->findOrInsertLayoutItem(node);
    const QString layoutName = m_driver->findOrInsertLayout(layout);

    QString opt;
    if (layout->attributeClass() == QLatin1String("QGridLayout")) {
        const int row = node->attributeRow();
        const int col = node->attributeColumn();

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

    m_output << "\n" << m_option.indent << layoutName << "->" << method << "(" << varName << opt << ");\n\n";
}

void WriteInitialization::acceptActionGroup(DomActionGroup *node)
{
    const QString actionName = m_driver->findOrInsertActionGroup(node);
    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = m_driver->findOrInsertActionGroup(m_actionGroupChain.top());

    m_output << m_option.indent << actionName << " = new QActionGroup(" << varName << ");\n";
    writeProperties(actionName, QLatin1String("QActionGroup"), node->elementProperty());

    m_actionGroupChain.push(node);
    TreeWalker::acceptActionGroup(node);
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptAction(DomAction *node)
{
    if (node->hasAttributeMenu())
        return;

    const QString actionName = m_driver->findOrInsertAction(node);
    m_registeredActions.insert(actionName, node);
    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

    if (m_actionGroupChain.top())
        varName = m_driver->findOrInsertActionGroup(m_actionGroupChain.top());

    m_output << m_option.indent << actionName << " = new QAction(" << varName << ");\n";
    writeProperties(actionName, QLatin1String("QAction"), node->elementProperty());
}

void WriteInitialization::acceptActionRef(DomActionRef *node)
{
    QString actionName = node->attributeName();
    const bool isSeparator = actionName == QLatin1String("separator");
    bool isMenu = false;

    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

    if (actionName.isEmpty() || !m_widgetChain.top()) {
        return;
    } else if (m_driver->actionGroupByName(actionName)) {
        return;
    } else if (DomWidget *w = m_driver->widgetByName(actionName)) {
        isMenu = m_uic->isMenu(w->attributeClass());
        bool inQ3ToolBar = m_uic->customWidgetsInfo()->extends(m_widgetChain.top()->attributeClass(), QLatin1String("Q3ToolBar"));
        if (!isMenu && inQ3ToolBar) {
            actionOut << m_option.indent << actionName << "->setParent(" << varName << ");\n";
            return;
        }
    } else if (!(m_driver->actionByName(actionName) || isSeparator)) {
        fprintf(stderr, "Warning: action `%s' not declared\n", actionName.toLatin1().data());
        return;
    }

    if (m_widgetChain.top() && isSeparator) {
        // separator is always reserved!
        actionOut << m_option.indent << varName << "->addSeparator();\n";
        return;
    }

    if (isMenu)
        actionName += QLatin1String("->menuAction()");

    actionOut << m_option.indent << varName << "->addAction(" << actionName << ");\n";
}

void WriteInitialization::writeProperties(const QString &varName,
                                          const QString &className,
                                          const QList<DomProperty*> &lst)
{
    const bool isTopLevel = m_widgetChain.count() == 1;

    if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
        QHash<QString, DomProperty*> properties = propertyMap(lst);
        if (properties.contains(QLatin1String("control"))) {
            DomProperty *p = properties.value(QLatin1String("control"));
            m_output << m_option.indent << varName << "->setControl(QString::fromUtf8("
                   << fixString(toString(p->elementString()), m_option.indent) << "));\n";
        }
    }

    DomWidget *buttonGroupWidget = findWidget(QLatin1String("Q3ButtonGroup"));

    m_output << m_option.indent << varName << "->setObjectName(QString::fromUtf8(" << fixString(varName, m_option.indent) << "));\n";

    for (int i=0; i<lst.size(); ++i) {
        const DomProperty *p = lst.at(i);
        const QString propertyName = p->attributeName();
        QString propertyValue;

        // special case for the property `geometry'
        if (isTopLevel && propertyName == QLatin1String("geometry") && p->elementRect()) {
            const DomRect *r = p->elementRect();
            const int w = r->elementWidth();
            const int h = r->elementHeight();
			QString tempName = m_driver->unique(QLatin1String("size"));
			resizeOut << m_option.indent << "QSize " << tempName << "(" << w << ", " << h << ");\n"
                      << m_option.indent << tempName << " = " << tempName << ".expandedTo("
                      << varName << "->minimumSizeHint());\n"
                      << m_option.indent << varName << "->resize(" << tempName << ");\n";                
            continue;
        } else if (propertyName == QLatin1String("buttonGroupId") && buttonGroupWidget) { // Q3ButtonGroup support
            m_output << m_option.indent << m_driver->findOrInsertWidget(buttonGroupWidget) << "->insert("
                   << varName << ", " << p->elementNumber() << ");\n";
            continue;
        } else if (propertyName == QLatin1String("currentRow") // QListWidget::currentRow
                    && m_uic->customWidgetsInfo()->extends(className, QLatin1String("QListWidget"))) {
            delayedOut << m_option.indent << varName << "->setCurrentRow("
                       << p->elementNumber() << ");\n";
            continue;
        } else if (propertyName == QLatin1String("currentIndex") // set currentIndex later
                    && (m_uic->customWidgetsInfo()->extends(className, QLatin1String("QComboBox"))
                    || m_uic->customWidgetsInfo()->extends(className, QLatin1String("QStackedWidget"))
                    || m_uic->customWidgetsInfo()->extends(className, QLatin1String("QTabWidget"))
                    || m_uic->customWidgetsInfo()->extends(className, QLatin1String("QToolBox")))) {
            delayedOut << m_option.indent << varName << "->setCurrentIndex("
                       << p->elementNumber() << ");\n";
            continue;
        } else if (propertyName == QLatin1String("control") // ActiveQt support
                    && m_uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
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
        } else if (propertyName == QLatin1String("orientation")
                    && m_uic->customWidgetsInfo()->extends(className, QLatin1String("Line"))) {
            // Line support
            QString shape = QLatin1String("QFrame::HLine");
            if (p->elementEnum() == QLatin1String("Qt::Vertical"))
                shape = QLatin1String("QFrame::VLine");

            m_output << m_option.indent << varName << "->setFrameShape(" << shape << ");\n";
            m_output << m_option.indent << varName << "->setFrameShadow(QFrame::Sunken);\n";
            continue;
        }

        bool stdset = m_stdsetdef;
        if (p->hasAttributeStdset())
            stdset = p->attributeStdset();

        QString setFunction;

        if (stdset) {
            setFunction = QLatin1String("->set")
                + propertyName.left(1).toUpper()
                + propertyName.mid(1)
                + QLatin1String("(");
        } else {
            setFunction = QLatin1String("->setProperty(\"")
                + propertyName
                + QLatin1String("\", QVariant(");
        }

        switch (p->kind()) {
        case DomProperty::Bool: {
            propertyValue = p->elementBool();
            break;
        }
        case DomProperty::Color: {
            const DomColor *c = p->elementColor();
            propertyValue = QString::fromLatin1("QColor(%1, %2, %3)")
                  .arg(c->elementRed())
                  .arg(c->elementGreen())
                  .arg(c->elementBlue()); }
            break;
        case DomProperty::Cstring:
            if (propertyName == QLatin1String("buddy") && m_uic->customWidgetsInfo()->extends(className, QLatin1String("QLabel"))) {
                m_buddies.append(Buddy(varName, p->elementCstring()));
            } else {
                if (stdset)
                    propertyValue = fixString(p->elementCstring(), m_option.indent);
                else
                    propertyValue = QLatin1String("QByteArray(") + fixString(p->elementCstring(), m_option.indent) + QLatin1String(")");
            }
            break;
        case DomProperty::Cursor:
            propertyValue = QString::fromLatin1("QCursor(static_cast<Qt::CursorShape>(%1))")
                            .arg(p->elementCursor());
            break;
        case DomProperty::CursorShape:
            propertyValue = QString::fromLatin1("QCursor(Qt::%1)")
                            .arg(p->elementCursorShape());
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
            const DomFont *f = p->elementFont();
            const QString fontName = m_driver->unique(QLatin1String("font"));
            m_output << m_option.indent << "QFont " << fontName << ";\n";
            if (f->hasElementFamily() && !f->elementFamily().isEmpty()) {
                m_output << m_option.indent << fontName << ".setFamily(QString::fromUtf8(" << fixString(f->elementFamily(), m_option.indent)
                    << "));\n";
            }
            if (f->hasElementPointSize() && f->elementPointSize() > 0) {
                m_output << m_option.indent << fontName << ".setPointSize(" << f->elementPointSize()
                    << ");\n";
            }
            if (f->hasElementBold()) {
                m_output << m_option.indent << fontName << ".setBold("
                    << (f->elementBold() ? "true" : "false") << ");\n";
            }
            if (f->hasElementItalic()) {
                m_output << m_option.indent << fontName << ".setItalic("
                    <<  (f->elementItalic() ? "true" : "false") << ");\n";
            }
            if (f->hasElementUnderline()) {
                m_output << m_option.indent << fontName << ".setUnderline("
                    << (f->elementUnderline() ? "true" : "false") << ");\n";
            }
            if (f->hasElementWeight() && f->elementWeight() > 0) {
                m_output << m_option.indent << fontName << ".setWeight("
                    << f->elementWeight() << ");" << endl;
            }
            if (f->hasElementStrikeOut()) {
                m_output << m_option.indent << fontName << ".setStrikeOut("
                    << (f->elementStrikeOut() ? "true" : "false") << ");\n";
            }
            if (f->hasElementKerning()) {
                m_output << m_option.indent << fontName << ".setKerning("
                    << (f->elementKerning() ? "true" : "false") << ");\n";
            }
            if (f->hasElementAntialiasing()) {
                m_output << m_option.indent << fontName << ".setStyleStrategy("
                    << (f->elementAntialiasing() ? "QFont::PreferDefault" : "QFont::NoAntialias") << ");\n";
            }
            if (f->hasElementStyleStrategy()) {
                m_output << m_option.indent << fontName << ".setStyleStrategy(QFont::"
                    << f->elementStyleStrategy() << ");\n";
            }
            propertyValue = fontName;
            break;
        }
        case DomProperty::IconSet:
            propertyValue = pixCall(p);
            break;

        case DomProperty::Pixmap:
            propertyValue = pixCall(p);
            break;

        case DomProperty::Palette: {
            const DomPalette *pal = p->elementPalette();
            const QString paletteName = m_driver->unique(QLatin1String("palette"));
            m_output << m_option.indent << "QPalette " << paletteName << ";\n";

            writeColorGroup(pal->elementActive(), QLatin1String("QPalette::Active"), paletteName);
            writeColorGroup(pal->elementInactive(), QLatin1String("QPalette::Inactive"), paletteName);
            writeColorGroup(pal->elementDisabled(), QLatin1String("QPalette::Disabled"), paletteName);

            propertyValue = paletteName;
            break;
        }
        case DomProperty::Point: {
            const DomPoint *po = p->elementPoint();
            propertyValue = QString::fromLatin1("QPoint(%1, %2)")
                            .arg(po->elementX()).arg(po->elementY());
            break;
        }
        case DomProperty::PointF: {
            const DomPointF *pof = p->elementPointF();
            propertyValue = QString::fromLatin1("QPointF(%1, %2)")
                            .arg(pof->elementX()).arg(pof->elementY());
            break;
        }
        case DomProperty::Rect: {
            const DomRect *r = p->elementRect();
            propertyValue = QString::fromLatin1("QRect(%1, %2, %3, %4)")
                            .arg(r->elementX()).arg(r->elementY())
                            .arg(r->elementWidth()).arg(r->elementHeight());
            break;
        }
        case DomProperty::RectF: {
            const DomRectF *rf = p->elementRectF();
            propertyValue = QString::fromLatin1("QRectF(%1, %2, %3, %4)")
                            .arg(rf->elementX()).arg(rf->elementY())
                            .arg(rf->elementWidth()).arg(rf->elementHeight());
            break;
        }
        case DomProperty::SizePolicy: {
            const DomSizePolicy *sp = p->elementSizePolicy();
            const QString spName = m_driver->unique(QLatin1String("sizePolicy"));
            if (sp->hasElementHSizeType() && sp->hasElementVSizeType()) {
                m_output << m_option.indent << "QSizePolicy " << spName << QString::fromLatin1(
                    "(static_cast<QSizePolicy::Policy>(%1), static_cast<QSizePolicy::Policy>(%2));\n")
                            .arg(sp->elementHSizeType())
                            .arg(sp->elementVSizeType());
            } else if (sp->hasAttributeHSizeType() && sp->hasAttributeVSizeType()) {
                m_output << m_option.indent << "QSizePolicy " << spName << QString::fromLatin1(
                    "(QSizePolicy::%1, QSizePolicy::%2);\n")
                            .arg(sp->attributeHSizeType())
                            .arg(sp->attributeVSizeType());
            }
            m_output << m_option.indent << spName << ".setHorizontalStretch("
                << sp->elementHorStretch() << ");\n";
            m_output << m_option.indent << spName << ".setVerticalStretch("
                << sp->elementVerStretch() << ");\n";
            m_output << m_option.indent << spName << QString::fromLatin1(
                ".setHeightForWidth(%1->sizePolicy().hasHeightForWidth());\n")
                .arg(varName);

            propertyValue = spName;
            break;
        }
        case DomProperty::Size: {
             const DomSize *s = p->elementSize();
              propertyValue = QString::fromLatin1("QSize(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::SizeF: {
            const DomSizeF *sf = p->elementSizeF();
             propertyValue = QString::fromLatin1("QSizeF(%1, %2)")
                            .arg(sf->elementWidth()).arg(sf->elementHeight());
            break;
        }
        case DomProperty::String: {
            if (propertyName == QLatin1String("objectName")) {
                const QString v = p->elementString()->text();
                if (v == varName)
                    break;

                // ### qWarning("Deprecated: the property `objectName' is different from the variable name");
            }

            if (p->elementString()->hasAttributeNotr()
                    && toBool(p->elementString()->attributeNotr())) {
                propertyValue = QLatin1String("QString::fromUtf8(")
                        + fixString(p->elementString()->text(), m_option.indent)
                        + QLatin1String(")");
            } else {
                propertyValue = trCall(p->elementString());
            }
            break;
        }
        case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;
        case DomProperty::UInt:
            propertyValue = QString::number(p->elementUInt()) + QLatin1String("u");
            break;
        case DomProperty::LongLong:
            propertyValue = QLatin1String("Q_INT64_C(") + QString::number(p->elementLongLong())
                        + QLatin1String(")");;
            break;
        case DomProperty::ULongLong:
            propertyValue = QLatin1String("Q_UINT64_C(") + QString::number(p->elementULongLong())
                        + QLatin1String(")");;
            break;
        case DomProperty::Float:
            propertyValue = QString::number(p->elementFloat());
            break;
        case DomProperty::Double:
            propertyValue = QString::number(p->elementDouble());
            break;
        case DomProperty::Char: {
            const DomChar *c = p->elementChar();
            propertyValue = QString::fromLatin1("QChar(%1)")
                            .arg(c->elementUnicode());
            break;
        }
        case DomProperty::Date: {
            const DomDate *d = p->elementDate();
            propertyValue = QString::fromLatin1("QDate(%1, %2, %3)")
                            .arg(d->elementYear())
                            .arg(d->elementMonth())
                            .arg(d->elementDay());
            break;
        }
        case DomProperty::Time: {
            const DomTime *t = p->elementTime();
            propertyValue = QString::fromLatin1("QTime(%1, %2, %3)")
                            .arg(t->elementHour())
                            .arg(t->elementMinute())
                            .arg(t->elementSecond());
            break;
        }
        case DomProperty::DateTime: {
            const DomDateTime *dt = p->elementDateTime();
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
                const QStringList lst = p->elementStringList()->elementString();
                for (int i=0; i<lst.size(); ++i) {
                    propertyValue += QLatin1String(" << ") + fixString(lst.at(i), m_option.indent);
                }
            }
            break;

        case DomProperty::Url: {
            const DomUrl* u = p->elementUrl();
            propertyValue = QString::fromLatin1("QUrl(%1)")
                            .arg(fixString(u->elementString()->text(), m_option.indent));
            break;
        }
        case DomProperty::Unknown:
            break;
        }

        if (propertyValue.size()) {
            QTextStream *o = &m_output;

            if (p->kind() == DomProperty::String
                    && (!p->elementString()->hasAttributeNotr() || !toBool(p->elementString()->attributeNotr())))
                o = &refreshOut;

#ifdef QT_NO_TOOLTIP
            if (propertyName == QLatin1String("toolTip"))
                continue;
#endif // QT_NO_TOOLTIP
#ifdef QT_NO_WHATSTHIS
            if (propertyName == QLatin1String("whatsThis"))
                continue;
#endif // QT_NO_WHATSTHIS
#ifdef QT_NO_STATUSTIP
            if (propertyName == QLatin1String("statusTip"))
                continue;
#endif // QT_NO_WHATSTHIS
            (*o) << m_option.indent << varName << setFunction << propertyValue;
            if (!stdset)
                (*o) << ")";
            (*o) << ");\n";
        }
    }
}

QString WriteInitialization::domColor2QString(const DomColor *c)
{
    if (c->hasAttributeAlpha())
        return QString::fromLatin1("QColor(%1, %2, %3, %4)")
            .arg(c->elementRed())
            .arg(c->elementGreen())
            .arg(c->elementBlue())
            .arg(c->attributeAlpha());
    return QString::fromLatin1("QColor(%1, %2, %3)")
        .arg(c->elementRed())
        .arg(c->elementGreen())
        .arg(c->elementBlue());
}

void WriteInitialization::writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName)
{
    if (!colorGroup)
        return;

    // old format
    const QList<DomColor*> colors = colorGroup->elementColor();
    for (int i=0; i<colors.size(); ++i) {
        const DomColor *color = colors.at(i);

        m_output << m_option.indent << paletteName << ".setColor(" << group
            << ", " << "static_cast<QPalette::ColorRole>(" << QString::number(i) << ")"
            << ", " << domColor2QString(color)
            << ");\n";
    }

    // new format
    const QList<DomColorRole *> colorRoles = colorGroup->elementColorRole();
    QListIterator<DomColorRole *> itRole(colorRoles);
    while (itRole.hasNext()) {
        const DomColorRole *colorRole = itRole.next();
        if (colorRole->hasAttributeRole()) {
            const QString brushName = m_driver->unique(QLatin1String("brush"));
            writeBrush(colorRole->elementBrush(), brushName);

            m_output << m_option.indent << paletteName << ".setBrush(" << group
                << ", " << "QPalette::" << colorRole->attributeRole()
                << ", " << brushName << ");\n";
        }
    }
}

void WriteInitialization::writeBrush(DomBrush *brush, const QString &brushName)
{
    QString style = QLatin1String("SolidPattern");
    if (brush->hasAttributeBrushStyle())
        style = brush->attributeBrushStyle();

    if (style == QLatin1String("LinearGradientPattern") ||
            style == QLatin1String("RadialGradientPattern") ||
            style == QLatin1String("ConicalGradientPattern")) {
        const DomGradient *gradient = brush->elementGradient();
        const QString gradientType = gradient->attributeType();
        const QString gradientName = m_driver->unique(QLatin1String("gradient"));
        if (gradientType == QLatin1String("LinearGradient")) {
            m_output << m_option.indent << "QLinearGradient " << gradientName
                << "(" << gradient->attributeStartX()
                << ", " << gradient->attributeStartY()
                << ", " << gradient->attributeEndX()
                << ", " << gradient->attributeEndY() << ");\n";
        } else if (gradientType == QLatin1String("RadialGradient")) {
            m_output << m_option.indent << "QRadialGradient " << gradientName
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeRadius()
                << ", " << gradient->attributeFocalX()
                << ", " << gradient->attributeFocalY() << ");\n";
        } else if (gradientType == QLatin1String("ConicalGradient")) {
            m_output << m_option.indent << "QConicalGradient " << gradientName
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeAngle() << ");\n";
        }

        m_output << m_option.indent << gradientName << ".setSpread(QGradient::"
            << gradient->attributeSpread() << ");\n";

        if (gradient->hasAttributeCoordinateMode()) {
            m_output << m_option.indent << gradientName << ".setCoordinateMode(QGradient::"
                << gradient->attributeCoordinateMode() << ");\n";
        }

       const  QList<DomGradientStop *> stops = gradient->elementGradientStop();
        QListIterator<DomGradientStop *> it(stops);
        while (it.hasNext()) {
            const DomGradientStop *stop = it.next();
            const DomColor *color = stop->elementColor();
            m_output << m_option.indent << gradientName << ".setColorAt("
                << stop->attributePosition() << ", "
                << domColor2QString(color) << ");\n";
        }
        m_output << m_option.indent << "QBrush " << brushName << "("
            << gradientName << ");\n";
    } else if (style == QLatin1String("TexturePattern")) {
        const DomProperty *property = brush->elementTexture();

        m_output << m_option.indent << "QBrush " << brushName << " = QBrush("
            << pixCall(property) << ");\n";
    } else {
        const DomColor *color = brush->elementColor();
        m_output << m_option.indent << "QBrush " << brushName << "("
            << domColor2QString(color) << ");\n";

        m_output << m_option.indent << brushName << ".setStyle("
            << "Qt::" << style << ");\n";
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

    const QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        const QString name = l.at(i);

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

        m_output << m_option.indent << "QWidget::setTabOrder(" << lastName << ", " << name << ");\n";

        lastName = name;
    }
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

void WriteInitialization::acceptLayoutFunction(DomLayoutFunction *node)
{
    m_marginFunction.clear();
    m_spacingFunction.clear();

    if (!node)
        return;

    if (node->hasAttributeMargin())
        m_marginFunction = node->attributeMargin();

    if (node->hasAttributeSpacing())
        m_spacingFunction = node->attributeSpacing();
}

void WriteInitialization::initializeQ3ListBox(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << m_option.indent << varName << "->clear();\n";

    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        refreshOut << m_option.indent << varName << "->insertItem(";
        if (pixmap) {
            refreshOut << pixCall(pixmap);

            if (text)
                refreshOut << ", ";
        }
        if (text)
            refreshOut << trCall(text->elementString());
        refreshOut << ");\n";
    }
}

void WriteInitialization::initializeQ3IconView(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << m_option.indent << varName << "->clear();\n";

    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        const QString itemName = m_driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << m_option.indent << "Q3IconViewItem *" << itemName << " = new Q3IconViewItem(" << varName << ");\n";

        if (pixmap) {
            refreshOut << m_option.indent << itemName << "->setPixmap(" << pixCall(pixmap) << ");\n";
        }

        if (text) {
            refreshOut << m_option.indent << itemName << "->setText(" << trCall(text->elementString()) << ");\n";
        }
    }
}

void WriteInitialization::initializeQ3ListView(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        const DomProperty *clickable = properties.value(QLatin1String("clickable"));
        const DomProperty *resizable = properties.value(QLatin1String("resizable"));

        const QString txt = trCall(text->elementString());
        m_output << m_option.indent << varName << "->addColumn(" << txt << ");\n";
        refreshOut << m_option.indent << varName << "->header()->setLabel(" << i << ", " << txt << ");\n";

        if (pixmap) {
            m_output << m_option.indent << varName << "->header()->setLabel("
                   << varName << "->header()->count() - 1, " << pixCall(pixmap) << ", " << txt << ");\n";
        }

        if (clickable != 0) {
            m_output << m_option.indent << varName << "->header()->setClickEnabled(" << clickable->elementBool() << ", " << varName << "->header()->count() - 1);\n";
        }

        if (resizable != 0) {
            m_output << m_option.indent << varName << "->header()->setResizeEnabled(" << resizable->elementBool() << ", " << varName << "->header()->count() - 1);\n";
        }
    }

    if (w->elementItem().size()) {
        refreshOut << m_option.indent << varName << "->clear();\n";

        initializeQ3ListViewItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeQ3ListViewItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QString itemName = m_driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << m_option.indent << "Q3ListViewItem *" << itemName << " = new Q3ListViewItem(" << varName << ");\n";

        int textCount = 0, pixCount = 0;
        const QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            const DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                refreshOut << m_option.indent << itemName << "->setText(" << textCount++ << ", "
                           << trCall(p->elementString()) << ");\n";

            if (p->attributeName() == QLatin1String("pixmap"))
                refreshOut << m_option.indent << itemName << "->setPixmap(" << pixCount++ << ", "
                           << pixCall(p) << ");\n";
        }

        if (item->elementItem().size()) {
            refreshOut << m_option.indent << itemName << "->setOpen(true);\n";
            initializeQ3ListViewItems(className, itemName, item->elementItem());
        }
    }
}

void WriteInitialization::initializeTreeWidgetItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QString itemName = m_driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << m_option.indent << "QTreeWidgetItem *" << itemName << " = new QTreeWidgetItem(" << varName << ");\n";

        int textCount = 0;
        const QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            const DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                refreshOut << m_option.indent << itemName << "->setText(" << textCount++ << ", "
                           << trCall(p->elementString()) << ");\n";

            if (p->attributeName() == QLatin1String("icon") && textCount > 0)
                refreshOut << m_option.indent << itemName << "->setIcon(" << textCount - 1 << ", "
                           << pixCall(p) << ");\n";
        }

        if (item->elementItem().size())
            initializeTreeWidgetItems(className, itemName, item->elementItem());
    }
}


void WriteInitialization::initializeQ3Table(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn*> columns = w->elementColumn();

    for (int i=0; i<columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << m_option.indent << varName << "->horizontalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap) << ", ";
        }
        refreshOut << trCall(text->elementString()) << ");\n";
    }

    // rows
    const QList<DomRow*> rows = w->elementRow();
    for (int i=0; i<rows.size(); ++i) {
        const DomRow *row = rows.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << m_option.indent << varName << "->verticalHeader()->setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap) << ", ";
        }
        refreshOut << trCall(text->elementString()) << ");\n";
    }


    //initializeQ3TableItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeQ3TableItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    Q_UNUSED(className);
    Q_UNUSED(varName);
    Q_UNUSED(items);
}

QString WriteInitialization::pixCall(const DomProperty *p) const
{
    Q_ASSERT(p->kind() == DomProperty::IconSet || p->kind() == DomProperty::Pixmap);

    QString type, s;
    if (p->kind() == DomProperty::IconSet) {
        type = QLatin1String("QIcon");
        s = p->elementIconSet()->text();
    } else {
        type = QLatin1String("QPixmap");
        s = p->elementPixmap()->text();
    }

    if (s.isEmpty())
        return type + QLatin1String("()");
    else if (findImage(s) != 0)
        return QLatin1String("icon(") + s + QLatin1String("_ID)");

    QString pixFunc = m_uic->pixmapFunction();
    if (pixFunc.isEmpty())
        pixFunc = QLatin1String("QString::fromUtf8");

    return type + QLatin1String("(") + pixFunc + QLatin1String("(") + fixString(s, m_option.indent) + QLatin1String(")") + QLatin1String(")");
}

void WriteInitialization::initializeComboBox(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << m_option.indent << varName << "->clear();\n";

    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *pixmap = properties.value(QLatin1String("icon"));
        if (!(text || pixmap))
            continue;

        refreshOut << m_option.indent << varName << "->addItem(";

        if (pixmap != 0) {
            refreshOut << pixCall(pixmap);

            if (text)
                refreshOut << ", ";
        }

        refreshOut << trCall(text->elementString()) << ");\n";
    }
}

void WriteInitialization::initializeListWidget(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << m_option.indent << varName << "->clear();\n";

    // items
    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);

        const QString itemName = m_driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << m_option.indent << "QListWidgetItem *" << itemName << " = new QListWidgetItem(" << varName << ");\n";

        const QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            const DomProperty *p = properties.at(i);

            if (p->attributeName() == QLatin1String("text"))
                refreshOut << m_option.indent << itemName << "->setText(" << trCall(p->elementString()) << ");\n";

            if (p->attributeName() == QLatin1String("icon"))
                refreshOut << m_option.indent << itemName << "->setIcon(" << pixCall(p) << ");\n";
        }
    }
}

void WriteInitialization::initializeTreeWidget(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *icon = properties.value(QLatin1String("icon"));

        const QString txt = trCall(text->elementString());
        refreshOut << m_option.indent << varName << "->headerItem()->setText(" << i << ", " << txt << ");\n";

        if (icon != 0 && icon->elementIconSet()) {
            m_output << m_option.indent << varName << "->headerItem()->setIcon("
                   << i << ", " << pixCall(icon) << ");\n";
        }
    }

    if (w->elementItem().size()) {
        refreshOut << m_option.indent << varName << "->clear();\n";

        initializeTreeWidgetItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeTableWidget(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn *> columns = w->elementColumn();

    if (columns.size() != 0) {
        refreshOut << m_option.indent << "if (" << varName << "->columnCount() < " << columns.size() << ")\n"
            << m_option.indent << m_option.indent << varName << "->setColumnCount(" << columns.size() << ");\n";
    }

    for (int i = 0; i < columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *icon = properties.value(QLatin1String("icon"));
        if (text || icon) {
            const QString itemName = m_driver->unique(QLatin1String("__colItem"));
            refreshOut << "\n";
            refreshOut << m_option.indent << "QTableWidgetItem *"
                           << itemName << " = new QTableWidgetItem();\n";

            if (text && text->attributeName() == QLatin1String("text"))
                refreshOut << m_option.indent << itemName << "->setText("
                           << trCall(text->elementString()) << ");\n";

            if (icon && icon->attributeName() == QLatin1String("icon"))
                refreshOut << m_option.indent << itemName << "->setIcon("
                           << pixCall(icon) << ");\n";
            refreshOut << m_option.indent << varName << "->setHorizontalHeaderItem("
                           << i << ", " << itemName << ");\n";
        }
    }

    // rows
    const QList<DomRow *> rows = w->elementRow();

    if (rows.size() != 0) {
        refreshOut << m_option.indent << "if (" << varName << "->rowCount() < " << rows.size() << ")\n"
            << m_option.indent << m_option.indent << varName << "->setRowCount(" << rows.size() << ");\n";
    }

    for (int i = 0; i < rows.size(); ++i) {
        const DomRow *row = rows.at(i);

        const QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        const DomProperty *text = properties.value(QLatin1String("text"));
        const DomProperty *icon = properties.value(QLatin1String("icon"));
        if (text || icon) {
            const QString itemName = m_driver->unique(QLatin1String("__rowItem"));
            refreshOut << "\n";
            refreshOut << m_option.indent << "QTableWidgetItem *"
                           << itemName << " = new QTableWidgetItem();\n";

            if (text && text->attributeName() == QLatin1String("text"))
                refreshOut << m_option.indent << itemName << "->setText("
                           << trCall(text->elementString()) << ");\n";

            if (icon && icon->attributeName() == QLatin1String("icon"))
                refreshOut << m_option.indent << itemName << "->setIcon("
                           << pixCall(icon) << ");\n";
            refreshOut << m_option.indent << varName << "->setVerticalHeaderItem("
                           << i << ", " << itemName << ");\n";
        }
    }

    // items
    const QList<DomItem *> items = w->elementItem();
    QString tempName;
    if (!items.isEmpty()) {
        // turn off sortingEnabled to force programmatic item order (setItem())
        tempName = m_driver->unique(QLatin1String("__sortingEnabled"));
        refreshOut << "\n";
        refreshOut << m_option.indent << "bool " << tempName
                   << " = " << varName << "->isSortingEnabled();\n";
        refreshOut << m_option.indent << varName << "->setSortingEnabled(false);\n";
    }
    for (int i = 0; i < items.size(); ++i) {
        const DomItem *item = items.at(i);
        if (item->hasAttributeRow() && item->hasAttributeColumn()) {
            const QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
            const DomProperty *text = properties.value(QLatin1String("text"));
            const DomProperty *icon = properties.value(QLatin1String("icon"));
            if (text || icon) {
                const QString itemName = m_driver->unique(QLatin1String("__item"));
                refreshOut << "\n";
                refreshOut << m_option.indent << "QTableWidgetItem *"
                    << itemName << " = new QTableWidgetItem();\n";

                if (text && text->attributeName() == QLatin1String("text"))
                    refreshOut << m_option.indent << itemName << "->setText("
                        << trCall(text->elementString()) << ");\n";

                if (icon && icon->attributeName() == QLatin1String("icon"))
                    refreshOut << m_option.indent << itemName << "->setIcon("
                        << pixCall(icon) << ");\n";
                refreshOut << m_option.indent << varName << "->setItem("
                    << item->attributeRow() << ", "
                    << item->attributeColumn() << ", "
                    << itemName << ");\n";
            }
        }
    }
    if (!items.isEmpty()) {
        // restore value of sortingEnabled
        refreshOut << "\n";
        refreshOut << m_option.indent << varName << "->setSortingEnabled(" << tempName << ");\n";
        refreshOut << "\n";
    }
}

QString WriteInitialization::trCall(const QString &str, const QString &commentHint) const
{
    if (str.isEmpty())
        return QLatin1String("QString()");

    QString result;
    const QString comment = commentHint.isEmpty() ? QString::fromUtf8("0") : fixString(commentHint, m_option.indent);

    if (m_option.translateFunction.isEmpty()) {
        result = QLatin1String("QApplication::translate(\"");
        result += m_generatedClass;
        result += QLatin1String("\"");
        result += QLatin1String(", ");
    } else {
        result = m_option.translateFunction + QLatin1String("(");
    }

    result += fixString(str, m_option.indent);
    result += QLatin1String(", ");
    result += comment;

    if (m_option.translateFunction.isEmpty()) {
        result += QLatin1String(", ");
        result += QLatin1String("QApplication::UnicodeUTF8");
    }

    result += QLatin1String(")");
    return result;
}

void WriteInitialization::initializeQ3SqlDataTable(DomWidget *w)
{
    const QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    const DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    const DomProperty *db = properties.value(QLatin1String("database"), 0);
    if (db && db->elementStringList()) {
        const QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

   const QString varName = m_driver->findOrInsertWidget(w);

    m_output << m_option.indent << "if (!" << varName << "->sqlCursor()) {\n";

    m_output << m_option.indent << m_option.indent << varName << "->setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        m_output << "new Q3SqlCursor(" << fixString(table, m_option.indent) << "), false, true);\n";
    } else {
        m_output << "new Q3SqlCursor(" << fixString(table, m_option.indent) << ", true, " << connection << "Connection" << "), false, true);\n";
    }
    m_output << m_option.indent << m_option.indent << varName << "->refresh(Q3DataTable::RefreshAll);\n";
    m_output << m_option.indent << "}\n";
}

void WriteInitialization::initializeQ3SqlDataBrowser(DomWidget *w)
{
    const QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    const DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    const DomProperty *db = properties.value(QLatin1String("database"), 0);
    if (db && db->elementStringList()) {
        const QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    const QString varName = m_driver->findOrInsertWidget(w);

    m_output << m_option.indent << "if (!" << varName << "->sqlCursor()) {\n";

    m_output << m_option.indent << m_option.indent << varName << "->setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        m_output << "new Q3SqlCursor(" << fixString(table, m_option.indent) << "), true);\n";
    } else {
        m_output << "new Q3SqlCursor(" << fixString(table, m_option.indent) << ", true, " << connection << "Connection" << "), false, true);\n";
    }
    m_output << m_option.indent << m_option.indent << varName << "->refresh();\n";
    m_output << m_option.indent << "}\n";
}

void WriteInitialization::initializeMenu(DomWidget *w, const QString &/*parentWidget*/)
{
    const QString menuName = m_driver->findOrInsertWidget(w);
    const QString menuAction = menuName + QLatin1String("Action");

    const DomAction *action = m_driver->actionByName(menuAction);
    if (action && action->hasAttributeMenu()) {
        m_output << m_option.indent << menuAction << " = " << menuName << "->menuAction();\n";
    }
}

QString WriteInitialization::trCall(DomString *str) const
{
    return trCall(toString(str), str->attributeComment());
}

bool WriteInitialization::isValidObject(const QString &name) const
{
    return m_registeredWidgets.contains(name)
        || m_registeredActions.contains(name);
}

QString WriteInitialization::findDeclaration(const QString &name)
{
    const QString normalized = Driver::normalizedName(name);

    if (DomWidget *widget = m_driver->widgetByName(normalized))
        return m_driver->findOrInsertWidget(widget);
    else if (DomAction *action = m_driver->actionByName(normalized))
        return m_driver->findOrInsertAction(action);

    return QString();
}

void WriteInitialization::acceptConnection(DomConnection *connection)
{
    const QString sender = findDeclaration(connection->elementSender());
    const QString receiver = findDeclaration(connection->elementReceiver());

    if (sender.isEmpty() || receiver.isEmpty())
        return;

    m_output << m_option.indent << "QObject::connect("
        << sender
        << ", "
        << "SIGNAL(" << connection->elementSignal() << ")"
        << ", "
        << receiver
        << ", "
        << "SLOT(" << connection->elementSlot() << ")"
        << ");\n";
}

DomImage *WriteInitialization::findImage(const QString &name) const
{
    return m_registeredImages.value(name);
}

DomWidget *WriteInitialization::findWidget(const QString &widgetClass)
{
    for (int i = m_widgetChain.count() - 1; i >= 0; --i) {
        DomWidget *widget = m_widgetChain.at(i);

        if (widget && m_uic->customWidgetsInfo()->extends(widget->attributeClass(), widgetClass))
            return widget;
    }

    return 0;
}

void WriteInitialization::acceptImage(DomImage *image)
{
    if (!image->hasAttributeName())
        return;

    m_registeredImages.insert(image->attributeName(), image);
}

} // namespace CPP
