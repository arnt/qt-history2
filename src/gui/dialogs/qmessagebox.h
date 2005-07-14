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

#ifndef QMESSAGEBOX_H
#define QMESSAGEBOX_H

#include "QtGui/qdialog.h"

QT_MODULE(Gui)

#ifndef QT_NO_MESSAGEBOX

class QLabel;
class QMessageBoxPrivate;

class Q_GUI_EXPORT QMessageBox : public QDialog
{
    Q_OBJECT
    Q_ENUMS(Icon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Icon icon READ icon WRITE setIcon)
    Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)

public:
    enum Icon {
        NoIcon = 0,
        Information = 1,
        Warning = 2,
        Critical = 3,
        Question = 4
    };

    explicit QMessageBox(QWidget *parent = 0);
    QMessageBox(const QString &caption, const QString &text, Icon icon,
                int button0, int button1, int button2,
                QWidget *parent = 0, Qt::WFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    ~QMessageBox();

    enum Button { NoButton = 0, Ok = 1, Cancel = 2, Yes = 3, No = 4, Abort = 5,
           Retry = 6, Ignore = 7, YesAll = 8, NoAll = 9, ButtonMask = 0xff,
           Default = 0x100, Escape = 0x200, FlagMask = 0x300 };

    QString text() const;
    void setText(const QString &);

    Icon icon() const;
    void setIcon(Icon);

    QPixmap iconPixmap() const;
    void setIconPixmap(const QPixmap &);

    QString buttonText(int button) const;
    void setButtonText(int button, const QString &);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat);

    static int information(QWidget *parent, const QString &caption,
                            const QString& text,
                            int button0, int button1=0, int button2=0);
    static int information(QWidget *parent, const QString &caption,
                            const QString& text,
                            const QString& button0Text = QString(),
                            const QString& button1Text = QString(),
                            const QString& button2Text = QString(),
                            int defaultButtonNumber = 0,
                            int escapeButtonNumber = -1);

    static int question(QWidget *parent, const QString &caption,
                         const QString& text,
                         int button0, int button1=0, int button2=0);
    static int question(QWidget *parent, const QString &caption,
                         const QString& text,
                         const QString& button0Text = QString(),
                         const QString& button1Text = QString(),
                         const QString& button2Text = QString(),
                         int defaultButtonNumber = 0,
                         int escapeButtonNumber = -1);

    static int warning(QWidget *parent, const QString &caption,
                        const QString& text,
                        int button0, int button1, int button2=0);
    static int warning(QWidget *parent, const QString &caption,
                        const QString& text,
                        const QString& button0Text = QString(),
                        const QString& button1Text = QString(),
                        const QString& button2Text = QString(),
                        int defaultButtonNumber = 0,
                        int escapeButtonNumber = -1);

    static int critical(QWidget *parent, const QString &caption,
                         const QString& text,
                         int button0, int button1, int button2=0);
    static int critical(QWidget *parent, const QString &caption,
                         const QString& text,
                         const QString& button0Text = QString(),
                         const QString& button1Text = QString(),
                         const QString& button2Text = QString(),
                         int defaultButtonNumber = 0,
                         int escapeButtonNumber = -1);

    static void about(QWidget *parent, const QString &caption,
                       const QString& text);

    static void aboutQt(QWidget *parent,
                         const QString& caption=QString());

    QSize sizeHint() const;

#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QMessageBox(const QString &caption, const QString &text, Icon icon,
                                      int button0, int button1, int button2,
                                      QWidget *parent, const char *name, bool modal,
                                      Qt::WFlags f =  Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    QT3_SUPPORT_CONSTRUCTOR QMessageBox(QWidget *parent, const char *name);

    static QT3_SUPPORT QPixmap standardIcon(Icon icon, Qt::GUIStyle);
    static QT3_SUPPORT int message(const QString &caption,
                                 const QString& text,
                                 const QString& buttonText=QString(),
                                 QWidget *parent=0, const char * =0) {
        return QMessageBox::information(parent, caption, text,
                                        buttonText.isEmpty()
                                     ? tr("OK") : buttonText) == 0;
    }
    static QT3_SUPPORT bool query(const QString &caption,
                                const QString& text,
                                const QString& yesButtonText=QString(),
                                const QString& noButtonText=QString(),
                                QWidget *parent=0, const char * = 0) {
        return QMessageBox::information(parent, caption, text,
                                     yesButtonText.isEmpty()
                                     ? tr("OK") : yesButtonText,
                                     noButtonText) == 0;
    }
#endif

    static QPixmap standardIcon(Icon icon);

protected:
    void        resizeEvent(QResizeEvent *);
    void        showEvent(QShowEvent *);
    void        closeEvent(QCloseEvent *);
    void        keyPressEvent(QKeyEvent *);
    void        changeEvent(QEvent *);

private:
    Q_PRIVATE_SLOT(d_func(), void buttonClicked())

    Q_DISABLE_COPY(QMessageBox)
    Q_DECLARE_PRIVATE(QMessageBox)
};

#define QT_REQUIRE_VERSION(argc, argv, str) { QString s=QString::fromLatin1(str);\
QString sq=QString::fromLatin1(qVersion()); if ((sq.section('.',0,0).toInt()<<16)+\
(sq.section('.',1,1).toInt()<<8)+sq.section('.',2,2).toInt()<(s.section('.',0,0).toInt()<<16)+\
(s.section('.',1,1).toInt()<<8)+s.section('.',2,2).toInt()){if (!qApp){ int c=0; new \
QApplication(argc,argv);} QString s = QApplication::tr("Executable '%1' requires Qt "\
 "%2, found Qt %3.").arg(QString::fromLatin1(qAppName())).arg(QString::fromLatin1(\
str)).arg(QString::fromLatin1(qVersion())); QMessageBox::critical(0, QApplication::tr(\
"Incompatible Qt Library Error"), s, QMessageBox::Abort,0); qFatal(s.ascii()); }}


#endif // QT_NO_MESSAGEBOX

#endif // QMESSAGEBOX_H
