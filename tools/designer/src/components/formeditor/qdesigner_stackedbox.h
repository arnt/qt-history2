#ifndef QDESIGNER_STACKEDBOX_H
#define QDESIGNER_STACKEDBOX_H

#include "formeditor_global.h"

#include <QStackedWidget>
#include <QList>

class QAction;
class QToolButton;

class QT_FORMEDITOR_EXPORT QDesignerStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_OVERRIDE(int currentIndex READ currentIndex DESIGNABLE true)

public:
    QDesignerStackedWidget(QWidget *parent);

    inline QAction *actionPreviousPage() const
    { return m_actionPreviousPage; }

    inline QAction *actionNextPage() const
    { return m_actionNextPage; }

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

public slots:
    void updateButtons();

protected:
    void childEvent(QChildEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void prevPage();
    void nextPage();
    void removeCurrentPage();
    void addPage();

private:
    QToolButton *prev, *next;
    QAction *m_actionPreviousPage;
    QAction *m_actionNextPage;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
};

#endif // QDESIGNER_STACKEDBOX_H
