
#include "CaptureSerializer.h"
#include "Serialization.h"
#include "ScopeTimer.h"

#include "Capture.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "App.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"

#include <fstream>
#include <memory>

using namespace std;

//-----------------------------------------------------------------------------
CaptureSerializer::CaptureSerializer()
{
    m_Version = 2;
    m_TimerVersion = Timer::Version;
    m_SizeOfTimer = sizeof(Timer);
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Save( const wstring a_FileName )
{
    Capture::PreSave();

    basic_ostream<char> Stream( &GStreamCounter );
    cereal::BinaryOutputArchive CountingArchive( Stream );
    GStreamCounter.Reset();
    Save( CountingArchive );
    PRINT_VAR( GStreamCounter.Size() );
    GStreamCounter.Reset();

    // Binary
    m_CaptureName = ws2s(a_FileName);
    ofstream myfile( m_CaptureName, ios::binary );
    if( !myfile.fail() )
    {
        SCOPE_TIMER_LOG( Format( L"Saving capture in %s", a_FileName.c_str() ) );
        cereal::BinaryOutputArchive archive( myfile );
        Save( archive );
        myfile.close();
    }
}

//-----------------------------------------------------------------------------
template <class T> void CaptureSerializer::Save( T & a_Archive )
{
    m_NumTimers = m_TimeGraph->m_TextBoxes.size();

    // Header
    a_Archive( cereal::make_nvp( "Capture", *this ) );

    // Functions
    {
        ORBIT_SIZE_SCOPE( "Functions" );
        vector<Function> functions;
        for( auto & pair : Capture::GSelectedFunctionsMap )
        {
            Function * func = pair.second;
            if( func )
            {
                functions.push_back(*func);
                functions.back().m_Address = func->GetVirtualAddress();
            }
        }

        a_Archive(functions);
    }

    // Function Count
    a_Archive( Capture::GFunctionCountMap );

    // Process
    {
        ORBIT_SIZE_SCOPE( "Capture::GTargetProcess" );
        a_Archive( Capture::GTargetProcess );
    }

    // Callstacks
    {
        ORBIT_SIZE_SCOPE( "Capture::GCallstacks" );
        a_Archive( Capture::GCallstacks );
    }

    // Sampling profiler
    {
        ORBIT_SIZE_SCOPE( "SamplingProfiler" );
        a_Archive( Capture::GSamplingProfiler );
    }

    // Event buffer
    {
        ORBIT_SIZE_SCOPE( "Event Buffer" );
        a_Archive( GEventTracer.GetEventBuffer() );
    }

    // Timers
    int numWrites = 0;
    for( TextBox & box : m_TimeGraph->m_TextBoxes )
    {
        a_Archive( cereal::binary_data( (char*)&box.GetTimer(), sizeof( Timer ) ) );

        if( ++numWrites > m_NumTimers )
        {
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Load( const wstring a_FileName )
{
    SCOPE_TIMER_LOG( Format( L"Loading capture %s", a_FileName.c_str() ) );

#ifdef _WIN32
    // Binary
    ifstream file( ws2s(a_FileName), ios::binary );
    if( !file.fail() )
    {
        // header
        cereal::BinaryInputArchive archive( file );
        archive( *this );

        // functions
        shared_ptr<Module> module = make_shared<Module>();
        Capture::GTargetProcess->AddModule(module);

        module->m_Pdb = make_shared<Pdb>( a_FileName.c_str() );
        archive( module->m_Pdb->GetFunctions() );
        module->m_Pdb->ProcessData();
        GPdbDbg = module->m_Pdb;

        Capture::GSelectedFunctionsMap.clear();
        for( Function & func : module->m_Pdb->GetFunctions() )
        {
            Capture::GSelectedFunctionsMap[func.m_Address] = &func;
        }
        Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

        // Function count
        archive( Capture::GFunctionCountMap );

        // Process
        archive( Capture::GTargetProcess );

        // Callstacks
        archive( Capture::GCallstacks );

        // Sampling profiler
        archive( Capture::GSamplingProfiler );
        Capture::GSamplingProfiler->SortByThreadUsage();
        GOrbitApp->AddSamplingReport( Capture::GSamplingProfiler );
        Capture::GSamplingProfiler->SetLoadedFromFile( true );

        // Event buffer
        archive( GEventTracer.GetEventBuffer() );

        // Timers
        Timer timer;
        while( file.read( (char*)&timer, sizeof(Timer) ) )
        {
            m_TimeGraph->ProcessTimer( timer );
            m_TimeGraph->UpdateThreadDepth( timer.m_TID, timer.m_Depth );
        }

        GOrbitApp->FireRefreshCallbacks();
    }

#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CaptureSerializer, 0 )
{
    ORBIT_NVP_VAL( 0, m_CaptureName );
    ORBIT_NVP_VAL( 0, m_Version );
    ORBIT_NVP_VAL( 0, m_TimerVersion );
    ORBIT_NVP_VAL( 0, m_NumTimers );
    ORBIT_NVP_VAL( 0, m_SizeOfTimer );
}
