#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"

namespace qdesigner { namespace components { namespace propertyeditor {

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

} } } // namespace qdesigner::components::propertyeditor

#endif // PALETTEEDITOR_H
