
#include "paletteeditor.h"

#include <QtCore/qdebug.h>
#include <QtGui/QMessageBox>

PaletteEditor::PaletteEditor(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::on_btnAdvanced_clicked()
{
    QMessageBox::information(this, tr("Palette Editor..."), tr("Feature not yet implemented!"));

#if 0 // ###
    bool ok;
    QPalette pal = PaletteEditorAdvanced::getPalette(&ok, m_editPalette, backgroundMode, this, "tune_palette", formWindow);
    if (!ok) return;
    m_editPalette = pal;
#endif

    updatePreviewPalette();
}

void PaletteEditor::on_buttonMainColor_clicked()
{
    buildPalette();
}

void PaletteEditor::on_buttonMainColor2_clicked()
{
    buildPalette();
}

void PaletteEditor::on_paletteCombo_activated(int)
{
    updatePreviewPalette();
}

void PaletteEditor::buildPalette()
{
    QColor btn = ui.buttonMainColor->brush().color();
    QColor back = ui.buttonMainColor2->brush().color();

    setEditPalette(QPalette(btn, back));
}

void PaletteEditor::buildActiveEffect()
{
    QColor btn = m_editPalette.color(QPalette::Button);

    QPalette temp(btn, btn);
    temp.setCurrentColorGroup(QPalette::Active);

    m_editPalette.setCurrentColorGroup(QPalette::Active);

    m_editPalette.setBrush(QPalette::Light, temp.light());
    m_editPalette.setBrush(QPalette::Midlight, temp.midlight());
    m_editPalette.setBrush(QPalette::Mid, temp.mid());
    m_editPalette.setBrush(QPalette::Dark, temp.dark());
    m_editPalette.setBrush(QPalette::Shadow, temp.shadow());
}

void PaletteEditor::updatePaletteEffect(QPalette::ColorGroup g)
{
    QColor btn = m_editPalette.color(g, QPalette::Button);

    QColor light = btn.light(150);
    QColor midlight = btn.light(115);
    QColor mid = btn.dark(150);
    QColor dark = btn.dark();
    QColor shadow = Qt::black;

    m_editPalette.setColor(g, QPalette::Light, light);
    m_editPalette.setColor(g, QPalette::Midlight, midlight);
    m_editPalette.setColor(g, QPalette::Mid, mid);
    m_editPalette.setColor(g, QPalette::Dark, dark);
    m_editPalette.setColor(g, QPalette::Shadow, shadow);
}

QPalette::ColorGroup PaletteEditor::selectedColorGroup() const
{
    switch (ui.paletteCombo->currentIndex()) {
    default: return QPalette::Active;
    case 0: return QPalette::Active;
    case 1: return QPalette::Inactive;
    case 2: return QPalette::Disabled;
    }
}

void PaletteEditor::updatePreviewPalette()
{
    QPalette::ColorGroup g = selectedColorGroup();

    // build the preview palette
    QPalette previewPalette = m_editPalette;
    for (QPalette::ColorRole r = QPalette::Foreground; r < QPalette::NColorRoles; reinterpret_cast<int&>(r)++) {
        previewPalette.setColor(QPalette::Active, r, m_editPalette.color(g, r));
        previewPalette.setColor(QPalette::Inactive, r, m_editPalette.color(g, r));
        previewPalette.setColor(QPalette::Disabled, r, m_editPalette.color(g, r));
    }

    ui.previewFrame->setPreviewPalette(previewPalette);
}

void PaletteEditor::updateStyledButtons()
{
    ui.buttonMainColor->setBrush(m_editPalette.color(QPalette::Active, QPalette::Button));
    ui.buttonMainColor2->setBrush(m_editPalette.color(QPalette::Active, QPalette::Background));
}

void PaletteEditor::setEditPalette(const QPalette& pal)
{
    m_editPalette = pal;
    buildActiveEffect();
    updatePaletteEffect(QPalette::Inactive);
    updatePaletteEffect(QPalette::Disabled);
    updatePreviewPalette();
    updateStyledButtons();
}

QPalette PaletteEditor::editPalette() const
{
    return m_editPalette;
}


QPalette PaletteEditor::getPalette(QWidget* parent, const QPalette &init, int *ok)
{
    PaletteEditor dlg(parent);
    dlg.setEditPalette(init);

    int result = dlg.exec();
    if (ok) *ok = result;

    return result == QDialog::Accepted ? dlg.editPalette() : init;
}

