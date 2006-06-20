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
    void addButton();
    void defaultButton();
    void escapeButton();
    void button();
    void statics();
    void about();

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

void tst_QMessageBoxEx::addButton()
{
    QMessageBoxEx msgBox;
    int i;
    // check our guarantee that buttons indices are 0, 1, 2...
    i = msgBox.addButton("retry", QMessageBoxEx::DestructiveRole);
    QCOMPARE(i, 0);
    i = msgBox.addButton(QMessageBoxEx::Ok);
    QCOMPARE(i, 1);
    i = msgBox.addButton(QMessageBoxEx::Cancel);
    QCOMPARE(i, 2);
    i = msgBox.addButton("cancel", QMessageBoxEx::DestructiveRole);
    QCOMPARE(i, 3);
}

void tst_QMessageBoxEx::defaultButton()
{
    QMessageBoxEx msgBox;
    QCOMPARE(msgBox.defaultButton(), -1);
    msgBox.addButton(QMessageBoxEx::Ok);
    msgBox.addButton(QMessageBoxEx::Cancel);
    QCOMPARE(msgBox.defaultButton(), -1);
    msgBox.setDefaultButton(2);
    QCOMPARE(msgBox.defaultButton(), -1);
    msgBox.addButton(QMessageBoxEx::Retry);
    msgBox.setDefaultButton(2);
    QCOMPARE(msgBox.defaultButton(), 2);
    QCOMPARE(exec(&msgBox), msgBox.escapeButton());

    QCOMPARE(exec(&msgBox, Qt::Key_Enter), 2);
}

void tst_QMessageBoxEx::escapeButton()
{
    QMessageBoxEx msgBox;
    QCOMPARE(msgBox.escapeButton(), -1);
    msgBox.addButton(QMessageBoxEx::Ok);
    msgBox.addButton(QMessageBoxEx::Cancel);
    QCOMPARE(msgBox.escapeButton(), -1);
    msgBox.setEscapeButton(2);
    QCOMPARE(msgBox.escapeButton(), -1);
    msgBox.addButton(QMessageBoxEx::Retry);

    QCOMPARE(exec(&msgBox), -1);

    msgBox.setEscapeButton(2);
    QCOMPARE(msgBox.escapeButton(), 2);
    QCOMPARE(exec(&msgBox, Qt::Key_Escape), 2);
    QCOMPARE(exec(&msgBox), 2);
}

void tst_QMessageBoxEx::button()
{
    QMessageBoxEx msgBox;
    msgBox.addButton("cancel", QMessageBoxEx::DestructiveRole);
    msgBox.addButton("ok", QMessageBoxEx::AcceptRole);
    QPushButton *pb = msgBox.button(-1);
    QVERIFY(pb == 0);
    pb = msgBox.button(2);
    QVERIFY(pb == 0);
    pb = msgBox.button(1);
    QCOMPARE(pb->text(), QLatin1String("ok"));
    pb = msgBox.button(0);
    QCOMPARE(pb->text(), QLatin1String("cancel"));
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

QTEST_MAIN(tst_QMessageBoxEx)
#include "tst_qmessageboxex.moc"
