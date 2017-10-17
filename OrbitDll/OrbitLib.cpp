//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TcpClient.h"
#include "OrbitLib.h"
#include "ScopeTimer.h"
#include "ClientTimerManager.h"
#include "Hijacking.h"
#include "CrashHandler.h"
#include "PrintVar.h"

using namespace std;

ClientTimerManager* GTimerManager;

//-----------------------------------------------------------------------------
void Orbit::Init( const string & a_Host )
{
    PRINT_FUNC;
    PRINT_VAR(a_Host);
    
    delete GTimerManager;
    GTimerManager = nullptr;
    
    GTcpClient = make_unique<TcpClient>(a_Host);

    if( GTcpClient->IsValid() )
    {
        GTcpClient->Start();
        GTimerManager = new ClientTimerManager();
    }
    else
    {
        GTcpClient = nullptr;
    }
}

//-----------------------------------------------------------------------------
void Orbit::InitRemote( const string & a_Host )
{
    Init( a_Host );
    
    CrashHandler::SendMiniDump();
}

//-----------------------------------------------------------------------------
HMODULE GetCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule, &hModule);
    return hModule;
}

//-----------------------------------------------------------------------------
void Orbit::DeInit()
{
    if( GTimerManager )
    {
        GTimerManager->Stop();
        delete GTimerManager;
        GTimerManager = nullptr;
    }

    HMODULE module = GetCurrentModule();
    FreeLibraryAndExitThread(module, 0);
}

//-----------------------------------------------------------------------------
void Orbit::Start()
{
    GTimerManager->StartClient();
}

//-----------------------------------------------------------------------------
void Orbit::Stop()
{
    GTimerManager->StopClient();
    Hijacking::DisableAllHooks();
}
