//--------------------------------------------------
//
// Universal Component Model, Proposal 4
//      libucm.cpp:     implementation of core
//			datatypes & utilities
//
//--------------------------------------------------

#include <ucm/libucm.h>
#include <dlfcn.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

//--------------------------------------------------
//
// Utility functions
//
//--------------------------------------------------

extern "C" {

typedef struct _ucmguid_ { int a,b,c,d; } Block16;

int TypeEqual(Guid* a, Guid* b)
{
    Block16* x = (Block16*)a;
    Block16* y = (Block16*)b;
    return (x->a==y->a && x->b==y->b && x->c==y->c && x->d==y->d) ? 1 : 0;
}

void setGuid(Guid* a, Guid* b)
{
    Block16* x = (Block16*)a;
    Block16* y = (Block16*)b;
    x->a = y->a; x->b=y->b; x->c=y->c; x->d=y->d;
}

// No "proof of concept" code here yet
char* LookupTypeDLL(Guid *)
{
	return "libucm.so";
}

Variant* NewVariant(Guid* t)
{
    Variant* v;

    // LookupTypeDLL finds the DLL name that has registered
    // the type guid t in some database
    char* fn = LookupTypeDLL( t );
    void* handle = dlopen(fn, RTLD_LAZY);
    if( !handle ) return NULL;
    void* vp = dlsym(handle, "VariantFactory");
    if( !vp ) return NULL;

    Variant* (*fp)() = (Variant*(*)())vp;
    v = (*fp)(t);
}

void FreeVariant(Variant* v)
{
    v->Clear();
    delete v;
}

int ChangeType(Variant* v, Guid* type)
{
  if(v->VType != type)
  {
    Variant* v2 = NewVariant( type );
    if( !v2 ) return false;
    if ( v2->AssignMeTo(v) )
    {
      v->Clear();
      *v = *v2;
      FreeVariant( v2 );
      return true;
    }
    else
      FreeVariant( v2 );
      return false;
  }
  return true;
}

Variant CInvoke(DispatchInterface* dif, int fun, ...)
{
    static VariantNone vn;

    void* ap = (void*)(&fun+1);
    int i = 0;
    Guid* paramType;

    const InterfaceDescription* idp = dif->interfaceDescription();
    if( !idp || fun>=idp->methodCount ) return vn;

    const InterfaceDescription::MethodDescription* mdp = idp->methods;
    while( mdp && i<fun ) { mdp = mdp->next; i++; }
    if( !mdp || i!=fun ) return vn;

    const InterfaceDescription::MethodDescription::Parameter* pdp \
	  = mdp->parameters;
    int count = mdp->count;
    Variant* v = (Variant*) new Variant[count];

    for(i=1; i<=count; i++) {
	paramType = pdp[i].type;
	v[i] = *NewVariant( paramType );
	v[i].Fetch( &ap );
	}

    if( !dif->invoke(fun, v) ) {
    	delete v;
	return vn;
	}
    Variant retval = v[0];
    delete v;
    return retval;
}

} // extern "C"


//--------------------------------------------------
//
// Implementation of the core datatypes
//
//--------------------------------------------------

static Guid _None	= { VKindNone,      0, 0, {0,0,0,0,0,0,0,0} };
static Guid _Null	= { VKindNull,      0, 0, {0,0,0,0,0,0,0,0} };
static Guid _Integral	= { VKindIntegral,  0, 0, {0,0,0,0,0,0,0,0} };
static Guid _Floating	= { VKindFloating,  0, 0, {0,0,0,0,0,0,0,0} };
static Guid _String	= { VKindString,    0, 0, {0,0,0,0,0,0,0,0} };
static Guid _Image	= { VKindImage,     0, 0, {0,0,0,0,0,0,0,0} };

Guid* VType_None	=&_None;
Guid* VType_Null	=&_Null;
Guid* VType_Integral	=&_Integral;
Guid* VType_Floating	=&_Floating;
Guid* VType_String	=&_String;
Guid* VType_Image	=&_Image;

extern "C" {

Variant* VariantFactory(Guid* t)
{
    // Not very efficient, but you get the drift...
    if(TypeEqual(t, VType_None))      return new VariantNone;
    if(TypeEqual(t, VType_Null))      return new VariantNull;
    if(TypeEqual(t, VType_Integral))  return new VariantIntegral;
    if(TypeEqual(t, VType_Floating))  return new VariantFloating;
    if(TypeEqual(t, VType_String))    return new VariantString;
    if(TypeEqual(t, VType_Image))     return new VariantImage;
    return NULL;
}

} // extern "C"

//
// "None" - empty type (should perhaps be merged with Variant itself)
//

char* VariantNone::Description() { return "None"; }

void VariantNone::Init()
{
    VType = VType_None;
    memset(&payload, 0, sizeof(payload));
}

void VariantNone::Clear()
{
    Init();
}

int VariantNone::Clone(Variant* source)
{
    if( TypeEqual(VType, source->VType) ) {
	memcpy(&payload, &source->payload, sizeof(payload));
	return 1; }
    else {
	Init();
	return 0; }
}

static CastList NoneCasts = {0, 0};

CastList* VariantNone::CastInfo() { return &NoneCasts; }

int VariantNone::AssignMeFrom(Variant* source)
{
    return 0;
}

int VariantNone::AssignMeTo(Variant* dest)
{
    return 0;
}

int VariantNone::MarshallMeFrom(Stream* source)
{
    return 1;
}

int VariantNone::MarshallMeTo(Stream* dest)
{
    return 1;
}

void VariantNone::Fetch(void** ap)
{
}

//
// "Null" - mainly for use with DB components
//

char* VariantNull::Description() { return "Null"; }

void VariantNull::Init()
{
    VType = VType_Null;
    memset(&payload, 0, sizeof(payload));
}

void VariantNull::Clear()
{
    Init();
}

int VariantNull::Clone(Variant* source)
{
    if( TypeEqual(VType, source->VType) )
	memcpy(&payload, &source->payload, sizeof(payload));
    else
	Init();
}

static CastList NullCasts = {0, 0};

CastList* VariantNull::CastInfo() { return &NullCasts; }

int VariantNull::AssignMeFrom(Variant* source)
{
    return 0;
}

int VariantNull::AssignMeTo(Variant* dest)
{
    return 0;
}

int VariantNull::MarshallMeFrom(Stream* source)
{
    return 1;
}

int VariantNull::MarshallMeTo(Stream* dest)
{
    return 1;
}

void VariantNull::Fetch(void** ap)
{
}

//
// "INTEGRAL" - signed and unsigned integral values of all sizes
//

char* VariantIntegral::Description() { return "Integral"; }

void VariantIntegral::Init()
{
    VType = VType_Integral;
    memset(&payload, 0, sizeof(payload));
}

void VariantIntegral::Clear()
{
    Init();
}

int VariantIntegral::Clone(Variant* source)
{
    if( TypeEqual(VType, source->VType) ) {
	payload.integral.value = source->payload.integral.value;
	return 1;
	}
    else {
	Init();
	return 0;
	}
}

static CastList::TypeDescr IntegralData[] = {
	{ &_Floating, CastList::TypeDescr::FromTo },
	{ &_String, CastList::TypeDescr::FromTo }
};

static CastList IntegralCasts = {
	sizeof(IntegralData)/sizeof(CastList::TypeDescr), IntegralData
};

CastList* VariantIntegral::CastInfo() { return &IntegralCasts; }

int VariantIntegral::AssignMeFrom(Variant* source)
{
    if( TypeEqual(source->VType, VType_Integral) ) {
	payload.integral.value = source->payload.integral.value;
	return 1;
	}
   if( TypeEqual(source->VType, VType_Floating) ) {
	payload.integral.value = (unsigned long)source->payload.floating.value;
	return 1;
	}
    if( TypeEqual(source->VType, VType_String) ) {
	VariantString s;
	s.Clone( (VariantString*)source );
	s.SubCast( VariantString::str_ascii );
	int rc = sscanf( (char*)s.Grab(), "%d", &payload.floating.value );
	s.Init();
	return rc ? 1 : 0;
	}

    // If I can't do it, see if the other type can...
    return source->AssignMeTo( this );
}

int VariantIntegral::AssignMeTo(Variant* dest)
{
    if( TypeEqual(dest->VType, VType_Integral) ) {
	dest->payload.integral.value = payload.integral.value;
	return 1;
	}
    if( TypeEqual(dest->VType, VType_Floating) ) {
	dest->payload.floating.value = (double)payload.integral.value;
	return 1;
	}
    if( TypeEqual(dest->VType, VType_String) ) {
	char tmp[32]; tmp[0] = '\0';
	sprintf(tmp, "%d", payload.integral.value);
	int i = strlen(tmp);

	VariantString* s = (VariantString*)dest;
	s->Init();
	// Matthias: This code to be synchronised with VariantString
	s->payload.string.subtype = VariantString::str_ascii;
	s->payload.string.size = i;
	s->payload.string.length = i;
	s->payload.string.value = malloc(i);
	strcpy((char*)s->payload.string.value, tmp);
	return 1;
	}

    // If I can't do it, see if the other type can...
    return dest->AssignMeFrom( this );
}

int VariantIntegral::MarshallMeFrom(Stream* source)
{
    char intSerial[21];
    if( !source->ReadBytes(intSerial, 21) ) return 0;
    if( !TypeEqual( (Guid*)intSerial, VType_Integral) ) return 0;
    payload.integral.subtype = intSerial[16];
    for(int i=0; i<4; i++)
	payload.stream.bytes[4+i] = intSerial[17+i];
    return 1;
}

int VariantIntegral::MarshallMeTo(Stream* dest)
{
    char intSerial[21];
    setGuid( (Guid*)intSerial, VType);
    intSerial[16] = payload.integral.subtype;
    for(int i=0; i<4; i++)
	intSerial[17+i] = payload.stream.bytes[4+i];
    return dest->WriteBytes(intSerial, 21);
}

void VariantIntegral::Fetch(void** ap)
{
     payload.integral.value = *((long*)*ap)++;
     payload.integral.subtype = int_long;
}

int VariantIntegral::SubCast(int newsubtype)
{
    // Conversion is conceptual for Integral
    payload.integral.subtype = newsubtype;
    return 1;
}

//
// "FLOATING" - float and double values
//

char* VariantFloating::Description() { return "Floating"; }

void VariantFloating::Init()
{
    VType = VType_Floating;
    memset(&payload, 0, sizeof(payload));
}

void VariantFloating::Clear()
{
    Init();
}

int VariantFloating::Clone(Variant* source)
{
    if( TypeEqual(VType, source->VType) ) {
	payload.floating.value = source->payload.floating.value;
	return 1;
	}
    else {
	Init();
	return 0;
	}
}

static CastList::TypeDescr FloatingData[] = {
	{ &_Integral, CastList::TypeDescr::FromTo },
	{ &_String, CastList::TypeDescr::FromTo }
};

static CastList FloatingCasts = {
	sizeof(FloatingData)/sizeof(CastList::TypeDescr), FloatingData
};

CastList* VariantFloating::CastInfo() { return &FloatingCasts; }

int VariantFloating::AssignMeFrom(Variant* source)
{
    if( TypeEqual(source->VType, VType_Floating) ) {
	payload.floating.value = source->payload.floating.value;
	return 1;
	}
   if( TypeEqual(source->VType, VType_Integral) ) {
	payload.floating.value = (double)source->payload.integral.value;
	return 1;
	}
    if( TypeEqual(source->VType, VType_String) ) {
	VariantString s;
	s.Clone( (VariantString*)source );
	s.SubCast( VariantString::str_ascii );
	int rc = sscanf( (char*)s.Grab(), "%f", &payload.floating.value );
	s.Init();
	return rc ? 1 : 0;
	}

    // If I can't do it, see if the other type can...
    return source->AssignMeTo( this );
}

int VariantFloating::AssignMeTo(Variant* dest)
{
    if( TypeEqual(dest->VType, VType_Floating) ) {
	dest->payload.floating.value = payload.floating.value;
	return 1;
	}
    if( TypeEqual(dest->VType, VType_Integral) ) {
	dest->payload.integral.value = (long)payload.floating.value;
	return 1;
	}
    if( TypeEqual(dest->VType, VType_String) ) {
	char tmp[32]; tmp[0] = '\0';
	sprintf(tmp, "%f", payload.floating.value);
	int i = strlen(tmp);

	VariantString* s = (VariantString*)dest;
	s->Init();
	// Matthias: This code to be synchronised with VariantString
	s->payload.string.subtype = VariantString::str_ascii;
	s->payload.string.size = i;
	s->payload.string.length = i;
	s->payload.string.value = malloc(i);
	strcpy((char*)s->payload.string.value, tmp);
	return 1;
	}

    // If I can't do it, see if the other type can...
    return dest->AssignMeFrom( this );
}

int VariantFloating::MarshallMeFrom(Stream* source)
{
    char intSerial[25];
    if( !source->ReadBytes(intSerial, 25) ) return 0;
    if( !TypeEqual( (Guid*)intSerial, VType_Floating) ) return 0;
    payload.floating.subtype = intSerial[16];
    for(int i=0; i<8; i++)
	payload.stream.bytes[4+i] = intSerial[17+i];
    return 1;
}

int VariantFloating::MarshallMeTo(Stream* dest)
{
    char intSerial[25];
    setGuid( (Guid*)intSerial, VType);
    intSerial[16] = payload.floating.subtype;
    for(int i=0; i<8; i++)
	intSerial[17+i] = payload.stream.bytes[4+i];
    return dest->WriteBytes(intSerial, 25);
}

void VariantFloating::Fetch(void** ap)
{
     payload.floating.value = *((double*)*ap)++;
     payload.floating.subtype = flt_double;
}

int VariantFloating::SubCast(int newsubtype)
{
    // Conversion is conceptual for Floating
    payload.floating.subtype = newsubtype;
    return 1;
}

//
// Matthias, pls put STRING here
//

//
// Chuck, pls put IMAGE here
//

