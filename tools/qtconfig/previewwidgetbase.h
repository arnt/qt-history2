#ifndef PREVIEWWIDGETBASE_H
#define PREVIEWWIDGETBASE_H

#include <qvariant.h>


#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3ProgressBar>
#include <Qt3Support/Q3TextView>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollBar>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <Qt3Support/Q3MimeSourceFactory>

class Ui_PreviewWidgetBase
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    Q3ButtonGroup *ButtonGroup1;
    QVBoxLayout *vboxLayout2;
    QRadioButton *RadioButton1;
    QRadioButton *RadioButton2;
    QRadioButton *RadioButton3;
    Q3ButtonGroup *ButtonGroup2;
    QVBoxLayout *vboxLayout3;
    QCheckBox *CheckBox1;
    QCheckBox *CheckBox2;
    Q3ProgressBar *ProgressBar1;
    QVBoxLayout *vboxLayout4;
    QLineEdit *LineEdit1;
    QComboBox *ComboBox1;
    QHBoxLayout *hboxLayout1;
    QSpinBox *SpinBox1;
    QPushButton *PushButton1;
    QScrollBar *ScrollBar1;
    QSlider *Slider1;
    Q3TextView *textView;
    QSpacerItem *spacerItem;

    void setupUi(QWidget *PreviewWidgetBase)
    {
    PreviewWidgetBase->setObjectName(QString::fromUtf8("PreviewWidgetBase"));
    PreviewWidgetBase->resize(QSize(378, 236).expandedTo(PreviewWidgetBase->minimumSizeHint()));
    QSizePolicy sizePolicy((QSizePolicy::Policy)1, (QSizePolicy::Policy)1);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(PreviewWidgetBase->sizePolicy().hasHeightForWidth());
    PreviewWidgetBase->setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(PreviewWidgetBase);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout->setMargin(11);
    vboxLayout->setSpacing(6);
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    hboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(6);
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    vboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout1->setMargin(0);
    vboxLayout1->setSpacing(6);
    ButtonGroup1 = new Q3ButtonGroup(PreviewWidgetBase);
    ButtonGroup1->setObjectName(QString::fromUtf8("ButtonGroup1"));
    ButtonGroup1->setColumnLayout(0, Qt::Vertical);
    ButtonGroup1->layout()->setSpacing(6);
    ButtonGroup1->layout()->setMargin(11);
    vboxLayout2 = new QVBoxLayout(ButtonGroup1->layout());
    vboxLayout2->setAlignment(Qt::AlignTop);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout2->setMargin(11);
    vboxLayout2->setSpacing(6);
    RadioButton1 = new QRadioButton(ButtonGroup1);
    RadioButton1->setObjectName(QString::fromUtf8("RadioButton1"));
    RadioButton1->setChecked(true);

    vboxLayout2->addWidget(RadioButton1);

    RadioButton2 = new QRadioButton(ButtonGroup1);
    RadioButton2->setObjectName(QString::fromUtf8("RadioButton2"));

    vboxLayout2->addWidget(RadioButton2);

    RadioButton3 = new QRadioButton(ButtonGroup1);
    RadioButton3->setObjectName(QString::fromUtf8("RadioButton3"));

    vboxLayout2->addWidget(RadioButton3);


    vboxLayout1->addWidget(ButtonGroup1);

    ButtonGroup2 = new Q3ButtonGroup(PreviewWidgetBase);
    ButtonGroup2->setObjectName(QString::fromUtf8("ButtonGroup2"));
    ButtonGroup2->setColumnLayout(0, Qt::Vertical);
    ButtonGroup2->layout()->setSpacing(6);
    ButtonGroup2->layout()->setMargin(11);
    vboxLayout3 = new QVBoxLayout(ButtonGroup2->layout());
    vboxLayout3->setAlignment(Qt::AlignTop);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    vboxLayout3->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout3->setMargin(11);
    vboxLayout3->setSpacing(6);
    CheckBox1 = new QCheckBox(ButtonGroup2);
    CheckBox1->setObjectName(QString::fromUtf8("CheckBox1"));
    CheckBox1->setChecked(true);

    vboxLayout3->addWidget(CheckBox1);

    CheckBox2 = new QCheckBox(ButtonGroup2);
    CheckBox2->setObjectName(QString::fromUtf8("CheckBox2"));

    vboxLayout3->addWidget(CheckBox2);


    vboxLayout1->addWidget(ButtonGroup2);

    ProgressBar1 = new Q3ProgressBar(PreviewWidgetBase);
    ProgressBar1->setObjectName(QString::fromUtf8("ProgressBar1"));
    ProgressBar1->setProgress(50);

    vboxLayout1->addWidget(ProgressBar1);


    hboxLayout->addLayout(vboxLayout1);

    vboxLayout4 = new QVBoxLayout();
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    vboxLayout4->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout4->setMargin(0);
    vboxLayout4->setSpacing(6);
    LineEdit1 = new QLineEdit(PreviewWidgetBase);
    LineEdit1->setObjectName(QString::fromUtf8("LineEdit1"));

    vboxLayout4->addWidget(LineEdit1);

    ComboBox1 = new QComboBox(PreviewWidgetBase);
    ComboBox1->setObjectName(QString::fromUtf8("ComboBox1"));

    vboxLayout4->addWidget(ComboBox1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout1->setMargin(0);
    hboxLayout1->setSpacing(6);
    SpinBox1 = new QSpinBox(PreviewWidgetBase);
    SpinBox1->setObjectName(QString::fromUtf8("SpinBox1"));

    hboxLayout1->addWidget(SpinBox1);

    PushButton1 = new QPushButton(PreviewWidgetBase);
    PushButton1->setObjectName(QString::fromUtf8("PushButton1"));

    hboxLayout1->addWidget(PushButton1);


    vboxLayout4->addLayout(hboxLayout1);

    ScrollBar1 = new QScrollBar(PreviewWidgetBase);
    ScrollBar1->setObjectName(QString::fromUtf8("ScrollBar1"));
    ScrollBar1->setOrientation(Qt::Horizontal);

    vboxLayout4->addWidget(ScrollBar1);

    Slider1 = new QSlider(PreviewWidgetBase);
    Slider1->setObjectName(QString::fromUtf8("Slider1"));
    Slider1->setOrientation(Qt::Horizontal);

    vboxLayout4->addWidget(Slider1);

    textView = new Q3TextView(PreviewWidgetBase);
    textView->setObjectName(QString::fromUtf8("textView"));
    textView->setMaximumSize(QSize(32767, 50));

    vboxLayout4->addWidget(textView);


    hboxLayout->addLayout(vboxLayout4);


    vboxLayout->addLayout(hboxLayout);

    spacerItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    retranslateUi(PreviewWidgetBase);

    QMetaObject::connectSlotsByName(PreviewWidgetBase);
    } // setupUi

    void retranslateUi(QWidget *PreviewWidgetBase)
    {
    PreviewWidgetBase->setWindowTitle(QApplication::translate("PreviewWidgetBase", "Preview Window"));
    ButtonGroup1->setTitle(QApplication::translate("PreviewWidgetBase", "ButtonGroup"));
    RadioButton1->setText(QApplication::translate("PreviewWidgetBase", "RadioButton1"));
    RadioButton2->setText(QApplication::translate("PreviewWidgetBase", "RadioButton2"));
    RadioButton3->setText(QApplication::translate("PreviewWidgetBase", "RadioButton3"));
    ButtonGroup2->setTitle(QApplication::translate("PreviewWidgetBase", "ButtonGroup2"));
    CheckBox1->setText(QApplication::translate("PreviewWidgetBase", "CheckBox1"));
    CheckBox2->setText(QApplication::translate("PreviewWidgetBase", "CheckBox2"));
    LineEdit1->setText(QApplication::translate("PreviewWidgetBase", "LineEdit"));
    ComboBox1->addItem(QApplication::translate("PreviewWidgetBase", "ComboBox"));
    PushButton1->setText(QApplication::translate("PreviewWidgetBase", "PushButton"));
    textView->setText(QApplication::translate("PreviewWidgetBase", "<p>\n"
"<a href=\"http://www.trolltech.com\">http://www.trolltech.com</a>\n"
"</p>\n"
"<p>\n"
"<a href=\"http://www.kde.org\">http://www.kde.org</a>\n"
"</p>"));
    Q_UNUSED(PreviewWidgetBase);
    } // retranslateUi

};

namespace Ui
{
    class PreviewWidgetBase: public Ui_PreviewWidgetBase {};
} // namespace Ui

class PreviewWidgetBase : public QWidget, public Ui::PreviewWidgetBase
{
    Q_OBJECT

public:
    PreviewWidgetBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~PreviewWidgetBase();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();


};

#endif // PREVIEWWIDGETBASE_H
