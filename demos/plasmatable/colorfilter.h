/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#ifndef COLORMODEL_H
#define COLORMODEL_H

#include <qproxymodel.h>

class ColorFilter : public QProxyModel
{
    Q_OBJECT
public:
    ColorFilter(QObject *parent = 0);
    ~ColorFilter();

    inline void setFilter(unsigned int filter) { colorfilter = filter; }
    inline unsigned int filter() const { return colorfilter; }

    QVariant data(const QModelIndex &index, int role) const;

private:
    unsigned int colorfilter;
};

#endif
