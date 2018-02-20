//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ScopeTimer.h"
#include "Message.h"
#include "Log.h"
#include "PrintVar.h"

using namespace std;

thread_local int CurrentDepth = 0;
thread_local int CurrentDepthLocal = 0;

//-----------------------------------------------------------------------------
void Timer::Start()
{
    m_TID = GetCurrentThreadId();
    m_Depth = CurrentDepth++;
    m_SessionID = Message::GSessionID;
    m_PerfCounter.start();
}

//-----------------------------------------------------------------------------
void Timer::Stop()
{
    m_PerfCounter.stop();
    --CurrentDepth;
}

//-----------------------------------------------------------------------------
ScopeTimer::ScopeTimer( const char* )
{
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
ScopeTimer::~ScopeTimer()
{
    m_Timer.Stop();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer() : m_Millis(nullptr)
{
    ++CurrentDepthLocal;
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer(double* a_Millis) : m_Millis( a_Millis )
{
    ++CurrentDepthLocal;
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer( const wstring & a_Msg ) : m_Millis(nullptr), m_Msg( a_Msg )
{
    wstring tabs;
    for (int i = 0; i < CurrentDepthLocal; ++i)
    {
        tabs += L"  ";
    }
    PRINT( Format(L"%sStarting %s...\n", tabs.c_str(), m_Msg.c_str()) );

    ++CurrentDepthLocal;
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::~LocalScopeTimer()
{
    m_Timer.Stop();
    --CurrentDepthLocal;

    if( m_Millis )
    {
        *m_Millis = m_Timer.ElapsedMillis();
    }

    if( m_Msg.length() > 0 )
    {
        wstring tabs;
        for (int i = 0; i < CurrentDepthLocal; ++i)
        {
            tabs += L"  ";
        }

        PRINT( Format( L"%s%s took %f ms.\n", tabs.c_str(), m_Msg.c_str(), m_Timer.ElapsedMillis() ) );
    }
}

//-----------------------------------------------------------------------------
void ConditionalScopeTimer::Start( const char* a_Name )
{
    //strcpy_s(m_Name, NameSize, a_Name);
    m_Timer.Start();
    m_Active = true;
}

//-----------------------------------------------------------------------------
ConditionalScopeTimer::~ConditionalScopeTimer()
{
    if( m_Active )
    {
        m_Timer.Stop();
    }
}
