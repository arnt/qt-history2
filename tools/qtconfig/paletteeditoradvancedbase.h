#ifndef PALETTEEDITORADVANCEDBASE_H
#define PALETTEEDITORADVANCEDBASE_H

#include <qvariant.h>

class ColorButton;

#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3GroupBox>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include "colorbutton.h"

class Ui_PaletteEditorAdvancedBase
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *TextLabel1;
    QComboBox *paletteCombo;
    Q3ButtonGroup *ButtonGroup1;
    QVBoxLayout *vboxLayout1;
    QCheckBox *checkBuildInactive;
    QCheckBox *checkBuildDisabled;
    Q3GroupBox *groupCentral;
    QVBoxLayout *vboxLayout2;
    QComboBox *comboCentral;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem;
    QLabel *labelCentral;
    ColorButton *buttonCentral;
    Q3GroupBox *groupEffect;
    QVBoxLayout *vboxLayout3;
    QHBoxLayout *hboxLayout2;
    QCheckBox *checkBuildEffect;
    QComboBox *comboEffect;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem1;
    QLabel *labelEffect;
    ColorButton *buttonEffect;
    QHBoxLayout *hboxLayout4;
    QSpacerItem *spacerItem2;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    void setupUi(QDialog *PaletteEditorAdvancedBase)
    {
    PaletteEditorAdvancedBase->setObjectName(QString::fromUtf8("PaletteEditorAdvancedBase"));
    PaletteEditorAdvancedBase->setEnabled(true);
    PaletteEditorAdvancedBase->resize(QSize(295, 346).expandedTo(PaletteEditorAdvancedBase->minimumSizeHint()));
    PaletteEditorAdvancedBase->setSizeGripEnabled(true);
    vboxLayout = new QVBoxLayout(PaletteEditorAdvancedBase);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout->setMargin(11);
    vboxLayout->setSpacing(6);
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    hboxLayout->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(6);
    TextLabel1 = new QLabel(PaletteEditorAdvancedBase);
    TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));

    hboxLayout->addWidget(TextLabel1);

    paletteCombo = new QComboBox(PaletteEditorAdvancedBase);
    paletteCombo->setObjectName(QString::fromUtf8("paletteCombo"));

    hboxLayout->addWidget(paletteCombo);


    vboxLayout->addLayout(hboxLayout);

    ButtonGroup1 = new Q3ButtonGroup(PaletteEditorAdvancedBase);
    ButtonGroup1->setObjectName(QString::fromUtf8("ButtonGroup1"));
    QSizePolicy sizePolicy((QSizePolicy::Policy)5, (QSizePolicy::Policy)4);
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
    vboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout1->setMargin(11);
    vboxLayout1->setSpacing(6);
    checkBuildInactive = new QCheckBox(ButtonGroup1);
    checkBuildInactive->setObjectName(QString::fromUtf8("checkBuildInactive"));
    checkBuildInactive->setChecked(true);

    vboxLayout1->addWidget(checkBuildInactive);

    checkBuildDisabled = new QCheckBox(ButtonGroup1);
    checkBuildDisabled->setObjectName(QString::fromUtf8("checkBuildDisabled"));
    checkBuildDisabled->setChecked(true);

    vboxLayout1->addWidget(checkBuildDisabled);


    vboxLayout->addWidget(ButtonGroup1);

    groupCentral = new Q3GroupBox(PaletteEditorAdvancedBase);
    groupCentral->setObjectName(QString::fromUtf8("groupCentral"));
    groupCentral->setColumnLayout(0, Qt::Vertical);
    groupCentral->layout()->setSpacing(6);
    groupCentral->layout()->setMargin(11);
    vboxLayout2 = new QVBoxLayout(groupCentral->layout());
    vboxLayout2->setAlignment(Qt::AlignTop);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout2->setMargin(11);
    vboxLayout2->setSpacing(6);
    comboCentral = new QComboBox(groupCentral);
    comboCentral->setObjectName(QString::fromUtf8("comboCentral"));

    vboxLayout2->addWidget(comboCentral);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout1->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout1->setMargin(0);
    hboxLayout1->setSpacing(6);
    spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem);

    labelCentral = new QLabel(groupCentral);
    labelCentral->setObjectName(QString::fromUtf8("labelCentral"));
    QSizePolicy sizePolicy1((QSizePolicy::Policy)1, (QSizePolicy::Policy)1);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(labelCentral->sizePolicy().hasHeightForWidth());
    labelCentral->setSizePolicy(sizePolicy1);
    labelCentral->setMinimumSize(QSize(0, 0));

    hboxLayout1->addWidget(labelCentral);

    buttonCentral = new ColorButton(groupCentral);
    buttonCentral->setObjectName(QString::fromUtf8("buttonCentral"));
    QSizePolicy sizePolicy2((QSizePolicy::Policy)0, (QSizePolicy::Policy)0);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(buttonCentral->sizePolicy().hasHeightForWidth());
    buttonCentral->setSizePolicy(sizePolicy2);
    buttonCentral->setFocusPolicy(Qt::TabFocus);

    hboxLayout1->addWidget(buttonCentral);


    vboxLayout2->addLayout(hboxLayout1);


    vboxLayout->addWidget(groupCentral);

    groupEffect = new Q3GroupBox(PaletteEditorAdvancedBase);
    groupEffect->setObjectName(QString::fromUtf8("groupEffect"));
    groupEffect->setColumnLayout(0, Qt::Vertical);
    groupEffect->layout()->setSpacing(6);
    groupEffect->layout()->setMargin(11);
    vboxLayout3 = new QVBoxLayout(groupEffect->layout());
    vboxLayout3->setAlignment(Qt::AlignTop);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    vboxLayout3->setObjectName(QString::fromUtf8("unnamed"));
    vboxLayout3->setMargin(11);
    vboxLayout3->setSpacing(6);
    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    hboxLayout2->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout2->setMargin(0);
    hboxLayout2->setSpacing(6);
    checkBuildEffect = new QCheckBox(groupEffect);
    checkBuildEffect->setObjectName(QString::fromUtf8("checkBuildEffect"));
    checkBuildEffect->setChecked(true);

    hboxLayout2->addWidget(checkBuildEffect);

    comboEffect = new QComboBox(groupEffect);
    comboEffect->setObjectName(QString::fromUtf8("comboEffect"));

    hboxLayout2->addWidget(comboEffect);


    vboxLayout3->addLayout(hboxLayout2);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    hboxLayout3->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout3->setMargin(0);
    hboxLayout3->setSpacing(6);
    spacerItem1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout3->addItem(spacerItem1);

    labelEffect = new QLabel(groupEffect);
    labelEffect->setObjectName(QString::fromUtf8("labelEffect"));
    QSizePolicy sizePolicy3((QSizePolicy::Policy)1, (QSizePolicy::Policy)1);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(labelEffect->sizePolicy().hasHeightForWidth());
    labelEffect->setSizePolicy(sizePolicy3);
    labelEffect->setMinimumSize(QSize(0, 0));

    hboxLayout3->addWidget(labelEffect);

    buttonEffect = new ColorButton(groupEffect);
    buttonEffect->setObjectName(QString::fromUtf8("buttonEffect"));
    QSizePolicy sizePolicy4((QSizePolicy::Policy)0, (QSizePolicy::Policy)0);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(buttonEffect->sizePolicy().hasHeightForWidth());
    buttonEffect->setSizePolicy(sizePolicy4);
    buttonEffect->setFocusPolicy(Qt::TabFocus);

    hboxLayout3->addWidget(buttonEffect);


    vboxLayout3->addLayout(hboxLayout3);


    vboxLayout->addWidget(groupEffect);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    hboxLayout4->setObjectName(QString::fromUtf8("unnamed"));
    hboxLayout4->setMargin(0);
    hboxLayout4->setSpacing(6);
    spacerItem2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout4->addItem(spacerItem2);

    buttonOk = new QPushButton(PaletteEditorAdvancedBase);
    buttonOk->setObjectName(QString::fromUtf8("buttonOk"));
    buttonOk->setAutoDefault(true);
    buttonOk->setDefault(true);

    hboxLayout4->addWidget(buttonOk);

    buttonCancel = new QPushButton(PaletteEditorAdvancedBase);
    buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
    buttonCancel->setAutoDefault(true);

    hboxLayout4->addWidget(buttonCancel);


    vboxLayout->addLayout(hboxLayout4);

    TextLabel1->setBuddy(paletteCombo);
    labelCentral->setBuddy(buttonCentral);
    labelEffect->setBuddy(buttonEffect);
    QWidget::setTabOrder(buttonOk, buttonCancel);
    QWidget::setTabOrder(buttonCancel, paletteCombo);
    QWidget::setTabOrder(paletteCombo, checkBuildInactive);
    QWidget::setTabOrder(checkBuildInactive, checkBuildDisabled);
    QWidget::setTabOrder(checkBuildDisabled, comboCentral);
    QWidget::setTabOrder(comboCentral, buttonCentral);
    QWidget::setTabOrder(buttonCentral, checkBuildEffect);
    QWidget::setTabOrder(checkBuildEffect, comboEffect);
    QWidget::setTabOrder(comboEffect, buttonEffect);
    retranslateUi(PaletteEditorAdvancedBase);

    QMetaObject::connectSlotsByName(PaletteEditorAdvancedBase);
    } // setupUi

    void retranslateUi(QDialog *PaletteEditorAdvancedBase)
    {
    PaletteEditorAdvancedBase->setWindowTitle(QApplication::translate("PaletteEditorAdvancedBase", "Tune Palette"));
    PaletteEditorAdvancedBase->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "<b>Edit Palette</b><p>Change the palette of the current widget or form.</p><p>Use a generated palette or select colors for each color group and each color role.</p><p>The palette can be tested with different widget layouts in the preview section.</p>")));
    TextLabel1->setText(QApplication::translate("PaletteEditorAdvancedBase", "Select &Palette:"));
    paletteCombo->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Active Palette"));
    paletteCombo->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Inactive Palette"));
    paletteCombo->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Disabled Palette"));
    ButtonGroup1->setTitle(QApplication::translate("PaletteEditorAdvancedBase", "Auto"));
    checkBuildInactive->setText(QApplication::translate("PaletteEditorAdvancedBase", "Build inactive palette from active"));
    checkBuildDisabled->setText(QApplication::translate("PaletteEditorAdvancedBase", "Build disabled palette from active"));
    groupCentral->setTitle(QApplication::translate("PaletteEditorAdvancedBase", "Central color &roles"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Background"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Foreground"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Button"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Base"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Text"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "BrightText"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "ButtonText"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Highlight"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "HighlightText"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Link"));
    comboCentral->addItem(QApplication::translate("PaletteEditorAdvancedBase", "LinkVisited"));
    comboCentral->setProperty("toolTip", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose central color role")));
    comboCentral->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "<b>Select a color role.</b><p>Available central roles are: <ul> <li>Background - general background color.</li> <li>Foreground - general foreground color. </li> <li>Base - used as background color for e.g. text entry widgets, usually white or another light color. </li> <li>Text - the forground color used with Base. Usually this is the same as the Foreground, in what case it must provide good contrast both with Background and Base. </li> <li>Button - general button background color, where buttons need a background different from Background, as in the Macintosh style. </li> <li>ButtonText - a foreground color used with the Button color. </li> <li>Highlight - a color to indicate a selected or highlighted item. </li> <li>HighlightedText - a text color that contrasts to Highlight. </li> <li>BrightText - a text color that is very different from Foreground and contrasts well with e.g. black. </li> </ul> </p>")));
    labelCentral->setText(QApplication::translate("PaletteEditorAdvancedBase", "&Select Color:"));
    buttonCentral->setProperty("toolTip", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose a color")));
    buttonCentral->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose a color for the selected central color role.")));
    groupEffect->setTitle(QApplication::translate("PaletteEditorAdvancedBase", "3-D shadow &effects"));
    checkBuildEffect->setText(QApplication::translate("PaletteEditorAdvancedBase", "Build &from button color"));
    checkBuildEffect->setProperty("toolTip", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Generate shadings")));
    checkBuildEffect->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Check to let 3D-effect colors be calculated from button-color.")));
    comboEffect->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Light"));
    comboEffect->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Midlight"));
    comboEffect->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Mid"));
    comboEffect->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Dark"));
    comboEffect->addItem(QApplication::translate("PaletteEditorAdvancedBase", "Shadow"));
    comboEffect->setProperty("toolTip", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose 3D-effect color role")));
    comboEffect->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "<b>Select a color role.</b><p>Available effect roles are: <ul> <li>Light - lighter than Button color. </li> <li>Midlight - between Button and Light. </li> <li>Mid - between Button and Dark. </li> <li>Dark - darker than Button. </li> <li>Shadow - a very dark color. </li> </ul>")));
    labelEffect->setText(QApplication::translate("PaletteEditorAdvancedBase", "Select Co&lor:"));
    buttonEffect->setProperty("toolTip", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose a color")));
    buttonEffect->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Choose a color for the selected effect color role.")));
    buttonOk->setText(QApplication::translate("PaletteEditorAdvancedBase", "OK"));
    buttonOk->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Close dialog and apply all changes.")));
    buttonCancel->setText(QApplication::translate("PaletteEditorAdvancedBase", "Cancel"));
    buttonCancel->setProperty("whatsThis", QVariant(QApplication::translate("PaletteEditorAdvancedBase", "Close dialog and discard all changes.")));
    Q_UNUSED(PaletteEditorAdvancedBase);
    } // retranslateUi


protected:
    enum IconID
    {
        image0_ID,
        unknown_ID
    };
    static QPixmap icon(IconID id)
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
    } // switch
    } // icon

};

namespace Ui
{
    class PaletteEditorAdvancedBase: public Ui_PaletteEditorAdvancedBase {};
} // namespace Ui

class PaletteEditorAdvancedBase : public QDialog, public Ui::PaletteEditorAdvancedBase
{
    Q_OBJECT

public:
    PaletteEditorAdvancedBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~PaletteEditorAdvancedBase();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();
    virtual void onCentral(int);
    virtual void onChooseCentralColor();
    virtual void onChooseEffectColor();
    virtual void onEffect(int);
    virtual void onToggleBuildDisabled(bool);
    virtual void onToggleBuildEffects(bool);
    virtual void onToggleBuildInactive(bool);
    virtual void paletteSelected(int);


};

#endif // PALETTEEDITORADVANCEDBASE_H
