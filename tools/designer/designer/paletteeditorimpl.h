/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PALETTEEDITORIMPL_H
#define PALETTEEDITORIMPL_H

#include "paletteeditor.h"

class FormWindow;

class PaletteEditor : public PaletteEditorBase
{
    Q_OBJECT

public:
    PaletteEditor( FormWindow *fw, QWidget * parent=0, const char * name=0, bool modal=FALSE, WFlags f=0 );
    ~PaletteEditor();

    static QPalette getPalette( bool *ok, const QPalette &pal, BackgroundMode mode = PaletteBackground,
				QWidget* parent = 0, const char* name = 0, FormWindow *fw = 0 );

protected slots:
    void onChooseMainColor();
    void onChoose2ndMainColor();
    void onTune();
    void paletteSelected(int);

protected:
    void buildPalette();
    void buildActiveEffect();
    void buildInactive();
    void buildInactiveEffect();
    void buildDisabled();
    void buildDisabledEffect();

private:
    void setPreviewPalette( const QPalette& );
    void updateStyledButtons();
    void setupBackgroundMode( BackgroundMode mode ) { backgroundMode = mode; }

    QPalette pal() const;
    void setPal( const QPalette& );

    QPalette::ColorRole centralFromItem( int );
    QPalette::ColorRole effectFromItem( int );
    QPalette editPalette;
    QPalette previewPalette;

    FormWindow *formWindow;
    BackgroundMode backgroundMode;
};

#endif
