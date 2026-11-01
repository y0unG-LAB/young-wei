/* ============================================================
 *  顺序表 完整版：插入 + 删除（王卓风格，i 从 1 开始）
 *
 *  ★ 一个 .c 文件的固定骨架，从上到下：
 *      1. #include     引头文件
 *      2. #define      常量
 *      3. typedef      类型别名
 *      4. struct       结构体定义
 *      5. 辅助函数     Print
 *      6. 功能函数     ListInsert / ListDelete
 *      7. int main()   ← 必须最后，且只能有一个
 *
 *  ★ 规则：任何东西都要"先定义、后使用"。
 *    东西写得越多，越要按这个顺序排，否则编译器会说
 *    "未声明的标识符"。
 * ============================================================ */
#include <stdio.h>

#define MAXSIZE 10
#define OK      1
#define ERROR   0

typedef int Status;
typedef int ElemType;

typedef struct {
    ElemType elem[MAXSIZE];   /* 连续内存，一格一个元素 */
    int      length;          /* 现在装了几个 */
} SqList;

/* ---------- 辅助函数：打印整张表 ---------- */
static void Print(const char *tag, const SqList *L)
{
    int k;

    printf("  %-16s len=%d :", tag, L->length);
    for (k = 0; k < L->length; k++) {
        printf(" %d", L->elem[k]);
    }
    printf("\n");
}

/* ---------- 插入：在第 i 个位置（1-based）插入 e ---------- */
Status ListInsert(SqList *L, int i, ElemType e)
{
    int j;

    /* i 是"第几个"：合法范围 1 .. length+1（length+1 表示插到末尾） */
    if (i < 1 || i > L->length + 1)
        return ERROR;
    if (L->length >= MAXSIZE)
        return ERROR;

    /* 从最后一个元素（下标 length-1）倒着挪到下标 i-1 */
    for (j = L->length - 1; j >= i - 1; j--)
        L->elem[j + 1] = L->elem[j];

    L->elem[i - 1] = e;
    L->length++;
    return OK;
}

/* ---------- 删除：删掉第 i 个（1-based），值从 *e 带出 ---------- */
Status ListDelete(SqList *L, int i, ElemType *e)
{
    int j;

    /* 只能删已经存在的：1 .. length */
    if (i < 1 || i > L->length)
        return ERROR;

    *e = L->elem[i - 1];              /* 先保存，后面会被覆盖 */

    /* 下标 i .. length-1 的元素，各自往前挪一格 */
    for (j = i; j <= L->length - 1; j++)
        L->elem[j - 1] = L->elem[j];

    L->length--;
    return OK;
}

/* ---------- 主函数 ---------- */
int main(void)
{
    SqList A = {{0}, 0};              /* 插入用 */
    SqList D = {{0}, 0};              /* 删除用 */
    ElemType out = 0;
    int k, st;

    /* ========== 插入测试 ========== */
    printf("========== insert test ==========\n");

    ListInsert(&A, 1, 10);
    Print("insert #1 (10)", &A);            /* 10 */

    ListInsert(&A, 2, 20);
    ListInsert(&A, 3, 30);
    Print("push_back 20,30", &A);           /* 10 20 30 */

    ListInsert(&A, 1, 5);
    Print("insert head 5", &A);             /* 5 10 20 30 */

    ListInsert(&A, 3, 15);
    Print("insert middle 15", &A);          /* 5 10 15 20 30 */

    printf("  i=0 -> %d (expect 0)\n", ListInsert(&A, 0, 99));
    printf("  i=7 -> %d (expect 0)\n", ListInsert(&A, 7, 99));
    Print("after illegal", &A);             /* 5 10 15 20 30，不变 */

    /* ========== 删除测试 ========== */
    for (k = 0; k < 4; k++) {
        D.elem[k] = (k + 1) * 10;           /* 10 20 30 40 */
    }
    D.length = 4;

    printf("\n========== delete test ==========\n");
    Print("before", &D);

    st = ListDelete(&D, 2, &out);           /* 删中间的 20 */
    printf("  delete #2 -> %d (expect 1), value=%d (expect 20)\n", st, out);
    Print("after delete #2", &D);           /* 10 30 40 */

    st = ListDelete(&D, 1, &out);           /* 删第一个 10 */
    printf("  delete #1 -> %d (expect 1), value=%d (expect 10)\n", st, out);
    Print("after delete #1", &D);           /* 30 40 */

    st = ListDelete(&D, 2, &out);           /* 删最后一个 40 */
    printf("  delete #2 -> %d (expect 1), value=%d (expect 40)\n", st, out);
    Print("after delete #2", &D);           /* 30 */

    st = ListDelete(&D, 1, &out);           /* 删到空 */
    printf("  delete #1 -> %d (expect 1), value=%d (expect 30)\n", st, out);
    Print("after delete #1", &D);           /* 空表 */

    printf("  delete on empty -> %d (expect 0)\n", ListDelete(&D, 1, &out));

    D.elem[0] = 7;
    D.length = 1;                           /* 重置成 [7] */
    printf("  delete #0 -> %d (expect 0)\n", ListDelete(&D, 0, &out));
    printf("  delete #2 -> %d (expect 0)\n", ListDelete(&D, 2, &out));
    Print("after illegal", &D);             /* 7，表没被破坏 */

    /* ========== 隐藏区检查 ========== */
    {
        SqList H = {{0}, 0};

        for (k = 0; k < 4; k++) {
            H.elem[k] = (k + 1) * 10;       /* 10 20 30 40 */
        }
        H.length = 4;

        ListDelete(&H, 2, &out);            /* 删 20 */
        printf("\n  hidden slots after delete: ");
        for (k = 0; k < 6; k++) {
            printf("%d ", H.elem[k]);
        }
        printf("\n  (expect: 10 30 40 40 0 0)\n");
    }

    return 0;
}
