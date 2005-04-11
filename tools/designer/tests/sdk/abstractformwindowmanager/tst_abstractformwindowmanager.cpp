
// qtest
#include "tst_abstractformwindowmanager.h"
#include <ideapplication.h>
#include <qtesttable.h>

// sdk
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

tst_QDesignerFormWindowManagerInterface::tst_QDesignerFormWindowManagerInterface(int &argc, char *argv[])
    : QTestCase(argc, argv)
{
}

tst_QDesignerFormWindowManagerInterface::~tst_QDesignerFormWindowManagerInterface()
{
}

void tst_QDesignerFormWindowManagerInterface::createFormWindow()
{
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    {
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formWindow != 0);
        delete formWindow;
    }
}

void tst_QDesignerFormWindowManagerInterface::formWindowCount()
{
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    VERIFY(formManager->formWindowCount() == 0);

    {
        QDesignerFormWindowInterface *formWindow = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 1);
        delete formWindow;
        VERIFY(formManager->formWindowCount() == 0);
    }

    {
        QDesignerFormWindowInterface *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 1);

        QDesignerFormWindowInterface *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 2);

        QDesignerFormWindowInterface *formWindow3 = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 3);

        delete formWindow3;
        delete formWindow2;
        delete formWindow1;

        VERIFY(formManager->formWindowCount() == 0);
    }
}

class Helper: public QObject
{
    Q_OBJECT
public:
    Helper(QObject *parent = 0)
        : QObject(parent),
          formWindowAdded_signal_count(0),
          formWindowRemoved_signal_count(0) {}

    int formWindowAdded_signal_count;
    int formWindowRemoved_signal_count;

public slots:
    void formWindowAdded(QDesignerFormWindowInterface *)
    { ++formWindowAdded_signal_count; }

    void formWindowRemoved(QDesignerFormWindowInterface *)
    { ++formWindowRemoved_signal_count; }
};

void tst_QDesignerFormWindowManagerInterface::formWindowAdded()
{
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    { // test the SIGNAL QDesignerFormWindowManagerInterface::formWindowAdded()
        Helper h;
        connect(formManager, SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            &h, SLOT(formWindowAdded(QDesignerFormWindowInterface*)));

        COMPARE(h.formWindowAdded_signal_count, 0);

        {
            QDesignerFormWindowInterface *formWindow = formManager->createFormWindow(/*parent=*/ 0);
            COMPARE(h.formWindowAdded_signal_count, 1);
            delete formWindow;
        }

        {
            QDesignerFormWindowInterface *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
            QDesignerFormWindowInterface *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);

            COMPARE(h.formWindowAdded_signal_count, 3);

            delete formWindow1;
            delete formWindow2;
        }
    }
}

void tst_QDesignerFormWindowManagerInterface::formWindowRemoved()
{
    QDesignerFormWindowManagerInterface *formManager = IDEApplication::core()->formManager();

    { // test the SIGNAL QDesignerFormWindowManagerInterface::formWindowRemoved()
        Helper h;
        connect(formManager, SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            &h, SLOT(formWindowRemoved(QDesignerFormWindowInterface*)));

        COMPARE(h.formWindowRemoved_signal_count, 0);

        {
            QDesignerFormWindowInterface *formWindow = formManager->createFormWindow(/*parent=*/ 0);
            delete formWindow;
            COMPARE(h.formWindowRemoved_signal_count, 1);
        }

        {
            QDesignerFormWindowInterface *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
            QDesignerFormWindowInterface *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);

            delete formWindow1;
            delete formWindow2;

            COMPARE(h.formWindowRemoved_signal_count, 3);
        }
    }
}

#include "tst_abstractformwindowmanager.moc"
