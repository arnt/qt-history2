#include <QtTest/QtTest>
#include <QMessageBox>
#include <QDebug>
#include <QPair>
#include <QList>
#include <QPointer>
#include <QTimer>
#include <QApplication>
#include <QPushButton>

class tst_QMessageBox : public QObject
{
    Q_OBJECT
public:
    tst_QMessageBox();
    int exec(QMessageBox *msgBox, int key = -1);

public slots:
    void sendKey();

private slots:
    void sanityTest();
    void defaultButton();
    void escapeButton();
    void button();
    void statics();
    void about();

    void shortcut();

    void staticSourceCompat();
    void staticBinaryCompat();

private:
    int keyToSend;
};

tst_QMessageBox::tst_QMessageBox() : keyToSend(-1)
{
}

int tst_QMessageBox::exec(QMessageBox *msgBox, int key)
{
    if (key == -1) {
        QTimer::singleShot(1000, msgBox, SLOT(close()));
    } else {
        keyToSend = key;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
    }
    return msgBox->exec();
}

void tst_QMessageBox::sendKey()
{
    if (keyToSend == -2) {
        QApplication::activeModalWidget()->close();
        return;
    }
    if (keyToSend == -1)
        return;
    QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, keyToSend, Qt::NoModifier);
    qApp->postEvent(QApplication::activeModalWidget(), ke);
    keyToSend = -1;
}

void tst_QMessageBox::sanityTest()
{
    QMessageBox msgBox;
    msgBox.setText("This is insane");
    for (int i = 0; i < 10; i++)
        msgBox.setIcon(QMessageBox::Icon(i));
    msgBox.setIconPixmap(QPixmap());
    msgBox.setIconPixmap(QPixmap("whatever.png"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextFormat(Qt::PlainText);
    exec(&msgBox);
}

void tst_QMessageBox::button()
{
    QMessageBox msgBox;
    msgBox.addButton("retry", QMessageBox::DestructiveRole);
    QVERIFY(msgBox.button(QMessageBox::Ok) == 0); // not added yet
    QPushButton *b1 = msgBox.addButton(QMessageBox::Ok);
    QCOMPARE(msgBox.button(QMessageBox::Ok), b1);  // just added
    QCOMPARE(msgBox.standardButton(b1), QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    QCOMPARE(msgBox.standardButtons(), QMessageBox::Ok | QMessageBox::Cancel);
    
    // remove the cancel, should not exist anymore
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QVERIFY(msgBox.button(QMessageBox::Cancel) == 0); 
    QVERIFY(msgBox.button(QMessageBox::Yes) != 0);

    // should not crash
    QPushButton *b4 = new QPushButton;
    msgBox.addButton(b4, QMessageBox::DestructiveRole);
    msgBox.addButton(0, QMessageBox::ActionRole);
}

void tst_QMessageBox::defaultButton()
{
    QMessageBox msgBox;
    QVERIFY(msgBox.defaultButton() == 0);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    QVERIFY(msgBox.defaultButton() == 0);
    QPushButton pushButton;
    msgBox.setDefaultButton(&pushButton);
    QVERIFY(msgBox.defaultButton() == 0); // we have not added it yet
    QPushButton *retryButton = msgBox.addButton(QMessageBox::Retry);
    msgBox.setDefaultButton(retryButton);
    QCOMPARE(msgBox.defaultButton(), retryButton);
    exec(&msgBox);
    QCOMPARE(msgBox.clickedButton(), msgBox.escapeButton());

    exec(&msgBox, Qt::Key_Enter);
    QCOMPARE(msgBox.clickedButton(), retryButton);
}

void tst_QMessageBox::escapeButton()
{
    QMessageBox msgBox;
    QVERIFY(msgBox.escapeButton() == 0);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton invalidButton;
    msgBox.setEscapeButton(&invalidButton);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton *retryButton = msgBox.addButton(QMessageBox::Retry);

    exec(&msgBox);
    QVERIFY(msgBox.clickedButton() == 0);

    msgBox.setEscapeButton(retryButton);
    QCOMPARE(msgBox.escapeButton(), retryButton);

    // with escape
    exec(&msgBox, Qt::Key_Escape);
    QCOMPARE(msgBox.clickedButton(), retryButton);

    // with close
    exec(&msgBox);
    QCOMPARE(msgBox.clickedButton(), retryButton);
}

void tst_QMessageBox::statics()
{
    QMessageBox::StandardButton (*statics[4])(QWidget *, const QString &,
         const QString&, QMessageBox::StandardButtons buttons, 
         QMessageBox::StandardButton);

    statics[0] = QMessageBox::information;
    statics[1] = QMessageBox::critical;
    statics[2] = QMessageBox::question;
    statics[3] = QMessageBox::warning;

    for (int i = 0; i < 4; i++) {
        keyToSend = Qt::Key_Escape;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        QMessageBox::StandardButton sb = (*statics[i])(0, "caption", 
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Cancel);

        keyToSend = -2; // close()
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Cancel);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Yes);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
            QMessageBox::No);
        QCOMPARE(sb, QMessageBox::No);
    }
}

void tst_QMessageBox::shortcut()
{
    QMessageBox msgBox;
    msgBox.addButton("O&k", QMessageBox::YesRole);
    msgBox.addButton("&No", QMessageBox::YesRole);
    msgBox.addButton("&Maybe", QMessageBox::YesRole);
    QVERIFY(exec(&msgBox, Qt::Key_M), 2);
}

void tst_QMessageBox::about()
{
    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBox::about(0, "Caption", "This is an auto test");

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBox::aboutQt(0, "Caption");
}

// Old message box enums
const int Old_Ok = 1;
const int Old_Cancel = 2;
const int Old_Yes = 3;
const int Old_No = 4;
const int Old_Abort = 5;
const int Old_Retry = 6;
const int Old_Ignore = 7;
const int Old_YesAll = 8;
const int Old_NoAll = 9;
const int Old_Default = 0x100;
const int Old_Escape = 0x200;

void tst_QMessageBox::staticSourceCompat()
{
    int ret;

    // source compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes, QMessageBox::No);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    QCOMPARE(ret, int(QMessageBox::No));
    
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Escape, QMessageBox::No | QMessageBox::Default);
    QCOMPARE(ret, int(QMessageBox::No));
}

void tst_QMessageBox::staticBinaryCompat()
{
    int ret;

    // binary compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes, Old_No, 0);
    QCOMPARE(ret, -1);

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Default, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes, Old_No | Old_Default, 0);
    QCOMPARE(ret, -1);

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Escape, Old_No | Old_Default, 0);
    QCOMPARE(ret, Old_Yes);

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Default, Old_No | Old_Escape, 0);
    QCOMPARE(ret, Old_No);
}

QTEST_MAIN(tst_QMessageBox)
#include "tst_qmessageboxex.moc"
