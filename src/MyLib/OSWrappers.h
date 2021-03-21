#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>
class CSemaphore {
    SemaphoreHandle_t m_semId;
public:
    static int s_semaphoreNum;
public:
    CSemaphore();
    ~CSemaphore();
    bool create(uint32_t nInitialValue = 1, uint32_t nMaxCount = 1);
    void destroy();
    int getCount();
    bool waitSemaphore();
    bool waitSemaphoreMs(uint32_t nTime);
    void postSemaphore();
    void postSemaphoreIsr();
};

class CMutex {
    SemaphoreHandle_t m_mutexId;
public:
    static int s_mutexNum;
public:
    CMutex();
    virtual ~CMutex();
    virtual bool create();

    void destroy();
    void lock();
    void unlock();
};

class CCriticalSection : public CMutex
{
public:
    CCriticalSection(){CMutex::create();}
    virtual ~CCriticalSection(){}
protected:
    virtual bool Create(){return false;}
};


class CScopedCritSec
{
    CMutex *m_pCritSec;
private:
    CScopedCritSec(const CScopedCritSec& obj);
    CScopedCritSec& operator=(CScopedCritSec& obj);
public:
    CScopedCritSec(CMutex &critSec){m_pCritSec = &critSec; m_pCritSec->lock();}
    ~CScopedCritSec(){m_pCritSec->unlock();}
};

/*
class CScopedUnsheduledRegion   // simple clear interrupt
{
public:
    CScopedUnsheduledRegion(){__disable_irq();}
    ~CScopedUnsheduledRegion(){__enable_irq();}
};
*/

void taskDelayMs(uint32_t delayMs);


