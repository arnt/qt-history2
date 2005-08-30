#ifndef CONFIG_H
#define CONFIG_H

#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3GroupBox>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include "gammaview.h"

class Ui_Config
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

    void setupUi(QDialog *Config)
    {
    Config->setObjectName(QString::fromUtf8("Config"));
    Config->resize(QSize(585, 756).expandedTo(Config->minimumSizeHint()));
    Config->setSizeGripEnabled(true);
    vboxLayout = new QVBoxLayout(Config);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(11);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    ButtonGroup1 = new Q3ButtonGroup(Config);
    ButtonGroup1->setObjectName(QString::fromUtf8("ButtonGroup1"));
    QSizePolicy sizePolicy((QSizePolicy::Policy)5, (QSizePolicy::Policy)5);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ButtonGroup1->sizePolicy().hasHeightForWidth());
    ButtonGroup1->setSizePolicy(sizePolicy);
    ButtonGroup1->setColumnLayout(0, Qt::Vertical);
    ButtonGroup1->layout()->setSpacing(6);
    ButtonGroup1->layout()->setMargin(11);
    vboxLayout1 = new QVBoxLayout(ButtonGroup1->layout());
    vboxLayout1->setAlignment(Qt::AlignTop);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
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
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    size_custom = new QRadioButton(ButtonGroup1);
    size_custom->setObjectName(QString::fromUtf8("size_custom"));
    QSizePolicy sizePolicy1((QSizePolicy::Policy)0, (QSizePolicy::Policy)0);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(size_custom->sizePolicy().hasHeightForWidth());
    size_custom->setSizePolicy(sizePolicy1);

    hboxLayout1->addWidget(size_custom);

    size_width = new QSpinBox(ButtonGroup1);
    size_width->setObjectName(QString::fromUtf8("size_width"));
    QSizePolicy sizePolicy2((QSizePolicy::Policy)7, (QSizePolicy::Policy)0);
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
    QSizePolicy sizePolicy3((QSizePolicy::Policy)7, (QSizePolicy::Policy)0);
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
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
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
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    TextLabel1_3 = new QLabel(Config);
    TextLabel1_3->setObjectName(QString::fromUtf8("TextLabel1_3"));

    hboxLayout2->addWidget(TextLabel1_3);

    skin = new QComboBox(Config);
    skin->setObjectName(QString::fromUtf8("skin"));
    QSizePolicy sizePolicy4((QSizePolicy::Policy)7, (QSizePolicy::Policy)0);
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
    QSizePolicy sizePolicy5((QSizePolicy::Policy)5, (QSizePolicy::Policy)1);
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
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
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
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette2.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette2.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette2.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette3.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette3.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 0));
    palette3.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(0, 0, 0));
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
    } // setupUi

    void retranslateUi(QDialog *Config)
    {
    Config->setWindowTitle(QApplication::translate("Config", "Configure", 0, QApplication::UnicodeUTF8));
    ButtonGroup1->setTitle(QApplication::translate("Config", "Size", 0, QApplication::UnicodeUTF8));
    size_176_220->setText(QApplication::translate("Config", "176x220 \"SmartPhone\"", 0, QApplication::UnicodeUTF8));
    size_240_320->setText(QApplication::translate("Config", "240x320 \"PDA\"", 0, QApplication::UnicodeUTF8));
    size_320_240->setText(QApplication::translate("Config", "320x240 \"TV\" / \"QVGA\"", 0, QApplication::UnicodeUTF8));
    size_640_480->setText(QApplication::translate("Config", "640x480 \"VGA\"", 0, QApplication::UnicodeUTF8));
    size_custom->setText(QApplication::translate("Config", "Custom", 0, QApplication::UnicodeUTF8));
    ButtonGroup2->setTitle(QApplication::translate("Config", "Depth", 0, QApplication::UnicodeUTF8));
    depth_1->setText(QApplication::translate("Config", "1 bit monochrome", 0, QApplication::UnicodeUTF8));
    depth_4gray->setText(QApplication::translate("Config", "4 bit grayscale", 0, QApplication::UnicodeUTF8));
    depth_8->setText(QApplication::translate("Config", "8 bit", 0, QApplication::UnicodeUTF8));
    depth_12->setText(QApplication::translate("Config", "12 (16) bit", 0, QApplication::UnicodeUTF8));
    depth_16->setText(QApplication::translate("Config", "16 bit", 0, QApplication::UnicodeUTF8));
    depth_32->setText(QApplication::translate("Config", "32 bit", 0, QApplication::UnicodeUTF8));
    TextLabel1_3->setText(QApplication::translate("Config", "Skin", 0, QApplication::UnicodeUTF8));
    skin->addItem(QApplication::translate("Config", "None", 0, QApplication::UnicodeUTF8));
    touchScreen->setText(QApplication::translate("Config", "Emulate touch screen (no mouse move)", 0, QApplication::UnicodeUTF8));
    lcdScreen->setText(QApplication::translate("Config", "Emulate LCD screen (Only with fixed zoom of 3.0 times magnification)", 0, QApplication::UnicodeUTF8));
    TextLabel1->setText(QApplication::translate("Config", "<p>Note that any applications using the virtual framebuffer will be terminated if you change the Size or Depth <i>above</i>. You may freely modify the Gamma <i>below</i>.", 0, QApplication::UnicodeUTF8));
    GroupBox1->setTitle(QApplication::translate("Config", "Gamma", 0, QApplication::UnicodeUTF8));
    TextLabel3->setText(QApplication::translate("Config", "Blue", 0, QApplication::UnicodeUTF8));
    blabel->setText(QApplication::translate("Config", "1.0", 0, QApplication::UnicodeUTF8));
    TextLabel2->setText(QApplication::translate("Config", "Green", 0, QApplication::UnicodeUTF8));
    glabel->setText(QApplication::translate("Config", "1.0", 0, QApplication::UnicodeUTF8));
    TextLabel7->setText(QApplication::translate("Config", "All", 0, QApplication::UnicodeUTF8));
    TextLabel8->setText(QApplication::translate("Config", "1.0", 0, QApplication::UnicodeUTF8));
    TextLabel1_2->setText(QApplication::translate("Config", "Red", 0, QApplication::UnicodeUTF8));
    rlabel->setText(QApplication::translate("Config", "1.0", 0, QApplication::UnicodeUTF8));
    PushButton3->setText(QApplication::translate("Config", "Set all to 1.0", 0, QApplication::UnicodeUTF8));
    buttonOk->setText(QApplication::translate("Config", "&OK", 0, QApplication::UnicodeUTF8));
    buttonCancel->setText(QApplication::translate("Config", "&Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Config);
    } // retranslateUi


protected:
    enum IconID
    {
        image0_ID,
        unknown_ID
    };
    static QPixmap icon(IconID id)
    {
    static const unsigned char image0_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xc7, 0x49, 0x44, 0x41, 0x54, 0x18, 0x95, 0xad, 0x55, 0xdb, 0x11, 0x84,
    0x20, 0x0c, 0x4c, 0x18, 0x0b, 0xb0, 0x05, 0xea, 0xb1, 0xcd, 0x6b, 0x81,
    0x12, 0xb4, 0x0d, 0x4b, 0xf1, 0xbe, 0xe2, 0x48, 0x6e, 0xf3, 0x00, 0x6f,
    0xbf, 0x1c, 0x93, 0xec, 0xe6, 0x05, 0xf0, 0x7e, 0xec, 0xe4, 0xe1, 0xa4,
    0xed, 0x42, 0xff, 0x2b, 0x35, 0xf6, 0xe2, 0x96, 0x11, 0x32, 0xe4, 0x63,
    0x09, 0x94, 0x19, 0x52, 0xed, 0x8f, 0x62, 0x8a, 0x76, 0x1a, 0x21, 0xf5,
    0x62, 0x17, 0xcb, 0x20, 0xb0, 0x4a, 0x45, 0xfe, 0x27, 0x6d, 0x97, 0xf8,
    0xff, 0xb4, 0x22, 0x43, 0x1a, 0xd9, 0x6e, 0x62, 0xa4, 0xee, 0x05, 0x7a,
    0x83, 0x13, 0x1b, 0xcc, 0x38, 0x43, 0x1a, 0x6d, 0x05, 0x7f, 0x8e, 0xb5,
    0xcb, 0x36, 0x43, 0x8a, 0xfc, 0xb5, 0xcd, 0xed, 0x71, 0x96, 0x14, 0x01,
    0x1e, 0x90, 0x88, 0x30, 0x83, 0x74, 0xc6, 0x1a, 0xa9, 0xad, 0xb0, 0xe0,
    0x65, 0x1b, 0x55, 0x52, 0xb4, 0xb2, 0x9e, 0xfa, 0xac, 0xb0, 0x99, 0x31,
    0x2a, 0xd5, 0xda, 0x5b, 0x2d, 0x50, 0xa9, 0x31, 0x24, 0x46, 0x3b, 0x2a,
    0xdf, 0x51, 0x6f, 0xbb, 0x8c, 0xbd, 0x13, 0x54, 0xa9, 0xb1, 0xb6, 0x67,
    0xc8, 0xa7, 0x87, 0x67, 0x41, 0x44, 0xf9, 0xf9, 0x82, 0xbc, 0xb9, 0x36,
    0x35, 0xba, 0x8c, 0xb3, 0xfd, 0x1b, 0x26, 0x16, 0xf2, 0x51, 0x01, 0x34,
    0x03, 0xfe, 0xe7, 0x63, 0xfa, 0xbc, 0xe8, 0xbf, 0xf0, 0xaa, 0x6c, 0x98,
    0x11, 0x45, 0x2e, 0x15, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

    switch (id) {
        case image0_ID:  { QImage img; img.loadFromData(image0_data, sizeof(image0_data), "PNG"); return QPixmap::fromImage(img); }
        default: return QPixmap();
    } // switch
    } // icon

};

namespace Ui {
    class Config: public Ui_Config {};
} // namespace Ui

#endif // CONFIG_H
