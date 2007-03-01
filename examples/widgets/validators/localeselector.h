/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LOCALESELECTOR_H
#define LOCALESELECTOR_H

#include <QComboBox>

class LocaleSelector : public QComboBox
{
    Q_OBJECT

public:
    LocaleSelector(QWidget *parent = 0);

signals:
    void localeSelected(const QLocale &locale);

private slots:
    void emitLocaleSelected(int index);
};

#endif //LOCALESELECTOR_H
