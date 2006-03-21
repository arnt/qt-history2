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

#include "mainwindow.h"
#include "colorbutton.h"
#include "previewframe.h"
#include "paletteeditoradvanced.h"

#include <QLabel>
#include <QApplication>
#include <QComboBox>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QAction>
#include <QStatusBar>
#include <QSettings>
#include <QMessageBox>
#include <QStyle>
#include <QtEvents>
#include <Q3ValueList>
#include <QtDebug>

#include <stdlib.h>

// from qapplication.cpp and qapplication_x11.cpp - These are NOT for
// external use ignore them
// extern bool Q_CORE_EXPORT qt_resolve_symlinks;

static const char *appearance_text =
"<p><b><font size+=2>Appearance</font></b></p>"
"<hr>"
"<p>Use this tab to customize the appearance of your Qt applications.</p>"
"<p>You can select the default GUI Style from the drop down list and "
"customize the colors.</p>"
"<p>Any GUI Style plugins in your plugin path will automatically be added "
"to the list of built-in Qt styles. (See the Library Paths tab for "
"information on adding new plugin paths.)</p>"
"<p>When you choose 3-D Effects and Background colors, the Qt Configuration "
"program will automatically generate a palette for you.  To customize "
"colors further, press the Tune Palette button to open the advanced "
"palette editor."
"<p>The Preview Window shows what the selected Style and colors look "
"like.";

static const char *font_text =
"<p><b><font size+=2>Fonts</font></b></p>"
"<hr>"
"<p>Use this tab to select the default font for your Qt applications. "
"The selected font is shown (initially as 'Sample Text') in the line "
"edit below the Family, "
"Style and Point Size drop down lists.</p>"
"<p>Qt has a powerful font substitution feature that allows you to "
"specify a list of substitute fonts.  Substitute fonts are used "
"when a font cannot be loaded, or if the specified font doesn't have "
"a particular character."
"<p>For example, if you select the font Lucida, which doesn't have Korean "
"characters, but need to show some Korean text using the Mincho font family "
"you can do so by adding Mincho to the list. Once Mincho is added, any "
"Korean characters that are not found in the Lucida font will be taken "
"from the Mincho font.  Because the font substitutions are "
"lists, you can also select multiple families, such as Song Ti (for "
"use with Chinese text).";

static const char *interface_text =
"<p><b><font size+=2>Interface</font></b></p>"
"<hr>"
"<p>Use this tab to customize the feel of your Qt applications.</p>"
"<p>If the Resolve Symlinks checkbox is checked Qt will follow symlinks "
"when handling URLs. For example, in the file dialog, if this setting is turned "
"on and /usr/tmp is a symlink to /var/tmp, entering the /usr/tmp directory "
"will cause the file dialog to change to /var/tmp.  With this setting turned "
"off, symlinks are not resolved or followed.</p>"
"<p>The Global Strut setting is useful for people who require a "
"minimum size for all widgets (e.g. when using a touch panel or for users "
"who are visually impaired).  Leaving the Global Strut width and height "
"at 0 will disable the Global Strut feature</p>"
"<p>XIM (Extended Input Methods) are used for entering characters in "
"languages that have large character sets, for example, Chinese and "
"Japanese.";
// ### What does the 'Enhanced support for languages written R2L do?

static const char *printer_text =
"<p><b><font size+=2>Printer</font></b></p>"
"<hr>"
"<p>Use this tab to configure the way Qt generates output for the printer."
"You can specify if Qt should try to embed fonts into its generated output."
"If you enable font embedding, the resulting postscript will be more "
"portable and will more accurately reflect the "
"visual output on the screen; however the resulting postscript file "
"size will be bigger."
"<p>When using font embedding you can select additional directories where "
"Qt should search for embeddable font files.  By default, the X "
"server font path is used.";


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
    const QObjectList children = w->children();
    for (int i = 0; i < children.size(); ++i) {
        QObject *child = children.at(i);
        if (child->isWidgetType())
            setStyleHelper((QWidget *) child, s);
    }
    w->setStyle(s);
}


MainWindow::MainWindow()
    : MainWindowBase(0, "main window"),
      editPalette(palette()), previewPalette(palette()), previewstyle(0)
{
    modified = true;

    QStringList gstyles = QStyleFactory::keys();
    gstyles.sort();
    gstylecombo->insertStringList(gstyles);

    QSettings settings("Trolltech");
    settings.beginGroup("Qt");

    QString currentstyle = settings.value("style").toString();
    if (currentstyle.isNull())
        currentstyle = QApplication::style()->name();
    {
        int s = 0;
        QStringList::Iterator git = gstyles.begin();
        while (git != gstyles.end()) {
            if ((*git).lower() == currentstyle.lower())
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
    connect(buttonMainColor, SIGNAL(colorChanged(QColor)),
                this, SLOT(buildPalette()));
    connect(buttonMainColor2, SIGNAL(colorChanged(QColor)),
                this, SLOT(buildPalette()));

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
    Q3ValueList<int> sizes = db.standardSizes();
    Q3ValueList<int>::Iterator it = sizes.begin();
    while (it != sizes.end())
        psizecombo->insertItem(QString::number(*it++));

    dcispin->setValue(QApplication::doubleClickInterval());
    cfispin->setValue(QApplication::cursorFlashTime());
    wslspin->setValue(QApplication::wheelScrollLines());
    // #############
//    resolvelinks->setChecked(qt_resolve_symlinks);

    effectcheckbox->setChecked(QApplication::isEffectEnabled(Qt::UI_General));
    effectbase->setEnabled(effectcheckbox->isChecked());

    if (QApplication::isEffectEnabled(Qt::UI_FadeMenu))
        menueffect->setCurrentItem(2);
    else if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu))
        menueffect->setCurrentItem(1);

    if (QApplication::isEffectEnabled(Qt::UI_AnimateCombo))
        comboeffect->setCurrentItem(1);

    if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip))
        tooltipeffect->setCurrentItem(2);
    else if (QApplication::isEffectEnabled(Qt::UI_AnimateTooltip))
        tooltipeffect->setCurrentItem(1);

    if ( QApplication::isEffectEnabled( Qt::UI_AnimateToolBox ) )
        toolboxeffect->setCurrentItem( 1 );

    QSize globalStrut = QApplication::globalStrut();
    strutwidth->setValue(globalStrut.width());
    strutheight->setValue(globalStrut.height());

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
    for (int psize = QApplication::font().pointSize(); i < psizecombo->count(); ++i) {
        const int sz = psizecombo->text(i).toInt();
        if (sz == psize) {
            psizecombo->setCurrentItem(i);
            break;
        } else if(sz > psize) {
            psizecombo->insertItem(i, QString::number(psize));
            psizecombo->setCurrentItem(i);
            break;
        }
    }

    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    sublistbox->clear();
    sublistbox->insertStringList(subs);

    rtlExtensions->setChecked(settings.value("useRtlExtensions", false).toBool());

#ifdef Q_WS_X11
    inputStyle->setCurrentText(settings.value("XIMInputStyle", trUtf8("On The Spot")).toString());
#else
    inputStyle->hide();
    inputStyleLabel->hide();
#endif

    fontembeddingcheckbox->setChecked(settings.value("embedFonts", true).toBool());
    fontpaths = settings.value("fontPath").toStringList();
    fontpathlistbox->insertStringList(fontpaths);

    settings.endGroup(); // Qt

    helpview->setText(tr(appearance_text));

    setModified(false);
}


MainWindow::~MainWindow()
{
}

#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

void MainWindow::fileSave()
{
    if (! modified) {
        statusBar()->showMessage("No changes to be saved.", 2000);
        return;
    }

    statusBar()->showMessage("Saving changes...");

    {
        QSettings settings("Trolltech");
        settings.beginGroup("Qt");
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

        settings.setValue("font", font.toString());
        settings.setValue("Palette/active", actcg);
        settings.setValue("Palette/inactive", inactcg);
        settings.setValue("Palette/disabled", discg);

        settings.setValue("fontPath", fontpaths);
        settings.setValue("embedFonts", fontembeddingcheckbox->isChecked());
        settings.setValue("style", gstylecombo->currentText());
        settings.setValue("doubleClickInterval", dcispin->value());
        settings.setValue("cursorFlashTime", cfispin->value() == 9 ? 0 : cfispin->value() );
        settings.setValue("wheelScrollLines", wslspin->value());
        settings.setValue("resolveSymlinks", resolvelinks->isChecked());

        QSize strut(strutwidth->value(), strutheight->value());
        settings.setValue("globalStrut/width", strut.width());
        settings.setValue("globalStrut/height", strut.height());

        settings.setValue("useRtlExtensions", rtlExtensions->isChecked());

#ifdef Q_WS_X11
        QString style = inputStyle->currentText();
        QString str = "On The Spot";
        if ( style == trUtf8( "Over The Spot" ) )
            str = "Over The Spot";
        else if ( style == trUtf8( "Off The Spot" ) )
            str = "Off The Spot";
        else if ( style == trUtf8( "Root" ) )
            str = "Root";
        settings.setValue( "XIMInputStyle", str );
#endif

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

            switch ( toolboxeffect->currentItem() ) {
            case 1: effects << "animatetoolbox"; break;
            }
        } else
            effects << "none";
        settings.setValue("GUIEffects", effects);

        QStringList familysubs = QFont::substitutions();
        QStringList::Iterator fit = familysubs.begin();
        settings.beginGroup(QLatin1String("Font Substitutions"));
        while (fit != familysubs.end()) {
            QStringList subs = QFont::substitutes(*fit);
            settings.setValue(*fit, subs);
            fit++;
        }
        settings.endGroup(); // Font Substitutions
        settings.endGroup(); // Qt
    }

#if defined(Q_WS_X11)
    qt_x11_apply_settings_in_all_apps();
#endif // Q_WS_X11

    setModified(false);
    statusBar()->showMessage("Saved changes.");
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

    setModified(true);
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
    shadow = Qt::black;

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
    cg.setColor( QColorGroup::ButtonText, Qt::darkGray );
    cg.setColor( QColorGroup::Foreground, Qt::darkGray );
    cg.setColor( QColorGroup::Text, Qt::darkGray );
    cg.setColor( QColorGroup::HighlightedText, Qt::darkGray );
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
    shadow = Qt::black;

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
    setModified(true);
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

    setModified(true);
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
    setModified(true);
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
    setModified(true);
}


void MainWindow::addSubstitute()
{
    if (sublistbox->currentItem() < 0 ||
        uint(sublistbox->currentItem()) > sublistbox->count()) {
        QFont::insertSubstitution(familysubcombo->currentText(), choosesubcombo->currentText());
        QStringList subs = QFont::substitutes(familysubcombo->currentText());
        sublistbox->clear();
        sublistbox->insertStringList(subs);
        setModified(true);
        return;
    }

    int item = sublistbox->currentItem();
    QFont::insertSubstitution(familysubcombo->currentText(), choosesubcombo->currentText());
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item);
    setModified(true);
}


void MainWindow::downSubstitute()
{
    if (sublistbox->currentItem() < 0 ||
        uint(sublistbox->currentItem()) >= sublistbox->count())
        return;

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    QString fam = subs.at(item);
    subs.removeAt(item);
    subs.insert(item+1, fam);
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item + 1);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(true);
}


void MainWindow::upSubstitute()
{
    if (sublistbox->currentItem() < 1)
        return;

    int item = sublistbox->currentItem();
    QStringList subs = QFont::substitutes(familysubcombo->currentText());
    QString fam = subs.at(item);
    subs.removeAt(item);
    subs.insert(item-1, fam);
    sublistbox->clear();
    sublistbox->insertStringList(subs);
    sublistbox->setCurrentItem(item - 1);
    QFont::removeSubstitution(familysubcombo->currentText());
    QFont::insertSubstitutions(familysubcombo->currentText(), subs);
    setModified(true);
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
    setModified(true);
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
        setModified(true);

        return;
    }

    int item = fontpathlistbox->currentItem();
    fontpaths.insert(fontpathlistbox->currentItem()+1,
                     fontpathlineedit->text());
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item);
    setModified(true);
}


void MainWindow::downFontpath()
{
    if (fontpathlistbox->currentItem() < 0 ||
        uint(fontpathlistbox->currentItem()) >= fontpathlistbox->count() - 1)
        return;

    int item = fontpathlistbox->currentItem();
    QString fam = fontpaths.at(item);
    fontpaths.removeAt(item);
    fontpaths.insert(item+1, fam);
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item + 1);
    setModified(true);
}


void MainWindow::upFontpath()
{
    if (fontpathlistbox->currentItem() < 1)
        return;

    int item = fontpathlistbox->currentItem();
    QString fam = fontpaths.at(item);
    fontpaths.removeAt(item);
    fontpaths.insert(item-1, fam);
    fontpathlistbox->clear();
    fontpathlistbox->insertStringList(fontpaths);
    fontpathlistbox->setCurrentItem(item - 1);
    setModified(true);
}


void MainWindow::browseFontpath()
{
    QString dirname = QFileDialog::getExistingDirectory(QString(), this, 0,
                                                        tr("Select a Directory"));
    if (dirname.isNull())
        return;

   fontpathlineedit->setText(dirname);
}


void MainWindow::somethingModified()
{
    setModified(true);
}


void MainWindow::helpAbout()
{
    QMessageBox box(this);
    box.setText(tr("<h3>%1</h3>"
                   "<br/>Version %2"
#if QT_EDITION == QT_EDITION_OPENSOURCE
                   " Open Source Edition</center><p>"
                   "This version of Qt Configuration is part of the Qt Open Source Edition, for use "
                   "in the development of Open Source applications. "
                   "Qt is a comprehensive C++ framework for cross-platform application "
                   "development.<br/><br/>"
                   "You need a commercial Qt license for development of proprietary (closed "
                   "source) applications. Please see <tt>http://www.trolltech.com/company/model"
                   ".html</tt> for an overview of Qt licensing."
#else
                   "</center><p>This program is licensed to you under the terms of the "
                   "Qt Commercial License Agreement. For details, see the file LICENSE "
                   "that came with this software distribution."
#endif
                   "<br/><br/>Copyright (C) 2000-$THISYEAR$ Trolltech AS. All rights reserved."
                   "<br/><br/>The program is provided AS IS with NO WARRANTY OF ANY KIND,"
                   " INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A"
                   " PARTICULAR PURPOSE.<br/> ")
                   .arg(tr("Qt Configuration")).arg(QT_VERSION_STR));
    box.setWindowTitle(tr("Qt Configuration"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}


void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Configuration"));
}


void MainWindow::pageChanged(QWidget *page)
{
    if (page == tab)
        helpview->setText(tr(appearance_text));
    else if (page == tab1)
        helpview->setText(tr(font_text));
    else if (page == tab2)
        helpview->setText(tr(interface_text));
    else if (page == tab3)
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
