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

#include <QtCore/qdebug.h>

PaletteEditorButton::PaletteEditorButton(const QPalette &palette, QWidget *parent)
    : QToolButton(parent),
      m_palette(palette)
{
    setFocusPolicy(Qt::NoFocus);
    setText(tr("Change Palette"));

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
    QPalette pal = PaletteEditor::getPalette(0, m_palette, &result);
    if (result == QDialog::Accepted) {
        m_palette = pal;
        emit changed();
    }
}
