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

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include "phrase.h"
#include "phrasemodel.h"

#include <qdialog.h>
#include "ui_phrasebookbox.h"

class PhraseBookBox : public QDialog, public Ui::PhraseBookBox
{
    Q_OBJECT
public:
    PhraseBookBox(const QString &filename, const PhraseBook &phraseBook,
        QWidget *parent = 0);

    const PhraseBook &phraseBook() const {return pb;}

protected:
    virtual void keyPressEvent(QKeyEvent *ev);

private slots:
    void newPhrase();
    void removePhrase();
    void save();
    void sourceChanged(const QString &source);
    void targetChanged(const QString &target);
    void definitionChanged(const QString &definition);
    void selectionChanged();

private:
    void sortAndSelectItem(const QModelIndex &index);
    void selectItem(const QModelIndex &index);
    void enableDisable();
    bool blockListSignals;

    QString fn;
    PhraseBook pb;
    PhraseModel *phrMdl;
};

#endif
