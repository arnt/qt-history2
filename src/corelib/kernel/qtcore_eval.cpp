
#include <qdatetime.h>
#include <qlibraryinfo.h>
#include "keyinfo.h"

static const char * const dont_mess_with_me =
    "\nQt %1 Evaluation License\n"
    "Copyright (C) 1992-$THISYEAR$ by Trolltech AS.\n"
    "All rights reserved.\n\n"
    "This trial version may only be used for evaluation purposes\n"
    "and will shut down after 30 minuttes.\n"
    "Registered to:\n"
    "   Licensee: %2\n\n"
    "The evaluation expires in %4 days\n\n"
    "Contact sales@trolltech.com for pricing and purchasing information.\n";


static const char * const dont_mess_with_me_either =
    "This software is using the trial version of the Qt GUI toolkit.\n"
    "The trial period has expired. If you need more time to\n"
    "evaluate Qt, or if you have any questions about Qt, contact us\n"
    "at: sales@trolltech.com.\n\n";

static const char * const dont_mess_with_you_huh =
    "\nThe evaluation of Qt will SHUT DOWN in 1 minutte.\n"
    "Contact sales@trolltech.com for pricing and purchasing information.\n";

static const char * const dont_mess_with_you_huh2 =
    "\nThe evaluation of Qt has now reached its automatic\n"
    "timeout and will shut down.\n"
    "Contact sales@trolltech.com for pricing and purchasing information.\n";

static const char qt_eval_key_data[512 + 12]     = "qt_qevalkey=";

// expiration date really
static int qt_eval_figure_out()
{
    QDate today = QDate::currentDate();
    QByteArray license_key(qt_eval_key_data + 12);

    uint products, platform, schema, features, id;
    QDate expiry;
    if (decodeLicenseKey(license_key, &products, &platform, &schema, &features, &id, &expiry)) {

        if ((products & QtDesktop) == 0)
            return -100000;

#if defined(Q_WS_WIN)
        if ((platform & Windows) == 0)
#elif defined(Q_WS_MAC)
        if ((platform & Mac) == 0)
#elif defined(Q_WS_X11)
        if ((platform & X11) == 0)
#else
        if ((platform & Embedded) == 0)
#endif
            return -100001;

        if ((schema & 0x07) == 0)
            return -100002;

        return today.daysTo(expiry);
    } else {
        return -100003;
    }
}


static bool qt_eval_is_expired()
{
    return qt_eval_figure_out() < 0;
}

QString qt_eval_string() {
    QString str = QString(QLatin1String(dont_mess_with_me))
                  .arg(QT_VERSION_STR)
                  .arg(QLibraryInfo::licensee())
                  .arg(qt_eval_figure_out());
    return str;
}

#define WARN_TIMEOUT 60 * 1000 * 29
#define KILL_DELAY 60 * 1000 * 1

class QCoreFuriCuri : public QObject
{
public:

    int warn;
    int kill;

    QCoreFuriCuri()
    {
        warn = startTimer(WARN_TIMEOUT);
        kill = 0;
    }

    void timerEvent(QTimerEvent *e) {
        if (e->timerId() == warn) {
            killTimer(warn);
            fprintf(stderr, "%s\n", dont_mess_with_you_huh);
            kill = startTimer(KILL_DELAY);
        } else if (e->timerId() == kill) {
            fprintf(stderr, "%s\n", dont_mess_with_you_huh2);
            QCoreApplication::instance()->quit();
        }
    }
};

#ifdef QT_BUILD_CORE_LIB

void qt_core_eval_init(uint type)
{
    if (qt_eval_is_expired()) {
        fprintf(stderr, "%s\n", dont_mess_with_me_either);

        int code = qt_eval_figure_out();
        if (code <= -100000)
            fprintf(stderr, "Error code: %d\n", code);

        if (type == 0) {
            // if we're a console app only.
            exit(0);
        }
    } else {
        fprintf(stderr, "%s\n", qPrintable(qt_eval_string()));
        if (type == 0) {
            Q_UNUSED(new QCoreFuriCuri());
        }
    }
}
#endif

#ifdef QT_BUILD_GUI_LIB

#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

static const char * const qtlogo_eval_xpm[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};

class EvalMessageBox : public QDialog
{
public:
    EvalMessageBox(bool expired)
    {
        setWindowTitle(" ");

        QString str = expired
                      ? QString(QLatin1String(dont_mess_with_me_either))
                      : qt_eval_string();
        str = str.trimmed();

        QFrame *border = new QFrame(this);

        QLabel *pixmap_label = new QLabel(border);
        pixmap_label->setPixmap(qtlogo_eval_xpm);
        pixmap_label->setAlignment(Qt::AlignTop);

        QLabel *text_label = new QLabel(str, border);

        QHBoxLayout *pm_and_text_layout = new QHBoxLayout();
        pm_and_text_layout->addWidget(pixmap_label);
        pm_and_text_layout->addWidget(text_label);

        QVBoxLayout *master_layout = new QVBoxLayout(border);
        master_layout->addLayout(pm_and_text_layout);

        QVBoxLayout *border_layout = new QVBoxLayout(this);
        border_layout->setMargin(0);
        border_layout->addWidget(border);

        if (expired) {
            int code = qt_eval_figure_out();
            if (code <= -100000) {
                QString text = QString(QLatin1String("Error code: %1")).arg(code);
                QLabel *extra_text = new QLabel(text, border);
                extra_text->setAlignment(Qt::AlignHCenter);
                master_layout->addWidget(extra_text);
            }

            QPushButton *cmd = new QPushButton("OK", border);
            cmd->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            cmd->setDefault(true);

            QHBoxLayout *button_layout = new QHBoxLayout();
            master_layout->addLayout(button_layout);
            button_layout->addWidget(cmd);

            connect(cmd, SIGNAL(clicked()), this, SLOT(close()));

        } else {
            border->setFrameShape(QFrame::WinPanel);
            border->setFrameShadow(QFrame::Raised);
            setParent(parentWidget(), Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
            QTimer::singleShot(7000, this, SLOT(close()));
            setAttribute(Qt::WA_DeleteOnClose);
        }

        setFixedSize(sizeHint());
    }
};

class QGuiFuriCuri : public QCoreFuriCuri
{
public:
    void timerEvent(QTimerEvent *e) {
        if (e->timerId() == warn) {
            killTimer(warn);
            QMessageBox::information(0, "Automatic Timeout", dont_mess_with_you_huh);
            kill = startTimer(KILL_DELAY);
        } else if (e->timerId() == kill) {
            killTimer(kill);
            QMessageBox::information(0, "Automatic Timeout", dont_mess_with_you_huh2);
            qApp->quit();
        }
    }
};


void qt_gui_eval_init(uint type)
{
    if (type != 0) {
        if (qt_eval_is_expired()) {
            EvalMessageBox box(true);
            box.exec();
            QApplication::exit(0);
        } else {
            QString eval_string = qt_eval_string();
            EvalMessageBox *box = new EvalMessageBox(false);
            box->show();
        }

        Q_UNUSED(new QGuiFuriCuri());
    }
}

QString qt_eval_adapt_window_title(const QString &title)
{
    return QLatin1String("[Qt Evaluation] ") + title;
}

void qt_eval_init_widget(QWidget *w)
{
    if (w->isTopLevel())
        w->setWindowTitle(" ");
}
#endif
