//--------------------------------------------------
//
// Universal Component Model, Proposal 4
//      libucm.h:     core types & utilities
//
//--------------------------------------------------

#include <ucm/ucm.h>

typedef enum {
  VKindNone,
  VKindNull,
  VKindIntegral,
  VKindFloating,
  VKindString,
  VKindImage
} VKind;

//--------------------------------------------------
//
// Utility functions
//
//--------------------------------------------------

extern "C"
{
// Data related

int      TypeEqual(Guid* a, Guid* b);
Variant* NewVariant(Guid* t);
void     FreeVariant(Variant*);
int      ChangeType(Variant* v, Guid* t);

// Component related

UnknownInterface* CreateInstance(Guid* i);
Variant CInvoke(DispatchInterface*, int, ...);
}

//--------------------------------------------------
//
// The core variant datatypes
//
//--------------------------------------------------

extern Guid* VType_None;
extern Guid* VType_Null;
extern Guid* VType_Integral;
extern Guid* VType_Floating;
extern Guid* VType_String;
extern Guid* VType_Image;

struct VariantNone : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);
};

struct VariantNull : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);

    void* Grab() { return NULL; }
};

struct VariantIntegral : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);

    unsigned long Grab() { return payload.integral.value; }
    int SubCast( int newsubtype );

    enum { int_char,  int_short,  int_int,  int_long,
	   int_uchar, int_ushort, int_uint, int_ulong };
};

struct VariantFloating : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);

    double Grab() { return payload.floating.value; }
    int SubCast( int newsubtype );

    enum { flt_float, flt_double };
};

struct VariantString : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);

    void* Grab() { return payload.string.value; }
    int SubCast( int newsubtype );

    enum { str_ascii, str_local8bit, str_utf, str_unicode };
};

struct VariantImage : public Variant
{
    virtual char* Description();
    virtual void  Init();
    virtual void  Clear();
    virtual int   Clone(Variant* source);

    virtual CastList* CastInfo();
    virtual int   AssignMeFrom(Variant* source);
    virtual int   AssignMeTo(Variant* dest);

    virtual int   MarshallMeFrom(Stream* source);
    virtual int   MarshallMeTo(Stream* dest);

    virtual void  Fetch(void** ap);

    void* Grab() { return payload.image.value; }
    int SubCast( int newsubtype );

    enum { img_bmp, img_xpm };
};

