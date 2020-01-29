/**
* Malloc Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _data_list
{
    size_t size;
    int free;
    struct _data_list* next;
    struct _data_list* prev;
}data;

typedef struct _tag
{
    size_t size;
}tag;

static data* head = NULL;
static data* tail = NULL;

static void* start = NULL;
static void* end = NULL;

static int first = 1;


void split(data* tmp, size_t size)
{
    //fprintf(stderr, "spliting\n");
    tag* end_tag = (tag*) ((void*)tmp + size+ sizeof(data));
    //fprintf(stderr, "end tag: %p\n", end_tag);

    end_tag -> size = size;
    
    //fprintf(stderr, "end tag\n");
    size_t next_size = tmp -> size - size - sizeof(data) - sizeof(tag);
    data* next_block = (data*)((void*) end_tag + sizeof(tag));

    //fprintf(stderr, "splited block: %p\n", next_block);
    next_block -> size = next_size;
    next_block -> next = NULL;
    next_block -> prev = tail;
    next_block -> free = 1;
    //fprintf(stderr, "next_block\n");


    if(tail)                
    {
        //fprintf(stderr, "tail\n");
        tail -> next = next_block;
        tail = next_block;
    }
    else if(!tail && !head)
    {
        //fprintf(stderr, "not head or tail\n");
        head = next_block;
        tail = next_block;
    }

    //fprintf(stderr, "next size: %zu\n", next_size);
                
    tag* next_tag = (tag*) ((void*) (next_block+1) + next_size);
    next_tag -> size = next_size;

    //fprintf(stderr, "splitted\n");
                
    tmp -> size = size;

    //return next_block;
}

void reconnect_free_pointer(data* tmp, size_t size)
{
    //fprintf(stderr, "reconnecting\n");
    tmp -> free = 0;
    if(tmp == tail && tmp == head)
    {
        //fprintf(stderr, "head&tail\n");
        head = NULL;
        tail = NULL;
    }
    else if(tmp == head && tmp != tail)
    {
        //fprintf(stderr, "head\n");
        tmp -> next -> prev = NULL;
        //fprintf(stderr, "head has next\n");

        head = tmp -> next;
    }
    else if(tmp == tail && tmp != head)
    {
        //fprintf(stderr, "tail\n");
        tmp -> prev -> next = NULL;
        tail = tmp -> prev;
    }
    else
    {
        //fprintf(stderr, "middle\n");
        tmp -> prev -> next = tmp -> next;
        //fprintf(stderr, "middle has prev\n");
        tmp -> next -> prev = tmp -> prev;
        //fprintf(stderr, "middle has next\n");
    }

    tmp -> prev = NULL;
    tmp -> next = NULL;

    //fprintf(stderr, "pointers are reconnected\n");
    //
    //data* result = NULL;

    if((tmp -> size - size) > (sizeof(data) + sizeof(tag) + 4))
    {
        split(tmp, size);
    }

    //fprintf(stderr, "pointers are splited\n");

    //return result;

}

void* create_new_pointer(size_t size)
{
    data* node = sbrk(sizeof(data)+size+sizeof(tag));
    end = ((void*) node) + sizeof(data)+size+sizeof(tag);
    node -> size = size;
    node -> free = 0;
    node -> prev = NULL;
    node -> next = NULL;

    //fprintf(stderr, "block created: %p\n", node);


    tag* end_tag = (tag*)((void*)(node+1) + size);
    end_tag -> size = size;

    return (void*)(node+1);

}

void merge_next(data* tmp, data* next_block, tag* next_tag)
{
   // fprintf(stderr, "next block is free\n");
    if(next_block == head && next_block == tail)
    {
       // fprintf(stderr, "head&tail\n");
        head = tmp;
        tail = tmp;
    }
    else if(next_block == head)
    {
        //fprintf(stderr, "head\n");
        tmp -> next = next_block -> next;
        next_block -> next -> prev = tmp;
        head = tmp;
    }
    else if(next_block == tail)
    {
        //fprintf(stderr, "tail\n");
        tmp -> prev = next_block -> prev;
        next_block -> prev -> next = tmp;
        tail = tmp;
    }
    else 
    {
        //fprintf(stderr, "neither\n");
        next_block -> prev -> next = tmp;
        //fprintf(stderr, "next block has prev\n");
        next_block -> next -> prev = tmp;
        //fprintf(stderr, "next block has next\n");

        tmp -> prev = next_block -> prev;
        tmp -> next = next_block -> next;
    }

    tmp -> size = tmp->size + sizeof(data) + sizeof(tag) + next_block -> size;
    next_tag -> size = tmp -> size;

    //fprintf(stderr, "block_size: %zu, tag position: %p\n", next_tag -> size, next_tag);

}

//void *malloc(size_t size);

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    
    if(first)
    {
        first = 0;
        start = sbrk(0);

        //fprintf(stderr, "first: %p\n", start);
    }
    
    //fprintf(stderr, "malloc\n");
    data* tmp = head;

    while (tmp)
    {
        if(tmp->size >= num*size)
        {
            //fprintf(stderr, "find block: %p\n", tmp);
            reconnect_free_pointer(tmp, num*size);

            void* result = (void*)(tmp+1);
            memset(result, 0, num*size);

            return result;
        }
        tmp = tmp -> next;
    }

    void* result = create_new_pointer(num*size);


    memset(result, 0, num*size);

    return result;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!

    //check if there is data that is fit.
    //
    if(first)
    {
        first = 0;
        start = sbrk(0);

        //fprintf(stderr, "first: %p\n", start);
    }
    
    //fprintf(stderr, "malloc\n");
    data* tmp = head;

    //fprintf(stderr, "tmp?\n");

    while (tmp)
    {
        //fprintf(stderr, "looping tmp\n");
        //fprintf(stderr, "tmp size: %zu\n", tmp->size );

        if(tmp->size >= size)
        {
            //fprintf(stderr, "find block: %p\n", tmp);
            
            reconnect_free_pointer(tmp, size);
            //fprintf(stderr, "split block: %p\n", tmp);
            

            return (void*)(tmp+1);
        }
        tmp = tmp -> next;
    }

    //fprintf(stderr, "not find\n");



    return create_new_pointer(size);
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (!ptr)
    {
        //fprintf(stderr, "not exits\n");
        return;
    }

    //fprintf(stderr, "free\n");

    data* tmp = ((data*) ptr) - 1;
    tmp -> free = 1;
    
    
    data* prev_block = NULL;
    size_t prev_size = 0;
    tag* prev_tag = (tag*)(((void*) tmp) - sizeof(tag));


    size_t tags = sizeof(tag) + sizeof(data);

    if((void*)prev_tag >= start && (void*)prev_tag < end)
    {
        prev_size = prev_tag -> size;
        //fprintf(stderr, "prev size = %zu, prev tag: %p\n", prev_size, prev_tag);
    }

    if(prev_size != 0)
    {
        prev_block = (data*) (((void*) tmp) - tags - prev_size);
        //fprintf(stderr, "assign prev: %p\n", ((void*) tmp) - tags - prev_size);
    }

    data* next_block = (data*) (ptr + tmp -> size + sizeof(tag));
    size_t next_size = 0;
    tag* next_tag = NULL;
    
    if(!((void*)next_block >= start && (void*)next_block < end))
    {
        next_block = NULL;
    }

    //fprintf(stderr, "assign next: %p\n", ptr + tmp -> size + sizeof(tag));

    
    if(next_block)
    {
        next_size = next_block -> size;
        next_tag = (tag*)(ptr + tmp -> size + tags + next_block -> size);
        //fprintf(stderr, "next size = %zu, next tag: %p\n", next_size, next_tag);
    }


    //fprintf(stderr, "begin free\n");
    
    if(prev_block && prev_block -> free)
    {
        //fprintf(stderr, "prev block is free\n");
        if(!(next_block && next_block -> free))
        {
            //fprintf(stderr, "next block is not free\n");
            
            prev_block -> size = prev_block -> size + tags + tmp -> size;
            tag* new_tag = (tag*) (ptr + tmp->size);
            new_tag -> size = prev_block -> size;
            //fprintf(stderr, "block_size: %zu\n", prev_block -> size);

        }
        else
        {
            //fprintf(stderr, "next block is free\n");

            if (next_block == head && next_block != tail)
            {
                //fprintf(stderr, "head\n");
                next_block -> next -> prev = NULL;
                head = next_block -> next;
            }
            else if(next_block != head && next_block == tail)
            {
                //fprintf(stderr, "tail\n");
                next_block -> prev -> next = NULL;
                tail = next_block -> prev;
            }
            else if(next_block != head && next_block != tail)
            {
                //fprintf(stderr, "middle\n");
                next_block -> prev -> next = next_block -> next;
                //fprintf(stderr, "has prev\n");
                next_block -> next -> prev = next_block -> prev;
                //fprintf(stderr, "has next\n");
            }

            prev_block -> size = prev_block -> size + tags*2 + tmp -> size + next_block->size;
            next_tag -> size = prev_block -> size;
            //fprintf(stderr, "block_size: %zu\n", prev_block -> size);

        }

        
        return;
    }
    else if(next_block && next_block -> free)
    {
        merge_next(tmp, next_block, next_tag);
    }
    else
    {
        //fprintf(stderr, "no block is free\n");
        if (!head && !tail)
        {
            head = tmp;
            tail = tmp;
        }
        else
        {
            tail -> next = tmp;
            tmp -> prev = tail;
            tail = tmp;
        }

        //fprintf(stderr, "block_size: %zu\n", tmp -> size);
    }


    return;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    //fprintf(stderr, "realloc\n");
    if(!ptr)
        return malloc(size);

    //fprintf(stderr, "realloc\n");

    data* entry = ((data*) ptr) -1;

    size_t old_size = 0;

    if(size >= entry->size)
    {
        old_size = entry->size;
    }
    else
    {
        old_size = size;
    }
        
    void* result = malloc(size);
    memcpy(result, ptr, old_size);
    free(ptr);

    return result;
}
