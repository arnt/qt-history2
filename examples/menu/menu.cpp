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

#include "menu.h"
#include <qcursor.h>
#include <qmenu.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpainter.h>

/* XPM */
static const char * p1_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"                ",
"                ",
"         ....   ",
"        .XXXX.  ",
" .............. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .............. ",
"                "};

/* XPM */
static const char * p2_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"   ......       ",
"   .XXX.X.      ",
"   .XXX.XX.     ",
"   .XXX.XXX.    ",
"   .XXX.....    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .........    ",
"                ",
"                "};

/* XPM */
static const char * p3_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"                ",
"   .........    ",
"  ...........   ",
"  ........ ..   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"   .........    ",
"                ",
"                "};

MenuExample::MenuExample(QWidget *parent)
    : QWidget(parent)
{
    QAction *action;
    QPixmap p1(p1_xpm);
    QPixmap p2(p2_xpm);
    QPixmap p3(p3_xpm);
    QMenu *print = new QMenu(this);
    Q_CHECK_PTR(print);
    print->setTearOffEnabled(true);
    print->addAction("&Print to printer", this, SLOT(printer()));
    print->addAction("Print to &file", this, SLOT(file()));
    print->addAction("Print to fa&x", this, SLOT(fax()));
    print->addSeparator();
    print->addAction("Printer &Setup", this, SLOT(printerSetup()));
#ifdef Q_WS_MAC
    extern void qt_mac_set_dock_menu(QMenu *);
    qt_mac_set_dock_menu(print);
#endif

    QMenu *file = new QMenu(this);
    Q_CHECK_PTR(file);
    action = file->addAction(p1, "&Open",  this, SLOT(open()));
    action->setShortcut(Qt::CTRL+Qt::Key_O);
    action = file->addAction(p2, "&New", this, SLOT(news()));
    action->setShortcut(Qt::CTRL+Qt::Key_N);
    action = file->addAction(p3, "&Save", this, SLOT(save()));
    action->setShortcut(Qt::CTRL+Qt::Key_S);
    action = file->addAction("&Close", this, SLOT(closeDoc()));
    action->setShortcut(Qt::CTRL+Qt::Key_W);
    file->addSeparator();
    action = file->addMenu("&Print", print);
    action->setShortcut(Qt::CTRL+Qt::Key_P);
    file->addSeparator();
    action = file->addAction("E&xit",  qApp, SLOT(quit()));
    action->setShortcut(Qt::CTRL+Qt::Key_Q);

    QMenu *edit = new QMenu(this);
    Q_CHECK_PTR(edit);
    QAction *undoAction = edit->addAction("&Undo", this, SLOT(undo()));
    undoAction->setEnabled(false);
    QAction *redoAction = edit->addAction("&Redo", this, SLOT(redo()));
    redoAction->setEnabled(false);

    QMenu* options = new QMenu(this);
    Q_CHECK_PTR(options);
    options->setTearOffEnabled(true);
    options->setWindowTitle("Options");
    options->addAction("&Normal Font", this, SLOT(normal()));
    options->addSeparator();

    boldAct = options->addAction("Bold");
    QFont bold;
    bold.setBold(true);
    boldAct->setFont(bold);
    boldAct->setShortcut(Qt::CTRL+Qt::Key_B);
    QObject::connect(boldAct, SIGNAL(triggered()), this, SLOT(bold()));
    underlineAct = options->addAction("Underline");
    QFont underline;
    underline.setUnderline(true);
    underlineAct->setFont(underline);
    underlineAct->setShortcut(Qt::CTRL+Qt::Key_U);
    QObject::connect(underlineAct, SIGNAL(triggered()), this, SLOT(underline()));

    isBold = FALSE;
    isUnderline = FALSE;
    options->setCheckable(TRUE);

    QMenu *help = new QMenu(this);
    Q_CHECK_PTR(help);
    help->addAction("&About", this, SLOT(about()));
    action->setShortcut(Qt::CTRL+Qt::Key_H);
    help->addAction("About &Qt", this, SLOT(aboutQt()));

    // If we used a QMainWindow we could use its built-in menuBar().
    menu = new QMenuBar(this);
    Q_CHECK_PTR(menu);
    menu->addMenu("&File", file);
    menu->addMenu("&Edit", edit);
    menu->addMenu("&Options", options);
    menu->addSeparator();
    menu->addMenu("&Help", help);

    QLabel *msg = new QLabel(this);
    Q_CHECK_PTR(msg);
    msg->setText("A context menu is available.\n"
		  "Invoke it by right-clicking or by"
		  " pressing the 'context' button.");
    msg->setGeometry(0, height() - 60, width(), 60);
    msg->setAlignment(Qt::AlignCenter);

    label = new QLabel(this);
    Q_CHECK_PTR(label);
    label->setGeometry(20, rect().center().y()-20, width()-40, 40);
    label->setFrameStyle(QFrame::Box | QFrame::Raised);
    label->setLineWidth(1);
    label->setAlignment(Qt::AlignCenter);

    connect(this,  SIGNAL(explain(const QString&)),
	     label, SLOT(setText(const QString&)));

    setMinimumSize(100, 80);
    setFocusPolicy(Qt::ClickFocus);
}

class MyFancyMenu : public QMenu
{
    const QString text;
    int margin;
    QFont myFont() const {
        QFont ret = font();
        ret.setPixelSize(30);
        ret.setBold(true);
        return ret;
    }
public:
    MyFancyMenu(const QString &t, QWidget *widget) : QMenu(widget), text(t) {
        margin = QFontMetrics(myFont()).height() + 10;
        setContentsMargins(margin, 0, 0, 0);
    }

    QSize sizeHint() const {
        QSize ret = QMenu::sizeHint();
        int length = QFontMetrics(myFont()).width(text);
        if(ret.height() < length)
            ret.setHeight(length);
        return ret;
    }
protected:

    void paintEvent(QPaintEvent *e) {
        QMenu::paintEvent(e);

        QPainter p(this);
        //draw gradiant
        p.setClipRect(QRect(0, 0, margin, height()));
        p.setBrush(QBrush(QPoint(margin/2, 0), Qt::black, QPoint(margin/2, height()), Qt::red));
        p.drawRect(0, 0, margin, height());
        //draw text
        p.setPen(Qt::white);
        p.setFont(myFont());
        p.translate((margin/2)+(p.fontMetrics().ascent()/2), height());
        p.rotate(-90);
        p.drawText(0, 0, text);
    }
};

void MenuExample::contextMenuEvent(QContextMenuEvent *)
{
    MyFancyMenu contextMenu(tr("Context!"), this);
    contextMenu.addAction("&New",  this, SLOT(news()));
    contextMenu.addAction("&Open...", this, SLOT(open()));
    contextMenu.addAction("&Save", this, SLOT(save()));
    QMenu *submenu = new QMenu(this);
    submenu->addAction("&Print to printer", this, SLOT(printer()));
    submenu->addAction("Print to &file", this, SLOT(file()));
    submenu->addAction("Print to fa&x", this, SLOT(fax()));
    contextMenu.addMenu("&Print", submenu);
    contextMenu.exec(QCursor::pos());
}


void MenuExample::open()
{
    emit explain("File/Open selected");
}


void MenuExample::news()
{
    emit explain("File/New selected");
}

void MenuExample::save()
{
    emit explain("File/Save selected");
}


void MenuExample::closeDoc()
{
    emit explain("File/Close selected");
}


void MenuExample::undo()
{
    emit explain("Edit/Undo selected");
}


void MenuExample::redo()
{
    emit explain("Edit/Redo selected");
}


void MenuExample::normal()
{
    isBold = FALSE;
    isUnderline = FALSE;
    QFont font;
    label->setFont(font);
    boldAct->setChecked(isBold);
    underlineAct->setChecked(isUnderline);
    emit explain("Options/Normal selected");
}


void MenuExample::bold()
{
    isBold = !isBold;
    QFont font;
    font.setBold(isBold);
    font.setUnderline(isUnderline);
    label->setFont(font);
    boldAct->setChecked(isBold);
    emit explain("Options/Bold selected");
}


void MenuExample::underline()
{
    isUnderline = !isUnderline;
    QFont font;
    font.setBold(isBold);
    font.setUnderline(isUnderline);
    label->setFont(font);
    underlineAct->setChecked(isUnderline);
    emit explain("Options/Underline selected");
}


void MenuExample::about()
{
    QMessageBox::about(this, "Qt Menu Example",
			"This example demonstrates simple use of Qt menus.\n"
			"You can cut and paste lines from it to your own\n"
			"programs.");
}


void MenuExample::aboutQt()
{
    QMessageBox::aboutQt(this, "Qt Menu Example");
}


void MenuExample::printer()
{
    emit explain("File/Printer/Print selected");
}

void MenuExample::file()
{
    emit explain("File/Printer/Print To File selected");
}

void MenuExample::fax()
{
    emit explain("File/Printer/Print To Fax selected");
}

void MenuExample::printerSetup()
{
    emit explain("File/Printer/Printer Setup selected");
}


void MenuExample::resizeEvent(QResizeEvent *)
{
    label->setGeometry(20, rect().center().y()-20, width()-40, 40);
}


int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    MenuExample m;
    m.setWindowTitle("Qt Examples - Menus");
    a.setMainWidget(&m);
    m.show();
    return a.exec();
}
