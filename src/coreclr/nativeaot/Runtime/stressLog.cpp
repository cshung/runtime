// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// ---------------------------------------------------------------------------
// StressLog.cpp
//
// StressLog infrastructure
// ---------------------------------------------------------------------------

#include "common.h"
#ifdef DACCESS_COMPILE
#include <windows.h>
#include "sospriv.h"
#endif // DACCESS_COMPILE
#include "CommonTypes.h"
#include "CommonMacros.h"
#include "PalRedhawkCommon.h"
#include "PalRedhawk.h"
#include "daccess.h"
#include "stressLog.h"
#include "holder.h"
#include "Crst.h"
#include "rhassert.h"
#include "slist.h"
#include "gcrhinterface.h"
#include "varint.h"
#include "regdisplay.h"
#include "StackFrameIterator.h"
#include "thread.h"
#include "event.h"
#include "threadstore.h"
#include "threadstore.inl"
#include "thread.inl"

template<typename T> inline T VolatileLoad(T const * pt) { return *(T volatile const *)pt; }
template<typename T> inline void VolatileStore(T* pt, T val) { *(T volatile *)pt = val; }

#ifdef STRESS_LOG

typedef DPTR(StressLog) PTR_StressLog;
GPTR_IMPL(StressLog, g_pStressLog /*, &StressLog::theLog*/);

#ifndef DACCESS_COMPILE

/*********************************************************************************/
#if defined(HOST_X86)

/* This is like QueryPerformanceCounter but a lot faster.  On machines with
   variable-speed CPUs (for power management), this is not accurate, but may
   be good enough.
*/
inline __declspec(naked) unsigned __int64 getTimeStamp() {

   __asm {
        RDTSC   // read time stamp counter
        ret
    };
}

#else // HOST_X86
unsigned __int64 getTimeStamp()
{
    return PalQueryPerformanceCounter();
}

#endif // HOST_X86 else

/*********************************************************************************/
/* Get the frequency corresponding to 'getTimeStamp'.  For non-x86
   architectures, this is just the performance counter frequency.
*/
unsigned __int64 getTickFrequency()
{
    return PalQueryPerformanceFrequency();
}

#endif // DACCESS_COMPILE

StressLog StressLog::theLog = { 0, 0, 0, 0, 0, 0 };
const static unsigned __int64 RECYCLE_AGE = 0x40000000L;        // after a billion cycles, we can discard old threads

/*********************************************************************************/

#ifdef MEMORY_MAPPED_STRESSLOG

bool StressLogChunk::s_memoryMapped = true;

void* StressLog::AllocMemoryMapped(size_t n)
{
    if ((ptrdiff_t)n > 0)
    {
        StressLogHeader* hdr = theLog.stressLogHeader;
        assert(hdr != nullptr);
        uint8_t* oldMemValue = (uint8_t*)PalInterlockedExchangeAdd64((uint64_t*)&hdr->memoryCur, n);
        if (oldMemValue + n < hdr->memoryLimit)
        {
            return oldMemValue;
        }
        // when we run out, we just can't allocate anymore
        hdr->memoryCur = hdr->memoryLimit;
    }
    return nullptr;
}

void* StressLogChunk::operator new (size_t count, const std::nothrow_t& tag)
{
    if (StressLogChunk::s_memoryMapped)
    {
        return StressLog::AllocMemoryMapped(count);
    }
    else 
    {
        return malloc(count);
    }
}

void  StressLogChunk::operator delete (void * chunk)
{
    if (!StressLogChunk::s_memoryMapped)
    {
        free(chunk);
    }
}

void* ThreadStressLog::operator new (size_t count, const std::nothrow_t& tag)
{
    if (StressLogChunk::s_memoryMapped)
    {
        return StressLog::AllocMemoryMapped(count);
    }
    else 
    {
        return malloc(count);
    }
}

void  ThreadStressLog::operator delete (void * chunk)
{
    if (!StressLogChunk::s_memoryMapped)
    {
        free(chunk);
    }
}

#define MAX_PATH 260

void ReplacePid(wchar_t* original, wchar_t* replaced, size_t replacedLength)
{
    // if the string "{pid}" occurs in the logFilename,
    // replace it by the PID of our process
    // only the first occurrence will be replaced
    const wchar_t* pidLit =  L"{pid}";
    const wchar_t* pidPtr = wcsstr(original, pidLit);
    if (pidPtr != nullptr)
    {
        // copy the file name up to the "{pid}" occurrence
        ptrdiff_t pidInx = pidPtr - original;
        wcsncpy(replaced, original, pidInx);

        // append the string representation of the PID
        uint32_t pid = PalGetCurrentProcessId();
        wchar_t pidStr[20];
        swprintf(pidStr, ARRAY_SIZE(pidStr), L"%d", pid);
        
        wcscat(replaced, pidStr);

        // append the rest of the filename
        wcscat(replaced, original + pidInx + wcslen(pidLit));
    }
    else
    {
        size_t originalLength = wcslen(original);
        wcsncpy(replaced, original, originalLength);
    }
}

#endif // MEMORY_MAPPED_STRESSLOG

#ifndef DACCESS_COMPILE

void* PalCreateMemoryMappedFile(wchar_t* logFilename, size_t maxBytesTotal);

void StressLog::Initialize(unsigned facilities,  unsigned level, unsigned maxBytesPerThreadArg,
            unsigned maxBytesTotalArg, HANDLE hMod)
{
    if (theLog.MaxSizePerThread != 0)
    {
        // guard ourself against multiple initialization. First init wins.
        return;
    }

    g_pStressLog = &theLog;

    theLog.pLock = new (nothrow) CrstStatic();
    theLog.pLock->Init(CrstStressLog);
    size_t maxBytesPerThread = maxBytesPerThreadArg;
    if (maxBytesPerThread < STRESSLOG_CHUNK_SIZE)
    {
        // in this case, interpret the number as GB
        maxBytesPerThread *= (1024 * 1024 * 1024);
    }
    theLog.MaxSizePerThread = (unsigned)min(maxBytesPerThread,0xffffffff);

    size_t maxBytesTotal = maxBytesTotalArg;
    if (maxBytesTotal < STRESSLOG_CHUNK_SIZE * 256)
    {
        // in this case, interpret the number as GB
        maxBytesTotal *= (1024 * 1024 * 1024);
    }

    theLog.MaxSizeTotal = maxBytesTotal;
    theLog.totalChunk = 0;
    theLog.facilitiesToLog = facilities | LF_ALWAYS;
    theLog.levelToLog = level;
    theLog.deadCount = 0;

    theLog.tickFrequency = getTickFrequency();

    PalGetSystemTimeAsFileTime (&theLog.startTime);
    theLog.startTimeStamp = getTimeStamp();

    theLog.moduleOffset = (size_t)hMod; // HMODULES are base addresses.

#ifdef MEMORY_MAPPED_STRESSLOG
    StressLogChunk::s_memoryMapped = false;
    wchar_t* logFilename = nullptr;
    logFilename = (wchar_t*)L"D:\\StressLog.{pid}.log";
    if ((logFilename != nullptr) && (maxBytesTotal >= sizeof(StressLog::StressLogHeader)))
    {
        wchar_t logFilenameReplaced[MAX_PATH];
        ReplacePid(logFilename, logFilenameReplaced, MAX_PATH);
        theLog.hMapView = PalCreateMemoryMappedFile(logFilenameReplaced, maxBytesTotal);
        if (theLog.hMapView != nullptr)
        {
            StressLogChunk::s_memoryMapped = true;
            StressLogHeader* hdr = (StressLogHeader*)(uint8_t*)(void*)theLog.hMapView;
            hdr->headerSize = sizeof(StressLogHeader);
            hdr->magic = *(uint32_t*)"LRTS";
            hdr->version = 0x00010001;
            hdr->memoryBase = (uint8_t*)hdr;
            hdr->memoryCur = hdr->memoryBase + sizeof(StressLogHeader);
            hdr->memoryLimit = hdr->memoryBase + maxBytesTotal;
            hdr->logs = nullptr;
            hdr->tickFrequency = theLog.tickFrequency;
            hdr->startTimeStamp = theLog.startTimeStamp;
            theLog.stressLogHeader = hdr;
        }
    }
#endif //MEMORY_MAPPED_STRESSLOG
}

/*********************************************************************************/
/* create a new thread stress log buffer associated with pThread                 */

ThreadStressLog* StressLog::CreateThreadStressLog(Thread * pThread) {
    if (theLog.facilitiesToLog == 0)
        return NULL;

    if (pThread == NULL)
        pThread = ThreadStore::GetCurrentThread();

    ThreadStressLog* msgs = reinterpret_cast<ThreadStressLog*>(pThread->GetThreadStressLog());
    if (msgs != NULL)
    {
        return msgs;
    }

    // if it looks like we won't be allowed to allocate a new chunk, exit early
    if (VolatileLoad(&theLog.deadCount) == 0 && !AllowNewChunk (0))
    {
        return NULL;
    }

    CrstHolder holder(theLog.pLock);

    msgs = CreateThreadStressLogHelper(pThread);

    return msgs;
}

ThreadStressLog* StressLog::CreateThreadStressLogHelper(Thread * pThread) {

    bool skipInsert = FALSE;
    ThreadStressLog* msgs = NULL;

    // See if we can recycle a dead thread
    if (VolatileLoad(&theLog.deadCount) > 0)
    {
        unsigned __int64 recycleStamp = getTimeStamp() - RECYCLE_AGE;
        msgs = VolatileLoad(&theLog.logs);
        //find out oldest dead ThreadStressLog in case we can't find one within
        //recycle age but can't create a new chunk
        ThreadStressLog * oldestDeadMsg = NULL;

        while(msgs != 0)
        {
            if (msgs->isDead)
            {
                bool hasTimeStamp = msgs->curPtr != (StressMsg *)msgs->chunkListTail->EndPtr();
                if (hasTimeStamp && msgs->curPtr->timeStamp < recycleStamp)
                {
                    skipInsert = TRUE;
                    PalInterlockedDecrement(&theLog.deadCount);
                    break;
                }

                if (!oldestDeadMsg)
                {
                    oldestDeadMsg = msgs;
                }
                else if (hasTimeStamp && oldestDeadMsg->curPtr->timeStamp > msgs->curPtr->timeStamp)
                {
                    oldestDeadMsg = msgs;
                }
            }

            msgs = msgs->next;
        }

        //if the total stress log size limit is already passed and we can't add new chunk,
        //always reuse the oldest dead msg
        if (!AllowNewChunk (0) && !msgs)
        {
            msgs = oldestDeadMsg;
            skipInsert = TRUE;
            PalInterlockedDecrement(&theLog.deadCount);
        }
    }

    if (msgs == 0)  {
        msgs = new (nothrow) ThreadStressLog();

        if (msgs == 0 ||!msgs->IsValid ())
        {
            delete msgs;
            msgs = 0;
            goto LEAVE;
        }
    }

    msgs->Activate (pThread);

    if (!skipInsert) {
#ifdef _DEBUG
        ThreadStressLog* walk = VolatileLoad(&theLog.logs);
        while (walk)
        {
            _ASSERTE (walk != msgs);
            walk = walk->next;
        }
#endif
        // Put it into the stress log
        msgs->next = VolatileLoad(&theLog.logs);
        VolatileStore(&theLog.logs, msgs);
    }

LEAVE:
    ;
    return msgs;
}

/*********************************************************************************/
/* static */
void StressLog::ThreadDetach(ThreadStressLog *msgs) {

    if (msgs == 0)
    {
        return;
    }

    // We should write this message to the StressLog for deleted fiber.
    msgs->LogMsg (LF_STARTUP, 0, "******* DllMain THREAD_DETACH called Thread dying *******\n");

    msgs->isDead = TRUE;
    PalInterlockedIncrement(&theLog.deadCount);
}

bool StressLog::AllowNewChunk (long numChunksInCurThread)
{
    Thread* pCurrentThread = ThreadStore::RawGetCurrentThread();

    _ASSERTE (numChunksInCurThread <= VolatileLoad(&theLog.totalChunk));
    uint32_t perThreadLimit = theLog.MaxSizePerThread;

    if (numChunksInCurThread == 0 /*&& IsSuspendEEThread()*/)
        return TRUE;

    if (pCurrentThread->IsGCSpecial())
    {
        perThreadLimit *= GC_STRESSLOG_MULTIPLY;
    }

    if ((uint32_t)numChunksInCurThread * STRESSLOG_CHUNK_SIZE >= perThreadLimit)
    {
        return FALSE;
    }

    return (uint32_t)VolatileLoad(&theLog.totalChunk) * STRESSLOG_CHUNK_SIZE < theLog.MaxSizeTotal;
}

bool StressLog::ReserveStressLogChunks (unsigned chunksToReserve)
{
    Thread *pThread = ThreadStore::GetCurrentThread();
    ThreadStressLog* msgs = reinterpret_cast<ThreadStressLog*>(pThread->GetThreadStressLog());
    if (msgs == 0)
    {
        msgs = CreateThreadStressLog(pThread);

        if (msgs == 0)
            return FALSE;
    }

    if (chunksToReserve == 0)
    {
        chunksToReserve = (theLog.MaxSizePerThread + STRESSLOG_CHUNK_SIZE - 1)  / STRESSLOG_CHUNK_SIZE;
    }

    long numTries = (long)chunksToReserve - msgs->chunkListLength;
    for (long i = 0; i < numTries; i++)
    {
        msgs->GrowChunkList ();
    }

    return msgs->chunkListLength >= (long)chunksToReserve;
}

/*********************************************************************************/
/* fetch a buffer that can be used to write a stress message, it is thread safe */

void ThreadStressLog::LogMsg ( uint32_t facility, int cArgs, const char* format, va_list Args)
{

    // Asserts in this function cause infinite loops in the asserting mechanism.
    // Just use debug breaks instead.

    ASSERT( cArgs >= 0 && (size_t)cArgs <= StressMsg::maxArgCnt );

    size_t offs = ((size_t)format - StressLog::theLog.moduleOffset);

    ASSERT(offs < StressMsg::maxOffset);
    if (offs >= StressMsg::maxOffset)
    {
        // Set it to this string instead.
        offs =
#ifdef _DEBUG
            (size_t)"<BUG: StressLog format string beyond maxOffset>";
#else // _DEBUG
            0; // a 0 offset is ignored by StressLog::Dump
#endif // _DEBUG else
    }

    // Get next available slot
    StressMsg* msg = AdvanceWrite(cArgs);

    msg->timeStamp = getTimeStamp();
    msg->facility = facility;
    msg->formatOffset = offs;
    msg->numberOfArgs = cArgs & 0x7;
    msg->numberOfArgsX = cArgs >> 3;

    for ( int i = 0; i < cArgs; ++i )
    {
        void* data = va_arg(Args, void*);
        msg->args[i] = data;
    }

    ASSERT(IsValid() && threadId == PalGetCurrentOSThreadId());
}


void ThreadStressLog::Activate (Thread * pThread)
{
    _ASSERTE(pThread != NULL);
    //there is no need to zero buffers because we could handle garbage contents
    threadId = PalGetCurrentOSThreadId();
    isDead = FALSE;
    curWriteChunk = chunkListTail;
    curPtr = (StressMsg *)curWriteChunk->EndPtr ();
    writeHasWrapped = FALSE;
    this->pThread = pThread;
}

/* static */
void StressLog::LogMsg (unsigned facility, int cArgs, const char* format, ... )
{
    _ASSERTE ( cArgs >= 0 && (size_t)cArgs <= StressMsg::maxArgCnt );

    va_list Args;
    va_start(Args, format);

    Thread *pThread = ThreadStore::RawGetCurrentThread();
    if (pThread == NULL)
        return;

    ThreadStressLog* msgs = reinterpret_cast<ThreadStressLog*>(pThread->GetThreadStressLog());

    if (msgs == 0) {
        msgs = CreateThreadStressLog(pThread);

        if (msgs == 0)
            return;
    }
    msgs->LogMsg (facility, cArgs, format, Args);
}

#ifdef _DEBUG

/* static */
void  StressLog::LogCallStack(const char *const callTag){

    size_t  CallStackTrace[MAX_CALL_STACK_TRACE];
    uint32_t hash;
    unsigned short stackTraceCount = PalCaptureStackBackTrace (2, MAX_CALL_STACK_TRACE, (void**)CallStackTrace, &hash);
    if (stackTraceCount > MAX_CALL_STACK_TRACE)
        stackTraceCount = MAX_CALL_STACK_TRACE;
    LogMsgOL("Start of %s stack \n", callTag);
    unsigned short i = 0;
    for (;i < stackTraceCount; i++)
    {
        LogMsgOL("(%s stack)%pK\n", callTag, CallStackTrace[i]);
    }
    LogMsgOL("End of %s stack\n", callTag);
}

#endif //_DEBUG

#else // DACCESS_COMPILE

bool StressLog::Initialize()
{
    ThreadStressLog* logs = 0;

    ThreadStressLog* curThreadStressLog = this->logs;
    unsigned __int64 lastTimeStamp = 0; // timestamp of last log entry
    while(curThreadStressLog != 0)
    {
        if (!curThreadStressLog->IsReadyForRead())
        {
            if (curThreadStressLog->origCurPtr == NULL)
                curThreadStressLog->origCurPtr = curThreadStressLog->curPtr;

            // avoid repeated calls into this function
            StressLogChunk * head = curThreadStressLog->chunkListHead;
            StressLogChunk * curChunk = head;
            bool curPtrInitialized = false;
            do
            {
                if (!curChunk->IsValid ())
                {
                    // TODO: Report corrupt chunk PTR_HOST_TO_TADDR(curChunk)
                }

                if (!curPtrInitialized && curChunk == curThreadStressLog->curWriteChunk)
                {
                    // adjust curPtr to the debugger's address space
                    curThreadStressLog->curPtr = (StressMsg *)((uint8_t *)curChunk + ((uint8_t *)curThreadStressLog->curPtr - (uint8_t *)PTR_HOST_TO_TADDR(curChunk)));
                    curPtrInitialized = true;
                }

                curChunk = curChunk->next;
            } while (curChunk != head);

            if (!curPtrInitialized)
            {
                delete curThreadStressLog;
                return false;
            }

            // adjust readPtr and curPtr if needed
            curThreadStressLog->Activate (NULL);
        }
        curThreadStressLog = curThreadStressLog->next;
    }
    return true;
}

void StressLog::ResetForRead()
{
    ThreadStressLog* curThreadStressLog = this->logs;
    while(curThreadStressLog != 0)
    {
        curThreadStressLog->readPtr = NULL;
        curThreadStressLog->curPtr = curThreadStressLog->origCurPtr;
        curThreadStressLog = curThreadStressLog->next;
    }
}

// Initialization of the ThreadStressLog when dumping the log
inline void ThreadStressLog::Activate (Thread * /*pThread*/)
{
    // avoid repeated calls into this function
    if (IsReadyForRead())
        return;

    curReadChunk = curWriteChunk;
    readPtr = curPtr;
    readHasWrapped = false;
    // the last written log, if it wrapped around may have partially overwritten
    // a previous record.  Update curPtr to reflect the last safe beginning of a record,
    // but curPtr shouldn't wrap around, otherwise it'll break our assumptions about stress
    // log
    curPtr = (StressMsg*)((char*)curPtr - StressMsg::maxMsgSize());
    if (curPtr < (StressMsg*)curWriteChunk->StartPtr())
    {
        curPtr = (StressMsg *)curWriteChunk->StartPtr();
    }
    // corner case: the log is empty
    if (readPtr == (StressMsg *)curReadChunk->EndPtr ())
    {
        AdvReadPastBoundary();
    }
}

ThreadStressLog* StressLog::FindLatestThreadLog() const
{
    const ThreadStressLog* latestLog = 0;
    for (const ThreadStressLog* ptr = this->logs; ptr != NULL; ptr = ptr->next)
    {
        if (ptr->readPtr != NULL)
            if (latestLog == 0 || ptr->readPtr->timeStamp > latestLog->readPtr->timeStamp)
                latestLog = ptr;
    }
    return const_cast<ThreadStressLog*>(latestLog);
}

// Can't refer to the types in sospriv.h because it drags in windows.h
void StressLog::EnumerateStressMsgs(/*STRESSMSGCALLBACK*/void* smcbWrapper, /*ENDTHREADLOGCALLBACK*/void* etcbWrapper, void *token)
{
    STRESSMSGCALLBACK smcb = (STRESSMSGCALLBACK)smcbWrapper;
    ENDTHREADLOGCALLBACK etcb = (ENDTHREADLOGCALLBACK) etcbWrapper;
    void *argsCopy[StressMsg::maxArgCnt];

    for (;;)
    {
        ThreadStressLog* latestLog = this->FindLatestThreadLog();

        if (latestLog == 0)
        {
            break;
        }
        StressMsg* latestMsg = latestLog->readPtr;
        if (latestMsg->formatOffset != 0 && !latestLog->CompletedDump())
        {
            char format[256];
            TADDR taFmt = (latestMsg->formatOffset) + (TADDR)(this->moduleOffset);
            HRESULT hr = DacReadAll(taFmt, format, _countof(format), false);
            if (hr != S_OK)
                strcpy_s(format, _countof(format), "Could not read address of format string");

            double deltaTime = ((double) (latestMsg->timeStamp - this->startTimeStamp)) / this->tickFrequency;

            // Pass a copy of the args to the callback to avoid foreign code overwriting the stress log
            // entries (this was the case for %s arguments)
            memcpy_s(argsCopy, sizeof(argsCopy), latestMsg->args, (latestMsg->numberOfArgs)*sizeof(void*));

            // @TODO: Truncating threadId to 32-bit
            if (!smcb((UINT32)latestLog->threadId, deltaTime, latestMsg->facility, format, argsCopy, token))
                break;
        }

        latestLog->readPtr = latestLog->AdvanceRead();
        if (latestLog->CompletedDump())
        {
            latestLog->readPtr = NULL;

            // @TODO: Truncating threadId to 32-bit
            if (!etcb((UINT32)latestLog->threadId, token))
                break;
        }
    }
}

typedef DPTR(SIZE_T) PTR_SIZE_T;

// Can't refer to the types in sospriv.h because it drags in windows.h
void StressLog::EnumStressLogMemRanges(/*STRESSLOGMEMRANGECALLBACK*/void* slmrcbWrapper, void *token)
{
    STRESSLOGMEMRANGECALLBACK slmrcb = (STRESSLOGMEMRANGECALLBACK)slmrcbWrapper;

    // we go to extreme lengths to ensure we don't read in the whole memory representation
    // of the stress log, but only the ranges...
    //

    size_t ThreadStressLogAddr = *dac_cast<PTR_SIZE_T>(PTR_HOST_MEMBER_TADDR(StressLog, this, logs));
    while (ThreadStressLogAddr != NULL)
    {
        size_t ChunkListHeadAddr = *dac_cast<PTR_SIZE_T>(ThreadStressLogAddr + offsetof(ThreadStressLog, chunkListHead));
        size_t StressLogChunkAddr = ChunkListHeadAddr;

        do
        {
            slmrcb(StressLogChunkAddr, sizeof (StressLogChunk), token);
            StressLogChunkAddr = *dac_cast<PTR_SIZE_T>(StressLogChunkAddr + offsetof (StressLogChunk, next));
            if (StressLogChunkAddr == NULL)
            {
                return;
            }
        } while (StressLogChunkAddr != ChunkListHeadAddr);

        ThreadStressLogAddr = *dac_cast<PTR_SIZE_T>(ThreadStressLogAddr + offsetof(ThreadStressLog, next));
    }
}


#endif // !DACCESS_COMPILE

#endif // STRESS_LOG

