/**
* Text Editor Lab
* CS 241 - Fall 2018
*/

#include "document.h"
#include "editor.h"
#include "format.h"
#include "sstring.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_filename(int argc, char *argv[]) {
    // TODO implement get_filename
    // take a look at editor_main.c to see what this is used for
    return argv[argc-1];
}

sstring *handle_create_string() {
    // TODO create empty string
    sstring* result = cstr_to_sstring("");
    return result;
}
document *handle_create_document(const char *path_to_file) {
    // TODO create the document
    document* result = document_create_from_file(path_to_file);
    return result;
}

void handle_cleanup(editor *editor) {
    // TODO destroy the document
    document_destroy(editor->document);
    sstring_destroy(editor->string);
}

void handle_display_command(editor *editor, size_t start_line,
                            ssize_t max_lines, size_t start_col_index,
                            ssize_t max_cols) {
    // TODO implement handle_display_command
    if(document_size(editor->document) == 0 || start_line == 0)
    {
        print_document_empty_error();
    }
    
    size_t lines = document_size(editor->document);

    if(!(lines < start_line+(size_t)max_lines || max_lines == -1))
    {
        lines = start_line + (size_t) max_lines -1;
    }
	
    for(size_t i = start_line; i <= lines; i++)
    {
        //char* line = document_get_line(editor->document, i);

        print_line(editor->document, i, start_col_index, max_cols);
    }
}

void handle_insert_command(editor *editor, location loc, const char *line) {
    // TODO implement handle_insert_command
    if (loc.line_no > document_size(editor->document))
    {
            document_insert_line(editor->document, loc.line_no, "");
    }
    
    const char* oldline = document_get_line(editor->document, loc.line_no);

    assert(0 <= loc.idx && loc.idx <= strlen(oldline));

    char* newline = (char*) malloc(strlen(oldline)+strlen(line)+1);
    
    strncpy(newline, oldline, loc.idx);
    strncpy(newline+loc.idx, line, strlen(line));
    strncpy(newline+loc.idx+(int)strlen(line), oldline+loc.idx, strlen(oldline)-loc.idx);
    newline[strlen(oldline)+strlen(line)] = '\0';
    
    document_set_line(editor->document, loc.line_no, newline);
    free(newline);
}

void handle_append_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_append_command
    assert(line);
    assert(line_no>=1);

    int set = 0;
    //int inst = 0;

    if (line_no > document_size(editor->document))
    {
        document_insert_line(editor->document, line_no, "");
    }

    //fprintf(stderr, "length: %lu\n", strlen(document_get_line(editor->document, line_no)));
    //fprintf(stderr, "set: %d\n", set);

    char* insert = (char*) malloc(strlen(line)+1);
    int decrease = 0;
    //int size = 0;
    size_t lines = 0;

    const char* old_line = document_get_line(editor->document, line_no);
    
    for(size_t i = 0; i < strlen(line); i++)
    {
        if(line[i] == '\\')
        {
            if(line[i+1] == 'n' && i > 0)
            {                
                //fprintf(stderr, "set: %d\n", set);
                insert = (char*) realloc(insert, i-decrease+1);
                //strncpy(insert, line+i-size-1, size);
                insert[i-decrease] = '\0';
                if(lines > 0)
                {
                  //fprintf(stderr, "%zu, insert: %s\n", line_no+lines, insert);

                    document_insert_line(editor->document, line_no+lines, insert);
                    free(insert);
                }
                else
                {
                    //fprintf(stderr, "inside\n");
                    //fprintf(stderr, "set: %zu\n", document_size(editor->document));
                    insert[i-decrease] = '\0';
                    char* replace = (char*) malloc(i-decrease+strlen(old_line)+1);
                    
                    replace[0] = '\0';
                    strcat(replace, old_line);
                    replace[strlen(old_line)] = '\0';
                    strcat(replace, insert);
                    replace[i-decrease+strlen(old_line)] = '\0';
                    free(insert);
                    
                    document_set_line(editor->document, line_no+lines, replace);
                    free(replace);
                    //free(insert);
                    set = 0;
                    //inst = 1;
                }
                
                //free(insert);
                insert = (char*) malloc(strlen(line)+1-i);
                lines++;
                decrease = i+2;
                i++;
                //size = 0;
            }
            else if(line[i+1] == 'n' && i == 0)
            {
                lines++;
                decrease = 2;
                i++;
                set = 0;
            }
            else if(line[i+1] == '\\')
            {
                insert[i-decrease] = line[i];
                decrease++;
                i++;
                //size++;
            }
            else
            {
                decrease++;
            }

            continue;
        }
        else
        {
            insert[i-decrease] = line[i];
            //size++;
        }
        
    }
    insert[strlen(line)-decrease] = '\0';

    //fprintf(stderr, "insert: %s\n", insert);


    if(lines > 0)
    {
        //fprintf(stderr, "insert: %s\n", insert);
        document_insert_line(editor->document, line_no+lines, insert);
        free(insert);
        //fprintf(stderr, "inserted: %s\n", document_get_line(editor->document, line_no+lines));
       
    }
    else
    {
        char* replace = (char*) malloc(strlen(old_line)-decrease+strlen(line)+1);
                    
        replace[0] = '\0';
        strcat(replace, old_line);
        replace[strlen(old_line)] = '\0';
        //fprintf(stderr, "null:%d, replace1: %s\n", (int)strlen(old_line), replace);

        strcat(replace, insert);
        replace[strlen(line)-decrease+strlen(old_line)] = '\0';
        //fprintf(stderr, "null:%d, replace2: %s\n", (int)(strlen(line)-decrease+strlen(old_line)), replace);

        free(insert);
                    
        document_set_line(editor->document, line_no+lines, replace);
        free(replace);
     }
}

void handle_write_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_write_command
    assert(line);
    assert(line_no >= 1);

    //printf("size in write: %zu\n", document_size(editor->document));
    
    if (line_no > document_size(editor->document))
    {
        document_insert_line(editor->document, line_no, "");
    }

    char* insert = (char*) malloc(strlen(line)+1);
    int decrease = 0;
    //int size = 0;
    size_t lines = 0;
    int start = 0;
    
    for(size_t i = 0; i < strlen(line); i++)
    {
        if(line[i] == '\\')
        {
            if(line[i+1] == 'n' && i > 0)
            {
                insert = (char*) realloc(insert, i-decrease+1);
                //strncpy(insert, line+i-size-1, size);
                insert[i-decrease] = '\0';
                document_set_line(editor->document, line_no, insert);
                
                //free(insert);
                //insert = (char*) malloc(strlen(line)+1-i);
                lines++;
                decrease = i+2;
                start = i+2;
                break;
                //size = 0;
            }
            else if(line[i+1] == 'n' && i == 0)
            {
                lines++;
                document_set_line(editor->document, line_no, "");
                decrease = 2;
                start = 2;
                i++;
                break;
            }
            else if(line[i+1] == '\\')
            {
                insert[i-decrease] = line[i];
                decrease++;
                i++;
            }
            else
            {
                decrease++;
            }
            continue;
        }
        else
        {
            insert[i-decrease] = line[i];
        }
    }

    free(insert);
    insert = (char*) malloc(strlen(line)+1-start);

    for(size_t i = start; i < strlen(line); i++)
    {
        if(line[i] == '\\')
        {
            if(line[i+1] == 'n' && i > 0)
            {
                
                insert = (char*) realloc(insert, i-decrease+1);
                //strncpy(insert, line+i-size-1, size);
                insert[i-decrease] = '\0';
                //fprintf(stderr, "lines: %zu, insert: %s\n", line_no+lines, strdup(insert));

                document_insert_line(editor->document, line_no+lines, insert);
                
                free(insert);
                insert = (char*) malloc(strlen(line)+1-i);
                //fprintf(stderr, "before lines: %zu, insert: %s, %zu\n", line_no+lines, insert, i-decrease);

                lines++;
                decrease = i+2;
                i++;
                //start = i+2;
                //size = 0;
            }
            else if(line[i+1] == '\\')
            {
                insert[i-decrease] = line[i];
                decrease++;
                i++;
            }
            else
            {
                decrease++;
            }
            continue;
        }
        else
        {
            insert[i-decrease] = line[i];
            //fprintf(stderr, "i: %zu, i-decrease: %zu, insert: %c", i, i-decrease, line[i]);
        }
    }

    
    insert[strlen(line)-decrease] = '\0';
    //fprintf(stderr, "after lines: %zu, insert: %s\n", line_no+lines, insert);

    if(start > 0)
    {
        document_insert_line(editor->document, line_no+lines, insert);
    }
    else
    {
        document_set_line(editor->document, line_no+lines, insert);
    }
    free(insert);

    //fprintf(stderr, "%zu, %d, %s\n", line_no+1, start, line+start);

}

void handle_delete_command(editor *editor, location loc, size_t num_chars) {
    // TODO implement handle_delete_command
    assert(editor);
    assert(loc.line_no >= 1);

    const char* oldline = document_get_line(editor->document, loc.line_no);

    if (strlen(oldline) - loc.idx <= num_chars)
    {
        document_set_line(editor->document, loc.line_no, "");
        return;
    }
    else if(loc.line_no > document_size(editor->document))
    {
        return;
    }

    char* newline = (char*) malloc(strlen(oldline)-num_chars+1);

    int end = loc.idx;

    if(loc.idx > strlen(oldline))
    {
        end = strlen(oldline);
    }
    
    strncpy(newline, oldline, end);
    newline[loc.idx] = '\0';
    //fprintf(stderr, "before: %s\n", newline);
    strncpy(newline+end, oldline+end+num_chars, strlen(oldline)-end-num_chars);
    newline[strlen(oldline)-num_chars] = '\0';
    //fprintf(stderr, "after: %s\n", newline);

    
    document_set_line(editor->document, loc.line_no, newline);
    free(newline);

}

void handle_delete_line(editor *editor, size_t line_no) {
    // TODO implement handle_delete_line
    if(line_no > document_size(editor->document))
        return;
    document_delete_line(editor->document, line_no);
}

location handle_search_command(editor *editor, location loc,
                               const char *search_str) {
    // TODO implement handle_search_command
    if(strcmp(search_str, "") == 0)
    {
        return (location){0, 0};
    }

    if(document_size(editor->document) == 0)
        return (location) {0, 0};

    size_t start = loc.line_no;
    
    
    if(loc.line_no > document_size(editor->document))
    {
        start = 1;
    }

    const char* begin = document_get_line(editor->document, loc.line_no);
    if (loc.idx <= strlen(begin))
    {
        char* found = strstr(begin+(int)loc.idx, search_str);

        if(found)
        {
            return (location){start, found-begin};
        }
        start++;
    }
    else
    {
        start++;
    }
    

        
    for(size_t i = start; i <= document_size(editor->document); i++)
    {
        //fprintf(stderr, "in loop\n");
        const char* line = document_get_line(editor->document, i);
        char* found = strstr(line, search_str);

        if(found)
        {
            return (location){i, found-line};
        }
    }

    for(size_t i = 1; i < start; i++)
    {
        const char* line = document_get_line(editor->document, i);
        char* found = strstr(line, search_str);

        if(found)
        {
            return (location){i, found-line};
        }

    }
    
    return (location){0, 0};
}

void handle_merge_line(editor *editor, size_t line_no) {
    // TODO implement handle_merge_line
    assert(line_no <= document_size(editor->document) && line_no >= 1);
    
    const char* line = document_get_line(editor->document, line_no);
    const char* follow = document_get_line(editor->document, line_no+1);

    char* merge = (char*)malloc(strlen(line)+strlen(follow)+1);
    merge[0] = '\0';
    strcat(merge, line);
    strcat(merge, follow);

    document_set_line(editor->document, line_no, merge);
    document_delete_line(editor->document, line_no+1);
    free(merge);
}

void handle_split_line(editor *editor, location loc) {
    // TODO implement handle_split_line
    const char* line = document_get_line(editor->document, loc.line_no);
    char* first = (char*) malloc(loc.idx+1);
    char* second = (char*) malloc(strlen(line)-loc.idx+1);

    strncpy(first, line, (int)loc.idx);
    strncpy(second, line+(int)loc.idx, (int)strlen(line)-loc.idx);
    first[loc.idx] = '\0';
    second[strlen(line)-loc.idx] = '\0';

    document_set_line(editor->document, loc.line_no, first);
    document_insert_line(editor->document, loc.line_no+1, second);

    free(first);
    free(second);

}

void handle_save_command(editor *editor) {
    // TODO implement handle_save_command
    document_write_to_file(editor->document, editor->filename);
}
