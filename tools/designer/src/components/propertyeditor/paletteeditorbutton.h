#ifndef PALETTEEDITORBUTTON_H
#define PALETTEEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QPalette>
#include <QPushButton>

class QT_PROPERTYEDITOR_EXPORT PaletteEditorButton: public QPushButton
{
    Q_OBJECT
public:
    PaletteEditorButton(const QPalette &palette, QWidget *parent = 0);
    virtual ~PaletteEditorButton();

    QPalette palette() const
    { return m_palette; }

signals:
    void changed();

public slots:
    void setPalette(const QPalette &palette);

private slots:
    void showPaletteEditor();

private:
    QPalette m_palette;
};

#endif // PALETTEEDITORBUTTON_H
