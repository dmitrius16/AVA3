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





