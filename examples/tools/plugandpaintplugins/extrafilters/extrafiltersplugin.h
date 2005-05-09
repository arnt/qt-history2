/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H

#include <QObject>

#include <plugandpaint/interfaces.h>

class ExtraFiltersPlugin : public QObject, public FilterInterface
{
    Q_OBJECT
    Q_INTERFACES(FilterInterface)

public:
    QStringList filters() const;
    QImage filterImage(const QString &filter, const QImage &image,
                       QWidget *parent);
};

#endif
