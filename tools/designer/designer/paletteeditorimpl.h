/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

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
    static void getPalette( bool *ok, QWidget* target, QWidget* parent = 0, const char* name = 0 );

protected slots:
    void apply();

    void onActiveCentral( int );
    void onActiveEffect( int );
    void onInactiveCentral( int );
    void onInactiveEffect( int );
    void onDisabledCentral( int );
    void onDisabledEffect( int );

    void onChooseMainColor();
    void onChoose2ndMainColor();
    void onChooseActiveCentralColor();
    void onChooseActiveEffectColor();
    void onChooseActivePixmap();
    void onChooseInactiveCentralColor();
    void onChooseInactiveEffectColor();
    void onChooseInactivePixmap();
    void onChooseDisabledCentralColor();
    void onChooseDisabledEffectColor();
    void onChooseDisabledPixmap();

    void onToggleBuildAll( bool );
    void onToggleBuildInactive( bool );
    void onToggleBuildDisabled( bool );
    void onToggleActiveBuildEffects( bool );
    void onToggleInactiveBuildEffects( bool );
    void onToggleDisabledBuildEffects( bool );

    void onTabPage( const QString& );

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

    QColorGroup::ColorRole centralFromItem( int );
    QColorGroup::ColorRole effectFromItem( int );
    QPalette editPalette;
    QPalette previewPalette;

    FormWindow *formWindow;
};
