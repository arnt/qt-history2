// __________________________________________________________________
//
// Performance Measurement Classes
// ===============================
//   These classes may be used to do manual performance measurements.
//   They shall _not_ contain any Qt code, so it will be possible to
//   do measurements of native sourcecode without linking to the Qt
//   library
//
//
// How to use (Globally)
// ======================
//
//   PM_MEASURE(x)- Does a measurement, and sets the label for the
//                  given value. The label will be shown with the
//                  value, in the measurement list displayed by
//                  DISPLAY_PERF.
//
//   (PM_INIT)    - Initializes the global delta, to hold the
//                  function call overhead, and resets the
//                  measurement count. Call this to restart a
//                  measurement session. (This is called on object
//                  contruction, and is therefore not usually
//                  needed.)
//
//   (PM_DISPLAY) - Displays the list of measurements performed since
//                  INIT_PERF. If you call this, the performance
//                  statistics will _not_ be displayed at object
//                  destruction, to not clutter the output with huge
//                  amounts of information.
//
//   (PM_DDISPLAY)- Usually called after a PM_DISPLAY to show the
//                  statistics at object destruction.
//
// One may also operate directy on the globally defined performance
// measurement object itself. (globalPM)
//
//
// How to use (Instances)
// ======================
//
// Class name:
//   PM
//
// Functions:
//   init()                     - Same as PM_INIT
//   measure( const char* = 0 ) - Same as PM_MEASURE(x)
//   display                    - Same as PM_DISPLAY
//   ddisplay                   - Same as PM_DDISPLAY
//
//
// How to use (Simple)
// ======================
//
// Class name:
//   PMSimple
//
// Functions:
//   PMSimple( const char* = 0 )- Calls start()
//   ~PMSimple()                - Calls stop()
//   start( const char* = 0 )   - Starts the timer, and sets label
//   stop()                     - Stops the timer, and displays the
//                                time spent since start().
//
//
// Settings
// ========
// ->  #define EXTERN_PERFORMANCE_DATA      // Declare data as extern
//
// When using the measurement macros cross implementation files, make
// sure that the variables and function implementations are only done
// in one file, and define EXTERN_PERFORMANCE_DATA in all other
// files.
//
//
//
// ->  #define USE_INTEL_ASM_CODE  // Force Intel calls under Windows
//
// The performance measuring classes currently supports two methods
// of measuring the time spent.
//   1. Native Windows32 API calls
//   2. Intel assembly calls
// By default, they'll choose the native Win32 API on Windows
// platforms, and Intel calls on others. However, you may also choose
// to use Intel calls on Windows by defining USE_INTEL_ASM_CODE
// before including this headerfile.
//
// !!NOTE!!
// Always use the same timing methods in one application!
// All the functions are inlined, so using different schemes in the
// same executable will create garbage data.
//
//
//
// ->  #define PM_MAX_DATA <value> // Change array size in PM objects
//
// Change PM_MAX_DATA  (or define it before file inclusion) to suit
// the measurements you're doing. By default, the value is 50, which
// should suffice for the everyday measurements. However, in some
// circumstances (such as loops etc.), the number of measurements
// might grow significantly.
//
//
//
// ->  #define PM_MSEC  <value>    // Cycles per millisecond
// ->  #define PM_DELTA <value>    // Cycles per sample
//
// The gloabally defined object globalPM will pause for a second
// while timing the CPU clock cycles, if the Intel timing method is
// used. If you know the average number of clockcycles for 1 milli-
// second, and the number of cycle per sample on your machine, you
// may define these values at compilation time, and the performance
// measuring routines will use these values, instead of the automatic
// timing.
// ( You can get these values by doing a test-run first, and look at
//   the header from the debug output.)
// Simply use the two defines above before the include of
// implementation to adjust these values.
//
//
//
// ->  #define PM_NO_GLOBAL_PM     // Force no globalPM object
//
// If you for some reason do not wish to have a globally defined
// performance measurement object, then you may define PM_NO_GLOBAL.
// However, note that if you do so, you may not use the macros for
// doing measurements, but must use the object directly.
//
//
//   - Marius Storm-Olsen
// __________________________________________________________________
#ifndef PERFORMANCE_MEASUREMENT_MACROS
#define PERFORMANCE_MEASUREMENT_MACROS


// __________________________________________________________________
// Adjustable settings
#ifndef PM_MAX_DATA
// Define the number of elements in the perf_meas array
// NOTE: Warnings will be emitted when measurements are closing in on
// this value.
#define PM_MAX_DATA 50
#endif

#ifndef PM_MSEC
// Define the number of cycles per millisecond
#define PM_MSEC 0
#endif

#ifndef PM_DELTA
// Define the number of cycles overhead per measurement call
#define PM_DELTA 0
#endif

// Use the define below to turn off global performance measurement
//#define PM_NO_GLOBAL_PM

// __________________________________________________________________
// Choose timing implementation (By default, Win32 on Windows)
#if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__) \
    || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
    || defined(__NT__))
# ifndef USE_INTEL_ASM_CODE
    // This will use the Windows Query Performance API
    #define USE_WINDOWS_CODE
    #define CODE_STR "Windows native API"
#endif // USE_INTEL_ASM_CODE (predefined)
#endif // Different Windows platforms

#if !defined(USE_WINDOWS_CODE)
    // This will use the Intel specific assembly code
    # define USE_INTEL_ASM_CODE
    # define CODE_STR "Intel assembly code"
#endif



// __________________________________________________________________
// Windows specific 64 bit integers
#if defined(_MSC_VER)
    #include <windows.h>
    #define IS_MSVC
    // Windows defines %I64u as a 64 bits unsigned value
    #define PM_LLI "I64"
    typedef __int64 I64;

// __________________________________________________________________
// GNU specific 64 bit integers
#elif defined(__GNUC__)
    #include <stdarg.h>
    #include <unistd.h>
    #define IS_GNU
    // BSD & POSIX defines %llu as a 64 bits unsigned value
    #define PM_LLI "ll"
    typedef long long I64;

// __________________________________________________________________
// Unknown - Try POSIX
#else
    #include <stdarg.h>
    #include <unistd.h>
    #define IS_POSIX
    // BSD & POSIX defines %llu as a 64 bits unsigned value
    #define PM_LLI "ll"
    typedef int64_t I64;
#endif



// __________________________________________________________________
// Globals
#ifndef EXTERN_PERFORMANCE_DATA
    const char* PM_str_header1 = ">Performance measurements ------- @ %11" PM_LLI "u";
    const char* PM_str_header1b= "                                 -> %11" PM_LLI "u, using ";
    const char* PM_str_header2 = " 1 ms    : %7" PM_LLI "u cycles %s";
    const char* PM_str_header3 = " Overhead: %7" PM_LLI "u cycles %s";
    const char* PM_str_header4 = " From->To:( Totaltime) Separatetime ( CPU clockcycle) To's textlabel";
    const char* PM_str_sep     = " ----------------------------------------------------------------------------";
    const char* PM_str_main1   = " %03d->%03d:(%6.2lf %3s) %9.2lf ms (%11" PM_LLI "u cyc) ";
    const char* PM_str_footer1 = "<Total   :(%6.2lf %3s) %9.2lf ms (%11" PM_LLI "u cycles)";
    const char* PM_str_nocount = "<>Performance measurements ---- (no measurements done since init()) ------";
    const char* PM_str_crt1    = "*** CRITICAL: Reached maximium number of performance measurements (%u)!";
    const char* PM_str_crt2    = "              Measurement aborted... (Increase ";
    const char* PM_str_wrn1    = "*** Warning: Performance measurement calls closing in on PM_MAX_DATA!";
    const char* PM_str_wrn2    = "***          Currently %u  (Max is %u)";
    const char* PM_str_wrn3    = "***          You may increase buffer by adding '#define PM_MAX_DATA <value>',";
    const char* PM_str_wrn4    = "             before your '#include \"%s\"'";
    I64 perf_ms    = PM_MSEC;
    I64 perf_delta = PM_DELTA;
    int PM_max_data = PM_MAX_DATA;

    void PM_Debug( const char *msg, ... )
    {
        char perf_buffer[256];
        va_list ap;
        va_start( ap, msg );
        vsprintf( perf_buffer, msg, ap );
        va_end( ap );
        strcat( perf_buffer, "\n" );

        // To standard error output
        fprintf( stderr, perf_buffer );

    #ifdef IS_MSVC
        TCHAR perf_wbuffer[256];
        // Convert to Unicode
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, perf_buffer, -1,
                            perf_wbuffer, 256 );

        // To MSVC debugger
        OutputDebugString( perf_wbuffer );
    #endif
    }

#else // EXTERN_PERFORMANCE_DATA
    extern const char* PM_str_header1;
    extern const char* PM_str_header1b;
    extern const char* PM_str_header2;
    extern const char* PM_str_header3;
    extern const char* PM_str_header4;
    extern const char* PM_str_sep;
    extern const char* PM_str_main1;
    extern const char* PM_str_footer1;
    extern const char* PM_str_footer2;
    extern const char* PM_str_nocount;
    extern const char* PM_str_crt1;
    extern const char* PM_str_crt2;
    extern const char* PM_str_wrn1;
    extern const char* PM_str_wrn2;
    extern const char* PM_str_wrn3;
    extern const char* PM_str_wrn4;
    extern I64 perf_ms;
    extern I64 perf_delta;
    extern int PM_max_data;
    extern void PM_Debug( const char *msg, ... );
#endif // EXTERN_PERFORMANCE_DATA



// __________________________________________________________________
// Windows specific performance measurement API
#if defined(USE_WINDOWS_CODE)
    inline bool PM_GetFreq( I64 *value )
    {
        return !!QueryPerformanceFrequency( (LARGE_INTEGER*)value );
    }
    inline bool PM_GetCounter( I64 *value )
    {
        return !!QueryPerformanceCounter( (LARGE_INTEGER*)value );
    }

#else // USE_WINDOWS_CODE - Other platforms using assembly now
    // ______________________________________________________________
    // Intel specific performance measurement API for Windows
    #if defined(USE_INTEL_ASM_CODE) && defined(_MSC_VER)
        I64 PM_GetCPUCycles()
        {
            __asm {
                _emit 0x0F
                _emit 0x31
            }
        }
    // ______________________________________________________________
    // Intel specific performance measurement API for G++
    #elif defined(USE_INTEL_ASM_CODE)
        I64 PM_GetCPUCycles()
        {
            I64 result;
            __asm__ __volatile__("rdtsc" : "=A" (result) :);
            return result;
        }

    // ______________________________________________________________
    // Unknown platform - Help!
    #else
    #error "Performance measurements for this platform not available"
    #endif

    inline bool PM_GetFreq( I64 *value )
    {
        I64 start64, delta64, end64;
        // Make sure we compensate for the actual calling of
        // PM_GetCPUCycles(), eventhough it's usually less than 200
        // cycles
        start64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        end64 = PM_GetCPUCycles();
        delta64 = (end64-start64) / 10;

        start64 = PM_GetCPUCycles();

        // Using sleep might be too crude, but should suffice in
        // most cases. Please help implementing a better way, if
        // you feel sleeping is not good enough!.. :-)
        #if defined(IS_MSVC)
        Sleep(1000);
        #elif defined(IS_GNU) || defined(IS_POSIX)
        sleep(1);
        #endif

        end64 = PM_GetCPUCycles();
        *value = (end64-start64) - 2*delta64;
        return true;
    }
    inline bool PM_GetCounter( I64 *value )
    {
        *value = PM_GetCPUCycles();
        return true;
    }

#endif // USE_WINDOWS_CODE



// __________________________________________________________________
// Declaration
class PM
{
public:
    PM();
    ~PM();

    void init();
    void measure( const char* = 0 );
    void display();
    void ddisplay();

private:
    bool active;
    int perf_count;
    struct PDS {
        I64         perf_value;
        const char* perf_text;
    } perf_meas[PM_MAX_DATA];
};



// __________________________________________________________________
// Global instance for fast x-function performance measuring
#ifndef PM_NO_GLOBAL_PM
# ifndef EXTERN_PERFORMANCE_DATA
    PM globalPM;
# else
    extern PM globalPM;
# endif // EXTERN_PERFORMANCE_DATA
#endif // PM_NO_GLOBAL_PM


// __________________________________________________________________
// Macros
#define PM_INIT       globalPM.init()
#define PM_MEASURE(x) globalPM.measure(x)
#define PM_DISPLAY    globalPM.display()
#define PM_DDISPLAY   globalPM.ddisplay()


// __________________________________________________________________
// Implementation
inline
PM::PM()
{
    if (!perf_ms) {
	// Get number of cycles per millisecond
	PM_GetFreq( &(perf_meas[0].perf_value) );
	perf_ms = perf_meas[0].perf_value / 1000;
    }
    if (!perf_delta) {
	// Get average overhead for measurements
	measure( "0" );  measure( "1" );
	measure( "2" );  measure( "3" );
	measure( "4" );  measure( "5" );
	measure( "6" );  measure( "7" );
	measure( "8" );  measure( "9" );
	measure( "10" ); measure( "11" );
	measure( "12" ); measure( "13" );
	measure( "14" ); measure( "15" );
	measure( "16" ); measure( "17" );
	measure( "18" ); measure( "19" );
	perf_delta = (perf_meas[19].perf_value
		    - perf_meas[0].perf_value) / 20;
    }
    init();
}

inline
PM::~PM()
{
    if ( !active )
	return;
    measure( "(object destruction)" );
    display();
}

inline
void PM::init()
{
    // Erase all previous tracks
    memset( perf_meas, 0, PM_max_data * sizeof(PDS) );

    active = true;
    perf_count = 0;
    measure( "init()" );
    return;
}

inline
void PM::measure( const char* label )
{
    if ( perf_count > PM_max_data - 6 ) {
        if ( perf_count > PM_max_data - 1 ) {
            PM_Debug( PM_str_crt1, PM_max_data );
            PM_Debug( PM_str_crt2 );
            return;
        } else {
            PM_Debug( PM_str_wrn1 );
        }
        PM_Debug( PM_str_wrn2, perf_count, PM_max_data );
        PM_Debug( PM_str_wrn3 );
        PM_Debug( PM_str_wrn4, __FILE__ );
    }

    perf_meas[perf_count].perf_text = label;
    PM_GetCounter( &(perf_meas[perf_count++].perf_value) );
}

inline
void PM::display()
{
    active = false;
    if ( !perf_count ) {
        PM_Debug( PM_str_nocount );
        return;
    }
    // Will contain the sum of measurement time, in a readable
    // format, so no need to a full sized variable
    double sum = 0.0;

    // Time postfix and time divider, used for readable timing
    char* PM_timearray[] = { 0, "ms", "sec", "min",    "hr",   "ERR" };
    int   PM_timediv[]   = { 1,    1,  1000, 60000, 3600000, 3600000 };

    // Index into time postfix, and time divider
    int thousands = 1;

    // Delta between current and next measurement
    I64 _delta_;

    // printf doesn't like PMInt64s, use a secondary buffer
    char buffer[256];

    // Output headers
    PM_Debug( PM_str_header1, perf_meas[0].perf_value );
    sprintf( buffer, PM_str_header1b,
             perf_meas[perf_count-1].perf_value);
    if (CODE_STR) strcat(buffer, CODE_STR); // CE sprintf workaround
    PM_Debug( buffer );
    PM_Debug( PM_str_header2, perf_ms, PM_MSEC ? "(override)" : "" );
    PM_Debug( PM_str_header3, perf_delta, PM_DELTA ? "override" : "average delta" );
    PM_Debug( PM_str_header4 );
    PM_Debug( PM_str_sep );

    for ( int perf_run = 1; perf_run < perf_count; perf_run++ ) {

        // Get delta
        _delta_ = perf_meas[perf_run].perf_value
                - perf_meas[perf_run-1].perf_value
		- perf_delta;

	// Sometimes the overhead is larger that the time passed
	// between two measurements. If so, then set delta to zero,
	// as it's too small for this tool to detect properly. We
	// need to do instruction counting to get a better result.
	// That's for version 2.0 :-)
	if ( _delta_ < 0 )
	    _delta_ = 0;

        // Update current sum
        sum += _delta_ / ((double)perf_ms
               * PM_timediv[thousands]);

        // Time to update time divider?
        if ( sum > 1000.0 ) { ++thousands; sum = sum/1000.0; }

        // Use to secondary buffer, since printf doesn't like
        // PMInt64
        sprintf( buffer, PM_str_main1,
                 perf_run-1, perf_run,           // From, To
                 sum, PM_timearray[thousands],   // Sum, time postfix
                 _delta_/(double)perf_ms,
                 _delta_ );              // Measure in cycles

        if ( perf_meas[perf_run].perf_text )     // Label
            strcat( buffer, perf_meas[perf_run].perf_text );

        // Ouput measurement data
        PM_Debug( buffer );
    }

    // Output footers
    _delta_ = perf_meas[perf_count-1].perf_value
            - perf_meas[0].perf_value
	    - (perf_count * perf_delta);
    // Proper calc, instead of accumulative calc
    sum = _delta_ / ((double)perf_ms
           * PM_timediv[thousands]);
    PM_Debug( PM_str_sep );
    PM_Debug( PM_str_footer1,
               sum, PM_timearray[thousands],
               _delta_/(double)perf_ms,
               _delta_ );
}

inline
void PM::ddisplay()
{
    active = true;
}



// __________________________________________________________________
// Declaration
class PMSimple
{
public:
    PMSimple( const char* = 0 );
    ~PMSimple();

    void start( const char* = 0 );
    void stop();

private:
    const char* perf_name;
    I64 perf_start;
    I64 perf_end;
    bool active;
};



// __________________________________________________________________
// Implementation
inline
PMSimple::PMSimple( const char* name )
    : perf_name( 0 )
{
    start( name );
}

inline
PMSimple::~PMSimple()
{
    stop();
}

inline
void PMSimple::start( const char* name )
{
    if ( !perf_name )
        perf_name = name;

    perf_end = 0;
    active = true;
    PM_GetCounter( &perf_start );
}

inline
void PMSimple::stop()
{
    PM_GetCounter( &perf_end );

    if ( !active )
        return;

    active = false;
    char buffer[256];
    double sum = perf_end - perf_start / (double)perf_ms;
    int thousands = 1;

    // Time postfix and time divider, used for readable timing
    char* PM_timearray[] = { 0, "ms", "sec", "min",    "hr",   "ERR" };
    int   PM_timediv[]   = { 1,    1,  1000, 60000, 3600000, 3600000 };

    while (sum > 1000) {
        ++thousands;
        sum = (perf_end - perf_start) /
              ((double)perf_ms * PM_timediv[thousands]);
    }

    if ( perf_name ) PM_Debug( ">>Label: %s", perf_name );
    PM_Debug( PM_str_header1, perf_start );
    sprintf( buffer, PM_str_header1b, perf_end );
    strcat( buffer, CODE_STR );
    PM_Debug( buffer );
    PM_Debug( PM_str_footer1,
              sum, PM_timearray[thousands],
              (perf_end - perf_start)/(double)perf_ms,
              perf_end - perf_start );
}

#endif // PERFORMANCE_MEASUREMENT_MACROS
