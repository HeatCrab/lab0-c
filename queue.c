#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    /* 善用 list.h 中的函式 */
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        element_t *e = list_entry(node, element_t, list);
        /* 使用 queue.h 中的函式替代直接呼叫 free() */
        q_release_element(e);
    }
    test_free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;
    new_element->value = strdup(s);
    /* 再次檢查是否有記憶體上的問題發生 */
    if (!new_element->value) {
        free(new_element);
        return false;
    }
    list_add(&new_element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;
    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }
    list_add_tail(&new_element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_first_entry(head, element_t, list);
    list_del(&e->list);

    if (sp && bufsize > 0) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_last_entry(head, element_t, list);
    list_del(&e->list);

    if (sp && bufsize > 0) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int len = 0;
    struct list_head *node;

    list_for_each (node, head)
        len++;

    return len;
}

bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    if (list_is_singular(head)) {
        element_t *elem = list_first_entry(head, element_t, list);
        list_del(&elem->list);
        q_release_element(elem);
        return true;
    }

    struct list_head *slow = head->next;
    struct list_head *fast = head->next;

    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    element_t *mid = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(mid);

    return true;
}

bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head || list_empty(head))
        return false;

    if (list_is_singular(head))
        return true;

    LIST_HEAD(to_discard);
    element_t *curr = NULL, *safe = NULL;
    struct list_head *last_unique = head;

    list_for_each_entry_safe (curr, safe, head, list) {
        /* 如果 safe 不是 head 且與 curr 值相同，跳過，表示仍在重複區段內 */
        if (&safe->list != head && !strcmp(safe->value, curr->value))
            continue;

        if (curr->list.prev != last_unique) {
            /* 臨時存放切割出的重複區段 */
            LIST_HEAD(tmp);
            /* 切割重複區段 */
            list_cut_position(&tmp, last_unique, &curr->list);
            /* 將重複區段加入待丟棄列表 */
            list_splice(&tmp, &to_discard);
        }
        last_unique = safe->list.prev;  // 更新
    }

    list_for_each_entry_safe (curr, safe, &to_discard, list)
        q_release_element(curr);

    return true;
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    // LIST_HEAD(new_head);

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);

    /* 將反轉後的串鏈拼接到原始 head */
    // list_splice(&new_head, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *list_tail = head;
    struct list_head *curr = head->next;
    int count = q_size(head);

    while (count >= k) {
        LIST_HEAD(tmp);
        for (int i = 0; i < k; i++) {
            struct list_head *next_node = curr->next;
            list_move_tail(curr, &tmp);
            curr = next_node;
        }

        q_reverse(&tmp);
        list_splice(&tmp, list_tail);

        /* 將反轉的終點更新 */
        for (int i = 0; i < k; i++)
            list_tail = list_tail->next;

        count -= k;
    }
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    q_reverseK(head, 2);
    /*
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node = head->next;
    while (node != head && node->next != head) {
        struct list_head *next = node->next;
        struct list_head *next_next = next->next;

        list_del(node);
        list_add(node, next);
        node = next_next;
    }
    */
}

/* Merges two sorted circular doubly-linked lists into 'left'
 * in ascending / descending (descend=true) order, clearing 'right'. */
void q_merge_two(struct list_head *left, struct list_head *right, bool descend)
{
    if (!left || !right || list_empty(right))
        return;

    if (list_empty(left)) {
        list_splice_init(right, left);
        return;
    }

    LIST_HEAD(merged);
    struct list_head *l_node = left->next;
    struct list_head *r_node = right->next;

    while (l_node != left && r_node != right) {
        const element_t *l_elem = list_entry(l_node, element_t, list);
        const element_t *r_elem = list_entry(r_node, element_t, list);
        int cmp = strcmp(l_elem->value, r_elem->value);

        if (descend ? cmp > 0 : cmp <= 0) {
            struct list_head *next = l_node->next;
            list_move_tail(l_node, &merged);
            l_node = next;
        } else {
            struct list_head *next = r_node->next;
            list_move_tail(r_node, &merged);
            r_node = next;
        }
    }

    if (l_node != left)
        list_splice_tail_init(left, &merged);

    if (r_node != right)
        list_splice_tail_init(right, &merged);

    list_splice_init(&merged, left);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *fast = head->next;
    struct list_head *slow = head->next;

    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    LIST_HEAD(left);
    list_cut_position(&left, head, slow->prev);

    q_sort(&left, descend);
    q_sort(head, descend);

    q_merge_two(&left, head, descend);
    INIT_LIST_HEAD(head);
    list_splice(&left, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    element_t *cur = list_last_entry(head, element_t, list);
    const char *max_value = cur->value;
    int kept_count = 1;

    while (cur->list.prev != head) {
        element_t *prev = list_last_entry(&cur->list, element_t, list);
        if (strcmp(prev->value, max_value) > 0) {
            max_value = prev->value;
            kept_count++;
            cur = prev;
        } else {
            list_del(&prev->list);
            q_release_element(prev);
        }
    }

    return kept_count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    element_t *cur = list_last_entry(head, element_t, list);
    const char *min_value = cur->value;
    int kept_count = 1;

    while (cur->list.prev != head) {
        element_t *prev = list_last_entry(&cur->list, element_t, list);
        if (strcmp(prev->value, min_value) < 0) {
            min_value = prev->value;
            kept_count++;
            cur = prev;
        } else {
            list_del(&prev->list);
            q_release_element(prev);
        }
    }

    return kept_count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return list_first_entry(head, queue_contex_t, chain)->size;

    queue_contex_t *target_queue =
        list_first_entry(head, queue_contex_t, chain);
    struct list_head *target_list = target_queue->q;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        if (node == &target_queue->chain)
            continue;
        queue_contex_t *curr_queue = list_entry(node, queue_contex_t, chain);
        struct list_head *curr_list = curr_queue->q;
        if (curr_list) {
            q_merge_two(target_list, curr_list, descend);  // 合併到 target_list
        }
    }

    return q_size(target_list);
}
