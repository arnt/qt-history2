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
    int result;
    QPalette pal = PaletteEditor::getPalette(this, m_palette, &result);
    if (result == QDialog::Accepted) {
        m_palette = pal;
        emit changed();
    }
}
