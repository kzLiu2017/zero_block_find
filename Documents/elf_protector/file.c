#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct code_cave_struct
{
    int size;
    int start;
    int vaddr;
    int end;
} code_cave_t;

int find_code_cave(int fd, code_cave_t *cc)
{
    char buf[128];
    struct stat file_info;
    int bytes_read, current_cave = 0;
    int bytes_processed = 0;
    int start = 1;
    int prev_offset = 0;
    int addr_start = 0;


    cc->size = 0;

    lseek(fd, 0, SEEK_SET);

    if ( fstat(fd, &file_info) )
    {
        puts("error fstat()");
        return -1;
    }
    
    printf("File Size->%d\n", file_info.st_size);

    /*
    *   Loop through the file, byte by byte looking for a string of 0's that we can use to inject code into.
    *   bytes_read = the amount of bytes read from recent read() call  
    *   ccs = struct code_cave_s which is used to store the details of the current code cave
    *   current_cave = the size of the current "code cave" we've found, if bigger than ccs.size we over the value at ccs.size
    *   start = a boolean flag to determine if we're at the start of a code cave (first 0 byte found), if yes, set start to 0 until we hit non-zero value
    *   prev_offset = previous read call offset, eg: if we read() 128 bytes twice, the file offset will be 256 the prev_file_offset will be 128.
    *   bytes_processed = the current number of bytes we've processed under the current read() call
    *   addr_start = starting address of code cave, this only gets assigned a value when start is 1 (first byte of code cave found)
    *   addr_start is determined by adding together the prev_offset value and the current bytes_processed
    *   eg: we call read() twice for 128 bytes each.
    *       prev_offset = 128
    *       code cave found 55 bytes_processed into the second read() call
    *       code cave start offset should be 128+55 = 183. (prev_offset + bytes_processed)
    *
    */
    printf("Current cc size %d\n", cc->size);
    bytes_read = read(fd, buf, sizeof(buf));
    while (bytes_read > 0)
    {
        for (int i=0;i<bytes_read;i++)
        {
            bytes_processed++;
            if (buf[i] == 0)
            {
                if (start)
                {
                    addr_start = ( prev_offset +  bytes_processed );
                    start--;
                }
                current_cave++;
            }
            else
            {
                if (current_cave > cc->size)
                {
                    cc->start = addr_start;
                    cc->size = current_cave;
                    cc->end = (prev_offset + bytes_processed );
                }
                current_cave = 0;
                start = 1;
            }

        }
            prev_offset += bytes_processed;
            bytes_processed = 0;
            bytes_read = read(fd, buf, sizeof(buf));
    }
}


int main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDONLY);
    code_cave_t cc;

    find_code_cave(fd, &cc);
    printf("code cave start point: %x, code cave end point: %x,  size: %d\n", cc.start, cc.end, cc.size);
}

