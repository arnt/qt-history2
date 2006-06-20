#include <QtTest/QtTest>
#include <QtGui/QPushButton>
#include <QtGui/QStyle>
#include <qdialogbuttonbox.h>
#include <limits.h>

//TESTED_CLASS=QDialogButtonBox
//TESTED_FILES=gui/qwidgets/qdialogbuttonbox.h gui/widgets/qdialogbuttonbox.cpp

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QDialogButtonBox::ButtonRole)
Q_DECLARE_METATYPE(QDialogButtonBox::StandardButton)
Q_DECLARE_METATYPE(QDialogButtonBox::StandardButtons)
Q_DECLARE_METATYPE(QAbstractButton*)

class tst_QDialogButtonBox : public QObject
{
    Q_OBJECT
public:
    tst_QDialogButtonBox();
    ~tst_QDialogButtonBox();

public slots:
    void buttonClicked1(QAbstractButton *);
    void acceptClicked();
    void rejectClicked();
    void helpRequestedClicked();

private slots:
    void testConstructor1();
    void testConstrurtor2();
    void testConstrurtor2_data();
    void testConstructor3();
    void testConstructor3_data();
    void setOrientation_data();
    void setOrientation();
    void addButton1_data();
    void addButton1();
    void addButton2_data();
    void addButton2();
    void addButton3_data();
    void addButton3();
    void clear_data();
    void clear();
    void removeButton_data();
    void removeButton();
    void buttonRole_data();
    void buttonRole();
    void setStandardButtons_data();
    void setStandardButtons();
    void standardButtons();

    // Skip these tests, buttons is used in every test thus far.
//    void buttons_data();
//    void buttons();

    void testDelete();
    void testRemove();
    void testMultipleAdd();
    void testStandardButtonMapping_data();
    void testStandardButtonMapping();
    void testSignals_data();
    void testSignals();
    void testSignalOrder();

private:
    qint64 timeStamp;
    qint64 buttonClicked1TimeStamp;
    qint64 acceptTimeStamp;
    qint64 rejectTimeStamp;
    qint64 helpRequestedTimeStamp;
};

tst_QDialogButtonBox::tst_QDialogButtonBox()
{
}

tst_QDialogButtonBox::~tst_QDialogButtonBox()
{
}

void tst_QDialogButtonBox::testConstructor1()
{
    QDialogButtonBox buttonbox;
    QCOMPARE(buttonbox.orientation(), Qt::Horizontal);

    QCOMPARE(buttonbox.buttons().count(), 0);
}

void tst_QDialogButtonBox::testConstrurtor2_data()
{
    QTest::addColumn<int>("orientation");

    QTest::newRow("horizontal") << int(Qt::Horizontal);
    QTest::newRow("vertical") << int(Qt::Vertical);
}

void tst_QDialogButtonBox::testConstrurtor2()
{
    QFETCH(int, orientation);
    Qt::Orientation orient = Qt::Orientation(orientation);
    QDialogButtonBox buttonBox(orient);

    QCOMPARE(buttonBox.orientation(), orient);
    QCOMPARE(buttonBox.buttons().count(), 0);
}

void tst_QDialogButtonBox::testConstructor3_data()
{
    QTest::addColumn<int>("orientation");
    QTest::addColumn<QDialogButtonBox::StandardButtons>("buttons");
    QTest::addColumn<int>("buttonCount");

    QTest::newRow("nothing") << int(Qt::Horizontal) << (QDialogButtonBox::StandardButtons)0 << 0;
    QTest::newRow("only 1") << int(Qt::Horizontal) << QDialogButtonBox::StandardButtons(QDialogButtonBox::Ok) << 1;
    QTest::newRow("only 1.. twice") << int(Qt::Horizontal)
                        << (QDialogButtonBox::Ok | QDialogButtonBox::Ok)
                        << 1;
    QTest::newRow("only 2") << int(Qt::Horizontal)
            << (QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
            << 2;
    QTest::newRow("two different things") << int(Qt::Horizontal)
            << (QDialogButtonBox::Save | QDialogButtonBox::Close)
            << 2;
    QTest::newRow("three") << int(Qt::Horizontal)
            << (QDialogButtonBox::Ok
                    | QDialogButtonBox::Cancel
                    | QDialogButtonBox::Help)
            << 3;
    QTest::newRow("everything") << int(Qt::Vertical)
            << (QDialogButtonBox::StandardButtons)UINT_MAX
            << 17;
}

void tst_QDialogButtonBox::testConstructor3()
{
    QFETCH(int, orientation);
    QFETCH(QDialogButtonBox::StandardButtons, buttons);

    QDialogButtonBox buttonBox(buttons, (Qt::Orientation)orientation);
    QCOMPARE(int(buttonBox.orientation()), orientation);
    QTEST(buttonBox.buttons().count(), "buttonCount");
}

void tst_QDialogButtonBox::setOrientation_data()
{
    QTest::addColumn<int>("orientation");

    QTest::newRow("Horizontal") << int(Qt::Horizontal);
    QTest::newRow("Vertical") << int(Qt::Vertical);
}

void tst_QDialogButtonBox::setOrientation()
{
    QFETCH(int, orientation);
    QDialogButtonBox buttonBox;
    QCOMPARE(int(buttonBox.orientation()), int(Qt::Horizontal));

    buttonBox.setOrientation(Qt::Orientation(orientation));
    QCOMPARE(int(buttonBox.orientation()), orientation);
}

/*
void tst_QDialogButtonBox::setLayoutPolicy_data()
{
    QTest::addColumn<int>("layoutPolicy");

    QTest::newRow("win") << int(QDialogButtonBox::WinLayout);
    QTest::newRow("mac") << int(QDialogButtonBox::MacLayout);
    QTest::newRow("kde") << int(QDialogButtonBox::KdeLayout);
    QTest::newRow("gnome") << int(QDialogButtonBox::GnomeLayout);

}

void tst_QDialogButtonBox::setLayoutPolicy()
{
    QFETCH(int, layoutPolicy);

    QDialogButtonBox buttonBox;
    QCOMPARE(int(buttonBox.layoutPolicy()),
             int(buttonBox.style()->styleHint(QStyle::SH_DialogButtonLayout)));
    buttonBox.setLayoutPolicy(QDialogButtonBox::ButtonLayout(layoutPolicy));
    QCOMPARE(int(buttonBox.layoutPolicy()), layoutPolicy);
}
*/

void tst_QDialogButtonBox::addButton1_data()
{
    QTest::addColumn<QDialogButtonBox::ButtonRole>("role");
    QTest::addColumn<int>("totalCount");

    QTest::newRow("InvalidRole") << QDialogButtonBox::InvalidRole << 0;
    QTest::newRow("AcceptRole") << QDialogButtonBox::AcceptRole << 1;
    QTest::newRow("RejectRole") << QDialogButtonBox::RejectRole << 1;
    QTest::newRow("DestructiveRole") << QDialogButtonBox::DestructiveRole << 1;
    QTest::newRow("ActionRole") << QDialogButtonBox::ActionRole << 1;
    QTest::newRow("HelpRole") << QDialogButtonBox::HelpRole << 1;
    QTest::newRow("WackyValue") << (QDialogButtonBox::ButtonRole)-1 << 0;
}

void tst_QDialogButtonBox::addButton1()
{
    QFETCH(QDialogButtonBox::ButtonRole, role);
    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);
    QPushButton *button = new QPushButton();
    buttonBox.addButton(button, role);
    QTEST(buttonBox.buttons().count(), "totalCount");
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QTEST(children.count(), "totalCount");
}

void tst_QDialogButtonBox::addButton2_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QDialogButtonBox::ButtonRole>("role");
    QTest::addColumn<int>("totalCount");
    QTest::newRow("InvalidRole") << QString("foo") << QDialogButtonBox::InvalidRole << 0;
    QTest::newRow("AcceptRole") << QString("foo") << QDialogButtonBox::AcceptRole << 1;
    QTest::newRow("RejectRole") << QString("foo") << QDialogButtonBox::RejectRole << 1;
    QTest::newRow("DestructiveRole") << QString("foo") << QDialogButtonBox::DestructiveRole << 1;
    QTest::newRow("ActionRole") << QString("foo") << QDialogButtonBox::ActionRole << 1;
    QTest::newRow("HelpRole") << QString("foo") << QDialogButtonBox::HelpRole << 1;
    QTest::newRow("WackyValue") << QString("foo") << (QDialogButtonBox::ButtonRole)-1 << 0;
}

void tst_QDialogButtonBox::addButton2()
{
    QFETCH(QString, text);
    QFETCH(QDialogButtonBox::ButtonRole, role);
    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);
    buttonBox.addButton(text, role);
    QTEST(buttonBox.buttons().count(), "totalCount");
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QTEST(children.count(), "totalCount");
}

void tst_QDialogButtonBox::addButton3_data()
{
    QTest::addColumn<QDialogButtonBox::StandardButton>("button");
    QTest::addColumn<int>("totalCount");
    QTest::newRow("Ok") << QDialogButtonBox::Ok << 1;
    QTest::newRow("Open") << QDialogButtonBox::Open << 1;
    QTest::newRow("Save") << QDialogButtonBox::Save << 1;
    QTest::newRow("Cancel") << QDialogButtonBox::Cancel << 1;
    QTest::newRow("Close") << QDialogButtonBox::Close << 1;
    QTest::newRow("Discard") << QDialogButtonBox::Discard << 1;
    QTest::newRow("Apply") << QDialogButtonBox::Apply << 1;
    QTest::newRow("Reset") << QDialogButtonBox::Reset << 1;
    QTest::newRow("Help") << QDialogButtonBox::Help << 1;
    QTest::newRow("noButton") << (QDialogButtonBox::StandardButton)0 << 0;
}

void tst_QDialogButtonBox::addButton3()
{
    QFETCH(QDialogButtonBox::StandardButton, button);
    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);
    buttonBox.addButton(button);
    QTEST(buttonBox.buttons().count(), "totalCount");
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QTEST(children.count(), "totalCount");
}

void tst_QDialogButtonBox::clear_data()
{
    QTest::addColumn<int>("rolesToAdd");

    QTest::newRow("nothing") << 0;
    QTest::newRow("one") << 1;
    QTest::newRow("all") << int(QDialogButtonBox::NRoles);
}

void tst_QDialogButtonBox::clear()
{
    QFETCH(int, rolesToAdd);

    QDialogButtonBox buttonBox;
    for (int i = 1; i < rolesToAdd; ++i)
        buttonBox.addButton("Happy", QDialogButtonBox::ButtonRole(i));
    buttonBox.clear();
    QCOMPARE(buttonBox.buttons().count(), 0);
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QCOMPARE(children.count(), 0);
}

void tst_QDialogButtonBox::removeButton_data()
{
    QTest::addColumn<QDialogButtonBox::ButtonRole>("roleToAdd");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("no button added") << QDialogButtonBox::InvalidRole << 0;
    QTest::newRow("a button") << QDialogButtonBox::AcceptRole << 1;
}

void tst_QDialogButtonBox::removeButton()
{
    QFETCH(QDialogButtonBox::ButtonRole, roleToAdd);

    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);
    QPushButton *button = new QPushButton("RemoveButton test");
    buttonBox.addButton(button, roleToAdd);
    QTEST(buttonBox.buttons().count(), "expectedCount");

    buttonBox.removeButton(button);
    QCOMPARE(buttonBox.buttons().count(), 0);
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QTEST(children.count(), "expectedCount");
}

void tst_QDialogButtonBox::testDelete()
{
    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);

    QPushButton *deleteMe = new QPushButton("Happy");
    buttonBox.addButton(deleteMe, QDialogButtonBox::HelpRole);
    QCOMPARE(buttonBox.buttons().count(), 1);
    QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(&buttonBox);
    QCOMPARE(children.count(), 1);

    delete deleteMe;
    children = qFindChildren<QAbstractButton *>(&buttonBox);
    QCOMPARE(children.count(), 0);
    QCOMPARE(buttonBox.buttons().count(), 0);
}

void tst_QDialogButtonBox::testMultipleAdd()
{
    // Add a button into the thing multiple times.
    QDialogButtonBox buttonBox;
    QCOMPARE(buttonBox.buttons().count(), 0);

    QPushButton *button = new QPushButton("Foo away");
    buttonBox.addButton(button, QDialogButtonBox::AcceptRole);
    QCOMPARE(buttonBox.buttons().count(), 1);
    QCOMPARE(buttonBox.buttonRole(button), QDialogButtonBox::AcceptRole);
    buttonBox.addButton(button, QDialogButtonBox::AcceptRole);
    QCOMPARE(buttonBox.buttons().count(), 1);
    QCOMPARE(buttonBox.buttonRole(button), QDialogButtonBox::AcceptRole);

    // Add it again with a different role
    buttonBox.addButton(button, QDialogButtonBox::RejectRole);
    QCOMPARE(buttonBox.buttons().count(), 1);
    QCOMPARE(buttonBox.buttonRole(button), QDialogButtonBox::RejectRole);

    // Add it as an "invalid" role
    buttonBox.addButton(button, QDialogButtonBox::InvalidRole);
    QCOMPARE(buttonBox.buttons().count(), 1);
    QCOMPARE(buttonBox.buttonRole(button), QDialogButtonBox::RejectRole);
}

void tst_QDialogButtonBox::buttonRole_data()
{
    QTest::addColumn<QDialogButtonBox::ButtonRole>("roleToAdd");
    QTest::addColumn<QDialogButtonBox::ButtonRole>("expectedRole");

    QTest::newRow("AcceptRole stuff") << QDialogButtonBox::AcceptRole
                                      << QDialogButtonBox::AcceptRole;
    QTest::newRow("Nothing") << QDialogButtonBox::InvalidRole << QDialogButtonBox::InvalidRole;
    QTest::newRow("bad stuff") << (QDialogButtonBox::ButtonRole)-1 << QDialogButtonBox::InvalidRole;
}

void tst_QDialogButtonBox::buttonRole()
{
    QFETCH(QDialogButtonBox::ButtonRole, roleToAdd);
    QDialogButtonBox buttonBox;
    QAbstractButton *button = buttonBox.addButton("Here's a button", roleToAdd);
    QTEST(buttonBox.buttonRole(button), "expectedRole");
}

void tst_QDialogButtonBox::testStandardButtonMapping_data()
{
    QTest::addColumn<QDialogButtonBox::StandardButton>("button");
    QTest::addColumn<QDialogButtonBox::ButtonRole>("expectedRole");
    QTest::addColumn<QString>("expectedText");

    int layoutPolicy = qApp->style()->styleHint(QStyle::SH_DialogButtonLayout);

    QTest::newRow("QDialogButtonBox::Ok") << QDialogButtonBox::Ok
                                          << QDialogButtonBox::AcceptRole
                                          << QDialogButtonBox::tr("OK");
    QTest::newRow("QDialogButtonBox::Open") << QDialogButtonBox::Open
                                            << QDialogButtonBox::AcceptRole
                                            << QDialogButtonBox::tr("Open");
    QTest::newRow("QDialogButtonBox::Save") << QDialogButtonBox::Save
                                            << QDialogButtonBox::AcceptRole
                                            << QDialogButtonBox::tr("Save");
    QTest::newRow("QDialogButtonBox::Cancel") << QDialogButtonBox::Cancel
                                              << QDialogButtonBox::RejectRole
                                              << QDialogButtonBox::tr("Cancel");
    QTest::newRow("QDialogButtonBox::Close") << QDialogButtonBox::Close
                                             << QDialogButtonBox::RejectRole
                                             << QDialogButtonBox::tr("Close");
    if (layoutPolicy == QDialogButtonBox::MacLayout) {
        QTest::newRow("QDialogButtonBox::Discard") << QDialogButtonBox::Discard
                                                    << QDialogButtonBox::DestructiveRole
                                                    << QDialogButtonBox::tr("Don't Save");
    } else {
        QTest::newRow("QDialogButtonBox::Discard") << QDialogButtonBox::Discard
                                                    << QDialogButtonBox::DestructiveRole
                                                    << QDialogButtonBox::tr("Discard");
    }
    QTest::newRow("QDialogButtonBox::Apply") << QDialogButtonBox::Apply
                                             << QDialogButtonBox::ActionRole
                                             << QDialogButtonBox::tr("Apply");
    QTest::newRow("QDialogButtonBox::Reset") << QDialogButtonBox::Reset
                                             << QDialogButtonBox::ActionRole
                                             << QDialogButtonBox::tr("Reset");
    QTest::newRow("QDialogButtonBox::Help") << QDialogButtonBox::Help
                                            << QDialogButtonBox::HelpRole
                                            << QDialogButtonBox::tr("Help");
}

void tst_QDialogButtonBox::testStandardButtonMapping()
{
    QFETCH(QDialogButtonBox::StandardButton, button);
    QDialogButtonBox buttonBox;

    QAbstractButton *theButton = buttonBox.addButton(button);
    QTEST(buttonBox.buttonRole(theButton), "expectedRole");
    QTEST(theButton->text(), "expectedText");
}

void tst_QDialogButtonBox::testSignals_data()
{
    QTest::addColumn<QDialogButtonBox::ButtonRole>("buttonToClick");
    QTest::addColumn<int>("clicked2Count");
    QTest::addColumn<int>("acceptCount");
    QTest::addColumn<int>("rejectCount");
    QTest::addColumn<int>("helpRequestedCount");

    QTest::newRow("nothing") << QDialogButtonBox::InvalidRole << 0 << 0 << 0 << 0;
    QTest::newRow("accept") << QDialogButtonBox::AcceptRole << 1 << 1 << 0 << 0;
    QTest::newRow("reject") << QDialogButtonBox::RejectRole << 1 << 0 << 1 << 0;
    QTest::newRow("destructive") << QDialogButtonBox::DestructiveRole << 1 << 0 << 0 << 0;
    QTest::newRow("Action") << QDialogButtonBox::ActionRole << 1 << 0 << 0 << 0;
    QTest::newRow("Help") << QDialogButtonBox::HelpRole << 1 << 0 << 0 << 1;
}

void tst_QDialogButtonBox::testSignals()
{
    QFETCH(QDialogButtonBox::ButtonRole, buttonToClick);
    QDialogButtonBox buttonBox;
    qRegisterMetaType<QAbstractButton *>("QAbstractButton*");
    QSignalSpy clicked2(&buttonBox, SIGNAL(clicked(QAbstractButton*)));
    QSignalSpy accept(&buttonBox, SIGNAL(accepted()));
    QSignalSpy reject(&buttonBox, SIGNAL(rejected()));
    QSignalSpy helpRequested(&buttonBox, SIGNAL(helpRequested()));

    QPushButton *clickMe = 0;
    for (int i = QDialogButtonBox::AcceptRole; i < QDialogButtonBox::NRoles; ++i) {
        QPushButton *button = buttonBox.addButton(QString::number(i),
                                                  QDialogButtonBox::ButtonRole(i));

        if (i == buttonToClick)
            clickMe = button;
    }
    if (clickMe) {
        clickMe->animateClick(0);
        QTest::qWait(100);
    }

    QTEST(clicked2.count(), "clicked2Count");
    if (clicked2.count() > 0)
        QCOMPARE(qvariant_cast<QAbstractButton *>(clicked2.at(0).at(0)), clickMe);

    QTEST(accept.count(), "acceptCount");
    QTEST(reject.count(), "rejectCount");
    QTEST(helpRequested.count(), "helpRequestedCount");
}

void tst_QDialogButtonBox::testSignalOrder()
{
    buttonClicked1TimeStamp = acceptTimeStamp
            = rejectTimeStamp = helpRequestedTimeStamp = timeStamp = 0;
    QDialogButtonBox buttonBox;
    connect(&buttonBox, SIGNAL(clicked(QAbstractButton *)),
            this, SLOT(buttonClicked1(QAbstractButton *)));
    connect(&buttonBox, SIGNAL(accepted()), this, SLOT(acceptClicked()));
    connect(&buttonBox, SIGNAL(rejected()), this, SLOT(rejectClicked()));
    connect(&buttonBox, SIGNAL(helpRequested()), this, SLOT(helpRequestedClicked()));

    QPushButton *acceptButton = buttonBox.addButton("OK", QDialogButtonBox::AcceptRole);
    QPushButton *rejectButton = buttonBox.addButton("Cancel", QDialogButtonBox::RejectRole);
    QPushButton *actionButton = buttonBox.addButton("Action This", QDialogButtonBox::ActionRole);
    QPushButton *helpButton = buttonBox.addButton("Help Me!", QDialogButtonBox::HelpRole);

    // Try accept
    acceptButton->animateClick(0);
    QTest::qWait(100);
    QCOMPARE(rejectTimeStamp, 0LL);
    QCOMPARE(helpRequestedTimeStamp, 0LL);

    QVERIFY(buttonClicked1TimeStamp < acceptTimeStamp);
    acceptTimeStamp = 0;

    rejectButton->animateClick(0);
    QTest::qWait(100);
    QCOMPARE(acceptTimeStamp, 0LL);
    QCOMPARE(helpRequestedTimeStamp, 0LL);
    QVERIFY(buttonClicked1TimeStamp < rejectTimeStamp);

    rejectTimeStamp = 0;
    actionButton->animateClick(0);
    QTest::qWait(100);
    QCOMPARE(acceptTimeStamp, 0LL);
    QCOMPARE(rejectTimeStamp, 0LL);
    QCOMPARE(helpRequestedTimeStamp, 0LL);

    helpButton->animateClick(0);
    QTest::qWait(100);
    QCOMPARE(acceptTimeStamp, 0LL);
    QCOMPARE(rejectTimeStamp, 0LL);
    QVERIFY(buttonClicked1TimeStamp < helpRequestedTimeStamp);
}

void tst_QDialogButtonBox::buttonClicked1(QAbstractButton *)
{
    buttonClicked1TimeStamp = ++timeStamp;
}

void tst_QDialogButtonBox::acceptClicked()
{
    acceptTimeStamp = ++timeStamp;
}

void tst_QDialogButtonBox::rejectClicked()
{
    rejectTimeStamp = ++timeStamp;
}

void tst_QDialogButtonBox::helpRequestedClicked()
{
    helpRequestedTimeStamp = ++timeStamp;
}

void tst_QDialogButtonBox::setStandardButtons_data()
{
    QTest::addColumn<QDialogButtonBox::StandardButtons>("buttonsToAdd");
    QTest::addColumn<QDialogButtonBox::StandardButtons>("expectedResult");

    QTest::newRow("Nothing") << QDialogButtonBox::StandardButtons(QDialogButtonBox::NoButton)
                             << QDialogButtonBox::StandardButtons(QDialogButtonBox::NoButton);
    QTest::newRow("Everything") << (QDialogButtonBox::StandardButtons)0xffffffff
                                << (QDialogButtonBox::Ok
                                   | QDialogButtonBox::Open
                                   | QDialogButtonBox::Save
                                   | QDialogButtonBox::Cancel
                                   | QDialogButtonBox::Close
                                   | QDialogButtonBox::Discard
                                   | QDialogButtonBox::Apply
                                   | QDialogButtonBox::Reset
                                   | QDialogButtonBox::Help
                                   | QDialogButtonBox::Yes
                                   | QDialogButtonBox::YesToAll
                                   | QDialogButtonBox::No
                                   | QDialogButtonBox::NoToAll
                                   | QDialogButtonBox::SaveAll
                                   | QDialogButtonBox::Abort
                                   | QDialogButtonBox::Retry
                                   | QDialogButtonBox::Ignore
                                   );
    QTest::newRow("Simple thing") << QDialogButtonBox::StandardButtons(QDialogButtonBox::Help)
                                  << QDialogButtonBox::StandardButtons(QDialogButtonBox::Help);
    QTest::newRow("Standard thing") << (QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
                                    << (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
}

void tst_QDialogButtonBox::setStandardButtons()
{
    QFETCH(QDialogButtonBox::StandardButtons, buttonsToAdd);
    QDialogButtonBox buttonBox;
    buttonBox.setStandardButtons(buttonsToAdd);
    QTEST(buttonBox.standardButtons(), "expectedResult");
}

void tst_QDialogButtonBox::standardButtons()
{
    // Test various cases of setting StandardButtons
    QDialogButtonBox buttonBox;

    QCOMPARE(buttonBox.standardButtons(),
             QDialogButtonBox::StandardButtons(QDialogButtonBox::NoButton));

    // Set some buttons
    buttonBox.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QCOMPARE(buttonBox.standardButtons(), QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Now add a button
    buttonBox.addButton(QDialogButtonBox::Apply);
    QCOMPARE(buttonBox.standardButtons(),
             QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);

    // Set the standard buttons to other things
    buttonBox.setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel
                                 | QDialogButtonBox::Discard);
    QCOMPARE(buttonBox.standardButtons(), QDialogButtonBox::Save | QDialogButtonBox::Cancel
                                          | QDialogButtonBox::Discard);
    QCOMPARE(buttonBox.buttons().size(), 3);

    // Add another button (not a standard one)
    buttonBox.addButton(QLatin1String("Help"), QDialogButtonBox::HelpRole);
    QCOMPARE(buttonBox.standardButtons(), QDialogButtonBox::Save | QDialogButtonBox::Cancel
                                          | QDialogButtonBox::Discard);
    QCOMPARE(buttonBox.buttons().size(), 4);

    // Finally, check if we construct with standard buttons that they show up.
    QDialogButtonBox buttonBox2(QDialogButtonBox::Open | QDialogButtonBox::Reset);
    QCOMPARE(buttonBox2.standardButtons(), QDialogButtonBox::Open | QDialogButtonBox::Reset);
}

void tst_QDialogButtonBox::testRemove()
{
    // Make sure that removing a button and clicking it, doesn't trigger any latent signals
    QDialogButtonBox buttonBox;
    QSignalSpy clicked(&buttonBox, SIGNAL(clicked(QAbstractButton*)));

    QAbstractButton *button = buttonBox.addButton(QDialogButtonBox::Ok);

    buttonBox.removeButton(button);

    button->animateClick(0);
    QTest::qWait(100);
    QCOMPARE(clicked.count(), 0);
}

QTEST_MAIN(tst_QDialogButtonBox)
#include "tst_qdialogbuttonbox.moc"
