
#include "paletteeditorbutton.h"
#include "paletteeditor.h"

PaletteEditorButton::PaletteEditorButton(const QPalette &palette, QWidget *parent)
    : QPushButton(tr("Change Palette"), parent),
      m_palette(palette)
{
    connect(this, SIGNAL(clicked()), this, SLOT(showPaletteEditor()));
}

PaletteEditorButton::~PaletteEditorButton()
{
}

void PaletteEditorButton::setPalette(const QPalette &palette)
{
    m_palette = palette;
}

void PaletteEditorButton::showPaletteEditor()
{
    bool ok = false;
    QPalette pal = PaletteEditor::getPalette(&ok, m_palette, this);
    if (ok) {
        m_palette = pal;
        emit changed();
    }
}


