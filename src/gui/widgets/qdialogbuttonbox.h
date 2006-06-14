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

#ifndef QDIALOGBUTTONBOX_H
#define QDIALOGBUTTONBOX_H

#include <QtGui/QWidget>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QAbstractButton;
class QPushButton;
class QDialogButtonBoxPrivate;
class Q_GUI_EXPORT QDialogButtonBox : public QWidget
{
    Q_OBJECT
    Q_FLAGS(StandardButtons)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)

public:
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

    enum ButtonLayout {
        WinLayout,
        MacLayout,
        KdeLayout,
        GnomeLayout
    };

    QDialogButtonBox(QWidget *parent = 0);
    QDialogButtonBox(Qt::Orientation orientation, QWidget *parent = 0);
    QDialogButtonBox(StandardButtons buttons, Qt::Orientation orientation = Qt::Horizontal,
                     QWidget *parent = 0);
    ~QDialogButtonBox();

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void addButton(QAbstractButton *button, ButtonRole role);
    QPushButton *addButton(const QString &text, ButtonRole role);
    QPushButton *addButton(StandardButton button);
    void removeButton(QAbstractButton *button);
    void clear();

    QList<QAbstractButton *> buttons() const;
    ButtonRole buttonRole(QAbstractButton *button) const;

    void setStandardButtons(StandardButtons buttons);
    StandardButtons standardButtons() const;
    QPushButton *button(StandardButton which) const;

Q_SIGNALS:
    void clicked(QAbstractButton *button);
    void accepted();
    void helpRequested();
    void rejected();

protected:
    void changeEvent(QEvent *event);
    bool event(QEvent *event);

private:
    Q_DISABLE_COPY(QDialogButtonBox)
    Q_DECLARE_PRIVATE(QDialogButtonBox)
    Q_PRIVATE_SLOT(d_func(), void _q_handleButtonClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_handleButtonDestroyed())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDialogButtonBox::StandardButtons)

QT_END_HEADER

#endif // QDIALOGBUTTONBOX_H
