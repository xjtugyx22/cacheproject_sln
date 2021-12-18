#include "Cache_Line.h"

void Cache_Line::mem_write_to_l2_safe(_u64 index, _u64 addr, _u64* cache_set_shifts, _u64* cache_line_shifts, _u64 tick_count, _u64* m_data)
{
        getWriteLock();
        tag = addr >> (cache_set_shifts[1] + cache_line_shifts[1]);
        flag = (_u8)~CACHE_FLAG_MASK;
        flag |= CACHE_FLAG_VALID;
        count = tick_count;
        buf = *m_data;
        this->addr = m_data;
        realseWriteLock();
}
void Cache_Line::l2_write_to_l1_safe(_u64 l1_index, _u64 l2_index, _u64 addr) {
    
}


void Cache_Line::getReadLock() {
    //std::shared_lock<std::shared_mutex> lock(mutex_);
    //lock.lock();
}
void Cache_Line::getWriteLock() {
    //std::unique_lock<std::shared_mutex> lock(mutex_);
    //lock.lock();
}
void Cache_Line::realseReadLock() {
    //std::shared_lock<std::shared_mutex> lock(mutex_);
    ///lock.unlock();
}
void Cache_Line::realseWriteLock() {
    //std::unique_lock<std::shared_mutex> lock(mutex_);
    //lock.unlock();
}


