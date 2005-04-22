#ifndef MAINWINDOWBASE_H
#define MAINWINDOWBASE_H

#include <qvariant.h>

class ColorButton;
class PreviewFrame;

#include <Qt3Support/Q3Frame>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3ListBox>
#include <Qt3Support/Q3MainWindow>
#include <Qt3Support/Q3TextView>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "colorbutton.h"
#include "previewframe.h"

class Ui_MainWindowBase
{
public:
    QAction *fileSaveAction;
    QAction *fileExitAction;
    QAction *helpAboutAction;
    QAction *helpAboutQtAction;
    QWidget *widget;
    QHBoxLayout *hboxLayout;
    Q3TextView *helpview;
    QTabWidget *TabWidget3;
    QWidget *tab;
    QVBoxLayout *vboxLayout;
    Q3GroupBox *GroupBox40;
    QHBoxLayout *hboxLayout1;
    QLabel *gstylebuddy;
    QComboBox *gstylecombo;
    Q3GroupBox *groupAutoPalette;
    QHBoxLayout *hboxLayout2;
    QLabel *labelMainColor;
    ColorButton *buttonMainColor;
    QLabel *labelMainColor2;
    ColorButton *buttonMainColor2;
    QSpacerItem *spacerItem;
    QPushButton *btnAdvanced;
    Q3GroupBox *GroupBox126;
    QGridLayout *gridLayout;
    QLabel *TextLabel1;
    QComboBox *paletteCombo;
    PreviewFrame *previewFrame;
    QWidget *tab1;
    QVBoxLayout *vboxLayout1;
    Q3GroupBox *GroupBox1;
    QGridLayout *gridLayout1;
    QComboBox *stylecombo;
    QComboBox *familycombo;
    QComboBox *psizecombo;
    QLabel *stylebuddy;
    QLabel *psizebuddy;
    QLabel *familybuddy;
    QLineEdit *samplelineedit;
    Q3GroupBox *GroupBox2;
    QVBoxLayout *vboxLayout2;
    QHBoxLayout *hboxLayout3;
    QLabel *famsubbuddy;
    QComboBox *familysubcombo;
    QFrame *Line1;
    QLabel *TextLabel5;
    Q3ListBox *sublistbox;
    QHBoxLayout *hboxLayout4;
    QPushButton *PushButton2;
    QPushButton *PushButton3;
    QPushButton *PushButton4;
    QFrame *Line2;
    QHBoxLayout *hboxLayout5;
    QLabel *choosebuddy;
    QComboBox *choosesubcombo;
    QPushButton *PushButton1;
    QWidget *tab2;
    QVBoxLayout *vboxLayout3;
    Q3GroupBox *GroupBox4;
    QGridLayout *gridLayout2;
    QSpinBox *dcispin;
    QLabel *dcibuddy;
    QSpinBox *cfispin;
    QLabel *cfibuddy;
    QSpinBox *wslspin;
    QLabel *wslbuddy;
    QCheckBox *resolvelinks;
    Q3GroupBox *GroupBox3;
    QVBoxLayout *vboxLayout4;
    QCheckBox *effectcheckbox;
    Q3Frame *effectbase;
    QGridLayout *gridLayout3;
    QLabel *meffectbuddy;
    QLabel *ceffectbuddy;
    QLabel *teffectbuddy;
    QLabel *beffectbuddy;
    QComboBox *menueffect;
    QComboBox *comboeffect;
    QComboBox *tooltipeffect;
    QComboBox *toolboxeffect;
    Q3GroupBox *GroupBox5;
    QGridLayout *gridLayout4;
    QLabel *swbuddy;
    QLabel *shbuddy;
    QSpinBox *strutwidth;
    QSpinBox *strutheight;
    QCheckBox *rtlExtensions;
    QLabel *inputStyleLabel;
    QComboBox *inputStyle;
    QSpacerItem *spacerItem1;
    QWidget *tab3;
    QVBoxLayout *vboxLayout5;
    Q3GroupBox *GroupBox39;
    QVBoxLayout *vboxLayout6;
    QGridLayout *gridLayout5;
    Q3ListBox *libpathlistbox;
    QPushButton *PushButton8;
    QPushButton *PushButton9;
    QPushButton *PushButton7;
    QGridLayout *gridLayout6;
    QSpacerItem *spacerItem2;
    QLabel *TextLabel15;
    QLineEdit *libpathlineedit;
    QPushButton *PushButton5;
    QPushButton *PushButton6;
    QWidget *tab4;
    QVBoxLayout *vboxLayout7;
    QCheckBox *fontembeddingcheckbox;
    Q3GroupBox *GroupBox10;
    QVBoxLayout *vboxLayout8;
    QGridLayout *gridLayout7;
    QPushButton *PushButton11;
    QPushButton *PushButton13;
    Q3ListBox *fontpathlistbox;
    QPushButton *PushButton12;
    QGridLayout *gridLayout8;
    QSpacerItem *spacerItem3;
    QLabel *TextLabel15_2;
    QPushButton *PushButton15;
    QLineEdit *fontpathlineedit;
    QPushButton *PushButton14;
    QMenuBar *menubar;
    QMenu *PopupMenu;
    QMenu *PopupMenu_2;

    void setupUi(Q3MainWindow *MainWindowBase)
    {
    MainWindowBase->setObjectName(QString::fromUtf8("MainWindowBase"));
    MainWindowBase->resize(QSize(700, 600).expandedTo(MainWindowBase->minimumSizeHint()));
    MainWindowBase->setWindowIcon(QPixmap(QString::fromUtf8("appicon.png")));
    fileSaveAction = new QAction(MainWindowBase);
    fileSaveAction->setObjectName(QString::fromUtf8("fileSaveAction"));
    fileSaveAction->setIcon(QPixmap());
    fileExitAction = new QAction(MainWindowBase);
    fileExitAction->setObjectName(QString::fromUtf8("fileExitAction"));
    helpAboutAction = new QAction(MainWindowBase);
    helpAboutAction->setObjectName(QString::fromUtf8("helpAboutAction"));
    helpAboutQtAction = new QAction(MainWindowBase);
    helpAboutQtAction->setObjectName(QString::fromUtf8("helpAboutQtAction"));
    widget = new QWidget(MainWindowBase);
    widget->setObjectName(QString::fromUtf8("widget"));
    hboxLayout = new QHBoxLayout(widget);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    hboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout->setMargin(8);
    hboxLayout->setSpacing(4);
    helpview = new Q3TextView(widget);
    helpview->setObjectName(QString::fromUtf8("helpview"));
    helpview->setMinimumSize(QSize(200, 0));

    hboxLayout->addWidget(helpview);

    TabWidget3 = new QTabWidget(widget);
    TabWidget3->setObjectName(QString::fromUtf8("TabWidget3"));
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    vboxLayout = new QVBoxLayout(tab);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout->setMargin(4);
    vboxLayout->setSpacing(4);
    GroupBox40 = new Q3GroupBox(tab);
    GroupBox40->setObjectName(QString::fromUtf8("GroupBox40"));
    GroupBox40->setColumnLayout(0, Qt::Vertical);
    GroupBox40->layout()->setSpacing(4);
    GroupBox40->layout()->setMargin(8);
    hboxLayout1 = new QHBoxLayout(GroupBox40->layout());
    hboxLayout1->setAlignment(Qt::AlignTop);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout1->setMargin(8);
    hboxLayout1->setSpacing(4);
    gstylebuddy = new QLabel(GroupBox40);
    gstylebuddy->setObjectName(QString::fromUtf8("gstylebuddy"));

    hboxLayout1->addWidget(gstylebuddy);

    gstylecombo = new QComboBox(GroupBox40);
    gstylecombo->setObjectName(QString::fromUtf8("gstylecombo"));

    hboxLayout1->addWidget(gstylecombo);


    vboxLayout->addWidget(GroupBox40);

    groupAutoPalette = new Q3GroupBox(tab);
    groupAutoPalette->setObjectName(QString::fromUtf8("groupAutoPalette"));
    QSizePolicy sizePolicy((QSizePolicy::Policy)5, (QSizePolicy::Policy)4);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupAutoPalette->sizePolicy().hasHeightForWidth());
    groupAutoPalette->setSizePolicy(sizePolicy);
    groupAutoPalette->setAlignment(Qt::AlignAuto);
    groupAutoPalette->setColumnLayout(0, Qt::Vertical);
    groupAutoPalette->layout()->setSpacing(4);
    groupAutoPalette->layout()->setMargin(8);
    hboxLayout2 = new QHBoxLayout(groupAutoPalette->layout());
    hboxLayout2->setAlignment(Qt::AlignTop);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    hboxLayout2->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout2->setMargin(8);
    hboxLayout2->setSpacing(4);
    labelMainColor = new QLabel(groupAutoPalette);
    labelMainColor->setObjectName(QString::fromUtf8("labelMainColor"));

    hboxLayout2->addWidget(labelMainColor);

    buttonMainColor = new ColorButton(groupAutoPalette);
    buttonMainColor->setObjectName(QString::fromUtf8("buttonMainColor"));

    hboxLayout2->addWidget(buttonMainColor);

    labelMainColor2 = new QLabel(groupAutoPalette);
    labelMainColor2->setObjectName(QString::fromUtf8("labelMainColor2"));
    QSizePolicy sizePolicy1((QSizePolicy::Policy)1, (QSizePolicy::Policy)1);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(labelMainColor2->sizePolicy().hasHeightForWidth());
    labelMainColor2->setSizePolicy(sizePolicy1);
    labelMainColor2->setMinimumSize(QSize(50, 0));
    labelMainColor2->setLineWidth(1);
    labelMainColor2->setMargin(0);
    labelMainColor2->setMidLineWidth(0);
    labelMainColor2->setAlignment(Qt::AlignVCenter);

    hboxLayout2->addWidget(labelMainColor2);

    buttonMainColor2 = new ColorButton(groupAutoPalette);
    buttonMainColor2->setObjectName(QString::fromUtf8("buttonMainColor2"));

    hboxLayout2->addWidget(buttonMainColor2);

    spacerItem = new QSpacerItem(70, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem);

    btnAdvanced = new QPushButton(groupAutoPalette);
    btnAdvanced->setObjectName(QString::fromUtf8("btnAdvanced"));

    hboxLayout2->addWidget(btnAdvanced);


    vboxLayout->addWidget(groupAutoPalette);

    GroupBox126 = new Q3GroupBox(tab);
    GroupBox126->setObjectName(QString::fromUtf8("GroupBox126"));
    QSizePolicy sizePolicy2((QSizePolicy::Policy)5, (QSizePolicy::Policy)7);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(GroupBox126->sizePolicy().hasHeightForWidth());
    GroupBox126->setSizePolicy(sizePolicy2);
    GroupBox126->setColumnLayout(0, Qt::Vertical);
    GroupBox126->layout()->setSpacing(4);
    GroupBox126->layout()->setMargin(8);
    gridLayout = new QGridLayout(GroupBox126->layout());
    gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout->setMargin(8);
    gridLayout->setSpacing(4);
    TextLabel1 = new QLabel(GroupBox126);
    TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));

    gridLayout->addWidget(TextLabel1, 0, 0, 1, 1);

    paletteCombo = new QComboBox(GroupBox126);
    paletteCombo->setObjectName(QString::fromUtf8("paletteCombo"));

    gridLayout->addWidget(paletteCombo, 0, 1, 1, 1);

    previewFrame = new PreviewFrame(GroupBox126);
    previewFrame->setObjectName(QString::fromUtf8("previewFrame"));
    QSizePolicy sizePolicy3((QSizePolicy::Policy)7, (QSizePolicy::Policy)7);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(previewFrame->sizePolicy().hasHeightForWidth());
    previewFrame->setSizePolicy(sizePolicy3);
    previewFrame->setMinimumSize(QSize(410, 260));

    gridLayout->addWidget(previewFrame, 1, 0, 1, 2);


    vboxLayout->addWidget(GroupBox126);

    TabWidget3->addTab(tab, QApplication::translate("MainWindowBase", "Appearance"));
    tab1 = new QWidget();
    tab1->setObjectName(QString::fromUtf8("tab1"));
    tab1->setObjectName(QString::fromUtf8("tab"));
    vboxLayout1 = new QVBoxLayout(tab1);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    vboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout1->setMargin(8);
    vboxLayout1->setSpacing(4);
    GroupBox1 = new Q3GroupBox(tab1);
    GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
    GroupBox1->setColumnLayout(0, Qt::Vertical);
    GroupBox1->layout()->setSpacing(4);
    GroupBox1->layout()->setMargin(8);
    gridLayout1 = new QGridLayout(GroupBox1->layout());
    gridLayout1->setAlignment(Qt::AlignTop);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    gridLayout1->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout1->setMargin(8);
    gridLayout1->setSpacing(4);
    stylecombo = new QComboBox(GroupBox1);
    stylecombo->setObjectName(QString::fromUtf8("stylecombo"));
    stylecombo->setAutoCompletion(true);
    stylecombo->setDuplicatesEnabled(false);

    gridLayout1->addWidget(stylecombo, 1, 1, 1, 1);

    familycombo = new QComboBox(GroupBox1);
    familycombo->setObjectName(QString::fromUtf8("familycombo"));
    familycombo->setAutoCompletion(true);
    familycombo->setDuplicatesEnabled(false);

    gridLayout1->addWidget(familycombo, 0, 1, 1, 1);

    psizecombo = new QComboBox(GroupBox1);
    psizecombo->setObjectName(QString::fromUtf8("psizecombo"));
    psizecombo->setEditable(true);
    psizecombo->setAutoCompletion(true);
    psizecombo->setDuplicatesEnabled(false);

    gridLayout1->addWidget(psizecombo, 2, 1, 1, 1);

    stylebuddy = new QLabel(GroupBox1);
    stylebuddy->setObjectName(QString::fromUtf8("stylebuddy"));

    gridLayout1->addWidget(stylebuddy, 1, 0, 1, 1);

    psizebuddy = new QLabel(GroupBox1);
    psizebuddy->setObjectName(QString::fromUtf8("psizebuddy"));

    gridLayout1->addWidget(psizebuddy, 2, 0, 1, 1);

    familybuddy = new QLabel(GroupBox1);
    familybuddy->setObjectName(QString::fromUtf8("familybuddy"));

    gridLayout1->addWidget(familybuddy, 0, 0, 1, 1);

    samplelineedit = new QLineEdit(GroupBox1);
    samplelineedit->setObjectName(QString::fromUtf8("samplelineedit"));
    samplelineedit->setAlignment(Qt::AlignHCenter);

    gridLayout1->addWidget(samplelineedit, 3, 0, 1, 2);


    vboxLayout1->addWidget(GroupBox1);

    GroupBox2 = new Q3GroupBox(tab1);
    GroupBox2->setObjectName(QString::fromUtf8("GroupBox2"));
    GroupBox2->setFrameShape(Q3GroupBox::Box);
    GroupBox2->setFrameShadow(Q3GroupBox::Sunken);
    GroupBox2->setColumnLayout(0, Qt::Vertical);
    GroupBox2->layout()->setSpacing(4);
    GroupBox2->layout()->setMargin(8);
    vboxLayout2 = new QVBoxLayout(GroupBox2->layout());
    vboxLayout2->setAlignment(Qt::AlignTop);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout2->setMargin(8);
    vboxLayout2->setSpacing(4);
    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    hboxLayout3->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout3->setMargin(0);
    hboxLayout3->setSpacing(4);
    famsubbuddy = new QLabel(GroupBox2);
    famsubbuddy->setObjectName(QString::fromUtf8("famsubbuddy"));

    hboxLayout3->addWidget(famsubbuddy);

    familysubcombo = new QComboBox(GroupBox2);
    familysubcombo->setObjectName(QString::fromUtf8("familysubcombo"));
    familysubcombo->setEditable(true);
    familysubcombo->setAutoCompletion(true);
    familysubcombo->setDuplicatesEnabled(false);

    hboxLayout3->addWidget(familysubcombo);


    vboxLayout2->addLayout(hboxLayout3);

    Line1 = new QFrame(GroupBox2);
    Line1->setObjectName(QString::fromUtf8("Line1"));
    Line1->setFrameShape(QFrame::HLine);
    Line1->setFrameShadow(QFrame::Sunken);

    vboxLayout2->addWidget(Line1);

    TextLabel5 = new QLabel(GroupBox2);
    TextLabel5->setObjectName(QString::fromUtf8("TextLabel5"));

    vboxLayout2->addWidget(TextLabel5);

    sublistbox = new Q3ListBox(GroupBox2);
    sublistbox->setObjectName(QString::fromUtf8("sublistbox"));

    vboxLayout2->addWidget(sublistbox);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    hboxLayout4->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout4->setMargin(0);
    hboxLayout4->setSpacing(4);
    PushButton2 = new QPushButton(GroupBox2);
    PushButton2->setObjectName(QString::fromUtf8("PushButton2"));

    hboxLayout4->addWidget(PushButton2);

    PushButton3 = new QPushButton(GroupBox2);
    PushButton3->setObjectName(QString::fromUtf8("PushButton3"));

    hboxLayout4->addWidget(PushButton3);

    PushButton4 = new QPushButton(GroupBox2);
    PushButton4->setObjectName(QString::fromUtf8("PushButton4"));

    hboxLayout4->addWidget(PushButton4);


    vboxLayout2->addLayout(hboxLayout4);

    Line2 = new QFrame(GroupBox2);
    Line2->setObjectName(QString::fromUtf8("Line2"));
    Line2->setFrameShape(QFrame::HLine);
    Line2->setFrameShadow(QFrame::Sunken);

    vboxLayout2->addWidget(Line2);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    hboxLayout5->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout5->setMargin(0);
    hboxLayout5->setSpacing(4);
    choosebuddy = new QLabel(GroupBox2);
    choosebuddy->setObjectName(QString::fromUtf8("choosebuddy"));

    hboxLayout5->addWidget(choosebuddy);

    choosesubcombo = new QComboBox(GroupBox2);
    choosesubcombo->setObjectName(QString::fromUtf8("choosesubcombo"));
    choosesubcombo->setAutoCompletion(true);
    choosesubcombo->setDuplicatesEnabled(false);

    hboxLayout5->addWidget(choosesubcombo);

    PushButton1 = new QPushButton(GroupBox2);
    PushButton1->setObjectName(QString::fromUtf8("PushButton1"));

    hboxLayout5->addWidget(PushButton1);


    vboxLayout2->addLayout(hboxLayout5);


    vboxLayout1->addWidget(GroupBox2);

    TabWidget3->addTab(tab1, QApplication::translate("MainWindowBase", "Fonts"));
    tab2 = new QWidget();
    tab2->setObjectName(QString::fromUtf8("tab2"));
    tab2->setObjectName(QString::fromUtf8("tab"));
    vboxLayout3 = new QVBoxLayout(tab2);
    vboxLayout3->setMargin(8);
    vboxLayout3->setSpacing(4);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    vboxLayout3->setObjectName(QString::fromUtf8("unnamed"));
    GroupBox4 = new Q3GroupBox(tab2);
    GroupBox4->setObjectName(QString::fromUtf8("GroupBox4"));
    GroupBox4->setColumnLayout(0, Qt::Vertical);
    GroupBox4->layout()->setSpacing(4);
    GroupBox4->layout()->setMargin(8);
    gridLayout2 = new QGridLayout(GroupBox4->layout());
    gridLayout2->setAlignment(Qt::AlignTop);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    gridLayout2->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout2->setMargin(8);
    gridLayout2->setSpacing(4);
    dcispin = new QSpinBox(GroupBox4);
    dcispin->setObjectName(QString::fromUtf8("dcispin"));
    dcispin->setMaximum(10000);
    dcispin->setMinimum(10);

    gridLayout2->addWidget(dcispin, 0, 1, 1, 1);

    dcibuddy = new QLabel(GroupBox4);
    dcibuddy->setObjectName(QString::fromUtf8("dcibuddy"));

    gridLayout2->addWidget(dcibuddy, 0, 0, 1, 1);

    cfispin = new QSpinBox(GroupBox4);
    cfispin->setObjectName(QString::fromUtf8("cfispin"));
    cfispin->setMaximum(10000);
    cfispin->setMinimum(9);

    gridLayout2->addWidget(cfispin, 1, 1, 1, 1);

    cfibuddy = new QLabel(GroupBox4);
    cfibuddy->setObjectName(QString::fromUtf8("cfibuddy"));

    gridLayout2->addWidget(cfibuddy, 1, 0, 1, 1);

    wslspin = new QSpinBox(GroupBox4);
    wslspin->setObjectName(QString::fromUtf8("wslspin"));
    wslspin->setMaximum(20);
    wslspin->setMinimum(1);

    gridLayout2->addWidget(wslspin, 2, 1, 1, 1);

    wslbuddy = new QLabel(GroupBox4);
    wslbuddy->setObjectName(QString::fromUtf8("wslbuddy"));

    gridLayout2->addWidget(wslbuddy, 2, 0, 1, 1);

    resolvelinks = new QCheckBox(GroupBox4);
    resolvelinks->setObjectName(QString::fromUtf8("resolvelinks"));

    gridLayout2->addWidget(resolvelinks, 3, 0, 1, 2);


    vboxLayout3->addWidget(GroupBox4);

    GroupBox3 = new Q3GroupBox(tab2);
    GroupBox3->setObjectName(QString::fromUtf8("GroupBox3"));
    GroupBox3->setColumnLayout(0, Qt::Vertical);
    GroupBox3->layout()->setSpacing(4);
    GroupBox3->layout()->setMargin(8);
    vboxLayout4 = new QVBoxLayout(GroupBox3->layout());
    vboxLayout4->setAlignment(Qt::AlignTop);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    vboxLayout4->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout4->setMargin(8);
    vboxLayout4->setSpacing(4);
    effectcheckbox = new QCheckBox(GroupBox3);
    effectcheckbox->setObjectName(QString::fromUtf8("effectcheckbox"));

    vboxLayout4->addWidget(effectcheckbox);

    effectbase = new Q3Frame(GroupBox3);
    effectbase->setObjectName(QString::fromUtf8("effectbase"));
    effectbase->setFrameShape(Q3Frame::NoFrame);
    effectbase->setFrameShadow(Q3Frame::Plain);
    gridLayout3 = new QGridLayout(effectbase);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    gridLayout3->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout3->setMargin(0);
    gridLayout3->setSpacing(4);
    meffectbuddy = new QLabel(effectbase);
    meffectbuddy->setObjectName(QString::fromUtf8("meffectbuddy"));

    gridLayout3->addWidget(meffectbuddy, 0, 0, 1, 1);

    ceffectbuddy = new QLabel(effectbase);
    ceffectbuddy->setObjectName(QString::fromUtf8("ceffectbuddy"));

    gridLayout3->addWidget(ceffectbuddy, 1, 0, 1, 1);

    teffectbuddy = new QLabel(effectbase);
    teffectbuddy->setObjectName(QString::fromUtf8("teffectbuddy"));

    gridLayout3->addWidget(teffectbuddy, 2, 0, 1, 1);

    beffectbuddy = new QLabel(effectbase);
    beffectbuddy->setObjectName(QString::fromUtf8("beffectbuddy"));

    gridLayout3->addWidget(beffectbuddy, 3, 0, 1, 1);

    menueffect = new QComboBox(effectbase);
    menueffect->setObjectName(QString::fromUtf8("menueffect"));
    menueffect->setCurrentIndex(0);
    menueffect->setAutoCompletion(true);

    gridLayout3->addWidget(menueffect, 0, 1, 1, 1);

    comboeffect = new QComboBox(effectbase);
    comboeffect->setObjectName(QString::fromUtf8("comboeffect"));

    gridLayout3->addWidget(comboeffect, 1, 1, 1, 1);

    tooltipeffect = new QComboBox(effectbase);
    tooltipeffect->setObjectName(QString::fromUtf8("tooltipeffect"));

    gridLayout3->addWidget(tooltipeffect, 2, 1, 1, 1);

    toolboxeffect = new QComboBox(effectbase);
    toolboxeffect->setObjectName(QString::fromUtf8("toolboxeffect"));

    gridLayout3->addWidget(toolboxeffect, 3, 1, 1, 1);


    vboxLayout4->addWidget(effectbase);


    vboxLayout3->addWidget(GroupBox3);

    GroupBox5 = new Q3GroupBox(tab2);
    GroupBox5->setObjectName(QString::fromUtf8("GroupBox5"));
    GroupBox5->setColumnLayout(0, Qt::Vertical);
    GroupBox5->layout()->setSpacing(4);
    GroupBox5->layout()->setMargin(8);
    gridLayout4 = new QGridLayout(GroupBox5->layout());
    gridLayout4->setAlignment(Qt::AlignTop);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    gridLayout4->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout4->setMargin(8);
    gridLayout4->setSpacing(4);
    swbuddy = new QLabel(GroupBox5);
    swbuddy->setObjectName(QString::fromUtf8("swbuddy"));

    gridLayout4->addWidget(swbuddy, 0, 0, 1, 1);

    shbuddy = new QLabel(GroupBox5);
    shbuddy->setObjectName(QString::fromUtf8("shbuddy"));

    gridLayout4->addWidget(shbuddy, 1, 0, 1, 1);

    strutwidth = new QSpinBox(GroupBox5);
    strutwidth->setObjectName(QString::fromUtf8("strutwidth"));
    strutwidth->setMaximum(1000);

    gridLayout4->addWidget(strutwidth, 0, 1, 1, 1);

    strutheight = new QSpinBox(GroupBox5);
    strutheight->setObjectName(QString::fromUtf8("strutheight"));
    strutheight->setMaximum(1000);

    gridLayout4->addWidget(strutheight, 1, 1, 1, 1);


    vboxLayout3->addWidget(GroupBox5);

    rtlExtensions = new QCheckBox(tab2);
    rtlExtensions->setObjectName(QString::fromUtf8("rtlExtensions"));

    vboxLayout3->addWidget(rtlExtensions);

    inputStyleLabel = new QLabel(tab2);
    inputStyleLabel->setObjectName(QString::fromUtf8("inputStyleLabel"));

    vboxLayout3->addWidget(inputStyleLabel);

    inputStyle = new QComboBox(tab2);
    inputStyle->setObjectName(QString::fromUtf8("inputStyle"));
    inputStyle->setCurrentIndex(0);

    vboxLayout3->addWidget(inputStyle);

    spacerItem1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout3->addItem(spacerItem1);

    TabWidget3->addTab(tab2, QApplication::translate("MainWindowBase", "Interface"));
    tab3 = new QWidget();
    tab3->setObjectName(QString::fromUtf8("tab3"));
    tab3->setObjectName(QString::fromUtf8("tab"));
    vboxLayout5 = new QVBoxLayout(tab3);
    vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
    vboxLayout5->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout5->setMargin(4);
    vboxLayout5->setSpacing(4);
    GroupBox39 = new Q3GroupBox(tab3);
    GroupBox39->setObjectName(QString::fromUtf8("GroupBox39"));
    GroupBox39->setColumnLayout(0, Qt::Vertical);
    GroupBox39->layout()->setSpacing(4);
    GroupBox39->layout()->setMargin(8);
    vboxLayout6 = new QVBoxLayout(GroupBox39->layout());
    vboxLayout6->setAlignment(Qt::AlignTop);
    vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
    vboxLayout6->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout6->setMargin(8);
    vboxLayout6->setSpacing(4);
    gridLayout5 = new QGridLayout();
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    gridLayout5->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout5->setMargin(0);
    gridLayout5->setSpacing(4);
    libpathlistbox = new Q3ListBox(GroupBox39);
    libpathlistbox->setObjectName(QString::fromUtf8("libpathlistbox"));
    libpathlistbox->setFrameShape(QFrame::StyledPanel);
    libpathlistbox->setFrameShadow(QFrame::Sunken);

    gridLayout5->addWidget(libpathlistbox, 0, 0, 1, 3);

    PushButton8 = new QPushButton(GroupBox39);
    PushButton8->setObjectName(QString::fromUtf8("PushButton8"));

    gridLayout5->addWidget(PushButton8, 1, 1, 1, 1);

    PushButton9 = new QPushButton(GroupBox39);
    PushButton9->setObjectName(QString::fromUtf8("PushButton9"));

    gridLayout5->addWidget(PushButton9, 1, 2, 1, 1);

    PushButton7 = new QPushButton(GroupBox39);
    PushButton7->setObjectName(QString::fromUtf8("PushButton7"));

    gridLayout5->addWidget(PushButton7, 1, 0, 1, 1);


    vboxLayout6->addLayout(gridLayout5);

    gridLayout6 = new QGridLayout();
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
    gridLayout6->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout6->setMargin(0);
    gridLayout6->setSpacing(4);
    spacerItem2 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

    gridLayout6->addItem(spacerItem2, 2, 0, 1, 1);

    TextLabel15 = new QLabel(GroupBox39);
    TextLabel15->setObjectName(QString::fromUtf8("TextLabel15"));

    gridLayout6->addWidget(TextLabel15, 0, 0, 1, 3);

    libpathlineedit = new QLineEdit(GroupBox39);
    libpathlineedit->setObjectName(QString::fromUtf8("libpathlineedit"));

    gridLayout6->addWidget(libpathlineedit, 1, 0, 1, 3);

    PushButton5 = new QPushButton(GroupBox39);
    PushButton5->setObjectName(QString::fromUtf8("PushButton5"));

    gridLayout6->addWidget(PushButton5, 2, 1, 1, 1);

    PushButton6 = new QPushButton(GroupBox39);
    PushButton6->setObjectName(QString::fromUtf8("PushButton6"));

    gridLayout6->addWidget(PushButton6, 2, 2, 1, 1);


    vboxLayout6->addLayout(gridLayout6);


    vboxLayout5->addWidget(GroupBox39);

    TabWidget3->addTab(tab3, QApplication::translate("MainWindowBase", "Library Paths"));
    tab4 = new QWidget();
    tab4->setObjectName(QString::fromUtf8("tab4"));
    tab4->setObjectName(QString::fromUtf8("tab"));
    vboxLayout7 = new QVBoxLayout(tab4);
    vboxLayout7->setObjectName(QString::fromUtf8("vboxLayout7"));
    vboxLayout7->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout7->setMargin(8);
    vboxLayout7->setSpacing(4);
    fontembeddingcheckbox = new QCheckBox(tab4);
    fontembeddingcheckbox->setObjectName(QString::fromUtf8("fontembeddingcheckbox"));
    fontembeddingcheckbox->setChecked(true);

    vboxLayout7->addWidget(fontembeddingcheckbox);

    GroupBox10 = new Q3GroupBox(tab4);
    GroupBox10->setObjectName(QString::fromUtf8("GroupBox10"));
    QSizePolicy sizePolicy4((QSizePolicy::Policy)5, (QSizePolicy::Policy)7);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(GroupBox10->sizePolicy().hasHeightForWidth());
    GroupBox10->setSizePolicy(sizePolicy4);
    GroupBox10->setColumnLayout(0, Qt::Vertical);
    GroupBox10->layout()->setSpacing(4);
    GroupBox10->layout()->setMargin(8);
    vboxLayout8 = new QVBoxLayout(GroupBox10->layout());
    vboxLayout8->setAlignment(Qt::AlignTop);
    vboxLayout8->setObjectName(QString::fromUtf8("vboxLayout8"));
    vboxLayout8->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout8->setMargin(8);
    vboxLayout8->setSpacing(4);
    gridLayout7 = new QGridLayout();
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
    gridLayout7->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout7->setMargin(0);
    gridLayout7->setSpacing(4);
    PushButton11 = new QPushButton(GroupBox10);
    PushButton11->setObjectName(QString::fromUtf8("PushButton11"));

    gridLayout7->addWidget(PushButton11, 1, 0, 1, 1);

    PushButton13 = new QPushButton(GroupBox10);
    PushButton13->setObjectName(QString::fromUtf8("PushButton13"));

    gridLayout7->addWidget(PushButton13, 1, 2, 1, 1);

    fontpathlistbox = new Q3ListBox(GroupBox10);
    fontpathlistbox->setObjectName(QString::fromUtf8("fontpathlistbox"));

    gridLayout7->addWidget(fontpathlistbox, 0, 0, 1, 3);

    PushButton12 = new QPushButton(GroupBox10);
    PushButton12->setObjectName(QString::fromUtf8("PushButton12"));

    gridLayout7->addWidget(PushButton12, 1, 1, 1, 1);


    vboxLayout8->addLayout(gridLayout7);

    gridLayout8 = new QGridLayout();
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));
    gridLayout8->setObjectName(QString::fromUtf8("unnamed"));
    gridLayout8->setMargin(0);
    gridLayout8->setSpacing(4);
    spacerItem3 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

    gridLayout8->addItem(spacerItem3, 2, 0, 1, 1);

    TextLabel15_2 = new QLabel(GroupBox10);
    TextLabel15_2->setObjectName(QString::fromUtf8("TextLabel15_2"));

    gridLayout8->addWidget(TextLabel15_2, 0, 0, 1, 3);

    PushButton15 = new QPushButton(GroupBox10);
    PushButton15->setObjectName(QString::fromUtf8("PushButton15"));

    gridLayout8->addWidget(PushButton15, 2, 2, 1, 1);

    fontpathlineedit = new QLineEdit(GroupBox10);
    fontpathlineedit->setObjectName(QString::fromUtf8("fontpathlineedit"));

    gridLayout8->addWidget(fontpathlineedit, 1, 0, 1, 3);

    PushButton14 = new QPushButton(GroupBox10);
    PushButton14->setObjectName(QString::fromUtf8("PushButton14"));

    gridLayout8->addWidget(PushButton14, 2, 1, 1, 1);


    vboxLayout8->addLayout(gridLayout8);


    vboxLayout7->addWidget(GroupBox10);

    TabWidget3->addTab(tab4, QApplication::translate("MainWindowBase", "Printer"));

    hboxLayout->addWidget(TabWidget3);

    MainWindowBase->setCentralWidget(widget);
    menubar = new QMenuBar(MainWindowBase);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    PopupMenu = new QMenu(menubar);
    PopupMenu->setObjectName(QString::fromUtf8("PopupMenu"));
    PopupMenu_2 = new QMenu(menubar);
    PopupMenu_2->setObjectName(QString::fromUtf8("PopupMenu_2"));
    gstylebuddy->setBuddy(gstylecombo);
    labelMainColor->setBuddy(buttonMainColor);
    labelMainColor2->setBuddy(buttonMainColor2);
    TextLabel1->setBuddy(paletteCombo);
    stylebuddy->setBuddy(stylecombo);
    psizebuddy->setBuddy(psizecombo);
    familybuddy->setBuddy(familycombo);
    famsubbuddy->setBuddy(familysubcombo);
    choosebuddy->setBuddy(choosesubcombo);
    dcibuddy->setBuddy(dcispin);
    cfibuddy->setBuddy(cfispin);
    wslbuddy->setBuddy(wslspin);
    meffectbuddy->setBuddy(menueffect);
    ceffectbuddy->setBuddy(comboeffect);
    teffectbuddy->setBuddy(tooltipeffect);
    beffectbuddy->setBuddy(toolboxeffect);
    swbuddy->setBuddy(strutwidth);
    shbuddy->setBuddy(strutheight);
    QWidget::setTabOrder(helpview, TabWidget3);
    QWidget::setTabOrder(TabWidget3, familycombo);
    QWidget::setTabOrder(familycombo, stylecombo);
    QWidget::setTabOrder(stylecombo, psizecombo);
    QWidget::setTabOrder(psizecombo, samplelineedit);
    QWidget::setTabOrder(samplelineedit, familysubcombo);
    QWidget::setTabOrder(familysubcombo, PushButton2);
    QWidget::setTabOrder(PushButton2, PushButton3);
    QWidget::setTabOrder(PushButton3, PushButton4);
    QWidget::setTabOrder(PushButton4, choosesubcombo);
    QWidget::setTabOrder(choosesubcombo, PushButton1);
    QWidget::setTabOrder(PushButton1, dcispin);
    QWidget::setTabOrder(dcispin, cfispin);
    QWidget::setTabOrder(cfispin, wslspin);
    QWidget::setTabOrder(wslspin, effectcheckbox);
    QWidget::setTabOrder(effectcheckbox, menueffect);
    QWidget::setTabOrder(menueffect, comboeffect);
    QWidget::setTabOrder(comboeffect, tooltipeffect);
    QWidget::setTabOrder(tooltipeffect, strutwidth);
    QWidget::setTabOrder(strutwidth, strutheight);
    QWidget::setTabOrder(strutheight, libpathlineedit);
    QWidget::setTabOrder(libpathlineedit, PushButton5);
    QWidget::setTabOrder(PushButton5, PushButton6);
    QWidget::setTabOrder(PushButton6, PushButton7);
    QWidget::setTabOrder(PushButton7, PushButton8);
    QWidget::setTabOrder(PushButton8, PushButton9);
    QWidget::setTabOrder(PushButton9, sublistbox);
    QWidget::setTabOrder(sublistbox, libpathlistbox);

    menubar->addAction(PopupMenu->menuAction());
    menubar->addSeparator();
    menubar->addAction(PopupMenu_2->menuAction());
    PopupMenu->addAction(fileSaveAction);
    PopupMenu->addSeparator();
    PopupMenu->addAction(fileExitAction);
    PopupMenu_2->addAction(helpAboutAction);
    PopupMenu_2->addAction(helpAboutQtAction);
    retranslateUi(MainWindowBase);

    QMetaObject::connectSlotsByName(MainWindowBase);
    } // setupUi

    void retranslateUi(Q3MainWindow *MainWindowBase)
    {
    MainWindowBase->setWindowTitle(QApplication::translate("MainWindowBase", "Qt Configuration"));
    fileSaveAction->setIconText(QApplication::translate("MainWindowBase", "Save"));
    fileSaveAction->setText(QApplication::translate("MainWindowBase", "&Save"));
    fileSaveAction->setShortcut(QApplication::translate("MainWindowBase", "Ctrl+S"));
    fileExitAction->setIconText(QApplication::translate("MainWindowBase", "Exit"));
    fileExitAction->setText(QApplication::translate("MainWindowBase", "E&xit"));
    fileExitAction->setShortcut(QApplication::translate("MainWindowBase", ""));
    helpAboutAction->setIconText(QApplication::translate("MainWindowBase", "About"));
    helpAboutAction->setText(QApplication::translate("MainWindowBase", "&About"));
    helpAboutAction->setShortcut(QApplication::translate("MainWindowBase", ""));
    helpAboutQtAction->setIconText(QApplication::translate("MainWindowBase", "About Qt"));
    helpAboutQtAction->setText(QApplication::translate("MainWindowBase", "About &Qt"));
    helpview->setText(QApplication::translate("MainWindowBase", "<p align=center><b>Documentation</b></p>\n"
"<p>This QTextView will display a brief explanation about the current page, so that the user isn't confused about the settings he/she is twiddling.</p>"));
    GroupBox40->setTitle(QApplication::translate("MainWindowBase", "GUI Style"));
    gstylebuddy->setText(QApplication::translate("MainWindowBase", "Select GUI &Style:"));
    groupAutoPalette->setTitle(QApplication::translate("MainWindowBase", "Build Palette"));
    labelMainColor->setText(QApplication::translate("MainWindowBase", "&3-D Effects:"));
    labelMainColor2->setText(QApplication::translate("MainWindowBase", "Back&ground:"));
    btnAdvanced->setText(QApplication::translate("MainWindowBase", "&Tune Palette..."));
    GroupBox126->setTitle(QApplication::translate("MainWindowBase", "Preview"));
    TextLabel1->setText(QApplication::translate("MainWindowBase", "Select &Palette:"));
    paletteCombo->addItem(QApplication::translate("MainWindowBase", "Active Palette"));
    paletteCombo->addItem(QApplication::translate("MainWindowBase", "Inactive Palette"));
    paletteCombo->addItem(QApplication::translate("MainWindowBase", "Disabled Palette"));
    TabWidget3->setTabText(TabWidget3->indexOf(tab), QApplication::translate("MainWindowBase", "Appearance"));
    GroupBox1->setTitle(QApplication::translate("MainWindowBase", "Default Font"));
    stylebuddy->setText(QApplication::translate("MainWindowBase", "&Style:"));
    psizebuddy->setText(QApplication::translate("MainWindowBase", "&Point Size:"));
    familybuddy->setText(QApplication::translate("MainWindowBase", "F&amily:"));
    samplelineedit->setText(QApplication::translate("MainWindowBase", "Sample Text"));
    GroupBox2->setTitle(QApplication::translate("MainWindowBase", "Font Substitution"));
    famsubbuddy->setText(QApplication::translate("MainWindowBase", "S&elect or Enter a Family:"));
    TextLabel5->setText(QApplication::translate("MainWindowBase", "Current Substitutions:"));
    PushButton2->setText(QApplication::translate("MainWindowBase", "Up"));
    PushButton3->setText(QApplication::translate("MainWindowBase", "Down"));
    PushButton4->setText(QApplication::translate("MainWindowBase", "Remove"));
    choosebuddy->setText(QApplication::translate("MainWindowBase", "Select s&ubstitute Family:"));
    PushButton1->setText(QApplication::translate("MainWindowBase", "Add"));
    TabWidget3->setTabText(TabWidget3->indexOf(tab1), QApplication::translate("MainWindowBase", "Fonts"));
    GroupBox4->setTitle(QApplication::translate("MainWindowBase", "Feel Settings"));
    dcispin->setSuffix(QApplication::translate("MainWindowBase", " ms"));
    dcibuddy->setText(QApplication::translate("MainWindowBase", "&Double Click Interval:"));
    cfispin->setSuffix(QApplication::translate("MainWindowBase", " ms"));
    cfispin->setSpecialValueText(QApplication::translate("MainWindowBase", "No blinking"));
    cfibuddy->setText(QApplication::translate("MainWindowBase", "&Cursor Flash Time:"));
    wslspin->setSuffix(QApplication::translate("MainWindowBase", " lines"));
    wslbuddy->setText(QApplication::translate("MainWindowBase", "Wheel &Scroll Lines:"));
    resolvelinks->setText(QApplication::translate("MainWindowBase", "Resolve symlinks in URLs"));
    GroupBox3->setTitle(QApplication::translate("MainWindowBase", "GUI Effects"));
    effectcheckbox->setText(QApplication::translate("MainWindowBase", "&Enable"));
    effectcheckbox->setShortcut(QApplication::translate("MainWindowBase", "Alt+E"));
    meffectbuddy->setText(QApplication::translate("MainWindowBase", "&Menu Effect:"));
    ceffectbuddy->setText(QApplication::translate("MainWindowBase", "C&omboBox Effect:"));
    teffectbuddy->setText(QApplication::translate("MainWindowBase", "&ToolTip Effect:"));
    beffectbuddy->setText(QApplication::translate("MainWindowBase", "Tool&Box Effect:"));
    menueffect->addItem(QApplication::translate("MainWindowBase", "Disable"));
    menueffect->addItem(QApplication::translate("MainWindowBase", "Animate"));
    menueffect->addItem(QApplication::translate("MainWindowBase", "Fade"));
    comboeffect->addItem(QApplication::translate("MainWindowBase", "Disable"));
    comboeffect->addItem(QApplication::translate("MainWindowBase", "Animate"));
    tooltipeffect->addItem(QApplication::translate("MainWindowBase", "Disable"));
    tooltipeffect->addItem(QApplication::translate("MainWindowBase", "Animate"));
    tooltipeffect->addItem(QApplication::translate("MainWindowBase", "Fade"));
    toolboxeffect->addItem(QApplication::translate("MainWindowBase", "Disable"));
    toolboxeffect->addItem(QApplication::translate("MainWindowBase", "Animate"));
    GroupBox5->setTitle(QApplication::translate("MainWindowBase", "Global Strut"));
    swbuddy->setText(QApplication::translate("MainWindowBase", "Minimum &Width:"));
    shbuddy->setText(QApplication::translate("MainWindowBase", "Minimum Hei&ght:"));
    strutwidth->setSuffix(QApplication::translate("MainWindowBase", " pixels"));
    strutheight->setSuffix(QApplication::translate("MainWindowBase", " pixels"));
    rtlExtensions->setText(QApplication::translate("MainWindowBase", "Enhanced support for languages written right-to-left"));
    inputStyleLabel->setText(QApplication::translate("MainWindowBase", "XIM Input Style:"));
    inputStyle->addItem(QApplication::translate("MainWindowBase", "On The Spot"));
    inputStyle->addItem(QApplication::translate("MainWindowBase", "Over The Spot"));
    inputStyle->addItem(QApplication::translate("MainWindowBase", "Off The Spot"));
    inputStyle->addItem(QApplication::translate("MainWindowBase", "Root"));
    TabWidget3->setTabText(TabWidget3->indexOf(tab2), QApplication::translate("MainWindowBase", "Interface"));
    GroupBox39->setTitle(QApplication::translate("MainWindowBase", "Library Paths"));
    PushButton8->setText(QApplication::translate("MainWindowBase", "Down"));
    PushButton9->setText(QApplication::translate("MainWindowBase", "Remove"));
    PushButton7->setText(QApplication::translate("MainWindowBase", "Up"));
    TextLabel15->setText(QApplication::translate("MainWindowBase", "Press the <b>Browse</b> button or enter a directory and press Enter to add them to the list."));
    PushButton5->setText(QApplication::translate("MainWindowBase", "Browse..."));
    PushButton6->setText(QApplication::translate("MainWindowBase", "Add"));
    TabWidget3->setTabText(TabWidget3->indexOf(tab3), QApplication::translate("MainWindowBase", "Library Paths"));
    fontembeddingcheckbox->setText(QApplication::translate("MainWindowBase", "Enable Font embedding"));
    GroupBox10->setTitle(QApplication::translate("MainWindowBase", "Font Paths"));
    PushButton11->setText(QApplication::translate("MainWindowBase", "Up"));
    PushButton13->setText(QApplication::translate("MainWindowBase", "Remove"));
    PushButton12->setText(QApplication::translate("MainWindowBase", "Down"));
    TextLabel15_2->setText(QApplication::translate("MainWindowBase", "Press the <b>Browse</b> button or enter a directory and press Enter to add them to the list."));
    PushButton15->setText(QApplication::translate("MainWindowBase", "Add"));
    PushButton14->setText(QApplication::translate("MainWindowBase", "Browse..."));
    TabWidget3->setTabText(TabWidget3->indexOf(tab4), QApplication::translate("MainWindowBase", "Printer"));
    PopupMenu->setTitle(QApplication::translate("MainWindowBase", "&File"));
    PopupMenu_2->setTitle(QApplication::translate("MainWindowBase", "&Help"));
    } // retranslateUi


protected:
    enum IconID
    {
        image0_ID,
        image1_ID,
        unknown_ID
    };
    static QPixmap icon(IconID id)
    {
    static const char* const image0_data[] = { 
"22 22 203 2",
"Qt c None",
".f c #292c10",
".b c #293010",
".c c #293018",
".a c #313018",
"a3 c #313429",
"b# c #313820",
".W c #393829",
"#I c #393831",
"bi c #393c20",
"bg c #393c29",
"#o c #393c31",
"bf c #394020",
"bh c #394029",
"## c #413c31",
"#A c #413c39",
".D c #414431",
".E c #414818",
".# c #414820",
".n c #414839",
".d c #414c20",
"#S c #4a4839",
"#. c #4a4841",
"a4 c #4a4c39",
".V c #4a4c41",
"#n c #525541",
".U c #52554a",
"be c #525d20",
"b. c #525d29",
"#z c #5a554a",
".9 c #5a594a",
".C c #5a5952",
"#H c #5a5d41",
"#u c #5a5d4a",
"#t c #5a5d52",
"#0 c #5a614a",
".T c #5a6152",
"#8 c #625d52",
"aX c #626152",
"#m c #62654a",
"aD c #626552",
"aN c #62694a",
"aF c #626952",
".S c #62695a",
".o c #626d29",
".e c #626d31",
"#E c #626d41",
"ar c #626d4a",
"bd c #627129",
".m c #6a695a",
"af c #6a6d4a",
".8 c #6a6d52",
".R c #6a6d62",
"an c #6a7141",
"ae c #6a714a",
"a5 c #6a7531",
"#9 c #6a7541",
"a2 c #6a7920",
"aq c #737552",
"#s c #737939",
"#1 c #737941",
"aj c #737952",
"aJ c #737d31",
"#M c #737d39",
"ba c #738120",
"az c #738129",
"#R c #738131",
"bc c #738520",
"a7 c #738529",
".B c #7b7973",
".A c #7b7d73",
".H c #7b8173",
"aU c #7b8531",
"#7 c #7b8539",
"aV c #7b8939",
".Q c #7b8941",
"a9 c #7b8d20",
"#y c #7b8d31",
".z c #83817b",
"aa c #838562",
".l c #83857b",
"aK c #838962",
".y c #83896a",
"aT c #838d31",
"#v c #839139",
"aY c #839529",
"aO c #839541",
"#2 c #839920",
"aS c #839931",
"aW c #839d20",
"#a c #8b8983",
"#4 c #8b8d73",
".g c #8b8d83",
".k c #8b9183",
"#F c #8b9952",
"#h c #8ba120",
"bb c #8ba518",
"#f c #8ba520",
"#g c #8ba529",
"#l c #8baa18",
"a1 c #8bae08",
"a8 c #8bae10",
".p c #949183",
".j c #94918b",
".h c #94958b",
".x c #94996a",
"a6 c #94a520",
"#i c #94a529",
".7 c #94aa20",
"ax c #94aa29",
"aG c #94aa39",
"aR c #94ae18",
"ag c #94b210",
"#D c #94b218",
".G c #9c9994",
".s c #9c9d8b",
".r c #9c9d94",
".i c #9c9d9c",
"#V c #9ca17b",
"#N c #9ca573",
"aC c #9cae41",
"#3 c #9cb220",
"a# c #9cb229",
".P c #9cb239",
"a0 c #9cb610",
"#r c #9cb618",
"aZ c #9cba08",
"aI c #9cba10",
"a. c #9cba18",
"aM c #9cba20",
"aQ c #9cbe08",
"#Q c #9cbe10",
"#G c #9cc210",
".X c #a4a194",
"#b c #a4a594",
"aA c #a4b262",
"aE c #a4ba39",
"ay c #a4be10",
"#j c #a4be31",
"#x c #a4c210",
"av c #a4c239",
".6 c #a4c610",
"#L c #a4c618",
".F c #acaea4",
".q c #acaeac",
"#B c #acb294",
"ai c #acbe4a",
".w c #acbe6a",
"am c #acc239",
"#w c #acc610",
"aL c #acc629",
"ad c #acc639",
"#q c #acca20",
"#Z c #acca29",
"aP c #acce29",
"#e c #acce31",
".O c #b4ca41",
"as c #b4ca5a",
"#k c #b4ce29",
".2 c #b4ce31",
"aw c #b4ce39",
"aH c #b4ce4a",
"ap c #b4ce52",
".3 c #b4d239",
".4 c #b4d241",
".v c #bdca7b",
"ao c #bdce62",
".5 c #bdd241",
".1 c #bdd24a",
"au c #bdd252",
"aB c #bdd64a",
"#U c #bdd652",
"#Y c #bdd65a",
".t c #c5caa4",
".u c #c5ce8b",
".I c #c5ceac",
".N c #c5d65a",
"at c #c5d662",
"ah c #c5d67b",
"#P c #c5da5a",
".M c #c5da62",
".0 c #c5da6a",
"#K c #c5de6a",
".L c #cdde7b",
"#T c #cdde8b",
"#O c #cde27b",
"al c #cde283",
"#J c #d5daa4",
"#p c #d5e283",
"#d c #d5e28b",
"ab c #d5e2a4",
"#X c #d5e68b",
".K c #d5e694",
"#5 c #dee2b4",
"ak c #dee694",
"#C c #dee69c",
".J c #dee6ac",
".Z c #deeaa4",
"#W c #deeaac",
"ac c #e6eaac",
"#c c #e6eabd",
"#6 c #e6eeac",
".Y c #eeeec5",
"QtQtQt.#.a.b.c.b.a.b.c.b.a.b.c.b.a.b.dQtQtQt",
"Qt.e.f.g.h.i.h.j.k.j.k.j.k.j.g.g.l.m.n.b.oQt",
"Qt.b.p.q.r.g.p.s.t.u.v.w.x.y.z.A.B.m.C.D.cQt",
".E.g.F.G.H.s.I.J.K.L.M.N.O.P.Q.R.S.T.U.V.W.#",
".a.h.r.z.X.Y.Z.0.1.2.3.4.5.2.6.7.8.9.9#.##.b",
".f.i#a#b#c#d#e.6#f#g#h#i#j#k.6.6#l#m#n.V#o.c",
".c.h.X#c#p#q#r#s#t#u#t#u#m#v#w#x#x#y#z#.#A.b",
".f.j#B#C.2#D#E#t.9#t.9#F.Q#t#y#x#G#r#H#.#I.b",
".a.j#J#K#L#M#t#u#t#u#N#O#P.Q#t#f.6#Q#R#S#A.b",
".f.j#T#U#D#t.9#t.9#V#W#X#Y#Z#0#1#G#x#2#.#I.c",
".c.j#d.1#3#u#t#u#4#5#6.L.1#7#8#9#x#Qa.#.#o.b",
".f.j.L#Pa##t.9aaabac#d.Madae.9af#D#xag#.#I.b",
".a.jah.Mai#uaj.Lakal.0.5.3amaean.6#Q.7#S#A.b",
".f.jao.Mapaqarasatauavaw.4.1avax#Gayaz#.#I.c",
".c.kaA.1aBaCaDaeaE.QaFaGaH.5.2#x#xaIaJ#.#o.b",
".f.gaKaL#q#qaMaN.9#t.9#taOaP#x#xaQaR#H#.#I.b",
".a.l.BaS.6#x.6a.aTaUaV#7ax#x#x#QayagaW#S#I.b",
".f.m.SaXaY#x#G#x#G.6#G#x#QayaZaIaIa0a1a2a3.c",
".da4.C.U.9a5a6#Q#x#Q#x#Q#QaI#ha7a0a8a9b.b#.#",
"Qt.b.D.V#S.V#S.eba#lbbbbbc.ea4#.bda9bebf.fQt",
"Qt.o.cbg###o#A#o#A#o#o#o#A#o#o#I#Ibhbi.b.dQt",
"QtQtQt.#.f.c.f.b.f.c.f.b.f.c.f.b.f.c.EQtQtQt"};

    static const char* const image1_data[] = { 
"22 22 16 1",
". c None",
"a c #313000",
"# c #313062",
"j c #316562",
"h c #4a4c4a",
"b c #626562",
"i c #629962",
"d c #9c9962",
"c c #9c999c",
"l c #9cce00",
"g c #9cce62",
"m c #cdce00",
"f c #cdce62",
"e c #cdce9c",
"n c #cdff9c",
"k c #ffff9c",
"...#aaaaaaaaaaaaaaa...",
".bacccccccccccccdb#ab.",
".acecccceefgdddbbbh#a.",
"aceciceeeffffdibbjhh##",
"accdckefffffffldbhhh#a",
"accceeglddddgflllbhh#a",
"acceeglbjjjjbdmllihh#a",
"aceeflbjhjhdijilllbh#a",
"aceflbjjjjdffijdllih#a",
"acefljhjhdenfgjblldh#a",
"acefdjjjdenffibblllh#a",
"acffdjhdeeefgbhblllh#a",
"acfffjbfeefffgbblldh#a",
"acfffbbfffgfffgdllih#a",
"acgffgbbgibdffflllbh#a",
"acdggggbhjhjdgllllbh#a",
"adbdlllldiiidllllldh#a",
"abbbdllllllllllllllb#a",
"ahhhhbdllllllldillib##",
".a#hhhhbilllibhhbib#a.",
".ba################aa.",
"...#aaaaaaaaaaaaaaa..."};

    switch (id) {
        case image0_ID: return QPixmap((const char**)image0_data);
        case image1_ID: return QPixmap((const char**)image1_data);
        default: return QPixmap();
    } // switch
    } // icon

};

namespace Ui
{
    class MainWindowBase: public Ui_MainWindowBase {};
} // namespace Ui

class MainWindowBase : public Q3MainWindow, public Ui::MainWindowBase
{
    Q_OBJECT

public:
    MainWindowBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel);
    ~MainWindowBase();

public slots:
    virtual void addFontpath();
    virtual void addLibpath();
    virtual void addSubstitute();
    virtual void browseFontpath();
    virtual void browseLibpath();
    virtual void buildFont();
    virtual void buildPalette();
    virtual void downFontpath();
    virtual void downLibpath();
    virtual void downSubstitute();
    virtual void familySelected( const QString & );
    virtual void fileExit();
    virtual void fileSave();
    virtual void helpAbout();
    virtual void helpAboutQt();
    virtual void new_slot();
    virtual void pageChanged( QWidget * );
    virtual void paletteSelected( int );
    virtual void removeFontpath();
    virtual void removeLibpath();
    virtual void removeSubstitute();
    virtual void somethingModified();
    virtual void styleSelected( const QString & );
    virtual void substituteSelected( const QString & );
    virtual void tunePalette();
    virtual void upFontpath();
    virtual void upLibpath();
    virtual void upSubstitute();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();


};

#endif // MAINWINDOWBASE_H
