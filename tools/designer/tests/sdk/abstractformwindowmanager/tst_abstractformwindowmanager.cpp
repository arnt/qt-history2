
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

tst_AbstractFormWindowManager::tst_AbstractFormWindowManager(int &argc, char *argv[])
    : QTestCase(argc, argv)
{
}

tst_AbstractFormWindowManager::~tst_AbstractFormWindowManager()
{
}

void tst_AbstractFormWindowManager::createFormWindow()
{
    AbstractFormWindowManager *formManager = IDEApplication::core()->formManager();

    {
        AbstractFormWindow *formWindow = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formWindow != 0);
        delete formWindow;
    }
}

void tst_AbstractFormWindowManager::formWindowCount()
{
    AbstractFormWindowManager *formManager = IDEApplication::core()->formManager();

    VERIFY(formManager->formWindowCount() == 0);

    {
        AbstractFormWindow *formWindow = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 1);
        delete formWindow;
        VERIFY(formManager->formWindowCount() == 0);
    }

    {
        AbstractFormWindow *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 1);

        AbstractFormWindow *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);
        VERIFY(formManager->formWindowCount() == 2);

        AbstractFormWindow *formWindow3 = formManager->createFormWindow(/*parent=*/ 0);
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
    void formWindowAdded(AbstractFormWindow *)
    { ++formWindowAdded_signal_count; }

    void formWindowRemoved(AbstractFormWindow *)
    { ++formWindowRemoved_signal_count; }
};

void tst_AbstractFormWindowManager::formWindowAdded()
{
    AbstractFormWindowManager *formManager = IDEApplication::core()->formManager();

    { // test the SIGNAL AbstractFormWindowManager::formWindowAdded()
        Helper h;
        connect(formManager, SIGNAL(formWindowAdded(AbstractFormWindow*)),
            &h, SLOT(formWindowAdded(AbstractFormWindow*)));

        COMPARE(h.formWindowAdded_signal_count, 0);

        {
            AbstractFormWindow *formWindow = formManager->createFormWindow(/*parent=*/ 0);
            COMPARE(h.formWindowAdded_signal_count, 1);
            delete formWindow;
        }

        {
            AbstractFormWindow *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
            AbstractFormWindow *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);

            COMPARE(h.formWindowAdded_signal_count, 3);

            delete formWindow1;
            delete formWindow2;
        }
    }
}

void tst_AbstractFormWindowManager::formWindowRemoved()
{
    AbstractFormWindowManager *formManager = IDEApplication::core()->formManager();

    { // test the SIGNAL AbstractFormWindowManager::formWindowRemoved()
        Helper h;
        connect(formManager, SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            &h, SLOT(formWindowRemoved(AbstractFormWindow*)));

        COMPARE(h.formWindowRemoved_signal_count, 0);

        {
            AbstractFormWindow *formWindow = formManager->createFormWindow(/*parent=*/ 0);
            delete formWindow;
            COMPARE(h.formWindowRemoved_signal_count, 1);
        }

        {
            AbstractFormWindow *formWindow1 = formManager->createFormWindow(/*parent=*/ 0);
            AbstractFormWindow *formWindow2 = formManager->createFormWindow(/*parent=*/ 0);

            delete formWindow1;
            delete formWindow2;

            COMPARE(h.formWindowRemoved_signal_count, 3);
        }
    }
}

#include "tst_abstractformwindowmanager.moc"
