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

#include "textreplacement.h"
#include <iostream>
using namespace std;

bool TextReplacements::insert(QByteArray newText, int insertPosition, int currentLenght)
{
    //bubble sort the new replacement into the list
    int i;
    for(i=0; i<textReplacementList.size(); ++i) {
        if (insertPosition == textReplacementList.at(i).insertPosition)
            return false; // multiple replacements on the same insertPosition is not allowed.
        if(insertPosition < textReplacementList.at(i).insertPosition)
            break;  //we found the right position
    }
    //++i;
  //  cout << "inserting new text " << newText.constData() << endl;
    // %s at %d overwriting %d bytes at list pos %d\n", newText.constData(), insertPosition, currentLenght, i);
    TextReplacement rep;
    rep.newText=newText;
    rep.insertPosition=insertPosition;
    rep.currentLenght=currentLenght;

    textReplacementList.insert(i, rep);
    return true;
}

void TextReplacements::clear()
{
    textReplacementList.clear();
}

QByteArray TextReplacements::apply(QByteArray current)
{
    QByteArray newData=current;
    int i;
    int replacementOffset=0;

    for(i=0; i<textReplacementList.size(); ++i) {
        TextReplacement rep=textReplacementList.at(i);
        //printf("applying new text %s insert at %d overwriting %d bytes \n", rep.newText.constData(), rep.insertPosition, rep.currentLenght);
        newData.remove(rep.insertPosition+replacementOffset, rep.currentLenght);
        newData.insert(rep.insertPosition+replacementOffset, rep.newText);

        //modify all remaining replacements if we change the document length
        replacementOffset+=(rep.newText.size() - rep.currentLenght);
    }

    return newData;
}

TextReplacements &TextReplacements::operator+=(const TextReplacements &other)
{
    foreach(TextReplacement rep, other.replacements()) {
        insert(rep.newText, rep.insertPosition, rep.currentLenght);
    }
    return *this;
}
