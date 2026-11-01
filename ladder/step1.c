/* ============================================================
 *  阶梯第 1 步：固定大小的顺序表（不用 malloc，不用指针）
 *
 *  这一版只讲清楚三件事：
 *    1. size 到底是什么
 *    2. 插入为什么要"往后搬"
 *    3. 删除为什么要"往前搬"
 *
 *  编译运行（在这个目录下）：
 *      cl step1.c       然后  step1.exe
 *    或 gcc step1.c -o step1   然后  ./step1
 *
 *  你要达成的输出（程序现在跑出来是错的，正常）：
 *      size=4 : 5 10 15 20
 *      size=3 : 5 15 20
 * ============================================================ */
#include <stdio.h>

#define CAP 8              /* 最多能放 8 个 */

int data[CAP];             /* 真正存数据的地方 */
int size = 0;              /* 现在放了几个（不是容量！） */

void print(void)
{
    int i;

    printf("size=%d :", size);
    for (i = 0; i < size; i++) {
        printf(" %d", data[i]);
    }
    printf("\n");
}

/* 在位置 pos 插入 v。pos 可以是 0 到 size（含 size，表示追加到末尾）。
 * 成功返回 0；pos 非法或表满返回 -1。
 *
 * 先想，再写：
 *   1. pos < 0 或 pos > size  → 非法
 *   2. size >= CAP            → 满了
 *   3. 把 data[pos] ... data[size-1] 整体往后挪一格
 *   4. data[pos] = v， size++
 *
 * ★ 关键问题：第 3 步的循环，索引要从小到大，还是从大到小？
 *   拿 data = [5, 10, 15]，size = 3，插入 pos = 1 试一遍就知道了。
 */
int insert(int pos, int v)
{
    /* TODO: 你来实现，大约 8 行 */
    (void)pos;
    (void)v;
    return -1;
}

/* 删除位置 pos 的元素。pos 范围 [0, size-1]。
 * 成功返回 0；pos 非法返回 -1。
 *
 * 先想，再写：
 *   1. pos 合不合法
 *   2. 把 data[pos+1] ... data[size-1] 整体往前挪一格
 *   3. size--
 *
 * ★ 关键问题：这次的循环方向和插入相反，为什么？
 */
int remove_at(int pos)
{
    /* TODO: 你来实现，大约 6 行 */
    (void)pos;
    return -1;
}

int main(void)
{
    insert(0, 10);   /* 空表，放到 0 号位       → 10 */
    insert(1, 20);   /* 追加到末尾             → 10 20 */
    insert(0, 5);    /* 头插，原来的被挤到后面   → 5 10 20 */
    insert(2, 15);   /* 中间插                 → 5 10 15 20 */
    print();

    remove_at(1);    /* 删掉 10                → 5 15 20 */
    print();

    return 0;
}
