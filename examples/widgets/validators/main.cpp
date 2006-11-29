#include <qdebug.h>
#include <QApplication>
#include <QLineEdit>
#include <QValidator>

#include "ui_validators.h"

class ValidatorWidget : public QWidget, public Ui::ValidatorsForm
{
    Q_OBJECT
public:
    ValidatorWidget(QWidget *parent = 0);

private slots:
    void updateValidator();
    void updateDoubleValidator();
    void _setLocale(const QLocale &l) { setLocale(l); updateValidator(); updateDoubleValidator(); }

private:
    QIntValidator *validator;
    QDoubleValidator *doubleValidator;
};

ValidatorWidget::ValidatorWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(localeSelector, SIGNAL(localeSelected(QLocale)), this, SLOT(_setLocale(QLocale)));

    connect(minVal, SIGNAL(editingFinished()), this, SLOT(updateValidator()));
    connect(maxVal, SIGNAL(editingFinished()), this, SLOT(updateValidator()));
    connect(editor, SIGNAL(editingFinished()), ledWidget, SLOT(flash()));

    connect(doubleMaxVal, SIGNAL(editingFinished()), this, SLOT(updateDoubleValidator()));
    connect(doubleMinVal, SIGNAL(editingFinished()), this, SLOT(updateDoubleValidator()));
    connect(doubleDecimals, SIGNAL(valueChanged(int)), this, SLOT(updateDoubleValidator()));
    connect(doubleFormat, SIGNAL(activated(int)), this, SLOT(updateDoubleValidator()));
    connect(doubleEditor, SIGNAL(editingFinished()), doubleLedWidget, SLOT(flash()));

    validator = 0;
    doubleValidator = 0;
    updateValidator();
    updateDoubleValidator();
};

void ValidatorWidget::updateValidator()
{
    QIntValidator *v = new QIntValidator(minVal->value(), maxVal->value(), this);
    v->setLocale(locale());
    editor->setValidator(v);
    delete validator;
    validator = v;

    QString s = editor->text();
    int i = 0;
    if (validator->validate(s, i) == QValidator::Invalid) {
        editor->clear();
    } else {
        editor->setText(s);
    }
}

void ValidatorWidget::updateDoubleValidator()
{
    QDoubleValidator *v
        = new QDoubleValidator(doubleMinVal->value(), doubleMaxVal->value(),
                                doubleDecimals->value(), this);
    v->setNotation(static_cast<QDoubleValidator::Notation>(doubleFormat->currentIndex()));
    v->setLocale(locale());
    doubleEditor->setValidator(v);
    delete doubleValidator;
    doubleValidator = v;

    QString s = doubleEditor->text();
    int i = 0;
    if (doubleValidator->validate(s, i) == QValidator::Invalid) {
        doubleEditor->clear();
    } else {
        doubleEditor->setText(s);
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ValidatorWidget w;
    w.show();

    return app.exec();
}

#include "main.moc"
