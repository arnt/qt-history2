/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindowbase.h"
#include "colorbutton.h"
#include "previewframe.h"

#include <QVariant>
#include <QImage>
#include <QPixmap>

/*
 *  Constructs a MainWindowBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
MainWindowBase::MainWindowBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : Q3MainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(fontpathlineedit, SIGNAL(returnPressed()), this, SLOT(addFontpath()));
    connect(PushButton15, SIGNAL(clicked()), this, SLOT(addFontpath()));
    connect(PushButton1, SIGNAL(clicked()), this, SLOT(addSubstitute()));
    connect(PushButton14, SIGNAL(clicked()), this, SLOT(browseFontpath()));
    connect(stylecombo, SIGNAL(activated(int)), this, SLOT(buildFont()));
    connect(psizecombo, SIGNAL(activated(int)), this, SLOT(buildFont()));
    connect(PushButton12, SIGNAL(clicked()), this, SLOT(downFontpath()));
    connect(PushButton3, SIGNAL(clicked()), this, SLOT(downSubstitute()));
    connect(familycombo, SIGNAL(activated(QString)), this, SLOT(familySelected(QString)));
    connect(fileExitAction, SIGNAL(activated()), this, SLOT(fileExit()));
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(fileSave()));
    connect(helpAboutAction, SIGNAL(activated()), this, SLOT(helpAbout()));
    connect(helpAboutQtAction, SIGNAL(activated()), this, SLOT(helpAboutQt()));
    connect(TabWidget3, SIGNAL(currentChanged(QWidget*)), this, SLOT(pageChanged(QWidget*)));
    connect(paletteCombo, SIGNAL(activated(int)), this, SLOT(paletteSelected(int)));
    connect(PushButton13, SIGNAL(clicked()), this, SLOT(removeFontpath()));
    connect(PushButton4, SIGNAL(clicked()), this, SLOT(removeSubstitute()));
    connect(effectcheckbox, SIGNAL(toggled(bool)), effectbase, SLOT(setEnabled(bool)));
    connect(fontembeddingcheckbox, SIGNAL(toggled(bool)), GroupBox10, SLOT(setEnabled(bool)));
    connect(toolboxeffect, SIGNAL(activated(int)), this, SLOT(somethingModified()));
    connect(dcispin, SIGNAL(valueChanged(int)), this, SLOT(somethingModified()));
    connect(cfispin, SIGNAL(valueChanged(int)), this, SLOT(somethingModified()));
    connect(wslspin, SIGNAL(valueChanged(int)), this, SLOT(somethingModified()));
    connect(menueffect, SIGNAL(activated(int)), this, SLOT(somethingModified()));
    connect(comboeffect, SIGNAL(activated(int)), this, SLOT(somethingModified()));
    connect(tooltipeffect, SIGNAL(activated(int)), this, SLOT(somethingModified()));
    connect(strutwidth, SIGNAL(valueChanged(int)), this, SLOT(somethingModified()));
    connect(strutheight, SIGNAL(valueChanged(int)), this, SLOT(somethingModified()));
    connect(effectcheckbox, SIGNAL(toggled(bool)), this, SLOT(somethingModified()));
    connect(resolvelinks, SIGNAL(toggled(bool)), this, SLOT(somethingModified()));
    connect(fontembeddingcheckbox, SIGNAL(clicked()), this, SLOT(somethingModified()));
    connect(rtlExtensions, SIGNAL(toggled(bool)), this, SLOT(somethingModified()));
    connect(inputStyle, SIGNAL(activated(int)), this, SLOT(somethingModified()));
    connect(gstylecombo, SIGNAL(activated(QString)), this, SLOT(styleSelected(QString)));
    connect(familysubcombo, SIGNAL(activated(QString)), this, SLOT(substituteSelected(QString)));
    connect(btnAdvanced, SIGNAL(clicked()), this, SLOT(tunePalette()));
    connect(PushButton11, SIGNAL(clicked()), this, SLOT(upFontpath()));
    connect(PushButton2, SIGNAL(clicked()), this, SLOT(upSubstitute()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainWindowBase::~MainWindowBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MainWindowBase::languageChange()
{
    retranslateUi(this);
}

void MainWindowBase::init()
{
}

void MainWindowBase::destroy()
{
}

void MainWindowBase::addFontpath()
{
    qWarning("MainWindowBase::addFontpath(): Not implemented yet");
}

void MainWindowBase::addSubstitute()
{
    qWarning("MainWindowBase::addSubstitute(): Not implemented yet");
}

void MainWindowBase::browseFontpath()
{
    qWarning("MainWindowBase::browseFontpath(): Not implemented yet");
}

void MainWindowBase::buildFont()
{
    qWarning("MainWindowBase::buildFont(): Not implemented yet");
}

void MainWindowBase::buildPalette()
{
    qWarning("MainWindowBase::buildPalette(): Not implemented yet");
}

void MainWindowBase::downFontpath()
{
    qWarning("MainWindowBase::downFontpath(): Not implemented yet");
}

void MainWindowBase::downSubstitute()
{
    qWarning("MainWindowBase::downSubstitute(): Not implemented yet");
}

void MainWindowBase::familySelected( const QString &)
{
    qWarning("MainWindowBase::familySelected( const QString &): Not implemented yet");
}

void MainWindowBase::fileExit()
{
    qWarning("MainWindowBase::fileExit(): Not implemented yet");
}

void MainWindowBase::fileSave()
{
    qWarning("MainWindowBase::fileSave(): Not implemented yet");
}

void MainWindowBase::helpAbout()
{
    qWarning("MainWindowBase::helpAbout(): Not implemented yet");
}

void MainWindowBase::helpAboutQt()
{
    qWarning("MainWindowBase::helpAboutQt(): Not implemented yet");
}

void MainWindowBase::new_slot()
{
    qWarning("MainWindowBase::new_slot(): Not implemented yet");
}

void MainWindowBase::pageChanged( QWidget *)
{
    qWarning("MainWindowBase::pageChanged( QWidget *): Not implemented yet");
}

void MainWindowBase::paletteSelected(int)
{
    qWarning("MainWindowBase::paletteSelected(int): Not implemented yet");
}

void MainWindowBase::removeFontpath()
{
    qWarning("MainWindowBase::removeFontpath(): Not implemented yet");
}

void MainWindowBase::removeSubstitute()
{
    qWarning("MainWindowBase::removeSubstitute(): Not implemented yet");
}

void MainWindowBase::somethingModified()
{
    qWarning("MainWindowBase::somethingModified(): Not implemented yet");
}

void MainWindowBase::styleSelected( const QString &)
{
    qWarning("MainWindowBase::styleSelected( const QString &): Not implemented yet");
}

void MainWindowBase::substituteSelected( const QString &)
{
    qWarning("MainWindowBase::substituteSelected( const QString &): Not implemented yet");
}

void MainWindowBase::tunePalette()
{
    qWarning("MainWindowBase::tunePalette(): Not implemented yet");
}

void MainWindowBase::upFontpath()
{
    qWarning("MainWindowBase::upFontpath(): Not implemented yet");
}

void MainWindowBase::upSubstitute()
{
    qWarning("MainWindowBase::upSubstitute(): Not implemented yet");
}
