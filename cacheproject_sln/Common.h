#pragma once
typedef unsigned char _u8;
typedef unsigned long long _u64;

/* ����Cache��־λ */
const unsigned char CACHE_FLAG_VALID = 0x01;    // ��Чλ�����Cache line�Ƿ񱣴�����Ч����
const unsigned char CACHE_FLAG_DIRTY = 0x02;    // ��λ�����Ҫ��д���ڴ��е�Cache line
const unsigned char CACHE_FLAG_MASK = 0xFF;     // ��д��Cache lineʱ���г�ʼ��flag�õ�
