/* ============================================================
 *  王卓版风格的顺序表插入（完整可运行版）
 *
 *  教材 -> 代码 的对应关系：
 *      Status   -> int            OK -> 1        ERROR -> 0
 *      ElemType -> int            MAXSIZE -> 10
 *      Sqlist   -> SqList         &L -> SqList *L   （C 没有引用，用指针）
 *      第 i 个  -> L->elem[i-1]   （教材 i 从 1 开始，数组从 0 开始）
 *
 *  ★ 程序输出全部用英文：源码是 UTF-8，而 Windows 控制台默认 GBK，
 *    打印中文会变成"椤哄簭琛?"这种乱码。英文没有这个问题。
 *    注释保留中文（注释不进程序输出，不影响运行）。
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

/* 打印整张表，方便肉眼对照 */
static void Print(const char *tag, const SqList *L)
{
    int k;

    printf("  %-18s length=%d :", tag, L->length);
    for (k = 0; k < L->length; k++) {
        printf(" %d", L->elem[k]);
    }
    printf("\n");
}

/* 在第 i 个位置（1-based）插入 e。成功返回 OK，失败返回 ERROR。 */
Status ListInsert(SqList *L, int i, ElemType e) {
    if (i<1||i>L->length+1)
        return ERROR;
    if (L->length==MAXSIZE)
        return ERROR;
    for (int j=L->length-1;j>=i-1;j--)
        L->elem[j+1]=L->elem[j];
    L->elem[i-1]=e;
    L->length++;
    return OK;
}
Status ListDelete(SqList *L,int i,ElemType *e) {
    if (i<1||i>L->length)
        return ERROR;
    *e=L->elem[i-1];
    for (int j=i;j<=L->length-1;j++)
        L->elem[j-1]=L->elem[j];
    L->length--;
    return OK;


}


int main(void)
{
    SqList L = {{0}, 0};

    printf("========== insert test ==========\n");

    ListInsert(&L, 1, 10);            /* 空表插第 1 个 */
    Print("insert #1 (10)", &L);      /* expect: length=1 : 10 */

    ListInsert(&L, 2, 20);            /* 尾插 */
    ListInsert(&L, 3, 30);            /* 尾插 */
    Print("push_back 20,30", &L);     /* expect: length=3 : 10 20 30 */

    ListInsert(&L, 1, 5);             /* 头插 */
    Print("insert head 5", &L);       /* expect: length=4 : 5 10 20 30 */

    ListInsert(&L, 3, 15);            /* 中间插 */
    Print("insert middle 15", &L);    /* expect: length=5 : 5 10 15 20 30 */

    printf("\n========== boundary test ==========\n");

    printf("  i=0    returned %d", ListInsert(&L, 0, 99));
    printf("   (expect %d, list must stay unchanged)\n", ERROR);
    Print("after illegal", &L);       /* expect: length=5, content unchanged */

    /* 不要把 ListInsert 塞进 printf 的参数里：C 的参数求值顺序不确定，
     * ListInsert 会改 length，打印出来的 i 可能是改过之后的值。先算好再传。 */
    {
        int pos = L.length + 1;                  /* 尾插位置：合法 */
        printf("  i=%d    returned %d", pos, ListInsert(&L, pos, 40));
        printf("   (expect %d, tail insert ok)\n", OK);
    }
    Print("push_back 40", &L);        /* expect: length=6 : 5 10 15 20 30 40 */

    {
        int pos = L.length + 2;                  /* 跳位：非法 */
        printf("  i=%d    returned %d", pos, ListInsert(&L, pos, 50));
        printf("   (expect %d, gap not allowed)\n", ERROR);
    }

    return 0;
}
