
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

char *trace_file_name;

uint64_t convert_address(char memory_addr[])
/* Converts the physical 32-bit address in the trace file to the "binary" \\
 * (a uint64 that can have bitwise operations on it) */
{
    uint64_t binary = 0;
    int i = 0;

    while (memory_addr[i] != '\n')
    {
        if (memory_addr[i] <= '9' && memory_addr[i] >= '0')
        {
            binary = (binary * 16) + (memory_addr[i] - '0');
        }
        else
        {
            if (memory_addr[i] == 'a' || memory_addr[i] == 'A')
            {
                binary = (binary * 16) + 10;
            }
            if (memory_addr[i] == 'b' || memory_addr[i] == 'B')
            {
                binary = (binary * 16) + 11;
            }
            if (memory_addr[i] == 'c' || memory_addr[i] == 'C')
            {
                binary = (binary * 16) + 12;
            }
            if (memory_addr[i] == 'd' || memory_addr[i] == 'D')
            {
                binary = (binary * 16) + 13;
            }
            if (memory_addr[i] == 'e' || memory_addr[i] == 'E')
            {
                binary = (binary * 16) + 14;
            }
            if (memory_addr[i] == 'f' || memory_addr[i] == 'F')
            {
                binary = (binary * 16) + 15;
            }
        }
        i++;
    }

#ifdef DBG
    printf("%s converted to %llu\n", memory_addr, binary);
#endif
    return binary;
}

void getting_hit_miss_rate(int total_blocks, int nway, int block_size)
{
    
    int no_sets = total_blocks / nway;
    struct direct_mapped_cache
    {
        unsigned valid_field[total_blocks]; /* Valid field */
        unsigned dirty_field[total_blocks]; /* Dirty field; since we don't distinguish writes and \\
                                             reads in this project yet, this field doesn't really matter */
        uint64_t tag_field[total_blocks];   /* Tag field */
        int hits;                                  /* Hit count */
        int misses;                                /* Miss count */
    };
    struct direct_mapped_cache d_cache;

    for (int i = 0; i < total_blocks; i++)
    {
        d_cache.valid_field[i] = 0;
        d_cache.dirty_field[i] = 0;
        d_cache.tag_field[i] = 0;
    }
    d_cache.hits = 0;
    d_cache.misses = 0;

    char mem_req[20];
    FILE *file_p;
    file_p = fopen(trace_file_name, "r");
    uint64_t addrss;

  
    while (fgets(mem_req, 20, file_p) != NULL)
    {
        addrss = convert_address(mem_req);
        uint64_t block_addr = addrss >> (unsigned)log2(block_size);
        int setNumber = block_addr % no_sets;
        uint64_t tag = block_addr >> (unsigned)log2(no_sets);
        int startIndex = ((int)setNumber) * nway;

        int endIndex = startIndex + nway - 1;

        int hitMade = 0;
        int isAnySpaceEmpty = 0;
        int nway_Temp = nway;
        int loopIndex = startIndex;
        int i = 0;
        
        
        while (nway_Temp > 0)
        {
            i++;
            
            if (d_cache.valid_field[loopIndex] && d_cache.tag_field[loopIndex] == tag)
            { 
                // Cache hit 
                d_cache.hits += 1;
                hitMade = 1;
                break;
            }
            if (d_cache.valid_field[loopIndex] == 0)
            {
                isAnySpaceEmpty = 1;
            }

            loopIndex += 1;
            nway_Temp--;
        }

        if (hitMade == 0)
        {
            
            d_cache.misses += 1;
            if (isAnySpaceEmpty > 0)
            {
                
                nway_Temp = nway;
                loopIndex = startIndex;
                while (nway_Temp > 0)
                {
                    if (d_cache.valid_field[loopIndex] == 0)
                    {
                        
                        d_cache.valid_field[loopIndex] = 1;
                        d_cache.tag_field[loopIndex] = tag;
                        break;
                    }

                    loopIndex += 1;
                    nway_Temp--;
                }
            }
            else
            {
                

                int randomIndex = (rand() % (endIndex - startIndex + 1)) + startIndex;
                
                d_cache.valid_field[randomIndex] = 1;
                d_cache.tag_field[randomIndex] = tag;
            }
        }
    }
    
    printf("==================================\n");
    printf("Cache Hits:    %d\n", d_cache.hits);
    printf("Cache Misses:  %d\n", d_cache.misses);
    printf("Cache Hit Rate : %0.9f %%\n", ((float)d_cache.hits / (float)(d_cache.hits + d_cache.misses))*100);
    printf("Cache Miss Rate : %0.9f %%\n", ((float)d_cache.misses / (float)(d_cache.hits + d_cache.misses))*100);
    printf("\n");
    fclose(file_p);
}

void get_hit(int cacheSize, int total_blocks, int nway, int block_size)
{
    
    getting_hit_miss_rate(total_blocks, nway, block_size);
}

void nway_part_1()
{
    int to_continue = -1;
    do
    {
    long cacheSize = 32 * 1024;
    int total_blocks = 0;
    int cs = 0;
    printf("For 32KB Cache size:  Enter 1 for 16 bytes cache line size,  2 for 32 bytes cache line size, 3 for 128 bytes cache line size : \n");
    scanf("%d", &cs);
    int nway_select = 0;
    printf("Enter 1 for fully associative execution, 2 for 2 way execution, 3 for 4 way execution, 4 for 8 way execution:\n ");
    scanf("%d", &nway_select);
    switch (cs)
    {
    case 1:
        printf("\n\n32kb cache size and 16 bytes cache line size/block size ========== \n");
        total_blocks = (int)cacheSize / 16;
        switch (nway_select)
        {
        case 1:
            printf("for Fully Associative Execution\n");
            get_hit(32, total_blocks, total_blocks, 16);
            break;
        case 2:
            printf("for 2 way Execution\n");
            get_hit(32, total_blocks, 2, 16);
            break;
        case 3:
            printf("for 4 way Execution\n");
            get_hit(32, total_blocks, 4, 16);
            break;
        case 4:
            printf("for 8 way Execution\n");
            get_hit(32, total_blocks, 8, 16);
            break;
        default:
            break;
        }

        break;
    case 2:
        printf("\n\n 32kb cache size and 32 bytes cache line size/block size========== \n");
        total_blocks = (int)cacheSize / 32;
        switch (nway_select)
        {
        case 1:
            printf("for Fully Associative Execution\n");
            get_hit(32, total_blocks, total_blocks, 32);
            break;
        case 2:
            printf("for 2 way Execution\n");
            get_hit(32, total_blocks, 2, 32);
            break;
        case 3:
            printf("for 4 way Execution\n");
            get_hit(32, total_blocks, 4, 32);
            break;
        case 4:
            printf("for 8 way Execution\n");
            get_hit(32, total_blocks, 8, 32);
            break;
        
       
        
        default:
            break;
        }

        break;
    case 3:
        printf("\n\n32kb cache size and 128 bytes cache line size/block size ========== \n");
        total_blocks = (int)cacheSize / 128;
        switch (nway_select)
        {
        case 1:
            printf("for Fully Associative Execution\n");
            get_hit(32, total_blocks, total_blocks, 1628);
            break;
        case 2:
            printf("for 2 way Execution\n");
            get_hit(32, total_blocks, 2, 128);
            break;
        case 3:
            printf("for 4 way Execution\n");
            get_hit(32, total_blocks, 4, 128);
            break;
        case 4:
            printf("for 8 way Execution\n");
            get_hit(32, total_blocks, 8, 128);
            break;
        
        
        
        default:
            break;
        }

        break;
    }

        printf("Enter 1 to continue the test cases, -1 to exit:\n ");
        scanf("%d",&to_continue);
    } while (to_continue==1);
    

   
}
void nway_part_2()
{

     int to_continue = -1;
    do
    {
 long cacheSize = 16 * 1024;
    int total_blocks = 0;
    int cs = 0;
    printf("64Bytes Cache line size ,Click 1 for 16KB Cachesize, 2 for 32KB Cachesize, 3 for 64KB Cachesize  \n");
    scanf("%d", &cs);
    int nway_select;
    printf("enter 1 for fully associative execution, 2 for 2 way execution,3 for 4 way execution,4 for 8 way execution:\n ");
    scanf("%d", &nway_select);
 
        switch (cs)
        {
        case 1:
            total_blocks = (int)cacheSize / 64;
            printf("\n\n16kb cache size and 64 bytes cache line size/block size=======\n");
            switch (nway_select)
            {
            
            case 1:
                printf("for Fully Associative Execution\n");
                get_hit(16, total_blocks, total_blocks, 64);
                break;
            case 2:
                printf("for 2 way Execution\n");
                get_hit(16, total_blocks, 2, 64);
                break;
            case 3:
                printf("for 4 way Execution\n");
                get_hit(16, total_blocks, 4, 64);
                break;
            case 4:
                printf("for 8 way Execution\n");
                get_hit(16, total_blocks, 8, 64);
                break;

            default:
                break;
            }

            break;
        case 2:
            printf("\n\n32kb cache size and 64 bytes cache line size/block size=======\n");
            cacheSize = 32 * 1024;
            total_blocks = (int)cacheSize / 64;
            switch (nway_select)
            {
            case 1:
                printf("for Fully Associative Execution\n");
                get_hit(32, total_blocks, total_blocks, 64);
                break;
            case 2:
                 printf("for 2 way Execution\n");
                get_hit(32, total_blocks, 2, 64);
                break;
            case 3:
                 printf("for 4 way Execution\n");
                get_hit(32, total_blocks, 4, 64);
                break;
            case 4:
                printf("for 8 way Execution\n");
                get_hit(32, total_blocks, 8, 64);
                break;
            
            
            

            default:
                break;
            }
            break;
        case 3:
            printf("\n\n64kb cache size and 64 bytes cache line size/block size=======\n");
            cacheSize = 64 * 1024;
            total_blocks = (int)cacheSize / 64;
            switch (nway_select)
            {
            case 8:
                printf("for 8 way Execution\n");
                get_hit(64, total_blocks, 8, 64);
                break;
            case 4:
                printf("for 4 way Execution\n");
                get_hit(64, total_blocks, 4, 64);
                break;
            case 2:
                printf("for 2 way Execution\n");
                get_hit(64, total_blocks, 2, 64);
                break;
            case 1:
                printf("for Fully Associative Execution\n");
                get_hit(64, total_blocks, total_blocks, 64);
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }


        printf("Select 1 to continue the test cases, -1 to exit:\n ");
        scanf("%d",&to_continue);
    } while (to_continue==1);


   
}

int main(int argc, char *argv[])
{
    trace_file_name = argv[2];
    int switchCase = 0;
    printf("Select 1 to  execute Question 1 or 2 to Question 2 :\n");
    scanf("%d", &switchCase);

    switch (switchCase)
    {
    case 1:
        nway_part_1();
        break;

    case 2:
        nway_part_2();
        break;
    }
}