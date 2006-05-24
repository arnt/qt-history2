/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TEXTREPLACEMENT_H
#define TEXTREPLACEMENT_H

#include <QByteArray>
#include <QList>
#include <QtAlgorithms>

class TextReplacement
{
public:
    QByteArray newText;
    int insertPosition;
    int currentLenght;    //lenght of the text that is going to be replaced.
    bool operator<(const TextReplacement &other) const
    {
        return  (insertPosition < other.insertPosition);
    }
};

class TextReplacements
{
public:
    /*
        creates a TextReplacement that inserts newText at insertPosition. currentLength bytes
        are overwritten in the original text. If there already is an insert at insertPosition,
        the insert will not be performed.

        insert maintains the TextReplacement list in sorted order.

        Returns true if the insert was successfull, false otherwise;
    */
    bool insert(QByteArray newText, int insertPosition, int currentLenght);
    void clear();
    QList<TextReplacement> replacements() const
    {
        return textReplacementList;
    }
    QByteArray apply(QByteArray current);

    TextReplacements &operator+=(const TextReplacements &other);

private:
    QList<TextReplacement> textReplacementList;
};

#endif
