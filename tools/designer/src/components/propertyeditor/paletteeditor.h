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

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QPalette editPalette READ editPalette WRITE setEditPalette)
public:
    virtual ~PaletteEditor();

    static QPalette getPalette(QWidget* parent, const QPalette &init = QPalette(), int *result = 0);

    QPalette editPalette() const;
    void setEditPalette(const QPalette&);

private slots:
    void on_buttonMainColor_clicked();
    void on_buttonMainColor2_clicked();
    void on_btnAdvanced_clicked();
    void on_paletteCombo_activated(int);

protected:
    PaletteEditor(QWidget *parent);

private:
    void buildPalette();

    void buildActiveEffect();
    void updatePaletteEffect(QPalette::ColorGroup g);

    void updatePreviewPalette();
    void updateStyledButtons();

    QPalette::ColorGroup selectedColorGroup() const;

private:
    Ui::PaletteEditor ui;
    QPalette m_editPalette;
};

}  // namespace qdesigner_internal

#endif // PALETTEEDITOR_H
