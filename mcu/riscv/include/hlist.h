
#ifndef _HLIST_H_
#define _HLIST_H_

#include <stddef.h>

/*
 * 该结构体用于嵌入到业务数据结构体中(entry)，用于实现链表
 * 例：
 *     struct Entry {           // 你的业务数据结构体
 *         ...
 *         struct node node;    // 嵌入其中，位置任意
 *         ...
 *     };
 */
struct node {
    struct node *next, *prev;
};

/*
 * 由成员变量 node 地址获取结构体 entry 地址
 * 例：
 *     struct Entry entry;
 *     struct node *n = &entry.node;
 *     struct Entry *p = node_entry(n, struct Entry, node);
 *     此时 p 指向 entry
 */
#define node_entry(node, type, member) \
    ((type*)((char*)(node) - (size_t)&((type*)0)->member))

/* 带哨兵节点的双向链表 */
struct list {
    struct node base;
};

static inline void list_init(struct list *list)
{
    list->base.next = &list->base;
    list->base.prev = &list->base;
}

static inline bool list_empty(const struct list *list)
{
    return list->base.next == &list->base;
}

static inline bool list_is_head(const struct list *list, const struct node *node)
{
    return list->base.next == node;
}

static inline bool list_is_tail(const struct list *list, const struct node *node)
{
    return list->base.prev == node;
}

/* node 插入到 pos 后面 */
static inline void list_insert_backward(struct node *pos, struct node *node)
{
    node->prev = pos;
    node->next = pos->next;
    node->prev->next = node;
    node->next->prev = node;
}

/* node 插入到 pos 前面 */
static inline void list_insert(struct node *pos, struct node *node)
{
    node->prev = pos->prev;
    node->next = pos;
    node->prev->next = node;
    node->next->prev = node;
}

static inline void list_add_tail(struct list *list, struct node *node)
{
    list_insert(&list->base, node);
}

static inline void list_add_head(struct list *list, struct node *node)
{
    list_insert(list->base.next, node);
}

static inline void list_remove(struct node *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

static inline void list_remove_tail(struct list *list)
{
    list_remove(list->base.prev);
}

static inline void list_remove_head(struct list *list)
{
    list_remove(list->base.next);
}

static inline void list_replace(struct node *old, struct node *node)
{
    node->next = old->next;
    node->next->prev = node;
    node->prev = old->prev;
    node->prev->next = node;
}

#define list_for_each(node, list)  \
    for (node = (list)->base.next; node != &(list)->base; node = (node)->next)

#define list_for_each_safe(node, tmp, list)  \
    for (node = (list)->base.next, tmp = (node)->next; node != &(list)->base; node = tmp, tmp = (node)->next)

/* 获取头结点，或空 */
#define list_head_entry(list, type, member) \
    (list_empty(list) ? NULL : node_entry((list)->base.next, type, member))

/* 获取尾结点，或空 */
#define list_tail_entry(list, type, member) \
    (list_empty(list) ? NULL : node_entry((list)->base.prev, type, member))

/* 获取下一结点，或空 */
#define list_next_entry(entry, list, type, member) \
    (list_is_tail(list, &(entry)->member) ? \
        NULL : \
        node_entry((entry)->member.next, type, member))

/* 获取上一结点，或空 */
#define list_prev_entry(entry, list, type, member) \
    (list_is_head(list, &(entry)->member) ? \
        NULL : \
        node_entry((entry)->member.prev, type, member))

/* 遍历链表；过程中如需操作链表，请使用 _SAFE 版本 */
#define list_for_each_entry(entry, list, type, member) \
    for (entry = node_entry((list)->base.next, type, member); \
         &(entry)->member != &(list)->base; \
         entry = node_entry((entry)->member.next, type, member))

#define list_for_each_entry_safe(entry, tmp, list, type, member) \
    for (entry = node_entry((list)->base.next, type, member), \
         tmp = node_entry((entry)->member.next, type, member); \
         &(entry)->member != &(list)->base; \
         entry = tmp, tmp = node_entry((entry)->member.next, type, member))

#endif /* _HLIST_H_ */
