#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "task.h"

/* ============================================================
 * 单链表（带头结点）—— 本文件的工作区
 *
 * 结构约定（一定要先读懂，再动手）：
 *
 *        head                                  tail
 *         |                                     |
 *         v                                     v
 *      [ 哨兵 ] --> [ 数据 ] --> [ 数据 ] --> [ 数据 ] --> NULL
 *
 *   - head 是"头结点/哨兵"，它的 task 字段不使用，目的是消灭"空表插入"
 *     和"在表头插入"这两类边界判断。
 *   - 空表时：head->next == NULL 且 tail == head，size == 0。
 *   - 不变的约束（invariant）：head->next ... tail 就是全部数据结点，
 *     且 tail->next == NULL。每次插入/删除后都必须让它继续成立。
 * ============================================================ */

typedef struct LinkedTaskList LinkedTaskList;

LinkedTaskList *llist_create(void);
void            llist_destroy(LinkedTaskList *l);

int llist_size(const LinkedTaskList *l);

/* 在逻辑位置 pos 处插入 t（pos 取值 [0, size]）。
 * 返回 0 成功；返回 -1 表示位置非法或内存不足。 */
int llist_insert(LinkedTaskList *l, int pos, Task t);

int llist_push_back(LinkedTaskList *l, Task t);

/* 删除位置 pos 上的元素（pos 取值 [0, size-1]）。返回 0 成功，-1 位置非法。 */
int llist_remove(LinkedTaskList *l, int pos);

/* 取位置 pos 的元素的指针。越界返回 NULL。
 * 把它和 seq_at 对比着看：同样的功能，这里为什么只能 O(n)？ */
const Task *llist_at(const LinkedTaskList *l, int pos);

int llist_find(const LinkedTaskList *l, const char *keyword, int *out_positions, int max_out);

/* 原地逆序（不许新建结点，只改 next 指针）。返回 0。 */
int llist_reverse(LinkedTaskList *l);

/* 归并两个有序表到 dst。语义同 seq_merge_sorted。
 * 提示：链表做归并比数组干净得多 —— 你不需要搬移任何元素，只改指针。 */
int llist_merge_sorted(LinkedTaskList *dst, const LinkedTaskList *a, const LinkedTaskList *b);

int llist_save(const LinkedTaskList *l, const char *path);
int llist_load(LinkedTaskList *l, const char *path);

#endif /* LINKEDLIST_H */
