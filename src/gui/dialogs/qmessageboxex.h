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

#ifndef QMESSAGEBOXEX_H
#define QMESSAGEBOXEX_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_MESSAGEBOXEX

class QLabel;
class QMessageBoxExPrivate;

class Q_GUI_EXPORT QMessageBoxEx : public QDialog
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

    enum ButtonRole {
        InvalidRole = -1,
        AcceptRole,
        RejectRole,
        DestructiveRole,
        ActionRole,
        HelpRole,
        NRoles
    };

    enum StandardButton {
        NoButton           = 0x00000000,
        Ok                 = 0x00000001,
        Save               = 0x00000002,
        SaveAll            = 0x00000004,
        Open               = 0x00000008,
        Yes                = 0x00000010,
        YesToAll           = 0x00000020,
        No                 = 0x00000040,
        NoToAll            = 0x00000080,
        Abort              = 0x00000100,
        Retry              = 0x00000200,
        Ignore             = 0x00000400,
        Close              = 0x00000800,
        Cancel             = 0x00001000,
        Discard            = 0x00002000,
        Help               = 0x00004000,
        Apply              = 0x00008000,
        Reset              = 0x00010000
    };

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    explicit QMessageBoxEx(QWidget *parent = 0);
    QMessageBoxEx(const QString &caption, const QString &text, Icon icon,
                  QWidget *parent = 0, 
                  Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    ~QMessageBoxEx();

    int addButton(const QString &text, ButtonRole role);
    int addButton(StandardButton button);

    int defaultButton() const;
    void setDefaultButton(int id);

    int escapeButton() const;
    void setEscapeButton(int id);

    QPushButton *button(int id) const;

    QString text() const;
    void setText(const QString &text);

    Icon icon() const;
    void setIcon(Icon);

    QPixmap iconPixmap() const;
    void setIconPixmap(const QPixmap &pixmap);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat);

    static StandardButton information(QWidget *parent, const QString &caption,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    static StandardButton question(QWidget *parent, const QString &caption,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    static StandardButton warning(QWidget *parent, const QString &caption,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    static StandardButton critical(QWidget *parent, const QString &caption,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    static void about(QWidget *parent, const QString &caption, const QString &text);
    static void aboutQt(QWidget *parent, const QString &caption=QString());

protected:
    bool event(QEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void changeEvent(QEvent *);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_buttonClicked(QAbstractButton *))

    Q_DISABLE_COPY(QMessageBoxEx)
    Q_DECLARE_PRIVATE(QMessageBoxEx)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMessageBoxEx::StandardButtons)

#endif // QT_NO_MESSAGEBOXEX

QT_END_HEADER

#endif // QMESSAGEBOXEX_H
