/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "paletteeditor.h"

PaletteEditor::PaletteEditor(QWidget *parent, Qt::WFlags f, QMap<int, QString> *smap)
    : QDialog(parent, f), snrMap(smap)
{
    setupUi(this);

    editPalette = QApplication::palette();
    setPreviewPalette(editPalette);

    buttonCheckBox->setChecked(true);
    inactiveCheckBox->setChecked(true);
    disabledCheckBox->setChecked(true);
    advPixmapButton->setButtonType(StyleButton::PixmapButton);

    //connections
    connect(paletteCombo, SIGNAL(activated(int)), this, SLOT(paletteSelected(int)));
    connect(rolesCombo, SIGNAL(activated(int)), this, SLOT(roleSelected(int)));

    connect(backgroundButton, SIGNAL(changed()), this, SLOT(onChooseBasicColor()));
    connect(effectButton, SIGNAL(changed()), this, SLOT(onChooseBasicColor()));
    connect(advColorButton, SIGNAL(changed()), this, SLOT(onChooseAdvancedColor()));

    connect(advPixmapButton, SIGNAL(changed()), this, SLOT(onChoosePixmap()));

    connect(buttonCheckBox, SIGNAL(toggled(bool)), this, SLOT(updatePaletteEditor()));
    connect(disabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(updatePaletteEditor()));
    connect(inactiveCheckBox, SIGNAL(toggled(bool)), this, SLOT(updatePaletteEditor()));

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    //update the editor state
    updatePaletteEditor();
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::cleanUpsnrMap()
{
    //checking the palette if there are unused keys in snrMap
    QPalette::ColorGroup cg;
    QPixmap p;
    QList<int> tmpList;

    for (int i=0; i<3; ++i) {
        cg = groupFromItem(i);
        for (int j=0; j<16; ++j) {
            p = editPalette.brush(cg, roleFromItem(j)).texture();
            if (!p.isNull())
                tmpList.append(p.serialNumber());
        }
    }

    //TODO: check if works...
    QMap<int, QString>::iterator it = snrMap->begin();
    while (it != snrMap->end()) {
        if(!tmpList.contains(it.key()))
            snrMap->remove(it.key());
        ++it;
    }
}

void PaletteEditor::onChoosePixmap()
{
    QPalette::ColorGroup cg = groupFromItem(paletteCombo->currentIndex());
    QPalette::ColorRole cr = roleFromItem(rolesCombo->currentIndex());
    const QBrush oldBrush = editPalette.brush(cg, cr);

    QPixmap p(advPixmapButton->pixmapFileName());

    if (!p.isNull())
    {
        QBrush b(editPalette.brush(cg, cr));
        b.setTexture(p);
        editPalette.setBrush(cg, cr, b);

        //add the snr and filename to the map...
        snrMap->insertMulti(p.serialNumber(), advPixmapButton->pixmapFileName());

        if (inactiveCheckBox->isChecked())
            buildInactive();

        if (disabledCheckBox->isChecked())
            buildDisabled();

        //remove unused keys from snrMap
        cleanUpsnrMap();

        setPreviewPalette(editPalette);
        updatePaletteEditor();
    }
}

void PaletteEditor::onChooseAdvancedColor()
{
    QPalette::ColorGroup cg = groupFromItem(paletteCombo->currentIndex());
    QPalette::ColorRole cr = roleFromItem(rolesCombo->currentIndex());

    QBrush b(editPalette.brush(cg, cr));
    b.setColor(advColorButton->brush().color());
    editPalette.setBrush(cg, cr, b);

    if (buttonCheckBox->isChecked())
        buildEffect(cg);

    if (inactiveCheckBox->isChecked())
        buildInactive();

    if (disabledCheckBox->isChecked())
        buildDisabled();

    setPreviewPalette(editPalette);
    updatePaletteEditor();
}

void PaletteEditor::onChooseBasicColor()
{
    editPalette = QPalette(effectButton->brush().color(),
        backgroundButton->brush().color());

    setPreviewPalette(editPalette);
    updatePaletteEditor();
}

void PaletteEditor::roleSelected(int item)
{
    updatePaletteEditor();
    Q_UNUSED(item);
}

void PaletteEditor::paletteSelected(int item)
{
    setPreviewPalette(editPalette);
    updatePaletteEditor();
    Q_UNUSED(item);
}

QPalette::ColorGroup PaletteEditor::groupFromItem(int item)
{
    switch(item)
    {
    case 0:
        return QPalette::Active;
    case 1:
        return QPalette::Inactive;
    case 2:
        return QPalette::Disabled;
    default:
        return QPalette::NColorGroups;
    }
}

QPalette::ColorRole PaletteEditor::roleFromItem(int item)
{
    switch(item)
    {
    case 0:
        return QPalette::Background;
    case 1:
        return QPalette::Foreground;
    case 2:
        return QPalette::Base;
    case 3:
        return QPalette::Text;
    case 4:
        return QPalette::Button;
    case 5:
        return QPalette::ButtonText;
    case 6:
        return QPalette::Light;
    case 7:
        return QPalette::Midlight;
    case 8:
        return QPalette::Mid;
    case 9:
        return QPalette::Dark;
    case 10:
        return QPalette::Shadow;
    case 11:
        return QPalette::Highlight;
    case 12:
        return QPalette::HighlightedText;
    case 13:
        return QPalette::BrightText;
    case 14:
        return QPalette::Link;
    case 15:
        return QPalette::LinkVisited;
    default:
        return QPalette::NColorRoles;
    }
}

void PaletteEditor::buildInactive()
{
    copyColorGroup(editPalette, editPalette, QPalette::Active, QPalette::Inactive);
}

void PaletteEditor::buildDisabled()
{
    copyColorGroup(editPalette, editPalette, QPalette::Active, QPalette::Disabled);
    editPalette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    editPalette.setColor(QPalette::Disabled, QPalette::Foreground, Qt::darkGray);
    editPalette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
}

void PaletteEditor::buildEffect(QPalette::ColorGroup cg)
{
    QColor light, midlight, mid, dark, shadow;
    QColor btn = editPalette.color(cg, QPalette::Button);

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = Qt::black;

    editPalette.setColor(cg, QPalette::Light, light);
    editPalette.setColor(cg, QPalette::Midlight, midlight);
    editPalette.setColor(cg, QPalette::Mid, mid);
    editPalette.setColor(cg, QPalette::Dark, dark);
    editPalette.setColor(cg, QPalette::Shadow, shadow);
}

void PaletteEditor::copyColorGroup(const QPalette &fpal, QPalette &tpal, QPalette::ColorGroup fcg, QPalette::ColorGroup tcg)
{
    QPalette::ColorRole clrRole;
    for (int i=0; i<16; ++i) {
        clrRole = roleFromItem(i);
        tpal.setBrush(tcg, clrRole, fpal.brush(fcg, clrRole));
    }
}

void PaletteEditor::setPreviewPalette(const QPalette &pal)
{
    QPalette::ColorGroup cg = groupFromItem(paletteCombo->currentIndex());

    copyColorGroup(pal, previewPalette, cg, QPalette::Active);
    copyColorGroup(pal, previewPalette, cg, QPalette::Inactive);
    copyColorGroup(pal, previewPalette, cg, QPalette::Disabled);

    previewWindow->setPalette(pal);
}

void PaletteEditor::updatePaletteEditor()
{
    effectButton->setBrush(editPalette.brush(QPalette::Active, QPalette::Button));
    backgroundButton->setBrush(editPalette.brush(QPalette::Active, QPalette::Background));

    int groupItem = paletteCombo->currentIndex();
    int roleItem = rolesCombo->currentIndex();
    QPalette::ColorGroup cg = groupFromItem(groupItem);
    QPalette::ColorRole cr = roleFromItem(roleItem);

    advColorButton->setBrush(editPalette.brush(cg, cr));

    //This will delete the pixmap, because the pixmap is'n in the palette...
    advPixmapButton->setBrush(editPalette.brush(cg, cr));

    bool roleDisabled = (inactiveCheckBox->isChecked() && (cg == QPalette::Inactive)) ||
        (disabledCheckBox->isChecked() && (cg == QPalette::Disabled));
    bool advButtonDisabled = (buttonCheckBox->isChecked()) && (roleItem > 5) && (roleItem < 11);

    rolesGroup->setEnabled(!roleDisabled);
    advColorButton->setEnabled(!advButtonDisabled);
    advPixmapButton->setEnabled(!advButtonDisabled && snrMap);
    colorLabel->setEnabled(!advButtonDisabled);
    pixmapLabel->setEnabled(!advButtonDisabled);
}

void PaletteEditor::setPal( const QPalette& pal )
{
    editPalette = pal;
    setPreviewPalette(pal);
}

QPalette PaletteEditor::pal() const
{
    return editPalette;
}

QPalette PaletteEditor::getPalette(bool *ok, const QPalette &init,
                                   QWidget *parent, QMap<int, QString> *snrMap)
{
    PaletteEditor* dlg = new PaletteEditor(parent, 0, snrMap);

    if (init != QPalette())
        dlg->setPal(init);
    int resultCode = dlg->exec();

    QPalette result = init;
    if (resultCode == QDialog::Accepted) {
        if (ok)
            *ok = true;
        result = dlg->pal();
    }
    else {
        if (ok)
            *ok = false;
    }
    delete dlg;
    return result;
}
