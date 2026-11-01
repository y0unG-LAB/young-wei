/* 线性表一致性测试套件
 *
 * 同一份测试代码，分别链接 seqlist.c 和 linkedlist.c 各编译一次：
 *     -DUSE_SEQLIST      顺序表
 *     -DUSE_LINKEDLIST   单链表
 * 只要两个二进制都全绿，就说明两种实现满足同一个"契约"。
 * 这是 C 语言里做"接口一致性"检验的常用土办法：用宏把类型和函数名换掉。 */

#include <stdio.h>
#include <string.h>

#if !defined(USE_SEQLIST) && !defined(USE_LINKEDLIST)
#define USE_SEQLIST 1
#endif

#if defined(USE_SEQLIST)
#include "seqlist.h"
#define LIST_T            SeqList
#define list_create       seq_create
#define list_destroy      seq_destroy
#define list_size         seq_size
#define list_insert       seq_insert
#define list_push_back    seq_push_back
#define list_remove       seq_remove
#define list_at           seq_at
#define list_find         seq_find
#define list_reverse      seq_reverse
#define list_merge_sorted seq_merge_sorted
#define list_save         seq_save
#define list_load         seq_load
#define IMPL_LABEL        "SeqList (dynamic array)"
#else
#include "linkedlist.h"
#define LIST_T            LinkedTaskList
#define list_create       llist_create
#define list_destroy      llist_destroy
#define list_size         llist_size
#define list_insert       llist_insert
#define list_push_back    llist_push_back
#define list_remove       llist_remove
#define list_at           llist_at
#define list_find         llist_find
#define list_reverse      llist_reverse
#define list_merge_sorted llist_merge_sorted
#define list_save         llist_save
#define list_load         llist_load
#define IMPL_LABEL        "LinkedTaskList (singly linked)"
#endif

#include "test_util.h"

/* ---------------- 期望值表 ---------------- */

static const char *const T1_ONLY[] = { "only" };
static const char *const T1_C[]    = { "c" };
static const char *const T1_Z[]    = { "z" };
static const char *const T2_AB[]   = { "a", "b" };
static const char *const T2_AC[]   = { "a", "c" };
static const char *const T3_BASIC[] = { "读书", "写代码", "跑步" };
static const char *const T3_ACD[]  = { "a", "c", "d" };
static const char *const T4_ABCD[] = { "a", "b", "c", "d" };
static const char *const T4_DCBA[] = { "d", "c", "b", "a" };
static const char *const T5_ABCDE[] = { "a", "b", "c", "d", "e" };
static const char *const T5_EDCBA[] = { "e", "d", "c", "b", "a" };
static const char *const M4_SORTED[] = { "a1", "a2", "a3", "a4" };
static const char *const M2_SORTED[] = { "a1", "a3" };
static const char *const M2_TIE[]    = { "y", "x" };

/* ---------------- 辅助函数 ---------------- */

/* 断言"空表"：size 为 0 且 at(0) 取不到东西。
 * 每一步都先判 NULL 再解引用，这样即使实现还是空的也不会段错误，
 * 而是干净地报 FAIL —— 测试崩掉比测试失败难查得多。 */
static void expect_blank(const LIST_T *l)
{
    CHECK_INT(list_size(l), 0);
    CHECK(list_at(l, 0) == NULL);
}

static void expect_titles(const LIST_T *l, const char *const *want, int n)
{
    int i;

    CHECK_INT(list_size(l), n);
    for (i = 0; i < n; i++) {
        const Task *t = list_at(l, i);
        CHECK(t != NULL);
        if (t == NULL) {
            return;
        }
        CHECK_STR(t->title, want[i]);
    }
}

/* 从头到尾造一条表，用来给后面的用例当底料。 */
static LIST_T *build(const char *const *titles, int n, int id_base)
{
    LIST_T *l = list_create();
    int i;

    if (l == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        if (list_push_back(l, task_make(id_base + i, 1, titles[i])) != 0) {
            list_destroy(l);
            return NULL;
        }
    }
    return l;
}

/* ---------------- 用例 1：空表 ---------------- */

static void t_empty(void)
{
    LIST_T *l = list_create();

    TC_BEGIN("empty list: size / at / remove / reverse");
    CHECK(l != NULL);
    if (l != NULL) {
        expect_blank(l);
        CHECK_INT(list_remove(l, 0), -1);    /* 空表删不了 */
        CHECK_INT(list_remove(l, -1), -1);
        CHECK_INT(list_reverse(l), 0);       /* 空表逆序是合法操作，不算错 */
        expect_blank(l);
        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 2：尾插 ---------------- */

static void t_push_back(void)
{
    LIST_T *l = list_create();
    const Task *t;

    TC_BEGIN("push_back + size + at boundaries");
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_push_back(l, task_make(1, 1, "读书")), 0);
        CHECK_INT(list_push_back(l, task_make(2, 2, "写代码")), 0);
        CHECK_INT(list_push_back(l, task_make(3, 3, "跑步")), 0);

        CHECK_INT(list_size(l), 3);
        expect_titles(l, T3_BASIC, 3);

        t = list_at(l, 2);
        CHECK(t != NULL);
        if (t != NULL) {
            CHECK_INT(t->id, 3);
            CHECK_INT(t->priority, 3);
        }

        CHECK(list_at(l, 3) == NULL);        /* 上越界 */
        CHECK(list_at(l, -1) == NULL);       /* 下越界 */

        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 3：三个位置插入 ---------------- */

static void t_insert_positions(void)
{
    LIST_T *l = list_create();

    TC_BEGIN("insert at head / middle / tail");
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_insert(l, 0, task_make(1, 1, "b")), 0);
        CHECK_INT(list_insert(l, 0, task_make(2, 1, "a")), 0);   /* 头插 */
        expect_titles(l, T2_AB, 2);

        CHECK_INT(list_insert(l, 2, task_make(3, 1, "d")), 0);   /* 尾插：pos == size */
        CHECK_INT(list_insert(l, 2, task_make(4, 1, "c")), 0);   /* 中间 */
        expect_titles(l, T4_ABCD, 4);

        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 4：非法位置插入 ---------------- */

static void t_insert_invalid(void)
{
    LIST_T *l;

    TC_BEGIN("insert at illegal pos: reject and keep intact");
    l = build(T2_AB, 2, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_insert(l, -1, task_make(9, 1, "x")), -1);
        CHECK_INT(list_insert(l, 3, task_make(9, 1, "x")), -1);  /* size + 1 */

        /* 关键：拒绝之后表必须原样不动，不能"插了一半"。 */
        CHECK_INT(list_size(l), 2);
        expect_titles(l, T2_AB, 2);
        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 5：三个位置删除 ---------------- */

static void t_remove_positions(void)
{
    LIST_T *l;

    TC_BEGIN("remove at middle / tail / head");
    l = build(T4_ABCD, 4, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_remove(l, 1), 0);     /* 删 b */
        expect_titles(l, T3_ACD, 3);

        CHECK_INT(list_remove(l, 2), 0);     /* 删尾 */
        expect_titles(l, T2_AC, 2);

        CHECK_INT(list_remove(l, 0), 0);     /* 删头 */
        expect_titles(l, T1_C, 1);

        CHECK_INT(list_remove(l, 0), 0);     /* 删到空 */
        expect_blank(l);

        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 6：非法位置删除 ---------------- */

static void t_remove_invalid(void)
{
    LIST_T *l;

    TC_BEGIN("remove at illegal pos: reject and keep intact");
    l = build(T2_AB, 2, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_remove(l, -1), -1);
        CHECK_INT(list_remove(l, 2), -1);
        CHECK_INT(list_size(l), 2);
        expect_titles(l, T2_AB, 2);
        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 7：查找 ---------------- */

static void t_find(void)
{
    static const char *const titles[] = { "读书", "写代码", "读文档", "写报告" };
    LIST_T *l;
    int pos[8];
    int n;

    TC_BEGIN("find: multi-hit / none / truncation / count-only");
    l = build(titles, 4, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        n = list_find(l, "写", pos, 8);
        CHECK_INT(n, 2);
        if (n == 2) {
            CHECK_INT(pos[0], 1);            /* 写代码 */
            CHECK_INT(pos[1], 3);            /* 写报告 */
        }

        n = list_find(l, "读", pos, 8);
        CHECK_INT(n, 2);
        if (n == 2) {
            CHECK_INT(pos[0], 0);
            CHECK_INT(pos[1], 2);
        }

        CHECK_INT(list_find(l, "zzz", pos, 8), 0);

        /* 缓冲区只给 1 个：总数要如实返回 2，但只写 1 个下标 */
        n = list_find(l, "写", pos, 1);
        CHECK_INT(n, 2);
        CHECK_INT(pos[0], 1);

        /* out 传 NULL：只统计，不写 */
        CHECK_INT(list_find(l, "读", NULL, 0), 2);

        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 8：逆序 ---------------- */

static void t_reverse(void)
{
    LIST_T *l;

    TC_BEGIN("reverse: empty / single / even / odd");
    l = list_create();
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_reverse(l), 0);       /* 空表 */
        expect_blank(l);
        list_destroy(l);
    }

    l = build(T1_ONLY, 1, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_reverse(l), 0);       /* 单元素 */
        expect_titles(l, T1_ONLY, 1);
        list_destroy(l);
    }

    l = build(T4_ABCD, 4, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_reverse(l), 0);       /* 偶数个 */
        expect_titles(l, T4_DCBA, 4);
        list_destroy(l);
    }

    l = build(T5_ABCDE, 5, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_reverse(l), 0);       /* 奇数个 */
        expect_titles(l, T5_EDCBA, 5);
        list_destroy(l);
    }
    TC_END();
}

/* ---------------- 用例 9：归并两个有序表 ---------------- */

static void t_merge(void)
{
    static const char *const a_titles[] = { "a1", "a3" };
    static const char *const b_titles[] = { "a2", "a4" };
    LIST_T *a;
    LIST_T *b;
    LIST_T *dst;
    LIST_T *empty;

    TC_BEGIN("merge_sorted: discard dst / empty / one side / tie");
    a = list_create();
    b = list_create();
    dst = list_create();
    empty = list_create();
    CHECK(a != NULL && b != NULL && dst != NULL && empty != NULL);

    if (a != NULL && b != NULL && dst != NULL && empty != NULL) {
        /* a = 优先级 1,3 ；b = 优先级 2,4（priority 和 id 都等于序号，方便看） */
        list_push_back(a, task_make(1, 1, "a1"));
        list_push_back(a, task_make(3, 3, "a3"));
        list_push_back(b, task_make(2, 2, "a2"));
        list_push_back(b, task_make(4, 4, "a4"));

        /* dst 里先塞垃圾，归并后必须被丢弃 */
        list_push_back(dst, task_make(99, 9, "junk"));
        CHECK_INT(list_merge_sorted(dst, a, b), 0);
        expect_titles(dst, M4_SORTED, 4);

        /* 再归并一次（dst 已非空），结果应该完全一样，不能变成 8 条 */
        CHECK_INT(list_merge_sorted(dst, a, b), 0);
        expect_titles(dst, M4_SORTED, 4);

        /* 两边都空 */
        CHECK_INT(list_merge_sorted(dst, empty, empty), 0);
        expect_blank(dst);

        /* 一边空 */
        CHECK_INT(list_merge_sorted(dst, a, empty), 0);
        expect_titles(dst, M2_SORTED, 2);
    }

    /* 优先级相同 -> 按 id 升序：y(id=5) 在 x(id=7) 前面 */
    if (a != NULL && b != NULL && dst != NULL) {
        list_destroy(a);
        list_destroy(b);
        a = list_create();
        b = list_create();
        if (a != NULL && b != NULL) {
            list_push_back(a, task_make(7, 1, "x"));
            list_push_back(b, task_make(5, 1, "y"));
            CHECK_INT(list_merge_sorted(dst, a, b), 0);
            expect_titles(dst, M2_TIE, 2);
        }
    }

    if (a != NULL)     list_destroy(a);
    if (b != NULL)     list_destroy(b);
    if (dst != NULL)   list_destroy(dst);
    if (empty != NULL) list_destroy(empty);
    TC_END();
}

/* ---------------- 用例 10：存盘 / 读盘往返 ---------------- */

static void t_roundtrip(void)
{
    const char *path = "test_roundtrip.tmp";
    LIST_T *l;
    LIST_T *m;
    const Task *t;
    int i;

    TC_BEGIN("save / load roundtrip (same process)");
    l = build(T3_BASIC, 3, 1);
    CHECK(l != NULL);
    if (l != NULL) {
        CHECK_INT(list_save(l, path), 0);

        m = list_create();
        CHECK(m != NULL);
        if (m != NULL) {
            CHECK_INT(list_load(m, path), 0);
            CHECK_INT(list_size(m), 3);
            for (i = 0; i < 3 && i < list_size(m); i++) {
                t = list_at(m, i);
                CHECK(t != NULL);
                if (t != NULL) {
                    CHECK_STR(t->title, T3_BASIC[i]);
                    CHECK_INT(t->id, i + 1);
                    CHECK_INT(t->priority, 1);
                }
            }

            /* 再 load 一次：语义是"先清空再装入"，不能追加成 6 条 */
            CHECK_INT(list_load(m, path), 0);
            CHECK_INT(list_size(m), 3);

            /* 读一个不存在的文件要失败，而不是崩溃 */
            CHECK_INT(list_load(m, "no_such_file_xyz.tmp"), -1);

            list_destroy(m);
        }
        list_destroy(l);
        remove(path);
    }
    TC_END();
}

/* ---------------- 用例 11：压力 + 不变量 ---------------- */

#define STRESS_N 2000

static void t_stress(void)
{
    LIST_T *l;
    int i;

    TC_BEGIN("stress: 2000 push / head-insert / drain / reuse");
    l = list_create();
    CHECK(l != NULL);
    if (l == NULL) {
        TC_END();
        return;
    }

    for (i = 0; i < STRESS_N; i++) {
        if (list_push_back(l, task_make(i + 1, (i % 3) + 1, "任务")) != 0) {
            break;
        }
    }
    CHECK_INT(list_size(l), STRESS_N);

    /* 反复头插：检验头插和 size 的一致性 */
    for (i = 0; i < 100; i++) {
        if (list_insert(l, 0, task_make(10000 + i, 1, "头插")) != 0) {
            break;
        }
    }
    CHECK_INT(list_size(l), STRESS_N + 100);
    CHECK_INT(list_remove(l, 0), 0);
    CHECK_INT(list_size(l), STRESS_N + 99);

    /* 一路删到空 —— 链表在这里最容易出"尾指针没更新"的错，
     * 删掉最后一个之后 tail 应该回到哨兵（head）。 */
    while (list_size(l) > 0) {
        if (list_remove(l, 0) != 0) {
            break;
        }
    }
    expect_blank(l);

    /* 清空后再插一条：如果 tail 还指着已释放的结点，这一步就会写坏内存 */
    CHECK_INT(list_push_back(l, task_make(1, 1, "z")), 0);
    expect_titles(l, T1_Z, 1);
    CHECK_INT(list_insert(l, 0, task_make(2, 1, "y")), 0);
    CHECK_INT(list_size(l), 2);
    CHECK_STR(list_at(l, 0) != NULL ? list_at(l, 0)->title : "(null)", "y");

    list_destroy(l);
    TC_END();
}

/* ---------------- main ---------------- */

int main(void)
{
    printf("\n===== linear list contract suite : %s =====\n\n", IMPL_LABEL);

    t_empty();
    t_push_back();
    t_insert_positions();
    t_insert_invalid();
    t_remove_positions();
    t_remove_invalid();
    t_find();
    t_reverse();
    t_merge();
    t_roundtrip();
    t_stress();

    return t_summary();
}
