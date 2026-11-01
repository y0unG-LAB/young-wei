#include "seqlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEQ_INIT_CAP 8

/* 顺序表 = 一段连续的 Task 数组 + 用了多少（size）+ 申请了多少（cap）。
 * 三个字段缺一不可：只存 size 不知道何时扩容，只存 cap 不知道哪个位置是空的。 */
struct SeqList {
    Task *data;
    int   size;
    int   cap;
};

/* 保证容量至少能放 need 个元素。
 * 扩容策略：1.5 倍增长（不是每次 +1）。为什么？
 *   - 每次 +1：n 次尾插要搬 n^2 次，退化成 O(n^2)
 *   - 每次 2 倍：均摊 O(1)，但平均浪费近一半空间
 *   - 1.5 倍：均摊仍是 O(1)，且腾出的旧块更容易被复用
 * 这就是"均摊分析"在工程里的样子。 */
static int seq_reserve(SeqList *l, int need)
{
    int newcap;
    Task *p;

    if (need <= l->cap) {
        return 0;
    }

    newcap = (l->cap > 0) ? l->cap : SEQ_INIT_CAP;
    while (newcap < need) {
        newcap = newcap + newcap / 2;
    }

    /* 注意：realloc 可能把整块内存搬到新地址，所以必须用返回值重新赋值，
     * 绝不能写 l->data = realloc(l->data, ...) 之后还去用旧的 l->data。 */
    p = (Task *)realloc(l->data, (size_t)newcap * sizeof(Task));
    if (p == NULL) {
        return -1;              /* 原内存还在，l 依然有效 */
    }
    l->data = p;
    l->cap = newcap;
    return 0;
}

SeqList *seq_create(void)
{
    SeqList *l = (SeqList *)malloc(sizeof(SeqList));

    if (l == NULL) {
        return NULL;
    }
    l->data = NULL;
    l->size = 0;
    l->cap = 0;

    if (seq_reserve(l, SEQ_INIT_CAP) != 0) {
        free(l);
        return NULL;
    }
    return l;
}

void seq_destroy(SeqList *l)
{
    if (l == NULL) {
        return;
    }
    free(l->data);              /* data 是一整块，一次 free 就够，这是数组版的好处 */
    free(l);
}

int seq_size(const SeqList *l)
{
    return (l == NULL) ? 0 : l->size;
}

int seq_insert(SeqList *l, int pos, Task t)
{
    if (l == NULL || pos < 0 || pos > l->size) {
        return -1;
    }
    if (seq_reserve(l, l->size + 1) != 0) {
        return -1;
    }

    /* 核心就这一行：把 [pos, size) 整体后移一格，腾出 pos。
     * 用 memmove 而不是 memcpy —— 源和目标区间重叠时 memcpy 行为未定义。
     * 这一步的开销 = 要搬的元素个数，所以中间插入是 O(n)。 */
    memmove(&l->data[pos + 1], &l->data[pos],
            (size_t)(l->size - pos) * sizeof(Task));

    l->data[pos] = t;
    l->size++;
    return 0;
}

int seq_push_back(SeqList *l, Task t)
{
    if (l == NULL) {
        return -1;
    }
    return seq_insert(l, l->size, t);   /* 插在末尾：memmove 长度为 0，什么也不搬 */
}

int seq_remove(SeqList *l, int pos)
{
    if (l == NULL || pos < 0 || pos >= l->size) {
        return -1;
    }

    /* 把 [pos+1, size) 整体前移一格，覆盖掉 pos。同样是 O(n)。 */
    memmove(&l->data[pos], &l->data[pos + 1],
            (size_t)(l->size - pos - 1) * sizeof(Task));

    l->size--;                          /* 只是逻辑上少一个，内存不还 */
    return 0;
}

const Task *seq_at(const SeqList *l, int pos)
{
    if (l == NULL || pos < 0 || pos >= l->size) {
        return NULL;
    }
    /* 一次乘加算出地址：这就是"随机访问 O(1)"的真身。 */
    return &l->data[pos];
}

int seq_find(const SeqList *l, const char *keyword, int *out_positions, int max_out)
{
    int i;
    int hits = 0;

    if (l == NULL || keyword == NULL) {
        return 0;
    }
    for (i = 0; i < l->size; i++) {
        if (strstr(l->data[i].title, keyword) != NULL) {
            if (out_positions != NULL && hits < max_out) {
                out_positions[hits] = i;
            }
            hits++;                     /* 即使写不下也继续数，用来告诉调用方被截断了 */
        }
    }
    return hits;
}

int seq_reverse(SeqList *l)
{
    int i, j;
    Task tmp;

    if (l == NULL) {
        return -1;
    }
    for (i = 0, j = l->size - 1; i < j; i++, j--) {
        tmp = l->data[i];
        l->data[i] = l->data[j];
        l->data[j] = tmp;
    }
    return 0;
}

int seq_merge_sorted(SeqList *dst, const SeqList *a, const SeqList *b)
{
    int i = 0, j = 0;

    if (dst == NULL || a == NULL || b == NULL) {
        return -1;
    }
    if (dst == a || dst == b) {
        return -1;                      /* 自己归并自己会把源数据冲掉 */
    }

    dst->size = 0;                      /* 语义：丢弃 dst 原有内容 */

    /* 双指针：每次从两表头部挑较小的那个。总共走 m+n 步，所以是 O(m+n)。 */
    while (i < a->size && j < b->size) {
        if (task_compare(&a->data[i], &b->data[j]) <= 0) {
            seq_push_back(dst, a->data[i]);
            i++;
        } else {
            seq_push_back(dst, b->data[j]);
            j++;
        }
    }
    while (i < a->size) {               /* a 剩下的部分整体尾巴 */
        seq_push_back(dst, a->data[i]);
        i++;
    }
    while (j < b->size) {
        seq_push_back(dst, b->data[j]);
        j++;
    }
    return 0;
}

/* 文件格式（两种实现共用，所以 seq_save 的文件能被 llist_load 读进来）：
 *     # tasklist v1
 *     1|1|读书
 *     2|3|写代码
 * 以 # 开头是注释。用 | 分隔，因为标题里可能出现空格。 */

int seq_save(const SeqList *l, const char *path)
{
    FILE *f;
    int i;

    if (l == NULL || path == NULL) {
        return -1;
    }
    f = fopen(path, "w");
    if (f == NULL) {
        return -1;
    }

    fprintf(f, "# tasklist v1\n");
    for (i = 0; i < l->size; i++) {
        fprintf(f, "%d|%d|%s\n", l->data[i].id, l->data[i].priority, l->data[i].title);
    }

    fclose(f);
    return 0;
}

int seq_load(SeqList *l, const char *path)
{
    FILE *f;
    char line[256];

    if (l == NULL || path == NULL) {
        return -1;
    }
    f = fopen(path, "r");
    if (f == NULL) {
        return -1;
    }

    l->size = 0;                        /* 语义：先清空再装入 */
    while (fgets(line, (int)sizeof line, f) != NULL) {
        int id = 0, prio = 0;
        char title[TASK_TITLE_MAX];

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        title[0] = '\0';
        /* %63 是硬编码的 TASK_TITLE_MAX-1；改 TASK_TITLE_MAX 时这里也要改。 */
        if (sscanf(line, "%d|%d|%63[^\n]", &id, &prio, title) >= 2) {
            (void)seq_push_back(l, task_make(id, prio, title));
        }
    }

    fclose(f);
    return 0;
}
