#ifndef CUSTOMACTION_H
#define CUSTOMACTION_H

#include <qaction.h>

class CustomAction : public QAction
{
    Q_OBJECT
public:
    CustomAction( QObject* parent );

public slots:
    void onActivate();
};

#endif // CUSTOMACTION_H
