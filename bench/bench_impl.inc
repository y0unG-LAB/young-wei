/* bench_impl.inc —— 被 bench.c 用两组宏各包含一次，
 * 从而为两种实现生成"同构"的基准函数，避免复制粘贴两遍。
 *
 * 包含前必须已定义：
 *   B_FN       本文件要生成的函数名（seq_run / linked_run）
 *   LIST_T     表类型
 *   B_CREATE   创建
 *   B_DESTROY  销毁
 *   B_PUSH     尾插
 *   B_INSERT   指定位置插入
 *   B_AT       按下标取
 *   B_REMOVE   按下标删
 *   B_SIZE     取长度
 */

static void B_FN(int n, BenchRow *row)
{
    int m = n / 10;
    int i;
    int pos;
    double t0, t1;
    LIST_T *l;

    if (m < 500) {
        m = 500;
    }

    /* ---- 1) 尾部追加 n 次：顺序表 realloc，链表 malloc 一个结点 ---- */
    l = B_CREATE();
    t0 = now_sec();
    for (i = 0; i < n; i++) {
        B_PUSH(l, task_make(i + 1, (i % 3) + 1, "append"));
    }
    t1 = now_sec();
    row->push_ms = (t1 - t0) * 1000.0;
    B_DESTROY(l);

    /* ---- 2) 在随机位置插入 m 次：顺序表 memmove，链表走位 ---- */
    l = B_CREATE();
    for (i = 0; i < n / 2; i++) {
        B_PUSH(l, task_make(i + 1, (i % 3) + 1, "base"));
    }
    row->insert_ms = 0.0;
    t0 = now_sec();
    for (i = 0; i < m; i++) {
        /* 用固定的乘法散列代替 rand()，保证两种实现插入的位置序列完全相同 */
        pos = (int)(((long)i * 7919L) % (long)(B_SIZE(l) + 1));
        if (B_INSERT(l, pos, task_make(n + i + 1, 2, "random")) != 0) {
            row->insert_ms = -1.0;   /* 标记为"不支持/未实现" */
            break;
        }
    }
    t1 = now_sec();
    if (row->insert_ms >= 0.0) {
        row->insert_ms = (t1 - t0) * 1000.0;
    }
    B_DESTROY(l);

    /* ---- 3) 随机访问 m 次：顺序表 O(1)，链表 O(n) ---- */
    l = B_CREATE();
    for (i = 0; i < n; i++) {
        B_PUSH(l, task_make(i + 1, 1, "access"));
    }
    t0 = now_sec();
    for (i = 0; i < m; i++) {
        const Task *t = B_AT(l, (int)(((long)i * 7919L) % (long)n));
        if (t != NULL) {
            g_sink += t->id;         /* 防止被优化掉 */
        }
    }
    t1 = now_sec();
    row->access_ms = (t1 - t0) * 1000.0;
    B_DESTROY(l);

    /* ---- 4) 从头部删除 m 次：顺序表每次搬 n 个，链表只改两个指针 ---- */
    l = B_CREATE();
    for (i = 0; i < n; i++) {
        B_PUSH(l, task_make(i + 1, 1, "remove"));
    }
    t0 = now_sec();
    for (i = 0; i < m; i++) {
        if (B_REMOVE(l, 0) != 0) {
            break;
        }
    }
    t1 = now_sec();
    row->remove_ms = (t1 - t0) * 1000.0;
    B_DESTROY(l);
}
