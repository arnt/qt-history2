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

#ifndef PALETTEEDITORADVANCEDIMPL_H
#define PALETTEEDITORADVANCEDIMPL_H

#include "paletteeditoradvanced.h"

class FormWindow;

class PaletteEditorAdvanced : public PaletteEditorAdvancedBase
{
    Q_OBJECT
public:
    PaletteEditorAdvanced( FormWindow *fw, QWidget * parent=0, const char * name=0,
			   bool modal=FALSE, WFlags f=0 );
    ~PaletteEditorAdvanced();

    static QPalette getPalette( bool *ok, const QPalette &pal, BackgroundMode mode = PaletteBackground,
				QWidget* parent = 0, const char* name = 0, FormWindow *fw = 0 );

protected slots:
    void paletteSelected(int);

    void onCentral( int );
    void onEffect( int );

    void onChooseCentralColor();
    void onChooseEffectColor();
    void onChoosePixmap();

    void onToggleBuildEffects( bool );
    void onToggleBuildInactive( bool );
    void onToggleBuildDisabled( bool );

protected:
    void mapToActiveCentralRole( const QColor& );
    void mapToActiveEffectRole( const QColor& );
    void mapToActivePixmapRole( const QPixmap& );
    void mapToInactiveCentralRole( const QColor& );
    void mapToInactiveEffectRole( const QColor& );
    void mapToInactivePixmapRole( const QPixmap& );
    void mapToDisabledCentralRole( const QColor& );
    void mapToDisabledEffectRole( const QColor& );
    void mapToDisabledPixmapRole( const QPixmap& );


    void buildPalette();
    void buildActiveEffect();
    void buildInactive();
    void buildInactiveEffect();
    void buildDisabled();
    void buildDisabledEffect();

private:
    void setPreviewPalette( const QPalette& );
    void updateStyledButtons();
    void setupBackgroundMode( BackgroundMode );

    QPalette pal() const;
    void setPal( const QPalette& );

    QPalette::ColorRole centralFromItem( int );
    QPalette::ColorRole effectFromItem( int );
    QPalette editPalette;
    QPalette previewPalette;

    FormWindow *formWindow;

    int selectedPalette;
};

#endif
