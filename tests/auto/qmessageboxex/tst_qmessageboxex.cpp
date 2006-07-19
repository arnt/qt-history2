#include <QtTest/QtTest>
#include <QMessageBoxEx>
#include <QDebug>
#include <QPair>
#include <QList>
#include <QPointer>
#include <QTimer>
#include <QApplication>
#include <QPushButton>

class tst_QMessageBoxEx : public QObject
{
    Q_OBJECT
public:
    tst_QMessageBoxEx();
    int exec(QMessageBoxEx *msgBox, int key = -1);

public slots:
    void sendKey();

private slots:
    void sanityTest();
    void defaultButton();
    void escapeButton();
    void button();
    void statics();
    void about();

    void staticSourceCompat();
    void staticBinaryCompat();

private:
    int keyToSend;
};

tst_QMessageBoxEx::tst_QMessageBoxEx() : keyToSend(-1)
{
}

int tst_QMessageBoxEx::exec(QMessageBoxEx *msgBox, int key)
{
    if (key == -1) {
        QTimer::singleShot(1000, msgBox, SLOT(close()));
    } else {
        keyToSend = key;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
    }
    return msgBox->exec();
}

void tst_QMessageBoxEx::sendKey()
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

void tst_QMessageBoxEx::sanityTest()
{
    QMessageBoxEx msgBox;
    msgBox.setText("This is insane");
    for (int i = 0; i < 10; i++)
        msgBox.setIcon(QMessageBoxEx::Icon(i));
    msgBox.setIconPixmap(QPixmap());
    msgBox.setIconPixmap(QPixmap("whatever.png"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextFormat(Qt::PlainText);
    exec(&msgBox);
}

void tst_QMessageBoxEx::button()
{
    QMessageBoxEx msgBox;
    msgBox.addButton("retry", QMessageBoxEx::DestructiveRole);
    QVERIFY(msgBox.button(QMessageBoxEx::Ok) == 0); // not added yet
    QPushButton *b1 = msgBox.addButton(QMessageBoxEx::Ok);
    QCOMPARE(msgBox.button(QMessageBoxEx::Ok), b1);  // just added
    QCOMPARE(msgBox.standardButton(b1), QMessageBoxEx::Ok);
    msgBox.addButton(QMessageBoxEx::Cancel);
    QCOMPARE(msgBox.standardButtons(), QMessageBoxEx::Ok | QMessageBoxEx::Cancel);
    
    // remove the cancel, should not exist anymore
    msgBox.setStandardButtons(QMessageBoxEx::Yes | QMessageBoxEx::No);
    QVERIFY(msgBox.button(QMessageBoxEx::Cancel) == 0); 
    QVERIFY(msgBox.button(QMessageBoxEx::Yes) != 0);

    // should not crash
    QPushButton *b4 = new QPushButton;
    msgBox.addButton(b4, QMessageBoxEx::DestructiveRole);
    msgBox.addButton(0, QMessageBoxEx::ActionRole);
}

void tst_QMessageBoxEx::defaultButton()
{
    QMessageBoxEx msgBox;
    QVERIFY(msgBox.defaultButton() == 0);
    msgBox.addButton(QMessageBoxEx::Ok);
    msgBox.addButton(QMessageBoxEx::Cancel);
    QVERIFY(msgBox.defaultButton() == 0);
    QPushButton pushButton;
    msgBox.setDefaultButton(&pushButton);
    QVERIFY(msgBox.defaultButton() == 0); // we have not added it yet
    QPushButton *retryButton = msgBox.addButton(QMessageBoxEx::Retry);
    msgBox.setDefaultButton(retryButton);
    QCOMPARE(msgBox.defaultButton(), retryButton);
    exec(&msgBox);
    QCOMPARE(msgBox.clickedButton(), msgBox.escapeButton());

    exec(&msgBox, Qt::Key_Enter);
    QCOMPARE(msgBox.clickedButton(), retryButton);
}

void tst_QMessageBoxEx::escapeButton()
{
    QMessageBoxEx msgBox;
    QVERIFY(msgBox.escapeButton() == 0);
    msgBox.addButton(QMessageBoxEx::Ok);
    msgBox.addButton(QMessageBoxEx::Cancel);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton invalidButton;
    msgBox.setEscapeButton(&invalidButton);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton *retryButton = msgBox.addButton(QMessageBoxEx::Retry);

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

void tst_QMessageBoxEx::statics()
{
    QMessageBoxEx::StandardButton (*statics[4])(QWidget *, const QString &,
         const QString&, QMessageBoxEx::StandardButtons buttons, 
         QMessageBoxEx::StandardButton);

    statics[0] = QMessageBoxEx::information;
    statics[1] = QMessageBoxEx::critical;
    statics[2] = QMessageBoxEx::question;
    statics[3] = QMessageBoxEx::warning;

    for (int i = 0; i < 4; i++) {
        keyToSend = Qt::Key_Escape;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        QMessageBoxEx::StandardButton sb = (*statics[i])(0, "caption", 
           "text", QMessageBoxEx::Yes | QMessageBoxEx::No | QMessageBoxEx::Help,
           QMessageBoxEx::NoButton);
        QCOMPARE(sb, QMessageBoxEx::Cancel);

        keyToSend = -2; // close()
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBoxEx::Yes | QMessageBoxEx::No | QMessageBoxEx::Help,
           QMessageBoxEx::NoButton);
        QCOMPARE(sb, QMessageBoxEx::Cancel);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBoxEx::Yes | QMessageBoxEx::No | QMessageBoxEx::Help,
           QMessageBoxEx::NoButton);
        QCOMPARE(sb, QMessageBoxEx::Yes);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption", 
           "text", QMessageBoxEx::Yes | QMessageBoxEx::No | QMessageBoxEx::Help,
            QMessageBoxEx::No);
        QCOMPARE(sb, QMessageBoxEx::No);
    }
}

void tst_QMessageBoxEx::about()
{
    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBoxEx::about(0, "Caption", "This is an auto test");

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBoxEx::aboutQt(0, "Caption");
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

void tst_QMessageBoxEx::staticSourceCompat()
{
    int ret;

    // source compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", QMessageBoxEx::Yes, QMessageBoxEx::No);
    QCOMPARE(ret, int(QMessageBoxEx::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", QMessageBoxEx::Yes | QMessageBoxEx::Default, QMessageBoxEx::No);
    QCOMPARE(ret, int(QMessageBoxEx::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", QMessageBoxEx::Yes, QMessageBoxEx::No | QMessageBoxEx::Default);
    QCOMPARE(ret, int(QMessageBoxEx::No));
    
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", QMessageBoxEx::Yes | QMessageBoxEx::Default, QMessageBoxEx::No | QMessageBoxEx::Escape);
    QCOMPARE(ret, int(QMessageBoxEx::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", QMessageBoxEx::Yes | QMessageBoxEx::Escape, QMessageBoxEx::No | QMessageBoxEx::Default);
    QCOMPARE(ret, int(QMessageBoxEx::No));
}

void tst_QMessageBoxEx::staticBinaryCompat()
{
    int ret;

    // binary compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes, Old_No, 0);
    QCOMPARE(ret, -1);

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes | Old_Default, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes, Old_No | Old_Default, 0);
    QCOMPARE(ret, -1);

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes | Old_Escape, Old_No | Old_Default, 0);
    QCOMPARE(ret, Old_Yes);

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBoxEx::information(0, "title", "text", Old_Yes | Old_Default, Old_No | Old_Escape, 0);
    QCOMPARE(ret, Old_No);
}

QTEST_MAIN(tst_QMessageBoxEx)
#include "tst_qmessageboxex.moc"
