#include <QObject>

namespace Foo {
    class Bar : public QObject {
        Q_OBJECT
        Q_FLAGS( Flags )
        Q_PROPERTY( Flags flags READ flags WRITE setFlags )
    public:
        explicit Bar( QObject * parent=0 ) : QObject( parent ), mFlags() {}

        enum Flag { Read=1, Write=2 };
        Q_DECLARE_FLAGS( Flags, Flag )

        void setFlags( Flags f ) { mFlags = f; }
        Flags flags() const { return mFlags; }

    private:
        Flags mFlags;
    };

    class Baz : public QObject {
        Q_OBJECT
        //Q_PROPERTY( Bar::Flags flags READ flags WRITE setFlags ) // triggers assertion
        Q_PROPERTY( Foo::Bar::Flags flags READ flags WRITE setFlags ) // fails to compile, or with the same assertion if moc fix is applied
    public:
        explicit Baz( QObject * parent=0 ) : QObject( parent ), mFlags() {}

        void setFlags( Bar::Flags f ) { mFlags = f; }
        Bar::Flags flags() const { return mFlags; }
    private:
        Bar::Flags mFlags;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Foo::Bar::Flags )
