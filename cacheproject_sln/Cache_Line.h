#pragma once
#include "Common.h"
#include <mutex>
#include <shared_mutex>
class Cache_Line {
public:
    _u64 tag;
    // ������FIFO���¼��һ��ʼ�ķ���ʱ�䣬LRU���¼��һ�η��ʵ�ʱ��
    // count������¼���ʵ�ʱ�䣬����union���ݽṹ�������ڸ���count��ʱ��
    // ֻ��Ҫ��Բ�ͬ���滻�㷨���������⸳ֵ���ɣ����Ҳ��ᵼ��һֱʹ��count���ֵ�˼·����
    union {
        _u64 count;
        _u64 lru_count;
        _u64 fifo_count;
    };
    _u8 flag;           // ���λ�洢����Чλ����λ
    _u64 buf;           // �洢���ڴ�������ݣ���һ��Cache�е����ݣ�ʹ��ʱ����ռ�
    _u64* addr;         // �洢����һ���洢�������ڴ棩�����ݵĵ�ַ������ʱ�����ҵ����ݣ��ɾ���ʱ˵���ڴ��д�����Cache�е�����
                        // ��Ҫ�����ռ����У��λ�����ռ�Ĵ�Сȡ����parity check��SEC-DED��û����ӵ�Cache����ȥ
    _u8* extra_cache;   // У��λ�洢λ�ã�Cache�ռ䣩
    _u8* extra_mem;     // У��λ�洢λ�ã��ڴ�ռ䣩
    // ����ÿ��32λ����4���ֽ�
    // һ��Cache�����32�ֽڣ���ôһ��Cache�о���8�����ֿ�
    // ��żУ�����ÿ���֣�SEC-DED���Cache��
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
