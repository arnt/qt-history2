#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include "formeditor_global.h"

#include <QObject>

#include <abstractformwindowcursor.h>
#include "formwindow.h"

class QT_FORMEDITOR_EXPORT FormWindowCursor: public QObject, public AbstractFormWindowCursor
{
    Q_OBJECT
public:
    FormWindowCursor(FormWindow *fw, QObject *parent = 0);
    virtual ~FormWindowCursor();

    virtual FormWindow *formWindow() const;

    virtual bool movePosition(MoveOperation op, MoveMode mode);

    virtual int position() const;
    virtual void setPosition(int pos, MoveMode mode);

    virtual QWidget *current() const;

    virtual int widgetCount() const;
    virtual QWidget *widget(int index) const;

    virtual bool hasSelection() const;
    virtual int selectedWidgetCount() const;
    virtual QWidget *selectedWidget(int index) const;

    virtual void setProperty(const QString &name, const QVariant &value);

public slots:
    void update();

private:
    FormWindow *m_formWindow;
    int m_iterator;
};

#endif // FORMWINDOWCURSOR_H
