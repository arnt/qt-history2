#ifndef UCOM_H
#define UCOM_H

#include <memory.h>
#include <qstring.h>

struct UObject;
struct UInterfaceDescription;


//######### TODO useful error codes, prepare for proper exception
//handling
#define URESULT int
#define URESULT_OK 0
#define URESULT_TYPE_MISMATCH -1
#define URESULT_INVALID_ID 5


// Universal unique interface ids
struct UUid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

// the mandatory unknown interface
struct UUnknownInterface
{
    virtual UUnknownInterface* queryInterface( const UUid& ) = 0;
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;
};

//##### TODO: UUid for dispatch interface. UUid for unknown interface


// the dispatch interface that inherits the unknown interface.. It is
// used to explore interfaces during runtime and to do dynamic calls.
struct UDispatchInterface : public UUnknownInterface
{
    // returns the interface description of this dispatch interface.
    virtual const UInterfaceDescription* interfaceDescription() const = 0;

    // returns the event description of this dispatch interface.
    virtual const UInterfaceDescription* eventDescription() const = 0;

    // invokes method id with parameters V*. Returns some sort of
    // exception code.
    virtual URESULT invoke( int id, UObject* o ) = 0;

    // installs listener as event listener
    virtual void installListener( UDispatchInterface* listener ) = 0;

    // remove listener as event listener
    virtual void removeListener( UDispatchInterface* listener ) = 0;
};


// A type for a UObject
struct UType
{
    virtual const UUid *uuid() const = 0;
    virtual const char *desc() const = 0;

    virtual bool convertFrom( UObject *, UType * ) = 0;
    // virtual private, only called by convertFrom
    virtual bool convertTo( UObject *, UType * ) = 0;

    virtual void clear( UObject * );
    virtual void copy( UObject *, const UObject * );

    static bool isEqual( const UType *t1, const UType *t2 ) {
	return t1 == t2 ||
	       t1->uuid() == t2->uuid() ||
	       !memcmp( t1->uuid(), t2->uuid(), sizeof(UUid) );
    }
};


// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
extern const UUid TID_UType_Null;
extern UType *pUType_Null;



// The magic UObject
struct UObject
{
    UObject();
    ~UObject();
 
    UType *type;

    // the unavoidable union
    union
    {
	bool b;
	
	char c;
	short s;
	int i;
	long l;
	
	unsigned char uc;
	unsigned short us;
	unsigned int ui;
	unsigned long ul;
	
	float f;
	double d;
	
	char byte[16];
	struct {
	   unsigned long size;
	   char* data;
	} bytearray;
	
	void *ptr;
	char* charstar;
	char* utf8;
	char* local8bit;
	
	UUnknownInterface* iface;
	UDispatchInterface* idisp;
	
    } payload;
};


// A parameter description describes one method parameters. A
// parameter has a name, a type and a flag describing whether it's an
// in parameter, an out parameter, or both ways
struct UParameter
{
    const char* name;
    UType *type;
    const char* desc; // most often type->desc() but may be different for generic types like UType_ptr
    enum { In = 1, Out = 2, InOut = In | Out };
    int inOut;
};

// A method description describes one method. A method has a name, an
// id and an array of parameters.
struct UMethod
{
    const char* name;
    int id;
    int count;
    const UParameter* parameter;
};

// A Property description. Not used yet in the example.
struct UProperty
{
    const char* name;
    UType* type;
    int set; // -1 undefined
    int get; // -1 undefined

    int designable; // -1 FALSE, -2 TRUE, else method
    int stored; // -1 FALSE, -2 TRUE, else method
};

// An interface description describes one interface, that is all its
// methods and properties.
struct UInterfaceDescription
{
    int methodCount;
    const UMethod* methods;
    int propertyCount;
    const UProperty* properties;
};


struct UKeyValueItem
{
     // #### cannot we just embed this as UEnumType::Item ? Mangling issue?
    const char* key;
    int value;
};

struct UEnumType : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
    void copy( UObject *, const UObject * );
    
    const char *scope() const; 				// - enumerator scope
    const char *name() const;				// - enumerator name
    
    unsigned int count() const;					// - number of values
    const UKeyValueItem *items() const;				// - the name/value pairs
};


#endif // UCOM_H
