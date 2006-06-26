/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __AST_VIEW_H__
#define __AST_VIEW_H__

#include <qwidget.h>
#include <q3ptrlist.h>
#include <q3intdict.h>
#include <qtimer.h>
#include <QGraphicsScene>
#include <QGraphicsView>
//Added by qt3to4:
#include <QTimerEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include "sprites.h"

#define MAX_POWER_LEVEL          1000

class KAsteroidsView : public QWidget
{
    Q_OBJECT
public:
    KAsteroidsView( QWidget *parent = 0, const char *name = 0 );
    virtual ~KAsteroidsView();

    int refreshRate;

    void reset();
    void setRockSpeed( double rs ) { rockSpeed = rs; }
    void addRocks( int num );
    void newGame();
    void endGame();
    void newShip();

    void rotateLeft( bool r ) { rotateL = r; rotateSlow = 5; }
    void rotateRight( bool r ) { rotateR = r; rotateSlow = 5; }
    void thrust( bool t ) { thrustShip = t && shipPower > 0; }
    void shoot( bool s ) { shootShip = s; shootDelay = 0; }
    void setShield( bool s );
    void teleport( bool te) { teleportShip = te && mTeleportCount; }
    void brake( bool b );
    void pause( bool p);

    void showText( const QString &text, const QColor &color, bool scroll=TRUE );
    void hideText();

    int shots() const { return shotsFired; }
    int hits() const { return shotsHit; }
    int power() const { return shipPower; }

    int teleportCount() const { return mTeleportCount; }
    int brakeCount() const { return mBrakeCount; }
    int shieldCount() const { return mShieldCount; }
    int shootCount() const { return mShootCount; }

signals:
    void shipKilled();
    void rockHit( int size );
    void rocksRemoved();
    void updateVitals();

private slots:
    void hideShield();

protected:
    bool readSprites();
    void wrapSprite( QGraphicsItem * );
    void rockHit( AnimatedPixmapItem * );
    void reducePower( int val );
    void addExhaust( double x, double y, double dx, double dy, int count );
    void processMissiles();
    void processShip();
    void processPowerups();
    void processShield();
    double randDouble();
    int randInt( int range );

    virtual void resizeEvent( QResizeEvent *event );
    virtual void timerEvent( QTimerEvent * );

    void showEvent( QShowEvent * );

private:
    QGraphicsScene field;
    QGraphicsView view;
    QMap<int, QList<QPixmap> > animation;
    Q3PtrList<AnimatedPixmapItem> rocks;
    Q3PtrList<KMissile> missiles;
    Q3PtrList<KBit> bits;
    Q3PtrList<KExhaust> exhaust;
    Q3PtrList<KPowerup> powerups;
    KShield *shield;
    AnimatedPixmapItem *ship;
    QGraphicsTextItem *textSprite;

    bool rotateL;
    bool rotateR;
    bool thrustShip;
    bool shootShip;
    bool teleportShip;
    bool brakeShip;
    bool pauseShip;
    bool shieldOn;

    bool vitalsChanged;

    int  shipAngle;
    int  rotateSlow;
    int  rotateRate;
    int  shipPower;

    int shotsFired;
    int shotsHit;
    int shootDelay;

    int mBrakeCount;
    int mShieldCount;
    int mTeleportCount;
    int mShootCount;

    double shipDx;
    double shipDy;

    int  textDy;
    int  mFrameNum;
    bool mPaused;
    int  mTimerId;

    double rockSpeed;
    double powerupSpeed;

    bool can_destroy_powerups;

    QTimer *shieldTimer;
    bool initialized;
};

#endif
