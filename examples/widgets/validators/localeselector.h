#ifndef LOCALESELECTOR_H
#define LOCALESELECTOR_H

#include <QComboBox>

class LocaleSelector : public QComboBox
{
    Q_OBJECT

public:
    LocaleSelector(QWidget *parent = 0);

signals:
    void localeSelected(const QLocale &locale);

private slots:
    void emitLocaleSelected(int index);
};

#endif //LOCALESELECTOR_H
