//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "Pdb.h"
#include "Path.h"
#include "OrbitType.h"
#include "OrbitSession.h"
#include "OrbitThread.h"
#include "Injection.h"
#include "ScopeTimer.h"
#include "Serialization.h"

#ifdef _WIN32
#include "SymbolUtils.h"
#include <tlhelp32.h>
#endif

using namespace std;

//-----------------------------------------------------------------------------
Process::Process() : m_ID(0)
                   , m_Handle(0)
                   , m_Is64Bit(false)
                   , m_CpuUsage(0)
                   , m_DebugInfoLoaded(false)
                   , m_IsRemote(false)
                   , m_IsElevated(false)
{
}

//-----------------------------------------------------------------------------
Process::Process(DWORD a_ID) : m_ID(a_ID)
                             , m_LastUserTime({0})
                             , m_LastKernTime({0})
                             , m_CpuUsage(0.f)
                             , m_Is64Bit(false)
                             , m_DebugInfoLoaded(false)
                             , m_IsElevated(false)
{
    Init();
}

//-----------------------------------------------------------------------------
Process::~Process()
{
    /*if( m_DebugInfoLoaded )
    {
        OrbitSymCleanup( m_Handle );
    }*/
}

//-----------------------------------------------------------------------------
void Process::Init()
{
#ifdef _WIN32
    m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_ID);
    m_Is64Bit = ProcessUtils::Is64Bit(m_Handle);
    m_IsElevated = IsElevated( m_Handle );
    m_UpdateCpuTimer.Start();
#endif
}

//-----------------------------------------------------------------------------
void Process::LoadDebugInfo()
{
#ifdef _WIN32
    if( !m_DebugInfoLoaded )
    {
        if( m_Handle == nullptr )
        {
            m_Handle = GetCurrentProcess();
        }

        // Initialize dbghelp
        //SymInit(m_Handle);

        // Load module information
        /*wstring symbolPath = Path::GetDirectory(this->GetFullName()).c_str();
        SymSetSearchPath(m_Handle, symbolPath.c_str());*/

        // List threads
        //EnumerateThreads();
        m_DebugInfoLoaded = true;
    }
#endif
}

//-----------------------------------------------------------------------------
void Process::SetID( DWORD a_ID )
{
    m_ID = a_ID;
    Init();
}

//-----------------------------------------------------------------------------
Process::ModuleMap_t Process::ListModules()
{
    SCOPE_TIMER_LOG( L"ListModules" );

    ClearTransients();

#ifdef _WIN32
    m_Modules = SymUtils::ListModules(m_Handle);
#endif

    for( auto & pair : m_Modules )
    {
        shared_ptr<Module> & module = pair.second;
        wstring name = ToLower( module->m_Name );
        m_NameToModuleMap[name] = module;
        module->LoadDebugInfo();
    }
    return m_Modules;
}

//-----------------------------------------------------------------------------
void Process::ClearTransients()
{
    m_Functions.clear();
    m_Types.clear();
    m_Globals.clear();
    m_WatchedVariables.clear();
    m_NameToModuleMap.clear();
}

//-----------------------------------------------------------------------------
void Process::EnumerateThreads()
{
    m_Threads.clear();
    m_ThreadIds.clear();

#ifdef _WIN32
    // https://blogs.msdn.microsoft.com/oldnewthing/20060223-14/?p=32173/
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_ID);
    if (h != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(h, &te))
        {
            do
            {
                if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
                {
                    HANDLE thandle;
                    {
                        thandle = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                    }

                    if (thandle == NULL)
                    {
                        //ORBIT_LOG(GetLastErrorAsString());
                        continue;
                    }

                    if (te.th32OwnerProcessID == m_ID)
                    {
                        shared_ptr<Thread> thread = make_shared<Thread>();
                        thread->m_Handle = thandle;
                        thread->m_TID = te.th32ThreadID;
                        m_Threads.push_back( thread );
                    }
                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(h, &te));
        }

        CloseHandle(h);
    }

    for (shared_ptr<Thread> & thread : m_Threads)
    {
        m_ThreadIds.insert(thread->m_TID);
    }
#endif
}

//-----------------------------------------------------------------------------
void Process::UpdateCpuTime()
{
#ifdef _WIN32
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernTime;
    FILETIME userTime;

    double elapsedMillis = m_UpdateCpuTimer.QueryMillis();
    m_UpdateCpuTimer.Start();

    if( GetProcessTimes( m_Handle, &creationTime, &exitTime, &kernTime, &userTime ) )
    {
        unsigned numCores = thread::hardware_concurrency();
        LONGLONG kernMs = FileTimeDiffInMillis( m_LastKernTime, kernTime );
        LONGLONG userMs = FileTimeDiffInMillis( m_LastUserTime, userTime );
        m_LastKernTime = kernTime;
        m_LastUserTime = userTime;
        m_CpuUsage = ( 100.0 * double( kernMs + userMs ) / elapsedMillis ) / numCores;
    }
#endif
}

//-----------------------------------------------------------------------------
void Process::UpdateThreadUsage()
{
    for( auto & thread : m_Threads )
    {
        thread->UpdateUsage();
    }
}

//-----------------------------------------------------------------------------
void Process::SortThreadsByUsage()
{
    sort( m_Threads.begin()
             , m_Threads.end()
             , [](shared_ptr<Thread> & a_T0, shared_ptr<Thread> & a_T1)
             { 
                 return a_T1->m_Usage.Latest() < a_T0->m_Usage.Latest(); 
             } );
}

//-----------------------------------------------------------------------------
void Process::SortThreadsById()
{
    sort( m_Threads.begin()
             , m_Threads.end()
             , [](shared_ptr<Thread> & a_T1, shared_ptr<Thread> & a_T0)
             { 
                 return a_T1->m_TID < a_T0->m_TID; 
             } );
}

//-----------------------------------------------------------------------------
shared_ptr<Module> Process::FindModule( const wstring & a_ModuleName )
{
    wstring moduleName = ToLower( Path::GetFileNameNoExt( a_ModuleName ) );

    for( auto & it : m_Modules )
    {
        shared_ptr<Module> & module = it.second;
        if( ToLower( Path::GetFileNameNoExt( module->m_Name ) ) == moduleName )
        {
            return module;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
Function* Process::GetFunctionFromAddress( DWORD64 a_Address, bool a_IsExact )
{
    DWORD64 address = (DWORD64)a_Address;
    auto it = m_Modules.upper_bound( address );
    if( !m_Modules.empty() && it != m_Modules.begin() )
    {
        --it;
        shared_ptr<Module> & module = it->second;
        if( address < module->m_AddressEnd )
        {
            if( module->m_Pdb != nullptr )
            {
                if( a_IsExact )
                {
                    return module->m_Pdb->GetFunctionFromExactAddress( a_Address );
                }
                else
                {
                    return module->m_Pdb->GetFunctionFromProgramCounter( a_Address );
                }
            }
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
shared_ptr<Module> Process::GetModuleFromAddress( DWORD64 a_Address )
{
	DWORD64 address = (DWORD64)a_Address;
    auto it = m_Modules.upper_bound(address);
	if (!m_Modules.empty() && it != m_Modules.begin())
	{
		--it;
		shared_ptr<Module> & module = it->second;
		return module;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
IDiaSymbol * Process::SymbolFromAddress( DWORD64 a_Address )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->SymbolFromAddress( a_Address );
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
bool Process::LineInfoFromAddress( DWORD64 a_Address, LineInfo & o_LineInfo )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->LineInfoFromAddress( a_Address, o_LineInfo );
    }

    return false;
}

//-----------------------------------------------------------------------------
void Process::LoadSession( const Session& )
{

}

//-----------------------------------------------------------------------------
void Process::SaveSession()
{
    
}

//-----------------------------------------------------------------------------
void Process::RefreshWatchedVariables()
{
    for( shared_ptr<Variable> var : m_WatchedVariables )
    {
        var->SyncValue();
    }
}

//-----------------------------------------------------------------------------
void Process::ClearWatchedVariables()
{
    m_WatchedVariables.clear();
}

//-----------------------------------------------------------------------------
void Process::AddType(Type & a_Type)
{
    bool isPtr = a_Type.m_Name.find(L"Pointer to") != string::npos;
    if (!isPtr)
    {
        unsigned long long typeHash = a_Type.Hash();
        auto it = m_UniqueTypeHash.insert(typeHash);
        if (it.second == true)
        {
            m_Types.push_back(&a_Type);
        }
    }
}

//-----------------------------------------------------------------------------
void Process::AddModule( shared_ptr<Module> & a_Module )
{
    m_Modules[a_Module->m_AddressStart] = a_Module;
}

//-----------------------------------------------------------------------------
void Process::FindPdbs( const vector< wstring > & a_SearchLocations )
{
#ifdef _WIN32
    unordered_map< wstring, vector<wstring> > nameToPaths;

    // Populate list of all available pdb files
    for( const wstring & dir : a_SearchLocations )
    {
        vector< wstring > pdbFiles = Path::ListFiles( dir, L".pdb" );
        for( const wstring & pdb : pdbFiles )
        {
            wstring pdbLower = Path::GetFileName( ToLower( pdb ) );
            nameToPaths[pdbLower].push_back( pdb );
        }
    }

    // Find matching pdb
    for( auto & modulePair : m_Modules )
    {
        shared_ptr<Module> module =  modulePair.second;

        if( !module->m_FoundPdb )
        {
            wstring moduleName = ToLower( module->m_Name );
            wstring pdbName = Path::StripExtension( moduleName ) + L".pdb";

            const vector< wstring > & pdbs = nameToPaths[pdbName];

            for( const wstring & pdb : pdbs )
            {

                module->m_PdbName = pdb;
                module->m_FoundPdb = true;
                module->LoadDebugInfo();

                wstring signature = s2ws( GuidToString( module->m_Pdb->GetGuid() ) );

                if( Contains( module->m_DebugSignature, signature ) )
                {
                    // Found matching pdb
                    module->m_PdbSize = ::tr2::sys::file_size( module->m_PdbName );
                    break;
                }
                else
                {
                    module->m_FoundPdb = false;
                }
            }
        }       
    }
#endif
}

//-----------------------------------------------------------------------------
bool Process::IsElevated( HANDLE a_Process )
{
    bool fRet = false;
#ifdef _WIN32
    HANDLE hToken = NULL;
    if( OpenProcessToken( a_Process, TOKEN_QUERY, &hToken ) ) 
    {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) 
        {
            fRet = Elevation.TokenIsElevated != 0;
        }
    }
    if( hToken ) 
    {
        CloseHandle( hToken );
    }
#endif
    return fRet;
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
bool Process::SetPrivilege( LPCTSTR a_Name, bool a_Enable )
{
    HANDLE hToken;
    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
    {
        ORBIT_ERROR;
        PRINT_VAR( GetLastErrorAsString() );
    }

    TOKEN_PRIVILEGES tp;
    LUID luid;
    if( !LookupPrivilegeValue(NULL, a_Name, &luid ) )
    {
        ORBIT_ERROR;
        PRINT( "LookupPrivilegeValue error: " );
        PRINT_VAR( GetLastErrorAsString() );
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = a_Enable ? SE_PRIVILEGE_ENABLED : 0;

    if( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL ) )
    {
        ORBIT_ERROR;
        PRINT( "AdjustTokenPrivileges error: " );
        PRINT_VAR( GetLastErrorAsString() );
        return false;
    }

    if( GetLastError() == ERROR_NOT_ALL_ASSIGNED )
    {
        PRINT( "The token does not have the specified privilege. \n" );
        return false;
    }

    return true;
}
#endif

//-----------------------------------------------------------------------------
DWORD64 Process::GetOutputDebugStringAddress()
{
#ifdef _WIN32
    auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if( it != m_NameToModuleMap.end() )
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress( GetHandle(), module->m_ModuleHandle, "OutputDebugStringA" );
        return (DWORD64)remoteAddr;
    }
#endif

    return 0;
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetRaiseExceptionAddress()
{
#ifdef _WIN32
    auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if (it != m_NameToModuleMap.end())
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress(GetHandle(), module->m_ModuleHandle, "RaiseException");
        return (DWORD64)remoteAddr;
    }
#endif

    return 0;
}

//-----------------------------------------------------------------------------
void Process::FindCoreFunctions()
{
	return;
#if 0 // XXX: Unreachable code
    SCOPE_TIMER_LOG(L"FindCoreFunctions");

    const auto prio = oqpi::task_priority::normal;
    oqpi_tk::scheduler().workersCount( prio );
    //int numWorkers = oqpi::thread::hardware_concurrency();

    oqpi_tk::parallel_for( "FindAllocFreeFunctions", (int32_t)m_Functions.size(), [&]( int32_t, int32_t a_ElementIndex )
    {
        Function* func = m_Functions[a_ElementIndex];
        const wstring & name = func->Lower();

        if( Contains( name, L"operator new" ) || Contains( name, L"FMallocBinned::Malloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::ALLOC;
        }
        else if( Contains( name, L"operator delete" ) || name == L"FMallocBinned::Free" )
        {
            func->Select();
            func->m_OrbitType = Function::FREE;
        } 
        else if( Contains( name, L"realloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::REALLOC;
        }
    } );
#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Process, 0 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_FullName );
    ORBIT_NVP_VAL( 0, m_ID );
    ORBIT_NVP_VAL( 0, m_IsElevated );
    ORBIT_NVP_VAL( 0, m_CpuUsage );
    ORBIT_NVP_VAL( 0, m_Is64Bit );
    ORBIT_NVP_VAL( 0, m_DebugInfoLoaded );
    ORBIT_NVP_VAL( 0, m_IsRemote );
    ORBIT_NVP_VAL( 0, m_Modules );
    ORBIT_NVP_VAL( 0, m_NameToModuleMap );
    ORBIT_NVP_VAL( 0, m_ThreadIds );
}
