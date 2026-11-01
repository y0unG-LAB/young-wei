#ifndef SEQLIST_H
#define SEQLIST_H

#include "task.h"

/* ============================================================
 * 顺序表（动态数组实现）—— 本文件是"完整参考实现"
 *
 * 学习重点：为什么尾部插入是 O(1) 均摊，而中间插入是 O(n)？
 * 答案在 seq_insert() 里的那次 memmove。
 * ============================================================ */

typedef struct SeqList SeqList;   /* 不透明类型：内部布局只写在 seqlist.c 里 */

SeqList *seq_create(void);
void      seq_destroy(SeqList *l);

/* 元素个数。空表返回 0；l 为 NULL 时返回 0。 */
int seq_size(const SeqList *l);

/* 在逻辑位置 pos 处插入 t（pos 取值 [0, size]）。
 * 返回 0 成功；返回 -1 表示位置非法或内存不足。 */
int seq_insert(SeqList *l, int pos, Task t);

/* 等价于 seq_insert(l, seq_size(l), t)，但它是尾插的常用写法。 */
int seq_push_back(SeqList *l, Task t);

/* 删除位置 pos 上的元素（pos 取值 [0, size-1]）。
 * 返回 0 成功；返回 -1 表示位置非法。 */
int seq_remove(SeqList *l, int pos);

/* 取位置 pos 的元素的指针。越界返回 NULL。
 * 注意：返回值指向表内部存储，表被修改后可能失效，不要长期保存。 */
const Task *seq_at(const SeqList *l, int pos);

/* 在 title 里做子串匹配。命中的下标依次写入 out_positions（最多写 max_out 个），
 * 返回"命中总数"（可能大于 max_out，用来告诉你结果被截断了）。
 * out_positions 可以为 NULL，此时只统计数量。 */
int seq_find(const SeqList *l, const char *keyword, int *out_positions, int max_out);

/* 原地逆序。返回 0。 */
int seq_reverse(SeqList *l);

/* 把 a、b 两个"已按 task_compare 升序排好"的表归并进 dst。
 * 语义：dst 原有内容被丢弃；dst 不能等于 a 或 b。返回 0 成功。 */
int seq_merge_sorted(SeqList *dst, const SeqList *a, const SeqList *b);

/* ---- 序列化：文本格式，两种实现互通 ----
 * 每行一条：  id|priority|title
 * 以 # 开头的是注释行，加载时跳过。 */
int seq_save(const SeqList *l, const char *path);
int seq_load(SeqList *l, const char *path);   /* 语义：先清空，再装入 */

#endif /* SEQLIST_H */
