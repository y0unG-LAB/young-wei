#include "linkedlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 *  结点与表头
 *
 *  链表 = 一堆散落在堆上的结点，用 next 指针串起来。
 *  链表"表头结构"和"数据结点"是两种不同的东西，分开定义：
 *
 *    struct LinkedTaskList   只存 head / tail / size 三个元信息
 *    struct Node             存一条数据 + 指向下一个结点的指针
 *
 *  为什么 head 是"哨兵"（dummy）？
 *    如果 head 直接指向第一个数据结点，那么"在表头插入"要改 head 本身，
 *    而"在别处插入"改的是某个结点的 next —— 两种写法。
 *    加一个空的哨兵结点后，所有插入都是"改某个结点的 next"，边界统一，
 *    代码少一半 if。代价只是一个结点和 16 字节内存。
 * ============================================================ */

typedef struct Node {
    Task task;
    struct Node *next;
} Node;

struct LinkedTaskList {
    Node *head;     /* 哨兵，task 字段不使用 */
    Node *tail;     /* 尾结点；空表时 tail == head */
    int   size;
};

/* 不变量（invariant），每次操作后都必须成立：
 *   1. head->next → ... → tail 是全部数据结点，共 size 个
 *   2. tail->next == NULL
 *   3. 空表时 head->next == NULL 且 tail == head 且 size == 0
 * 这三个条件就是你的调试器。函数返回前在脑子里过一遍。 */

LinkedTaskList *llist_create(void)
{
    LinkedTaskList *l = (LinkedTaskList *)malloc(sizeof(LinkedTaskList));
    Node *sentinel;

    if (l == NULL) {
        return NULL;
    }
    sentinel = (Node *)malloc(sizeof(Node));
    if (sentinel == NULL) {
        free(l);
        return NULL;
    }

    sentinel->next = NULL;
    sentinel->task = task_make(0, 0, "");   /* 哨兵的数据没人看，随手填 */

    l->head = sentinel;
    l->tail = sentinel;                     /* 空表：尾指针指回哨兵 */
    l->size = 0;
    return l;
}

/* 清空所有数据结点，但保留哨兵。destroy 和 load 都要用。 */
static void llist_clear(LinkedTaskList *l)
{
    Node *p = l->head->next;
    Node *next;

    while (p != NULL) {
        /* 关键顺序：先存 next，再 free(p)。
         * 反过来写就是 use-after-free —— 链表最经典的一段错误。 */
        next = p->next;
        free(p);
        p = next;
    }
    l->head->next = NULL;
    l->tail = l->head;
    l->size = 0;
}

void llist_destroy(LinkedTaskList *l)
{
    if (l == NULL) {
        return;
    }
    llist_clear(l);          /* 先还掉所有数据结点 */
    free(l->head);           /* 再还哨兵 */
    free(l);                 /* 最后还表头结构 */
}

int llist_size(const LinkedTaskList *l)
{
    return (l == NULL) ? 0 : l->size;
}

/* 从头走 pos 步，拿到第 pos 个数据结点。
 * 单链表没有"下标"，只能一步一步跳，所以是 O(pos) —— 这就是它的根本弱点。
 * 所有"按下标操作"的函数都复用它，避免每个函数各写一遍遍历。 */
static Node *llist_node_at(const LinkedTaskList *l, int pos)
{
    Node *p;

    if (l == NULL || pos < 0 || pos >= l->size) {
        return NULL;
    }
    p = l->head->next;
    while (pos > 0) {
        p = p->next;
        pos--;
    }
    return p;
}

/* 找到第 pos 个数据结点的"前一个"结点。
 * 为什么需要它？因为插入和删除都要改前驱的 next，
 * 而单链表没有"指向前驱"的指针，只能重新走一遍。 */
static Node *llist_prev_at(const LinkedTaskList *l, int pos)
{
    Node *prev = l->head;       /* pos == 0 时前驱就是哨兵，这是哨兵的第二个好处 */

    while (pos > 0) {
        prev = prev->next;
        pos--;
    }
    return prev;
}

const Task *llist_at(const LinkedTaskList *l, int pos)
{
    Node *p = llist_node_at(l, pos);

    return (p == NULL) ? NULL : &p->task;
}

int llist_insert(LinkedTaskList *l, int pos, Task t)
{
    Node *prev;
    Node *n;

    if (l == NULL || pos < 0 || pos > l->size) {
        return -1;
    }

    /* pos == size 是最常见的"追加"，此时前驱就是 tail，不必再走一遍链。 */
    prev = (pos == l->size) ? l->tail : llist_prev_at(l, pos);

    n = (Node *)malloc(sizeof(Node));
    if (n == NULL) {
        return -1;              /* 分配失败时表没有任何改动，安全 */
    }
    n->task = t;

    /* ★ 这两行的顺序不能反：
     *   先 n->next = prev->next   （新结点先接上后半段）
     *   再 prev->next = n         （前驱再指向新结点）
     * 如果先改 prev->next，后半段的入口就丢了，整条链从 pos 开始泄漏。
     * 记忆法：先接后断，永远不丢。 */
    n->next = prev->next;
    prev->next = n;

    /* 插到了表尾之后，尾指针必须跟着走；否则下一次尾插会插到错的地方。
     * 空表插入时 prev == head == tail，这条也能覆盖。 */
    if (l->tail == prev) {
        l->tail = n;
    }

    l->size++;
    return 0;
}

int llist_push_back(LinkedTaskList *l, Task t)
{
    Node *n;

    if (l == NULL) {
        return -1;
    }

    n = (Node *)malloc(sizeof(Node));
    if (n == NULL) {
        return -1;
    }
    n->task = t;
    n->next = NULL;

    /* 这里直接借 tail 落地，没有走 llist_insert。
     * 为什么？因为 llist_insert 要先 prev_at(pos) 找前驱，pos == size 时
     * 那是"从头走完整条链"的 O(n) —— 明明 tail 就指着尾巴，何必再走一遍？
     * 这就是"链表尾插 O(1)"的完整条件：不仅要有 tail，还要真的用 tail。
     * （我第一版偷懒写成 return llist_insert(l, l->size, t)，结果 20000 次
     *  尾插从 0.7ms 变成 582ms —— 基准测试抓出来的。） */
    l->tail->next = n;
    l->tail = n;
    l->size++;
    return 0;
}

int llist_remove(LinkedTaskList *l, int pos)
{
    Node *prev;
    Node *victim;

    if (l == NULL || pos < 0 || pos >= l->size) {
        return -1;
    }

    prev = llist_prev_at(l, pos);
    victim = prev->next;

    /* 摘链：让前驱跳过 victim。 */
    prev->next = victim->next;

    /* 如果删的是最后一个，尾指针要退回前驱；否则 tail 会指向已释放的内存。 */
    if (l->tail == victim) {
        l->tail = prev;
    }

    free(victim);               /* 摘下来之后再释放，顺序不能反 */
    l->size--;
    return 0;
}

int llist_find(const LinkedTaskList *l, const char *keyword, int *out_positions, int max_out)
{
    const Node *p;
    int hits = 0;
    int idx = 0;

    if (l == NULL || keyword == NULL) {
        return 0;
    }
    for (p = l->head->next; p != NULL; p = p->next, idx++) {
        if (strstr(p->task.title, keyword) != NULL) {
            if (out_positions != NULL && hits < max_out) {
                out_positions[hits] = idx;
            }
            hits++;
        }
    }
    return hits;
}

int llist_reverse(LinkedTaskList *l)
{
    Node *prev = NULL;
    Node *cur;
    Node *next;

    if (l == NULL) {
        return -1;
    }
    if (l->size < 2) {
        return 0;               /* 空表 / 单元素：转了等于没转 */
    }

    cur = l->head->next;

    /* 逆序后，原来的第一个结点变成最后一个，所以尾指针先落到它身上。
     * 这一步很容易忘，忘了之后尾插会从已经变成"中间"的结点往后接。 */
    l->tail = cur;

    /* 三指针标准写法：每一步把 cur 的 next 掉头指向 prev，然后三个指针齐步走。
     * 循环结束时 prev 指向原来的最后一个结点（也就是新的第一个），
     * cur / next 都是 NULL。 */
    while (cur != NULL) {
        next = cur->next;       /* 1. 先存住后半段，不然一改 next 就丢了 */
        cur->next = prev;       /* 2. 掉头 */
        prev = cur;             /* 3. 齐步走 */
        cur = next;
    }

    /* 哨兵重新指向新的第一个结点。注意逆序没有换结点、没有 new，
     * 只是把每条 next 反过来 —— 所以它没花额外内存。 */
    l->head->next = prev;
    return 0;
}

int llist_merge_sorted(LinkedTaskList *dst, const LinkedTaskList *a, const LinkedTaskList *b)
{
    const Node *pa;
    const Node *pb;

    if (dst == NULL || a == NULL || b == NULL) {
        return -1;
    }
    if (dst == a || dst == b) {
        return -1;              /* 会边清空边读源数据，必须禁止 */
    }

    llist_clear(dst);           /* 语义：丢弃 dst 原有内容 */

    /* 双指针归并。和数组版对比：这里不需要预先算好大小、不需要扩容、
     * 也不需要搬移任何元素 —— 只是按顺序把值抄进 dst。
     * 所以链表的归并天然是 O(m+n) 且不占额外空间（数组版要么占 O(m+n) 额外数组，
     * 要么就地归并搬得很痛苦）。 */
    pa = a->head->next;
    pb = b->head->next;

    while (pa != NULL && pb != NULL) {
        if (task_compare(&pa->task, &pb->task) <= 0) {
            llist_push_back(dst, pa->task);
            pa = pa->next;
        } else {
            llist_push_back(dst, pb->task);
            pb = pb->next;
        }
    }
    while (pa != NULL) {
        llist_push_back(dst, pa->task);
        pa = pa->next;
    }
    while (pb != NULL) {
        llist_push_back(dst, pb->task);
        pb = pb->next;
    }
    return 0;
}

int llist_save(const LinkedTaskList *l, const char *path)
{
    FILE *f;
    const Node *p;

    if (l == NULL || path == NULL) {
        return -1;
    }
    f = fopen(path, "w");
    if (f == NULL) {
        return -1;
    }

    fprintf(f, "# tasklist v1\n");
    for (p = l->head->next; p != NULL; p = p->next) {
        fprintf(f, "%d|%d|%s\n", p->task.id, p->task.priority, p->task.title);
    }

    fclose(f);
    return 0;
}

int llist_load(LinkedTaskList *l, const char *path)
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

    llist_clear(l);             /* 语义：先清空再装入 */
    while (fgets(line, (int)sizeof line, f) != NULL) {
        int id = 0, prio = 0;
        char title[TASK_TITLE_MAX];

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        title[0] = '\0';
        if (sscanf(line, "%d|%d|%63[^\n]", &id, &prio, title) >= 2) {
            (void)llist_push_back(l, task_make(id, prio, title));
        }
    }

    fclose(f);
    return 0;
}
