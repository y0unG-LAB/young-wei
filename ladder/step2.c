/* ============================================================
 *  第 2 级：动态扩容的顺序表
 *
 *  和上一版的区别就三行：
 *
 *      ElemType elem[MAXSIZE];   →   ElemType *elem;   （指针，内存要自己申请）
 *      int length;                    int length;      （用了几个）
 *                                     int cap;         （申请了几个）← 新增
 *
 *  ★ 这是本级的核心：
 *      内存不再"长在"结构体里面，而是 malloc 出来、由 elem 记住地址。
 *      满了不再报错，而是 realloc 一块更大的、把数据搬过去。
 *
 *  ★ 你要实现：ListReserve（就这一个函数）
 *
 *  运行后观察 cap 这一列，你会看到它 2 → 3 → 4 → 6 → 9 地长大。
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>

#define OK       1
#define ERROR    0
#define INIT_CAP 2          /* 故意设小：插第 3 个就要扩容，方便观察 */

typedef int Status;
typedef int ElemType;

typedef struct {
    ElemType *elem;         /* 指向堆上的数组（不是数组本身！） */
    int       length;       /* 现在装了几个 */
    int       cap;          /* 申请了几个位置 */
} SqList;

/* ---------- 辅助：打印 ---------- */
static void Print(const char *tag, const SqList *L)
{
    int k;

    printf("  %-12s len=%2d cap=%2d :", tag, L->length, L->cap);
    for (k = 0; k < L->length; k++) {
        printf(" %d", L->elem[k]);
    }
    printf("\n");
}

/* ---------- 初始化：申请 INIT_CAP 个位置 ---------- */
void ListInit(SqList *L)
{
    L->elem = (ElemType *)malloc((size_t)INIT_CAP * sizeof(ElemType));
    if (L->elem == NULL) {
        printf("malloc failed\n");
        exit(1);
    }
    L->cap    = INIT_CAP;
    L->length = 0;
}

/* ---------- 销毁：把 malloc 出来的内存还回去 ---------- */
void ListDestroy(SqList *L)
{
    free(L->elem);          /* 整块内存一次还清，这是顺序表的好处 */
    L->elem   = NULL;
    L->cap    = 0;
    L->length = 0;
}

/* ============================================================
 *  ★★★ 你要实现的唯一一个函数 ★★★
 *
 *  保证容量至少能放 need 个元素。
 *
 *  要求：
 *    1. 如果 L->cap >= need，已经够了，直接 return OK，什么都别做
 *    2. 不够就扩大：新容量从当前 cap 出发，每次乘 1.5（即 newcap + newcap/2），
 *       直到 newcap >= need
 *    3. 用 realloc 重新申请；失败返回 ERROR
 *    4. 成功后更新 L->elem 和 L->cap
 *
 *  ★ 陷阱（想清楚再写）：
 *      realloc 可能把整块内存搬到新地址。所以不能写：
 *          L->elem = realloc(L->elem, ...);      ← 如果失败返回 NULL，
 *                                                  你就把原地址弄丢了（内存泄漏）
 *      必须先存到临时变量，判断非 NULL 后再赋给 L->elem。
 * ============================================================ */
Status ListReserve(SqList *L, int need)
{
    /* TODO: 你来实现，大约 10 行 */

    (void)L;
    (void)need;
    return ERROR;
}

/* ---------- 插入：和上一版一样，只是"满了"改成了"扩容" ---------- */
Status ListInsert(SqList *L, int i, ElemType e)
{
    int j;

    if (i < 1 || i > L->length + 1)
        return ERROR;

    /* ★ 唯一的变化：不再 return ERROR，而是先扩容量 */
    if (ListReserve(L, L->length + 1) != OK)
        return ERROR;

    for (j = L->length - 1; j >= i - 1; j--)
        L->elem[j + 1] = L->elem[j];

    L->elem[i - 1] = e;
    L->length++;
    return OK;
}

/* ---------- 主函数 ---------- */
int main(void)
{
    SqList L;
    int i;

    ListInit(&L);
    Print("after init", &L);

    printf("\n-- insert 10 items, watch cap grow --\n");
    for (i = 1; i <= 10; i++) {
        ListInsert(&L, L.length + 1, i * 10);
        printf("  insert %3d -> len=%2d cap=%2d\n", i * 10, L.length, L.cap);
    }

    printf("\n");
    Print("final", &L);

    /* 检查内容对不对 */
    for (i = 0; i < 10; i++) {
        if (L.elem[i] != (i + 1) * 10) {
            printf("  !!! data corrupted at index %d\n", i);
            break;
        }
    }
    if (i == 10) {
        printf("  data check: OK\n");
    }

    ListDestroy(&L);
    Print("after destroy", &L);
    return 0;
}
