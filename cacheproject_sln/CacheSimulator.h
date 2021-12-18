#ifndef CACHE_SIMULATOR
#define CACHE_SIMULATOR

#include "mainwindow.h"
#include "Common.h"
#include "Cache_Line.h"
#include <string>
using namespace std;

/*
// 0表示不使用奇偶校验，1表示使用普通奇偶校验，2表示使用改进奇偶校验
# define PARITY_CHECK           2
// 0表示不使用Hamming或者SEC-DED，1表示使用Hamming code，2表示使用SEC-DED code
# define SECDED                 2
// 0表示不开启早期回写机制，1表示开启早期回写机制
# define EARLY_WRITE_BACK       1
// 0表示不开启紧急回写机制，1表示开启紧急回写机制
# define EMERGENCY_WRITE_BACK   1

// 0表示不使用MLREPS，1表示使用MLREPS，后续要更改
# define MLREPS                 1

// 0表示不启用故障注入，1表示启用故障注入
# define INJECT_STATE           1

*/

// 将某一位数取反
#define reversebit(x,y)  x^=(1<<y)


/* 设置多级Cache，此处为最多多少级Cache */
const int MAX_LEVEL = 3;

/* 设置read/write标识 */
const char OPERATION_READ = 'l';
const char OPERATION_WRITE = 's';

/* 替换算法 */
enum cache_swap_style {
    CACHE_SWAP_FIFO,    // 先进先出替换
    CACHE_SWAP_LRU,     // 最近最久未使用替换
    CACHE_SWAP_RAND,    // 随机替换
    CACHE_SWAP_MAX      // 不是某种替换算法，表示替换方法的数量
};

/* 创建Cache_Sim类 */
class CacheSim : public MainWindow {
public:
    _u64 cache_size[MAX_LEVEL];          // Cache的总大小，单位byte
    _u64 cache_line_size[MAX_LEVEL];     // 一个Cache line的大小，单位byte
    _u64 cache_line_num[MAX_LEVEL];      // Cache一共有多少个line
    _u64 cache_word_size[MAX_LEVEL];     // Cache字大小
    _u64 cache_word_num[MAX_LEVEL];      // 一个Cache行有几个字
    _u64 cache_mapping_ways[MAX_LEVEL];  // 几路组相联
    _u64 cache_set_size[MAX_LEVEL];      // 整个Cache有多少组
    _u64 cache_set_shifts[MAX_LEVEL];    // 在内存地址划分中占的位数，log2(cache_set_size)
    _u64 cache_line_shifts[MAX_LEVEL];   // 在内存地址划分中占的位数，log2(cache_line_num)
    Cache_Line* caches[MAX_LEVEL];       // 真正的Cache地址列，指针数组
    _u64* cache_buf[MAX_LEVEL];          // cache缓冲区，用来存储Cache行数据
    _u64 tick_count;                     // 指令计数器
    int write_style[MAX_LEVEL];          // 写操作
    int swap_style[MAX_LEVEL];           // 缓存替换算法
    _u64 cache_r_count[MAX_LEVEL], cache_w_count[MAX_LEVEL];          // 读写Cache的计数
    _u64 cache_w_memory_count;           // 实际写内存的计数，Cache—>Memory
    _u64 cache_r_memory_count;           // 实际读内存的计数，Memory—>L2
    _u64 cache_hit_count[MAX_LEVEL], cache_miss_count[MAX_LEVEL];     // Cache hit和miss的计数
    _u64 cache_free_num[MAX_LEVEL];      // 空闲Cache line的index记录，在寻找时，返回空闲line的index
    _u64 clean_count;                    // 干净行数目
    _u64 dirty_count;                    // 脏行数目（干净行和脏行针对L2 Cache）
    // L1 : L2 : Mem = 1 : 10 : 100，存取周期，单位时钟周期
    _u64 period;
    _u64 time_period;
    _u64 emergency_period;

    _u64 dirty_interval_time;

    _u64 clean_line_error_count;
    _u64 dirty_line_error_count;
    _u64 dirty_line_error_correct_count;

    _u64 m_data;                        // 全局变量，从该值的地址取值不至于出问题，从局部变量取值有问题

    bool START_MLREPS_FLAG;

    // add para
    int PARITY_CHECK;
    int SECDED;
    int EARLY_WRITE_BACK;
    int EMERGENCY_WRITE_BACK;
    int MLREPS;
    int ERROR_TYPE;
    int INJECT_TIMES;

    // 故障注入成功次数（Cache line无效则无法注入）
    _u64 inject_count;
    // 误纠正次数（错误纠正，包括hamming/sec-ded/mlreps）
    _u64 fail_correct;
    // 未检测到的错误次数
    _u64 error_but_not_detect_for_clean;
    _u64 error_but_not_detect_for_dirty;

    CacheSim();
    // ~CacheSim();

    /* 初始化 */
    void init(_u64 a_cache_size[], _u64 a_cache_line_size[], _u64 a_mapping_ways[], int l1_replace,
        int l2_replace, int a_parity, int a_secded, int a_mlreps, int a_early, int a_emergency,
        int a_error_type, int a_inject_times, Ui::MainWindow* ui);

    /* 检查是否命中 */
    int check_cache_hit(_u64 set_base, _u64 addr, int level, Ui::MainWindow* ui);

    /* 获取Cache当前set中空余的line */
    int get_cache_free_line(_u64 set_base, int level, int style, Ui::MainWindow* ui);

    /* 找到合适的line之后，将数据写入L1 Cache line中 */
    void cpu_mem_write_to_l1(_u64 index, _u64 addr, Ui::MainWindow* ui);

    /* 找到合适的line之后，将数据写入L2 Cache line中，这里需要添加编码操作 */
    void mem_write_to_l2(_u64 index, _u64 addr, Ui::MainWindow* ui);    // 干净数据编码
    void l1_cpu_write_to_l2(_u64 index, _u64 addr, Ui::MainWindow* ui); // 脏数据编码

    /* 将数据从L2写回内存，脏数据被替换，这里需要添加解码操作 */
    void l2_write_to_mem(_u64 l2_index, Ui::MainWindow* ui);

    /* 从l2读取数据到cpu，这里需要添加解码操作 */
    void l2_write_to_cpu(_u64 l2_index, Ui::MainWindow* ui);

    /* 从l2读取数据到l1，需要添加解码操作，以及对l1的填充 */
    void l2_write_to_l1(_u64 l1_index, _u64 l2_index, _u64 addr, Ui::MainWindow* ui);

    /* 对一个指令进行分析 */
    void do_cache_op(_u64 addr, char oper_style,  int core_num, Ui::MainWindow* ui);

    /* 读入trace文件 */
    void load_trace(const char* filename,const int core_num, Ui::MainWindow* ui);

    void re_init(int bits);

    /* 添加干净行校验 */
    int parity_check(_u64 m);
    int* update_parity_check(_u64 m);

    void parity_check_encode_for_cache(_u64 l2_index, _u64 data);
    void parity_check_decode_for_cache(_u64 l2_index, _u64 data, Ui::MainWindow* ui);

    /* 添加脏行校验 */
    _u64 cal(_u64 sz);
    _u64 antiCal(_u64 sz);
    string hamming_encode(string s);
    int hamming_decode(string d, string& s);
    void hamming_encode_for_cache(_u64 index, _u64 data);
    void hamming_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* 修改hamming为SEC-DED */
    string secded_encode(string s);
    int secded_decode(string d, string& s);
    void secded_encode_for_cache(_u64 index, _u64 data);
    void secded_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* MLREPS编码 */
    string mlreps_encode(string s);
    int mlreps_decode(string _d, string& s, Ui::MainWindow* ui);
    void mlreps_encode_for_cache(_u64 index, _u64 data);
    void mlreps_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* 脏行校验辅助函数 */
    string dec_to_bin(_u64 m);
    _u64 bin_to_dec(string bin);

    /* 早期回写 */
    void early_write_back(_u64 time, Ui::MainWindow* ui);

    /* 紧急回写，遇到脏数据发生错误时 */
    // 为了避免一段时间频繁的脏数据发生错误，需设置紧急回写的缓冲周期
    // 缓冲周期：即在一段时间内不再使用紧急回写机制，避免大量的写内存操作
    // 此时，应该将period重新设置为0
    void emergency_write_back(Ui::MainWindow* ui);

    /* 添加故障注入 */
    int* rand_0(int min, int max, int n);
    int* rand_1(int min, int max, int n);
    int* cache_error_inject(_u64 min, _u64 max, _u64 n, int error_type);


    void local_read(int& state, int level, long long core0_l1_hit_index, long long hit_index_l2, long long core1_l1_hit_index, _u64 set_base_l1_core1, _u64 set_base_l2, _u64 set_base_l1, long long free_index_l2, long long free_index_l1_core1, long long free_index_l1, _u64 addr, Ui::MainWindow* ui);
    void local_write(int& state, int level, long long core0_l1_hit_index, long long hit_index_l2, long long core1_l1_hit_index, _u64 set_base_l1_core1, _u64 set_base_l2, _u64 set_base_l1, long long free_index_l2, long long free_index_l1_core1, long long free_index_l1, _u64 addr, Ui::MainWindow* ui);
    void remote_read(int state);
    void remote_write(int state);

    void fun1(char * testcase,int core_num, Ui::MainWindow* ui);
};

#endif
