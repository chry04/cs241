/**
* Vector Lab
* CS 241 - Fall 2018
*/

#include "vector.h"
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#define TRY_N 8

static vector *vector1 = NULL;

void test_vector();

void test_string_vector();

void debug_vector(vector *v) {
    size_t n = vector_size(v);
    printf("vector: ");
    for (size_t i = 0; i < n; i++) {
        void *e = vector_get(v, i);
        printf("%zu ", (size_t) e);
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    // Write your test cases here
    test_string_vector();
    test_vector();
    return 0;
}

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

// test vector
void test_vector() {
    vector1 = shallow_vector_create();
    // test insert
    size_t count = 0;
    while (count < TRY_N) {
        vector_insert(vector1, count, (void *) count);
        count++;
    }
    // test get & insert
    count = 0;
    while (count < TRY_N) {
        size_t elem = (size_t) vector_get(vector1, count);
        assert(elem == count);
        count++;
    }

    assert(vector_size(vector1) == TRY_N);
    assert(*vector_begin(vector1) == 0);
    assert((size_t) (*vector_back(vector1)) == TRY_N - 1);
    assert(vector_capacity(vector1) == get_new_capacity(TRY_N));

    vector_resize(vector1, TRY_N * 3);
    assert(vector_capacity(vector1) == get_new_capacity(TRY_N * 3));
    vector_resize(vector1, TRY_N);

    count = 0;
    while (count < TRY_N * 9) {
        vector_push_back(vector1, (void *) (count));
        count++;
    }
    assert(vector_capacity(vector1) == get_new_capacity(TRY_N * 10));

    assert(vector_size(vector1) == TRY_N * 10);

    count = 0;
    while (count < TRY_N * 10) {
        vector_pop_back(vector1);
        count++;
    }
    assert(vector_empty(vector1) == true);
    // test     vector_clear(vector1);
    count = 0;
    while (count < TRY_N * 9) {
        vector_push_back(vector1, (void *) (count));
        count++;
    }
    vector_clear(vector1);
    assert(vector_empty(vector1) == true);

    // test erase
    size_t range = TRY_N;
    count = 0;
    while (count < range) {
        vector_push_back(vector1, (void *) (count));
        count++;
    }
    assert(vector_size(vector1) == range);

    // 2. erase
    debug_vector(vector1);

    size_t erase_times = 2;
    count = 0;
    while (count < erase_times) {
        vector_erase(vector1, 0);
        printf("after erase:");
        debug_vector(vector1);
        count++;
    }
    assert(vector_size(vector1) == range - erase_times);
    debug_vector(vector1);


    count = 0;
    while (count < range - erase_times) {
        size_t elem = (size_t) vector_get(vector1, count);
        assert(elem == count + erase_times);
        count++;
    }
    vector_clear(vector1);
    assert(vector_empty(vector1) == true);

    // test erase
    count = 0;
    while (count < range) {
        vector_push_back(vector1, (void *) (count));
        count++;
    }
    assert(vector_size(vector1) == range);

    size_t leave = 3;
    count = 0;
    while (count < range - leave) {
        vector_erase(vector1, 0);
        count++;
    }
    assert(vector_size(vector1) == leave);


    count = 0;
    while (count < leave) {
        size_t elem = (size_t) vector_get(vector1, count);
        assert(elem == count + range - leave);
        count++;
    }
    vector_destroy(vector1);
}


void test_string_vector() {
    vector *vec = string_vector_create();
    size_t i = 0;
    size_t n = 5;
    char *box[] = {"hello", "I", "am", "from", "....", NULL};
    char **ptr = box;
    // push back
    while (*ptr) {
        vector_push_back(vec, *ptr);
        ptr++;
    }
    // check, get
    while (i < n) {
        char *elem = vector_get(vec, i);
        assert(strcmp(elem, box[i]) == 0);
        i++;
    }
    vector_erase(vec, 2);
    char *elem = vector_get(vec, 2);
    assert(strcmp(elem, box[3]) == 0);
    i = 0;
    while (i < TRY_N){
        vector_insert(vec, i, box[i % n]);
        i++;
    }
    vector_clear(vec);
    vector_clear(vec);
    assert(vector_empty(vec) == true);
    vector_destroy(vec);


}