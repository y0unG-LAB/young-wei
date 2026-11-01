#ifndef TASK_H
#define TASK_H

/* 一条待办事项。两种线性表实现都存这个结构体。
 * 注意：Task 是"值类型"，插入/删除时会被整体拷贝，
 * 所以它要尽量小 —— 这就是为什么 title 用定长数组而不是 char*。 */
#define TASK_TITLE_MAX 64

typedef struct {
    int  id;                    /* 唯一编号，从 1 开始 */
    int  priority;              /* 1 = 高, 2 = 普通, 3 = 低 */
    char title[TASK_TITLE_MAX];
} Task;

Task task_make(int id, int priority, const char *title);

/* 把 src 拷进 dst，最多 TASK_TITLE_MAX-1 个字符，并且一定补 '\0'。
 * 不用 strncpy 是因为 strncpy 在源串过长时不会补结束符。 */
void task_copy_title(char *dst, const char *src);

/* 排序依据：先按 priority 升序（1 最靠前），priority 相同再按 id 升序。
 * 返回 <0 / 0 / >0，语义和 qsort 的比较函数一致。 */
int task_compare(const Task *a, const Task *b);

#endif /* TASK_H */
