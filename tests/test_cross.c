/* 交叉验证：两种实现共用同一种文本格式。
 *
 * 单独一个二进制，因为它必须同时链接 seqlist.c 和 linkedlist.c。
 * 价值：如果哪天你改了其中一边的存盘格式，这个测试会立刻报出来 ——
 * 这就是"契约测试"比"单元测试"更进一步的地方。 */

#include <stdio.h>
#include <string.h>

#include "seqlist.h"
#include "linkedlist.h"
#include "test_util.h"

static const char *const TITLES[] = { "读书", "写代码", "跑步" };

static void expect_same(const SeqList *s, const LinkedTaskList *l)
{
    int i;
    int n = seq_size(s);

    CHECK_INT(llist_size(l), n);
    for (i = 0; i < n; i++) {
        const Task *a = seq_at(s, i);
        const Task *b = llist_at(l, i);

        CHECK(a != NULL && b != NULL);
        if (a == NULL || b == NULL) {
            return;
        }
        CHECK_STR(b->title, a->title);
        CHECK_INT(b->id, a->id);
        CHECK_INT(b->priority, a->priority);
    }
}

int main(void)
{
    const char *path = "cross_format.tmp";
    SeqList *s;
    LinkedTaskList *l;
    int i;

    printf("\n===== cross-implementation file format test =====\n\n");

    /* 1) 顺序表存 -> 链表读 */
    TC_BEGIN("seq_save -> llist_load");
    s = seq_create();
    l = llist_create();
    CHECK(s != NULL && l != NULL);
    if (s != NULL && l != NULL) {
        for (i = 0; i < 3; i++) {
            seq_push_back(s, task_make(i + 1, i + 1, TITLES[i]));
        }
        CHECK_INT(seq_save(s, path), 0);
        CHECK_INT(llist_load(l, path), 0);
        expect_same(s, l);
    }
    if (s != NULL) seq_destroy(s);
    if (l != NULL) llist_destroy(l);
    TC_END();

    /* 2) 链表存 -> 顺序表读 */
    TC_BEGIN("llist_save -> seq_load");
    s = seq_create();
    l = llist_create();
    CHECK(s != NULL && l != NULL);
    if (s != NULL && l != NULL) {
        for (i = 0; i < 3; i++) {
            llist_push_back(l, task_make(10 + i, 3 - i, TITLES[i]));
        }
        CHECK_INT(llist_save(l, path), 0);
        CHECK_INT(seq_load(s, path), 0);
        expect_same(s, l);
    }
    if (s != NULL) seq_destroy(s);
    if (l != NULL) llist_destroy(l);
    TC_END();

    remove(path);
    return t_summary();
}
