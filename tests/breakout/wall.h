#ifndef WALL_H
#define WALL_H

#include <qcanvas.h>
#include <qmainwindow.h>
#include <qsocket.h>
#include <qserversocket.h>
#include <qdialog.h>

const double Pi = 3.1415926535;
const int BricksHorizontal = 20;
const int BricksVertical = 20;
const int InitBrickWidth = 30;
const int InitBrickHeight = 20;
const int InitLives = 3;
const int BrickPoints = 10;
const char LevelFile[] = "levels.dat";
const char LevelBeginTag[] = "START LEVEL";
const char LevelMultiplayerTag[] = "MULTIPLAYER LEVEL";
const char LevelEndTag[] = "END LEVEL";
const Q_UINT16 NetworkPort = 7731;

enum PlayerPosition { PlayerUp, PlayerDown };

class GameMain;
class Player;
class Remote;
class RemoteSocket;
class QTime;

class Ball : public QObject, public QCanvasEllipse
{
    Q_OBJECT
public:
    enum { RTTI = 7731 };
    Ball( GameMain *game, Player *pl = 0 );
    ~Ball();
    void advance( int phase );  //virtual
    int rtti() const { return RTTI; }
    void putOnPad();
    void start();
    void stop();
    bool isGlued() const { return glued; }
    void setGlued( bool g );
    static double getMaxBallSpeed() { return maxBallSpeed; }
    static double getMinBallSpeed() { return minBallSpeed; }
public slots:
    void setBall( double x, double y, double ang, double sp );
    void setSpeed( double ang, double sp = 0 );
    void setOwner( Player *player );
signals:
    void ballPosition( double x, double y, double ang, double sp );
    void ballOwnedBy( Player *player );
    void ballSpeed( double ang, double sp );
private:
    bool advanceStep( double period );
    GameMain *game;
    double angle;
    double speed;
    Player *owner;
    double startX;
    double startY;
    bool glued;
    QTime *timer;
    static double maxBallSpeed;
    static double minBallSpeed;
};

class Pad : public QObject, public QCanvasRectangle
{
    Q_OBJECT
public:
    enum { RTTI = 7732 };
    Pad( GameMain *game, Player *pl );
    void advance( int phase );  //virtual
    int rtti() const { return RTTI; }
    Player *getPlayer() const { return player; }
public slots:
    void setPosition( double x );
signals:
    void padPosition( double x );
private:
    GameMain *game;
    Player *player;
    int oldPosition;
};

class Brick : public QObject, public QCanvasRectangle
{
    Q_OBJECT
public:
    enum { RTTI = 7733 };
    Brick( GameMain *game, int x, int y, const QColor& col = QColor( "red" ), int score = 0 );
    virtual ~Brick();
    int rtti() const { return RTTI; }
    virtual void hit();
signals:
    void score( int value );
    void allDestroyed();
    void brickHit( int x, int y );
protected:
    GameMain *game;
    int points;
    QColor color;
    static int bricksToDestroy;
};

class StandardBrick : public Brick
{
public:
    StandardBrick( GameMain *game, int x, int y, const QColor& col, int score = 0 );
    ~StandardBrick();
};

class StoneBrick : public Brick
{
public:
    StoneBrick( GameMain *game, int x, int y, const QColor& col = QColor( "gray" ), int score = 0 );
    ~StoneBrick();
    void hit();
};

class HardBrick : public Brick
{
public:
    HardBrick( GameMain *game, int x, int y, const QColor& col, int hits = 1, int score = 0 );
    ~HardBrick();
    void hit();
protected:
    int hitsLeft;
    void paintIt();
};


class DeathLine : public QObject, public QCanvasLine
{
    Q_OBJECT
public:
    enum { RTTI = 7734 };
    DeathLine( GameMain *game, PlayerPosition position = PlayerDown );
    int rtti() const { return RTTI; }
    void hit( Ball *ball );
signals:
    void killedBall();
private:
    GameMain *game;
};

class Lives : public QObject, public QCanvasText
{
    Q_OBJECT
public:
    enum { RTTI = 7735 };
    Lives( GameMain *g, Player *player, int lives = InitLives );
    int rtti() const { return RTTI; }
    int getLives() const { return number; }
    bool hasLives() const { return number > 0; }
public slots:
    void looseLife();
    void setLives( int value );
signals:
    void newBall();
    void ballLost();
    void noMoreLives();
    void livesChanged( int value );
private:
    GameMain *game;
    void print();
    int number;
};

class Score : public QObject, public QCanvasText
{
    Q_OBJECT
public:
    enum { RTTI = 7736 };
    Score( GameMain *g, Player *player );
    int rtti() const { return RTTI; }
    int getScore() const { return number; }
public slots:
    void addScore( int value );
    void setScore( int value );
signals:
    void scoreChanged( int value );
private:
    GameMain *game;
    void print();
    int number;
};


class TempItems : public QObject
{
    Q_OBJECT
public:
    TempItems( int ms, QCanvas *c );
    TempItems( QCanvasItem *item, int ms );
    ~TempItems();
    void addItem( QCanvasItem *item );
    void start();
    static void deleteAll();
public slots:
    void stop();
private:
    QCanvasItemList list;
    QCanvas *canvas;
    int time;
    static QPtrList<TempItems> allObjects;
};

//------------------------------------------------



class Level : public QObject
{
    Q_OBJECT
public:
    Level( GameMain *g, const QString &filename, const QString &startTag );
    void load( const QString &filename = 0 );
    bool start();
    bool next();
public slots:
    void createLevel( QStringList *levelShape = 0 );
signals:
    void levelCreated( QStringList *levelShape );
private:
    GameMain *game;
    QString levelFile;
    QString levelStartTag;
    QPtrList< QStringList > levels;
};


class Player : public QObject
{
    Q_OBJECT
public:
    Player( GameMain *g, PlayerPosition pos = PlayerDown );
    Pad *getPad() const { return pad; }
    Lives *getLives() const { return lives; }
    Score *getScore() const { return score; }
    PlayerPosition getPosition() const { return position; }
public slots:
    void setMyBall();
signals:
    void livesChange( int value );
protected:
    GameMain *game;
    Pad *pad;
    Lives *lives;
    Score *score;
    PlayerPosition position;
};


class TableView : public QCanvasView
{
public:
    TableView( GameMain *parent = 0, const char *name = 0, WFlags f = 0 );
    void takeMouse( bool take );
protected:
    void contentsMouseMoveEvent( QMouseEvent* me);
    void contentsMousePressEvent( QMouseEvent* me);
    void keyPressEvent( QKeyEvent* e );
    void focusOutEvent( QFocusEvent * e );
private:
    GameMain *game;
};


class GameMain : public QMainWindow
{
    Q_OBJECT
public:
    GameMain( int cols = BricksHorizontal, int rows = BricksVertical, int width = InitBrickWidth, int height = InitBrickHeight, QWidget *parent = 0, const char *name = 0, WFlags f = WType_TopLevel );
    ~GameMain();
    Pad* getPad( Player *pl ) const { return pl->getPad(); }
    Pad* getMyPad() const { return getPad( player ); }
    TableView* getTableView() const { return tableView; }
    QCanvas* getCanvas() const { return canvas; }
    Score *getScore( Player *pl ) const { return pl->getScore(); }
    Score *getMyScore() const { return getScore( player ); }
    Ball *getBall() const { return ball; }
    Player *getMyPlayer() const { return player; }
    Remote *getRemote() const { return remote; }
    bool isMyBall() const { return currentPlayer == player; }
    bool isRunning() const { return running; }
    bool isMultiplayer() const { return multiplayer; }
    void setInverseColors( bool inver );
    const QColor& getBallColor() const { return ballColor; }
    const QColor& getPadColor() const { return padColor; }
    const QColor& getTextColor() const { return textColor; }
    int getBrickWidth() const { return brickWidth; }
    int getBrickHeight() const { return brickHeight; }
    int getBrickCols() const { return brickCols; }
    int getBrickRows() const { return brickHeight; }
    int getAdvancePeriod() const { return advancePeriod; }
    void removeAllBricks();
    bool isTestMode() const { return testMode; }
public slots:
    void killBall();
    void endGame();
    void endLevel();
    void nextLevel();
    void initBall();
    void playerDied();
    void setCurrentPlayer( Player *pl );
    void setItsBall();
    void setMyBall();
    void brickHit( int x, int y );
    void connectionLost();
    void setStartBallSpeed( double speed );

    void newGame();
    void serveMultiGame();
    void joinMultiGame();
    void closeConnection();
    void help();
    void about();
    void toggleColors();
    void togglePause();
    void loadLevels();
    void sensitivity();
    void setBallSpeed();
signals:
    void myBall();
    void start();
    void levelEnd();
    void ballInit();
    void ballStartSpeed( double speed );
protected:
    void keyPressEvent( QKeyEvent* e );
private:
    int brickWidth;
    int brickHeight;
    int brickCols;
    int brickRows;
    bool inverseColors;
    QColor ballColor;
    QColor padColor;
    QColor textColor;
    int advancePeriod;
    double startBallSpeed;
    bool running;
    bool multiplayer;
    QCanvasText *textPaused;
    bool testMode;

    TableView *tableView;
    QCanvas *canvas;
    Ball *ball;
    Player *player;
    Player *player2;
    Player *currentPlayer;
    DeathLine *deathLine;
    Level *level;
    Remote *remote;
    QPopupMenu *optionsMenu;
    QPopupMenu *multiplayerMenu;
    int colors_id;
    int paused_id;
    int closeConn_id;
};



class RemoteServer : public QServerSocket
{
    Q_OBJECT
public:
    RemoteServer( Q_UINT16 port = NetworkPort, QObject *parent = 0, const char *name = 0 );
    void newConnection( int socket );
signals:
    void newRemoteSocket( RemoteSocket *remoteSocket );
};

class RemoteSocket : public QSocket
{
    Q_OBJECT
public:
    RemoteSocket( int socketConnection, QObject *parent = 0, const char *name = 0 );
    RemoteSocket( const QString &host, Q_UINT16 port, QObject *parent = 0, const char *name = 0 );
};

class Remote : public QObject
{
    Q_OBJECT
public:
    Remote( GameMain *g, Q_UINT16 port = NetworkPort );
    Remote( GameMain *g, const QString &host, Q_UINT16 port = NetworkPort );
    bool isServer() const { return iAmServer; }
public slots:
    void sendPad( double x );
    void sendBall( double x, double y, double ang, double sp );
    void sendLives( int lives );
    void sendScore( int score );
    void sendBrickHit( int x, int y );
    void sendDied();
    void sendGameOver();
    void sendMyBall();
    void sendLevel( QStringList *levelShape );
    void sendEndLevel();
    void sendInitBall();
    void sendStartBallSpeed( double speed );
signals:
    void readPad( double x );
    void readBall( double x, double y, double ang, double sp );
    void readLives( int lives );
    void readScore( int score );
    void readDied();
    void readGameOver();
    void readBrickHit( int x, int y );
    void readItsBall();
    void readLevel( QStringList *levelShape );
    void readEndLevel();
    void readInitBall();
    void readStartBallSpeed( double speed );
    void success();
    void failed();
    void connectionClosed();
private slots:
    void remoteSocketCreated( RemoteSocket *remoteSocket );
    void socketReadyRead();
    void socketConnected();
    void socketError( int e );
private:
    GameMain *game;
    void init();
    void send( const QString &line );
    QTextStream stream;
    RemoteSocket *remoteSocket;
    RemoteServer *remoteServer;
    bool iAmServer;
};

class QLineEdit;

class NetworkDialog : public QDialog {
    Q_OBJECT
public:
    static Remote *makeConnection( bool srv, GameMain *parent = 0, const char *name = 0 );
public slots:
    void tryConnect();
    void failed();
private:
    NetworkDialog( bool server, GameMain *parent, const char *name = 0 );
    QLineEdit *editPort;
    QLineEdit *editHost;
    QPushButton *ok;
    Remote *remote;
    bool server;
    GameMain *owner;
    static QString lastPort;
    static QString lastHost;
};

class QSlider;

class SensitivityDialog : public QDialog {
    Q_OBJECT
public:
    SensitivityDialog( int *period, QWidget *parent = 0, const char *name = 0 );
private slots:
    void change();
private:
    QSlider *slider;
    int *value;
};

class BallSpeedDialog : public QDialog {
    Q_OBJECT
public:
    BallSpeedDialog( double *speed, QWidget *parent = 0, const char *name = 0 );
private slots:
    void change();
private:
    QSlider *slider;
    double *value;
};


#endif
