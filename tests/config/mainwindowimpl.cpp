#include "mainwindowimpl.h"
#include "colorbutton.h"
#include "previewframe.h"
#include "paletteeditoradvancedimpl.h"

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


extern bool qt_has_xft;


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

    if (QApplication::isEffectEnabled(UI_AnimateMenu))
	menueffect->setCurrentItem(1);
    else if (QApplication::isEffectEnabled(UI_FadeMenu))
	menueffect->setCurrentItem(2);

    if (QApplication::isEffectEnabled(UI_AnimateCombo))
	comboeffect->setCurrentItem(1);

    if (QApplication::isEffectEnabled(UI_AnimateTooltip))
	tooltipeffect->setCurrentItem(1);
    else if (QApplication::isEffectEnabled(UI_FadeTooltip))
	tooltipeffect->setCurrentItem(2);

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
    qApp->quit();
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

}


void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Configuration"));
}
