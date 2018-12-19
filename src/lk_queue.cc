#include "lk_queue.h"
#include <atomic>

#ifndef TBB
#include "concurrentqueue.h"

extern "C" {
struct lock_free_queue_s {
    moodycamel::ConcurrentQueue<void *> lk_queue;
    std::atomic<size_t> lk_size;
};

bool lk_queue_push(lock_free_queue_t *lk_queue, void *entry) {
    lk_queue->lk_size += 1;
    return lk_queue->lk_queue.enqueue(entry);
}

void *lk_queue_pop(lock_free_queue_t *lk_queue) {
    void *item;
    if (lk_queue->lk_queue.try_dequeue(item)) {
        lk_queue->lk_size -= 1;
        return item;
    }

    return nullptr;
}
} // end extern "C"

#else

#include <tbb/concurrent_queue.h>

extern "C" {
struct lock_free_queue_s {
    tbb::concurrent_queue<void *> lk_queue;
    std::atomic<size_t> lk_size;
};

bool lk_queue_push(lock_free_queue_t *lk_queue, void *entry) {
    lk_queue->lk_queue.push(entry);
    lk_queue->lk_size += 1;
    return true;
}

void *lk_queue_pop(lock_free_queue_t *lk_queue) {
    void *item;
    if (lk_queue->lk_queue.try_pop(item)) {
        lk_queue->lk_size -= 1;
        return item;
    }

    return nullptr;
}
} // end extern "C"

#endif

extern "C" {
lock_free_queue_t *new_lk_queue() {
    lock_free_queue_t *q = new lock_free_queue_t;
    q->lk_size = 0;
    return q;
}

void delete_lk_queue(lock_free_queue_t *lk_queue) {
    delete lk_queue;
}

size_t lk_queue_size(lock_free_queue_t *lk_queue) {
    return lk_queue->lk_size;
}

} // end extern "C"
