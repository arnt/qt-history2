
struct MyInterface
{
    virtual ~MyInterface() {}
    virtual void foo() = 0;
};

Q_DECLARE_INTERFACE(MyInterface, "foo.bar.blah")

class Test : public QObject, public MyInterface
{
    Q_OBJECT
};
