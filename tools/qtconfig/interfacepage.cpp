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

#include "interfacepage.h"
#include <QSettings>

InterfacePage::InterfacePage(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
#ifndef Q_WS_X11
    gbXIMInputStyle->hide();
#endif

    connect(sbDoubleClickInterval, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(sbCursorFlashTime, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(sbWheelScrollLines, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(gbEffects, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(cmbMenuEffect, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(cmbComboBoxEffect, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(cmbToolTipEffect, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(cmbToolBoxEffect, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(cbEnhancedRTLSupport, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(cmbXIMInputStyle, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(sbMinimumWidth, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(sbMinimumHeight, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));

    load();
}
void InterfacePage::load()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup("Qt");

    sbDoubleClickInterval->setValue(QApplication::doubleClickInterval());
    sbCursorFlashTime->setValue(QApplication::cursorFlashTime());
    sbWheelScrollLines->setValue(QApplication::wheelScrollLines());

    gbEffects->setChecked(true);
    if (!QApplication::isEffectEnabled(Qt::UI_General)) // needs to be done this way because
        gbEffects->setChecked(false); // calling setChecked(false) won't disable the widgets
    const QSize globalStrut = QApplication::globalStrut();
    sbMinimumWidth->setValue(globalStrut.width());
    sbMinimumHeight->setValue(globalStrut.height());
    cmbMenuEffect->clear();
    cmbMenuEffect->addItem(tr("Disable"));
    cmbMenuEffect->addItem(tr("Animate"), QLatin1String("animatemenu"));
    cmbMenuEffect->addItem(tr("Fade"), QLatin1String("fademenu"));
    cmbComboBoxEffect->clear();
    cmbComboBoxEffect->addItem(tr("Disable"));
    cmbComboBoxEffect->addItem(tr("Animate"), QLatin1String("animatecombo"));
    cmbToolTipEffect->clear();
    cmbToolTipEffect->addItem(tr("Disable"));
    cmbToolTipEffect->addItem(tr("Animate"), QLatin1String("animatetooltip"));
    cmbToolTipEffect->addItem(tr("Fade"), QLatin1String("fadetooltip"));
    cmbToolBoxEffect->clear();
    cmbToolBoxEffect->addItem(tr("Disable"));
    cmbToolBoxEffect->addItem(tr("Animate"), QLatin1String("animatetoolbox"));

    if (QApplication::isEffectEnabled(Qt::UI_FadeMenu)) {
        cmbMenuEffect->setCurrentIndex(2);
    } else if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu)) {
        cmbMenuEffect->setCurrentIndex(1);
    } else {
        cmbMenuEffect->setCurrentIndex(0);
    }

    if (QApplication::isEffectEnabled(Qt::UI_AnimateCombo)) {
        cmbComboBoxEffect->setCurrentIndex(1);
    } else {
        cmbComboBoxEffect->setCurrentIndex(0);
    }

    if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)) {
        cmbToolTipEffect->setCurrentIndex(2);
    } else if (QApplication::isEffectEnabled(Qt::UI_AnimateTooltip)) {
        cmbToolTipEffect->setCurrentIndex(1);
    } else {
        cmbToolTipEffect->setCurrentIndex(0);
    }

    if (QApplication::isEffectEnabled(Qt::UI_AnimateToolBox)) {
        cmbToolBoxEffect->setCurrentIndex(1);
    } else {
        cmbToolBoxEffect->setCurrentIndex(0);
    }

    cbEnhancedRTLSupport->setChecked(settings.value(QLatin1String("useRtlExtensions"), false).toBool());


#ifdef Q_WS_X11
    cmbXIMInputStyle->clear();
    cmbXIMInputStyle->addItem(tr("On The Spot"), QLatin1String("On The Spot"));
    cmbXIMInputStyle->addItem(tr("Over The Spot"), QLatin1String("Over The Spot"));
    cmbXIMInputStyle->addItem(tr("Off The Spot"), QLatin1String("Off The Spot"));
    cmbXIMInputStyle->addItem(tr("Root"), QLatin1String("Root"));


    const QString str = settings.value(QLatin1String("XIMInputStyle")).toString();
    if (!str.isEmpty()) {
        const int index = cmbXIMInputStyle->findData(str, Qt::UserRole, Qt::MatchFixedString);
        if (index == -1) {
            qWarning("Malformed settings XIMInputStyle='%s'", qPrintable(str));
        } else {
            cmbXIMInputStyle->setCurrentIndex(index);
        }
    }
    settings.endGroup();
#endif
}

void InterfacePage::save()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup("Qt");
    settings.setValue(QLatin1String("doubleClickInterval"), sbDoubleClickInterval->value());
    settings.setValue(QLatin1String("cursorFlashTime"), (sbCursorFlashTime->value() == sbCursorFlashTime->minimum() ? 0 : sbCursorFlashTime->value()));
    settings.setValue(QLatin1String("wheelScrollLines"), sbWheelScrollLines->value());
    settings.setValue(QLatin1String("globalStrut/width"), sbMinimumWidth->value());
    settings.setValue(QLatin1String("globalStrut/height"), sbMinimumHeight->value());
    settings.setValue(QLatin1String("useRtlExtensions"), cbEnhancedRTLSupport->isChecked());

#ifdef Q_WS_X11
    settings.setValue(QLatin1String("XIMInputStyle"),
                      cmbXIMInputStyle->itemData(cmbXIMInputStyle->currentIndex(), Qt::UserRole).toString());
#endif
    QStringList effects;
    if (gbEffects->isChecked()) {
        effects << QLatin1String("general")
                << cmbMenuEffect->itemData(cmbMenuEffect->currentIndex(), Qt::UserRole).toString()
                << cmbComboBoxEffect->itemData(cmbComboBoxEffect->currentIndex(), Qt::UserRole).toString()
                << cmbToolTipEffect->itemData(cmbToolTipEffect->currentIndex(), Qt::UserRole).toString()
                << cmbToolBoxEffect->itemData(cmbToolBoxEffect->currentIndex(), Qt::UserRole).toString();
    } else {
        effects << QLatin1String("none");
    }
    settings.setValue(QLatin1String("GUIEffects"), effects);
}
