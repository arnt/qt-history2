//--------------------------------------------------
//
// Universal Component Model, Proposal 4
//      ucm.h:     the core header file
//
//--------------------------------------------------

// Of course, we need Guid

typedef struct Guid_
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
} Guid;

//--------------------------------------------------
//
// The extensible variant datatype
//
//--------------------------------------------------

#ifndef NULL
#define NULL 0
#endif

// Stream is used for marshalling & needs fleshing out
struct Stream
{
   int WriteBytes(char* data, int count);
   int ReadBytes(char* data, int count);
};

// A CastList tells the user which types are
// acceptable to this type, either for casting in or out

struct CastList
{
    struct TypeDescr
    {
	Guid* type;
	enum { From = 1, To = 2, FromTo = From | To };
	int fromTo;
    };

    const int count;
    const TypeDescr* parameter;
};

struct Variant
{
    Variant() { VType=NULL; }

    virtual char* Description() { return NULL; }
    virtual void  Init() {;}
    virtual void  Clear() {;}
    virtual int   Clone(Variant* source) {;}

    virtual CastList* CastInfo() { return NULL; }
    virtual int   AssignMeFrom(Variant* source) { return 0; }
    virtual int   AssignMeTo(Variant* dest) { return 0; }

    virtual int   MarshallMeFrom(Stream* source) { return 0; }
    virtual int   MarshallMeTo(Stream* dest) { return 0; }

    virtual void  Fetch(void** ap) {;}

    // the type identifier
    Guid *VType;

    // the unavoidable union
    union
    {
	struct { char bytes[32]; } stream;
	struct {
	   unsigned int subtype;
	   unsigned long value;
	} integral;
	struct {
	   unsigned int subtype;
	   double value;
	} floating;
	struct {
	   unsigned int subtype;
	   unsigned long length;
	   unsigned long size;
	   void* value;
	} string;
	struct {
	   unsigned int subtype;
	   unsigned long width;
	   unsigned long height;
	   unsigned long size;
	   void* value;
	} image;
    } payload;
};

//--------------------------------------------------
//
// The type library
//
//--------------------------------------------------

// Proposed relation between InterfaceDescription & vtbl layout component:
//
//	queryInterface
//	addRef
//	release
//	interfaceDescription
//	invoke
//	installListener
//	removeListener
//	method 1
//	..
//	method n
//	getProp 1
//	setProp 1
//	..
//	getProp n
//	setProp n

struct InterfaceDescription
{
    // A MethodDescription describes one method. A method has a name,
    // an array of in-parameters and an array of out-parameters. Each
    // parameter is represented by a type, an alternative
    // typedescription for local types and an optional argument name
    // for elaborate context help in a text editor.

    struct MethodDescription
    {
	struct Parameter
	{
	    Guid* type;
	    const char* typdescr;
	    const char* name;
	    enum { In = 1, Out = 2, InOut = In | Out };
	    int inOut;
	};

	const char* name;
	const char* documentation;
	const int count;
	const Parameter* parameters;

	const MethodDescription* next;
    };

    const int methodCount;
    const MethodDescription* methods;

    const int eventCount;
    const MethodDescription* events;

    // A PropertyDescription. Not used yet in the example.
    struct PropertyDescription
    {
	const char* name;
	Guid* type;
	const char* typdescr;
	const char* documentation;
	int set; // -1 undefined
	int get; // -1 undefined

	int designable; // -1 FALSE, -2 TRUE, else method
	int stored; // -1 FALSE, -2 TRUE, else method

	const PropertyDescription* next;
    };

    const int propertyCount;
    const PropertyDescription* properties;
};


//--------------------------------------------------
//
// The component interfaces
//
//--------------------------------------------------


// The unavoidable unknown interface.
// The queryInterface rules of COM apply.

class UnknownInterface
{
public:
    virtual UnknownInterface* queryInterface( const Guid& ) = 0;
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;
};

// the basic dispatch interface that inherits UnknownInterface. It's
// used to explore interfaces during runtime and to do dynamic calls.

// Proposed relation between InterfaceDescription & vtbl layout listener:
//
//	queryInterface
//	addRef
//	release
//	event 1
//	..
//	event n

class DispatchInterface : public UnknownInterface
{
public:

    // returns the interface description of this dispatch interface.
    virtual const InterfaceDescription* interfaceDescription() const = 0;

    // invokes method id with parameters V*. Returns some sort of
    // exception code.
    virtual int invoke( int id, Variant* v ) = 0;

    // installs listener as event listener
    virtual void installListener( UnknownInterface* listener ) = 0;

    // remove listener as event listener
    virtual void removeListener( UnknownInterface* listener ) = 0;
};
