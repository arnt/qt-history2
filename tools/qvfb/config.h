#ifndef CONFIG_H
#define CONFIG_H

#include <qvariant.h>

class GammaView;

#include <qaction.h>
#include <q3buttongroup.h>
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qslider.h>
#include "gammaview.h"
#include <qapplication.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qdialog.h>

namespace Ui {

class Config
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    Q3ButtonGroup *ButtonGroup1;
    QVBoxLayout *vboxLayout1;
    QRadioButton *size_176_220;
    QRadioButton *size_240_320;
    QRadioButton *size_320_240;
    QRadioButton *size_640_480;
    QHBoxLayout *hboxLayout1;
    QRadioButton *size_custom;
    QSpinBox *size_width;
    QSpinBox *size_height;
    Q3ButtonGroup *ButtonGroup2;
    QVBoxLayout *vboxLayout2;
    QRadioButton *depth_1;
    QRadioButton *depth_4gray;
    QRadioButton *depth_8;
    QRadioButton *depth_12;
    QRadioButton *depth_16;
    QRadioButton *depth_32;
    QHBoxLayout *hboxLayout2;
    QLabel *TextLabel1_3;
    QComboBox *skin;
    QCheckBox *touchScreen;
    QCheckBox *lcdScreen;
    QLabel *TextLabel1;
    Q3GroupBox *GroupBox1;
    QGridLayout *gridLayout;
    QLabel *TextLabel3;
    QSlider *bslider;
    QLabel *blabel;
    QLabel *TextLabel2;
    QSlider *gslider;
    QLabel *glabel;
    QLabel *TextLabel7;
    QLabel *TextLabel8;
    QSlider *gammaslider;
    QLabel *TextLabel1_2;
    QLabel *rlabel;
    QSlider *rslider;
    QPushButton *PushButton3;
    GammaView *MyCustomWidget1;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    inline void setupUi(QDialog *Config);
    inline void retranslateUi(QDialog *Config);

protected:
    enum IconID
    {
        image0_ID,
        unknown_ID
    };

    static QPixmap icon(IconID id);
};

inline void Config::setupUi(QDialog *Config)
{
    Config->setObjectName(QString::fromUtf8("Config"));
    Config->resize(QSize(585, 756).expandedTo(Config->minimumSizeHint()));
    Config->setSizeGripEnabled(true);
    vboxLayout = new QVBoxLayout(Config);
    vboxLayout->setMargin(11);
    vboxLayout->setSpacing(6);
    hboxLayout = new QHBoxLayout();
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(6);
    ButtonGroup1 = new Q3ButtonGroup(Config);
    ButtonGroup1->setObjectName(QString::fromUtf8("ButtonGroup1"));
    QSizePolicy sizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ButtonGroup1->sizePolicy().hasHeightForWidth());
    ButtonGroup1->setSizePolicy(sizePolicy);
    ButtonGroup1->setColumnLayout(0, Qt::Vertical);
    ButtonGroup1->layout()->setSpacing(6);
    ButtonGroup1->layout()->setMargin(11);
    vboxLayout1 = new QVBoxLayout(ButtonGroup1->layout());
    vboxLayout1->setAlignment(Qt::AlignTop);
    vboxLayout1->setMargin(11);
    vboxLayout1->setSpacing(6);
    size_176_220 = new QRadioButton(ButtonGroup1);
    size_176_220->setObjectName(QString::fromUtf8("size_176_220"));

    vboxLayout1->addWidget(size_176_220);

    size_240_320 = new QRadioButton(ButtonGroup1);
    size_240_320->setObjectName(QString::fromUtf8("size_240_320"));

    vboxLayout1->addWidget(size_240_320);

    size_320_240 = new QRadioButton(ButtonGroup1);
    size_320_240->setObjectName(QString::fromUtf8("size_320_240"));

    vboxLayout1->addWidget(size_320_240);

    size_640_480 = new QRadioButton(ButtonGroup1);
    size_640_480->setObjectName(QString::fromUtf8("size_640_480"));

    vboxLayout1->addWidget(size_640_480);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setMargin(0);
    hboxLayout1->setSpacing(6);
    size_custom = new QRadioButton(ButtonGroup1);
    size_custom->setObjectName(QString::fromUtf8("size_custom"));
    QSizePolicy sizePolicy1((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(size_custom->sizePolicy().hasHeightForWidth());
    size_custom->setSizePolicy(sizePolicy1);

    hboxLayout1->addWidget(size_custom);

    size_width = new QSpinBox(ButtonGroup1);
    size_width->setObjectName(QString::fromUtf8("size_width"));
    QSizePolicy sizePolicy2((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(size_width->sizePolicy().hasHeightForWidth());
    size_width->setSizePolicy(sizePolicy2);
    size_width->setMaximum(1280);
    size_width->setMinimum(1);
    size_width->setSingleStep(16);
    size_width->setValue(400);

    hboxLayout1->addWidget(size_width);

    size_height = new QSpinBox(ButtonGroup1);
    size_height->setObjectName(QString::fromUtf8("size_height"));
    QSizePolicy sizePolicy3((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(size_height->sizePolicy().hasHeightForWidth());
    size_height->setSizePolicy(sizePolicy3);
    size_height->setMaximum(1024);
    size_height->setMinimum(1);
    size_height->setSingleStep(16);
    size_height->setValue(300);

    hboxLayout1->addWidget(size_height);


    vboxLayout1->addLayout(hboxLayout1);


    hboxLayout->addWidget(ButtonGroup1);

    ButtonGroup2 = new Q3ButtonGroup(Config);
    ButtonGroup2->setObjectName(QString::fromUtf8("ButtonGroup2"));
    ButtonGroup2->setColumnLayout(0, Qt::Vertical);
    ButtonGroup2->layout()->setSpacing(6);
    ButtonGroup2->layout()->setMargin(11);
    vboxLayout2 = new QVBoxLayout(ButtonGroup2->layout());
    vboxLayout2->setAlignment(Qt::AlignTop);
    vboxLayout2->setMargin(11);
    vboxLayout2->setSpacing(6);
    depth_1 = new QRadioButton(ButtonGroup2);
    depth_1->setObjectName(QString::fromUtf8("depth_1"));

    vboxLayout2->addWidget(depth_1);

    depth_4gray = new QRadioButton(ButtonGroup2);
    depth_4gray->setObjectName(QString::fromUtf8("depth_4gray"));

    vboxLayout2->addWidget(depth_4gray);

    depth_8 = new QRadioButton(ButtonGroup2);
    depth_8->setObjectName(QString::fromUtf8("depth_8"));

    vboxLayout2->addWidget(depth_8);

    depth_12 = new QRadioButton(ButtonGroup2);
    depth_12->setObjectName(QString::fromUtf8("depth_12"));

    vboxLayout2->addWidget(depth_12);

    depth_16 = new QRadioButton(ButtonGroup2);
    depth_16->setObjectName(QString::fromUtf8("depth_16"));

    vboxLayout2->addWidget(depth_16);

    depth_32 = new QRadioButton(ButtonGroup2);
    depth_32->setObjectName(QString::fromUtf8("depth_32"));

    vboxLayout2->addWidget(depth_32);


    hboxLayout->addWidget(ButtonGroup2);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setMargin(0);
    hboxLayout2->setSpacing(6);
    TextLabel1_3 = new QLabel(Config);
    TextLabel1_3->setObjectName(QString::fromUtf8("TextLabel1_3"));

    hboxLayout2->addWidget(TextLabel1_3);

    skin = new QComboBox(Config);
    skin->setObjectName(QString::fromUtf8("skin"));
    QSizePolicy sizePolicy4((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(skin->sizePolicy().hasHeightForWidth());
    skin->setSizePolicy(sizePolicy4);

    hboxLayout2->addWidget(skin);


    vboxLayout->addLayout(hboxLayout2);

    touchScreen = new QCheckBox(Config);
    touchScreen->setObjectName(QString::fromUtf8("touchScreen"));

    vboxLayout->addWidget(touchScreen);

    lcdScreen = new QCheckBox(Config);
    lcdScreen->setObjectName(QString::fromUtf8("lcdScreen"));

    vboxLayout->addWidget(lcdScreen);

    TextLabel1 = new QLabel(Config);
    TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));
    QSizePolicy sizePolicy5((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1);
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(TextLabel1->sizePolicy().hasHeightForWidth());
    TextLabel1->setSizePolicy(sizePolicy5);

    vboxLayout->addWidget(TextLabel1);

    GroupBox1 = new Q3GroupBox(Config);
    GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
    GroupBox1->setColumnLayout(0, Qt::Vertical);
    GroupBox1->layout()->setSpacing(6);
    GroupBox1->layout()->setMargin(11);
    gridLayout = new QGridLayout(GroupBox1->layout());
    gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setMargin(11);
    gridLayout->setSpacing(6);
    TextLabel3 = new QLabel(GroupBox1);
    TextLabel3->setObjectName(QString::fromUtf8("TextLabel3"));

    gridLayout->addWidget(TextLabel3, 6, 0, 1, 1);

    bslider = new QSlider(GroupBox1);
    bslider->setObjectName(QString::fromUtf8("bslider"));
    QPalette palette;
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(0, 0, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(127, 127, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(63, 63, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(0, 0, 127));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(0, 0, 170));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(0, 0, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(127, 127, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(38, 38, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(0, 0, 127));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(0, 0, 170));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(0, 0, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(127, 127, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(38, 38, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(0, 0, 127));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(0, 0, 170));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    bslider->setPalette(palette);
    bslider->setMaximum(400);
    bslider->setValue(100);
    bslider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(bslider, 6, 1, 1, 1);

    blabel = new QLabel(GroupBox1);
    blabel->setObjectName(QString::fromUtf8("blabel"));

    gridLayout->addWidget(blabel, 6, 2, 1, 1);

    TextLabel2 = new QLabel(GroupBox1);
    TextLabel2->setObjectName(QString::fromUtf8("TextLabel2"));

    gridLayout->addWidget(TextLabel2, 4, 0, 1, 1);

    gslider = new QSlider(GroupBox1);
    gslider->setObjectName(QString::fromUtf8("gslider"));
    QPalette palette1;
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(0, 255, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(127, 255, 127));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(63, 255, 63));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(0, 127, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(0, 170, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(0, 255, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(127, 255, 127));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(38, 255, 38));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(0, 127, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(0, 170, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(0, 255, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(127, 255, 127));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(38, 255, 38));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(0, 127, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(0, 170, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    gslider->setPalette(palette1);
    gslider->setMaximum(400);
    gslider->setValue(100);
    gslider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(gslider, 4, 1, 1, 1);

    glabel = new QLabel(GroupBox1);
    glabel->setObjectName(QString::fromUtf8("glabel"));

    gridLayout->addWidget(glabel, 4, 2, 1, 1);

    TextLabel7 = new QLabel(GroupBox1);
    TextLabel7->setObjectName(QString::fromUtf8("TextLabel7"));

    gridLayout->addWidget(TextLabel7, 0, 0, 1, 1);

    TextLabel8 = new QLabel(GroupBox1);
    TextLabel8->setObjectName(QString::fromUtf8("TextLabel8"));

    gridLayout->addWidget(TextLabel8, 0, 2, 1, 1);

    gammaslider = new QSlider(GroupBox1);
    gammaslider->setObjectName(QString::fromUtf8("gammaslider"));
    QPalette palette2;
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(255, 255, 255));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 255));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(127, 127, 127));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(170, 170, 170));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(127, 127, 127));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(170, 170, 170));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(127, 127, 127));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(170, 170, 170));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    gammaslider->setPalette(palette2);
    gammaslider->setMaximum(400);
    gammaslider->setValue(100);
    gammaslider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(gammaslider, 0, 1, 1, 1);

    TextLabel1_2 = new QLabel(GroupBox1);
    TextLabel1_2->setObjectName(QString::fromUtf8("TextLabel1_2"));

    gridLayout->addWidget(TextLabel1_2, 2, 0, 1, 1);

    rlabel = new QLabel(GroupBox1);
    rlabel->setObjectName(QString::fromUtf8("rlabel"));

    gridLayout->addWidget(rlabel, 2, 2, 1, 1);

    rslider = new QSlider(GroupBox1);
    rslider->setObjectName(QString::fromUtf8("rslider"));
    QPalette palette3;
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(255, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(255, 127, 127));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(255, 63, 63));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(127, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(170, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(0, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(255, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(255, 127, 127));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(255, 38, 38));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(127, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(170, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(0, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(255, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(255, 127, 127));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(255, 38, 38));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(127, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(170, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(0, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(220, 220, 220));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(10, 95, 137));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    rslider->setPalette(palette3);
    rslider->setMaximum(400);
    rslider->setValue(100);
    rslider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(rslider, 2, 1, 1, 1);

    PushButton3 = new QPushButton(GroupBox1);
    PushButton3->setObjectName(QString::fromUtf8("PushButton3"));

    gridLayout->addWidget(PushButton3, 8, 0, 1, 3);

    MyCustomWidget1 = new GammaView(GroupBox1);
    MyCustomWidget1->setObjectName(QString::fromUtf8("MyCustomWidget1"));

    gridLayout->addWidget(MyCustomWidget1, 0, 3, 9, 1);


    vboxLayout->addWidget(GroupBox1);

    buttonOk = new QPushButton(Config);
    buttonOk->setObjectName(QString::fromUtf8("buttonOk"));
    buttonOk->setAutoDefault(true);
    buttonOk->setDefault(true);

    vboxLayout->addWidget(buttonOk);

    buttonCancel = new QPushButton(Config);
    buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
    buttonCancel->setAutoDefault(true);

    vboxLayout->addWidget(buttonCancel);

    retranslateUi(Config);

    QMetaObject::connectSlotsByName(Config);
}

inline void Config::retranslateUi(QDialog *Config)
{
    Config->setWindowTitle(QApplication::translate("Config", "Configure"));
    ButtonGroup1->setTitle(QApplication::translate("Config", "Size"));
    size_176_220->setText(QApplication::translate("Config", "176x220 \"SmartPhone\""));
    size_240_320->setText(QApplication::translate("Config", "240x320 \"PDA\""));
    size_320_240->setText(QApplication::translate("Config", "320x240 \"TV\" / \"QVGA\""));
    size_640_480->setText(QApplication::translate("Config", "640x480 \"VGA\""));
    size_custom->setText(QApplication::translate("Config", "Custom"));
    ButtonGroup2->setTitle(QApplication::translate("Config", "Depth"));
    depth_1->setText(QApplication::translate("Config", "1 bit monochrome"));
    depth_4gray->setText(QApplication::translate("Config", "4 bit grayscale"));
    depth_8->setText(QApplication::translate("Config", "8 bit"));
    depth_12->setText(QApplication::translate("Config", "12 (16) bit"));
    depth_16->setText(QApplication::translate("Config", "16 bit"));
    depth_32->setText(QApplication::translate("Config", "32 bit"));
    TextLabel1_3->setText(QApplication::translate("Config", "Skin"));
    skin->clear();
    skin->insertItem(QApplication::translate("Config", "None"));
    touchScreen->setText(QApplication::translate("Config", "Emulate touch screen (no mouse move)"));
    lcdScreen->setText(QApplication::translate("Config", "Emulate LCD screen (Only with fixed zoom of 3.0 times magnification)"));
    TextLabel1->setText(QApplication::translate("Config", "<p>Note that any applications using the virtual framebuffer will be terminated if you change the Size or Depth <i>above</i>. You may freely modify the Gamma <i>below</i>."));
    GroupBox1->setTitle(QApplication::translate("Config", "Gamma"));
    TextLabel3->setText(QApplication::translate("Config", "Blue"));
    blabel->setText(QApplication::translate("Config", "1.0"));
    TextLabel2->setText(QApplication::translate("Config", "Green"));
    glabel->setText(QApplication::translate("Config", "1.0"));
    TextLabel7->setText(QApplication::translate("Config", "All"));
    TextLabel8->setText(QApplication::translate("Config", "1.0"));
    TextLabel1_2->setText(QApplication::translate("Config", "Red"));
    rlabel->setText(QApplication::translate("Config", "1.0"));
    PushButton3->setText(QApplication::translate("Config", "Set all to 1.0"));
    buttonOk->setText(QApplication::translate("Config", "&OK"));
    buttonCancel->setText(QApplication::translate("Config", "&Cancel"));
}

inline QPixmap Config::icon(Config::IconID id)
{
    static const char* const image0_data[] = { 
"22 22 2 1",
". c None",
"# c #a4c610",
"........######........",
".....###########......",
"....##############....",
"...################...",
"..######......######..",
"..#####........#####..",
".#####.......#..#####.",
".####.......###..####.",
"####.......#####..####",
"####......#####...####",
"####....#######...####",
"####....######....####",
"####...########...####",
".####.##########..####",
".####..####.#########.",
".#####..##...########.",
"..#####.......#######.",
"..######......######..",
"...###################",
"....##################",
"......###########.###.",
"........######.....#.."};

    switch (id) {
        case image0_ID: return QPixmap((const char**)image0_data);
        default: return QPixmap();
    }
}

}

class Config : public QDialog, public Ui::Config
{
    Q_OBJECT

public:
    Config(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~Config();

protected slots:
    virtual void languageChange();

};

#endif // CONFIG_H
