#include "msvc_objectmodel.h"

// Property strings for the XML output ------------------------------
/* 
   All string have the newline and indentation incorporated
   to keep preserve speen during project generation..
*/
const char* _AddModuleNamesToAssembly 		= "\n\t\t\t\tAddModuleNamesToAssembly=\"";
const char* _AdditionalDependencies 		= "\n\t\t\t\tAdditionalDependencies=\"";
const char* _AdditionalIncludeDirectories 	= "\n\t\t\t\tAdditionalIncludeDirectories=\"";
const char* _AdditionalLibraryDirectories 	= "\n\t\t\t\tAdditionalLibraryDirectories=\"";
const char* _AdditionalOptions 			= "\n\t\t\t\tAdditionalOptions=\"";
const char* _AdditionalUsingDirectories 	= "\n\t\t\t\tAdditionalUsingDirectories=\"";
const char* _AssemblerListingLocation 		= "\n\t\t\t\tAssemblerListingLocation=\"";
const char* _AssemblerOutput 			= "\n\t\t\t\tAssemblerOutput=\"";
const char* _BaseAddress 			= "\n\t\t\t\tBaseAddress=\"";
const char* _BasicRuntimeChecks 		= "\n\t\t\t\tBasicRuntimeChecks=\"";
const char* _BrowseInformation 			= "\n\t\t\t\tBrowseInformation=\"";
const char* _BrowseInformationFile 		= "\n\t\t\t\tBrowseInformationFile=\"";
const char* _BufferSecurityCheck 		= "\n\t\t\t\tBufferSecurityCheck=\"";
const char* _CPreprocessOptions 		= "\n\t\t\t\tCPreprocessOptions=\"";
const char* _CallingConvention 			= "\n\t\t\t\tCallingConvention=\"";
const char* _CommandLine 			= "\n\t\t\t\tCommandLine=\"";
const char* _CompileAs 				= "\n\t\t\t\tCompileAs=\"";
const char* _CompileAsManaged 			= "\n\t\t\t\tCompileAsManaged=\"";
const char* _CompileOnly 			= "\n\t\t\t\tCompileOnly=\"";
const char* _Culture				= "\n\t\t\t\tCulture=\"";
const char* _DLLDataFileName 			= "\n\t\t\t\tDLLDataFileName=\"";
const char* _DebugInformationFormat 		= "\n\t\t\t\tDebugInformationFormat=\"";
const char* _DefaultCharIsUnsigned 		= "\n\t\t\t\tDefaultCharIsUnsigned=\"";
const char* _DefaultCharType 			= "\n\t\t\t\tDefaultCharType=\"";
const char* _DelayLoadDLLs	 		= "\n\t\t\t\tDelayLoadDLLs=\"";
const char* _Description 			= "\n\t\t\t\tDescription=\"";
const char* _Detect64BitPortabilityProblems 	= "\n\t\t\t\tDetect64BitPortabilityProblems=\"";
const char* _DisableLanguageExtensions 		= "\n\t\t\t\tDisableLanguageExtensions=\"";
const char* _DisableSpecificWarnings 		= "\n\t\t\t\tDisableSpecificWarnings=\"";
const char* _EnableCOMDATFolding 		= "\n\t\t\t\tEnableCOMDATFolding=\"";
const char* _EnableErrorChecks 			= "\n\t\t\t\tEnableErrorChecks=\"";
const char* _EnableFiberSafeOptimizations 	= "\n\t\t\t\tEnableFiberSafeOptimizations=\"";
const char* _EnableFunctionLevelLinking 	= "\n\t\t\t\tEnableFunctionLevelLinking=\"";
const char* _EnableIntrinsicFunctions 		= "\n\t\t\t\tEnableIntrinsicFunctions=\"";
const char* _EntryPointSymbol 			= "\n\t\t\t\tEntryPointSymbol=\"";
const char* _ErrorCheckAllocations 		= "\n\t\t\t\tErrorCheckAllocations=\"";
const char* _ErrorCheckBounds 			= "\n\t\t\t\tErrorCheckBounds=\"";
const char* _ErrorCheckEnumRange 		= "\n\t\t\t\tErrorCheckEnumRange=\"";
const char* _ErrorCheckRefPointers 		= "\n\t\t\t\tErrorCheckRefPointers=\"";
const char* _ErrorCheckStubData 		= "\n\t\t\t\tErrorCheckStubData=\"";
const char* _ExceptionHandling 			= "\n\t\t\t\tExceptionHandling=\"";
const char* _ExcludedFromBuild 			= "\n\t\t\t\tExcludedFromBuild=\"";
const char* _ExpandAttributedSource 		= "\n\t\t\t\tExpandAttributedSource=\"";
const char* _FavorSizeOrSpeed 			= "\n\t\t\t\tFavorSizeOrSpeed=\"";
const char* _ForceConformanceInForLoopScope 	= "\n\t\t\t\tForceConformanceInForLoopScope=\"";
const char* _ForceSymbolReferences 		= "\n\t\t\t\tForceSymbolReferences=\"";
const char* _ForcedIncludeFiles 		= "\n\t\t\t\tForcedIncludeFiles=\"";
const char* _ForcedUsingFiles 			= "\n\t\t\t\tForcedUsingFiles=\"";
const char* _FullIncludePath 			= "\n\t\t\t\tFullIncludePath=\"";
const char* _FunctionOrder 			= "\n\t\t\t\tFunctionOrder=\"";
const char* _GenerateDebugInformation 		= "\n\t\t\t\tGenerateDebugInformation=\"";
const char* _GenerateMapFile 			= "\n\t\t\t\tGenerateMapFile=\"";
const char* _GeneratePreprocessedFile 		= "\n\t\t\t\tGeneratePreprocessedFile=\"";
const char* _GenerateStublessProxies 		= "\n\t\t\t\tGenerateStublessProxies=\"";
const char* _GenerateTypeLibrary 		= "\n\t\t\t\tGenerateTypeLibrary=\"";
const char* _GlobalOptimizations 		= "\n\t\t\t\tGlobalOptimizations=\"";
const char* _HeaderFileName 			= "\n\t\t\t\tHeaderFileName=\"";
const char* _HeapCommitSize 			= "\n\t\t\t\tHeapCommitSize=\"";
const char* _HeapReserveSize 			= "\n\t\t\t\tHeapReserveSize=\"";
const char* _IgnoreAllDefaultLibraries 		= "\n\t\t\t\tIgnoreAllDefaultLibraries=\"";
const char* _IgnoreDefaultLibraryNames 		= "\n\t\t\t\tIgnoreDefaultLibraryNames=\"";
const char* _IgnoreEmbeddedIDL 			= "\n\t\t\t\tIgnoreEmbeddedIDL=\"";
const char* _IgnoreImportLibrary 		= "\n\t\t\t\tIgnoreImportLibrary=\"";
const char* _IgnoreStandardIncludePath 		= "\n\t\t\t\tIgnoreStandardIncludePath=\"";
const char* _ImportLibrary 			= "\n\t\t\t\tImportLibrary=\"";
const char* _ImproveFloatingPointConsistency 	= "\n\t\t\t\tImproveFloatingPointConsistency=\"";
const char* _InlineFunctionExpansion 		= "\n\t\t\t\tInlineFunctionExpansion=\"";
const char* _InterfaceIdentifierFileName 	= "\n\t\t\t\tInterfaceIdentifierFileName=\"";
const char* _KeepComments 			= "\n\t\t\t\tKeepComments=\"";
const char* _LargeAddressAware 			= "\n\t\t\t\tLargeAddressAware=\"";
const char* _LinkDLL 				= "\n\t\t\t\tLinkDLL=\"";
const char* _LinkIncremental 			= "\n\t\t\t\tLinkIncremental=\"";
const char* _LinkTimeCodeGeneration 		= "\n\t\t\t\tLinkTimeCodeGeneration=\"";
const char* _LinkToManagedResourceFile 		= "\n\t\t\t\tLinkToManagedResourceFile=\"";
const char* _MapExports 			= "\n\t\t\t\tMapExports=\"";
const char* _MapFileName 			= "\n\t\t\t\tMapFileName=\"";
const char* _MapLines 				= "\n\t\t\t\tMapLines =\"";
const char* _MergeSections 			= "\n\t\t\t\tMergeSections=\"";
const char* _MergedIDLBaseFileName 		= "\n\t\t\t\tMergedIDLBaseFileName=\"";
const char* _MidlCommandFile 			= "\n\t\t\t\tMidlCommandFile=\"";
const char* _MinimalRebuild 			= "\n\t\t\t\tMinimalRebuild=\"";
const char* _MkTypLibCompatible			= "\n\t\t\t\tMkTypLibCompatible=\"";
const char* _ModuleDefinitionFile 		= "\n\t\t\t\tModuleDefinitionFile=\"";
const char* _ObjectFile 			= "\n\t\t\t\tObjectFile=\"";
const char* _OmitFramePointers 			= "\n\t\t\t\tOmitFramePointers=\"";
const char* _Optimization 			= "\n\t\t\t\tOptimization =\"";
const char* _OptimizeForProcessor 		= "\n\t\t\t\tOptimizeForProcessor=\"";
const char* _OptimizeForWindows98 		= "\n\t\t\t\tOptimizeForWindows98=\"";
const char* _OptimizeForWindowsApplication 	= "\n\t\t\t\tOptimizeForWindowsApplication=\"";
const char* _OptimizeReferences 		= "\n\t\t\t\tOptimizeReferences=\"";
const char* _OutputDirectory 			= "\n\t\t\t\tOutputDirectory=\"";
const char* _OutputFile 			= "\n\t\t\t\tOutputFile=\"";
const char* _Outputs 				= "\n\t\t\t\tOutputs=\"";
const char* _PrecompiledHeaderFile 		= "\n\t\t\t\tPrecompiledHeaderFile=\"";
const char* _PrecompiledHeaderThrough 		= "\n\t\t\t\tPrecompiledHeaderThrough=\"";
const char* _PreprocessorDefinitions 		= "\n\t\t\t\tPreprocessorDefinitions=\"";
const char* _ProgramDataBaseFileName 		= "\n\t\t\t\tProgramDataBaseFileName=\"";
const char* _ProgramDatabaseFile 		= "\n\t\t\t\tProgramDatabaseFile=\"";
const char* _ProxyFileName 			= "\n\t\t\t\tProxyFileName=\"";
const char* _RedirectOutputAndErrors 		= "\n\t\t\t\tRedirectOutputAndErrors=\"";
const char* _RegisterOutput 			= "\n\t\t\t\tRegisterOutput=\"";
const char* _ResourceOnlyDLL 			= "\n\t\t\t\tResourceOnlyDLL=\"";
const char* _ResourceOutputFileName 		= "\n\t\t\t\tResourceOutputFileName=\"";
const char* _RuntimeLibrary 			= "\n\t\t\t\tRuntimeLibrary=\"";
const char* _RuntimeTypeInfo 			= "\n\t\t\t\tRuntimeTypeInfo=\"";
const char* _SetChecksum 			= "\n\t\t\t\tSetChecksum=\"";
const char* _ShowIncludes 			= "\n\t\t\t\tShowIncludes=\"";
const char* _ShowProgress 			= "\n\t\t\t\tShowProgress=\"";
const char* _SmallerTypeCheck 			= "\n\t\t\t\tSmallerTypeCheck=\"";
const char* _StackCommitSize 			= "\n\t\t\t\tStackCommitSize=\"";
const char* _StackReserveSize 			= "\n\t\t\t\tStackReserveSize=\"";
const char* _StringPooling 			= "\n\t\t\t\tStringPooling=\"";
const char* _StripPrivateSymbols 		= "\n\t\t\t\tStripPrivateSymbols=\"";
const char* _StructMemberAlignment 		= "\n\t\t\t\tStructMemberAlignment=\"";
const char* _SubSystem 				= "\n\t\t\t\tSubSystem=\"";
const char* _SupportUnloadOfDelayLoadedDLL 	= "\n\t\t\t\tSupportUnloadOfDelayLoadedDLL=\"";
const char* _SuppressStartupBanner 		= "\n\t\t\t\tSuppressStartupBanner=\"";
const char* _SwapRunFromCD			= "\n\t\t\t\tSwapRunFromCD=\"";
const char* _SwapRunFromNet 			= "\n\t\t\t\tSwapRunFromNet=\"";
const char* _TargetEnvironment			= "\n\t\t\t\tTargetEnvironment=\"";
const char* _TargetMachine 			= "\n\t\t\t\tTargetMachine=\"";
const char* _TerminalServerAware 		= "\n\t\t\t\tTerminalServerAware=\"";
const char* _ToolName				= "\n\t\t\t\tName=\"";
const char* _ToolPath 				= "\n\t\t\t\tPath=\"";
const char* _TreatWChar_tAsBuiltInType		= "\n\t\t\t\tTreatWChar_tAsBuiltInType=\"";
const char* _TurnOffAssemblyGeneration 		= "\n\t\t\t\tTurnOffAssemblyGeneration=\"";
const char* _TypeLibraryFile 			= "\n\t\t\t\tTypeLibraryFile=\"";
const char* _TypeLibraryName			= "\n\t\t\t\tTypeLibraryName=\"";
const char* _TypeLibraryResourceID 		= "\n\t\t\t\tTypeLibraryResourceID=\"";
const char* _UndefineAllPreprocessorDefinitions = "\n\t\t\t\tUndefineAllPreprocessorDefinitions=\"";
const char* _UndefinePreprocessorDefinitions 	= "\n\t\t\t\tUndefinePreprocessorDefinitions=\"";
const char* _UsePrecompiledHeader 		= "\n\t\t\t\tUsePrecompiledHeader=\"";
const char* _ValidateParameters 		= "\n\t\t\t\tValidateParameters=\"";
const char* _Version 				= "\n\t\t\t\tVersion=\"";
const char* _WarnAsError 			= "\n\t\t\t\tWarnAsError=\"";
const char* _WarnLevel 				= "\n\t\t\t\tWarnLevel=\"";
const char* _WarningLevel 			= "\n\t\t\t\tWarningLevel=\"";
const char* _WholeProgramOptimization		= "\n\t\t\t\tWholeProgramOptimization=\"";

// VCCLCompilerTool -------------------------------------------------

VCCLCompilerTool::VCCLCompilerTool()
    :	BufferSecurityCheck( unset ), 
	CompileOnly( unset ), 
	DefaultCharIsUnsigned( unset ),
	Detect64BitPortabilityProblems( unset ), 
	DisableLanguageExtensions( unset ),
	EnableFiberSafeOptimizations( unset ), 
	EnableFunctionLevelLinking( unset ),
	EnableIntrinsicFunctions( unset ), 
	ExceptionHandling( unset ), 
	ExpandAttributedSource( unset ), 
	ForceConformanceInForLoopScope( unset ),
	GlobalOptimizations( unset ), 
	IgnoreStandardIncludePath( unset ),
	ImproveFloatingPointConsistency( unset ), 
	KeepComments( unset ), 
	MinimalRebuild( unset ), 
	OmitFramePointers( unset ), 
	OptimizeForWindowsApplication( unset ), 
	RuntimeTypeInfo( unset ), 
	ShowIncludes( unset ),
	SmallerTypeCheck( unset ), 
	StringPooling( unset ), 
	SuppressStartupBanner( unset ),
	TreatWChar_tAsBuiltInType( unset ), 
	TurnOffAssemblyGeneration( unset ),
	UndefineAllPreprocessorDefinitions( unset ), 
	WarnAsError( unset ),
	WholeProgramOptimization( unset )
{
}

bool VCCLCompilerTool::parseOption( const char* option )
{
    return FALSE;
}

// VCLinkerTool -----------------------------------------------------

VCLinkerTool::VCLinkerTool()
    :	GenerateDebugInformation( unset ),
	GenerateMapFile( unset ),
	IgnoreAllDefaultLibraries( unset ),
	IgnoreEmbeddedIDL( unset ),
	IgnoreImportLibrary( unset ),
	LinkDLL( unset ),
	LinkTimeCodeGeneration( unset ),
	MapExports( unset ),
	MapLines( unset ),
	RegisterOutput( unset ),
	ResourceOnlyDLL( unset ),
	SetChecksum( unset ),
	StackCommitSize( 0 ),
	StackReserveSize( 0 ),
	SupportUnloadOfDelayLoadedDLL( unset ),
	SuppressStartupBanner( unset ),
	SwapRunFromCD( unset ),
	SwapRunFromNet( unset ),
	TurnOffAssemblyGeneration( unset ),
	TypeLibraryResourceID( 0 )
{
}

bool VCLinkerTool::parseOption( const char* option )
{
    return FALSE;
}

// VCMIDLTool -------------------------------------------------------
VCMIDLTool::VCMIDLTool()
    :	DefaultCharType( midlCharUnsigned ),
	EnableErrorChecks( midlDisableAll ),
	ErrorCheckAllocations( unset ),
	ErrorCheckBounds( unset ),
	ErrorCheckEnumRange( unset ),
	ErrorCheckRefPointers( unset ),
	ErrorCheckStubData( unset ),
	GenerateStublessProxies( unset ),
	GenerateTypeLibrary( unset ),
	IgnoreStandardIncludePath( unset ),
	InterfaceIdentifierFileName( unset ),
	MkTypLibCompatible( unset ),
	StructMemberAlignment( alignNotSet ),
	SuppressStartupBanner( unset ),
	TargetEnvironment( midlTargetWin32 ),
	ValidateParameters( unset ),
	WarnAsError( unset ),
	WarningLevel( midlWarningLevel_0 )
{
}

bool VCMIDLTool::parseOption( const char* option )
{
    return FALSE;
}

// VCCustomBuildTool ------------------------------------------------
VCCustomBuildTool::VCCustomBuildTool()
{
}

// VCResourceCompilerTool -------------------------------------------
VCResourceCompilerTool::VCResourceCompilerTool()
    :   Culture( rcUseDefault ),
	IgnoreStandardIncludePath( unset ),
	ShowProgress( linkProgressNotSet )

{
    PreprocessorDefinitions = "NDEBUG";
}

// VCPostBuildEventTool ---------------------------------------------
VCPostBuildEventTool::VCPostBuildEventTool()
{
    ToolName = "VCPostBuildEventTool";
}

// VCPreBuildEventTool ----------------------------------------------
VCPreBuildEventTool::VCPreBuildEventTool()
{
    ToolName = "VCPreBuildEventTool";
}

// VCPreLinkEventTool -----------------------------------------------
VCPreLinkEventTool::VCPreLinkEventTool()
{
    ToolName = "VCPreLinkEventTool";
}
