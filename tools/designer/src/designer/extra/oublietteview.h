#ifndef DUNGEONVIEW_H
#define DUNGEONVIEW_H

#include <QtGui/QScrollArea>
class Oubliette;

class OublietteView : public QScrollArea
{
    Q_OBJECT
public:
    OublietteView();
    ~OublietteView();

public slots:
    void scrollToCharacter(const QPoint &pt);

private:
    Oubliette *m_dungeon;
};

#endif
