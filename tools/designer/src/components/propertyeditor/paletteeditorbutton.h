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

#ifndef PALETTEEDITORBUTTON_H
#define PALETTEEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtGui/QPalette>
#include <QtGui/QPushButton>

class QT_PROPERTYEDITOR_EXPORT PaletteEditorButton: public QPushButton
{
    Q_OBJECT
public:
    PaletteEditorButton(const QPalette &palette, QWidget *parent = 0);
    virtual ~PaletteEditorButton();

    inline QPalette palette() const
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
