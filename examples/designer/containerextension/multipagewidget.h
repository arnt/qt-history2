#ifndef MULTIPAGEWIDGET_H
#define MULTIPAGEWIDGET_H

#include <QWidget>

class QComboBox;
class QStackedWidget;
class QVBoxLayout;

class MultiPageWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(QString pageTitle READ pageTitle WRITE setPageTitle)

public:
    MultiPageWidget(QWidget *parent = 0);

    QSize sizeHint() const;

    void addPage(QWidget *page);
    void removePage(int index);
    int count();
    int currentIndex();
    void insertPage(int index, QWidget *page);
    void setCurrentIndex(int index);
    QWidget *widget(int index);

    QString pageTitle() const;
    void setPageTitle(QString const &newTitle);

private:
    QStackedWidget *stackWidget;
    QComboBox *comboBox;
    QVBoxLayout *layout;
};

#endif
