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
const char LevelEndTag[] = "END LEVEL";
const Q_UINT16 NetworkPort = 7731;

enum PlayerPosition { PlayerUp, PlayerDown };

class GameMain;
class Player;
class Remote;
class RemoteSocket;

class Ball : public QObject, public QCanvasEllipse 
{
    Q_OBJECT
public:
    enum { RTTI = 7731 };
    Ball( GameMain *game, Player *pl = 0 );
    void advance( int phase );
    int rtti() const { return RTTI; }
    void putOnPad();
public slots:
    void setBall( double x, double y, double ang, double sp );
    void setSpeed( double ang, double sp );
    void setOwner( Player *player );
signals:
    void ballPosition( double x, double y, double ang, double sp );
    void ballOwnedBy( Player *player );
    void ballSpeed( double ang, double sp );
private:
    GameMain *game;
    double angle;
    double speed;
    Player *owner;
};

class Pad : public QObject, public QCanvasRectangle 
{
    Q_OBJECT
public:
    enum { RTTI = 7732 };
    Pad( GameMain *game, Player *pl );
    int rtti() const { return RTTI; }
    Player *getPlayer() const { return player; }
public slots:
    void setPosition( double x );
signals:
    void padPosition( double x );
private:
    GameMain *game;
    Player *player;
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
    void addItem( QCanvasItem *item );
    void start();
public slots:
    void stop();
private:
    QCanvasItemList list;
    QCanvas *canvas;
    int time;
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
private:
    void createLevel();
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
signals:
    void livesChange( int value );
    void padChange( int value );
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
    bool mouseTaken;
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
    bool isMyBall() const { return currentPlayer == player; }
    int getBrickWidth() const { return brickWidth; }
    int getBrickHeight() const { return brickHeight; }
    int getBrickCols() const { return brickCols; }
    int getBrickRows() const { return brickHeight; }
    double getMaxBallSpeed() const { return maxBallSpeed; }
    bool getRunning() const { return running; }
    bool getGluedBall() const { return gluedBall; }
    void setGluedBall( bool glued );
    const QColor& getBallColor() const { return ballColor; }
    const QColor& getPadColor() const { return padColor; }
    const QColor& getTextColor() const { return textColor; }
    void setInverseColors( bool inver );
public slots:
    void killBall();
    void endGame();
    void endLevel();
    void startBall();
    void newGame( bool multi = FALSE );
    void serveMultiGame();
    void joinMultiGame();
    void help();
    void about();
    void toggleColors();
    void setCurrentPlayer( Player *pl ) { currentPlayer = pl; }
signals:
    void lostLife();
protected:
    void keyPressEvent( QKeyEvent* e );
private:
    TableView *tableView;
    QCanvas *canvas;
    QPopupMenu *optionsMenu;
    int colors_id;

    int brickWidth;
    int brickHeight;
    int brickCols;
    int brickRows;
    bool inverseColors;
    QColor ballColor;
    QColor padColor;
    QColor textColor;
    int advancePeriod;
    double maxBallSpeed;
    bool running;
    bool gluedBall;
    bool multiplayer;

    Ball *ball;
    Player *player;
    Player *player2;
    Player *currentPlayer;
    DeathLine *deathLine;
    Level *level;
    Remote *remote;
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
    Remote( Q_UINT16 port = NetworkPort, QObject *parent = 0);
    Remote( const QString &host, Q_UINT16 port = NetworkPort, QObject *parent = 0);
    bool isServer() const { return iAmServer; }
public slots:
    void sendPad( double x );
    void sendBall( double x, double y, double ang, double sp );
    void sendLives( int lives );
    void sendScore( int score );
    //void sendDied();
    //void ownBall();
signals:
    void readPad( double x );
    void readBall( double x, double y, double ang, double sp );
    void readLives( int lives );
    void readScore( int score );
    void readDied();
    void myBall();
    void success();
    void failed();
private slots:
    void remoteSocketCreated( RemoteSocket *remoteSocket );
    void socketReadyRead();
    void socketConnected();
    void socketConnectionClosed();
    void socketError( int e );
    //void closeConnection();
    //void socketClosed();
private:
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
    static Remote *makeConnection( bool srv, QWidget *parent = 0, const char *name = 0 );
public slots:
    void tryConnect();
    void failed();
private:
    NetworkDialog( bool server, QWidget *parent = 0, const char *name = 0 );
    QLineEdit *editPort;
    QLineEdit *editHost;
    QPushButton *ok;
    Remote *remote;
    bool server;
    QObject *owner;
};



#endif