#include <QtTest/QtTest>

#ifdef Q_WS_MAC

#include <private/qt_mac_p.h>
#undef verify // yes, lets reserve the word "verify"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QPushButton>
#include <QToolBar>
#include <QSlider>
#include <QListWidget>
#include <QAccessibleInterface>
#include <QAccessible>

#include <quiloader.h>

#include <sys/types.h> // for getpid()
#include <unistd.h>

Q_DECLARE_METATYPE(AXUIElementRef);

typedef QCFType<CFArrayRef> QCFArrayRef;

class tst_accessibiliry_mac : public QObject
{
Q_OBJECT
public slots:
    void printInfo();
    void testForm();
    void testButtons();
    void testLineEdit();
    void testLabel();
    void testGroups();
    void testTabs();
    void testComboBox();
    void testDeleteWidget();
    void testMultipleWindows();
    void testHiddenWidgets();
    void testActions();
    void testChangeState();
    void testSlider();
    void testListView();
    void testScrollBar();
    void testSplitter();
private slots:
    // ui tests load an .ui file.
    void uitest_data();
    void uitest();
    
    void tests_data();
    void tests();
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

QStringList toQStringList(const CFArrayRef array)
{
    const int numElements = CFArrayGetCount(array);
    QStringList qtStrings;
    
    for (int i = 0; i < numElements; ++i) {
        CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(array, i);
        qtStrings.append(QCFString::toQString(str));
    }
    
    return qtStrings;
}

QVariant AXValueToQVariant(AXValueRef value)
{
    QVariant var;
    const AXValueType type = AXValueGetType(value);
    switch (type) {
        case kAXValueCGPointType : {
            CGPoint point;
            if (AXValueGetValue(value, type, &point))
                var = QPointF(point.x, point.y);
        } break;
        case kAXValueCGSizeType : {
            CGSize size;
            if (AXValueGetValue(value, type, &size))
                var = QSizeF(size.width, size.height);
        } break;
        case kAXValueCGRectType :  {
            CGRect rect;
            if (AXValueGetValue(value, type, &rect))
                var = QRectF(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
        } break;
        case kAXValueCFRangeType :
        case kAXValueAXErrorType :  
        case kAXValueIllegalType :
        default:
        qDebug() << "Illegal/Unsuported AXValue:" << type;
        break;
    };
    return var;
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
    } else if (typeID == AXValueGetTypeID()) {
        var = AXValueToQVariant((AXValueRef)value);
    } else if (typeID == CFNumberGetTypeID()) {
        CFNumberRef number = (CFNumberRef)value;
        if (CFNumberGetType(number) != kCFNumberSInt32Type)
            qDebug() << "unsupported number type" << CFNumberGetType(number);
        int theNumber;
        CFNumberGetValue(number, kCFNumberSInt32Type, &theNumber);
        var.setValue(theNumber);
    } else if (typeID == CFArrayGetTypeID()) {
        CFArrayRef cfarray = static_cast<CFArrayRef>(value);
        QVariantList list;
        CFIndex size = CFArrayGetCount(cfarray);
        for (CFIndex i = 0; i < size; ++i)
            list << CFTypeToQVariant(CFArrayGetValueAtIndex(cfarray, i));
        var.setValue(list);
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
    CFArrayRef array;
    AXError err = AXUIElementCopyAttributeNames(element, &array);
    if (err) {
        testError(err, QLatin1String("unexpected error when testing for supported attribute") + QCFString::toQString(attribute));
        return false;
    }
    CFRange range;
    range.location = 0;
    range.length = CFArrayGetCount(array);
    return CFArrayContainsValue(array, range, attribute);
}

/*
    Returns the accessibility attribute specified with attribute in a QVariant
*/
QVariant attribute(AXUIElementRef element, CFStringRef attribute)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, attribute, &value);

    testError(err, QString("Error getting element attribute ") + QCFString::toQString(attribute));

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
    Returns the subrole for an element.
*/
QString subrole(AXUIElementRef element)
{
    return attribute(element, kAXSubroleAttribute).toString();
}

/*
    Returns the role description for an element.
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
    Returns thie size of the element.
*/
QSizeF size(AXUIElementRef element)
{
    return attribute(element, kAXSizeAttribute).value<QSizeF>();
}

/*
    Returns the position of the element.
*/
QPointF position(AXUIElementRef element)
{
    return attribute(element, kAXPositionAttribute).value<QPointF>();
}

QList<AXUIElementRef> tabs(AXUIElementRef element)
{
    CFTypeRef value;
    AXError err = AXUIElementCopyAttributeValue(element, kAXTabsAttribute, &value);
    if (err)
        return QList<AXUIElementRef>();
        
    CFArrayRef array = (CFArrayRef)value;
    QList<AXUIElementRef> elements;
    const int count = CFArrayGetCount(array);
    for (int i = 0; i < count; ++i)
        elements.append((AXUIElementRef)CFArrayGetValueAtIndex(array, i));
   
    return elements;
}

QList<AXUIElementRef> elementListAttribute(AXUIElementRef element, CFStringRef attributeName)
{
    QList<AXUIElementRef> elementList;
    QVariantList variants = attribute(element, attributeName).value<QVariantList>();
    foreach(QVariant variant, variants)
        elementList.append(variant.value<AXUIElementRef>());
    return elementList;
}

/*
    Returns the UIElement at the given position.
*/
AXUIElementRef childAtPoint(QPointF position)
{
    AXUIElementRef element = 0;
    const AXError err = AXUIElementCopyElementAtPosition(AXUIElementCreateApplication(getpid()), position.x(), position.y(), &element);
    if (err) {
        qDebug() << "Error getting element at " << position;
        return 0;
    }

    return element;
}

/*
    Returns a QStringList containing the names of the actions the ui element supports
*/
QStringList actionNames(AXUIElementRef element)
{
    CFArrayRef cfStrings;
    const AXError err = AXUIElementCopyActionNames(element, &cfStrings);
    testError(err, "Unable to get action names");
    return toQStringList(cfStrings);
}

bool supportsAction(const AXUIElementRef element, const QString &actionName)
{
    const QStringList actions = actionNames(element);
    return actions.contains(actionName);
}

bool performAction(const AXUIElementRef element, const QString &actionName)
{
    const AXError err = AXUIElementPerformAction(element, QCFString(actionName));
    return (err == 0);
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


CFArrayRef childrenArray(AXUIElementRef element)
{
    CFTypeRef value = 0;
    AXError err = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, &value);
    if (!err && CFGetTypeID(value) == CFArrayGetTypeID()) {
        return (CFArrayRef)value;
    }
   
    return CFArrayCreate(0,0,0,0);
}

/*
    Gest the child count from an element.
*/
int numChildren(AXUIElementRef element)
{
    return CFArrayGetCount(childrenArray(element));
}

/*
    Gets the child with index childIndex from element. Returns 0 if not found.
*/
AXUIElementRef child(AXUIElementRef element, int childIndex)
{
    CFArrayRef children = childrenArray(element);
    if (childIndex >= CFArrayGetCount(children))
        return 0;

    const void *data  = CFArrayGetValueAtIndex(children, childIndex);
    return (AXUIElementRef)data;
}

/*
    Gets the child titled childTitle from element. Returns 0 if not found.
*/
AXUIElementRef childByTitle(AXUIElementRef element, const QString &childTitle)
{
    CFArrayRef children  = childrenArray(element);
    const int numChildren = CFArrayGetCount(children);
    for (int i = 0; i < numChildren; ++i) {
        const AXUIElementRef childElement = (AXUIElementRef)CFArrayGetValueAtIndex(children, i);
        // Test for support for title attribute before getting it to avoid test fail.
        if (supportsAttribute(childElement, kAXTitleAttribute) && title(childElement) == childTitle)
            return childElement;
    }
    return 0;
}

/*
    Gets the child with the given value from element. Returns 0 if not found.
*/
AXUIElementRef childByValue(AXUIElementRef element, const QVariant &testValue)
{
    CFArrayRef children  = childrenArray(element);
    const int numChildren = CFArrayGetCount(children);
    for (int i = 0; i < numChildren; ++i) {
        const AXUIElementRef childElement = (AXUIElementRef)CFArrayGetValueAtIndex(children, i);
        // Test for support for title attribute before getting it to avoid test fail.
        if (supportsAttribute(childElement, kAXValueAttribute) && value(childElement) == testValue)
            return childElement;
    }
    return 0;
}

/*
    Gets the child by role from element. Returns 0 if not found.
*/
AXUIElementRef childByRole(AXUIElementRef element, const QString &macRole)
{
    CFArrayRef children  = childrenArray(element);
    const int numChildren = CFArrayGetCount(children);
    for (int i = 0; i < numChildren; ++i) {
        const AXUIElementRef childElement = (AXUIElementRef)CFArrayGetValueAtIndex(children, i);
        if (role(childElement) == macRole)
            return childElement;
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

int indent = 0;
QString space()
{
    QString space;
    for (int i = 0; i < indent; ++i) {
        space += " ";
    }
    return space;
}


/*
    Recursively prints cccesibility info for currentElement and all its children.
*/
void printElementInfo(AXUIElementRef currentElement)
{
    if (HIObjectIsAccessibilityIgnored(AXUIElementGetHIObject(currentElement))) {
        qDebug() << space() << "Ignoring element with role" << role(currentElement);
        return;
    }
    
    qDebug() << space() <<"Role" << role(currentElement);
    if (supportsAttribute(currentElement, kAXTitleAttribute))
        qDebug() << space() << "Title" << title(currentElement);
    else
        qDebug() << space() << "Title not supported";
    
    if (supportsAttribute(currentElement, kAXValueAttribute))
        qDebug() << space() << "Value" << attribute(currentElement, kAXValueAttribute);
    else
        qDebug() << space() << "Value not supported";
    
    qDebug() << space() << "Number of children" << numChildren(currentElement);
    for (int i = 0; i < numChildren(currentElement); ++i) {
        AXUIElementRef childElement = child(currentElement, i);
        // Skip the menu bar.
        if (role(childElement) != "AXMenuBar") {
            indent+= 4;
            printElementInfo(childElement);
            indent-= 4;
        }
    }
    qDebug() << " ";
}

bool isIgnored(AXUIElementRef currentElement)
{
    return HIObjectIsAccessibilityIgnored(AXUIElementGetHIObject(currentElement));
}

bool equal(CFTypeRef o1, CFTypeRef o2)
{
    if (o1 == 0 || o2 == 0)
        return false;
    return CFEqual(o1, o2);
}

/*
    Verifies basic element info.
*/
#define VERIFY_ELEMENT(element, _parent, _role) \
    QVERIFY(element != 0); \
    QVERIFY(role(element) == _role); \
    QVERIFY(equal(::parent(element), _parent));
/*
    Verifies that the application and the main form is there has the right info.
*/
void testAppAndForm(AXUIElementRef application)
{
    QVERIFY(title(application) == "tst_qaccessibility_mac");
    QVERIFY(role(application) == "AXApplication");

    AXUIElementRef form = childByTitle(application, "Form");
    VERIFY_ELEMENT(form, application, "AXWindow");
}

void tst_accessibiliry_mac::printInfo()
{
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    printElementInfo(currentApplication);
}

/*
    Tests for form.ui
*/
void tst_accessibiliry_mac::testForm()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    testAppAndForm(currentApplication);
    childByTitle(currentApplication, "Form");
}

/*
    Tests for buttons.ui
*/
void tst_accessibiliry_mac::testButtons()
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

void tst_accessibiliry_mac::testLabel()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());

    testAppAndForm(currentApplication);
    AXUIElementRef form = childByTitle(currentApplication, "Form");
    AXUIElementRef label = childByValue(form, "This is a Text Label");
    VERIFY_ELEMENT(label, form, "AXStaticText");
    VERIFY_ROLE_DESCRIPTION(label, "text");
}

/*
    Tests for lineedit.ui
*/
void tst_accessibiliry_mac::testLineEdit()
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
void tst_accessibiliry_mac::testGroups()
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

/*
    Tests for tabs.ui
*/
void tst_accessibiliry_mac::testTabs()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());

    testAppAndForm(currentApplication);
    const QString formTitle = "Form";
    AXUIElementRef form = childByTitle(currentApplication, formTitle);
    
    const QString tabRole = "AXTabGroup";
    AXUIElementRef tabGroup = childByRole(form, tabRole);
    QVERIFY(tabGroup != 0);
    
    // Test that we have two child buttons (the tabs)
    const int numChildren = ::numChildren(tabGroup);
    QVERIFY(numChildren == 2);
    
    const QString tab1Title = "Tab 1";
    AXUIElementRef tabButton1 = childByTitle(tabGroup, tab1Title);
    QVERIFY (tabButton1);
    VERIFY_ELEMENT(tabButton1, tabGroup, "AXRadioButton");
    QCOMPARE(title(tabButton1), tab1Title);

    const QString tab2Title = "Tab 2";
    const AXUIElementRef tabButton2 = childByTitle(tabGroup, tab2Title);
    QVERIFY(tabButton2);
    VERIFY_ELEMENT(tabButton2, tabGroup, "AXRadioButton");
    QCOMPARE(title(tabButton2), tab2Title);
    
    // Test that the window and top-level-ui-elment is the form.
    QVERIFY(equal(window(tabGroup), form));
    QVERIFY(equal(window(tabButton1), form));    
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    QVERIFY(equal(topLevelUIElement(tabGroup), form));
    QVERIFY(equal(topLevelUIElement(tabButton1), form));    
#endif
    // Test the bounding rectangles for the tab group and buttons.
    const QRectF groupRect(position(tabGroup), size(tabGroup));
    const QRectF tabButton1Rect(position(tabButton1), size(tabButton1));
    const QRectF tabButton2Rect(position(tabButton2), size(tabButton2));
    
    QVERIFY(groupRect.isNull() == false);
    QVERIFY(tabButton1Rect.isNull() == false);
    QVERIFY(tabButton1Rect.isNull() == false);
        
    QVERIFY(groupRect.contains(tabButton1Rect));
    QVERIFY(groupRect.contains(tabButton2Rect));
    QVERIFY(tabButton2Rect.contains(tabButton1Rect) == false);
    
    // Test the childAtPoint event.
    const AXUIElementRef childAtTab1Position = childAtPoint(position(tabButton1) + QPointF(5,5));
    QVERIFY(equal(childAtTab1Position, tabButton1));
    const AXUIElementRef childAtOtherPosition = childAtPoint(position(tabButton1) - QPointF(5,5));
    QVERIFY(equal(childAtOtherPosition, tabButton1) == false);

    // Test AXTabs attribute
    QVERIFY(supportsAttribute(tabGroup, kAXTabsAttribute));
    QList<AXUIElementRef> tabElements = tabs(tabGroup);
    QCOMPARE(tabElements.count(), 2);
    QVERIFY(equal(tabElements.at(0), tabButton1));
    QVERIFY(equal(tabElements.at(1), tabButton2));
    
    // Perform the press action on each child.
    for (int i = 0; i < numChildren; ++i) {
        const AXUIElementRef child = ::child(tabGroup, i);
        QVERIFY(supportsAction(child, "AXPress"));
        QVERIFY(performAction(child, "AXPress"));
    }
}

void tst_accessibiliry_mac::testComboBox()
{
    // Get reference to the current application.
    AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());

    testAppAndForm(currentApplication);
    const QString formTitle = "Form";
    AXUIElementRef form = childByTitle(currentApplication, formTitle);
    
    const QString comboBoxRole = "AXPopUpButton";
    AXUIElementRef comboBox = childByRole(form, comboBoxRole);
    QVERIFY(comboBox != 0);
}

void tst_accessibiliry_mac::testDeleteWidget()
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

void tst_accessibiliry_mac::testMultipleWindows()
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
    
    delete formA;
}

void tst_accessibiliry_mac::testHiddenWidgets()
{
    const QString windowTitle ="a widget";
    QWidget * const window = new QWidget(0);
    window->setWindowTitle(windowTitle);
    window->show();

    const AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    const AXUIElementRef windowElement = childByTitle(currentApplication, windowTitle);
    QVERIFY(windowElement);
    QCOMPARE(isIgnored(windowElement), false);
    
    const QString buttonTitle = "a button";
    QPushButton * const button = new QPushButton(window);    
    button->setText(buttonTitle);
    button->show();
    
    const AXUIElementRef buttonElement = childByTitle(windowElement, buttonTitle);
    QVERIFY(buttonElement);
    QCOMPARE(isIgnored(buttonElement), false);

    const QString toolbarTitle = "a toolbar";
    QToolBar * const toolbar = new QToolBar(toolbarTitle, window);
    toolbar->show();
    
    const AXUIElementRef toolBarElement = childByTitle(windowElement, toolbarTitle);
    QVERIFY(toolBarElement == 0);

    delete window;
};

void tst_accessibiliry_mac::testActions()
{
    // create a window with a push button
    const QString windowTitle ="a widget";
    QWidget * const window = new QWidget();
    window->setWindowTitle(windowTitle);
    window->show();

    const AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    const AXUIElementRef windowElement = childByTitle(currentApplication, windowTitle);
    QVERIFY(windowElement);

    const QString buttonTitle = "a button";
    QPushButton * const button = new QPushButton(window);    
    button->setText(buttonTitle);
    button->show();
    
    const AXUIElementRef buttonElement = childByTitle(windowElement, buttonTitle);
    QVERIFY(buttonElement);

    // Verify that the button has the Press action.
    const QStringList actions = actionNames(buttonElement);
    const QString pressActionName("AXPress");
    QVERIFY(actions.contains(pressActionName));
    
    // Press button and check the pressed signal
    QSignalSpy pressed(button, SIGNAL(pressed()));
    QVERIFY(performAction(buttonElement, pressActionName));
    QCOMPARE(pressed.count(), 1);
    
    pressed.clear();
    QVERIFY(performAction(buttonElement, QString("does not exist")));
    QCOMPARE(pressed.count(), 0);

    delete window;
};

void tst_accessibiliry_mac::testChangeState()
{
    const QString windowTitle ="a widget";
    QWidget * const window = new QWidget();
    window->setWindowTitle(windowTitle);
    window->show();
 
    const AXUIElementRef applicationElement = AXUIElementCreateApplication(getpid());
    const AXUIElementRef windowElement = childByTitle(applicationElement, windowTitle);
    QVERIFY(windowElement);
    const int otherChildren = numChildren(windowElement);
    
    const QString buttonTitle = "Button";
    QPushButton * const button = new QPushButton(buttonTitle, window);
    button->setText(buttonTitle);  

    // Test that show/hide adds/removes the button from the hierachy.
    QVERIFY(childByTitle(windowElement, buttonTitle) == 0);
    QCOMPARE(numChildren(windowElement), otherChildren);
    button->show();
    QVERIFY(childByTitle(windowElement, buttonTitle) != 0);
    QCOMPARE(numChildren(windowElement), otherChildren + 1);
    button->hide();
    QVERIFY(childByTitle(windowElement, buttonTitle) == 0);
    QCOMPARE(numChildren(windowElement), otherChildren);
    button->show();
    QVERIFY(childByTitle(windowElement, buttonTitle) != 0);
    QCOMPARE(numChildren(windowElement), otherChildren + 1);

    // Test that hiding and showing a widget also removes and adds all its children.
    {
        QWidget * const parent = new QWidget(window);
        const int otherChildren = numChildren(windowElement);
        
        QPushButton * const child = new QPushButton(parent);
        const QString childButtonTitle = "child button";
        child->setText(childButtonTitle);
        
        parent->show();
        QVERIFY(childByTitle(windowElement, childButtonTitle) != 0);
        QCOMPARE(numChildren(windowElement), otherChildren + 1);

        parent->hide();
        QVERIFY(childByTitle(windowElement, childButtonTitle) == 0);
        QCOMPARE(numChildren(windowElement), otherChildren );

        parent->show();
        QVERIFY(childByTitle(windowElement, childButtonTitle) != 0);
        QCOMPARE(numChildren(windowElement), otherChildren + 1);
        
        delete parent;
    }

    // Test that the enabled attribute is updated after a call to setEnabled.
    const AXUIElementRef buttonElement = childByTitle(windowElement, buttonTitle);
    QVERIFY(enabled(buttonElement));
    button->setEnabled(false);
    QVERIFY(enabled(buttonElement) == false);
    button->setEnabled(true);
    QVERIFY(enabled(buttonElement));

    // Test that changing the title updates the accessibility information.
    const QString buttonTitle2 = "Button 2";    
    button->setText(buttonTitle2);
    QVERIFY(childByTitle(windowElement, buttonTitle2) != 0);
    QVERIFY(childByTitle(windowElement, buttonTitle) == 0);

    delete window;
}

void tst_accessibiliry_mac::testSlider()
{
    const QString windowTitle = "a widget";
    QWidget * const window = new QWidget();
    window->setWindowTitle(windowTitle);
    window->show();
 
    const AXUIElementRef applicationElement = AXUIElementCreateApplication(getpid());
    QVERIFY(applicationElement);
    const AXUIElementRef windowElement = childByTitle(applicationElement, windowTitle);
    QVERIFY(windowElement);
    const int windowChildren = numChildren(windowElement);
    
    QSlider * const slider = new QSlider(window);
    slider->show();
    const AXUIElementRef sliderElement = childByRole(windowElement, "AXSlider");
    QVERIFY(sliderElement);

    // Test that the slider and its children are removed from the hierachy when we call hide().
    QCOMPARE(numChildren(windowElement), windowChildren + 1);
    slider->hide();
    QCOMPARE(numChildren(windowElement), windowChildren);
    
    delete slider;
}

void tst_accessibiliry_mac::testListView()
{
    QListWidget *listWidget = new QListWidget();

    listWidget->setObjectName("listwidget");
    const QString windowTitle("window");
    listWidget->setWindowTitle(windowTitle);


    qDebug()  << "object is" << listWidget;
    listWidget->addItem("A");
    listWidget->addItem("B");
    listWidget->addItem("C");
    listWidget->show();


    QAccessibleInterface *f = QAccessible::queryAccessibleInterface(listWidget);
/*
    qDebug() << f->childCount();
    qDebug() << hex << f->role(0);
    qDebug() << hex << f->role(1);
    qDebug() << f->isValid();
    qDebug() << f->childCount();
    qDebug() << f->text(QAccessible::Value, 1);
    qDebug() << f->text(QAccessible::Value, 2);
    qDebug() << f->text(QAccessible::Value, 3);

    QAccessibleInterface *child;
    qDebug() << f->navigate(QAccessible::Child, 1, &child);
    qDebug() << child->text(QAccessible::Value, 1);
    qDebug() << f->indexOfChild(child);
  */     

/*
    const AXUIElementRef applicationElement = AXUIElementCreateApplication(getpid());
    QVERIFY(applicationElement);
    const AXUIElementRef windowElement = childByTitle(applicationElement, windowTitle);
    QVERIFY(windowElement);

    const AXUIElementRef listElement = childByRole(windowElement, "AXList");
    QVERIFY(listElement);


    printElementInfo(listElement);
*/
    delete listWidget;
}

void tst_accessibiliry_mac::testScrollBar()
{
    const AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    testAppAndForm(currentApplication);
    const AXUIElementRef form = childByTitle(currentApplication, "Form");
    QVERIFY(form);
    
    const AXUIElementRef scrollBarElement = childByRole(form, "AXScrollBar");
    QVERIFY(scrollBarElement);
    QCOMPARE(attribute(scrollBarElement, kAXOrientationAttribute).toString(), QLatin1String("AXVerticalOrientation"));

    {
        const AXUIElementRef lineUpElement = childByTitle(scrollBarElement, "Line up");
        QVERIFY(lineUpElement);
        QCOMPARE (subrole(lineUpElement), QLatin1String("AXDecrementArrow"));
    }{
        const AXUIElementRef lineDownElement = childByTitle(scrollBarElement, "Line down");
        QVERIFY(lineDownElement);
        QCOMPARE (subrole(lineDownElement), QLatin1String("AXIncrementArrow"));
    }{
        const AXUIElementRef pageUpElement = childByTitle(scrollBarElement, "Page up");
        QVERIFY(pageUpElement);
        QCOMPARE (subrole(pageUpElement), QLatin1String("AXDecrementPage"));
    }{
        const AXUIElementRef pageDownElement = childByTitle(scrollBarElement, "Page down");
        QVERIFY(pageDownElement);
        QCOMPARE (subrole(pageDownElement), QLatin1String("AXIncrementPage"));
    }{
        const AXUIElementRef valueIndicatorElement = childByTitle(scrollBarElement, "Position");
        QVERIFY(valueIndicatorElement);
        QCOMPARE(value(valueIndicatorElement).toInt(), 50);
    }
}

void tst_accessibiliry_mac::testSplitter()
{
    const AXUIElementRef currentApplication = AXUIElementCreateApplication(getpid());
    testAppAndForm(currentApplication);
    const AXUIElementRef form = childByTitle(currentApplication, "Form");
    QVERIFY(form);
    
    const AXUIElementRef splitGroupElement = childByRole(form, "AXSplitGroup");
    QVERIFY(splitGroupElement);
    QList<AXUIElementRef> splitterList = elementListAttribute(splitGroupElement, kAXSplittersAttribute);
    QCOMPARE(splitterList.count(), 2); 


    // context1..3 and splitter1..2 below are numbered according to left-to-right
    // visual order, but in since 4.3 the actual child ordering does not
    // match the visual order.
    const AXUIElementRef contents1 = child(splitGroupElement, 4);
    QVERIFY(contents1);
    QCOMPARE(role(contents1), QLatin1String("AXStaticText"));
    QCOMPARE(title(contents1), QLatin1String("Foo"));

    const AXUIElementRef contents2 = child(splitGroupElement, 3);
    QVERIFY(contents2);
    QCOMPARE(role(contents2), QLatin1String("AXStaticText"));
    QCOMPARE(title(contents2), QLatin1String("Bar"));

    const AXUIElementRef contents3 = child(splitGroupElement, 1);
    QVERIFY(contents3);
    QCOMPARE(role(contents3), QLatin1String("AXStaticText"));
    QCOMPARE(title(contents3), QLatin1String("Baz"));

    const AXUIElementRef splitter1 = child(splitGroupElement, 2);
    QCOMPARE(role(splitter1), QLatin1String("AXSplitter"));
    QVERIFY(splitter1);
    QVERIFY(equal(splitterList.at(0), splitter1));
    QVERIFY(supportsAttribute(splitter1, kAXPreviousContentsAttribute));
    QVERIFY(supportsAttribute(splitter1, kAXNextContentsAttribute));
    QCOMPARE(attribute(splitter1, kAXOrientationAttribute).toString(), QLatin1String("AXVerticalOrientation"));
    {
        QList<AXUIElementRef> prevList = elementListAttribute(splitter1, kAXPreviousContentsAttribute);  
        QCOMPARE(prevList.count(), 1); 
        QVERIFY(equal(prevList.at(0), contents1));
        QList<AXUIElementRef> nextList = elementListAttribute(splitter1, kAXNextContentsAttribute);  
        QCOMPARE(nextList.count(), 1);
        QVERIFY(equal(nextList.at(0), contents2)); 
    }

    const AXUIElementRef splitter2 = child(splitGroupElement, 0);
    QCOMPARE(role(splitter2), QLatin1String("AXSplitter"));
    QVERIFY(splitter2);
    QVERIFY(equal(splitterList.at(1), splitter2));
    {
        QList<AXUIElementRef> prevList = elementListAttribute(splitter2, kAXPreviousContentsAttribute);  
        QCOMPARE(prevList.count(), 1);
        QVERIFY(equal(prevList.at(0), contents2));
        QList<AXUIElementRef> nextList = elementListAttribute(splitter2, kAXNextContentsAttribute);  
        QCOMPARE(nextList.count(), 1);
        QVERIFY(equal(nextList.at(0), contents3)); 
    }
}


void tst_accessibiliry_mac::uitest_data()
{
    QTest::addColumn<QString>("uiFilename");
    QTest::addColumn<QString>("testSlot");

    QTest::newRow("form") << "form.ui" << SLOT(testForm());
    QTest::newRow("buttons") << "buttons.ui" << SLOT(testButtons());
    QTest::newRow("label") << "label.ui" << SLOT(testLabel());
    QTest::newRow("line edit") << "lineedit.ui" << SLOT(testLineEdit());
    QTest::newRow("groups") << "groups.ui" << SLOT(testGroups());
    QTest::newRow("tabs") << "tabs.ui" << SLOT(testTabs());
    QTest::newRow("combobox") << "combobox.ui" << SLOT(testComboBox());
    QTest::newRow("scrollbar") << "scrollbar.ui" << SLOT(testScrollBar());
    QTest::newRow("splitters") << "splitters.ui" << SLOT(testSplitter());
}

void tst_accessibiliry_mac::uitest()
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
    QUiLoader loader;
    QFile file(uiFilename);
    QVERIFY(file.exists());
    file.open(QFile::ReadOnly);
    QWidget *window = loader.load(&file, 0);
    QVERIFY(window);
    file.close();
    window->show();

    QTimer::singleShot(50, this, qPrintable(testSlot));
    // Quit when returning to the main event loop after running tests.
    QTimer::singleShot(200, &app, SLOT(quit()));
    app.exec();
    delete window;
}

void tst_accessibiliry_mac::tests_data()
{
    QTest::addColumn<QString>("testSlot");
    QTest::newRow("deleteWidget") << SLOT(testDeleteWidget());
    QTest::newRow("multipleWindows") << SLOT(testMultipleWindows());
    QTest::newRow("hiddenWidgets") << SLOT(testHiddenWidgets());
    QTest::newRow("actions") << SLOT(testActions());
    QTest::newRow("changeState") << SLOT(testChangeState());
    QTest::newRow("slider") << SLOT(testSlider());
    QTest::newRow("listView") << SLOT(testListView());
}

void tst_accessibiliry_mac::tests()
{
    QFETCH(QString, testSlot);
    runTest(testSlot);
}

/*
    Tests show that querying the accessibility interface directly does not work. (I get a
    kAXErrorAPIDisabled error, indicating that the accessible API is disabled, which it isn't.)
    To work around this, we run the tests in a callback slot called from the main event loop.
*/
void tst_accessibiliry_mac::runTest(const QString &testSlot)
{
    // The Accessibility interface must be enabled to run this test.
    if (!AXAPIEnabled())
        QSKIP("Accessibility not enabled. Check \"Enable access for assistive devices\" in the system preferences -> universal access to run this test.", SkipAll);

    int argc = 0;
    char **argv = 0;
    QApplication app(argc, argv);

    QTimer::singleShot(50, this, qPrintable(testSlot));
    // Quit when returning to the main event loop after running tests.
    QTimer::singleShot(200, &app, SLOT(quit()));
    app.exec();

}

QTEST_APPLESS_MAIN(tst_accessibiliry_mac)

#else // Q_WS_MAC

QTEST_NOOP_MAIN

#endif

#include "tst_accessibility_mac.moc"


