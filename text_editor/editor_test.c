/**
* Text Editor Lab
* CS 241 - Fall 2018
*/

#include "document.h"
#include "editor.h"
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#define try(a, b)  assert(strcmp(a, b) == 0);
#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

#define show(str) LOG("--%s--\n", str);


//typedef enum {
//    insert_cmd, delete_line, delete_cmd,
//    append_cmd, write_cmd, search_cmd,
//    merge_line, split_line, save_cmd
//} command_kind;

typedef void (*TestFuc)(editor *editor);


#define LARGR_LINE_NO 1000
#define LARGE_STR_LEN 2000
#define LARGE_IDX 1000
/**
 * You can programatically test your text editor.
*/
static size_t start_size = 0;

void create_editor(char *file_name, editor *editor);

void test_insert(editor *editor);

void test_delete_cmd(editor *editor);

void test_append_cmd1(editor *editor);

void test_append_cmd2(editor *editor);

void test_write_cmd(editor *editor);

void test_merge(editor *editor);

void test_split_line(editor *editor);


void debug_vector(vector *v) {
    size_t n = vector_size(v);
    printf("vector: ");
    for (size_t i = 0; i < n; i++) {
        void *e = vector_get(v, i);
        printf("\'%s\' ", (char *) e);
    }
    printf("\n");
}

void simple_test_1(editor *editor);

void test_search(editor *arg);

void test_save_cmd(editor *arg);

int main() {
    TestFuc testFunctions[9] = {
            test_insert,
            test_delete_cmd, //xingzhi
            test_append_cmd1,
            test_append_cmd2,
            test_write_cmd, test_merge, test_split_line, test_search, test_save_cmd
    };


    for (int i = 0; i < 9; ++i) {
        editor e;
        editor *editor = &e;
        // Setting up a docment based on the file named 'filename'.
        char *filename = "test_files/simple.txt";
        create_editor(filename, editor);

        start_size = document_size(editor->document);
        assert(start_size == 5);

        testFunctions[i](editor);

        //cleanup
        handle_cleanup(editor);

    }
}

void create_editor(char *file_name, editor *editor) {
    document *doc = handle_create_document(file_name);
    if (!doc) {
        fprintf(stderr, "Document was NULL!\n");
        return;
    }
    sstring *string = handle_create_string();
    editor->document = doc;
    editor->string = string;
    editor->filename = file_name;
}

void simple_test_1(editor *editor) {
    // d 4
    size_t n = 4;
    handle_delete_line(editor, n);
    assert(document_size(editor->document) == start_size - 1);
    const char *line_4 = document_get_line(editor->document, n);
    try("In the shape of an L on her forehead", line_4);
    //printf("size after delete: %zu\n", document_size(editor->document));

    // a 1 ' me'
    handle_append_command(editor, 1, " me");
    const char *line_1 = document_get_line(editor->document, 1);
    try(line_1, "Somebody once told the world was gonna roll me");
    //printf("size after append: %zu\n", document_size(editor->document));

    // w 5 #I'mSorryForTheBadJoke
    handle_write_command(editor, 5, "#I'mSorryForTheBadJoke");
    const char *line_5 = document_get_line(editor->document, 5);
    try(line_5, "#I'mSorryForTheBadJoke");
    //printf("size after write: %zu\n", document_size(editor->document));
}

void test_insert(editor *editor) {
    location loc;

    // insert to large line_no
    loc.line_no = LARGR_LINE_NO;
    loc.idx = 0;
    handle_insert_command(editor, loc, "I am a vey far line");
    assert(document_size(editor->document) == LARGR_LINE_NO);
    const char *far_line = document_get_line(editor->document, LARGR_LINE_NO);
    try(far_line, "I am a vey far line");

    // insert to normal idx
    loc.line_no = 1;
    loc.idx = 4;
    handle_insert_command(editor, loc, "inserted_abcde_");
    const char *line_1 = document_get_line(editor->document, 1);
    try(line_1, "Someinserted_abcde_body once told the world was gonna roll");
    // insert to edge idx
    //   idx = strlen
    loc.idx = strlen(line_1);
    handle_insert_command(editor, loc, "inserted_abcde_");
    line_1 = document_get_line(editor->document, 1);
    try(line_1, "Someinserted_abcde_body once told the world was gonna rollinserted_abcde_");
    //   idx = 0
    loc.idx = 0;
    handle_insert_command(editor, loc, "inserted_abcde_");
    line_1 = document_get_line(editor->document, 1);
    try(line_1, "inserted_abcde_Someinserted_abcde_body once told the world was gonna rollinserted_abcde_");
    // insert very large large_str
    char large_str[LARGE_STR_LEN + 1];
    memset(large_str, 'a', LARGE_STR_LEN);
    large_str[LARGE_STR_LEN] = '\0';
    loc.line_no = 2;
    loc.idx = 0;
    char *old_line = strdup(document_get_line(editor->document, 2));
    handle_insert_command(editor, loc, large_str);
    line_1 = document_get_line(editor->document, 2);
    bool ret = strncmp(line_1, large_str, LARGE_STR_LEN) == 0 &&
               strcmp(line_1 + LARGE_STR_LEN, old_line) == 0;
    assert(ret);
    //free
    free(old_line);
}

void test_delete_cmd(editor *editor) {
    location loc;
    // del a line
    // 1. idx= strlen
    loc.idx = 0;
    loc.line_no = 2;
    const char *line = document_get_line(editor->document, 2);
    assert(strlen(line));
    handle_delete_command(editor, loc, strlen(line));
    line = document_get_line(editor->document, 2);
    assert(strlen(line) == 0);
    // 2. idx= large
    line = document_get_line(editor->document, 3);
//    show(line);
    assert(strlen(line));
    loc.line_no = 3;
    handle_delete_command(editor, loc, LARGE_IDX);
    line = document_get_line(editor->document, 3);
    assert(strlen(line) == 0);

    // del a part
    loc.idx = 22;
    loc.line_no = 5;
    handle_delete_command(editor, loc, 3);
    line = document_get_line(editor->document, loc.line_no);
//    show(line);
    try(line, "In the shape of an L oer forehead");
}

void test_append_cmd1(editor *editor)// also test write to empty_line
{
    // append to empty line
    handle_append_command(editor, 6, "Yes\\n...\\\\nN\\o\\");//"Yes\n...\\nN\o\"
    assert(document_size(editor->document) == start_size + 2);
    const char *line = document_get_line(editor->document, start_size + 1);
    const char *next_line = document_get_line(editor->document, start_size + 2);
    assert(next_line);
    try(line, "Yes");
    try(next_line, "...\\nNo");

}

void test_append_cmd2(editor *editor) {
    // append to a normal line
    handle_append_command(editor, 1, "\\nworld!\\nHaha\\n");
    assert(document_size(editor->document) == start_size + 3);
    const char *line1 = document_get_line(editor->document, 1);
    const char *line2 = document_get_line(editor->document, 2);
    const char *line3 = document_get_line(editor->document, 3);
    const char *line4 = document_get_line(editor->document, 4);

    try(line1, "Somebody once told the world was gonna roll");
    try(line2, "world!");
    try(line3, "Haha");
    assert(strlen(line4) == 0);
}

void test_write_cmd(editor *editor) {
    // write to a normal line
    // "\nworld!\nHaha\n"
    handle_write_command(editor, 1, "\\nw\\o\\r\\ld!\\nHaha\\n\\");
    assert(document_size(editor->document) == start_size + 3);
    const char *line1 = document_get_line(editor->document, 1);
    const char *line2 = document_get_line(editor->document, 2);
    const char *line3 = document_get_line(editor->document, 3);
    const char *line4 = document_get_line(editor->document, 4);

    assert(strlen(line1) == 0);
    try(line2, "world!");
    try(line3, "Haha");
    assert(strlen(line4) == 0);
}

void test_merge(editor *editor) {
    // merge all lines
    for (size_t i = 0; i < start_size - 1; ++i) {
        handle_merge_line(editor, 1);
    }
    assert(document_size(editor->document) == 1);
    char *ans = "Somebody once told the world was gonna rollI ain't the sharpest tool in the shedShe was looking kinda dumb with her finger and her thumbAccording to all known laws of aviationIn the shape of an L on her forehead";
    const char *line1 = document_get_line(editor->document, 1);
    try(line1, ans);
}

void test_split_line(editor *editor) {
    location locations[5] = {(location) {1, 11},
                             (location) {2 + 1, 0},//idx = 0
                             (location) {3 + 2, 56},//idx= strlen
                             (location) {4 + 3, 1},//idx= 1
                             (location) {5 + 4, 35}
    };
    for (size_t i = 0; i < 5; i++) {
        handle_split_line(editor, locations[i]);
    }
    char *lines[10] = {
            "Somebody on",
            "ce told the world was gonna roll",
            "",
            "I ain't the sharpest tool in the shed",
            "She was looking kinda dumb with her finger and her thumb",
            "",
            "A",
            "ccording to all known laws of aviation",
            "In the shape of an L on her forehea",
            "d"
    };

    assert(document_size(editor->document) == 10);
    for (size_t i = 1; i <= 10; i++) {
        const char *line = document_get_line(editor->document, i);
        try(line, lines[i - 1]);
    }
}

void test_search(editor *arg) {
    (void) arg;
    editor e;
    editor *editor = &e;
    // Setting up a docment based on the file named 'filename'.
    char *filename = "test_files/bee.txt";
    create_editor(filename, editor);

    location locations[6] = {(location) {2, 23},// hit first 'bee'
                             (location) {3, 0},// hit second 'bee'
                             (location) {3, 9},// goto next bee in last line
                             (location) {3, 18},// goto first bee
                             (location) {3, LARGE_IDX},// goto first bee
                             (location) {2, 36},//goto bee in last line
    };

    location answers[6] = {(location) {2, 23},
                           (location) {3, 0},
                           (location) {3, 15},
                           (location) {2, 23},
                           (location) {2, 23},
                           (location) {3, 0}
    };

    for (int i = 0; i < 6; ++i) {
        location loc = handle_search_command(editor, locations[i], "bee");
        assert(loc.idx == answers[i].idx);
        assert(loc.line_no == answers[i].line_no);
    }
    handle_cleanup(editor);
}

void test_save_cmd(editor *arg) {
    (void) arg;
    editor e;
    editor *editor = &e;
    char *cmd;
    // Setting up a docment based on the file named 'filename'.
    char *answer_file_name = "test_files/simple_answer.txt";
    char *to_modify_file_name = "test_files/simple_modify.txt";
    char *sample = "test_files/simple.txt";
    char *non_exist_file = "test_files/new.txt";

    // restore to_modify_file
    asprintf(&cmd, "cp %s %s", sample, to_modify_file_name);
    system(cmd);
    free(cmd);
    // rm new.txt
    asprintf(&cmd, "rm %s", non_exist_file);
    system(cmd);
    free(cmd);

    create_editor(to_modify_file_name, editor);

    simple_test_1(editor);
    handle_save_command(editor);
    asprintf(&cmd, "diff %s %s", answer_file_name, to_modify_file_name);
    system(cmd);
    free(cmd);

    // test O_CREATE
    umask(000);
    document_write_to_file(editor->document, non_exist_file);
    LOG("Your right should be r-w-... for every one!\n");
    char *tmp;
    asprintf(&tmp, "ls -l %s", non_exist_file);
    system(tmp);
    free(tmp);

    handle_cleanup(editor);
}


