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

#include "propertyeditor_global.h"
#include "ui_paletteeditor.h"
#include <qvariant.h>
#include <qdialog.h>
#include <qmap.h>

class QT_PROPERTYEDITOR_EXPORT PaletteEditor : public QDialog, public Ui::PaletteEditorBase
{
    Q_OBJECT

public:
    PaletteEditor(QWidget *parent = 0, Qt::WFlags f = 0, QMap<int, QString> *smap = 0);
    ~PaletteEditor();

    static QPalette getPalette(bool *ok, const QPalette &pal, QWidget *parent = 0, QMap<int, QString> *snrMap = 0);

protected slots:
    void onChooseBasicColor();
    void onChooseAdvancedColor();
    void onChoosePixmap();
    void paletteSelected(int item);
    void roleSelected(int item);
    void updatePaletteEditor();

protected:
    void buildInactive();
    void buildDisabled();
    void buildEffect(QPalette::ColorGroup cg);

private:
    void copyColorGroup(const QPalette &fpal, QPalette &tpal, QPalette::ColorGroup fcg, QPalette::ColorGroup tcg);
    void setPreviewPalette(const QPalette &pal);
    void cleanUpsnrMap();
    QPalette pal() const;
    void setPal( const QPalette& );

    QPalette::ColorRole roleFromItem(int item);
    QPalette::ColorGroup groupFromItem(int item);
    QPalette editPalette;
    QPalette previewPalette;

    QMap<int, QString> *snrMap;
};

#endif // PALETTEEDITOR_H
