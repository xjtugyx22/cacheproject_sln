#include "CacheSimulator.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <cstdio>
#include <time.h>
#include <climits>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

#include <Windows.h>
using namespace std;


CacheSim::CacheSim() {}

/*
 * @arg a_cache_size[] �༶Cache�Ĵ�С����
 * @arg a_cache_line_size[] �༶Cache��line size��С
 * @arg a_mapping_ways[] �����������ӷ�ʽ
*/
void CacheSim::init(_u64 a_cache_size[3], _u64 a_cache_line_size[3], _u64 a_mapping_ways[3], int l1_replace,
    int l2_replace, int a_parity, int a_secded, int a_mlreps, int a_early, int a_emergency,
    int a_error_type, int a_inject_times, Ui::MainWindow* ui) {
    // ����������ò�����Ҫ������Ϊ2��Cache
    if (a_cache_line_size[0] < 0 || a_cache_line_size[1] < 0
        || a_mapping_ways[0] < 1 || a_mapping_ways[1] < 1) {
        ui->textBrowser->insertPlainText("�������ò�����Ҫ��\n");
        return;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // line_size[] = {32, 32}, ways[] = {4, 4}, cache_size[] = {0x1000, 0x8000}
    // cache_size[0] = 0x1000 = 4kb, cache_size[1] = 0x8000 = 32kb
    // cache_line_size[0] = 32byte, cache_line_size[1] = 32byte
    // cache_line_num[0] = 128line, cache_line_num[1] = 1024line
    // cache_line_shifts[0] = 5, cache_line_shifts[1] = 5
    // cache_mapping_ways[0] = 4, cache_mapping_ways[1] = 4
    // cache_set_size[0] = 32, cache_set_size[1] = 256   �ж��ٸ��飨set��
    // cache_set_shifts[0] = 5, cache_set_shifts[1] = 8
    // cache_free_num[0] = 128, cache_free_num[1] = 128
    //////////////////////////////////////////////////////////////////////////////////

    PARITY_CHECK = a_parity;
    SECDED = a_secded;
    MLREPS = a_mlreps;
    EARLY_WRITE_BACK = a_early;
    EMERGENCY_WRITE_BACK = a_emergency;
    ERROR_TYPE = a_error_type;
    INJECT_TIMES = a_inject_times;

    //core0 l1 cache size
    cache_size[0] = a_cache_size[0] * 1024;
    //l2 cache size
    cache_size[1] = a_cache_size[1] * 1024;
    //core1 l1 cache size
    cache_size[2] = a_cache_size[0] * 1024;

    //core0 l1 cache
    cache_line_size[0] = a_cache_line_size[0];
    //l2 cache
    cache_line_size[1] = a_cache_line_size[1];
    //core1 l1 cache
    cache_line_size[2] = a_cache_line_size[0];


    // �ܵ�line�� = Cache�ܴ�С/ÿ��line�Ĵ�С��һ��64byte��ģ���ʱ������ã�
    //core0 l1 cache
    cache_line_num[0] = (_u64)cache_size[0] / a_cache_line_size[0];
    //l2 cache
    cache_line_num[1] = (_u64)cache_size[1] / a_cache_line_size[1];
    //core1 l1 cache
    cache_line_num[2] = (_u64)cache_size[0] / a_cache_line_size[0];

    //core0 l1 cache
    cache_line_shifts[0] = (_u64)log2(a_cache_line_size[0]);
    //l2 cache
    cache_line_shifts[1] = (_u64)log2(a_cache_line_size[1]);
    //core1 l1 cache
    cache_line_shifts[2] = (_u64)log2(a_cache_line_size[0]);

    // ��ʼ��һ���е�ÿ���ִ�С���ֽڣ�
    cache_word_size[0] = 4;
    cache_word_size[1] = 4;
    cache_word_size[2] = 4;


    cache_word_num[0] = cache_line_size[0] / cache_word_size[0];
    cache_word_num[1] = cache_line_size[1] / cache_word_size[1];
    cache_word_num[2] = cache_line_size[0] / cache_word_size[0];

    // ��·������
    //core0 l1 cache
    cache_mapping_ways[0] = a_mapping_ways[0];
    //l2 cache
    cache_mapping_ways[1] = a_mapping_ways[1];
    //core1 l1 cache
    cache_mapping_ways[2] = a_mapping_ways[0];

    // �ܹ��ж���set
    cache_set_size[0] = cache_line_num[0] / cache_mapping_ways[0];
    cache_set_size[1] = cache_line_num[1] / cache_mapping_ways[1];
    cache_set_size[2] = cache_line_num[2] / cache_mapping_ways[2];


    // �������ռ��λ����ͬ����shifts
    cache_set_shifts[0] = (_u64)log2(cache_set_size[0]);
    cache_set_shifts[1] = (_u64)log2(cache_set_size[1]);
    cache_set_shifts[2] = (_u64)log2(cache_set_size[0]);

    // ���п飨line������ʼ���еĿ鶼Ϊ��
    cache_free_num[0] = cache_line_num[0];
    cache_free_num[1] = cache_line_num[1];
    cache_free_num[2] = cache_line_num[0];

    cache_r_count[0] = 0;
    cache_r_count[1] = 0;
    cache_r_count[2] = 0;

    cache_w_count[0] = 0;
    cache_w_count[1] = 0;
    cache_w_count[2] = 0;

    cache_w_memory_count = 0;
    cache_r_memory_count = 0;

    clean_count = 0;
    dirty_count = 0;

    period = 0;
    time_period = 0;
    emergency_period = 0;

    dirty_interval_time = 0;

    // ��ʼ��MLREPS����Ϊ�ر�״̬
    // ���������г��ִ��󣬴�ʱ������д���ƴ��������б�Ϊ�ɾ��У�����������е�MLREPS����
    // MLREPS���������ڣ�����һ�����ڻ�дʱ��MLREPS�ı�־λ��Ϊfalse�����»���SEC-DED����
    START_MLREPS_FLAG = false;

    // ֻ�ڴ�дL2�����ֵ�Ϳ��ԣ��������þֲ�����û����
    m_data = 0xFFFF;    // ��ʾÿ��д��Cache��ֵ

    clean_line_error_count = 0;
    dirty_line_error_count = 0;
    dirty_line_error_correct_count = 0;

    inject_count = 0;
    fail_correct = 0;
    error_but_not_detect_for_dirty = 0;
    error_but_not_detect_for_clean = 0;

    // ָ��������Ҫ�������滻���Ե�ʱ���ṩ�Ƚϵ�key�������л���miss��ʱ����Ӧline������Լ���countΪ��ʱ��tick_count
    tick_count = 0;

    // Ϊÿһ�з���ռ�
    int check_bits = 0;
    for (int i = 0; i < 3; ++i) {
        caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
        memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
        cache_buf[i] = (_u64*)malloc(cache_size[i]);
        memset(cache_buf[i], 0, cache_size[i]);
        //???
        if (i == 1) {
            // Cache�ִ�С=Cache�д�С/��·������
            // parity��һ��Cache������У��λ
            // SEC-DED��Ҫ������Ϣ���λ��ȷ����������Cache�д�С��λ��ȷ��
            // int small_block_num = cache_mapping_ways[1];
            int word_size_bits = (cache_line_size[1] / cache_mapping_ways[1]) * 8;
            // word_size_bits��ò�Ҫ����16���ֽڣ�Ҫ�ǳ���16���ֽڻ������
            if (word_size_bits >= 5 && word_size_bits <= 11) check_bits = 4;
            else if (word_size_bits >= 12 && word_size_bits <= 26) check_bits = 5;
            else if (word_size_bits >= 27 && word_size_bits <= 57) check_bits = 6;
            else if (word_size_bits >= 58 && word_size_bits <= 120) check_bits = 7;
            else check_bits = 8;
            check_bits = check_bits * cache_mapping_ways[1];
        }
    }

    //????
    // Ϊÿһ��Cache�з���У��λ�ռ�
    for (_u64 i = 0; i < cache_line_num[1]; ++i) {
        caches[1][i].extra_cache = (_u8*)malloc(sizeof(_u8) * check_bits);
        memset(caches[1][i].extra_cache, 0, sizeof(_u8) * check_bits);
        caches[1][i].extra_mem = (_u8*)malloc(sizeof(_u8) * 40);
        memset(caches[1][i].extra_mem, 0, sizeof(_u8) * 40);
    }


    // Replacement strategy
    if (l1_replace == 0) {
        swap_style[0] = CACHE_SWAP_LRU;
        swap_style[2] = CACHE_SWAP_LRU;
    }
    else if (l1_replace == 1) {
        swap_style[0] = CACHE_SWAP_FIFO;
        swap_style[2] = CACHE_SWAP_FIFO;
    }
    else if (l1_replace == 2) {
        swap_style[0] = CACHE_SWAP_RAND;
        swap_style[2] = CACHE_SWAP_RAND;
    }

    if (l2_replace == 0) {
        swap_style[1] = CACHE_SWAP_LRU;
    }
    else if (l2_replace == 1) {
        swap_style[1] = CACHE_SWAP_FIFO;
    }
    else if (l2_replace == 2) {
        swap_style[1] = CACHE_SWAP_RAND;
    }

    re_init(check_bits);
    srand((unsigned)time(NULL));    // CACHE_SWAP_RAND�滻���
}

/*
 * �����ĳ�ʼ��������һ��ʼ�������;��Ҫ��tick_count���������caches����գ�ִ�д˺���
 * ��Ҫ��Ϊtick_count���������ܻᳬ��unsigned long long
 * ����һ��tick_count���㣬Caches���count����Ҳ�ͳ����˴���
*/
void CacheSim::re_init(int bits) {
    tick_count = 0;
    for (_u64 i = 0; i < cache_line_num[1]; ++i) {
        memset(caches[1][i].extra_cache, 0, sizeof(_u8) * bits);
        memset(caches[1][i].extra_mem, 0, sizeof(_u8) * 40);
    }
    memset(cache_hit_count, 0, sizeof(cache_hit_count));
    memset(cache_miss_count, 0, sizeof(cache_miss_count));
    cache_free_num[0] = cache_line_num[0];
    cache_free_num[1] = cache_line_num[1];
    cache_free_num[2] = cache_line_num[2];
    // init�����Ѿ�ʹ��malloc������caches�����˿ռ�
    memset(caches[0], 0, sizeof(Cache_Line) * cache_line_num[0]);
    memset(caches[1], 0, sizeof(Cache_Line) * cache_line_num[1]);
    memset(caches[2], 0, sizeof(Cache_Line) * cache_line_num[2]);
    memset(cache_buf[0], 0, cache_size[0]);
    memset(cache_buf[1], 0, cache_size[1]);
    memset(cache_buf[2], 0, cache_size[2]);
}

/* ����������ɡ������ƺ󡱹��� */
//CacheSim::~CacheSim() {
//    for (_u64 i = 0; i < cache_line_num[1]; ++i) {
//        free(caches[1][i].extra_cache);
//    }
//    free(cache_buf[0]);
//    free(cache_buf[1]);
//    free(caches[0]);
//    free(caches[1]);
//}

/* ���ļ���ȡtrace���Խ��ͳ�� */
int fuck = 0;
bool isModifyingUI = false;
void CacheSim::load_trace(const char* filename, const int core_num, Ui::MainWindow* ui) {
    int threadId = ++fuck;
    char buf[128] = { 0 };
    // ����Լ���input·��
    FILE* fin;
    // ��¼����trace��ָ��Ķ�д������Cache���ƣ��������Ķ�д������Ȼ��һ��
    // ��Ҫ��������õ�д�ط�����д�����Cache�У�ֱ�����滻
    _u64 rcount = 0, wcount = 0, sum = 0;
    fin = fopen(filename, "r");
    if (!fin) {
        ui->textBrowser->insertPlainText("load file failed��\n");
        return;
    }
    
    //  ����ע���ʼ����ȷ�����ϵ�λ��
    int n = INJECT_TIMES, min = 1, max = 300000;
    int* error_struct = (int*)malloc(sizeof(int) * 20 * n);
    memset(error_struct, 0, sizeof(int) * 20 * n);
    if (ERROR_TYPE > 0) {
        error_struct = cache_error_inject(min, max, n, ERROR_TYPE);
    }

    int k = 0;
   
    // trace�ļ����ж���
    while (fgets(buf, sizeof(buf), fin)) {
        _u8 style = 0;
        _u64 addr = 0;
        sscanf(buf, "%c %llx", &style, &addr);

        // ��¼�ܵĶ�д����
        sum++;

        // ÿ�ε������ʱ�䣬���ù���ע��
        if (ERROR_TYPE > 0) {
            // ����ע��ʱ��
            if ((int)sum == error_struct[k]) {
                k++;
                if (error_struct[k] == 1) {
                    k++;
                    if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                        inject_count++;
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                    }
                    k += 2;
                }
                if (error_struct[k] == 2) {
                    k++;
                    if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                        inject_count++;
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                    }
                    k += 3;
                }
                if (error_struct[k] == 3) {
                    k++;
                    if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                        inject_count++;
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                    }
                    k += 4;
                }
                if (error_struct[k] == 4) {
                    k++;
                    if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                        inject_count++;
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 4]);
                    }
                    k += 5;
                }
                if (error_struct[k] == 5) {
                    k++;
                    if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                        inject_count++;
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 4]);
                        reversebit(caches[1][error_struct[k]].buf, error_struct[k + 5]);
                    }
                    k += 6;
                }
                if (error_struct[k] == 12) {
                    k++;
                    if (error_struct[k] == 1) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        }
                        k += 2;
                    }
                    if (error_struct[k] == 2) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        }
                        k += 3;
                    }
                }
                if (error_struct[k] == 13) {
                    k++;
                    if (error_struct[k] == 1) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        }
                        k += 2;
                    }
                    if (error_struct[k] == 2) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        }
                        k += 3;
                    }
                    if (error_struct[k] == 3) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                        }
                        k += 4;
                    }
                }
                if (error_struct[k] == 14) {
                    k++;
                    if (error_struct[k] == 1) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        }
                        k += 2;
                    }
                    if (error_struct[k] == 2) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        }
                        k += 3;
                    }
                    if (error_struct[k] == 3) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                        }
                        k += 4;
                    }
                    if (error_struct[k] == 4) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 4]);
                        }
                        k += 5;
                    }
                }
                if (error_struct[k] == 15) {
                    k++;
                    if (error_struct[k] == 1) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                        }
                        k += 2;
                    }
                    if (error_struct[k] == 2) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                        }
                        k += 3;
                    }
                    if (error_struct[k] == 3) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                        }
                        k += 4;
                    }
                    if (error_struct[k] == 4) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 4]);
                        }
                        k += 5;
                    }
                    if (error_struct[k] == 5) {
                        k++;
                        if (caches[1][error_struct[k]].flag & CACHE_FLAG_VALID) {
                            inject_count++;
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 1]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 2]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 3]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 4]);
                            reversebit(caches[1][error_struct[k]].buf, error_struct[k + 5]);
                        }
                        k += 6;
                    }
                }
            }
        }
        // �Զ����һ�в���

        do_cache_op(addr, style, core_num,ui);

        switch (style) {
        case 'l':
            rcount++;
            break;
        case 's':
            wcount++;
            break;
        }
    }

    while (isModifyingUI);
    isModifyingUI = true;
    string text;
    text = "all r/w/sum: " + to_string(rcount) + "/" + to_string(wcount) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "read rate: " + to_string(100.0 * rcount / tick_count) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "write rate: " + to_string(100.0 * wcount / tick_count) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 miss/hit: " + to_string(cache_miss_count[0]) + "/" + to_string(cache_hit_count[0]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 miss/hit: " + to_string(cache_miss_count[2]) + "/" + to_string(cache_hit_count[2]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 hit rate: " + to_string(100.0 * cache_hit_count[0] / (cache_hit_count[0] + cache_miss_count[0])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 hit rate: " + to_string(100.0 * cache_hit_count[2] / (cache_hit_count[2] + cache_miss_count[2])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 miss rate: " + to_string(100.0 * cache_miss_count[0] / (cache_miss_count[0] + cache_hit_count[0])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 miss rate: " + to_string(100.0 * cache_miss_count[2] / (cache_miss_count[2] + cache_hit_count[2])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 miss/hit: " + to_string(cache_miss_count[1]) + "/" + to_string(cache_hit_count[1]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 hit rate: " + to_string(100.0 * cache_hit_count[1] / (cache_hit_count[1] + cache_miss_count[1])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 miss rate: " + to_string(100.0 * cache_miss_count[1] / (cache_miss_count[1] + cache_hit_count[1])) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "clean line num: " + to_string(clean_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "dirty line num: " + to_string(dirty_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "clean rate: " + to_string(100.0 * clean_count / (clean_count + dirty_count)) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "dirty rate: " + to_string(100.0 * dirty_count / (clean_count + dirty_count)) + "%" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    // ��дͨ�ţ�����Cache�Ļ�Ӧ���Ƕ�дL1/L2�Ĵ���
    text = "core0 L1 read num: " + to_string(cache_r_count[0]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 read num: " + to_string(cache_r_count[2]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 write num: " + to_string(cache_w_count[0]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 write num: " + to_string(cache_w_count[2]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 read num: " + to_string(cache_r_count[1]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 write num: " + to_string(cache_w_count[1]) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 read communication: " + to_string(((cache_r_count[0] * cache_line_size[0]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 read communication: " + to_string(((cache_r_count[2] * cache_line_size[2]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core0 L1 write communication: " + to_string(((cache_w_count[0] * cache_line_size[0]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "core1 L1 write communication: " + to_string(((cache_w_count[2] * cache_line_size[2]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 read communication: " + to_string(((cache_r_count[1] * cache_line_size[1]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "L2 write communication: " + to_string(((cache_w_count[1] * cache_line_size[1]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "Memory read num: " + to_string(cache_r_memory_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "Memory write num: " + to_string(cache_w_memory_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "Memory read communication: " + to_string(((cache_r_memory_count * cache_line_size[1]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "Memory write communication: " + to_string(((cache_w_memory_count * cache_line_size[1]) >> 10)) + "KB" + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "clean line error num: " + to_string(clean_line_error_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "dirty line error num: " + to_string(dirty_line_error_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "dirty line error correct num: " + to_string(dirty_line_error_correct_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "inject num: " + to_string(inject_count) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "fail correct num: " + to_string(fail_correct) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "error but not detect for clean: " + to_string(error_but_not_detect_for_clean) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    text = "error but not detect for dirty: " + to_string(error_but_not_detect_for_dirty) + "\n";
    ui->textBrowser->insertPlainText(QString::fromStdString(text));
    ui->textBrowser->insertPlainText("================================\n");
    ui->textBrowser->insertPlainText("\n");
    isModifyingUI = false;
    switch (threadId)
    {
    case 1:
        OutputDebugString(L"************dddddfuck***1111111111***********\n");
        break;
    case 2:
        OutputDebugString(L"************dddddfuck***222222222***********\n");
        break;
    default:
        OutputDebugString(L"************dddddfuck**************\n");
        break;
    }
    

    fclose(fin);
}





/*
 * @arg set_base[] set��ַ
 * @arg addr[] trace�ļ����ڴ��ַ
 * @arg level[] ��һ��Cache 0 core0 l1cache 1 l2 cache 2 core1 l1cache
 * ע��ѭ�����ҵ�ǰset������way��line����ͨ��tagƥ�䣬�鿴��ǰ��ַ�Ƿ���Cache��
*/
int CacheSim::check_cache_hit(_u64 set_base, _u64 addr, int level, Ui::MainWindow* ui) {
    _u64 i;
    for (i = 0; i < cache_mapping_ways[level]; ++i) {
        caches[level][set_base+i].getReadLock();
        if ((caches[level][set_base + i].flag & CACHE_FLAG_VALID) &&
            (caches[level][set_base + i].tag == ((addr >> (cache_set_shifts[level] + cache_line_shifts[level]))))) {
            // tag����Ƿ�����ǰ���Լ�У��
            caches[level][set_base + i].realseReadLock();
            return set_base + i;
        }
        caches[level][set_base + i].realseReadLock();
    }
    // -1��ʾ���addr�е����ݲ�û�м��ص�cache��
    return -1;
}

/*
 * @arg set_base[] set��ַ
 * @arg level[] ��һ��Cache Cache 0 core0 l1cache 1 l2 cache 2 core1 l1cache
 * ע����ȡ��ǰset�п��õ�line�����û�У����ҵ�Ҫ���滻�Ŀ�
*/
int CacheSim::get_cache_free_line(_u64 set_base, int level, int style, Ui::MainWindow* ui) {
    _u64 i, min_count, j;
    int free_index;
    // �ӵ�ǰcache set���ҿ��õĿ���line�����ã�������/�������ݣ���Ч���ݣ�
    // cache_free_num��ͳ�Ƶ�����cache�Ŀ��ÿ�
    for (i = 0; i < cache_mapping_ways[level]; ++i) {
        caches[level][set_base + i].getReadLock();
        if (!(caches[level][set_base + i].flag & CACHE_FLAG_VALID)) {
            // ��Ȼ���ߵ������˵��cache������Ч�ģ�����ʼ��Cache lineδռ�������Կ϶��п���λ��
            // ��ʱ������ڴ�д���ݵ�l2����ɾ��е���������
            if (cache_free_num[level] > 0) {
                cache_free_num[level]--;
                caches[level][set_base + i].realseReadLock();
                return set_base + i;
            }
        }
        caches[level][set_base + i].realseReadLock();
    }
    // û�п���line����ִ���滻�㷨
    free_index = -1;
    if (swap_style[level] == CACHE_SWAP_RAND) {
        free_index = rand() % cache_mapping_ways[level];
    }
    else {
        // FIFO����LRU�滻�㷨
        min_count = ULLONG_MAX;
        for (j = 0; j < cache_mapping_ways[level]; ++j) {
            if (caches[level][set_base + j].count < min_count) {
                min_count = caches[level][set_base + j].count;
                free_index = j;
            }
        }
    }
    if (free_index >= 0) {
        free_index += set_base;
        // ���ԭ��Cache line�������ݣ�д���ڴ�,ֻ��L2������������
        // l1��δ����l2��δ���У��ڴ���l2д���ݣ�����l2Ϊ���У���Ҫ�����滻����ʱ�µ�l2Ϊ�ɾ���
        // l2дδ���У�cpu/l1��l2д���ݣ�����l2Ϊ���У���Ҫ�����滻����ʱ�µ�l2Ϊ����
        if (caches[level][free_index].flag & CACHE_FLAG_DIRTY) {
            cache_r_count[1]++;
            cache_w_memory_count++;
            period = period + 10 + 100;
            // ��ӣ�����������
            dirty_interval_time = dirty_interval_time + 10 + 100;
            l2_write_to_mem((_u64)free_index,ui);
        }
        else {
            // ԭ��l2Ϊ�ɾ��У�ֱ�ӽ��ɾ����滻��Ȼ��Ѹ��б�Ϊ����
        }
        return free_index;
    }
    else {
        cout << "error!" << endl;
        return -1;
    }
}

/* �ҵ����ʵ�line֮�󣬽�����д��L1 Cache line�� */
void CacheSim::cpu_mem_write_to_l1(_u64 index, _u64 addr,Ui::MainWindow* ui) {
    Cache_Line* line = caches[0] + index;
    line->getWriteLock();
    line->buf = 0xFFFF;
    line->tag = addr >> (cache_set_shifts[0] + cache_line_shifts[0]);
    line->flag = (_u8)~CACHE_FLAG_MASK;
    line->flag |= CACHE_FLAG_VALID;
    line->count = tick_count;
    line->realseWriteLock();
}

/* �ҵ����ʵ�line֮�󣬽�����д��L1 Cache line�� */
void CacheSim::l2_write_to_l1(_u64 l1_index, _u64 l2_index, _u64 addr, Ui::MainWindow* ui) {
    Cache_Line* line = caches[1] + l2_index;
    line->getReadLock();
    // ��Ҫ�ȶ�l2���룬Ȼ�����������Ĳ���
    _u64 data = caches[1][l2_index].buf;
    if (caches[1][l2_index].flag & CACHE_FLAG_DIRTY) {
        // �����н���
        if (!START_MLREPS_FLAG) {
            if (SECDED == 1) hamming_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
            if (SECDED == 2) secded_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        }
        else {
            if (MLREPS == 1) mlreps_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        }
    }
    if (!(caches[1][l2_index].flag & CACHE_FLAG_DIRTY)) {
        // �Ըɾ��н���
        if (PARITY_CHECK == 1 || PARITY_CHECK == 2) {
            parity_check_decode_for_cache(l2_index, data,ui);
        }
    }
    line = caches[0] + l1_index;
    line->getWriteLock();
    line->buf = caches[1][l2_index].buf;
    line->tag = addr >> (cache_set_shifts[0] + cache_line_shifts[0]);
    line->flag = (_u8)~CACHE_FLAG_MASK;
    line->flag |= CACHE_FLAG_VALID;
    line->count = tick_count;
    line->realseWriteLock();
    line->realseReadLock();
}

/* l2д���ݵ�cpu */
void CacheSim::l2_write_to_cpu(_u64 l2_index, Ui::MainWindow* ui) {
    Cache_Line* line = caches[1] + l2_index;
    line->getReadLock();
    _u64 data = caches[1][l2_index].buf;
    if (caches[1][l2_index].flag & CACHE_FLAG_DIRTY) {
        // �����н���
        if (!START_MLREPS_FLAG) {
            if (SECDED == 1) hamming_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
            if (SECDED == 2) secded_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        }
        else {
            if (MLREPS == 1) mlreps_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        }
    }
    if (!(caches[1][l2_index].flag & CACHE_FLAG_DIRTY)) {
        // �Ըɾ��н���
        if (PARITY_CHECK == 1 || PARITY_CHECK == 2) {
            parity_check_decode_for_cache(l2_index, data,ui);
            // parity_check_decode_for_cache(l2_index, 62463);
        }
    }
    line->realseReadLock();
}

/* l2д���ݵ�mem����Ҫ�������Ƚ��� */
void CacheSim::l2_write_to_mem(_u64 l2_index, Ui::MainWindow* ui) {
    Cache_Line* line = caches[1] + l2_index;
    line->getReadLock();
    if (!START_MLREPS_FLAG) {
        if (SECDED == 1) hamming_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        if (SECDED == 2) secded_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        // if (SECDED == 1) secded_decode_for_cache(l2_index, 64511);
        // if (SECDED == 2) secded_decode_for_cache(l2_index, 64511);
    }
    else {
        if (MLREPS == 1) mlreps_decode_for_cache(l2_index, caches[1][l2_index].buf,ui);
        // if (MLREPS == 1) mlreps_decode_for_cache(l2_index, 63231);
    }
    // test 65503 = 1111111111011111    62463 = 1111001111111111
    _u64 data = caches[1][l2_index].buf;
    line->realseReadLock();
}

/* �ҵ����ʵ�line֮�󣬽�����д��L2 Cache line�� */
void CacheSim::l1_cpu_write_to_l2(_u64 index, _u64 addr, Ui::MainWindow* ui) {
    Cache_Line* line = caches[1] + index;
    line->getWriteLock();
    _u64 data = 0xFFFF;
    line->buf = data;
    line->tag = addr >> (cache_set_shifts[1] + cache_line_shifts[1]);
    line->flag = (_u8)~CACHE_FLAG_MASK;
    line->flag |= CACHE_FLAG_VALID;
    line->count = tick_count;
    // l1/cpuд���ݵ�l2��l2ֻ�����SEC-DED��У��λ����Ϊ��ʱl2�����
    if (!START_MLREPS_FLAG) {
        if (SECDED == 1) hamming_encode_for_cache(index, data);
        if (SECDED == 2) secded_encode_for_cache(index, data);
    }
    else {
        if (MLREPS == 1) mlreps_encode_for_cache(index, data);
    }
    line->realseWriteLock();
}

/*
 * ʵ����ͨ��żУ��
 * ��У�飺�����������м��顰1���ĸ����Ƿ�Ϊ��������Ϊ��������У��λΪ0����Ϊż������У��λΪ1
 * żУ�飺�����������м��顰1���ĸ����Ƿ�Ϊż������Ϊż������У��λΪ0����Ϊ�����������λΪ1
 * ���ϸ����ÿ����żУ�飬����һ�����������ΪһλУ��λ
 * ����ֵΪ1��ʾ���ɵ�У��λΪ1������ֵΪ1��ʾ���ɵ�У��λΪ0
*/
int CacheSim::parity_check(_u64 m) {
    bool parity = false;
    while (m) {
        parity = !parity;
        m = m & (m - 1);
    }
    return parity == true ? 1 : 0;
}

/*
 * ʵ�ּ��Ϊ1��żУ��
 * ����ֵΪָ�������ָ��
 * ��һ��Ԫ��Ϊ����λ����ӵ�У��λ���ڶ���Ԫ��Ϊż��λ����ӵ�У��λ
*/
int* CacheSim::update_parity_check(_u64 m) {
    int* parity = (int*)malloc(sizeof(int) * 2);
    int odd1 = 0, odd2 = 0;
    int i = 64;     // ��������64λ
    while (i > 0) {
        i = i - 2;
        int temp1 = 0, temp2 = 0;
        temp1 = m >> i;
        temp2 = m >> i;
        if ((temp1 & 1) == 1) odd1++;
        if ((temp2 & 2) == 2) odd2++;
    }
    parity[0] = (odd1 % 2 == 1) ? 1 : 0;
    parity[1] = (odd2 % 2 == 1) ? 1 : 0;
    return parity;
}

/* parity check decode for Cache */
void CacheSim::parity_check_decode_for_cache(_u64 l2_index, _u64 data, Ui::MainWindow* ui) {
    if (PARITY_CHECK == 1) {
        for (_u64 i = 0; i < cache_word_num[1]; ++i) {
            _u64 temp = data & (_u64)(pow(2, cache_word_size[1] * 8) - 1);
            if (caches[1][l2_index].extra_cache[i] != (_u8)parity_check(temp)) {
                clean_line_error_count++;
                ui->textBrowser->insertPlainText("��żУ���⵽����\n");
                // ֮����Ҫ����һ���洢���ָ�
                caches[1][l2_index].buf = *((_u64*)(void*)(caches[1][l2_index].addr));
                ui->textBrowser->insertPlainText("�Ѿ�����һ���洢���ָ���\n");
                ui->textBrowser->insertPlainText("��ȷ����Ϊ��");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][l2_index].buf)));
                ui->textBrowser->insertPlainText("\n");
                // ��⵽����ͻָ�����������
                return;
            }
            data = data >> cache_word_size[1] * 8;
        }
        if (caches[1][l2_index].buf != 0xFFFF) {
            ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][l2_index].buf)));
            ui->textBrowser->insertPlainText("\n");
            error_but_not_detect_for_clean++;
            ui->textBrowser->insertPlainText("parityδ��⵽����\n");
            // Ϊ��֮�����ע�����ȷ�������޸�Ϊ��ȷ����
            // Ҳ��Ҫһ�����±���
            parity_check_encode_for_cache(l2_index, 0xFFFF);
            caches[1][l2_index].buf = 0xFFFF;
        }
    }
    if (PARITY_CHECK == 2) {
        for (_u64 i = 0; i < 2 * cache_word_num[1]; i += 2) {
            _u64 temp = data & (_u64)(pow(2, cache_word_size[1] * 8) - 1);
            int* a = update_parity_check(temp);
            if (caches[1][l2_index].extra_cache[i] != a[0] ||
                caches[1][l2_index].extra_cache[i + 1] != a[1]) {
                clean_line_error_count++;
                ui->textBrowser->insertPlainText("��żУ���⵽����\n");
                caches[1][l2_index].buf = *((_u64*)(void*)(caches[1][l2_index].addr));
                ui->textBrowser->insertPlainText("�Ѿ�����һ���洢���ָ���\n");
                ui->textBrowser->insertPlainText("��ȷ����Ϊ��");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][l2_index].buf)));
                ui->textBrowser->insertPlainText("\n");
                return;
            }
            data = data >> cache_word_size[1] * 8;
        }
        if (caches[1][l2_index].buf != 0xFFFF) {
            ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][l2_index].buf)));
            ui->textBrowser->insertPlainText("\n");
            error_but_not_detect_for_clean++;
            ui->textBrowser->insertPlainText("parityδ��⵽����\n");
            // Ϊ��֮�����ע�����ȷ�������޸�Ϊ��ȷ����
            parity_check_encode_for_cache(l2_index, 0xFFFF);
            caches[1][l2_index].buf = 0xFFFF;
        }
    }
}

/* parity check encode for Cache */
void CacheSim::parity_check_encode_for_cache(_u64 l2_index, _u64 data) {
    // ��Ҫ���parity check bits
    if (PARITY_CHECK == 1) {
        caches[1][l2_index].extra_cache = (_u8*)malloc(sizeof(_u8) * cache_word_num[1]);
        memset(caches[1][l2_index].extra_cache, 0, sizeof(_u8) * cache_word_num[1]);
        for (_u64 i = 0; i < cache_word_num[1]; ++i) {
            _u64 temp = data & (_u64)(pow(2, cache_word_size[1] * 8) - 1);
            caches[1][l2_index].extra_cache[i] = (_u8)parity_check(temp);
            data = data >> cache_word_size[1] * 8;
        }
    }
    if (PARITY_CHECK == 2) {
        caches[1][l2_index].extra_cache = (_u8*)malloc(sizeof(_u8) * cache_word_num[1] * 2);
        memset(caches[1][l2_index].extra_cache, 0, sizeof(_u8) * cache_word_num[1] * 2);
        for (_u64 i = 0; i < 2 * cache_word_num[1]; i += 2) {
            _u64 temp = data & (_u64)(pow(2, cache_word_size[1] * 8) - 1);
            int* a = update_parity_check(temp);
            caches[1][l2_index].extra_cache[i] = a[0];
            caches[1][l2_index].extra_cache[i + 1] = a[1];
            data = data >> cache_word_size[1] * 8;
        }
    }
}

/* �ҵ����ʵ�line֮�󣬽�����д��L2 Cache line�� */
void CacheSim::mem_write_to_l2(_u64 index, _u64 addr,Ui::MainWindow* ui) {
    Cache_Line* line = caches[1] + index;
    line->mem_write_to_l2_safe(index, addr, &cache_set_shifts[1], &cache_line_shifts[1],tick_count,&m_data);
    //line->tag = addr >> (cache_set_shifts[1] + cache_line_shifts[1]);
    //line->flag = (_u8)~CACHE_FLAG_MASK;
    //line->flag |= CACHE_FLAG_VALID;
    //line->count = tick_count;
    //line->buf = m_data;
    //line->addr = (_u64*)&m_data;
    parity_check_encode_for_cache(index, m_data);
}

/*
 * @arg addr trace�ļ��еĵ�ַ
 * @arg oper_style trace�ļ��еĶ�д��ʶ
*/

void CacheSim::do_cache_op(_u64 addr, char oper_style, int core_num, Ui::MainWindow* ui) {
    int level;
    if (core_num == 0) {
        level = 0;
    }
    else if (core_num == 1) {
        level = 2;
    }
    // ӳ�䵽�ĸ�set����ǰset���׵�ַ
    _u64 set_l1, set_l2, set_base_l1, set_base_l2, set_l1_core1, set_base_l1_core1;
    long long hit_index_l1, hit_index_l2, free_index_l1=0, free_index_l2=0, hit_index_l1_core1, free_index_l1_core1=0;
    tick_count++;
    // ���ݶԵ�ַ�����򻮷֣�����ȡ��ǰ��ַӳ�䵽Cache����һ��set��
    set_l2 = (addr >> cache_line_shifts[1]) % cache_set_size[1];
    set_base_l2 = set_l2 * cache_mapping_ways[1];
    hit_index_l2 = check_cache_hit(set_base_l2, addr, 1,ui);

    set_l1 = (addr >> cache_line_shifts[0]) % cache_set_size[0];
    set_base_l1 = set_l1 * cache_mapping_ways[0];
    hit_index_l1 = check_cache_hit(set_base_l1, addr, 0, ui);

    set_l1_core1 = (addr >> cache_line_shifts[2]) % cache_set_size[2];
    set_base_l1_core1 = set_l1_core1 * cache_mapping_ways[2];
    hit_index_l1_core1 = check_cache_hit(set_base_l1_core1, addr, 2,ui);

    
    //if (level == 0) {
    //    // 1��l1�����У�����l2�Ƿ�����У�l1���д���+1��
    //    if (hit_index_l1 >= 0 && oper_style == OPERATION_READ) {
    //        // ��������l1�����ݵ�cpu
    //        // l1���д���+1
    //        cache_hit_count[level]++;
    //        // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
    //        cache_r_count[level]++;
    //        // �����l1�����ݵ�cpu������һ��ʱ������
    //        period = period + 1;
    //        dirty_interval_time = dirty_interval_time + 1;
    //        // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
    //        if (CACHE_SWAP_LRU == swap_style[level]) {
    //            caches[level][hit_index_l1].lru_count = tick_count;
    //        }
    //    }

    //    // 2��l1��δ���У�l2������
    //    if (hit_index_l1 < 0 && hit_index_l2 >= 0 && oper_style == OPERATION_READ) {
    //        // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
    //        cache_miss_count[level]++;
    //        cache_hit_count[1]++;
    //        cache_r_count[1]++;
    //        cache_w_count[level]++;
    //        period = period + 10 + 1;
    //        dirty_interval_time = dirty_interval_time + 10 + 1;
    //        free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ, ui);
    //        l2_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2, addr, ui);
    //        l2_write_to_cpu((_u64)hit_index_l2, ui);
    //        // ֻҪ�����ˣ����޸�����ķ���ʱ��
    //        if (CACHE_SWAP_LRU == swap_style[1]) {
    //            caches[1][hit_index_l2].lru_count = tick_count;
    //        }
    //    }

    //    // 3��l1��δ���У�l2��δ����
    //    if (hit_index_l1 < 0 && hit_index_l2 < 0 && oper_style == OPERATION_READ) {
    //        // ������ͬʱ���ڴ��ȡ���ݵ�l2/l1��cpu
    //        cache_miss_count[level]++;
    //        cache_miss_count[1]++;
    //        cache_w_count[level]++;
    //        cache_w_count[1]++;
    //        cache_r_memory_count++;
    //        period = period + 100 + 10;
    //        dirty_interval_time = dirty_interval_time + 100 + 10;
    //        // ���ڴ��ȡ���ݵ�l1/cpu
    //        free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ, ui);
    //        cpu_mem_write_to_l1((_u64)free_index_l1, addr, ui);
    //        // ���ڴ��ȡ���ݵ�l2
    //        free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_READ, ui);
    //        // ��ʱ����һ���ɾ���
    //        clean_count++;
    //        mem_write_to_l2((_u64)free_index_l2, addr, ui);
    //        caches[1][free_index_l2].flag &= ~CACHE_FLAG_DIRTY;
    //    }

    //    // 4��l1д���У�l2д����
    //    if (hit_index_l1 >= 0 && hit_index_l2 >= 0 && oper_style == OPERATION_WRITE) {
    //        // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
    //        cache_hit_count[level]++;
    //        cache_hit_count[1]++;
    //        if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //            dirty_count++;
    //            caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
    //            // ������Ҫ����
    //            if (!START_MLREPS_FLAG) {
    //                if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //                if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            }
    //            else {
    //                if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            }
    //        }
    //        if (CACHE_SWAP_LRU == swap_style[level]) {
    //            caches[level][hit_index_l1].lru_count = tick_count;
    //        }
    //        if (CACHE_SWAP_LRU == swap_style[1]) {
    //            caches[1][hit_index_l2].lru_count = tick_count;
    //        }
    //    }

    //    // 5��l1д���У�l2дδ����
    //    if (hit_index_l1 >= 0 && hit_index_l2 < 0 && oper_style == OPERATION_WRITE) {
    //        // �����������ݴ�l1д��l2����������l2Ϊ����
    //        cache_hit_count[level]++;
    //        cache_miss_count[1]++;
    //        cache_r_count[level]++;
    //        cache_w_count[1]++;
    //        period = period + 1 + 10;
    //        dirty_interval_time = dirty_interval_time + 1 + 10;
    //        free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE, ui);
    //        // ��Ҫ���б���
    //        l1_cpu_write_to_l2((_u64)free_index_l2, addr, ui);
    //        if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //            dirty_count++;
    //            caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
    //        }
    //        if (CACHE_SWAP_LRU == swap_style[level]) {
    //            caches[level][hit_index_l1].lru_count = tick_count;
    //        }
    //    }

    //    // 6��l1дδ���У�l2д����
    //    if (hit_index_l1 < 0 && hit_index_l2 >= 0 && oper_style == OPERATION_WRITE) {
    //        // ����������l2��λΪ1
    //        cache_miss_count[level]++;
    //        cache_hit_count[1]++;
    //        if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //            dirty_count++;
    //            caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
    //            if (!START_MLREPS_FLAG) {
    //                if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //                if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            }
    //            else {
    //                if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            }
    //        }
    //        if (CACHE_SWAP_LRU == swap_style[1]) {
    //            caches[1][hit_index_l2].lru_count = tick_count;
    //        }
    //    }

    //    // 7��l1дδ���У�l2дδ����
    //    if (hit_index_l1 < 0 && hit_index_l2 < 0 && oper_style == OPERATION_WRITE) {
    //        // ������cpuͬʱ������д��l1��l2��������l2����λΪ1
    //        cache_miss_count[level]++;
    //        cache_miss_count[1]++;
    //        cache_w_count[level]++;
    //        cache_w_count[1]++;
    //        period = period + 10;
    //        dirty_interval_time = dirty_interval_time + 10;
    //        free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_WRITE, ui);
    //        cpu_mem_write_to_l1((_u64)free_index_l1, addr, ui);
    //        free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE, ui);
    //        // ��Ҫ���б���
    //        l1_cpu_write_to_l2((_u64)free_index_l2, addr, ui);
    //        if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //            dirty_count++;
    //            caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
    //        }
    //    }
    //    
    //}
    //else {
    //if (hit_index_l1_core1 >= 0 && oper_style == OPERATION_READ) {
    //    // ��������l1�����ݵ�cpu
    //    // l1���д���+1
    //    cache_hit_count[level]++;
    //    // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
    //    cache_r_count[level]++;
    //    // �����l1�����ݵ�cpu������һ��ʱ������
    //    period = period + 1;
    //    dirty_interval_time = dirty_interval_time + 1;
    //    // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
    //    if (CACHE_SWAP_LRU == swap_style[level]) {
    //        caches[level][hit_index_l1_core1].lru_count = tick_count;
    //    }
    //}

    //// 2��l1��δ���У�l2������
    //if (hit_index_l1_core1 < 0 && hit_index_l2 >= 0 && oper_style == OPERATION_READ) {
    //    // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
    //    cache_miss_count[level]++;
    //    cache_hit_count[1]++;
    //    cache_r_count[1]++;
    //    cache_w_count[level]++;
    //    period = period + 10 + 1;
    //    dirty_interval_time = dirty_interval_time + 10 + 1;
    //    free_index_l1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ, ui);
    //    l2_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2, addr, ui);
    //    l2_write_to_cpu((_u64)hit_index_l2, ui);
    //    // ֻҪ�����ˣ����޸�����ķ���ʱ��
    //    if (CACHE_SWAP_LRU == swap_style[1]) {
    //        caches[1][hit_index_l2].lru_count = tick_count;
    //    }
    //}

    //// 3��l1��δ���У�l2��δ����
    //if (hit_index_l1_core1 < 0 && hit_index_l2 < 0 && oper_style == OPERATION_READ) {
    //    // ������ͬʱ���ڴ��ȡ���ݵ�l2/l1��cpu
    //    cache_miss_count[level]++;
    //    cache_miss_count[1]++;
    //    cache_w_count[level]++;
    //    cache_w_count[1]++;
    //    cache_r_memory_count++;
    //    period = period + 100 + 10;
    //    dirty_interval_time = dirty_interval_time + 100 + 10;
    //    // ���ڴ��ȡ���ݵ�l1/cpu
    //    free_index_l1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ, ui);
    //    cpu_mem_write_to_l1((_u64)free_index_l1, addr, ui);
    //    // ���ڴ��ȡ���ݵ�l2
    //    free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_READ, ui);
    //    // ��ʱ����һ���ɾ���
    //    clean_count++;
    //    mem_write_to_l2((_u64)free_index_l2, addr, ui);
    //    caches[1][free_index_l2].flag &= ~CACHE_FLAG_DIRTY;
    //}

    //// 4��l1д���У�l2д����
    //if (hit_index_l1_core1 >= 0 && hit_index_l2 >= 0 && oper_style == OPERATION_WRITE) {
    //    // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
    //    cache_hit_count[level]++;
    //    cache_hit_count[1]++;
    //    if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //        dirty_count++;
    //        caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
    //        // ������Ҫ����
    //        if (!START_MLREPS_FLAG) {
    //            if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //        }
    //        else {
    //            if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //        }
    //    }
    //    if (CACHE_SWAP_LRU == swap_style[level]) {
    //        caches[level][hit_index_l1_core1].lru_count = tick_count;
    //    }
    //    if (CACHE_SWAP_LRU == swap_style[1]) {
    //        caches[1][hit_index_l2].lru_count = tick_count;
    //    }
    //}

    //// 5��l1д���У�l2дδ����
    //if (hit_index_l1_core1 >= 0 && hit_index_l2 < 0 && oper_style == OPERATION_WRITE) {
    //    // �����������ݴ�l1д��l2����������l2Ϊ����
    //    cache_hit_count[level]++;
    //    cache_miss_count[1]++;
    //    cache_r_count[level]++;
    //    cache_w_count[1]++;
    //    period = period + 1 + 10;
    //    dirty_interval_time = dirty_interval_time + 1 + 10;
    //    free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE, ui);
    //    // ��Ҫ���б���
    //    l1_cpu_write_to_l2((_u64)free_index_l2, addr, ui);
    //    if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //        dirty_count++;
    //        caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
    //    }
    //    if (CACHE_SWAP_LRU == swap_style[level]) {
    //        caches[level][hit_index_l1_core1].lru_count = tick_count;
    //    }
    //}

    //// 6��l1дδ���У�l2д����
    //if (hit_index_l1_core1 < 0 && hit_index_l2 >= 0 && oper_style == OPERATION_WRITE) {
    //    // ����������l2��λΪ1
    //    cache_miss_count[level]++;
    //    cache_hit_count[1]++;
    //    if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //        dirty_count++;
    //        caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
    //        if (!START_MLREPS_FLAG) {
    //            if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //            if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //        }
    //        else {
    //            if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
    //        }
    //    }
    //    if (CACHE_SWAP_LRU == swap_style[1]) {
    //        caches[1][hit_index_l2].lru_count = tick_count;
    //    }
    //}

    //// 7��l1дδ���У�l2дδ����
    //if (hit_index_l1_core1 < 0 && hit_index_l2 < 0 && oper_style == OPERATION_WRITE) {
    //    // ������cpuͬʱ������д��l1��l2��������l2����λΪ1
    //    cache_miss_count[level]++;
    //    cache_miss_count[1]++;
    //    cache_w_count[level]++;
    //    cache_w_count[1]++;
    //    period = period + 10;
    //    dirty_interval_time = dirty_interval_time + 10;
    //    free_index_l1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_WRITE, ui);
    //    cpu_mem_write_to_l1((_u64)free_index_l1, addr, ui);
    //    free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE, ui);
    //    // ��Ҫ���б���
    //    l1_cpu_write_to_l2((_u64)free_index_l2, addr, ui);
    //    if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
    //        dirty_count++;
    //        caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
    //    }
    //}
    //    }
    int state = 0;
    if (oper_style == OPERATION_READ) {
        if (level == 0&&hit_index_l1>=0) {
            state = (caches[level] + hit_index_l1)->state;
        }
        else if (level == 2 && hit_index_l1_core1 >= 0) {
            state = (caches[level] + hit_index_l1_core1)->state;
        }
        local_read(state,level,hit_index_l1,hit_index_l2,hit_index_l1_core1,set_base_l1_core1,set_base_l2,set_base_l1,free_index_l2,free_index_l1_core1,free_index_l1,addr,ui);
    }
    else if (oper_style == OPERATION_WRITE) {
        if (level == 0 && hit_index_l1 >= 0) {
            state = (caches[level] + hit_index_l1)->state;
        }
        else if (level == 2 && hit_index_l1_core1 >= 0) {
            state = (caches[level] + hit_index_l1_core1)->state;
        }
        local_write(state, level, hit_index_l1, hit_index_l2, hit_index_l1_core1, set_base_l1_core1, set_base_l2, set_base_l1, free_index_l2, free_index_l1_core1, free_index_l1, addr,ui);
    }

    // ���ڻ�д
    if (EARLY_WRITE_BACK == 1) {
        early_write_back(50,ui);
    }
}

/* ���еı���/������� */
_u64 CacheSim::cal(_u64 sz) {
    _u64 k = 0;
    _u64 cur = 1;
    while (cur - 1 < sz + k) {
        cur <<= 1;
        k++;
    }
    return k;
}

string CacheSim::hamming_encode(string s) {
    string d;
    d.clear();
    _u64 k = cal(s.size());
    d.resize(s.size() + k);
    for (_u64 i = 0, j = 0, p = 0; i != d.size(); i++) {
        if ((i + 1) == pow(2, p) && p < k) {
            d[i] = '0';
            p++;
        }
        else if (s[j] == '0' || s[j] == '1') {
            d[i] = s[j++];
        }
        else {
            cout << "error!" << endl;
        }
    }
    for (_u64 i = 0; i != k; i++) {
        _u64 count = 0, index = 1 << i;
        for (_u64 j = index - 1; j < d.size(); j += index) {
            for (_u64 k = 0; k != index && j < d.size(); k++, j++) {
                count ^= d[j] - '0';
            }
        }
        d[index - 1] = '0' + count;
    }
    return d;
}

_u64 CacheSim::antiCal(_u64 sz) {
    _u64 k = 0;
    _u64 cur = 1;
    while (cur < sz) {
        cur <<= 1;
        k++;
    }
    return k;
}

int CacheSim::hamming_decode(string d, string& s) {
    s.clear();
    _u64 k = antiCal(d.size());
    s.resize(d.size() - k);
    _u64 sum = 0;
    for (_u64 p = 0; p != k; p++) {
        int pAnti = 0;
        _u64 index = 1 << p;
        for (_u64 i = index - 1; i < d.size(); i += index) {
            for (_u64 j = 0; j < index && i < d.size(); i++, j++) {
                pAnti ^= d[i] - '0';
            }
        }
        sum += pAnti << p;
    }
    // ���ڼٶ�Cache�д�С����Ϊ8�ֽڣ�����λΪ64λ����ôУ��λΪ7λ
    if (sum > 71) {
        // cout << "��������ֲ��ܾ����Ĵ���!" << endl;
        return -1;
    }
    if (sum != 0) {
        d[sum - 1] = (1 - (int)(d[sum - 1] - '0')) + '0';
    }
    for (_u64 i = 0, p = 0, j = 0; i != d.size(); i++) {
        if ((i + 1) == (1 << p) && p < k) {
            p++;
        }
        else {
            s[j++] = d[i];
        }
    }
    return sum;
}

string CacheSim::dec_to_bin(_u64 m) {
    char* bin = (char*)malloc(sizeof(char) * 65);
    memset(bin, 0, sizeof(char) * 65);
    for (int i = 63; i >= 0; i--) {
        int temp = ((m >> i) & 1);
        bin[i] = (char)(temp + '0');
    }
    bin[64] = '\0';
    return bin;
}

_u64 CacheSim::bin_to_dec(string bin) {
    _u64 size = bin.size();
    _u64 parseBinary = 0;
    for (_u64 i = 0; i < size; ++i) {
        if (bin[i] == '1') {
            parseBinary += pow(2.0, size - i - 1);
        }
    }
    return parseBinary;
}



//hamming_encode hamming_decode
void CacheSim::hamming_encode_for_cache(_u64 index, _u64 data) {
    // SEC-DED���Cache line������ÿ��д����64λ����������Cache line��СΪ8�ֽڣ���������Ϊ64λ���ݱ���
    // 64λ���ݱ�����Ҫ7��У��λ
    caches[1][index].extra_cache = (_u8*)malloc(sizeof(_u8) * 8);
    memset(caches[1][index].extra_cache, 0, sizeof(_u8) * 8);
    caches[1][index].extra_cache[7] = '\0';
    string str = dec_to_bin(data);
    string encode_str = hamming_encode(str);
    for (int i = 0; i < 7; ++i) {
        caches[1][index].extra_cache[i] = encode_str[pow(2, i) - 1];
    }
}

void CacheSim::hamming_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui) {
    string decode_str = "";
    string str = dec_to_bin(data);
    decode_str = decode_str + (char)caches[1][index].extra_cache[0] + (char)caches[1][index].extra_cache[1] +
        str.substr(0, 1) + (char)caches[1][index].extra_cache[2] + str.substr(1, 3) + (char)caches[1][index].extra_cache[3] +
        str.substr(4, 7) + (char)caches[1][index].extra_cache[4] + str.substr(11, 15) + (char)caches[1][index].extra_cache[5] +
        str.substr(26, 21) + (char)caches[1][index].extra_cache[6] + str.substr(47, 18);
    string real_data;
    int ret = hamming_decode(decode_str, real_data);;
    if (ret < 0) {
        dirty_line_error_count++;
        ui->textBrowser->insertPlainText("��������ֲ��ܾ����Ĵ���!\n");
        // ���ֲ��ܾ���Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
        hamming_encode_for_cache(index, 0xFFFF);
        caches[1][index].buf = 0xFFFF;

        // ���ý�����д���ƣ�������MLREPSУ��
        if (EMERGENCY_WRITE_BACK == 1) {
            emergency_write_back(ui);
        }
    }
    if (ret == 0) {
        if (caches[1][index].buf != 0xFFFF) {
            ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
            ui->textBrowser->insertPlainText("\n");
            error_but_not_detect_for_dirty++;
            ui->textBrowser->insertPlainText("δ��⵽����\n");
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            hamming_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
    }
    if (ret > 0) {
        dirty_line_error_count++;
        ui->textBrowser->insertPlainText("�������⵽����\n");
        reverse(real_data.begin(), real_data.end());
        caches[1][index].buf = bin_to_dec(real_data);
        ui->textBrowser->insertPlainText("�������޸���\n");
        ui->textBrowser->insertPlainText("��ȷ����Ϊ��");
        ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
        ui->textBrowser->insertPlainText("\n");

        if (caches[1][index].buf != 0xFFFF) {
            fail_correct++;
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            hamming_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
        else {
            dirty_line_error_correct_count++;
        }

        // ���ý�����д���ƣ�������MLREPSУ��
        if (EMERGENCY_WRITE_BACK == 1) {
            emergency_write_back(ui);
        }
    }
}


//secded_encode secded_decode
string CacheSim::secded_encode(string s) {
    string d;
    d.clear();
    _u64 k = cal(s.size());
    d.resize(s.size() + k);
    // ����У��λ�������0
    for (_u64 i = 0, j = 0, p = 0; i != d.size(); i++) {
        if ((i + 1) == pow(2, p) && p < k) {
            d[i] = '0';
            p++;
        }
        else if (s[j] == '0' || s[j] == '1') {
            d[i] = s[j++];
        }
        else {
            cout << "error!" << endl;
        }
    }
    // Ҫ��¼ÿ��λ�������˶��ٴ�
    int* n = (int*)malloc(sizeof(int) * d.size());
    memset(n, 0, sizeof(int) * d.size());
    for (_u64 i = 0; i != k; i++) {
        _u64 count = 0, index = 1 << i;
        for (_u64 j = index - 1; j < d.size(); j += index) {
            for (_u64 k = 0; k != index && j < d.size(); k++, j++) {
                count ^= d[j] - '0';
                n[j]++;
            }
        }
        d[index - 1] = '0' + count;
    }
    // ����һ��У��λ
    _u64 count = 0;
    for (_u64 i = 0; i < d.size(); ++i) {
        if (n[i] == 2) {
            count ^= d[i] - '0';
            n[i]++;
        }
    }
    d += ('0' + count);
    return d;
}

int CacheSim::secded_decode(string _d, string& s) {
    s.clear();
    string d = _d.substr(0, _d.size() - 1);
    _u64 k = antiCal(d.size());
    s.resize(d.size() - k);
    _u64* parity = (_u64*)malloc(sizeof(_u64) * (k + 1));
    memset(parity, 0, sizeof(int) * (k + 1));
    int* n = (int*)malloc(sizeof(int) * d.size());
    memset(n, 0, sizeof(int) * d.size());

    _u64 sum = 0;
    for (_u64 p = 0; p != k; p++) {
        _u64 pAnti = 0;
        _u64 index = 1 << p;
        for (_u64 i = index - 1; i < d.size(); i += index) {
            for (_u64 j = 0; j < index && i < d.size(); i++, j++) {
                pAnti ^= d[i] - '0';
                n[i]++;
            }
        }
        parity[p] = pAnti;
        sum += pAnti << p;
    }

    _u64 count = 0;
    for (_u64 i = 0; i < d.size(); ++i) {
        if (n[i] == 2) {
            count ^= d[i] - '0';
            n[i]++;
        }
    }

    parity[k] = count ^ (_d[_d.size() - 1] - '0');

    int not_zero = 0;
    for (int i = 0; i < k + 1; ++i) {
        if (parity[i] != 0) {
            ++not_zero;
        }
    }
    if (not_zero == 0) {
        s = "";
        return 0;
    }
    else if (not_zero >= 3 && not_zero <= 6) {
        if (sum > d.size()) {
            return -1;
        }
        if (sum != 0) {
            d[sum - 1] = (1 - (int)(d[sum - 1] - '0')) + '0';
        }
        for (_u64 i = 0, p = 0, j = 0; i != d.size(); i++) {
            if ((i + 1) == (1 << p) && p < k) {
                p++;
            }
            else {
                s[j++] = d[i];
            }
        }
        return 1;
    }
    else {
        // cout << "SEC-DED���ֲ��ܾ����Ĵ���!" << endl;
        return -1;
    }
    return 0;
}

void CacheSim::secded_encode_for_cache(_u64 index, _u64 data) {
    // SEC-DED���Cache line������ÿ��д����64λ����������Cache line��СΪ8�ֽڣ���������Ϊ64λ���ݱ���
    // 64λ���ݱ�����Ҫ8��У��λ��hamming��Ҫ7��У��λ
    caches[1][index].extra_cache = (_u8*)malloc(sizeof(_u8) * 9);
    memset(caches[1][index].extra_cache, 0, sizeof(_u8) * 9);
    caches[1][index].extra_cache[8] = '\0';
    string str = dec_to_bin(data);
    string encode_str = secded_encode(str);
    for (int i = 0; i < 7; ++i) {
        caches[1][index].extra_cache[i] = encode_str[pow(2, i) - 1];
    }
    caches[1][index].extra_cache[7] = encode_str[encode_str.size() - 1];
}

void CacheSim::secded_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui) {
    string decode_str = "";
    string str = dec_to_bin(data);
    decode_str = decode_str + (char)caches[1][index].extra_cache[0] + (char)caches[1][index].extra_cache[1] +
        str.substr(0, 1) + (char)caches[1][index].extra_cache[2] + str.substr(1, 3) + (char)caches[1][index].extra_cache[3] +
        str.substr(4, 7) + (char)caches[1][index].extra_cache[4] + str.substr(11, 15) + (char)caches[1][index].extra_cache[5] +
        str.substr(26, 21) + (char)caches[1][index].extra_cache[6] + str.substr(47, 18) + (char)caches[1][index].extra_cache[7];

    string real_data;
    int ret = secded_decode(decode_str, real_data);

    if (ret < 0) {
        dirty_line_error_count++;
        ui->textBrowser->insertPlainText("SEC-DED���ֲ��ܾ����Ĵ���!\n");
        // ���ֲ��ܾ���Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
        secded_encode_for_cache(index, 0xFFFF);
        caches[1][index].buf = 0xFFFF;

        // ���ý�����д���ƣ�������MLREPSУ��
        if (EMERGENCY_WRITE_BACK == 1) {
            emergency_write_back(ui);
        }
    }
    if (ret == 0) {
        // cout << "SEC-DEDδ��⵽����" << endl;
        if (caches[1][index].buf != 0xFFFF) {
            ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
            ui->textBrowser->insertPlainText("\n");
            error_but_not_detect_for_dirty++;
            ui->textBrowser->insertPlainText("δ��⵽����\n");
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            secded_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
    }
    if (ret == 1) {
        dirty_line_error_count++;
        ui->textBrowser->insertPlainText("SEC-DED��⵽����\n");
        reverse(real_data.begin(), real_data.end());
        caches[1][index].buf = bin_to_dec(real_data);

        ui->textBrowser->insertPlainText("�������޸���\n");
        ui->textBrowser->insertPlainText("��ȷ����Ϊ��");
        ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
        ui->textBrowser->insertPlainText("\n");

        if (caches[1][index].buf != 0xFFFF) {
            fail_correct++;
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            secded_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
        else {
            dirty_line_error_correct_count++;
        }

        // ���ý�����д���ƣ�������MLREPSУ��
        if (EMERGENCY_WRITE_BACK == 1) {
            emergency_write_back(ui);
        }
    }
}

string CacheSim::mlreps_encode(string s) {
    string d;
    char* hp = (char*)malloc(sizeof(char) * 17);
    memset(hp, 0, sizeof(char) * 17);
    char* lp = (char*)malloc(sizeof(char) * 49);
    memset(lp, 0, sizeof(char) * 49);
    char* vp = (char*)malloc(sizeof(char) * 25);
    memset(vp, 0, sizeof(char) * 25);
    hp[16] = '\0';
    lp[49] = '\0';
    vp[25] = '\0';

    int count = 0;
    int k = 0;
    for (int i = 0; i < 16; ++i) {
        count = (s[i + 0] - '0') ^ (s[i + 16] - '0') ^ (s[i + 32] - '0') ^ (s[i + 48] - '0');
        hp[i] = count + '0';
        count = (s[i + 0] - '0') ^ (s[i + 48] - '0');
        lp[k++] = count + '0';
        count = (s[i + 16] - '0') ^ (s[i + 48] - '0');
        lp[k++] = count + '0';
        count = (s[i + 32] - '0') ^ (s[i + 48] - '0');
        lp[k++] = count + '0';
    }

    k = 0;
    for (int i = 0; i < 24; i += 3) {
        count = (lp[i] - '0') ^ (lp[i + 24] - '0');
        vp[k++] = count + '0';
        count = (lp[i + 1] - '0') ^ (lp[i + 1 + 24] - '0');
        vp[k++] = count + '0';
        count = (lp[i + 2] - '0') ^ (lp[i + 2 + 24] - '0');
        vp[k++] = count + '0';
    }
    d = s + (string)hp + (string)vp;
    return d;
}

int CacheSim::mlreps_decode(string _d, string& s, Ui::MainWindow* ui) {
    string d = _d.substr(0, 64);
    string h_parity = _d.substr(64, 16);
    string v_parity = _d.substr(80, 24);

    char* lp = (char*)malloc(sizeof(char) * 49);
    memset(lp, 0, sizeof(char) * 49);
    lp[49] = '\0';

    string h_valid, v_valid;
    s = d;

    int count = 0;
    int k = 0;
    int n = 0;
    for (int i = 0; i < 16; ++i) {
        count = (d[i + 0] - '0') ^ (d[i + 16] - '0') ^ (d[i + 32] - '0') ^ (d[i + 48] - '0');
        if (((h_parity[i] - '0') ^ count) == 0) {
            h_valid.append(1, '0');
        }
        else {
            h_valid.append(1, '1');
            n++;
        }
        count = (d[i + 0] - '0') ^ (d[i + 48] - '0');
        lp[k++] = count + '0';
        count = (d[i + 16] - '0') ^ (d[i + 48] - '0');
        lp[k++] = count + '0';
        count = (d[i + 32] - '0') ^ (d[i + 48] - '0');
        lp[k++] = count + '0';
    }

    k = 0;
    for (int i = 0; i < 24; i += 3) {
        count = (lp[i] - '0') ^ (lp[i + 24] - '0');
        if ((count ^ (v_parity[k++] - '0')) == 0) {
            v_valid.append(1, '0');
        }
        else {
            v_valid.append(1, '1');
            n++;
        }
        count = (lp[i + 1] - '0') ^ (lp[i + 1 + 24] - '0');
        if ((count ^ (v_parity[k++] - '0')) == 0) {
            v_valid.append(1, '0');
        }
        else {
            v_valid.append(1, '1');
            n++;
        }
        count = (lp[i + 2] - '0') ^ (lp[i + 2 + 24] - '0');
        if ((count ^ (v_parity[k++] - '0')) == 0) {
            v_valid.append(1, '0');
        }
        else {
            v_valid.append(1, '1');
            n++;
        }
    }

    if (n == 0) {
        return 0;
    }

    int ret = 0;
    k = 0;
    for (int i = 0; i < 16; ++i) {
        if (h_valid[i] == '1') {
            if ((v_valid[k] == '0' && v_valid[k + 1] == '0' && v_valid[k + 2] == '0') ||
                (v_valid[k] == '1' && v_valid[k + 1] == '1' && v_valid[k + 2] == '0') ||
                (v_valid[k] == '1' && v_valid[k + 1] == '0' && v_valid[k + 2] == '1') ||
                (v_valid[k] == '0' && v_valid[k + 1] == '1' && v_valid[k + 2] == '1')) {
                ui->textBrowser->insertPlainText("MLREPS�޷�ȷ����������λ�ã�\n");
                return -1;
            }
            if (v_valid[k] == '1' && v_valid[k + 1] == '0' && v_valid[k + 2] == '0') {
                ui->textBrowser->insertPlainText("MLREPS:E");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(i + 0)));
                ui->textBrowser->insertPlainText("���ִ���\n");

                if (s[i] == '0') s[i] = '1';
                else s[i] = '0';
                ret = 1;
                ui->textBrowser->insertPlainText("�������޸���\n");
            }
            if (v_valid[k] == '0' && v_valid[k + 1] == '1' && v_valid[k + 2] == '0') {
                ui->textBrowser->insertPlainText("MLREPS:E");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(i + 16)));
                ui->textBrowser->insertPlainText("���ִ���\n");

                if (s[i + 16] == '0') s[i + 16] = '1';
                else s[i + 16] = '0';
                ret = 1;
                ui->textBrowser->insertPlainText("�������޸���\n");
            }
            if (v_valid[k] == '0' && v_valid[k + 1] == '0' && v_valid[k + 2] == '1') {
                ui->textBrowser->insertPlainText("MLREPS:E");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(i + 32)));
                ui->textBrowser->insertPlainText("���ִ���\n");

                if (s[i + 32] == '0') s[i + 32] = '1';
                else s[i + 32] = '0';
                ret = 1;
                ui->textBrowser->insertPlainText("�������޸���\n");
            }
            if (v_valid[k] == '1' && v_valid[k + 1] == '1' && v_valid[k + 2] == '1') {
                ui->textBrowser->insertPlainText("MLREPS:E");
                ui->textBrowser->insertPlainText(QString::fromStdString(to_string(i + 48)));
                ui->textBrowser->insertPlainText("���ִ���\n");

                if (s[i + 48] == '0') s[i + 48] = '1';
                else s[i + 48] = '0';
                ret = 1;
                ui->textBrowser->insertPlainText("�������޸���\n");
            }
        }
        k += 3;
        if (i == 7) {
            k = 0;
        }
    }
    return ret;
}

void CacheSim::mlreps_encode_for_cache(_u64 index, _u64 data) {
    // MLREPS���Cache line������ÿ��д����64λ����������Cache line��СΪ8�ֽڣ���������Ϊ64λ���ݱ���
    // 64λ���ݱ�����Ҫ40��У��λ������16��ˮƽ��żУ��λ��24����ֱ��żУ��λ
    caches[1][index].extra_mem = (_u8*)malloc(sizeof(_u8) * 41);
    memset(caches[1][index].extra_mem, 0, sizeof(_u8) * 41);
    caches[1][index].extra_mem[40] = '\0';
    string str = dec_to_bin(data);
    string encode_str = mlreps_encode(str);
    for (int i = 64; i < encode_str.size(); ++i) {
        caches[1][index].extra_mem[i - 64] = encode_str[i];
    }
}

void CacheSim::mlreps_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui) {
    string decode_str = "";
    string str = dec_to_bin(data);
    string temp;
    for (int i = 0; i < 40; ++i) {
        temp += caches[1][index].extra_mem[i];
    }
    decode_str = str + temp;
    string real_data;
    int ret = mlreps_decode(decode_str, real_data,ui);
    if (ret < 0) {
        dirty_line_error_count++;
        // ���ֲ��ܾ���Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
        mlreps_encode_for_cache(index, 0xFFFF);
        caches[1][index].buf = 0xFFFF;
        dirty_interval_time = 0;
    }
    if (ret == 0) {
        // cout << "MLREPSδ��⵽����" << endl;
        if (caches[1][index].buf != 0xFFFF) {
            error_but_not_detect_for_dirty++;
            ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
            ui->textBrowser->insertPlainText("\n");
            ui->textBrowser->insertPlainText("δ��⵽����\n");
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            // ������Ӧ�ı���ҲӦ���޸ģ����ﶼû���±���
            mlreps_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
    }
    if (ret == 1) {
        // ���ִ�������ô�����ʱ��
        dirty_interval_time = 0;
        dirty_line_error_count++;
        reverse(real_data.begin(), real_data.end());
        caches[1][index].buf = bin_to_dec(real_data);
        ui->textBrowser->insertPlainText("��ȷ����Ϊ��");
        ui->textBrowser->insertPlainText(QString::fromStdString(to_string(caches[1][index].buf)));
        ui->textBrowser->insertPlainText("\n");

        if (caches[1][index].buf != 0xFFFF) {
            fail_correct++;
            // ���ֲ��ܼ�����Ϊ�˺���ע�룬�������޸�Ϊ��ȷ��
            mlreps_encode_for_cache(index, 0xFFFF);
            caches[1][index].buf = 0xFFFF;
        }
        else {
            dirty_line_error_correct_count++;
        }
    }
}





/* ���ڻ�дʵ�֣�����Ϊ�̶������� */
void CacheSim::early_write_back(_u64 time, Ui::MainWindow* ui) {
    // ��ʼ����timeΪ10kʱ������
    if (period >= time * 1024) {
        // �������е��飬�ҵ����У������Ϊ�ɾ��У�������Ӹɾ��е�У��
        for (_u64 i = 0; i < cache_set_size[1] * cache_mapping_ways[1]; i++) {
            if (caches[1][i].flag & CACHE_FLAG_DIRTY) {
                // l2 to mem
                clean_count++;
                _u64 data = caches[1][i].buf;
                // �����ŵ��ڴ�ĵ�ַΪm_data��ȫ�ֱ������ĵ�ַ
                // �����н���
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_decode_for_cache(i, caches[1][i].buf,ui);
                    if (SECDED == 2) secded_decode_for_cache(i, caches[1][i].buf,ui);
                }
                else {
                    if (MLREPS == 1) mlreps_decode_for_cache(i, caches[1][i].buf,ui);
                }
                caches[1][i].flag &= ~CACHE_FLAG_DIRTY;
                caches[1][i].addr = (_u64*)&m_data;
                parity_check_encode_for_cache(i, m_data);
                cache_w_memory_count++;
                period = period + 100 + 10;
                dirty_interval_time = dirty_interval_time + 10 + 100;
            }
        }
        period = 0;
        // MLREPS�޳���������Ϊ20k����֮��������ת��ΪSEC-DED����
        if (MLREPS == 1 && START_MLREPS_FLAG == true && dirty_interval_time >= 100 * 1024) {
            // cout << dirty_interval_time << endl;
            if (START_MLREPS_FLAG == true) {
                START_MLREPS_FLAG = false;
                // ��������������������д����Ҳ��Ч
            }
        }
    }
}





/* ������дʵ�֣����е������ݶ���д��������������������� */
void CacheSim::emergency_write_back(Ui::MainWindow* ui) {
    // �������ڽ��������������������ٴλ�д
    // if (emergency_period == 0) {
    if (START_MLREPS_FLAG == false) {
        // ���ý�����д����Ϊ1kʱ������
        // �޸ģ�������д����Ӧ����MLREPS���뿪���ر��й�
        // ��������д����->������д�ر�->MLREPS����->MLREPS�ر�->������д����
        // emergency_period = 1 * 1024;
        // �������е��飬�ҵ����У������Ϊ�ɾ��У�������Ӹɾ��е�У��
        // ��ʵ�����п����ǳ�����У������Ȱ����еĴ������
        for (_u64 i = 0; i < cache_set_size[1] * cache_mapping_ways[1]; ++i) {
            if (caches[1][i].flag & CACHE_FLAG_DIRTY) {
                // l2 to mem
                clean_count++;
                _u64 data = caches[1][i].buf;
                // �����ŵ��ڴ�ĵ�ַΪm_data��ȫ�ֱ������ĵ�ַ
                // �����н���
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_decode_for_cache(i, caches[1][i].buf,ui);
                    if (SECDED == 2) secded_decode_for_cache(i, caches[1][i].buf,ui);
                }
                else {
                    if (MLREPS == 1) mlreps_decode_for_cache(i, caches[1][i].buf,ui);
                }
                caches[1][i].flag &= ~CACHE_FLAG_DIRTY;
                caches[1][i].addr = (_u64*)&m_data;
                parity_check_encode_for_cache(i, m_data);
                cache_w_memory_count++;
                period = period + 100 + 10;
                dirty_interval_time = dirty_interval_time + 100 + 10;
            }
        }
        period = 0;
        if (MLREPS == 1) {
            if (START_MLREPS_FLAG == false) {
                START_MLREPS_FLAG = true;
            }
        }
    }
}

int* CacheSim::rand_0(int min, int max, int n) {
    vector<int> temp;
    for (int i = min; i <= max; ++i) {
        temp.push_back(i);
    }
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(temp.begin(), temp.end(), default_random_engine(seed));
    int* trigger_time = (int*)malloc(sizeof(int) * n);
    memset(trigger_time, 0, sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        trigger_time[i] = temp[i];
    }
    sort(trigger_time, trigger_time + n);
    return trigger_time;
}

int* CacheSim::rand_1(int min, int max, int n) {
    vector<int> temp;
    for (int i = min; i <= max; ++i) {
        temp.push_back(i);
    }
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(temp.begin(), temp.end(), default_random_engine(seed));
    int* index = (int*)malloc(sizeof(int) * n);
    memset(index, 0, sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        index[i] = temp[i % 31];
    }
    return index;
}

int* CacheSim::cache_error_inject(_u64 min, _u64 max, _u64 n, int error_type) {
    int* error_struct = (int*)malloc(sizeof(int) * 20 * n);
    memset(error_struct, 0, sizeof(int) * 20 * n);
    int* trigger_time = (int*)malloc(sizeof(int) * n);
    memset(trigger_time, 0, sizeof(int) * n);
    int* line = (int*)malloc(sizeof(int) * n);
    memset(line, 0, sizeof(int) * n);
    int* index = (int*)malloc(sizeof(int) * n);
    memset(index, 0, sizeof(int) * n);

    trigger_time = rand_0(min, max, n);
    line = rand_1(0, 1023, n);
    index = rand_1(2, 61, n);

    int k = 0;
    for (int i = 0; i < n; ++i) {
        error_struct[k++] = trigger_time[i];
        if (error_type == 1) {
            error_struct[k++] = error_type;
            error_struct[k++] = line[i];
            error_struct[k++] = index[i];
        }
        if (error_type == 2) {
            error_struct[k++] = error_type;
            error_struct[k++] = line[i];
            error_struct[k++] = index[i];
            error_struct[k++] = index[i] + 1;
        }
        if (error_type == 3) {
            error_struct[k++] = error_type;
            error_struct[k++] = line[i];
            error_struct[k++] = index[i] - 1;
            error_struct[k++] = index[i];
            error_struct[k++] = index[i] + 1;
        }
        if (error_type == 4) {
            error_struct[k++] = error_type;
            error_struct[k++] = line[i];
            error_struct[k++] = index[i] - 1;
            error_struct[k++] = index[i];
            error_struct[k++] = index[i] + 1;
            error_struct[k++] = index[i] + 2;
        }
        if (error_type == 5) {
            error_struct[k++] = error_type;
            error_struct[k++] = line[i];
            error_struct[k++] = index[i] - 2;
            error_struct[k++] = index[i] - 1;
            error_struct[k++] = index[i];
            error_struct[k++] = index[i] + 1;
            error_struct[k++] = index[i] + 2;
        }
        if (error_type == 12) {
            error_struct[k++] = 12;
            if (line[i] % 2 == 1) {
                error_struct[k++] = 1;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
            }
            if (line[i] % 2 == 0) {
                error_struct[k++] = 2;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
            }
        }
        if (error_type == 13) {
            error_struct[k++] = 13;
            if (line[i] % 3 == 1) {
                error_struct[k++] = 1;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
            }
            if (line[i] % 3 == 2) {
                error_struct[k++] = 2;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
            }
            if (line[i] % 3 == 0) {
                error_struct[k++] = 3;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
            }
        }
        if (error_type == 14) {
            error_struct[k++] = 14;
            if (line[i] % 4 == 1) {
                error_struct[k++] = 1;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
            }
            if (line[i] % 4 == 2) {
                error_struct[k++] = 2;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
            }
            if (line[i] % 4 == 3) {
                error_struct[k++] = 3;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
            }
            if (line[i] % 4 == 0) {
                error_struct[k++] = 4;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
                error_struct[k++] = index[(i + 3) % n];
            }
        }
        if (error_type == 15) {
            error_struct[k++] = 15;
            if (line[i] % 5 == 1) {
                error_struct[k++] = 1;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
            }
            if (line[i] % 5 == 2) {
                error_struct[k++] = 2;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
            }
            if (line[i] % 5 == 3) {
                error_struct[k++] = 3;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
            }
            if (line[i] % 5 == 4) {
                error_struct[k++] = 4;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
                error_struct[k++] = index[(i + 3) % n];
            }
            if (line[i] % 5 == 0) {
                error_struct[k++] = 5;
                error_struct[k++] = line[i];
                error_struct[k++] = index[i];
                error_struct[k++] = index[(i + 1) % n];
                error_struct[k++] = index[(i + 2) % n];
                error_struct[k++] = index[(i + 3) % n];
                error_struct[k++] = index[(i + 4) % n];
            }
        }
    }
    return error_struct;
}

void CacheSim::local_read(int &state,int level, long long core0_l1_hit_index,long long hit_index_l2,long long core1_l1_hit_index,_u64 set_base_l1_core1,_u64 set_base_l2,_u64 set_base_l1,long long free_index_l2,long long free_index_l1_core1,long long free_index_l1,_u64 addr, Ui::MainWindow* ui) {
    switch (state)
    {
        case 0:
            //�������Cacheû��������ݣ���Cache���ڴ���ȡ���ݣ�Cache line״̬���E��
            //local l1 read miss remote l1 miss l2 miss 
            if (level==0&&(core0_l1_hit_index < 0) && (hit_index_l2 < 0) && (core1_l1_hit_index < 0)) {
                state = 1;    //mesi state  0  I  1  E   2  S  3  M 
                // ������ͬʱ���ڴ��ȡ���ݵ�l2/l1��cpu
                cache_miss_count[level]++;
                cache_miss_count[1]++;
                cache_w_count[level]++;
                cache_w_count[1]++;
                cache_r_memory_count++;
                period = period + 100 + 10;
                dirty_interval_time = dirty_interval_time + 100 + 10;
                // ���ڴ��ȡ���ݵ�l1/cpu
                free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ,ui);
                cpu_mem_write_to_l1((_u64)free_index_l1, addr,ui);
                // ���ڴ��ȡ���ݵ�l2
                free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_READ,ui);
                // ��ʱ����һ���ɾ���
                clean_count++;
                mem_write_to_l2((_u64)free_index_l2, addr,ui);
                caches[1][free_index_l2].flag &= ~CACHE_FLAG_DIRTY;
            }
            //l1 hit  l2 miss 
            else if (level==0&& (core0_l1_hit_index >= 0)) {
                // ��������l1�����ݵ�cpu
            // l1���д���+1
            cache_hit_count[level]++;
            // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
            cache_r_count[level]++;
            // �����l1�����ݵ�cpu������һ��ʱ������
            period = period + 1;
            dirty_interval_time = dirty_interval_time + 1;
            // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
            }
            else if (level == 2 && (core1_l1_hit_index >= 0) ) {
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core1_l1_hit_index].lru_count = tick_count;
                }
            }
            else if (level == 2 && (core0_l1_hit_index < 0) && (hit_index_l2 < 0) && (core1_l1_hit_index < 0)) {
                state = 1;    //mesi state  0  I  1  E   2  S  3  M 
                // ������ͬʱ���ڴ��ȡ���ݵ�l2/l1��cpu
                cache_miss_count[level]++;
                cache_miss_count[1]++;
                cache_w_count[level]++;
                cache_w_count[1]++;
                cache_r_memory_count++;
                period = period + 100 + 10;
                dirty_interval_time = dirty_interval_time + 100 + 10;
                // ���ڴ��ȡ���ݵ�l1/cpu
                free_index_l1_core1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ,ui);
                cpu_mem_write_to_l1((_u64)free_index_l1_core1, addr,ui);
                // ���ڴ��ȡ���ݵ�l2
                free_index_l2 = get_cache_free_line(set_base_l1_core1, 1, OPERATION_READ,ui);
                // ��ʱ����һ���ɾ���
                clean_count++;
                mem_write_to_l2((_u64)free_index_l2, addr,ui);
                caches[1][free_index_l2].flag &= ~CACHE_FLAG_DIRTY;
            }
            //�������Cache��������ݣ���״̬ΪM�������ݸ��µ��ڴ棬��Cache�ٴ��ڴ���ȡ���ݣ�2��Cache ��Cache line״̬�����S��
            //local l1 read miss  remote l1 read hit m  l2 read hit m   ->s
            else if (level==0 && (core1_l1_hit_index>=0)&&(hit_index_l2>=0)&&(caches[1]+hit_index_l2)->state==3&&(caches[2]+core1_l1_hit_index)->state==3) {
                (caches[1] + hit_index_l2)->state = 2;
                (caches[2] + core1_l1_hit_index)->state = 2;
                state = 2;
                (caches[2] + core1_l1_hit_index)->flag &= 0xfd;

                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ,ui);
                l2_write_to_mem(hit_index_l2,ui);
                //l2_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2, addr,ui);
                cpu_mem_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2,ui);
                //l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }
            }
            else if (level == 2 && (core0_l1_hit_index >=0) && (hit_index_l2 >= 0) && (caches[1] + hit_index_l2)->state == 3 && (caches[0] + core1_l1_hit_index)->state == 3) {
                (caches[1] + hit_index_l2)->state = 2;
                (caches[0] + core0_l1_hit_index)->state = 2;
                (caches[0] + core0_l1_hit_index)->flag &= 0xfd;
                state = 2;

                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1_core1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ,ui);
                l2_write_to_mem(hit_index_l2,ui);
                //l2_write_to_l1((_u64)free_index_l1_core1, (_u64)hit_index_l2, addr,ui);
                cpu_mem_write_to_l1((_u64)free_index_l1_core1, (_u64)hit_index_l2,ui);
                //l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }
            }
            //local l1 read miss remote l1 read miss l2 read hit 
            else if (level == 0 && hit_index_l2 >= 0 && core0_l1_hit_index < 0 && core1_l1_hit_index < 0) {
                state = 1;
                (caches[1] + hit_index_l2)->state = 1;
                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ,ui);
                l2_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2, addr,ui);
                l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }

            }
            else if (level == 2 && hit_index_l2 >= 0 && core0_l1_hit_index < 0 && core1_l1_hit_index < 0) {
                state = 1;
                (caches[1] + hit_index_l2)->state = 1;
                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1_core1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ,ui);
                l2_write_to_l1((_u64)free_index_l1_core1, (_u64)hit_index_l2, addr,ui);
                l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }

            }
            //�������Cache��������ݣ���״̬ΪS����E����Cache���ڴ���ȡ���ݣ���ЩCache ��Cache line״̬�����S
            //local l1 read miss remote l1 read hit e l2 read hit e    ->s
            else if (level == 0 && (core1_l1_hit_index >= 0) && (hit_index_l2 >= 0) && (((caches[1] + hit_index_l2)->state == 1 && (caches[2] + core1_l1_hit_index)->state == 1)|| ((caches[1] + hit_index_l2)->state == 2 && (caches[2] + core1_l1_hit_index)->state == 2))) {
                (caches[1] + hit_index_l2)->state = 2;
                (caches[2] + core1_l1_hit_index)->state = 2;
                state = 2;

                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_READ,ui);
                l2_write_to_l1((_u64)free_index_l1, (_u64)hit_index_l2, addr,ui);
                l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }
            }
            else if (level == 2 && (core0_l1_hit_index > 0) && (hit_index_l2 >= 0) && (caches[1] + hit_index_l2)->state == 1 && (caches[0] + core1_l1_hit_index)->state == 1) {
                (caches[1] + hit_index_l2)->state = 2;
                (caches[0] + core1_l1_hit_index)->state = 2;
                state = 2;

                // ������ͬʱ��l2��ȡ���ݵ�l1��cpu
                cache_miss_count[level]++;
                cache_hit_count[1]++;
                cache_r_count[1]++;
                cache_w_count[level]++;
                period = period + 10 + 1;
                dirty_interval_time = dirty_interval_time + 10 + 1;
                free_index_l1_core1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_READ,ui);
                l2_write_to_l1((_u64)free_index_l1_core1, (_u64)hit_index_l2, addr,ui);
                l2_write_to_cpu((_u64)hit_index_l2,ui);
                // ֻҪ�����ˣ����޸�����ķ���ʱ��
                if (CACHE_SWAP_LRU == swap_style[1]) {
                    caches[1][hit_index_l2].lru_count = tick_count;
                }
            }
            break;
        case 1 :
            //��Cache��ȡ���ݣ�״̬����
            //local l1 hit local l2 hit
            if (level == 0&&(core0_l1_hit_index>=0) && (hit_index_l2>=0)) {
                (caches[1] + hit_index_l2)->state = 1;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
            }
            else if (level == 0 && (core0_l1_hit_index >= 0)&&(hit_index_l2<0)) {
                (caches[1] + hit_index_l2)->state = 1;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
                free_index_l2=get_cache_free_line(set_base_l2, 1, swap_style[1],ui);
                l1_cpu_write_to_l2(free_index_l2,addr,ui);
            }
            else if (level == 2 && (core1_l1_hit_index >= 0) && (hit_index_l2 >= 0)) {
                (caches[1] + hit_index_l2)->state = 1;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core1_l1_hit_index].lru_count = tick_count;
                }
                else if (level == 2 && (core1_l1_hit_index >= 0) && (hit_index_l2 < 0)) {
                    (caches[1] + hit_index_l2)->state = 1;
                    // ��������l1�����ݵ�cpu
                    // l1���д���+1
                    cache_hit_count[level]++;
                    // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                    cache_r_count[level]++;
                    // �����l1�����ݵ�cpu������һ��ʱ������
                    period = period + 1;
                    dirty_interval_time = dirty_interval_time + 1;
                    // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                    if (CACHE_SWAP_LRU == swap_style[level]) {
                        caches[level][core1_l1_hit_index].lru_count = tick_count;
                    }
                    free_index_l2 = get_cache_free_line(set_base_l2, 1, swap_style[1],ui);
                    l1_cpu_write_to_l2(free_index_l2, addr,ui);
                }
            
            }
               break;
        case 2 :
            //��Cache��ȡ���ݣ�״̬����
            //local l1 hit local l2 hit
            if (level == 0 && (core0_l1_hit_index >= 0) && (hit_index_l2 >= 0)) {
                (caches[1] + hit_index_l2)->state = 2;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
            }
            else if (level == 2 && (core1_l1_hit_index >= 0) && (hit_index_l2 >= 0)) {
                (caches[1] + hit_index_l2)->state = 2;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
            }
               break;
        case 3:
            //��Cache��ȡ���ݣ�״̬����
            //local l1 hit local l2 hit
            if (level == 0 && (core0_l1_hit_index >= 0) && (hit_index_l2 >= 0)) {
                (caches[1] + hit_index_l2)->state = 3;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
            }
            else if (level == 2 && (core1_l1_hit_index >= 0) && (hit_index_l2 >= 0)) {
                (caches[1] + hit_index_l2)->state = 3;
                // ��������l1�����ݵ�cpu
                // l1���д���+1
                cache_hit_count[level]++;
                // ��l1�����ݵ�cpu����l1�Ķ�ȡ����+1
                cache_r_count[level]++;
                // �����l1�����ݵ�cpu������һ��ʱ������
                period = period + 1;
                dirty_interval_time = dirty_interval_time + 1;
                // ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
                if (CACHE_SWAP_LRU == swap_style[level]) {
                    caches[level][core0_l1_hit_index].lru_count = tick_count;
                }
            }
              break;
    default:
        break;
    }
}
void CacheSim::local_write(int& state, int level, long long core0_l1_hit_index, long long hit_index_l2, long long core1_l1_hit_index,_u64 set_base_l1_core1,_u64 set_base_l2,_u64 set_base_l1,long long free_index_l2, long long free_index_l1_core1, long long free_index_l1,_u64 addr, Ui::MainWindow* ui) {
    switch (state)

    {
    case 0:
        //���ڴ���ȡ���ݣ���Cache���޸ģ�״̬���M��
        // local write miss l2 write miss
        if (level == 0 && (core0_l1_hit_index < 0) && (hit_index_l2 < 0)) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ������cpuͬʱ������д��l1��l2��������l2����λΪ1
            cache_miss_count[level]++;
            cache_miss_count[1]++;
            cache_w_count[level]++;
            cache_w_count[1]++;
            period = period + 10;
            dirty_interval_time = dirty_interval_time + 10;
            free_index_l1 = get_cache_free_line(set_base_l1, 0, OPERATION_WRITE,ui);
            cpu_mem_write_to_l1((_u64)free_index_l1, addr,ui);
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
        }
        else if (level == 2 && (core1_l1_hit_index < 0) && (hit_index_l2 < 0)) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ������cpuͬʱ������д��l1��l2��������l2����λΪ1
            cache_miss_count[level]++;
            cache_miss_count[1]++;
            cache_w_count[level]++;
            cache_w_count[1]++;
            period = period + 10;
            dirty_interval_time = dirty_interval_time + 10;
            free_index_l1_core1 = get_cache_free_line(set_base_l1_core1, 0, OPERATION_WRITE,ui);
            cpu_mem_write_to_l1((_u64)free_index_l1_core1, addr,ui);
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
        }
        //�������Cache��������ݣ���״̬ΪM����Ҫ�Ƚ����ݸ��µ��ڴ棻�������Cache��������ݣ�������Cache��Cache line״̬���I
        //local write miss l2 write hit  remote l1 miss
        else if (level==0&&hit_index_l2>0&&core0_l1_hit_index<0) {
            if (core1_l1_hit_index >= 0) {
                (caches[2] + core1_l1_hit_index)->state = 0;
            }
            //�������Cache��������ݣ���״̬ΪM ��д���ڴ�
            if((caches[1]+hit_index_l2)->state==3)
            l2_write_to_mem(hit_index_l2,ui);

            state = 3;    //mesi state  0  I  1  E   2  S  3  M 
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2��λΪ1
            cache_miss_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level == 2 && hit_index_l2 > 0 &&core1_l1_hit_index < 0) {
            if (core0_l1_hit_index >= 0) {
                (caches[0] + core0_l1_hit_index)->state = 0;
            }
            //�������Cache��������ݣ���״̬ΪM ��д���ڴ�
            if ((caches[1] + hit_index_l2)->state == 3)
                l2_write_to_mem(hit_index_l2,ui);
            state = 3;    //mesi state  0  I  1  E   2  S  3  M 
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2��λΪ1
            cache_miss_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        //�������Cache��������ݣ�������Cache��Cache line״̬���I
        else if (level == 0 && (core0_l1_hit_index < 0) && (hit_index_l2 > 0)&&core1_l1_hit_index>0) {
            state = 3;
            (caches[2] + core1_l1_hit_index)->state = 0;
            (caches[1] + hit_index_l2)->state = 3;
            (caches[2] + core1_l1_hit_index)->flag &= 0xfe;
            // ����������l2��λΪ1
            cache_miss_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }

        }
        else if (level == 2 && (core1_l1_hit_index < 0) && (hit_index_l2 > 0) && core0_l1_hit_index > 0) {
            state = 3;
            (caches[2] + core1_l1_hit_index)->state = 0;
            (caches[1] + hit_index_l2)->state = 3;
            (caches[0] + core0_l1_hit_index)->flag &= 0xfe;
            // ����������l2��λΪ1
            cache_miss_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }

        }
        break;
    case 1:
        //��Cache��ȡ���ݣ�״̬����
        //�޸�Cache�е����ݣ�״̬���M
        //local write hit local l2 write hit remote miss
        if (level==0&&core0_l1_hit_index>0&&hit_index_l2>0&&core1_l1_hit_index<0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level == 2 && core0_l1_hit_index < 0 && hit_index_l2 > 0 && core1_l1_hit_index > 0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core1_l1_hit_index].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level==0&&core0_l1_hit_index>=0&&hit_index_l2<0) {
            // �����������ݴ�l1д��l2����������l2Ϊ����
            cache_hit_count[level]++;
            cache_miss_count[1]++;
            cache_r_count[level]++;
            cache_w_count[1]++;
            period = period + 1 + 10;
            dirty_interval_time = dirty_interval_time + 1 + 10;
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
        }
        else if (level == 2 && core1_l1_hit_index >= 0 && hit_index_l2 < 0) {
            // �����������ݴ�l1д��l2����������l2Ϊ����
            cache_hit_count[level]++;
            cache_miss_count[1]++;
            cache_r_count[level]++;
            cache_w_count[1]++;
            period = period + 1 + 10;
            dirty_interval_time = dirty_interval_time + 1 + 10;
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core1_l1_hit_index].lru_count = tick_count;
            }
        }
        break;
    case 2:
        //�޸�Cache�е����ݣ�״̬���M��
        //�����˹����Cache line״̬���I
        //local write hit 
        if (level==0&&hit_index_l2>=0&&core0_l1_hit_index>=0&&core1_l1_hit_index>=0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            (caches[2] + core1_l1_hit_index)->state = 0;
            (caches[2] + core1_l1_hit_index)->flag &= 0xfe;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level == 2 && hit_index_l2 > 0 && core0_l1_hit_index > 0 && core1_l1_hit_index > 0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            (caches[0] + core0_l1_hit_index)->state = 0;
            (caches[0] + core0_l1_hit_index)->flag &= 0xfe;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core1_l1_hit_index].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level == 0 && core0_l1_hit_index >= 0 && hit_index_l2 < 0) {
            // �����������ݴ�l1д��l2����������l2Ϊ����
            cache_hit_count[level]++;
            cache_miss_count[1]++;
            cache_r_count[level]++;
            cache_w_count[1]++;
            period = period + 1 + 10;
            dirty_interval_time = dirty_interval_time + 1 + 10;
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
        }
        else if (level == 2 && core1_l1_hit_index >= 0 && hit_index_l2 < 0) {
            // �����������ݴ�l1д��l2����������l2Ϊ����
            cache_hit_count[level]++;
            cache_miss_count[1]++;
            cache_r_count[level]++;
            cache_w_count[1]++;
            period = period + 1 + 10;
            dirty_interval_time = dirty_interval_time + 1 + 10;
            free_index_l2 = get_cache_free_line(set_base_l2, 1, OPERATION_WRITE,ui);
            // ��Ҫ���б���
            l1_cpu_write_to_l2((_u64)free_index_l2, addr,ui);
            if (!(caches[1][free_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core1_l1_hit_index].lru_count = tick_count;
            }
        }
        break;
    case 3:
        //�޸�Cache�е����ݣ�״̬����
        if (level == 0 && hit_index_l2 > 0 && core0_l1_hit_index > 0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[level]) {
                caches[level][core0_l1_hit_index].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        else if (level == 2 && hit_index_l2 > 0 && core1_l1_hit_index > 0) {
            state = 3;
            (caches[1] + hit_index_l2)->state = 3;
            // ����������l2Ϊ���У����֮ǰ�Ǹɾ��еĻ���
            cache_hit_count[level]++;
            cache_hit_count[1]++;
            if (!(caches[1][hit_index_l2].flag & CACHE_FLAG_DIRTY)) {
                dirty_count++;
                caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
                // ������Ҫ����
                if (!START_MLREPS_FLAG) {
                    if (SECDED == 1) hamming_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                    if (SECDED == 2) secded_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
                else {
                    if (MLREPS == 1) mlreps_encode_for_cache(hit_index_l2, caches[1][hit_index_l2].buf);
                }
            }
            if (CACHE_SWAP_LRU == swap_style[hit_index_l2]) {
                caches[hit_index_l2][hit_index_l2].lru_count = tick_count;
            }
            if (CACHE_SWAP_LRU == swap_style[1]) {
                caches[1][hit_index_l2].lru_count = tick_count;
            }
        }
        break;
    default:
        break;
    }
}
void CacheSim::remote_read(int state) {
}
void CacheSim::remote_write(int state) {

}
void CacheSim::fun1(char* testcase,int core_num, Ui::MainWindow* ui) {
    load_trace(testcase,core_num,ui);
}
