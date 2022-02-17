#ifndef LIST_H
#define LIST_H

/**
 * See also linux lists https://github.com/torvalds/linux/blob/master/include/linux/list.h
 */

#include <assert.h>
#include <stddef.h>

#define container_of(ptr, type, member) (((type*)(((void*)ptr) - offsetof(type, member))))

struct list_head {
    struct list_head* next;
    struct list_head* prev;
};

static inline void list_init(struct list_head* head) {
    assert(head);
    head->next = head;
    head->prev = head;
}

static inline void __list_add(struct list_head* prev, struct list_head* next, struct list_head* element) {
    assert(prev);
    assert(next);
    assert(element);
    element->prev = prev;
    element->next = next;
    prev->next = element;
    next->prev = element;
}

static inline void list_add(struct list_head* head, struct list_head* element) {
    assert(head);
    __list_add(head, head->next, element);
}

static inline void list_add_tail(struct list_head* head, struct list_head* element) {
    assert(head);
    __list_add(head->prev, head, element);
}

static inline void __list_del(struct list_head* prev, struct list_head* next) {
    assert(prev);
    assert(next);
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head* element) {
    assert(element);
    __list_del(element->prev, element->next);
    element->next = NULL;
    element->prev = NULL;
}

#endif  // LIST_H
