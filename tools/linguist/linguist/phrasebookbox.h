/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include "phrase.h"

#include <qdialog.h>
#include <qvaluelist.h>

class QLineEdit;
class QPushButton;
class QListViewItem;

class PhraseLV;

class PhraseBookBox : public QDialog
{
    Q_OBJECT
public:
    PhraseBookBox( const QString& filename, const PhraseBook& phraseBook,
		   QWidget *parent = 0, const char *name = 0,
		   bool modal = FALSE );

    const PhraseBook& phraseBook() const { return pb; }

protected:
    virtual void keyPressEvent( QKeyEvent *ev );

private slots:
    void newPhrase();
    void removePhrase();
    void save();
    void sourceChanged( const QString& source );
    void targetChanged( const QString& target );
    void definitionChanged( const QString& definition );
    void selectionChanged( QListViewItem *item );

private:
    void selectItem( QListViewItem *item );
    void enableDisable();

    QLineEdit *sourceLed;
    QLineEdit *targetLed;
    QLineEdit *definitionLed;
    QPushButton *newBut;
    QPushButton *removeBut;
    PhraseLV *lv;
    QString fn;
    PhraseBook pb;
};

#endif
