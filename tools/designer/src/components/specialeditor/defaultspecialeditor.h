#ifndef DEFAULTSPECIALEDITOR_H
#define DEFAULTSPECIALEDITOR_H

#include "specialeditor.h"
#include "default_extensionfactory.h"
#include <QObject>

class QDialog;
class QLineEdit;
class QWidget;
class QAbstactButton;
struct IPropertySheet;

class QDesignerSpecialEditor : public QObject, public ISpecialEditor
{
    Q_OBJECT
    Q_INTERFACES(ISpecialEditor)
public:
    QDesignerSpecialEditor(QWidget *theWidget, IPropertySheet *properties, QObject *parent);
    ~QDesignerSpecialEditor();

    QWidget *createEditor(QWidget *parent);

public slots:
    void applyChanges();
    void revertChanges();

private:
    QWidget *mWidget;
    QDialog *mDialog;
    QLineEdit *mLe;
    IPropertySheet *mProperties;
};

class ButtonLineEdit;
class QAbstractButton;

class QButtonSpecialEditor : public QObject, public ISpecialEditor
{
    Q_OBJECT
    Q_INTERFACES(ISpecialEditor)
public:
    QButtonSpecialEditor(QAbstractButton *theButton, IPropertySheet *properties, QObject *parent);
    ~QButtonSpecialEditor();

    QWidget *createEditor(QWidget *parent);

public slots:
    void applyChanges();
    void revertChanges();

private:
    QAbstractButton *mButton;
    ButtonLineEdit *mLe;
    QString mOriginalText;
    IPropertySheet *mProperties;
};


class QDesignerSpecialEditorFactory : public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    QDesignerSpecialEditorFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};
#endif
