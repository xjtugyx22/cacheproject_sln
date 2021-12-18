#pragma once
#include "Common.h"
#include <mutex>
#include <shared_mutex>
class Cache_Line {
public:
    _u64 tag;
    // 计数，FIFO里记录最一开始的访问时间，LRU里记录上一次访问的时间
    // count用来记录访问的时间，采用union数据结构，后面在更新count的时候
    // 只需要针对不同的替换算法，进行特殊赋值即可，而且不会导致一直使用count出现的思路混乱
    union {
        _u64 count;
        _u64 lru_count;
        _u64 fifo_count;
    };
    _u8 flag;           // 标记位存储，有效位、脏位
    _u64 buf;           // 存储在内存里的数据，即一个Cache行的数据，使用时分配空间
    _u64* addr;         // 存储在下一级存储器（即内存）中数据的地址，脏行时不能找到数据，干净行时说明内存中存在有Cache中的数据
                        // 需要在这块空间添加校验位，这块空间的大小取决于parity check和SEC-DED，没有添加到Cache类里去
    _u8* extra_cache;   // 校验位存储位置（Cache空间）
    _u8* extra_mem;     // 校验位存储位置（内存空间）
    // 假设每字32位，即4个字节
    // 一个Cache行如果32字节，那么一个Cache行就有8个子字块
    // 奇偶校验针对每个字，SEC-DED针对Cache行
    int state=0;//mesi state  0  I  1  E   2  S  3  M 
    void mem_write_to_l2_safe(_u64 index, _u64 addr, _u64* cache_set_shifts, _u64* cache_line_shifts, _u64 tick_count, _u64* m_data);
    void l2_write_to_l1_safe(_u64 l1_index, _u64 l2_index, _u64 addr);
    void getReadLock();
    void getWriteLock();
    void realseReadLock();
    void realseWriteLock();


private:
    mutable std::shared_mutex mutex_;
};
