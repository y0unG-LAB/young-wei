#include "task.h"

#include <stddef.h>

void task_copy_title(char *dst, const char *src)
{
    size_t i = 0;

    if (dst == NULL) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    while (src[i] != '\0' && i < TASK_TITLE_MAX - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

Task task_make(int id, int priority, const char *title)
{
    Task t;

    t.id = id;
    t.priority = priority;
    task_copy_title(t.title, title);
    return t;
}

int task_compare(const Task *a, const Task *b)
{
    if (a->priority != b->priority) {
        return (a->priority < b->priority) ? -1 : 1;
    }
    if (a->id != b->id) {
        return (a->id < b->id) ? -1 : 1;
    }
    return 0;
}
