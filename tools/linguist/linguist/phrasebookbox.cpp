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

/*  TRANSLATOR PhraseBookBox

  Go to Phrase > Edit Phrase Book...  The dialog that pops up is a
  PhraseBookBox.
*/

#include "phrasebookbox.h"

#include <qapplication.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qheaderview.h>

#define NewPhrase tr("(New Phrase)")

PhraseBookBox::PhraseBookBox(const QString& filename,
                             const PhraseBook& phraseBook, QWidget *parent)
    : QDialog(parent), blockListSignals(false), fn(filename), pb(phraseBook)
{
    setupUi(this);
    setModal(false);
    source->setBuddy(sourceLed);
    target->setBuddy(targetLed);
    definition->setBuddy(definitionLed);

    phrMdl = new PhraseModel(this);
    phraseList->setModel(phrMdl);
    phraseList->setSelectionBehavior(QAbstractItemView::SelectRows);
    phraseList->setSelectionMode(QAbstractItemView::SingleSelection);
    phraseList->setRootIsDecorated(false);
    phraseList->header()->setResizeMode(QHeaderView::Stretch);

    connect(sourceLed, SIGNAL(textChanged(const QString&)),
        this, SLOT(sourceChanged(const QString&)));
    connect(targetLed, SIGNAL(textChanged(const QString&)),
        this, SLOT(targetChanged(const QString&)));
    connect(definitionLed, SIGNAL(textChanged(const QString&)),
        this, SLOT(definitionChanged(const QString&)));
    connect(phraseList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &,
        const QModelIndex &)), this, SLOT(selectionChanged()));
    connect(newBut, SIGNAL(clicked()), this, SLOT(newPhrase()));
    connect(removeBut, SIGNAL(clicked()), this, SLOT(removePhrase()));
    connect(saveBut, SIGNAL(clicked()), this, SLOT(save()));
    connect(closeBut, SIGNAL(clicked()), this, SLOT(accept()));

    connect(phraseList->header(), SIGNAL(sectionClicked(int, Qt::MouseButton, Qt::KeyboardModifiers)),
        this, SLOT(sortPhrases(int, Qt::MouseButton)));

    foreach(Phrase p, phraseBook) {
        phrMdl->addPhrase(p);
    }

    sortPhrases(0, Qt::LeftButton);
    enableDisable();
}

void PhraseBookBox::sortPhrases(int section, Qt::MouseButton state)
{
    if ((state == Qt::LeftButton) &&
        ((section >= 0) && (section <= 2))) {
        phraseList->clearSelection();
        Qt::SortOrder order;
        int column;

        if ((phrMdl->sortParameters(order, column))) {
            if ((order == Qt::AscendingOrder) && (column == section))
                order = Qt::DescendingOrder;
            else
                order = Qt::AscendingOrder;
        }
        else {
            order = Qt::AscendingOrder;
        }

        phraseList->header()->setSortIndicator(section, order);
        phraseList->header()->setSortIndicatorShown(true);
        phrMdl->sort(section, QModelIndex(), order);
    }
}

void PhraseBookBox::keyPressEvent(QKeyEvent *ev)
{
    // TODO:
    // does not work...
    /*if (ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up ||
        ev->key() == Qt::Key_Next || ev->key() == Qt::Key_Prior)
        QApplication::sendEvent(phraseList, new QKeyEvent(ev->type(),
        ev->key(), ev->state(), ev->text(), ev->isAutoRepeat(), ev->count()));
    else*/
        QDialog::keyPressEvent( ev );
}

void PhraseBookBox::newPhrase()
{
    Phrase ph;
    ph.setSource(NewPhrase);
    selectItem(phrMdl->addPhrase(ph));
}

void PhraseBookBox::removePhrase()
{
    phrMdl->removePhrase(phraseList->currentIndex());
}

void PhraseBookBox::save()
{
    pb.clear();

    QList<Phrase> pl = phrMdl->phraseList();
    Phrase p;

    for (int i=0; i<pl.count(); i++) {
        p = pl.at(i);
        if (!p.source().isEmpty() && p.source() != NewPhrase)
            pb.append(pl.at(i));
    }

    if (!pb.save(fn))
        QMessageBox::warning(this, tr("Qt Linguist"),
        tr("Cannot save phrase book '%1'.").arg(fn));
}

void PhraseBookBox::sourceChanged(const QString& source)
{
    QModelIndex index = phraseList->currentIndex();
    if (index.isValid()) {
        Phrase ph = phrMdl->phrase(index);
        ph.setSource(source);
        phrMdl->setPhrase(index, ph);
        sortAndSelectItem(index);
    }
}

void PhraseBookBox::targetChanged(const QString& target)
{
    QModelIndex index = phraseList->currentIndex();
    if (index.isValid()) {
        Phrase ph = phrMdl->phrase(index);
        ph.setTarget(target);
        phrMdl->setPhrase(index, ph);
        sortAndSelectItem(index);
    }
}

void PhraseBookBox::definitionChanged( const QString& definition )
{
    QModelIndex index = phraseList->currentIndex();
    if (index.isValid()) {
        Phrase ph = phrMdl->phrase(index);
        ph.setDefinition(definition);
        phrMdl->setPhrase(index, ph);
        sortAndSelectItem(index);
    }
}

void PhraseBookBox::sortAndSelectItem(const QModelIndex &index)
{
    Phrase curphr = phrMdl->phrase(index);
    phrMdl->resort();
    QModelIndex newIndex = phrMdl->index(curphr);

    // TODO
    // phraseList->blockSignals(bool) does not work (?)
    blockListSignals = true;
    selectItem(newIndex);
    blockListSignals = false;
}

void PhraseBookBox::selectionChanged()
{
    if (!blockListSignals)
        enableDisable();
}

void PhraseBookBox::selectItem(const QModelIndex &index)
{
    phraseList->ensureVisible(index);
    phraseList->setCurrentIndex(index);
}

void PhraseBookBox::enableDisable()
{
    QModelIndex index = phraseList->currentIndex();

    sourceLed->blockSignals(true);
    targetLed->blockSignals(true);
    definitionLed->blockSignals(true);

    bool indexValid = index.isValid();

    if (indexValid) {
        Phrase p = phrMdl->phrase(index);
        sourceLed->setText(p.source().simplified());
        targetLed->setText(p.target().simplified());
        definitionLed->setText(p.definition());
    }
    else {
        sourceLed->setText(QString::null);
        targetLed->setText(QString::null);
        definitionLed->setText(QString::null);
    }

    sourceLed->setEnabled(indexValid);
    targetLed->setEnabled(indexValid);
    definitionLed->setEnabled(indexValid);
    removeBut->setEnabled(indexValid);

    sourceLed->blockSignals(false);
    targetLed->blockSignals(false);
    definitionLed->blockSignals(false);

    QLineEdit *led = (sourceLed->text() == NewPhrase ? sourceLed : targetLed);
    led->setFocus();
    led->selectAll();
}
