/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
class QDialogButtonBoxPrivate;

class Q_GUI_EXPORT QDialogButtonBox : public QWidget
{
    Q_OBJECT
    Q_ENUMS(LayoutPolicy)
    Q_FLAGS(StandardButtons)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)

public:
    enum ButtonRole {
        InvalidRole = -1,
        AcceptRole,
        RejectRole,
        AlternateRole,
        DestructiveRole,
        ActionRole,
        HelpRole,
        NRoles
    };

    enum StandardButton {
        NoButtons          = 0x00000000,
        Ok                 = 0x00000001,
        Open               = 0x00000002,
        Save               = 0x00000004,
        Cancel             = 0x00000008,
        Close              = 0x00000010,
        DontSave           = 0x00000020,
        Apply              = 0x00000040,
        Reset              = 0x00000080,
        Help               = 0x00000100
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
    QAbstractButton *addButton(const QString &text, ButtonRole role);
    QAbstractButton *addButton(StandardButton button);
    void removeButton(QAbstractButton *button);
    void clear();

    QList<QAbstractButton *> buttons() const;
    ButtonRole buttonRole(QAbstractButton *button) const;

    void setStandardButtons(StandardButtons buttons);
    StandardButtons standardButtons() const;
    QAbstractButton *button(StandardButton which) const;

Q_SIGNALS:
    void clicked(QAbstractButton *button);
    void clicked(int buttonRole);
    void accepted();
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
