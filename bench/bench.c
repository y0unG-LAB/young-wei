/* 顺序表 vs 单链表 基准测试
 *
 * 用法：  bench.exe [N]
 *   默认 N = 20000。随机类操作次数是 N/10（至少 500）。
 *
 * 注意：这是"微观基准"，受缓存、分配器、编译优化影响很大。
 * 关键不是绝对毫秒数，而是同一台机器上两种实现的**比例**。 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "seqlist.h"
#include "linkedlist.h"

#ifdef _WIN32
#include <windows.h>
static double now_sec(void)
{
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return (double)cnt.QuadPart / (double)freq.QuadPart;
}
#else
#include <time.h>
static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif

typedef struct {
    const char *name;
    double push_ms;      /* 尾插 n 次 */
    double insert_ms;    /* 随机位置插入 m 次；-1 表示失败 */
    double access_ms;    /* 随机访问 m 次 */
    double remove_ms;    /* 头部删除 m 次 */
} BenchRow;

/* 让内层循环的结果"真的被用到"，否则 -O2 可能把整个循环删掉 */
static volatile long g_sink = 0;

/* ---------- 生成顺序表版本 ---------- */
#define B_FN      seq_run
#define LIST_T    SeqList
#define B_CREATE  seq_create
#define B_DESTROY seq_destroy
#define B_PUSH    seq_push_back
#define B_INSERT  seq_insert
#define B_AT      seq_at
#define B_REMOVE  seq_remove
#define B_SIZE    seq_size
#include "bench_impl.inc"
#undef B_FN
#undef LIST_T
#undef B_CREATE
#undef B_DESTROY
#undef B_PUSH
#undef B_INSERT
#undef B_AT
#undef B_REMOVE
#undef B_SIZE

/* ---------- 生成链表版本 ---------- */
#define B_FN      linked_run
#define LIST_T    LinkedTaskList
#define B_CREATE  llist_create
#define B_DESTROY llist_destroy
#define B_PUSH    llist_push_back
#define B_INSERT  llist_insert
#define B_AT      llist_at
#define B_REMOVE  llist_remove
#define B_SIZE    llist_size
#include "bench_impl.inc"
#undef B_FN
#undef LIST_T
#undef B_CREATE
#undef B_DESTROY
#undef B_PUSH
#undef B_INSERT
#undef B_AT
#undef B_REMOVE
#undef B_SIZE

static void print_ms(double v)
{
    if (v < 0.0) {
        printf("%12s", "n/a");
    } else if (v < 1.0) {
        printf("%12.3f", v);
    } else {
        printf("%12.1f", v);
    }
}

static void print_row(const BenchRow *r)
{
    printf("  %-14s", r->name);
    print_ms(r->push_ms);
    print_ms(r->insert_ms);
    print_ms(r->access_ms);
    print_ms(r->remove_ms);
    printf("\n");
}

int main(int argc, char **argv)
{
    int n = 20000;
    BenchRow rows[2];
    int m;

    if (argc > 1) {
        n = atoi(argv[1]);
    }
    if (n < 1000) {
        n = 1000;
    }
    m = n / 10;
    if (m < 500) {
        m = 500;
    }

    memset(rows, 0, sizeof rows);
    rows[0].name = "SeqList";
    rows[1].name = "LinkedTaskList";

    printf("\n===== linear list benchmark (all times in ms) =====\n");
    printf("  n = %d elements, m = %d random ops\n\n", n, m);
    printf("  %-14s%12s%12s%12s%12s\n", "impl", "push_back", "insert", "at()", "remove(0)");
    printf("  %-14s%12s%12s%12s%12s\n", "--------------", "---------", "------", "-----", "---------");

    seq_run(n, &rows[0]);
    linked_run(n, &rows[1]);

    print_row(&rows[0]);
    print_row(&rows[1]);

    printf("\n  notes:\n");
    printf("    push_back : n times\n");
    printf("    insert    : m times at pseudo-random positions\n");
    printf("    at()      : m random lookups\n");
    printf("    remove(0) : m removals from the front\n");
    printf("    (sink=%ld, ignore)\n\n", g_sink);

    return 0;
}
