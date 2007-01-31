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

#include "fontspage.h"
#include <QFontDatabase>
#include <QStringList>
#include <QFont>
#include <QDebug>
#include <QSettings>

FontsPage::FontsPage(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
    connect(cmbFamily, SIGNAL(activated(QString)), this, SLOT(onFamilyChanged(QString)));
    connect(cmbFontStyle, SIGNAL(activated(QString)), this, SLOT(onFontStyleChanged(QString)));
    connect(cmbPointSize, SIGNAL(activated(int)), this, SLOT(onPointSizeChanged()));
    connect(cmbSubstituteSource, SIGNAL(activated(QString)), this, SLOT(onSubsituteSourceSelected(QString)));
    connect(cmbSubstituteSource, SIGNAL(editTextChanged(QString)), this, SLOT(onSubsituteSourceSelected(QString)));
    connect(tbUp, SIGNAL(clicked()), lwSubstitutions, SLOT(moveCurrentUp()));
    connect(tbDown, SIGNAL(clicked()), lwSubstitutions, SLOT(moveCurrentDown()));
    connect(pbAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(pbRemove, SIGNAL(clicked()), this, SLOT(remove()));
    connect(lwSubstitutions, SIGNAL(changed()), this, SLOT(updateSubstitutions()));
    connect(lwSubstitutions, SIGNAL(currentRowChanged(int)), this, SLOT(onCurrentSubstitutionChanged()));
    connect(cmbSubstituteSource, SIGNAL(editTextChanged(QString)), this, SLOT(onSubsituteSourceSelected(QString)));
    load();
}

static QFont fontFromStyleString(const QString &/*family*/, const QString &str)
{
    QFont res;//(family); // ### doesn't really look that great
    if (str.startsWith(QLatin1String("Black"))) {
        res.setWeight(QFont::Black);
    } else if (str.startsWith(QLatin1String("Bold"))) {
        res.setWeight(QFont::Bold);
    } else if (str.startsWith(QLatin1String("Demi Bold"))) {
        res.setWeight(QFont::DemiBold);
    } else if (str.startsWith(QLatin1String("Light"))) {
        res.setWeight(QFont::Light);
    }

    if (str.contains(QLatin1String("Italic"))) {
        res.setStyle(QFont::StyleItalic);
    } else if (str.contains(QLatin1String("Oblique"))) {
        res.setStyle(QFont::StyleOblique);
    }
    return res;
}


void FontsPage::onFamilyChanged(const QString &family)
{
    const QString old = cmbFontStyle->currentText();

    cmbFontStyle->clear();
    cmbFontStyle->addItems(QFontDatabase().styles(family));
    for (int i=0; i<cmbFontStyle->count(); ++i) {
        if (cmbFontStyle->itemText(i) == old) {
            cmbFontStyle->setCurrentIndex(i);
        }
        cmbFontStyle->setItemData(i, fontFromStyleString(family, cmbFontStyle->itemText(i)), Qt::FontRole);
    }

    cmbFamily->setFont(QFont(family));
    updateFontLineEdit();
    emit changed();
}

void FontsPage::onPointSizeChanged()
{
    updateFontLineEdit();
    emit changed();
}


void FontsPage::updateFontLineEdit()
{
    leSampleText->setFont(QFontDatabase().font(cmbFamily->currentText(),
                                               cmbFontStyle->currentText(),
                                               cmbPointSize->currentText().toInt()));
}
void FontsPage::onFontStyleChanged(const QString &/*fontStyle*/)
{
    updateFontLineEdit();
    cmbFontStyle->setFont(qvariant_cast<QFont>(cmbFontStyle->itemData(cmbFontStyle->currentIndex(), Qt::FontRole)));
    emit changed();
}
void FontsPage::onSubsituteSourceSelected(const QString &family)
{
    lwSubstitutions->setItems(substitutions.contains(family) ? substitutions.value(family) : QFont::substitutes(family));
    onCurrentSubstitutionChanged();
}

void FontsPage::onCurrentSubstitutionChanged()
{
    pbRemove->setEnabled(lwSubstitutions->currentItem() != 0);
    tbUp->setEnabled(lwSubstitutions->isUpEnabled());
    tbDown->setEnabled(lwSubstitutions->isDownEnabled());
}
void FontsPage::add()
{
    lwSubstitutions->addItem(cmbSubstituteTarget->currentText());
    updateSubstitutions();
}
void FontsPage::remove()
{
    if (lwSubstitutions->currentItem()) {
        delete lwSubstitutions->currentItem();
        updateSubstitutions();
    }
}
void FontsPage::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
    switch (e->key()) {
    case Qt::Key_Delete:
        if (e->modifiers() == Qt::NoModifier) {
            remove();
            e->accept();
        }
        break;
    case Qt::Key_D:
        if (e->modifiers() == Qt::ControlModifier) {
            remove();
            e->accept();
        }
        break;
    default:
        break;
    }
}
void FontsPage::updateSubstitutions()
{
    substitutions[cmbSubstituteSource->currentText()] = lwSubstitutions->items();
    emit changed();
}
void FontsPage::save()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    QFontDatabase db;
    const QFont font = db.font(cmbFamily->currentText(),
                               cmbFontStyle->currentText(),
                               cmbPointSize->currentText().toInt());
    settings.setValue(QLatin1String("font"), font.toString());
    for (QMap<QString, QStringList>::const_iterator it = substitutions.begin(); it != substitutions.end(); ++it) {
        QFont::removeSubstitution(it.key());
        QFont::insertSubstitutions(it.key(), it.value());
    }

    const QStringList subs = QFont::substitutions();
    settings.beginGroup(QLatin1String("Font Substitutions"));
    for (int i=0; i<subs.size(); ++i) {
        settings.setValue(subs.at(i), QFont::substitutes(subs.at(i)));
    }
    settings.endGroup(); // Font Substitutions
    settings.endGroup(); // Qt
    QApplication::setFont(font);
}
void FontsPage::load()
{
    substitutions.clear();
    cmbSubstituteSource->setCurrentIndex(0);

    cmbFontStyle->clear();

    const QFont font;
    cmbFamily->setCurrentFont(font); // should already be done
    onFamilyChanged(cmbFamily->currentText());

    QFontDatabase db;
    const QString styleString = db.styleString(font);
    int found = 0;
    for (int i=0; i<cmbFontStyle->count(); ++i) {
        if (cmbFontStyle->itemText(i).contains(styleString)) {
            if (cmbFontStyle->itemText(i) == styleString) {
                found = i;
                break;
            } else {
                found = i;
            }
        }
    }
    cmbFontStyle->setCurrentIndex(found);

    const int fontSize = font.pointSize();

    QList<int> sizes = db.standardSizes();
    found = -1;
    for (int i=0; i<sizes.size(); ++i) {
        if (found == -1) {
            if (sizes.at(i) == fontSize) {
                found = i;
            } else if (sizes.at(i) > fontSize) {
                cmbPointSize->addItem(QString::number(fontSize));
                found = i;
            }
        }
        cmbPointSize->addItem(QString::number(sizes.at(i)));
    }
    cmbPointSize->setCurrentIndex(found);
}
