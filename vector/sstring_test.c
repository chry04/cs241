/**
* Vector Lab
* CS 241 - Fall 2018
*/

#include "sstring.h"
#include <assert.h>
#include <string.h>

void test_sstring();
void test_sstr_split();

int main(int argc, char *argv[]) {
    // TODO create some tests
    test_sstring();
    test_sstr_split();
    return 0;
}

void test_sstring()
{
    //1.
    sstring *str1 = cstr_to_sstring("abc");
    sstring *str2 = cstr_to_sstring("def");
    sstring_append(str1, str2);
    char *result1 = sstring_to_cstr(str1);
    assert(strcmp(result1, "abcdef") ==0); // == "abcdef"
    free(result1);
    // 2.
    char *answer[] = {"This", "is", "a", "sentence."};
    sstring* str3 = cstr_to_sstring("This is a sentence.");
    vector* vec = sstring_split(str3, ' ');
    for (size_t i = 0; i < vector_size(vec); i++) {
        assert(strcmp(answer[i], vector_get(vec, i)) == 0);
    }
    // 3.
    sstring *replace_me = cstr_to_sstring("This is a {} day, {}!");
    sstring_substitute(replace_me, 18, "{}", "friend");
    char* result2 = sstring_to_cstr(replace_me);
    //printf("%s\n", result2);
    assert(strcmp(result2, "This is a {} day, friend!") == 0); // == "This is a {} day, friend!"
    free(result2);

    sstring_substitute(replace_me, 0, "{}", "good");
    char* result3 = sstring_to_cstr(replace_me);
    //printf("result3: %s\n", result3);
    assert(strcmp(result3, "This is a good day, friend!") == 0); // == "This is a good day, friend!"
    free(result3);
    // 4.
    sstring *slice_me = cstr_to_sstring("1234567890");
    char * slice = sstring_slice(slice_me, 2, 5);
    // == "345" (don't forget about the null byte!)
    //printf("slice: %s\n", slice);
    assert(strcmp(slice, "345") == 0);


    free(slice);
    sstring_destroy(str1);
    sstring_destroy(str2);
    sstring_destroy(str3);
    sstring_destroy(replace_me);
    sstring_destroy(slice_me);
    vector_destroy(vec);
}
void debug_vector(vector *v) {
    size_t n = vector_size(v);
    printf("vector: ");
    for (size_t i = 0; i < n; i++) {
        void *e = vector_get(v, i);
        printf("\'%s\' ", (char *) e);
    }
    printf("\n");
}

void test_sstr_split()
{
    char *strs[4] = {"a b c ", "    ", " a b c ", "a b c"};
    for (size_t i = 0; i < 4; ++i) {
        sstring* sstr = cstr_to_sstring(strs[i]);
        vector* toks = sstring_split(sstr, ' ');
        printf("\'%s\'\t", strs[i]);
        debug_vector(toks);
        sstring_destroy(sstr);
        vector_destroy(toks);
    }
}
