/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input character setup
**
** Created : 20000414
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qdialog.h>
#include <qlist.h>
#include "qimpenchar.h"

class QListBox;
class QPushButton;
class QIMPenWidget;

class QIMPenSetup : public QDialog
{
    Q_OBJECT
public:
    QIMPenSetup( QList<QIMPenCharSet> *cs, QWidget *parent=0,
		const char *name=0, bool modal=FALSE, int WFlags=0 );

protected:
    void fillCharList();
    QIMPenChar *findPrev();
    QIMPenChar *findNext();
    void setCurrentChar( QIMPenChar * );

protected slots:
    void prevChar();
    void nextChar();
    void clearChar();
    void selectChar( int );
    void selectCharSet( int );
    void addChar();
    void removeChar();
    void defaultChars();
    void newStroke( QIMPenStroke * );

protected:
    QIMPenWidget *pw;
    QListBox *charList;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    uint currentCode;
    QIMPenChar *currentChar;
    QIMPenChar *inputChar;
    QIMPenCharSet *currentSet;
    QList<QIMPenCharSet> *charSets;
};

