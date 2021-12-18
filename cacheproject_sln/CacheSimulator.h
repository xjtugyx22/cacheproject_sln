#ifndef CACHE_SIMULATOR
#define CACHE_SIMULATOR

#include "mainwindow.h"
#include "Common.h"
#include "Cache_Line.h"
#include <string>
using namespace std;

/*
// 0��ʾ��ʹ����żУ�飬1��ʾʹ����ͨ��żУ�飬2��ʾʹ�øĽ���żУ��
# define PARITY_CHECK           2
// 0��ʾ��ʹ��Hamming����SEC-DED��1��ʾʹ��Hamming code��2��ʾʹ��SEC-DED code
# define SECDED                 2
// 0��ʾ���������ڻ�д���ƣ�1��ʾ�������ڻ�д����
# define EARLY_WRITE_BACK       1
// 0��ʾ������������д���ƣ�1��ʾ����������д����
# define EMERGENCY_WRITE_BACK   1

// 0��ʾ��ʹ��MLREPS��1��ʾʹ��MLREPS������Ҫ����
# define MLREPS                 1

// 0��ʾ�����ù���ע�룬1��ʾ���ù���ע��
# define INJECT_STATE           1

*/

// ��ĳһλ��ȡ��
#define reversebit(x,y)  x^=(1<<y)


/* ���ö༶Cache���˴�Ϊ�����ټ�Cache */
const int MAX_LEVEL = 3;

/* ����read/write��ʶ */
const char OPERATION_READ = 'l';
const char OPERATION_WRITE = 's';

/* �滻�㷨 */
enum cache_swap_style {
    CACHE_SWAP_FIFO,    // �Ƚ��ȳ��滻
    CACHE_SWAP_LRU,     // ������δʹ���滻
    CACHE_SWAP_RAND,    // ����滻
    CACHE_SWAP_MAX      // ����ĳ���滻�㷨����ʾ�滻����������
};

/* ����Cache_Sim�� */
class CacheSim : public MainWindow {
public:
    _u64 cache_size[MAX_LEVEL];          // Cache���ܴ�С����λbyte
    _u64 cache_line_size[MAX_LEVEL];     // һ��Cache line�Ĵ�С����λbyte
    _u64 cache_line_num[MAX_LEVEL];      // Cacheһ���ж��ٸ�line
    _u64 cache_word_size[MAX_LEVEL];     // Cache�ִ�С
    _u64 cache_word_num[MAX_LEVEL];      // һ��Cache���м�����
    _u64 cache_mapping_ways[MAX_LEVEL];  // ��·������
    _u64 cache_set_size[MAX_LEVEL];      // ����Cache�ж�����
    _u64 cache_set_shifts[MAX_LEVEL];    // ���ڴ��ַ������ռ��λ����log2(cache_set_size)
    _u64 cache_line_shifts[MAX_LEVEL];   // ���ڴ��ַ������ռ��λ����log2(cache_line_num)
    Cache_Line* caches[MAX_LEVEL];       // ������Cache��ַ�У�ָ������
    _u64* cache_buf[MAX_LEVEL];          // cache�������������洢Cache������
    _u64 tick_count;                     // ָ�������
    int write_style[MAX_LEVEL];          // д����
    int swap_style[MAX_LEVEL];           // �����滻�㷨
    _u64 cache_r_count[MAX_LEVEL], cache_w_count[MAX_LEVEL];          // ��дCache�ļ���
    _u64 cache_w_memory_count;           // ʵ��д�ڴ�ļ�����Cache��>Memory
    _u64 cache_r_memory_count;           // ʵ�ʶ��ڴ�ļ�����Memory��>L2
    _u64 cache_hit_count[MAX_LEVEL], cache_miss_count[MAX_LEVEL];     // Cache hit��miss�ļ���
    _u64 cache_free_num[MAX_LEVEL];      // ����Cache line��index��¼����Ѱ��ʱ�����ؿ���line��index
    _u64 clean_count;                    // �ɾ�����Ŀ
    _u64 dirty_count;                    // ������Ŀ���ɾ��к��������L2 Cache��
    // L1 : L2 : Mem = 1 : 10 : 100����ȡ���ڣ���λʱ������
    _u64 period;
    _u64 time_period;
    _u64 emergency_period;

    _u64 dirty_interval_time;

    _u64 clean_line_error_count;
    _u64 dirty_line_error_count;
    _u64 dirty_line_error_correct_count;

    _u64 m_data;                        // ȫ�ֱ������Ӹ�ֵ�ĵ�ַȡֵ�����ڳ����⣬�Ӿֲ�����ȡֵ������

    bool START_MLREPS_FLAG;

    // add para
    int PARITY_CHECK;
    int SECDED;
    int EARLY_WRITE_BACK;
    int EMERGENCY_WRITE_BACK;
    int MLREPS;
    int ERROR_TYPE;
    int INJECT_TIMES;

    // ����ע��ɹ�������Cache line��Ч���޷�ע�룩
    _u64 inject_count;
    // ������������������������hamming/sec-ded/mlreps��
    _u64 fail_correct;
    // δ��⵽�Ĵ������
    _u64 error_but_not_detect_for_clean;
    _u64 error_but_not_detect_for_dirty;

    CacheSim();
    // ~CacheSim();

    /* ��ʼ�� */
    void init(_u64 a_cache_size[], _u64 a_cache_line_size[], _u64 a_mapping_ways[], int l1_replace,
        int l2_replace, int a_parity, int a_secded, int a_mlreps, int a_early, int a_emergency,
        int a_error_type, int a_inject_times, Ui::MainWindow* ui);

    /* ����Ƿ����� */
    int check_cache_hit(_u64 set_base, _u64 addr, int level, Ui::MainWindow* ui);

    /* ��ȡCache��ǰset�п����line */
    int get_cache_free_line(_u64 set_base, int level, int style, Ui::MainWindow* ui);

    /* �ҵ����ʵ�line֮�󣬽�����д��L1 Cache line�� */
    void cpu_mem_write_to_l1(_u64 index, _u64 addr, Ui::MainWindow* ui);

    /* �ҵ����ʵ�line֮�󣬽�����д��L2 Cache line�У�������Ҫ��ӱ������ */
    void mem_write_to_l2(_u64 index, _u64 addr, Ui::MainWindow* ui);    // �ɾ����ݱ���
    void l1_cpu_write_to_l2(_u64 index, _u64 addr, Ui::MainWindow* ui); // �����ݱ���

    /* �����ݴ�L2д���ڴ棬�����ݱ��滻��������Ҫ��ӽ������ */
    void l2_write_to_mem(_u64 l2_index, Ui::MainWindow* ui);

    /* ��l2��ȡ���ݵ�cpu��������Ҫ��ӽ������ */
    void l2_write_to_cpu(_u64 l2_index, Ui::MainWindow* ui);

    /* ��l2��ȡ���ݵ�l1����Ҫ��ӽ���������Լ���l1����� */
    void l2_write_to_l1(_u64 l1_index, _u64 l2_index, _u64 addr, Ui::MainWindow* ui);

    /* ��һ��ָ����з��� */
    void do_cache_op(_u64 addr, char oper_style,  int core_num, Ui::MainWindow* ui);

    /* ����trace�ļ� */
    void load_trace(const char* filename,const int core_num, Ui::MainWindow* ui);

    void re_init(int bits);

    /* ��Ӹɾ���У�� */
    int parity_check(_u64 m);
    int* update_parity_check(_u64 m);

    void parity_check_encode_for_cache(_u64 l2_index, _u64 data);
    void parity_check_decode_for_cache(_u64 l2_index, _u64 data, Ui::MainWindow* ui);

    /* �������У�� */
    _u64 cal(_u64 sz);
    _u64 antiCal(_u64 sz);
    string hamming_encode(string s);
    int hamming_decode(string d, string& s);
    void hamming_encode_for_cache(_u64 index, _u64 data);
    void hamming_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* �޸�hammingΪSEC-DED */
    string secded_encode(string s);
    int secded_decode(string d, string& s);
    void secded_encode_for_cache(_u64 index, _u64 data);
    void secded_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* MLREPS���� */
    string mlreps_encode(string s);
    int mlreps_decode(string _d, string& s, Ui::MainWindow* ui);
    void mlreps_encode_for_cache(_u64 index, _u64 data);
    void mlreps_decode_for_cache(_u64 index, _u64 data, Ui::MainWindow* ui);

    /* ����У�鸨������ */
    string dec_to_bin(_u64 m);
    _u64 bin_to_dec(string bin);

    /* ���ڻ�д */
    void early_write_back(_u64 time, Ui::MainWindow* ui);

    /* ������д�����������ݷ�������ʱ */
    // Ϊ�˱���һ��ʱ��Ƶ���������ݷ������������ý�����д�Ļ�������
    // �������ڣ�����һ��ʱ���ڲ���ʹ�ý�����д���ƣ����������д�ڴ����
    // ��ʱ��Ӧ�ý�period��������Ϊ0
    void emergency_write_back(Ui::MainWindow* ui);

    /* ��ӹ���ע�� */
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
