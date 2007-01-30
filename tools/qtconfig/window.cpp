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

#include "window.h"
#include "appearancepage.h"
#include "preview.h"
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QFontDatabase>
#include <QFont>
#include <QStringList>
#include <QColorGroup>
#include <QPalette>
#include <QSize>
#include <QString>
#include <QStatusBar>
#include <QStyleFactory>
#include <QStyle>
#include <QList>
#include <QDebug>
#include <QRegExp>
#include <QKeySequence>
#include <QLatin1String>
#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QByteArray>
#include <QRect>

static const char *helpTexts[] = {
    QT_TRANSLATE_NOOP(
        "Window",
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
        "like."),

    QT_TRANSLATE_NOOP(
        "Window",
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
        "use with Chinese text)."),

    QT_TRANSLATE_NOOP(
        "Window",
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
        "Japanese."),
// ### What does the 'Enhanced support for languages written R2L do?
#ifndef Q_WS_MAC
    QT_TRANSLATE_NOOP(
        "Window",
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
        "server font path is used."),
#endif
    0 };

Window::Window(QWidget *parent)
    : QMainWindow(parent), modified(false)
{
    setupUi(this);
    connect(appearancePage, SIGNAL(changed()), this, SLOT(setModified()));
    connect(fontsPage, SIGNAL(changed()), this, SLOT(setModified()));
    connect(interfacePage, SIGNAL(changed()), this, SLOT(setModified()));
#ifndef Q_WS_MAC
    connect(printerPage, SIGNAL(changed()), this, SLOT(setModified()));
#endif
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabIndexChanged(int)));
    connect(actionRevert, SIGNAL(triggered(bool)), this, SLOT(revert()));
    connect(actionExit, SIGNAL(triggered(bool)), this, SLOT(exit()));
    connect(actionAbout, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(actionAboutQt, SIGNAL(triggered(bool)), this, SLOT(aboutQt()));
    connect(actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));

#ifdef Q_WS_MAC
    tabWidget->removeTab(3);
    delete printerPage;
#endif
    QRegExp rx("&([^&])");
    for (int i=0; helpTexts[i]; ++i) {
        QKeySequence seq(
#ifdef Q_WS_MAC
            Qt::CTRL
#else
            Qt::ALT
#endif
            + Qt::Key_1 + i);
        QString tabText = tabWidget->tabText(i);
        tabText.replace(rx, QLatin1String("<u>\\1</u>"));
        tabWidget->setTabToolTip(i, QString("<html>%0<br/><i>%1</i></html>").
                                 arg(tabText).
                                 arg(seq.toString(QKeySequence::NativeText)));
    }
    const QList<QWidget *> widgets = qFindChildren<QWidget*>(this);
    for (int i=0; i<widgets.size(); ++i) {
        widgets.at(i)->installEventFilter(this);
    }

}

void Window::about()
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
                   "You need a commercial Qt license for development of proprietary(closed "
                   "source) applications. Please see <tt>http://www.trolltech.com/company/model"
                   ".html</tt> for an overview of Qt licensing."
#else
                   "</center><p>This program is licensed to you under the terms of the "
                   "Qt Commercial License Agreement. For details, see the file LICENSE "
                   "that came with this software distribution."
#endif
                   "<br/><br/>Copyright(C) 2000-$THISYEAR$ Trolltech AS. All rights reserved."
                   "<br/><br/>The program is provided AS IS with NO WARRANTY OF ANY KIND,"
                   " INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A"
                   " PARTICULAR PURPOSE.<br/> ")
                .arg(tr("Qt Configuration")).arg(QT_VERSION_STR));
    box.setWindowTitle(tr("Qt Configuration"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}


void Window::aboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Configuration"));
}

void Window::setModified(bool m)
{
    if (m != modified) {
        modified = m;
        actionSave->setEnabled(m);
        actionRevert->setEnabled(m);
    }
}

void Window::exit()
{
    qApp->closeAllWindows();
}

void Window::closeEvent(QCloseEvent *e)
{
    if (modified) {
        switch (QMessageBox::warning(this, tr("Save Changes"),
                                     tr("Save changes to settings?"),
                                     QMessageBox::Yes|QMessageBox::Default, QMessageBox::No,
                                     QMessageBox::Cancel|QMessageBox::Escape)) {
        case QMessageBox::Yes: // save
            qApp->processEvents();
            save();
            // fall through intended
        case QMessageBox::No: // don't save
            break;

        case QMessageBox::Cancel: // cancel
            e->ignore();
            return;

        default: break;
        }
    }
    QSettings settings("Trolltech", "Qt Config");
    settings.setValue("Window/currentTab", tabWidget->currentIndex());
    settings.setValue("Window/geometry", saveGeometry());
    settings.setValue("Window/isMaximized", isMaximized());
    e->accept();
    QMainWindow::closeEvent(e);
}


#ifdef Q_WS_X11
extern void qt_x11_apply_settings_in_all_apps();
#endif

void Window::save()
{
    if (!modified) {
        statusBar()->showMessage(tr("No changes to be saved."), 2000);
        return;
    }

    statusBar()->showMessage(tr("Saving changes..."));
    appearancePage->save();
    fontsPage->save();
    interfacePage->save();
#if defined(Q_WS_X11)
    printerPage->save();
    qt_x11_apply_settings_in_all_apps();
#endif // Q_WS_X11

    setModified(false);
    statusBar()->showMessage(tr("Saved changes."));
}

void Window::revert()
{
    if (!modified) {
        statusBar()->showMessage(tr("No changes to revert."), 2000);
        return;
    }
    if (QMessageBox::warning(this, tr("Revert changes"),
                             tr("You have unsaved changes. Are you sure you want to revert?"),
                             QMessageBox::Yes, QMessageBox::No|QMessageBox::Default|QMessageBox::Escape)
        == QMessageBox::No) {
        return;
    }

    statusBar()->showMessage(tr("Reverting changes..."));
    appearancePage->load();
    fontsPage->load();
    interfacePage->load();
#if defined(Q_WS_X11)
    printerPage->load();
    qt_x11_apply_settings_in_all_apps();
#endif // Q_WS_X11

    setModified(false);
    statusBar()->showMessage(tr("Reverted changes."));
}


bool Window::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->modifiers() &
#ifdef Q_WS_MAC
            Qt::ControlModifier
#else
            Qt::AltModifier
#endif
            ) {
            const int tab = (ke->key() - Qt::Key_1);
            if (tab >= 0 && tab < tabWidget->count()) {
                tabWidget->setCurrentIndex(tab);
                return true;
            }
        }
    }
    return false;
}

void Window::showEvent(QShowEvent *e)
{
    QSettings settings(QLatin1String("Trolltech"), QLatin1String("Qt Config"));
    const int currentIndex = tabWidget->currentIndex();
    tabWidget->setCurrentIndex(settings.value(QLatin1String("Window/currentTab"), 0).toInt());
    if (currentIndex == tabWidget->currentIndex())
        onCurrentTabIndexChanged(currentIndex);

    const QByteArray geometry = settings.value("Window/geometry", QRect()).
                                toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    QMainWindow::showEvent(e);
}
void Window::onCurrentTabIndexChanged(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(helpTexts[index]);
    textEdit->setHtml(tr(helpTexts[index]));
}
