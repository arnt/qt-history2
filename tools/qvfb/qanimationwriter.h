/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
** EDITIONS: FREE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QANIMATIONWRITER_H
#define QANIMATIONWRITER_H

#include <qimage.h>

class QAnimationWriterData;

class QAnimationWriter
{
public:
    QAnimationWriter( const QString& filename, const char* format="MNG" );
    ~QAnimationWriter();

    bool okay() const;
    void setFrameRate(int);
    void appendBlankFrame();
    void appendFrame(const QImage&);
    void appendFrame(const QImage&, const QPoint& offset);

private:
    QImage prev;
    QIODevice* dev;
    QAnimationWriterData* d;
};

#endif
