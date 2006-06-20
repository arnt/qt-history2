/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if 0

#include <private/qt_mac_p.h>
#undef verify // yes, lets reserve the word "verify"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QPushButton>
#include <QtDesigner/QFormBuilder>

#include <sys/types.h> // for getpid()
#include <unistd.h>

Q_DECLARE_METATYPE(AXUIElementRef);

class tst_foo : public QObject
{
Q_OBJECT
private slots:
    void uitest_data();
    void uitest();
    void deleteWidget();
    void multipleWindows();
private:
    void runTest(const QString &testSlot);
};

/*
    VERIFYs that there is no error and prints an error message if there is.
*/
void testError(AXError error, const QString &text)
{
    if (error)
        qDebug() << "Error" << error << text;
    QVERIFY(error == 0);
}

/*
    Prints an CFArray holding CFStrings.
*/
void printCFStringArray(CFArrayRef array, const QString &title)
{
    const int numElements = CFArrayGetCount(array);
    qDebug() << "num" << title << " " <<  numElements;

    for (int i = 0; i < numElements; ++i) {
       CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(array, i);
       qDebug() << QCFString::toQString(str);
    }
}

/*
    Converts a CFTypeRef to a QVariant, for certain selected types. Prints
    an error message and returns QVariant() if the type is not supported.
*/
QVariant CFTypeToQVariant(CFTypeRef value)
{
    QVariant var;
    if (value == 0)
        return var;
    const uint typeID = CFGetTypeID(value);
    if (typeID == CFStringGetTypeID()) {
        var.setValue(QCFString::toQString((CFStringRef)value));
    } else if (typeID == CFBooleanGetTypeID()) {
        var.setValue((bool)CFBooleanGetValue((CFBooleanRef)value));
    } else if (typeID == AXUIElementGetTypeID()) {
        var.setValue((AXUIElementRef)value);
    } else {
        QCFString str = CFCopyTypeIDDescription(typeID);
        qDebug() << "Unknown CFType: " << typeID << (QString)str;
    }
    return var;
}

/*
    Tests if a given attribute is supported by an element. Expects either
    no error or error -25205 (Not supported). Causes a test failure
    on other error values.
*/
bool supportsAttribute(AXUIElementRef element, CFStringRef attribute)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, attribute, &value);
    if (err == 0)
        return true;
    else if(err == -25205) { // Error -25205: Unsupported.
        return false;
    }

    testError(err, QLatin1String("unexpected error when testing for supported attribute") + QCFString::toQString(attribute));
    return false;
}

/*
    Returns the accessibility attribute specified with attribute in a QVariant
*/
QVariant attribute(AXUIElementRef element, CFStringRef attribute)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, attribute, &value);

    testError(err, QString("Error getting element attribute") + QCFString::toQString(attribute));

    if (err)
        return QVariant();

    return CFTypeToQVariant(value);
}

/*
    Returns the title for an element.
*/
QString title(AXUIElementRef element)
{
    return attribute(element, kAXTitleAttribute).toString();
}

/*
    Returns the role for an element.
*/
QString role(AXUIElementRef element)
{
    return attribute(element, kAXRoleAttribute).toString();
}

/*
    Returns the role for an element.
*/
QString roleDescription(AXUIElementRef element)
{
    return attribute(element, kAXRoleDescriptionAttribute).toString();
}

/*
    Returns the enabled attribute for an element.
*/
bool enabled(AXUIElementRef element)
{
    return attribute(element, kAXEnabledAttribute).toBool();
}

/*
    Returns the value attribute for an element as an QVariant.
*/
QVariant value(AXUIElementRef element)
{
    return attribute(element, kAXValueAttribute);
}

/*
    Returns the value attribute for an element as an bool.
*/
bool boolValue(AXUIElementRef element)
{
    return attribute(element, kAXValueAttribute).toBool();
}

/*
    Returns the parent for an element
*/
AXUIElementRef parent(AXUIElementRef element)
{
    return attribute(element, kAXParentAttribute).value<AXUIElementRef>();
}

/*
    Returns the (top-level) window(not a sheet or a drawer) for an element
*/
AXUIElementRef window(AXUIElementRef element)
{
    return attribute(element, kAXWindowAttribute).value<AXUIElementRef>();
}

/*
    Returns the (top-level) UI element(can also be a sheet or drawer) for an element
*/
AXUIElementRef topLevelUIElement(AXUIElementRef element)
{
    return attribute(element, kAXTopLevelUIElementAttribute).value<AXUIElementRef>();
}

/*
    Om 10.4 and up, verifyes the AXRoleDescription attribute for an element,
    on 10.3 and below this test always passes.

    The reason for this is that the HICopyAccessibilityRoleDescription call
    used to implement this functionality was introduced in 10.4.
*/
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    #define VERIFY_ROLE_DESCRIPTION(ELEMENT,  TEXT) \
            QCOMPARE(roleDescription(ELEMENT), QString(TEXT))
#else
    #define VERIFY_ROLE_DESCRIPTION(ELEMENT,  TEXT) QVERIFY(true)
#endif

/*
    Gest the child count from an element.
*/
int numChildren(AXUIElementRef element)
{
    CFTypeRef value = 0;
    int ret = 0;
    AXError err = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, &value);
    if (!err && CFGetTypeID(value) == CFArrayGetTypeID()) {
        CFArrayRef children  = (CFArrayRef)value;
        ret = CFArrayGetCount(children);
    }
    return ret;
}

/*
    Gets the child with index childIndex from element. Returns 0 if not found.
*/
AXUIElementRef child(AXUIElementRef element, int childIndex)
{
    AXUIElementRef ret = 0;
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, &value);
    if (!err && CFGetTypeID(value) == CFArrayGetTypeID()) {
        CFArrayRef children  = (CFArrayRef)value;
        const void *data  = CFArrayGetValueAtIndex(children, childIndex);
        // Don't know how to get a TypeID from data here.
        // So I'll just assume that it is a AXUIElementRef and cast it :(
        ret = (AXUIElementRef)data;
    }
    return ret;
}

/*
    Gets the child titled childTitle from element. Returns 0 if not found.
*/
AXUIElementRef childByTitle(AXUIElementRef element, const QString &childTitle)
{
    AXUIElementRef ret = 0;
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, &value);
    if (!err && CFGetTypeID(value) == CFArrayGetTypeID()) {
        CFArrayRef children  = (CFArrayRef)value;
        const int numChildren = CFArrayGetCount(children);
        for (int i = 0; i < numChildren; ++i) {
            const void *data = CFArrayGetValueAtIndex(children, i);
            // Don't know how to get a TypeID from data here.
            // So I'll just assume that it is a AXUIElementRef and cast it :(
            AXUIElementRef childElement = (AXUIElementRef)data;
            // Test for support for title attribute before getting it to avoid test fail.
            if (supportsAttribute(childElement, kAXTitleAttribute) && title(childElement) == childTitle) {
                return childElement;
            }
        }
    }
    return ret;
}

/*
    Gets the child with the given value from element. Returns 0 if not found.
*/
AXUIElementRef childByValue(AXUIElementRef element, const QVariant &testValue)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, &value);
    if (!err && CFGetTypeID(value) == CFArrayGetTypeID()) {
        CFArrayRef children  = (CFArrayRef)value;
        const int numChildren = CFArrayGetCount(children);
        for (int i = 0; i < numChildren; ++i) {
            const void *data  = CFArrayGetValueAtIndex(children, i);
            // Don't know how to get a TypeID from data here.
            // So I'll just assume that it is a AXUIElementRef and cast it :(
            AXUIElementRef childElement = (AXUIElementRef)data;
            if (supportsAttribute(childElement, kAXValueAttribute)) {
                QVariant value = attribute(childElement, kAXValueAttribute);
                if (value == testValue)
                    return childElement;
            }
        }
    }
    return 0;
}


void printTypeForAttribute(AXUIElementRef element, CFStringRef attribute)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, attribute, &value);
    if (!err) {
        qDebug() << "type id" << CFGetTypeID(value);
        QCFString str = CFCopyTypeIDDescription(CFGetTypeID(value));
        qDebug() << (QString)str;
    } else {
        qDebug() << "Attribute Get error" << endl;
    }
}


/*
    Recursively prints cccesibility info for currentElement and all its children.
*/
void printElementInfo(AXUIElementRef currentElement)
{
    qDebug() << "Role" << role(currentElement);
    if (supportsAttribute(currentElement, kAXTitleAttribute))
        qDebug() << "Title" << title(currentElement);
    if (supportsAttribute(currentElement, kAXValueAttribute))
        qDebug() << "Value" << attribute(currentElement, kAXValueAttribute);

    qDebug() << "Number of children" << numChildren(currentElement);
    for (int i = 0; i < numChildren(currentElement); ++i) {
        AXUIElementRef childElement = child(currentElement, i);
        // Skip the menu bar.
        if (role(childElement) != "AXMenuBar")
            printElementInfo(childElement);
    }
}

/*
    Tests show that querying the accessibility interface directly does not work. (I get a
    kAXErrorAPIDisabled error, indicating that the accessible API is disabled, which it isn't.)
    To work around this, we run the tests in a callback slot called from the main event loop.
*/
class Callback : public QObject
{
Q_OBJECT
    void testAppAndForm(AXUIElementRef application);
public slots:
    void printInfo();
    void testButtons();
    void testLineEdit();
    void testLabel();
    void testGroups();
    void deleteWidget();
    void multipleWindows();
};

/*
    Verifies basic element info.
*/
#define VERIFY_ELEMENT(element, _parent, _role) \
    QVERIFY(element != 0); \
    QVERIFY(role(element) == _role); \
    QVERIFY(title(::parent(element)) == title(_parent));

/*
    Verifies that the application and the main form is there has the right info.
*/
void Callback::testAppAndForm(AXUIElementRef application)
{
    QVERIFY(title(application) == "test");
    QVERIFY(role(application) == "AXApplication");

    AXUIElementRef form = childByTitle(application, "Form");
    VERIFY_ELEMENT(form, application, "AXWindow");
}

void Callback::printInfo()
{
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    printElementInfo(currentApplication);
}

/*
    Tests for buttons.ui
*/
void Callback::testButtons()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    testAppAndForm(currentApplication);
    AXUIElementRef form = childByTitle(currentApplication, "Form");

    AXUIElementRef ren = childByTitle(form, "Ren");
    VERIFY_ELEMENT(ren, form, "AXButton");
    QVERIFY(enabled(ren) == true);
    VERIFY_ROLE_DESCRIPTION(ren, "button");

    AXUIElementRef stimpy = childByTitle(form, "Stimpy");
    VERIFY_ELEMENT(stimpy, form, "AXRadioButton");
    QVERIFY(enabled(stimpy) == true);
    QVERIFY(value(stimpy).toInt() == 1); // checked;
    VERIFY_ROLE_DESCRIPTION(stimpy, "radio button");

    AXUIElementRef pinky = childByTitle(form, "Pinky");
    VERIFY_ELEMENT(pinky, form, "AXCheckBox");
    QVERIFY(enabled(pinky) == false);
    QVERIFY(value(pinky).toInt() == 0); // unchecked;
    VERIFY_ROLE_DESCRIPTION(pinky, "check box");

    AXUIElementRef brain = childByTitle(form, "Brain");
    VERIFY_ELEMENT(brain, form, "AXButton");
    VERIFY_ROLE_DESCRIPTION(brain, "button");
}

void Callback::testLabel()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
//    printElementInfo(currentApplication);

    testAppAndForm(currentApplication);
    AXUIElementRef form = childByTitle(currentApplication, "Form");
    AXUIElementRef label = childByTitle(form, "This is a Text Label");
    VERIFY_ELEMENT(label, form, "AXStaticText");
    VERIFY_ROLE_DESCRIPTION(label, "text");
}

/*
    Tests for lineedit.ui
*/
void Callback::testLineEdit()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());

    testAppAndForm(currentApplication);
    AXUIElementRef form = childByTitle(currentApplication, "Form");
    AXUIElementRef lineEdit = childByValue(form, "Line edit");
    VERIFY_ELEMENT(lineEdit, form, "AXTextField");
    VERIFY_ROLE_DESCRIPTION(lineEdit, "text field");
}

/*
    Tests for groups.ui
*/
void Callback::testGroups()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());

    testAppAndForm(currentApplication);
    AXUIElementRef form = childByTitle(currentApplication, "Form");

    AXUIElementRef groupA = childByTitle(form, "Group A");
    VERIFY_ELEMENT(groupA, form, "AXGroup");
    AXUIElementRef button1 = childByTitle(groupA, "PushButton 1");
    VERIFY_ELEMENT(button1, groupA, "AXButton");
    VERIFY_ROLE_DESCRIPTION(groupA, "group");

    AXUIElementRef groupB = childByTitle(form, "Group B");
    VERIFY_ELEMENT(groupB, form, "AXGroup");
    AXUIElementRef button3 = childByTitle(groupB, "PushButton 3");
    VERIFY_ELEMENT(button3, groupB, "AXButton");
}

void Callback::deleteWidget()
{
    const QString buttonTitle = "Hi there";
    QWidget *form = new QWidget(0, Qt::Window);
    form->setWindowTitle("Form");
    form->show();
    QPushButton *button = new QPushButton(buttonTitle, form);
    button->show();

    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    testAppAndForm(currentApplication);
    AXUIElementRef formElement = childByTitle(currentApplication, "Form");

    AXUIElementRef buttonElement = childByTitle(formElement, buttonTitle);
    QVERIFY(buttonElement);

    button->hide();
    delete button;

    buttonElement = childByTitle(formElement, buttonTitle);
    QVERIFY(!buttonElement);
}

void Callback::multipleWindows()
{
    const QString formATitle("FormA");
    const QString formBTitle("FormB");

       // Create a window
    QWidget *formA = new QWidget(0, Qt::Window);
    formA->setWindowTitle(formATitle);
    formA->show();

    // Test if we can access the window
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    AXUIElementRef formAElement = childByTitle(currentApplication, formATitle);
    QVERIFY(formAElement);

    // Create another window
    QWidget *formB = new QWidget(0, Qt::Window);
    formB->setWindowTitle(formBTitle);
    formB->show();

    // Test if we can access both windows
    formAElement = childByTitle(currentApplication, formATitle);
    QVERIFY(formAElement);

    AXUIElementRef formBElement = childByTitle(currentApplication, formBTitle);
    QVERIFY(formBElement);
}

void tst_foo::uitest_data()
{
    QTest::addColumn<QString>("uiFilename");
    QTest::addColumn<QString>("testSlot");

    QTest::newRow("buttons") << "buttons.ui" << SLOT(testButtons());
    QTest::newRow("label") << "label.ui" << SLOT(testLabel());
    QTest::newRow("line edit") << "lineedit.ui" << SLOT(testLineEdit());
    QTest::newRow("groups") << "groups.ui" << SLOT(testGroups());

//    QTest::newRow("radio button") << "radiobutton.ui" << SLOT(printInfo());
//    QTest::newRow("print line edit") << "lineedit.ui" << SLOT(printInfo());
//    QTest::newRow("button") << "button.ui" << SLOT(printInfo());
//    QTest::newRow("blank") << "blank.ui" << SLOT(printInfo());
//    QTest::newRow("label-lineedit") << "label-lineedit.ui" << SLOT(printInfo());
//    QTest::newRow("label") << "label.ui" << SLOT(printInfo());
//    QTest::newRow("label-pushbutton") << "label-button.ui" << SLOT(printInfo());
}

void tst_foo::uitest()
{
    QFETCH(QString, uiFilename);
    QFETCH(QString, testSlot);

    // The Accessibility interface must be enabled to run this test.
    if (!AXAPIEnabled())
        QSKIP("Accessibility not enabled. Check \"Enable access for assistive devices\" in the system preferences -> universal access to run this test.", SkipAll);

    int argc = 0;
    char **argv = 0;
    QApplication app(argc, argv);

    // Create and display form.
    QFormBuilder builder;
    QFile file(uiFilename);
    QVERIFY(file.exists());
    file.open(QFile::ReadOnly);
    QWidget *window = builder.load(&file, 0);
    QVERIFY(window);
    file.close();
    window->show();

    // Run tests after 50 msecs.
    Callback callback;
    QTimer::singleShot(50, &callback, qPrintable(testSlot));
    // Quit when returning to the main event loop after running tests.
    QTimer::singleShot(60, &app, SLOT(quit()));
    app.exec();
}

void tst_foo::deleteWidget()
{
    runTest(SLOT(deleteWidget()));
}

void tst_foo::multipleWindows()
{
    runTest(SLOT(multipleWindows()));
}

void tst_foo::runTest(const QString &testSlot)
{
    // The Accessibility interface must be enabled to run this test.
    if (!AXAPIEnabled())
        QSKIP("Accessibility not enabled. Check \"Enable access for assistive devices\" in the system preferences -> universal access to run this test.", SkipAll);

    int argc = 0;
    char **argv = 0;
    QApplication app(argc, argv);

    Callback callback;
    QTimer::singleShot(50, &callback, qPrintable(testSlot));
    // Quit when returning to the main event loop after running tests.
    QTimer::singleShot(60, &app, SLOT(quit()));
    app.exec();
}

QTEST_APPLESS_MAIN(tst_foo)

#else // Q_OS_MAC

QTEST_NOOP_MAIN

#endif

#include "tst_accessibility_mac.moc"


