//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <vector>
#include <map>
#include <atomic>
#include <unordered_map>
#include <memory>
#include "Core.h"
#include "Threading.h"
#include "Profiling.h"
#include "Message.h"

class TcpClient;
class Message;
struct ContextSwitch;

//-----------------------------------------------------------------------------
class TimerManager
{
public:
    TimerManager();
    ~TimerManager();

    virtual void StartRecording();
    virtual void StopRecording();
    virtual void Stop();

    virtual void StartClient();
    virtual void StopClient();

    void Add( const Timer & a_Timer );
    void Add( const Message & a_Message );
    void Add( const ContextSwitch & a_CS );

    virtual void ConsumeTimers();
    bool HasQueuedEntries() const { return m_NumQueuedEntries > 0; }
    virtual void FlushQueue();

public:
    AutoResetEvent          m_ConditionVariable;

    std::atomic<bool>       m_Paused;
    std::atomic<bool>       m_IsFull;
    std::atomic<bool>       m_IsRecording;
    std::atomic<bool>       m_ExitRequested;
    std::atomic<bool>       m_FlushRequested;
    std::atomic<int>        m_NumQueuedEntries;
    std::atomic<int>        m_NumQueuedTimers;
    std::atomic<int>        m_NumQueuedMessages;
    std::atomic<int>        m_TimerIndex;
    std::atomic<int>        m_NumTimersFromPreviousSession;
    std::atomic<int>        m_NumFlushedTimers;

    PerfCounter             m_GlobalTimer;
    int                     m_ThreadCounter;
    LockFreeQueue<Timer>    m_LockFreeQueue;
    LockFreeQueue<Message>  m_LockFreeMessageQueue;
    std::thread*            m_ConsumerThread;

    typedef std::function<void(Timer&)> TimerAddedCallback;
    std::vector< TimerAddedCallback > m_TimerAddedCallbacks;

    typedef std::function<void(const struct ContextSwitch&)> ContextSwitchAddedCallback;
    ContextSwitchAddedCallback m_ContextSwitchAddedCallback;
};

//-----------------------------------------------------------------------------
extern TimerManager* GTimerManager;

