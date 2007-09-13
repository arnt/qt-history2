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

#ifndef CPPEXTRACTIMAGES_H
#define CPPEXTRACTIMAGES_H

#include "treewalker.h"
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

class QTextStream;
class Driver;
class Uic;

struct Option;

namespace CPP {

class ExtractImages : public TreeWalker
{
public:
    ExtractImages(const Option &opt);

    void acceptUI(DomUI *node);
    void acceptImages(DomImages *images);
    void acceptImage(DomImage *image);

private:
    QTextStream *m_output;
    const Option &m_option;
    QDir m_imagesDir;
};

} // namespace CPP

QT_END_NAMESPACE

#endif // CPPEXTRACTIMAGES_H
