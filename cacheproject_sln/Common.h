#pragma once
typedef unsigned char _u8;
typedef unsigned long long _u64;

/* 定义Cache标志位 */
const unsigned char CACHE_FLAG_VALID = 0x01;    // 有效位，标记Cache line是否保存着有效数据
const unsigned char CACHE_FLAG_DIRTY = 0x02;    // 脏位，标记要回写到内存中的Cache line
const unsigned char CACHE_FLAG_MASK = 0xFF;     // 在写入Cache line时进行初始化flag用到
