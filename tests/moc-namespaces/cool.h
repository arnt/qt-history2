#ifndef COOL_H
#define COOL_H

#include <qwidget.h>

namespace Fett {
    using namespace std;
    using std::printf;

    class CoolGuy : public QWidget {
	Q_OBJECT
    public:
	CoolGuy(){};
    signals:
	void hadMartini();
    public slots:
        void highFive( int times );
        void chillOut();
    };

    class BoringGuy : public QWidget {
	Q_OBJECT
    public:
	BoringGuy(){};
    signals:
	void fellAsleep();	
    public slots:
        void recitePi();
	void playChess() const;
    };


    class FunnyGuy : public CoolGuy {
        Q_OBJECT
    public:
	FunnyGuy(){};
    signals:
        void giggled();
    public slots:
        void tellJoke();
    };

    namespace Arne
    {
	namespace And {

	    class Anda : public Fett::CoolGuy {
		Q_OBJECT
	    public:
		Anda(){};
	    public slots:
	        void beenThere();
	    signals:
		void doneThat();
	    };
	}
    }

}

namespace Fetere {

    class CoolGuy : public Fett::CoolGuy {
	Q_OBJECT
    public:
	CoolGuy(){};
    signals:
	void hadMartini();
    public slots:
        void highFive( int times );
        void chillOutWith( Fett::CoolGuy & );
    };

    class BoringGuy : public QWidget {
	Q_OBJECT
    public:
	BoringGuy(){};
    signals:
	void fellAsleep();	
    public slots:
        void recitePi();
	void playChess() const;
    };

    class FunnyGuy : public CoolGuy {
        Q_OBJECT
    public:
	FunnyGuy(){};
    signals:
        void giggled();
    public slots:
        void tellJoke();
    };

    namespace Arne
    {
	namespace And {

	    class Anda : public QWidget {
		Q_OBJECT
	    public:
		Anda(){};
	    public slots:
	        void beenThere();
	    signals:
		void doneThat();
	    };
	}
    }
}

namespace F = Fetere;

using Fetere::CoolGuy;

namespace Arne
{

    namespace And {

	class Anda : public Fett::BoringGuy  {
	    Q_OBJECT
	public:
	    Anda(){};
        public slots:
	    void doneThat( const Anda & );
	private:
	    const Anda *i;
	};


    }
}


inline void Arne::And::Anda::doneThat( const Anda &f )
{
    warning( "Arne::And::Anda::doneThat" );
    i = &f;
}

#endif
