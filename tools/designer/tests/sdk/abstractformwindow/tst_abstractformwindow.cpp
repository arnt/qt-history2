
// qtest
#include "tst_abstractformwindow.h"
#include <ideapplication.h>
#include <qtesttable.h>

// sdk
#include <abstractformwindow.h>
#include <ui4.h>

// Qt
#include <qdom.h>

const char * const form_simple =
"<ui version=\"4.0\" >\n"
" <comment></comment>\n"
" <class>Form</class>\n"
" <widget class=\"QWidget\" >\n"
"  <property name=\"geometry\" >\n"
"   <rect>\n"
"    <x>0</x>\n"
"    <y>0</y>\n"
"    <width>173</width>\n"
"    <height>82</height>\n"
"   </rect>\n"
"  </property>\n"
"  <property name=\"objectName\" >\n"
"   <string notr=\"true\" >Form</string>\n"
"  </property>\n"
"  <layout class=\"QVBoxLayout\" >\n"
"   <property name=\"objectName\" >\n"
"    <string notr=\"true\" />\n"
"   </property>\n"
"   <item>\n"
"    <widget class=\"QPushButton\" name=\"PushButton\" >\n"
"     <property name=\"geometry\" >\n"
"      <rect>\n"
"       <x>9</x>\n"
"       <y>29</y>\n"
"       <width>155</width>\n"
"       <height>24</height>\n"
"      </rect>\n"
"     </property>\n"
"     <property name=\"objectName\" >\n"
"      <string notr=\"true\" >pushButton</string>\n"
"     </property>\n"
"     <property name=\"text\" >\n"
"      <string>PushButton</string>\n"
"     </property>\n"
"    </widget>\n"
"   </item>\n"
"  </layout>\n"
" </widget>\n"
" <pixmapfunction></pixmapfunction>\n"
"</ui>\n";

const char * const widget_pushbutton =
"<ui version=\"4.0\" >\n"
" <comment></comment>\n"
" <class></class>\n"
" <widget name=\"__qt_fake_top_level\" >\n"
"  <widget class=\"QPushButton\" name=\"PushButton\" >\n"
"   <property name=\"geometry\" >\n"
"    <rect>\n"
"     <x>199</x>\n"
"     <y>166</y>\n"
"     <width>80</width>\n"
"     <height>24</height>\n"
"    </rect>\n"
"   </property>\n"
"   <property name=\"objectName\" >\n"
"    <string notr=\"true\" >pushButton</string>\n"
"   </property>\n"
"   <property name=\"text\" >\n"
"    <string>PushButton</string>\n"
"   </property>\n"
"  </widget>\n"
" </widget>\n"
" <pixmapfunction></pixmapfunction>\n"
"</ui>\n";

tst_QDesignerFormWindowInterface::tst_QDesignerFormWindowInterface(int &argc, char *argv[])
    : QTestCase(argc, argv)
{
}

tst_QDesignerFormWindowInterface::~tst_QDesignerFormWindowInterface()
{
}

void tst_QDesignerFormWindowInterface::dirty_data(QTestTable &t)
{
    t.defineElement("bool", "retval1");
    t.defineElement("bool", "retval2");
    t.defineElement("bool", "retval3");

    *t.newData(QString::fromUtf8("dirty_data_1"))
        << /*retval1=*/ false
        << /*retval2=*/ false
        << /*retval3=*/ true;
}

void tst_QDesignerFormWindowInterface::dirty()
{
    // set up
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    QDomDocument doc;
    doc.setContent(QString::fromUtf8(widget_pushbutton));
    DomUI ui;
    ui.read(doc.firstChild().toElement());

    FETCH(bool, retval1);
    FETCH(bool, retval2);
    FETCH(bool, retval3);


    { // test the default value of QDesignerFormWindowInterface::isDirty()
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow();
        COMPARE(formWindow->isDirty(), retval1);
        delete formWindow;
    }

    { // test the value of QDesignerFormWindowInterface::isDirty() after QDesignerFormWindowInterface::setContents()
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow();
        formWindow->setContents(QString::fromUtf8(form_simple));
        COMPARE(formWindow->isDirty(), retval2);
        delete formWindow;
    }

    { // test the value of QDesignerFormWindowInterface::isDirty() after adding a widget
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow();
        formWindow->setContents(QString::fromUtf8(form_simple));
        QWidget *widget = formWindow->createWidget(&ui, QRect(0, 0, 100, 100), formWindow->mainContainer());
        VERIFY(widget != 0);
        COMPARE(formWindow->isDirty(), retval3);
        delete formWindow;
    }
}

void tst_QDesignerFormWindowInterface::mainContainer_data(QTestTable &t)
{
    t.defineElement("QString", "widgetName");

    *t.newData(QString::fromUtf8("mainContainer_data_1"))
        << /*widgetName=*/ QString::fromUtf8("Form");
}

void tst_QDesignerFormWindowInterface::mainContainer()
{
    // set up
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    FETCH(QString, widgetName);

    { // test the value of QDesignerFormWindowInterface::isDirty() after QDesignerFormWindowInterface::setContents()
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow();
        formWindow->setContents(QString::fromUtf8(form_simple));
        COMPARE(formWindow->mainContainer()->objectName(), widgetName);
        delete formWindow;
    }
}

