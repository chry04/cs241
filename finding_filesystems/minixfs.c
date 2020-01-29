/**
* Finding Filesystems Lab
* CS 241 - Fall 2018
*/

#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    if(!fs || !path)
    {
        return -1;
    }
    
    inode* file_inode = get_inode(fs, path);
    if(!file_inode)
    {
        errno = ENOENT;
        return -1;
    }

    int permission[3];
    int tmp = new_permissions;

    //printf("permissions: %d\n", new_permissions);
    
    for(int i = 0; i < 3; i++)
    {
        permission[2-i] = tmp%8;
        //printf("permission: %d\n", permission[2-i]);
        tmp = tmp/8;
    }

    file_inode -> mode = file_inode -> mode >> 9 << 9;

    for(int i = 0; i < 3; i++)
    {
        permission[i] <<= ((2-i)*3);
        file_inode -> mode = file_inode -> mode | permission[i];
    }

    clock_gettime(CLOCK_REALTIME, &(file_inode -> ctim));

    
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!    
    inode* file_inode = get_inode(fs, path);
    if(!file_inode)
    {
        errno = ENOENT;
        return -1;
    }

    if(owner != (uid_t) (-1))
    {
        file_inode -> uid = owner;
    }

    if(group != (gid_t) (-1))
    {
        file_inode -> gid = group;
    }

    clock_gettime(CLOCK_REALTIME, &(file_inode -> ctim));

    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* file_inode = get_inode(fs, path);
    if(file_inode)
    {
        return NULL;
    }

    inode_number index = first_unused_inode(fs);

    if(index == -1)
    {
        //printf("no inode avalible\n");
        return NULL;
    }

    inode* result = &(fs -> inode_root[index]);

    const char* name = "";

    inode* parent = parent_directory(fs, path, &name);
    init_inode(parent, result);

    if(valid_filename(name) != 1)
    {
        //printf("invalid path: %s\n", name);
        return NULL;
    }

    clock_gettime(CLOCK_REALTIME, &(parent -> mtim));
    
    minixfs_dirent info;
    info.name = strdup(name);
    info.inode_num = index;

    size_t block_num = (parent -> size) / (16 * KILOBYTE);
    size_t block_index = (parent -> size) % (16 * KILOBYTE);

    char* block = NULL;

    if(block_num <  NUM_DIRECT_INODES)
    {
        data_block_number direct_block = parent -> direct[block_num];

        block = (char*) (fs -> data_root[direct_block].data + block_index);
    }
    else
    {
        block_num -= NUM_DIRECT_INODES;

        data_block_number indirect_table = parent -> indirect;
        int indirect_block = (int) (fs -> data_root[indirect_table].data + 
                block_num*sizeof(data_block_number));

        block = (char*) (fs -> data_root[indirect_block].data + block_index);
    }

    make_string_from_dirent(block, info);
    parent->size += FILE_NAME_ENTRY;

    clock_gettime(CLOCK_REALTIME, &(result -> atim));
    clock_gettime(CLOCK_REALTIME, &(result -> mtim));
    clock_gettime(CLOCK_REALTIME, &(result -> ctim));

    //printf("finish creating inode\n");



    return result;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        char* map = GET_DATA_MAP(fs -> meta);

        ssize_t used_count = 0;
        for(uint64_t i = 0; i < fs -> meta -> dblock_count; i++)
        {
            if(map[i] == 1)
            {
                used_count++;
            }
        }

        char* tmp = (char*) block_info_string(used_count);

        long size = strlen(tmp);

        if(*off >= size)
        {
            return 0;
        }
        
        if(count + *off > (size_t)size)
        {
            count = size - *off;
        }        

        strncpy(buf+*off, tmp+*off, count);

        *off += count;    
        
        return count;
    }
    // TODO implement your own virtual file here
    errno = ENOENT;
    return -1;
}

int write_indirect_block(file_system *fs, const void *buf, inode* node,
                      size_t count, size_t indirect_i, size_t char_index)
{
    //printf("write_indirect\n");
    data_block_number* indirect_blocks = (data_block_number*) (fs->data_root+node -> indirect);
    data_block_number block_index = indirect_blocks[indirect_i];

    int i = 0;

    while((size_t)i < count)
    {
        
        if (count - i >= 16 * KILOBYTE)
        {
            if(char_index > 0)
            {
                memmove(fs->data_root[block_index].data+char_index, buf, 16 * KILOBYTE - char_index);

                i += (16 * KILOBYTE - char_index);
                char_index = 0;
            }
            else
            {
                memmove(fs->data_root[block_index].data, buf+i, 16 * KILOBYTE);
                i += (16 * KILOBYTE);
            }
        }
        else
        {
            if(char_index > 0)
            {
                memmove(fs->data_root[block_index].data+char_index, buf, count -i);

                i += (count-i);
                char_index = 0;
            }
            else
            {
                memmove(fs->data_root[block_index].data, buf+i, count - i);
                i += (count-i);
            }
        }


        indirect_i++;

        if(indirect_i >= NUM_INDIRECT_INODES)
            break;
        
        block_index = indirect_blocks[indirect_i];

    }

    return 0;

    
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
   // printf("writing\n");
    inode* file_inode = get_inode(fs, path);
    if(!file_inode)
    {
        file_inode = minixfs_create_inode_for_path(fs, path);
    }    

    size_t direct_block_size = NUM_DIRECT_INODES*16 * KILOBYTE;
    size_t total_file_size = direct_block_size +  NUM_INDIRECT_INODES *16 * KILOBYTE;

    if(*off + count > total_file_size)
    {
        errno = ENOSPC;
        return -1;
    }

    size_t total_block = (*off+count) / (16 * KILOBYTE);

    size_t direct_index = (*(size_t*) off)/(16 * KILOBYTE);
    size_t char_index = (*(size_t*) off)%(16 * KILOBYTE);

    int err = minixfs_min_blockcount(fs, path, total_block+1);

    if(err == -1)
    {
        errno = ENOSPC;
        return -1;
    }

    if(file_inode -> size < ((*off)+count))
    {
        file_inode -> size = (*off)+count;
    }



    if(direct_index < NUM_DIRECT_INODES)
    {
        size_t i = 0;

        int block_index = file_inode -> direct[direct_index];

        while(i < count)
        {
            if (count - i >= 16 * KILOBYTE)
            {
                if(char_index > 0)
                {
                    memmove(fs->data_root[block_index].data+char_index, buf, 16 * KILOBYTE - char_index);

                    i += (16 * KILOBYTE - char_index);
                    char_index = 0;
                }
                else
                {
                     memmove(fs->data_root[block_index].data, buf+i, 16 * KILOBYTE);
                     i += (16 * KILOBYTE);
                }
            }
            else
            {
                if(char_index > 0)
                {
                    memmove(fs->data_root[block_index].data+char_index, buf, count -i);

                    i += (count-i);
                    char_index = 0;
                }
                else
                {
                    memmove(fs->data_root[block_index].data, buf+i, count - i);
                    i += (count-i);
                }
            }

            

            direct_index++;

            if(direct_index >= NUM_DIRECT_INODES)
            {
                break;
            }

            block_index = file_inode -> direct[direct_index];

        }

        if(i == count)
        {
            clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));
            clock_gettime(CLOCK_REALTIME, &(file_inode -> mtim));

            (*off) += count;

            return count;
        }
        else
        {

            int result = write_indirect_block(fs, buf+i, file_inode, count-i, 0, 0);

            if(result == -1)
            {
                return -1;
            }

            (*off) += count;


            clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));
            clock_gettime(CLOCK_REALTIME, &(file_inode -> mtim));

            return count;
        }

    }
    else
    {
        int result = write_indirect_block(fs, buf, file_inode, count,
                direct_index-NUM_DIRECT_INODES, char_index);

        if(result == -1)
        {
            return -1;
        }

        (*off) += count;


        clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));
        clock_gettime(CLOCK_REALTIME, &(file_inode -> mtim));
        //file_inode -> mtim = current_time;

        
        return count;

    }

    
    return -1;
}


int read_indirect_block(file_system *fs, void *buf, inode* node,
                      size_t count, size_t indirect_i, size_t char_index)
{
    //printf("read indirect\n");
    data_block_number* indirect_blocks = (data_block_number*) (fs->data_root + node->indirect);
    data_block_number block_index = indirect_blocks[indirect_i];

    int i = 0;

    while((size_t) i < count && block_index != -1)
    {
        if (count - i >= 16 * KILOBYTE)
        {
            if(char_index > 0)
            {
                memmove(buf, fs->data_root[block_index].data+char_index, 16 * KILOBYTE - char_index);

                i += (16 * KILOBYTE - char_index);
                char_index = 0;
            }
            else
            {
                memmove(buf+i, fs->data_root[block_index].data, 16 * KILOBYTE);
                i += (16 * KILOBYTE);
            }
        }
        else
        {
            if(char_index > 0)
            {
                memmove(buf, fs->data_root[block_index].data+char_index, count -i);

                i += (count-i);
                char_index = 0;
            }
            else
            {
                memmove(buf+i, fs->data_root[block_index].data, count - i);
                i += (count-i);
            }
        }


        indirect_i++;

        if( indirect_i >= NUM_INDIRECT_INODES)
        {
            break;
        }

        block_index = indirect_blocks[indirect_i];

        //printf("indirect index: %zu, counter: %zu, block index: %d\n", indirect_i, current_i, block_index);

    }

    
    return 0;

    
}


ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
    {
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
        printf("end\n");
    }
    // 'ere be treasure!
    inode* file_inode = get_inode(fs, path);
    if(!file_inode)
    {
        //printf("invalid inode\n");
        errno = ENOENT;
        return -1;
    }

    //printf("count: %zu\n", count);
    if(*off + count > file_inode -> size)
    {
        if ((long)(file_inode->size) - (int)*off < 0)
        {
            count = 0;
        }
        else
        {
            count = (file_inode->size) - *off;
        }
        //printf("count after: %zu\n", count);
    }

    //printf("offset: %lu\n", *off);

    size_t direct_index = (*(size_t*) off)/(16 * KILOBYTE);
    //printf("direct index: %zu\n", direct_index);

    size_t char_index = (*(size_t*) off)%(16 * KILOBYTE);
    //printf("char index: %zu\n", char_index);


    if(direct_index < NUM_DIRECT_INODES)
    {
        size_t i = 0;

        size_t block_index = file_inode -> direct[direct_index];

        while(direct_index < NUM_DIRECT_INODES && i < count)
        {
            if (count - i >= 16 * KILOBYTE)
            {
                if(char_index > 0)
                {
                    memmove(buf, fs->data_root[block_index].data+char_index, 16 * KILOBYTE - char_index);

                    i += (16 * KILOBYTE - char_index);
                    char_index = 0;
                }
                else
                {
                    memmove(buf+i, fs->data_root[block_index].data, 16 * KILOBYTE);
                    i += (16 * KILOBYTE);
                }
            }
            else
            {
                if(char_index > 0)
                {
                    memmove(buf, fs->data_root[block_index].data+char_index, count -i);

                    i += (count-i);
                    char_index = 0;
                }
                else
                {
                    memmove(buf+i, fs->data_root[block_index].data, count - i);
                    i += (count-i);
                }
            }


            direct_index++;

            if(direct_index >= NUM_DIRECT_INODES)
            {
                break;
            }

            block_index = file_inode -> direct[direct_index];

        }

        //printf("i: %zu\n", i);


        if(i == count)
        {
            //printf("\n\nend\n\n");
            clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));
            (*off) += count;
            return count;
        }
        else
        {
            //printf("\n\nread indirect\n\n");
            read_indirect_block(fs, buf+i, file_inode, count-i, 0, 0);


            (*off) += count;

            clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));
            
            //printf("end reading indirect\n");

            return count;
        }

    }
    else
    {
        read_indirect_block(fs, buf, file_inode, count,
                direct_index - NUM_DIRECT_INODES, char_index);

        (*off) += count;
        
        clock_gettime(CLOCK_REALTIME, &(file_inode -> atim));

        return count;

    }

    return -1;
}
