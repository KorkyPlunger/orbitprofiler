#include <Injection.h>
#include <ServerTimerManager.h>
#include <Path.h>
#include <TcpServer.h>
#include <Log.h>
#include <SymbolUtils.h>

#include <gtest/gtest.h>
#include <shellapi.h>

#include <stdio.h>

using namespace std;

ServerTimerManager* GTimerManager = new ServerTimerManager();

namespace UnitTest2
{
    class InjectionTest : public testing::Test {
    protected:
        virtual void SetUp()
        {
            GTcpServer = new TcpServer();
        }

        virtual void TearDown() 
        {
        }
    };

    TEST_F(InjectionTest, InjectNotepad) 
    {
        ShellExecute(0, nullptr, L"C:\\WINDOWS\\system32\\notepad.exe", L"", 0, SW_HIDE);

        ProcessList processList;
        processList.SortByName();
        for (const auto& p : processList.m_Processes)
        {
            printf("%ws, %ws, %ld, %s, %zd\n", p->GetName().c_str(), p->GetFullName().c_str(), p->GetID(),
                p->GetIs64Bit() ? "64" : "32", p->GetThreads().size());
        }
        shared_ptr<Process> GTargetProcess;
        for (const auto& p : processList.m_Processes)
            if (p->GetName() == L"notepad.exe")
            {
                GTargetProcess = p;
                break;
            }
        ASSERT_TRUE(GTargetProcess);

        Injection inject;
        wstring dllName = Path::GetDllPath(GTargetProcess->GetIs64Bit());
        wstring dllPath = Path::GetDllPath(GTargetProcess->GetIs64Bit());

        GTcpServer->Start(19999);

        bool GInjected = inject.Inject(dllPath.c_str(), *GTargetProcess, "OrbitInit", L"localhost", 19999);
        ASSERT_TRUE(GInjected);
        if (GInjected)
        {
            wstring GInjectedProcessW = GTargetProcess->GetName();
            ORBIT_LOG(Format("Injected in %s", GInjectedProcessW.c_str()).c_str());
        }

        //SymUtils::ModuleMap_t modules = SymUtils::ListModules(GTargetProcess->GetHandle());
        GTargetProcess->ListModules();
        shared_ptr<Module> orbitModule = GTargetProcess->FindModule(dllName);
        ASSERT_TRUE(orbitModule);

        int numTries = 10;
        while (!GTcpServer->HasConnection() && numTries-- > 0)
        {
            ORBIT_LOG(Format("Waiting for connection on port %i", 19999).c_str());
            Sleep(100);
        }
        ASSERT_TRUE(GTcpServer->HasConnection());

        GTcpServer->Disconnect();
        ASSERT_FALSE(GTcpServer->HasConnection());
    }
}
