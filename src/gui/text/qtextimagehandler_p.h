#ifndef QTEXTIMAGEHANDLER_P_H
#define QTEXTIMAGEHANDLER_P_H

#include <qobject.h>
#include <private/qtextlayout_p.h>

#include "qtextglobal.h"

class QTextImageFormat;

class QTextImageHandler : public QObject, 
			public QTextInlineObjectInterface
{
    Q_OBJECT
public:
    QTextImageHandler(QObject *parent = 0);

    virtual void layoutObject(QTextObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *p, const QPoint &position, QTextObject item, const QTextFormat &format, QTextLayout::SelectionType selType);
};

#endif // QTEXTIMAGEHANDLER_P_H
