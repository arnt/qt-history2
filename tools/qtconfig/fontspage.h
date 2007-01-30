#ifndef FONTSPAGE_H
#define FONTSPAGE_H

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

#include <QFrame>
#include <QMap>
#include <QStringList>
#include "ui_fontspage.h"
class FontsPage : public QFrame, public Ui_FontsPage
{
    Q_OBJECT
public:
    FontsPage(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *e);
    void save();
    void load();
    void updateFontLineEdit();
public slots:
    void onCurrentSubstitutionChanged();
    void updateSubstitutions();
    void add();
    void remove();
    void onFamilyChanged(const QString &family);
    void onPointSizeChanged();
    void onFontStyleChanged(const QString &family);
    void onSubsituteSourceSelected(const QString &str);
signals:
    void changed();
private:
    QMap<QString, QStringList> substitutions;
};

#endif
