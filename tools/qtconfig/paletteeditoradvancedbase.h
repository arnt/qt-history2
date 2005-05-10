#ifndef PALETTEEDITORADVANCEDBASE_H
#define PALETTEEDITORADVANCEDBASE_H

#include <qvariant.h>
#include "ui_paletteeditoradvancedbase.h"

class ColorButton;

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
