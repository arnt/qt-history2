#include "mainwindow.h"
#include "colorbutton.h"
#include "previewframe.h"
#include "paletteeditoradvanced.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qstylefactory.h>
#include <qobjectlist.h>
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
#include <qstyle.h>

#include <stdlib.h>


// from qapplication.cpp and qapplication_x11.cpp - These are NOT for external use
// ignore them

#ifdef Q_WS_X11
extern bool qt_has_xft;
#endif
extern bool qt_resolve_symlinks;


static const char *appearance_text =
"<p><b><font size+=2>Appearance</font></b></p>"
"<hr>"
"<p>Here you can customize the appearance of your Qt applications.</p>"
"<p>You may select the default GUI Style from the drop down list and "
"customize the colors.</p>"
"<p>Any GUI Style plugins in your plugin path will automatically be added "
"to the list of built-in Qt styles.</p>"
"<p>Upon choosing 3-D Effects and Background colors, the Qt Configutaion "
"program will automatically generate a palette for you.  To customize "
"colors further, press the Tune Palette button to open the advanced "
"palette editor."
"<p>The preview will show your selected Style and colors, allowing "
"you to see how your Qt programs will look.";

static const char *font_text =
"<p><b><font size+=2>Fonts</font></b></p>"
"<hr>"
"<p>Here you can select the default font for Qt applications.  The font "
"you have selected will be previewed in the line edit below the Family, "
"Style and Point Size drop down lists.</p>"
"<p>Qt has a powerful font substitution feature that allows you to "
"specify a list of substitute fonts.  Substitute fonts are used "
"when a font cannot be loaded, or if the specified font doesn't have "
"a particular character."
"<p>For example, you select the font Lucida, which doesn't have Korean "
"characters.  For Korean text, you want to use the Mincho font family. "
"By adding Mincho to the list, any Korean characters not found in "
"Lucida will be used from Mincho.  Because the font substitutions are "
"lists, you can also select multiple families, such as Song Ti (for "
"use with Chinese text).";

static const char *interface_text =
"<p><b><font size+=2>Interface</font></b></p>"
"<hr>"
"<p>Here you can customize the feel of your Qt applications.  (Should I "
"say anything more here?  It should be pretty obvious what the Feel spin "
"boxes and Effects comboboxes do.  Tell us your opinion, Dear User, at "
"<b>qt-bugs@trolltech.com</b>).</p>"
"<p>The Resolve Symlinks settings controls how Qt follows symlinks when "
"handling URLs.  For example, in the file dialog, if this setting is turned "
"on and /usr/tmp is a symlink to /var/tmp, entering the /usr/tmp directory "
"will cause the file dialog to change to /var/tmp.  With this setting turned "
"off, symlinks are not resolved and followed.</p>"
"<p>The Global Strut setting is useful for people that require a "
"minimum size for all widgets (e.g. when using a touch panel or for users "
"that have visual impairments).  Leaving the Global Strut width and height "
"at 0 will disable the Global Strut feature</p>";

static const char *libpath_text =
"<p><b><font size+=2>Library Paths</font></b></p>"
"<hr>"
"<p>Here you can select additional directories where Qt should search "
"for component plugins."
"<p>These directories should be the base directory of where your plugins "
"are stored.  For example, if you wish to store GUI Style plugins in "
"$HOME/plugins/styles and Qt Designer plugins in $HOME/plugins/designer, "
"you would add $HOME/plugins to your Library Path.  NOTE: Qt automatically "
"searches in the directory where you installed Qt for component plugins. "
"Removing that path is not possible.</p>";

static const char *printer_text =
"<p><b><font size+=2>Printer</font></b></p>"
"<hr>"
"<p>Here you can configure the way Qt generates output for the printer."
"You can specify if Qt should try to embed fonts into it's generated output."
"The resulting postscript is more portable and reflects more accurately the"
"visual output on the screen, but the size of the resulting output will be bigger."
"<p>When using font embedding you can select additional directories where"
"Qt should search for embeddable font files.  By default, the X server font path is used, "
"so adding those directories is not needed.</p>";

static const char *about_text =
"<p><b><font size+=4>About Qt Configuration</font></b><br>"
"A graphical configuration tool for programs using Qt.</p>"
"<p>Version 1.0-beta</p>"
"<p>Copyright (C) 2001 Trolltech AS</p>"
"<p></p>"
"<p>This program is licensed to you under the terms of the GNU General "
"Public License Version 2 as published by the Free Software Foundation. This "
"gives you legal permission to copy, distribute and/or modify this software "
"under certain conditions. For details, see the file 'COPYING' that came with "
"this software distribution. If you did not get the file, send email to "
"info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY "
"OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
"FOR A PARTICULAR PURPOSE.</p>";


static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};


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

    QPtrListIterator<QObject> childit(*children);
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
    setIcon( QPixmap(logo_xpm) );

    modified = TRUE;

    QStringList gstyles = QStyleFactory::keys();
    gstylecombo->insertStringList(gstyles);

    QSettings settings;
    QString currentstyle = settings.readEntry("/qt/style");
    if (currentstyle.isNull())
	currentstyle = QApplication::style().className();
    {
	int s = 0;
	QStringList::Iterator git = gstyles.begin();
	while (git != gstyles.end()) {
	    if (*git == currentstyle)
		break;
	    s++;
	    git++;
	}

	if (s < gstylecombo->count()) {
	    gstylecombo->setCurrentItem(s);
	} else {
	    // no predefined style, try to find the closest match
	    // class names usually contain the name of the style, so we
	    // iterate over the items in the combobox, and use the one whose
	    // name is contained in the classname of the style
	    s = 0;
	    git = gstyles.begin();
	    while (git != gstyles.end()) {
		if (currentstyle.contains(*git))
		    break;
		s++;
		git++;
	    }

	    if (s < gstylecombo->count()) {
		gstylecombo->setCurrentItem(s);
	    } else {
		// we give up
		gstylecombo->insertItem("Unknown");
		gstylecombo->setCurrentItem(gstylecombo->count() - 1);
	    }
	}
    }

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
    resolvelinks->setChecked(qt_resolve_symlinks);

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

#ifdef Q_WS_X11
    xftcheckbox->setChecked( settings.readBoolEntry( "/qt/enableXft", TRUE ) );
    if ( xftcheckbox->isChecked() ) {
	aacheckbox->setEnabled( TRUE );
	aacheckbox->setChecked( settings.readBoolEntry( "/qt/useXft", TRUE) );
    } else {
	aacheckbox->setEnabled( FALSE );
	aacheckbox->setChecked( FALSE );
    }
#else
    xftcheckbox->setEnabled(false);
    aacheckbox->setEnabled(false);
#endif

    fontembeddingcheckbox->setChecked( settings.readBoolEntry("/qt/embedFonts", TRUE) );
    fontpaths = settings.readListEntry("/qt/fontPath", ':');
    fontpathlistbox->insertStringList(fontpaths);

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

	QStringList libpath = QApplication::libraryPaths();
	QString QTDIRpath = getenv("QTDIR");
	if ( ! QTDIRpath.isEmpty() ) {
	    QTDIRpath += "/plugins";
	    libpath.remove(QTDIRpath);
	}
	settings.writeEntry("/qt/libraryPath", libpath, ':');
	settings.writeEntry("/qt/fontPath", fontpaths, ':');
	settings.writeEntry("/qt/embedFonts", fontembeddingcheckbox->isChecked() );
	settings.writeEntry("/qt/style", gstylecombo->currentText());
	settings.writeEntry("/qt/enableXft", xftcheckbox->isChecked());
	settings.writeEntry("/qt/useXft", aacheckbox->isChecked());
	settings.writeEntry("/qt/doubleClickInterval",
					     dcispin->value());
	settings.writeEntry("/qt/cursorFlashTime", cfispin->value());
	settings.writeEntry("/qt/wheelScrollLines", wslspin->value());
	settings.writeEntry("/qt/resolveSymlinks", resolvelinks->isChecked());

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


void MainWindow::removeFontpath()
{
    if (fontpathlistbox->currentItem() < 0 ||
	uint(fontpathlistbox->currentItem()) > fontpathlistbox->count())
	return;

    int item = fontpathlistbox->currentItem();
    fontpaths.remove(fontpaths.at(fontpathlistbox->currentItem()));
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    if (uint(item) > fontpathlistbox->count())
	item = int(fontpathlistbox->count()) - 1;
    fontpathlistbox->setCurrentItem(item);
    setModified(TRUE);
}


void MainWindow::addFontpath()
{
    if (fontpathlineedit->text().isEmpty())
	return;

    if (fontpathlistbox->currentItem() < 0 ||
	uint(fontpathlistbox->currentItem()) > fontpathlistbox->count()) {
	fontpaths.append(fontpathlineedit->text());
	fontpathlistbox->clear();
	fontpathlistbox->insertStringList(fontpaths);
	setModified(TRUE);

	return;
    }

    int item = fontpathlistbox->currentItem();
    fontpaths.insert(++fontpaths.at(fontpathlistbox->currentItem()),
		     fontpathlineedit->text());
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item);
    setModified(TRUE);
}


void MainWindow::downFontpath()
{
    if (fontpathlistbox->currentItem() < 0 ||
	uint(fontpathlistbox->currentItem()) >= fontpathlistbox->count() - 1)
	return;

    int item = fontpathlistbox->currentItem();
    QStringList::Iterator it = fontpaths.at(item);
    QString fam = *it;
    fontpaths.remove(it);
    it = fontpaths.at(item);
    fontpaths.insert(++it, fam);
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item + 1);
    setModified(TRUE);
}


void MainWindow::upFontpath()
{
    if (fontpathlistbox->currentItem() < 1)
	return;

    int item = fontpathlistbox->currentItem();
    QStringList::Iterator it = fontpaths.at(item);
    QString fam = *it;
    fontpaths.remove(it);
    it = fontpaths.at(item - 1);
    fontpaths.insert(it, fam);
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item - 1);
    setModified(TRUE);
}


void MainWindow::browseFontpath()
{
    QString dirname = QFileDialog::getExistingDirectory(QString::null, this, 0,
							tr("Select a Directory"));
    if (dirname.isNull())
	return;

   fontpathlineedit->setText(dirname);
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
    else if (page == tab_5)
	helpview->setText(tr(printer_text));
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
