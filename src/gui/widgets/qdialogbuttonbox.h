#ifndef QDIALOGBUTTONBOX_H
#define QDIALOGBUTTONBOX_H

#include <QtGui/QWidget>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QAbstractButton;
class QDialogButtonBoxPrivate;
class QDialogButtonBox : public QWidget
{
    Q_OBJECT
    Q_ENUMS(QDialogButtonBox::LayoutPolicy)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QDialogButtonBox::LayoutPolicy layoutPolicy READ layoutPolicy WRITE setLayoutPolicy)
    Q_PROPERTY(QDialogButtonBox::StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)

public:
    enum ButtonRole {
        InvalidRole,
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

    enum LayoutPolicy {
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

    void setLayoutPolicy(LayoutPolicy style);
    LayoutPolicy layoutPolicy() const;

    void addButton(QAbstractButton *button, ButtonRole role);
    QAbstractButton *addButton(const QString &text, ButtonRole role);
    QAbstractButton *addButton(StandardButton button);
    void removeButton(QAbstractButton *button);
    void clear();

    QList<QAbstractButton *> buttons() const;
    ButtonRole buttonRole(QAbstractButton *button) const;

    void setStandardButtons(StandardButtons buttons);
    StandardButtons standardButtons() const;

Q_SIGNALS:
    void clicked(QAbstractButton *button);
    void clicked(int buttonRole);
    void accepted();
    void rejected();

private:
    Q_DISABLE_COPY(QDialogButtonBox)
    Q_DECLARE_PRIVATE(QDialogButtonBox);
    Q_PRIVATE_SLOT(d_func(), void _q_handleButtonClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_handleButtonDestroyed())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDialogButtonBox::StandardButtons)

QT_END_HEADER

#endif // QDIALOGBUTTONBOX_H
