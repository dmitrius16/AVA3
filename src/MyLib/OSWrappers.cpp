#include <limits>
#include "OSWrappers.h"
#include "freertos/task.h"
#include "modulInfo.h"

void taskDelayMs(uint32_t delayMs) {
    vTaskDelay(pdMS_TO_TICKS(delayMs));
}

// mutex wrapper
int CMutex::s_mutexNum = 0;

CMutex::CMutex() {  //need to implement
    m_mutexId = (SemaphoreHandle_t)std::numeric_limits<uint32_t>::max();
}

CMutex::~CMutex() { //need to implement
    destroy();
}

bool CMutex::create() {
    if (s_mutexNum < MAX_COUNT_MUTEX) {
        m_mutexId = xSemaphoreCreateMutex();
        if (m_mutexId != NULL) {
            s_mutexNum++;
            return true;
        }
    }
    return false;
}

void CMutex::destroy() {
    vSemaphoreDelete(m_mutexId);
}

void CMutex::lock() {
    xSemaphoreTake(m_mutexId, portMAX_DELAY);
}

void CMutex::unlock() {
    xSemaphoreGive(m_mutexId);
}

// end mutex wrapper
