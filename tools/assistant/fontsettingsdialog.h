/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _FONT_SETTINGS_DIALOG_H_
#define _FONT_SETTINGS_DIALOG_H_

#include <QtGui/QDialog>

class FontPanel;
struct FontSettings;
class QDialogButtonBox;

class FontSettingsDialog : public QDialog
{
public:
    FontSettingsDialog(QWidget *parent = 0);
    ~FontSettingsDialog();

    bool showDialog(FontSettings *settings);

private:
    void updateFontSettings(FontSettings *settings);
    void setupFontSettingsDialog(const FontSettings *settings);
    
private:
    FontPanel *m_windowFontPanel;
    FontPanel *m_browserFontPanel;
    QDialogButtonBox *m_dialogButtonBox;
};

#endif  // _FONT_SETTINGS_DIALOG_H_
