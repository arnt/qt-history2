#include "mainwindow.h"
#include "colorbutton.h"
#include "previewframe.h"
#include "paletteeditoradvanced.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qstylefactory.h>
#include <qobjcoll.h>
#include <qfontdatabase.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qsettings.h>
#include <qmessagebox.h>
#include <qtextview.h>


extern bool qt_has_xft;


static const char *appearance_text =
"<p><b><font size+=2>Appearance</font></b></p>
<hr>

<p>Here you can customize the appearance of your Qt applications.</p>
<p>You may select the default GUI Style from the drop down list and
customize the colors.</p>

<p>Any GUI Style plugins in your plugin path will automatically be added
to the list of built-in Qt styles.</p>

<p>Upon choosing 3-D Effects and Background colors, the Qt Configutaion
program will automatically generate a palette for you.  To customize
colors further, press the Tune Palette button to open the advanced
palette editor.

<p>The preview will show your selected Style and colors, allowing
you to see how your Qt programs will look.";

static const char *font_text =
"<p><b><font size+=2>Fonts</font></b></p>
<hr>

<p>Here you can select the default font for Qt applications.  The font
you have selected will be previewed in the line edit below the Family,
Style and Point Size drop down lists.</p>

<p>Qt has a powerful font substitution feature that allows you to
specify a list of substitute fonts.  Substitute fonts are used
when a font cannot be loaded, or if the specified font doesn't have
a particular character.

<p>For example, you select the font Lucida, which doesn't have Korean
characters.  For Korean text, you want to use the Mincho font family.
By adding Mincho to the list, any Korean characters not found in
Lucida will be used from Mincho.  Because the font substitutions are
lists, you can also select multiple families, such as Song Ti (for
use with Chinese text).";

static const char *interface_text =
"<p><b><font size+=2>Interface</font></b></p>
<hr>

<p>Here you can customize the feel of your Qt applications.  (Should I
say anything more here?  It should be pretty obvious what the Feel spin
boxes and Effects comboboxes do.  Tell us your opinion, Dear User, at
<b>qt-bugs@trolltech.com</b>).</p>

<p>The Global Strut setting is useful for people that require a
minimum size for all widgets (e.g. when using a touch panel or for users
that have visual impairments).  Leaving the Global Strut width and height
at 0 will disable the Global Strut feature</p>";

static const char *libpath_text =
"<p><b><font size+=2>Library Paths</font></b></p>
<hr>

<p>Here you can select additional directories where Qt should search
for component plugins.

<p>These directories should be the base directory of where your plugins
are stored.  For example, if you wish to store GUI Style plugins in
$HOME/plugins/styles and Qt Designer plugins in $HOME/plugins/designer,
you would add $HOME/plugins to your Library Path.  NOTE: Qt automatically
searches in the directory where you installed Qt for component plugins.
Removing that path is not possible.</p>";

static const char *about_text =
"<p><b><font size+=4>About Qt Configuration</font></b><br>
A graphical configuration tool for programs using Qt.</p>
<p>Version 1.0 (almost)</p>
<p>Copyright (c) 2001 Trolltech AS</p>
<p></p>
<p>This program is licensed to you under the terms of the GNU General
Public License Version 2 as published by the Free Software Foundation. This
gives you legal permission to copy, distribute and/or modify this software
under certain conditions. For details, see the file 'COPYING' that came with
this software distribution. If you did not get the file, send email to
info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY
OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE.</p>";


static QColorGroup::ColorRole centralFromItem( int item )
{
    switch( item ) {
    case 0:  return QColorGroup::Background;
    case 1:  return QColorGroup::Foreground;
    case 2:  return QColorGroup::Button;
    case 3:  return QColorGroup::Base;
    case 4:  return QColorGroup::Text;
    case 5:  return QColorGroup::BrightText;
    case 6:  return QColorGroup::ButtonText;
    case 7:  return QColorGroup::Highlight;
    case 8:  return QColorGroup::HighlightedText;
    default: return QColorGroup::NColorRoles;
    }
}


static QColorGroup::ColorRole effectFromItem( int item )
{
    switch( item ) {
    case 0:  return QColorGroup::Light;
    case 1:  return QColorGroup::Midlight;
    case 2:  return QColorGroup::Mid;
    case 3:  return QColorGroup::Dark;
    case 4:  return QColorGroup::Shadow;
    default: return QColorGroup::NColorRoles;
    }
}


static void setStyleHelper(QWidget *w, QStyle *s)
{
    w->setStyle(s);

    const QObjectList *children = w->children();
    if (! children)
	return;

    QListIterator<QObject> childit(*children);
    QObject *child;
    while ((child = childit.current()) != 0) {
	++childit;

	if (child->isWidgetType())
	    setStyleHelper((QWidget *) child, s);
    }
}


MainWindow::MainWindow()
    : MainWindowBase(0, "main window"),
      editPalette(palette()), previewPalette(palette()), previewstyle(0)
{
    modified = TRUE;

    gstylecombo->insertStringList(QStyleFactory::styles());

    buttonMainColor->setColor(palette().color(QPalette::Active,
					      QColorGroup::Button));
    buttonMainColor2->setColor(palette().color(QPalette::Active,
					       QColorGroup::Background));

    QFontDatabase db;
    QStringList families = db.families();
    familycombo->insertStringList(families);

    QStringList fs = families;
    QStringList fs2 = QFont::substitutions();
    QStringList::Iterator fsit = fs2.begin();
    while (fsit != fs2.end()) {
	if (! fs.contains(*fsit))
	    fs += *fsit;
	fsit++;
    }
    fs.sort();
    familysubcombo->insertStringList(fs);

    choosesubcombo->insertStringList(families);
    QValueList<int> sizes = db.standardSizes();
    QValueList<int>::Iterator it = sizes.begin();
    while (it != sizes.end())
	psizecombo->insertItem(QString::number(*it++));

    dcispin->setValue(QApplication::doubleClickInterval());
    cfispin->setValue(QApplication::cursorFlashTime());
    wslspin->setValue(QApplication::wheelScrollLines());

    effectcheckbox->setChecked(QApplication::isEffectEnabled(UI_General));
    effectbase->setEnabled(effectcheckbox->isChecked());

    if (QApplication::isEffectEnabled(UI_FadeMenu))
	menueffect->setCurrentItem(2);
    else if (QApplication::isEffectEnabled(UI_AnimateMenu))
	menueffect->setCurrentItem(1);

    if (QApplication::isEffectEnabled(UI_AnimateCombo))
	comboeffect->setCurrentItem(1);

    if (QApplication::isEffectEnabled(UI_FadeTooltip))
	tooltipeffect->setCurrentItem(2);
    else if (QApplication::isEffectEnabled(UI_AnimateTooltip))
	tooltipeffect->setCurrentItem(1);

    strutwidth->setValue(QApplication::globalStrut().width());
    strutheight->setValue(QApplication::globalStrut().height());

    libpathlistbox->clear();
    libpathlistbox->insertStringList(QApplication::libraryPaths());

    // find the default family
    QStringList::Iterator sit = families.begin();
    int i = 0, possible = -1;
    while (sit != families.end()) {
	if (*sit == QApplication::font().family())
	    break;
	if ((*sit).contains(QApplication::font().family()))
	    possible = i;

	i++;
	sit++;
    }
    if (sit == families.end())
	i = possible;
    if (i == -1) // no clue about the current font
	i = 0;

    familycombo->setCurrentItem(i);

    QStringList styles = db.styles(familycombo->currentText());
    stylecombo->insertStringList(styles);

    QString stylestring = db.styleString(QApplication::font());
    sit = styles.begin();
    i = 0;
    possible = -1;
    while (sit != styles.end()) {
	if (*sit == stylestring)
	    break;
	if ((*sit).contains(stylestring))
	    possible = i;

	i++;
	sit++;
    }
    if (sit == styles.end())
	i = possible;
    if (i == -1) // no clue about the current font
	i = 0;
    stylecombo->setCurrentItem(i);

    i = 0;
    while (i < psizecombo->count()) {
	if (psizecombo->text(i) == QString::number(QApplication::font().pointSize())) {
	    psizecombo->setCurrentItem(i);
	    break;
	}

	i++;
    }

    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    sublistbox->clear();
    sublistbox->insertStringList(subs);

    xftcheckbox->setChecked(qt_has_xft);

    setModified(FALSE);
}


MainWindow::~MainWindow()
{
}


void MainWindow::fileSave()
{
    if (! modified) {
	statusBar()->message("No changes to be saved.", 2000);
	return;
    }

    statusBar()->message("Saving changes...");

    {
	QSettings settings;
	QFontDatabase db;
	QFont font = db.font(familycombo->currentText(),
			     stylecombo->currentText(),
			     psizecombo->currentText().toInt());
	QStringList actcg, inactcg, discg;

	int i;
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    actcg << editPalette.color(QPalette::Active,
				       (QColorGroup::ColorRole) i).name();
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    inactcg << editPalette.color(QPalette::Inactive,
					 (QColorGroup::ColorRole) i).name();
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    discg << editPalette.color(QPalette::Disabled,
				       (QColorGroup::ColorRole) i).name();

	settings.writeEntry("/qt/font", font.toString());
	settings.writeEntry("/qt/Palette/active", actcg);
	settings.writeEntry("/qt/Palette/inactive", inactcg);
	settings.writeEntry("/qt/Palette/disabled", discg);
	settings.writeEntry("/qt/libraryPath", QApplication::libraryPaths(), ':');
	settings.writeEntry("/qt/style", gstylecombo->currentText());
	settings.writeEntry("/qt/useXft", xftcheckbox->isChecked());
	settings.writeEntry("/qt/doubleClickInterval",
					     dcispin->value());
	settings.writeEntry("/qt/cursorFlashTime", cfispin->value());
	settings.writeEntry("/qt/wheelScrollLines", wslspin->value());

	QStringList strut;
	strut << QString::number(strutwidth->value());
	strut << QString::number(strutheight->value());
	settings.writeEntry("/qt/globalStrut", strut);

	QStringList effects;
	if (effectcheckbox->isChecked()) {
	    effects << "general";

	    switch (menueffect->currentItem()) {
	    case 1: effects << "animatemenu"; break;
	    case 2: effects << "fademenu"; break;
	    }

	    switch (comboeffect->currentItem()) {
	    case 1: effects << "animatecombo"; break;
	    }

	    switch (tooltipeffect->currentItem()) {
	    case 1: effects << "animatetooltip"; break;
	    case 2: effects << "fadetooltip"; break;
	    }
	} else
	    effects << "none";
	settings.writeEntry("/qt/GUIEffects", effects);

	QStringList familysubs = QFont::substitutions();
	QStringList::Iterator fit = familysubs.begin();
	while (fit != familysubs.end()) {
	    QStringList subs = QFont::substitutes(*fit);
	    settings.writeEntry("/qt/Font Substitutions/" + *fit, subs);
	    fit++;
	}
    }

#if defined(Q_WS_X11)
    QApplication::x11_apply_settings();
#endif // Q_WS_X11

    setModified(FALSE);
    statusBar()->message("Saved changes.");
}


void MainWindow::fileExit()
{
    qApp->closeAllWindows();
}


void MainWindow::setModified(bool m)
{
    if (modified == m)
	return;

    modified = m;
    fileSaveAction->setEnabled(m);
}


void MainWindow::buildPalette()
{
    int i;
    QColorGroup cg;
    QColor btn = buttonMainColor->color();
    QColor back = buttonMainColor2->color();
    QPalette automake( btn, back );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), automake.active().color( centralFromItem(i) ) );

    editPalette.setActive( cg );
    buildActiveEffect();

    cg = editPalette.inactive();

    QPalette temp( editPalette.active().color( QColorGroup::Button ),
		   editPalette.active().color( QColorGroup::Background ) );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.inactive().color( centralFromItem(i) ) );

    editPalette.setInactive( cg );
    buildInactiveEffect();

    cg = editPalette.disabled();

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.disabled().color( centralFromItem(i) ) );

    editPalette.setDisabled( cg );
    buildDisabledEffect();

    updateColorButtons();

    setModified(TRUE);
}


void MainWindow::buildActiveEffect()
{
    QColorGroup cg = editPalette.active();
    QColor btn = cg.color( QColorGroup::Button );

    QPalette temp( btn, btn );

    for (int i = 0; i<5; i++)
	cg.setColor( effectFromItem(i), temp.active().color( effectFromItem(i) ) );

    editPalette.setActive( cg );
    setPreviewPalette( editPalette );

    updateColorButtons();
}


void MainWindow::buildInactive()
{
    editPalette.setInactive( editPalette.active() );
    buildInactiveEffect();
}


void MainWindow::buildInactiveEffect()
{
    QColorGroup cg = editPalette.inactive();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QColorGroup::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QColorGroup::Light, light );
    cg.setColor( QColorGroup::Midlight, midlight );
    cg.setColor( QColorGroup::Mid, mid );
    cg.setColor( QColorGroup::Dark, dark );
    cg.setColor( QColorGroup::Shadow, shadow );

    editPalette.setInactive( cg );
    setPreviewPalette( editPalette );
    updateColorButtons();
}


void MainWindow::buildDisabled()
{
    QColorGroup cg = editPalette.active();
    cg.setColor( QColorGroup::ButtonText, darkGray );
    cg.setColor( QColorGroup::Foreground, darkGray );
    cg.setColor( QColorGroup::Text, darkGray );
    cg.setColor( QColorGroup::HighlightedText, darkGray );
    editPalette.setDisabled( cg );

    buildDisabledEffect();
}


void MainWindow::buildDisabledEffect()
{
    QColorGroup cg = editPalette.disabled();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QColorGroup::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QColorGroup::Light, light );
    cg.setColor( QColorGroup::Midlight, midlight );
    cg.setColor( QColorGroup::Mid, mid );
    cg.setColor( QColorGroup::Dark, dark );
    cg.setColor( QColorGroup::Shadow, shadow );

    editPalette.setDisabled( cg );
    setPreviewPalette( editPalette );
    updateColorButtons();
}


void MainWindow::setPreviewPalette( const QPalette& pal )
{
    QColorGroup cg;

    switch (paletteCombo->currentItem()) {
    case 0:
    default:
	cg = pal.active();
	break;
    case 1:
	cg = pal.inactive();
	break;
    case 2:
	cg = pal.disabled();
	break;
    }
    previewPalette.setActive( cg );
    previewPalette.setInactive( cg );
    previewPalette.setDisabled( cg );

    previewFrame->setPreviewPalette(previewPalette);
}


void MainWindow::updateColorButtons()
{
    buttonMainColor->setColor( editPalette.active().color( QColorGroup::Button ));
    buttonMainColor2->setColor( editPalette.active().color( QColorGroup::Background ));
}


void MainWindow::tunePalette()
{
    bool ok;
    QPalette pal = PaletteEditorAdvanced::getPalette(&ok, editPalette,
						     backgroundMode(), this);
    if (! ok)
	return;

    editPalette = pal;
    setPreviewPalette(editPalette);
    setModified(TRUE);
}


void MainWindow::paletteSelected(int)
{
    setPreviewPalette(editPalette);
}


void MainWindow::styleSelected(const QString &stylename)
{
    QStyle *style = QStyleFactory::create(stylename);
    if (! style)
	return;

    setStyleHelper(previewFrame, style);
    delete previewstyle;
    previewstyle = style;

    setModified(TRUE);
}


void MainWindow::familySelected(const QString &family)
{
    QFontDatabase db;
    QStringList styles = db.styles(family);
    stylecombo->clear();
    stylecombo->insertStringList(styles);
    familysubcombo->insertItem(family);
    buildFont();
}


void MainWindow::buildFont()
{
    QFontDatabase db;
    QFont font = db.font(familycombo->currentText(),
			 stylecombo->currentText(),
			 psizecombo->currentText().toInt());
    samplelineedit->setFont(font);
    setModified(TRUE);
}


void MainWindow::substituteSelected(const QString &family)
{
    QStringList subs = QFont::substitutes(family);
    sublistbox->clear();
    sublistbox->insertStringList(subs);
}


void MainWindow::removeSubstitute()
{
    if (sublistbox->currentItem() < 0 ||
	uint(sublistbox->currentItem()) > sublistbox->count())
	return;

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    subs.remove(subs.at(sublistbox->currentItem()));
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    if (uint(item) > sublistbox->count())
	item = int(sublistbox->count()) - 1;
    sublistbox->setCurrentItem(item);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(TRUE);
}


void MainWindow::addSubstitute()
{
    if (sublistbox->currentItem() < 0 ||
	uint(sublistbox->currentItem()) > sublistbox->count()) {
	QStringList subs = QFont::substitutes(familysubcombo->currentText());
	subs.append(choosesubcombo->currentText());
	sublistbox->clear();
	sublistbox->insertStringList(subs);
	QFont::removeSubstitution(familysubcombo->currentText());
	QFont::insertSubstitutions(familysubcombo->currentText(), subs);
	setModified(TRUE);

	return;
    }

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    subs.insert(++subs.at(sublistbox->currentItem()), choosesubcombo->currentText());
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(TRUE);
}


void MainWindow::downSubstitute()
{
    if (sublistbox->currentItem() < 0 ||
	uint(sublistbox->currentItem()) >= sublistbox->count())
	return;

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    QStringList::Iterator it = subs.at(item);
    QString fam = *it;
    subs.remove(it);
    it = subs.at(item);
    subs.insert(++it, fam);
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item + 1);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(TRUE);
}


void MainWindow::upSubstitute()
{
    if (sublistbox->currentItem() < 1)
	return;

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    QStringList::Iterator it = subs.at(item);
    QString fam = *it;
    subs.remove(it);
    it = subs.at(item - 1);
    subs.insert(it, fam);
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item - 1);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(TRUE);
}


void MainWindow::removeLibpath()
{
    if (libpathlistbox->currentItem() < 0 ||
	uint(libpathlistbox->currentItem()) > libpathlistbox->count())
	return;

    int item = libpathlistbox->currentItem();
    QStringList paths = QApplication::libraryPaths();
    paths.remove(paths.at(libpathlistbox->currentItem()));
    libpathlistbox->clear();
    libpathlistbox->insertStringList(paths);
    if (uint(item) > libpathlistbox->count())
	item = int(libpathlistbox->count()) - 1;
    libpathlistbox->setCurrentItem(item);
    QApplication::setLibraryPaths(paths);
    setModified(TRUE);
}


void MainWindow::addLibpath()
{
    if (libpathlineedit->text().isEmpty())
	return;

    if (libpathlistbox->currentItem() < 0 ||
	uint(libpathlistbox->currentItem()) > libpathlistbox->count()) {
	QStringList paths = QApplication::libraryPaths();
	paths.append(libpathlineedit->text());
	libpathlistbox->clear();
	libpathlistbox->insertStringList(paths);
	QApplication::setLibraryPaths(paths);
	setModified(TRUE);

	return;
    }

    int item = libpathlistbox->currentItem();
    QStringList paths =QApplication::libraryPaths();
    paths.insert(++paths.at(libpathlistbox->currentItem()),
		 libpathlineedit->text());
    libpathlistbox->clear();
    libpathlistbox->insertStringList(paths);
    libpathlistbox->setCurrentItem(item);
    QApplication::setLibraryPaths(paths);
    setModified(TRUE);
}


void MainWindow::downLibpath()
{
    if (libpathlistbox->currentItem() < 0 ||
	uint(libpathlistbox->currentItem()) >= libpathlistbox->count() - 1)
	return;

    int item = libpathlistbox->currentItem();
    QStringList paths = QApplication::libraryPaths();
    QStringList::Iterator it = paths.at(item);
    QString fam = *it;
    paths.remove(it);
    it = paths.at(item);
    paths.insert(++it, fam);
    libpathlistbox->clear();
    libpathlistbox->insertStringList(paths);
    libpathlistbox->setCurrentItem(item + 1);
    QApplication::setLibraryPaths(paths);
    setModified(TRUE);
}


void MainWindow::upLibpath()
{
    if (libpathlistbox->currentItem() < 1)
	return;

    int item = libpathlistbox->currentItem();
    QStringList paths = QApplication::libraryPaths();
    QStringList::Iterator it = paths.at(item);
    QString fam = *it;
    paths.remove(it);
    it = paths.at(item - 1);
    paths.insert(it, fam);
    libpathlistbox->clear();
    libpathlistbox->insertStringList(paths);
    libpathlistbox->setCurrentItem(item - 1);
    QApplication::setLibraryPaths(paths);
    setModified(TRUE);
}


void MainWindow::browseLibpath()
{
    QString dirname = QFileDialog::getExistingDirectory(QString::null, this, 0,
							tr("Select a Directory"));
    if (dirname.isNull())
	return;

    libpathlineedit->setText(dirname);
}


void MainWindow::somethingModified()
{
    setModified(TRUE);
}


void MainWindow::helpAbout()
{
    QMessageBox::about(this, tr("Qt Configuration"),
		       tr(about_text));
}


void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Configuration"));
}


void MainWindow::pageChanged(QWidget *page)
{
    if (page == tab)
	helpview->setText(tr(appearance_text));
    else if (page == tab_2)
	helpview->setText(tr(font_text));
    else if (page == tab_3)
	helpview->setText(tr(interface_text));
    else if (page == tab_4)
	helpview->setText(tr(libpath_text));
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    if (modified) {
	switch(QMessageBox::warning(this, tr("Save Changes"),
				    tr("Save changes to settings?"),
				    tr("&Yes"), tr("&No"), tr("&Cancel"), 0, 2)) {
	case 0: // save
	    qApp->processEvents();
	    fileSave();

	    // fall through intended
	case 1: // don't save
	    e->accept();
	    break;

	case 2: // cancel
	    e->ignore();
	    break;

	default: break;
	}
    } else
	e->accept();
}
