/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#if 0
#include <QtTest/QtTest>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QToolButton>
#include "/home/jasmin/dev/solutions/widgets/qtwizard/src/qtwizard.h"

//TESTED_CLASS=QWizard
//TESTED_FILES=gui/dialogs/qwizard.h corelib/tools/qwizard.cpp

#define QWizard QtWizard
#define QWizardPage QtWizardPage

static QImage grabWidget(QWidget *window)
{
    return QPixmap::grabWidget(window).toImage();
}

class tst_QWizard : public QObject
{
    Q_OBJECT

public:
    tst_QWizard();

public slots:
    void init();
    void cleanup();

private slots:
    void buttonText();
    void setButtonLayout();
    void setButton();
    void setTitleFormatEtc();
    void setPixmap();
    void setDefaultProperty();
    void addPage();
    void setPage();
    void setStartId();
    void setOption_IndependentPages();
    void setOption_IgnoreSubTitles();
    void setOption_ExtendedWatermarkPixmap();
    void setOption_NoDefaultButton();
    void setOption_NoBackButtonOnStartPage();
    void setOption_NoBackButtonOnLastPage();
    void setOption_DisabledBackButtonOnLastPage();
    void setOption_HaveNextButtonOnLastPage();
    void setOption_HaveFinishButtonOnEarlyPages();
    void setOption_NoCancelButton();
    void setOption_CancelButtonOnLeft();
    void setOption_HaveHelpButton();
    void setOption_HelpButtonOnRight();
    void setOption_HaveCustomButtonX();

    /*
        Things that could be added:

        1. Test virtual functions that are called, signals that are
           emitted, etc.

        2. Test QWizardPage more thorougly.

        3. Test the look and field a bit more (especially the
           different wizard styles, and how they interact with
           pixmaps, titles, subtitles, etc.).

        4. Test minimum sizes, sizes, maximum sizes, resizing, etc.

        5. Try setting various options and wizard styles in various
           orders and check that the results are the same every time,
           no matter the order in which the properties were set.

        6. Test done() and restart().
    */
};

tst_QWizard::tst_QWizard()
{
}

void tst_QWizard::init()
{
}

void tst_QWizard::cleanup()
{
}

void tst_QWizard::buttonText()
{
    QWizard wizard;
    wizard.setWizardStyle(QWizard::ClassicStyle);

    /*
        Check the buttons' original text in Classic and Modern styles.
    */
    for (int pass = 0; pass < 2; ++pass) {
        QCOMPARE(wizard.buttonText(QWizard::BackButton), QString("< &Back"));
        QVERIFY(wizard.buttonText(QWizard::NextButton).contains("Next"));
        QVERIFY(wizard.buttonText(QWizard::FinishButton).endsWith("Finish"));
        QVERIFY(wizard.buttonText(QWizard::CancelButton).endsWith("Cancel"));
        QVERIFY(wizard.buttonText(QWizard::HelpButton).endsWith("Help"));

        QVERIFY(wizard.buttonText(QWizard::CustomButton1).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::CustomButton2).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::CustomButton3).isEmpty());

        // robustness
        QVERIFY(wizard.buttonText(QWizard::Stretch).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NoButton).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NStandardButtons).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NButtons).isEmpty());

        wizard.setWizardStyle(QWizard::ModernStyle);
    }

    /*
        Check the buttons' original text in Mac style.
    */
    wizard.setWizardStyle(QWizard::MacStyle);

    QCOMPARE(wizard.buttonText(QWizard::BackButton), QString("Go Back"));
    QCOMPARE(wizard.buttonText(QWizard::NextButton), QString("Continue"));
    QCOMPARE(wizard.buttonText(QWizard::FinishButton), QString("Done"));
    QCOMPARE(wizard.buttonText(QWizard::CancelButton), QString("Quit"));
    QCOMPARE(wizard.buttonText(QWizard::HelpButton), QString("Help"));

    QVERIFY(wizard.buttonText(QWizard::CustomButton1).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::CustomButton2).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::CustomButton3).isEmpty());

    // robustness
    QVERIFY(wizard.buttonText(QWizard::Stretch).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NoButton).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NStandardButtons).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NButtons).isEmpty());

    /*
        Modify the buttons' text and see what happens.
    */
    wizard.setButtonText(QWizard::NextButton, "N&este");
    wizard.setButtonText(QWizard::CustomButton2, "&Cucu");
    wizard.setButtonText(QWizard::Stretch, "Stretch");

    QCOMPARE(wizard.buttonText(QWizard::BackButton), QString("Go Back"));
    QCOMPARE(wizard.buttonText(QWizard::NextButton), QString("N&este"));
    QCOMPARE(wizard.buttonText(QWizard::FinishButton), QString("Done"));
    QCOMPARE(wizard.buttonText(QWizard::CancelButton), QString("Quit"));
    QCOMPARE(wizard.buttonText(QWizard::HelpButton), QString("Help"));

    QVERIFY(wizard.buttonText(QWizard::CustomButton1).isEmpty());
    QCOMPARE(wizard.buttonText(QWizard::CustomButton2), QString("&Cucu"));
    QVERIFY(wizard.buttonText(QWizard::CustomButton3).isEmpty());

    // robustness
    QVERIFY(wizard.buttonText(QWizard::Stretch).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NoButton).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NStandardButtons).isEmpty());
    QVERIFY(wizard.buttonText(QWizard::NButtons).isEmpty());

    /*
        Switch back to Classic style and see what happens.
    */
    wizard.setWizardStyle(QWizard::ClassicStyle);

    for (int pass = 0; pass < 2; ++pass) {
        QCOMPARE(wizard.buttonText(QWizard::BackButton), QString("< &Back"));
        QCOMPARE(wizard.buttonText(QWizard::NextButton), QString("N&este"));
        QVERIFY(wizard.buttonText(QWizard::FinishButton).endsWith("Finish"));
        QVERIFY(wizard.buttonText(QWizard::CancelButton).endsWith("Cancel"));
        QVERIFY(wizard.buttonText(QWizard::HelpButton).endsWith("Help"));

        QVERIFY(wizard.buttonText(QWizard::CustomButton1).isEmpty());
        QCOMPARE(wizard.buttonText(QWizard::CustomButton2), QString("&Cucu"));
        QVERIFY(wizard.buttonText(QWizard::CustomButton3).isEmpty());

        // robustness
        QVERIFY(wizard.buttonText(QWizard::Stretch).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NoButton).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NStandardButtons).isEmpty());
        QVERIFY(wizard.buttonText(QWizard::NButtons).isEmpty());

        wizard.setOptions(QWizard::NoDefaultButton
                          | QWizard::NoBackButtonOnStartPage
                          | QWizard::NoBackButtonOnLastPage
                          | QWizard::DisabledBackButtonOnLastPage
                          | QWizard::NoCancelButton
                          | QWizard::CancelButtonOnLeft
                          | QWizard::HaveHelpButton
                          | QWizard::HelpButtonOnRight
                          | QWizard::HaveCustomButton1
                          | QWizard::HaveCustomButton2
                          | QWizard::HaveCustomButton3);
    }
}

void tst_QWizard::setButtonLayout()
{
    QList<QWizard::WizardButton> layout;

    QWizard wizard;
    wizard.setWizardStyle(QWizard::ClassicStyle);
    wizard.setOptions(0);
    wizard.setButtonLayout(layout);
    wizard.show();
    qApp->processEvents();

    // if these crash, this means there's a bug in QWizard
    QVERIFY(wizard.button(QWizard::NextButton)->text().contains("Next"));
    QVERIFY(wizard.button(QWizard::BackButton)->text().contains("Back"));
    QVERIFY(wizard.button(QWizard::FinishButton)->text().contains("Finish"));
    QVERIFY(wizard.button(QWizard::CancelButton)->text().contains("Cancel"));
    QVERIFY(wizard.button(QWizard::HelpButton)->text().contains("Help"));
    QVERIFY(wizard.button(QWizard::CustomButton1)->text().isEmpty());
    QVERIFY(wizard.button(QWizard::CustomButton2)->text().isEmpty());
    QVERIFY(wizard.button(QWizard::CustomButton3)->text().isEmpty());
    QVERIFY(!wizard.button(QWizard::Stretch));
    QVERIFY(!wizard.button(QWizard::NoButton));

    QVERIFY(!wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::HelpButton)->isVisible());

    layout << QWizard::NextButton << QWizard::HelpButton;
    wizard.setButtonLayout(layout);
    qApp->processEvents();

    QVERIFY(!wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());

    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);
    qApp->processEvents();

    QVERIFY(!wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());

    wizard.restart();
    qApp->processEvents();

    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());

    layout.clear();
    layout << QWizard::NextButton << QWizard::HelpButton << QWizard::BackButton
           << QWizard::FinishButton << QWizard::CancelButton << QWizard::Stretch
           << QWizard::CustomButton2;

    /*
        Turn on all the button-related wizard options. Some of these
        should have no impact on a custom layout; others should.
    */
    wizard.setButtonLayout(layout);
    wizard.setOptions(QWizard::NoDefaultButton
                      | QWizard::NoBackButtonOnStartPage
                      | QWizard::NoBackButtonOnLastPage
                      | QWizard::DisabledBackButtonOnLastPage
                      | QWizard::HaveNextButtonOnLastPage
                      | QWizard::HaveFinishButtonOnEarlyPages
                      | QWizard::NoCancelButton
                      | QWizard::CancelButtonOnLeft
                      | QWizard::HaveHelpButton
                      | QWizard::HelpButtonOnRight
                      | QWizard::HaveCustomButton1
                      | QWizard::HaveCustomButton2
                      | QWizard::HaveCustomButton3);
    qApp->processEvents();

    // we're on first page
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());
    QVERIFY(wizard.button(QWizard::CancelButton)->isVisible()); // NoCancelButton overridden
    QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::CustomButton1)->isVisible());
    QVERIFY(wizard.button(QWizard::CustomButton2)->isVisible());    // HaveCustomButton2 overridden
    QVERIFY(!wizard.button(QWizard::CustomButton3)->isVisible());

    wizard.next();
    qApp->processEvents();

    // we're on last page
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::NextButton)->isEnabled());
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());
    QVERIFY(wizard.button(QWizard::FinishButton)->isEnabled());
    QVERIFY(wizard.button(QWizard::CancelButton)->isVisible()); // NoCancelButton overridden
    QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::CustomButton1)->isVisible());
    QVERIFY(wizard.button(QWizard::CustomButton2)->isVisible());    // HaveCustomButton2 overridden
    QVERIFY(!wizard.button(QWizard::CustomButton3)->isVisible());

    /*
        Check that the buttons are in the right order on screen.
    */
    for (int pass = 0; pass < 2; ++pass) {
        wizard.setLayoutDirection(pass == 0 ? Qt::LeftToRight : Qt::RightToLeft);
        qApp->processEvents();

        int sign = (pass == 0) ? +1 : -1;

        int p[5];
        p[0] = sign * wizard.button(QWizard::NextButton)->x();
        p[1] = sign * wizard.button(QWizard::HelpButton)->x();
        p[2] = sign * wizard.button(QWizard::FinishButton)->x();
        p[3] = sign * wizard.button(QWizard::CancelButton)->x();
        p[4] = sign * wizard.button(QWizard::CustomButton2)->x();

        QVERIFY(p[0] < p[1]);
        QVERIFY(p[1] < p[2]);
        QVERIFY(p[2] < p[3]);
        QVERIFY(p[3] < p[4]);
    }

    layout.clear();
    wizard.setButtonLayout(layout);
    qApp->processEvents();

    for (int i = -1; i < 50; ++i) {
        QAbstractButton *button = wizard.button(QWizard::WizardButton(i));
        QVERIFY(!button || !button->isVisible());
    }
}

void tst_QWizard::setButton()
{
    QPointer<QToolButton> toolButton = new QToolButton;

    QWizard wizard;
    wizard.setWizardStyle(QWizard::ClassicStyle);
    wizard.setButton(QWizard::NextButton, toolButton);
    wizard.setButton(QWizard::CustomButton2, new QCheckBox("Kustom 2"));

    QVERIFY(qobject_cast<QToolButton *>(wizard.button(QWizard::NextButton)));
    QVERIFY(qobject_cast<QCheckBox *>(wizard.button(QWizard::CustomButton2)));
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::CustomButton1)));

    QVERIFY(toolButton != 0);

    // resetting the same button does nothing
    wizard.setButton(QWizard::NextButton, toolButton);
    QVERIFY(toolButton != 0);

    // revert to default button
    wizard.setButton(QWizard::NextButton, 0);
    QVERIFY(toolButton == 0);
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton)));
    QVERIFY(wizard.button(QWizard::NextButton)->text().contains("Next"));
}

void tst_QWizard::setTitleFormatEtc()
{
    QWizard wizard;
    QVERIFY(wizard.titleFormat() == Qt::AutoText);
    QVERIFY(wizard.subTitleFormat() == Qt::AutoText);

    wizard.setTitleFormat(Qt::RichText);
    QVERIFY(wizard.titleFormat() == Qt::RichText);
    QVERIFY(wizard.subTitleFormat() == Qt::AutoText);

    wizard.setSubTitleFormat(Qt::PlainText);
    QVERIFY(wizard.titleFormat() == Qt::RichText);
    QVERIFY(wizard.subTitleFormat() == Qt::PlainText);
}

void tst_QWizard::setPixmap()
{
    QPixmap p1(1, 1);
    QPixmap p2(2, 2);
    QPixmap p3(3, 3);
    QPixmap p4(4, 4);
    QPixmap p5(5, 5);

    QWizard wizard;
    QWizardPage *page = new QWizardPage;
    QWizardPage *page2 = new QWizardPage;

    wizard.addPage(page);
    wizard.addPage(page2);

    QVERIFY(wizard.pixmap(QWizard::BannerPixmap).isNull());
    QVERIFY(wizard.pixmap(QWizard::LogoPixmap).isNull());
    QVERIFY(wizard.pixmap(QWizard::WatermarkPixmap).isNull());
    QVERIFY(wizard.pixmap(QWizard::BackgroundPixmap).isNull());

    QVERIFY(page->pixmap(QWizard::BannerPixmap).isNull());
    QVERIFY(page->pixmap(QWizard::LogoPixmap).isNull());
    QVERIFY(page->pixmap(QWizard::WatermarkPixmap).isNull());
    QVERIFY(page->pixmap(QWizard::BackgroundPixmap).isNull());

    wizard.setPixmap(QWizard::BannerPixmap, p1);
    wizard.setPixmap(QWizard::LogoPixmap, p2);
    wizard.setPixmap(QWizard::WatermarkPixmap, p3);
    wizard.setPixmap(QWizard::BackgroundPixmap, p4);

    page->setPixmap(QWizard::LogoPixmap, p5);

    QVERIFY(wizard.pixmap(QWizard::BannerPixmap).size() == p1.size());
    QVERIFY(wizard.pixmap(QWizard::LogoPixmap).size() == p2.size());
    QVERIFY(wizard.pixmap(QWizard::WatermarkPixmap).size() == p3.size());
    QVERIFY(wizard.pixmap(QWizard::BackgroundPixmap).size() == p4.size());

    QVERIFY(page->pixmap(QWizard::BannerPixmap).size() == p1.size());
    QVERIFY(page->pixmap(QWizard::LogoPixmap).size() == p5.size());
    QVERIFY(page->pixmap(QWizard::WatermarkPixmap).size() == p3.size());
    QVERIFY(page->pixmap(QWizard::BackgroundPixmap).size() == p4.size());

    QVERIFY(page2->pixmap(QWizard::BannerPixmap).size() == p1.size());
    QVERIFY(page2->pixmap(QWizard::LogoPixmap).size() == p2.size());
    QVERIFY(page2->pixmap(QWizard::WatermarkPixmap).size() == p3.size());
    QVERIFY(page2->pixmap(QWizard::BackgroundPixmap).size() == p4.size());
}

class MyPage1 : public QWizardPage
{
public:
    MyPage1() {
        edit1 = new QLineEdit("Bla 1");

        edit2 = new QLineEdit("Bla 2");
        edit2->setInputMask("Mask");

        edit3 = new QLineEdit("Bla 3");
        edit3->setMaxLength(25);

        edit4 = new QLineEdit("Bla 4");
    }

    void registerField(const QString &name, QWidget *widget,
                       const char *property = 0,
                       const char *changedSignal = 0)
        { QWizardPage::registerField(name, widget, property, changedSignal); }

    QLineEdit *edit1;
    QLineEdit *edit2;
    QLineEdit *edit3;
    QLineEdit *edit4;
};

void tst_QWizard::setDefaultProperty()
{
    QWizard wizard;
    MyPage1 *page = new MyPage1;
    wizard.addPage(page);

    page->registerField("edit1", page->edit1);

    wizard.setDefaultProperty("QLineEdit", "inputMask", 0);
    page->registerField("edit2", page->edit2);

    wizard.setDefaultProperty("QLineEdit", "maxLength", 0);
    page->registerField("edit3", page->edit3);

    wizard.setDefaultProperty("QLineEdit", "text", SIGNAL(textChanged(QString)));
    page->registerField("edit3bis", page->edit3);

    wizard.setDefaultProperty("QWidget", "enabled", 0); // less specific, i.e. ignored
    page->registerField("edit4", page->edit4);

    wizard.setDefaultProperty("QLineEdit", "customProperty", 0);
    page->registerField("edit4bis", page->edit4);

    QCOMPARE(wizard.field("edit1").toString(), QString("Bla 1"));
    QCOMPARE(wizard.field("edit2").toString(), page->edit2->inputMask());
    QCOMPARE(wizard.field("edit3").toInt(), 25);
    QCOMPARE(wizard.field("edit3bis").toString(), QString("Bla 3"));
    QCOMPARE(wizard.field("edit4").toString(), QString("Bla 4"));
    QCOMPARE(wizard.field("edit4bis").toString(), QString());

    wizard.setField("edit1", "Alpha");
    wizard.setField("edit2", "Beta");
    wizard.setField("edit3", 50);
    wizard.setField("edit3bis", "Gamma");
    wizard.setField("edit4", "Delta");
    wizard.setField("edit4bis", "Epsilon");

    QCOMPARE(wizard.field("edit1").toString(), QString("Alpha"));
    QVERIFY(wizard.field("edit2").toString().contains("Beta"));
    QCOMPARE(wizard.field("edit3").toInt(), 50);
    QCOMPARE(wizard.field("edit3bis").toString(), QString("Gamma"));
    QCOMPARE(wizard.field("edit4").toString(), QString("Delta"));
    QCOMPARE(wizard.field("edit4bis").toString(), QString("Epsilon"));

    // make sure the data structure is reasonable
    for (int i = 0; i < 200000; ++i) {
        wizard.setDefaultProperty("QLineEdit", "x" + QByteArray::number(i), 0);
        wizard.setDefaultProperty("QLabel", "y" + QByteArray::number(i), 0);
    }
}

void tst_QWizard::addPage()
{
    QWidget *parent = new QWidget;
    QWizard wizard;
    const int N = 100;
    QWizardPage *pages[N];

    for (int i = 0; i < N; ++i) {
        pages[i] = new QWizardPage(parent);
        QCOMPARE(wizard.addPage(pages[i]), i);
        QCOMPARE(pages[i]->window(), &wizard);
        QCOMPARE(wizard.startId(), 0);
    }

    for (int i = 0; i < N; ++i) {
        QVERIFY(pages[i] == wizard.page(i));
    }
    QVERIFY(!wizard.page(-1));
    QVERIFY(!wizard.page(N));
    QVERIFY(!wizard.page(N + 1));

    wizard.setPage(N + 50, new QWizardPage);
    wizard.setPage(-3000, new QWizardPage);

    QWizardPage *pageX = new QWizardPage;
    QCOMPARE(wizard.addPage(pageX), N + 51);
    QCOMPARE(wizard.page(N + 51), pageX);

    QCOMPARE(wizard.addPage(new QWizardPage), N + 52);

    wizard.addPage(0); // generates a warning
}

#define CHECK_VISITED(wizard, list) \
    do { \
        QList<int> myList = list; \
        QCOMPARE((wizard).visitedPages(), myList); \
        Q_FOREACH(int id, myList) \
            QVERIFY((wizard).hasVisitedPage(id)); \
    } while (0)

void tst_QWizard::setPage()
{
    QWidget *parent = new QWidget;
    QWizard wizard;
    QWizardPage *page;

    QCOMPARE(wizard.startId(), -1);
    QCOMPARE(wizard.currentId(), -1);
    QVERIFY(!wizard.currentPage());
    QCOMPARE(wizard.nextId(), -1);

    page = new QWizardPage(parent);
    wizard.setPage(-1, page);   // gives a warning and does nothing
    QVERIFY(!wizard.page(-2));
    QVERIFY(!wizard.page(-1));
    QVERIFY(!wizard.page(0));
    QCOMPARE(wizard.startId(), -1);
    QCOMPARE(wizard.currentId(), -1);
    QVERIFY(!wizard.currentPage());
    QCOMPARE(wizard.nextId(), -1);
    CHECK_VISITED(wizard, QList<int>());

    page = new QWizardPage(parent);
    wizard.setPage(0, page);
    QCOMPARE(page->window(), &wizard);
    QCOMPARE(wizard.page(0), page);
    QCOMPARE(wizard.startId(), 0);
    QCOMPARE(wizard.currentId(), -1);
    QVERIFY(!wizard.currentPage());
    QCOMPARE(wizard.nextId(), -1);
    CHECK_VISITED(wizard, QList<int>());

    page = new QWizardPage(parent);
    wizard.setPage(-2, page);
    QCOMPARE(page->window(), &wizard);
    QCOMPARE(wizard.page(-2), page);
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), -1);
    QVERIFY(!wizard.currentPage());
    QCOMPARE(wizard.nextId(), -1);
    CHECK_VISITED(wizard, QList<int>());

    wizard.restart();
    QCOMPARE(wizard.page(-2), page);
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == page);
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -2);

    page = new QWizardPage(parent);
    wizard.setPage(2, page);
    QCOMPARE(wizard.page(2), page);
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -2);

    wizard.restart();
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -2);

    page = new QWizardPage(parent);
    wizard.setPage(-3, page);
    QCOMPARE(wizard.page(-3), page);
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -2);

    wizard.restart();
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), -3);
    QVERIFY(wizard.currentPage() == wizard.page(-3));
    QCOMPARE(wizard.nextId(), -2);
    CHECK_VISITED(wizard, QList<int>() << -3);

    wizard.next();
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -3 << -2);

    wizard.next();
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), 0);
    QVERIFY(wizard.currentPage() == wizard.page(0));
    QCOMPARE(wizard.nextId(), 2);
    CHECK_VISITED(wizard, QList<int>() << -3 << -2 << 0);

    for (int i = 0; i < 100; ++i) {
        wizard.next();
        QCOMPARE(wizard.startId(), -3);
        QCOMPARE(wizard.currentId(), 2);
        QVERIFY(wizard.currentPage() == wizard.page(2));
        QCOMPARE(wizard.nextId(), -1);
        CHECK_VISITED(wizard, QList<int>() << -3 << -2 << 0 << 2);
    }

    wizard.back();
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), 0);
    QVERIFY(wizard.currentPage() == wizard.page(0));
    QCOMPARE(wizard.nextId(), 2);
    CHECK_VISITED(wizard, QList<int>() << -3 << -2 << 0);

    wizard.back();
    QCOMPARE(wizard.startId(), -3);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);
    CHECK_VISITED(wizard, QList<int>() << -3 << -2);

    for (int i = 0; i < 100; ++i) {
        wizard.back();
        QCOMPARE(wizard.startId(), -3);
        QCOMPARE(wizard.currentId(), -3);
        QVERIFY(wizard.currentPage() == wizard.page(-3));
        QCOMPARE(wizard.nextId(), -2);
        CHECK_VISITED(wizard, QList<int>() << -3);
    }

    for (int i = 0; i < 100; ++i) {
        wizard.restart();
        QCOMPARE(wizard.startId(), -3);
        QCOMPARE(wizard.currentId(), -3);
        QVERIFY(wizard.currentPage() == wizard.page(-3));
        QCOMPARE(wizard.nextId(), -2);
        CHECK_VISITED(wizard, QList<int>() << -3);
    }
}

void tst_QWizard::setStartId()
{
    QWizard wizard;
    QCOMPARE(wizard.startId(), -1);

    wizard.setPage(INT_MIN, new QWizardPage);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setPage(-2, new QWizardPage);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setPage(0, new QWizardPage);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setPage(1, new QWizardPage);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setPage(INT_MAX, new QWizardPage);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setStartId(-1);
    QCOMPARE(wizard.startId(), INT_MIN);

    wizard.setStartId(-2);
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.nextId(), -1);

    wizard.restart();
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), -2);
    QVERIFY(wizard.currentPage() == wizard.page(-2));
    QCOMPARE(wizard.nextId(), 0);

    wizard.next();
    QCOMPARE(wizard.startId(), -2);
    QCOMPARE(wizard.currentId(), 0);
    QVERIFY(wizard.currentPage() == wizard.page(0));
    QCOMPARE(wizard.nextId(), 1);

    wizard.setStartId(INT_MIN);
    QCOMPARE(wizard.startId(), INT_MIN);
    QCOMPARE(wizard.currentId(), 0);
    QVERIFY(wizard.currentPage() == wizard.page(0));
    QCOMPARE(wizard.nextId(), 1);

    wizard.next();
    QCOMPARE(wizard.startId(), INT_MIN);
    QCOMPARE(wizard.currentId(), 1);
    QVERIFY(wizard.currentPage() == wizard.page(1));
    QCOMPARE(wizard.nextId(), INT_MAX);

    wizard.next();
    QCOMPARE(wizard.startId(), INT_MIN);
    QCOMPARE(wizard.currentId(), INT_MAX);
    QVERIFY(wizard.currentPage() == wizard.page(INT_MAX));
    QCOMPARE(wizard.nextId(), -1);
    CHECK_VISITED(wizard, QList<int>() << -2 << 0 << 1 << INT_MAX);
}

struct MyPage2 : public QWizardPage
{
public:
    MyPage2() : init(0), cleanup(0), validate(0) {}

    void initializePage() { ++init; QWizardPage::initializePage(); checkInvariant(); }
    void cleanupPage() { ++cleanup; QWizardPage::cleanupPage(); checkInvariant(); }
    bool validatePage() { ++validate; return QWizardPage::validatePage(); }

    void check(int init, int cleanup) { Q_ASSERT(init == this->init && cleanup == this->cleanup); }

    int init;
    int cleanup;
    int validate;

private:
    void checkInvariant() { Q_ASSERT(init == cleanup || init - 1 == cleanup); }
};

#define CHECK_PAGE_INIT(i0, c0, i1, c1, i2, c2) \
    page0->check((i0), (c0)); \
    page1->check((i1), (c1)); \
    page2->check((i2), (c2));

void tst_QWizard::setOption_IndependentPages()
{
    MyPage2 *page0 = new MyPage2;
    MyPage2 *page1 = new MyPage2;
    MyPage2 *page2 = new MyPage2;

    QWizard wizard;
    wizard.addPage(page0);
    wizard.addPage(page1);
    wizard.addPage(page2);

    QVERIFY(!wizard.testOption(QWizard::IndependentPages));

    wizard.restart();

    /*
        Make sure initializePage() and cleanupPage() are called are
        they should when the
        wizard.testOption(QWizard::IndependentPages option is off.
    */
    for (int i = 0; i < 10; ++i) {
        CHECK_PAGE_INIT(i + 1, i, i, i, i, i);

        wizard.next();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i, i, i);

        wizard.next();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i, i + 1, i);

        wizard.next();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i, i + 1, i);

        wizard.back();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i, i + 1, i + 1);

        wizard.back();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i + 1, i + 1, i + 1);

        wizard.back();
        CHECK_PAGE_INIT(i + 1, i, i + 1, i + 1, i + 1, i + 1);

        wizard.restart();
    }

    CHECK_PAGE_INIT(11, 10, 10, 10, 10, 10);

    wizard.next();
    CHECK_PAGE_INIT(11, 10, 11, 10, 10, 10);

    /*
        Now, turn on the option and check that they're called at the
        appropiate times (which aren't the same).
    */
    wizard.setOption(QWizard::IndependentPages, true);
    CHECK_PAGE_INIT(11, 10, 11, 10, 10, 10);

    wizard.back();
    CHECK_PAGE_INIT(11, 10, 11, 10, 10, 10);

    wizard.next();
    CHECK_PAGE_INIT(11, 10, 11, 10, 10, 10);

    wizard.next();
    CHECK_PAGE_INIT(11, 10, 11, 10, 11, 10);

    wizard.next();
    CHECK_PAGE_INIT(11, 10, 11, 10, 11, 10);

    wizard.back();
    CHECK_PAGE_INIT(11, 10, 11, 10, 11, 10);

    wizard.setStartId(2);

    wizard.restart();
    CHECK_PAGE_INIT(11, 11, 11, 11, 12, 11);

    wizard.back();
    CHECK_PAGE_INIT(11, 11, 11, 11, 12, 11);

    wizard.next();
    CHECK_PAGE_INIT(11, 11, 11, 11, 12, 11);

    wizard.setStartId(0);
    wizard.restart();
    CHECK_PAGE_INIT(12, 11, 11, 11, 12, 12);

    wizard.next();
    CHECK_PAGE_INIT(12, 11, 12, 11, 12, 12);

    wizard.next();
    CHECK_PAGE_INIT(12, 11, 12, 11, 13, 12);

    wizard.back();
    CHECK_PAGE_INIT(12, 11, 12, 11, 13, 12);

    /*
        Fun stuff here.
    */

    wizard.setOption(QWizard::IndependentPages, false);
    CHECK_PAGE_INIT(12, 11, 12, 11, 13, 13);

    wizard.setOption(QWizard::IndependentPages, true);
    CHECK_PAGE_INIT(12, 11, 12, 11, 13, 13);

    wizard.setOption(QWizard::IndependentPages, false);
    CHECK_PAGE_INIT(12, 11, 12, 11, 13, 13);

    wizard.back();
    CHECK_PAGE_INIT(12, 11, 12, 12, 13, 13);

    wizard.back();
    CHECK_PAGE_INIT(12, 11, 12, 12, 13, 13);
}

void tst_QWizard::setOption_IgnoreSubTitles()
{
    QWizard wizard1;
    wizard1.setButtonLayout(QList<QWizard::WizardButton>() << QWizard::CancelButton);
    wizard1.resize(500, 500);
    QVERIFY(!wizard1.testOption(QWizard::IgnoreSubTitles));
    QWizardPage *page11 = new QWizardPage;
    page11->setTitle("Page X");
    page11->setSubTitle("Some subtitle");

    QWizardPage *page12 = new QWizardPage;
    page12->setTitle("Page X");

    wizard1.addPage(page11);
    wizard1.addPage(page12);

    QWizard wizard2;
    wizard2.setButtonLayout(QList<QWizard::WizardButton>() << QWizard::CancelButton);
    wizard2.resize(500, 500);
    wizard2.setOption(QWizard::IgnoreSubTitles, true);
    QWizardPage *page21 = new QWizardPage;
    page21->setTitle("Page X");
    page21->setSubTitle("Some subtitle");

    QWizardPage *page22 = new QWizardPage;
    page22->setTitle("Page X");

    wizard2.addPage(page21);
    wizard2.addPage(page22);

    wizard1.show();
    wizard2.show();

    /*
        Check that subtitles are shown when they should (i.e.,
        they're set and IgnoreSubTitles is off).
    */

    QImage i11 = grabWidget(&wizard1);
    QImage i21 = grabWidget(&wizard2);
    QVERIFY(i11 != i21);

    wizard1.next();
    wizard2.next();

    QImage i12 = grabWidget(&wizard1);
    QImage i22 = grabWidget(&wizard2);
    QVERIFY(i12 == i22);
    QVERIFY(i21 == i22);

    wizard1.back();
    wizard2.back();

    QImage i13 = grabWidget(&wizard1);
    QImage i23 = grabWidget(&wizard2);
    QVERIFY(i13 == i11);
    QVERIFY(i23 == i21);

    wizard1.setOption(QWizard::IgnoreSubTitles, true);
    wizard2.setOption(QWizard::IgnoreSubTitles, false);

    QImage i14 = grabWidget(&wizard1);
    QImage i24 = grabWidget(&wizard2);
    QVERIFY(i14 == i21);
    QVERIFY(i24 == i11);

    /*
        Check the impact of subtitles on the rest of the layout, by
        using a subtitle that looks empty (but that isn't). In
        Classic and Modern styles, this should be enough to trigger a
        "header"; in Mac style, this only creates a QLabel, with no
        text, i.e. it doesn't affect the layout.
    */

    page11->setSubTitle("<b>&nbsp;</b>");    // not quite empty, but looks empty

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            wizard1.setOption(QWizard::IgnoreSubTitles, j == 0);

            wizard1.setWizardStyle(i == 0 ? QWizard::ClassicStyle
                                   : i == 1 ? QWizard::ModernStyle
                                            : QWizard::MacStyle);
            wizard1.restart();
            QImage i1 = grabWidget(&wizard1);

            wizard1.next();
            QImage i2 = grabWidget(&wizard1);

            if (j == 0 || wizard1.wizardStyle() == QWizard::MacStyle) {
                QVERIFY(i1 == i2);
            } else {
                QVERIFY(i1 != i2);
            }
        }
    }
}

void tst_QWizard::setOption_ExtendedWatermarkPixmap()
{
    QPixmap watermarkPixmap(200, 400);

    QWizard wizard1;
    wizard1.setButtonLayout(QList<QWizard::WizardButton>() << QWizard::CancelButton);
    QVERIFY(!wizard1.testOption(QWizard::ExtendedWatermarkPixmap));
    QWizardPage *page11 = new QWizardPage;
    page11->setTitle("Page X");
    page11->setPixmap(QWizard::WatermarkPixmap, watermarkPixmap);

    QWizardPage *page12 = new QWizardPage;
    page12->setTitle("Page X");

    wizard1.addPage(page11);
    wizard1.addPage(page12);

    QWizard wizard2;
    wizard2.setButtonLayout(QList<QWizard::WizardButton>() << QWizard::CancelButton);
    wizard2.setOption(QWizard::ExtendedWatermarkPixmap, true);
    QWizardPage *page21 = new QWizardPage;
    page21->setTitle("Page X");
    page21->setPixmap(QWizard::WatermarkPixmap, watermarkPixmap);

    QWizardPage *page22 = new QWizardPage;
    page22->setTitle("Page X");

    wizard2.addPage(page21);
    wizard2.addPage(page22);

    wizard1.show();
    wizard2.show();

    /*
        Check the impact of watermark pixmaps on the rest of the layout.
    */

    for (int i = 0; i < 3; ++i) {
        QImage i1[2];
        QImage i2[2];
        for (int j = 0; j < 2; ++j) {
            wizard1.setOption(QWizard::ExtendedWatermarkPixmap, j == 0);

            wizard1.setWizardStyle(i == 0 ? QWizard::ClassicStyle
                                   : i == 1 ? QWizard::ModernStyle
                                            : QWizard::MacStyle);
            wizard1.restart();
            wizard1.setMaximumSize(1000, 1000);
            wizard1.resize(600, 600);
            i1[j] = grabWidget(&wizard1);

            wizard1.next();
            wizard1.setMaximumSize(1000, 1000);
            wizard1.resize(600, 600);
            i2[j] = grabWidget(&wizard1);
        }

        if (wizard1.wizardStyle() == QWizard::MacStyle) {
            QVERIFY(i1[0] == i1[1]);
            QVERIFY(i2[0] == i2[1]);
            QVERIFY(i1[0] == i2[0]);
        } else {
            QVERIFY(i1[0] != i1[1]);
            QVERIFY(i2[0] == i2[1]);
            QVERIFY(i1[0] != i2[0]);
            QVERIFY(i1[1] != i2[1]);
        }
    }
}

void tst_QWizard::setOption_NoDefaultButton()
{
    QWizard wizard;
    wizard.setOption(QWizard::NoDefaultButton, false);
    wizard.setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
    wizard.addPage(new QWizardPage);
    wizard.page(0)->setFinal(true);
    wizard.addPage(new QWizardPage);

    if (QPushButton *pb = qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton)))
        pb->setAutoDefault(false);
    if (QPushButton *pb = qobject_cast<QPushButton *>(wizard.button(QWizard::FinishButton)))
        pb->setAutoDefault(false);

    wizard.show();
    qApp->processEvents();
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());
    QVERIFY(wizard.button(QWizard::FinishButton)->isEnabled());

    wizard.next();
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::FinishButton))->isDefault());

    wizard.back();
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());

    wizard.setOption(QWizard::NoDefaultButton, true);
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::FinishButton))->isDefault());

    wizard.next();
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::FinishButton))->isDefault());

    wizard.back();
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());
    QVERIFY(!qobject_cast<QPushButton *>(wizard.button(QWizard::FinishButton))->isDefault());

    wizard.setOption(QWizard::NoDefaultButton, false);
    QVERIFY(qobject_cast<QPushButton *>(wizard.button(QWizard::NextButton))->isDefault());
}

void tst_QWizard::setOption_NoBackButtonOnStartPage()
{
    QWizard wizard;
    wizard.setOption(QWizard::NoBackButtonOnStartPage, true);
    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);

    wizard.setStartId(1);
    wizard.show();
    qApp->processEvents();

    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

    wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
    qApp->processEvents();
    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

    wizard.setOption(QWizard::NoBackButtonOnStartPage, true);
    qApp->processEvents();
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

    wizard.next();
    qApp->processEvents();
    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

    wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

    wizard.setOption(QWizard::NoBackButtonOnStartPage, true);
    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

    wizard.back();
    qApp->processEvents();
    QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());
}

void tst_QWizard::setOption_NoBackButtonOnLastPage()
{
    for (int i = 0; i < 2; ++i) {
        QWizard wizard;
        wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
        wizard.setOption(QWizard::NoBackButtonOnLastPage, true);
        wizard.setOption(QWizard::DisabledBackButtonOnLastPage, i == 0);    // changes nothing
        wizard.addPage(new QWizardPage);
        wizard.addPage(new QWizardPage);
        wizard.page(1)->setFinal(true);     // changes nothing (final != last in general)
        wizard.addPage(new QWizardPage);

        wizard.setStartId(1);
        wizard.show();
        qApp->processEvents();

        QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

        wizard.next();
        qApp->processEvents();
        QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

        wizard.next();
        qApp->processEvents();
        QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

        wizard.back();
        qApp->processEvents();
        QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

        wizard.next();
        qApp->processEvents();
        QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

        wizard.setOption(QWizard::NoBackButtonOnLastPage, false);
        QVERIFY(wizard.button(QWizard::BackButton)->isVisible());

        wizard.setOption(QWizard::NoBackButtonOnLastPage, true);
        QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());

        wizard.addPage(new QWizardPage);
        QVERIFY(!wizard.button(QWizard::BackButton)->isVisible());  // this is maybe wrong
    }
}

void tst_QWizard::setOption_DisabledBackButtonOnLastPage()
{
    QWizard wizard;
    wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
    wizard.setOption(QWizard::DisabledBackButtonOnLastPage, true);
    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);
    wizard.page(1)->setFinal(true);     // changes nothing (final != last in general)
    wizard.addPage(new QWizardPage);

    wizard.setStartId(1);
    wizard.show();
    qApp->processEvents();

    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.next();
    qApp->processEvents();
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.next();
    qApp->processEvents();
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.back();
    qApp->processEvents();
    QVERIFY(wizard.button(QWizard::BackButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.next();
    qApp->processEvents();
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.setOption(QWizard::DisabledBackButtonOnLastPage, false);
    QVERIFY(wizard.button(QWizard::BackButton)->isEnabled());

    wizard.setOption(QWizard::DisabledBackButtonOnLastPage, true);
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());

    wizard.addPage(new QWizardPage);
    QVERIFY(!wizard.button(QWizard::BackButton)->isEnabled());  // this is maybe wrong
}

void tst_QWizard::setOption_HaveNextButtonOnLastPage()
{
    QWizard wizard;
    wizard.setOption(QWizard::HaveNextButtonOnLastPage, false);
    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);
    wizard.page(1)->setFinal(true);     // changes nothing (final != last in general)
    wizard.addPage(new QWizardPage);

    wizard.setStartId(1);
    wizard.show();
    qApp->processEvents();

    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(wizard.button(QWizard::NextButton)->isEnabled());

    wizard.next();
    QVERIFY(!wizard.button(QWizard::NextButton)->isVisible());

    wizard.setOption(QWizard::HaveNextButtonOnLastPage, true);
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::NextButton)->isEnabled());

    wizard.next();
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::NextButton)->isEnabled());

    wizard.back();
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(wizard.button(QWizard::NextButton)->isEnabled());

    wizard.setOption(QWizard::HaveNextButtonOnLastPage, false);
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(wizard.button(QWizard::NextButton)->isEnabled());

    wizard.next();
    QVERIFY(!wizard.button(QWizard::NextButton)->isVisible());

    wizard.setOption(QWizard::HaveNextButtonOnLastPage, true);
    QVERIFY(wizard.button(QWizard::NextButton)->isVisible());
    QVERIFY(!wizard.button(QWizard::NextButton)->isEnabled());
}

void tst_QWizard::setOption_HaveFinishButtonOnEarlyPages()
{
    QWizard wizard;
    QVERIFY(!wizard.testOption(QWizard::HaveFinishButtonOnEarlyPages));
    wizard.addPage(new QWizardPage);
    wizard.addPage(new QWizardPage);
    wizard.page(1)->setFinal(true);
    wizard.addPage(new QWizardPage);

    wizard.show();
    qApp->processEvents();

    QVERIFY(!wizard.button(QWizard::FinishButton)->isVisible());

    wizard.next();
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.next();
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.back();
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.back();
    QVERIFY(!wizard.button(QWizard::FinishButton)->isVisible());

    wizard.setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.next();
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.setOption(QWizard::HaveFinishButtonOnEarlyPages, false);
    QVERIFY(wizard.button(QWizard::FinishButton)->isVisible());

    wizard.back();
    QVERIFY(!wizard.button(QWizard::FinishButton)->isVisible());
}

void tst_QWizard::setOption_NoCancelButton()
{
    for (int i = 0; i < 2; ++i) {
        QWizard wizard;
        wizard.setOption(QWizard::NoCancelButton, true);
        wizard.setOption(QWizard::CancelButtonOnLeft, i == 0);
        wizard.addPage(new QWizardPage);
        wizard.addPage(new QWizardPage);
        wizard.show();
        qApp->processEvents();

        QVERIFY(!wizard.button(QWizard::CancelButton)->isVisible());

        wizard.next();
        QVERIFY(!wizard.button(QWizard::CancelButton)->isVisible());

        wizard.setOption(QWizard::NoCancelButton, false);
        QVERIFY(wizard.button(QWizard::CancelButton)->isVisible());

        wizard.back();
        QVERIFY(wizard.button(QWizard::CancelButton)->isVisible());

        wizard.setOption(QWizard::NoCancelButton, true);
        QVERIFY(!wizard.button(QWizard::CancelButton)->isVisible());
    }
}

void tst_QWizard::setOption_CancelButtonOnLeft()
{
    for (int i = 0; i < 2; ++i) {
        int sign = (i == 0) ? +1 : -1;

        QWizard wizard;
        wizard.setLayoutDirection(i == 0 ? Qt::LeftToRight : Qt::RightToLeft);
        wizard.setOption(QWizard::NoCancelButton, false);
        wizard.setOption(QWizard::CancelButtonOnLeft, true);
        wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
        wizard.addPage(new QWizardPage);
        wizard.addPage(new QWizardPage);
        wizard.show();
        qApp->processEvents();

        QVERIFY(sign * wizard.button(QWizard::CancelButton)->x()
                < sign * wizard.button(QWizard::BackButton)->x());

        wizard.next();
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::CancelButton)->x()
                < sign * wizard.button(QWizard::BackButton)->x());

        wizard.setOption(QWizard::CancelButtonOnLeft, false);
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::CancelButton)->x()
                > sign * wizard.button(QWizard::BackButton)->x());

        wizard.back();
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::CancelButton)->x()
                > sign * wizard.button(QWizard::BackButton)->x());
    }
}

void tst_QWizard::setOption_HaveHelpButton()
{
    for (int i = 0; i < 2; ++i) {
        QWizard wizard;
        QVERIFY(!wizard.testOption(QWizard::HaveHelpButton));
        wizard.setOption(QWizard::HaveHelpButton, false);
        wizard.setOption(QWizard::HelpButtonOnRight, i == 0);
        wizard.addPage(new QWizardPage);
        wizard.addPage(new QWizardPage);
        wizard.show();
        qApp->processEvents();

        QVERIFY(!wizard.button(QWizard::HelpButton)->isVisible());

        wizard.next();
        QVERIFY(!wizard.button(QWizard::HelpButton)->isVisible());

        wizard.setOption(QWizard::HaveHelpButton, true);
        QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());

        wizard.back();
        QVERIFY(wizard.button(QWizard::HelpButton)->isVisible());

        wizard.setOption(QWizard::HaveHelpButton, false);
        QVERIFY(!wizard.button(QWizard::HelpButton)->isVisible());
    }
}

void tst_QWizard::setOption_HelpButtonOnRight()
{
    for (int i = 0; i < 2; ++i) {
        int sign = (i == 0) ? +1 : -1;

        QWizard wizard;
        wizard.setLayoutDirection(i == 0 ? Qt::LeftToRight : Qt::RightToLeft);
        wizard.setOption(QWizard::HaveHelpButton, true);
        wizard.setOption(QWizard::HelpButtonOnRight, false);
        wizard.setOption(QWizard::NoBackButtonOnStartPage, false);
        wizard.addPage(new QWizardPage);
        wizard.addPage(new QWizardPage);
        wizard.show();
        qApp->processEvents();

        QVERIFY(sign * wizard.button(QWizard::HelpButton)->x()
                < sign * wizard.button(QWizard::BackButton)->x());

        wizard.next();
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::HelpButton)->x()
                < sign * wizard.button(QWizard::BackButton)->x());

        wizard.setOption(QWizard::HelpButtonOnRight, true);
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::HelpButton)->x()
                > sign * wizard.button(QWizard::BackButton)->x());

        wizard.back();
        qApp->processEvents();
        QVERIFY(sign * wizard.button(QWizard::HelpButton)->x()
                > sign * wizard.button(QWizard::BackButton)->x());
    }
}

void tst_QWizard::setOption_HaveCustomButtonX()
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                QWizard wizard;
                wizard.setLayoutDirection(Qt::LeftToRight);
                wizard.addPage(new QWizardPage);
                wizard.addPage(new QWizardPage);
                wizard.show();

                wizard.setButtonText(QWizard::CustomButton1, "Foo");
                wizard.setButton(QWizard::CustomButton2, new QCheckBox("Bar"));
                wizard.button(QWizard::CustomButton3)->setText("Baz");

                wizard.setOption(QWizard::HaveCustomButton1, i == 0);
                wizard.setOption(QWizard::HaveCustomButton2, j == 0);
                wizard.setOption(QWizard::HaveCustomButton3, k == 0);

                QVERIFY(wizard.button(QWizard::CustomButton1)->isHidden() == (i != 0));
                QVERIFY(wizard.button(QWizard::CustomButton2)->isHidden() == (j != 0));
                QVERIFY(wizard.button(QWizard::CustomButton3)->isHidden() == (k != 0));

                if (i + j + k == 0) {
                    qApp->processEvents();
                    QVERIFY(wizard.button(QWizard::CustomButton1)->x()
                            < wizard.button(QWizard::CustomButton2)->x());
                    QVERIFY(wizard.button(QWizard::CustomButton2)->x()
                            < wizard.button(QWizard::CustomButton3)->x());
                }
            }
        }
    }
}

QTEST_MAIN(tst_QWizard)

#include "tst_qwizard.moc"
#else
int main()
{
    return 0;
}
#endif
