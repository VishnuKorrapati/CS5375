#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

char *trace_file_name;

struct L1Cache
{
    unsigned valid_field[1024];
    unsigned dirty_field[1024];
    uint64_t tag_field[1024];
    char data_field[1024][64];
    int hits;
    int misses;
};

struct L2Cache
{
    unsigned valid_field[16384];
    unsigned dirty_field[16384];
    uint64_t tag_field[16384];
    char data_field[16384][64];
    int hits;
    int misses;
};

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

int is_data_in_l1(uint64_t address, int nway, struct L1Cache *l1)
{
    uint64_t block_add = address >> (unsigned)log2(64);
    int set_Number = block_add % 512;
    uint64_t tag = block_add >> (unsigned)log2(512);
    int index_s= ((int)set_Number) * nway;
    int nwayTemp = nway;
    int index_l = index_s;

    while (nwayTemp > 0)
    {
        if (l1->valid_field[index_l] && l1->tag_field[index_l] == tag)
        {
            return 1;
        }
        index_l += 1;
        nwayTemp--;
    }
    return 0;
}

int is_data_in_l2(uint64_t address, int nway, struct L2Cache *l2)
{
    uint64_t block_add = address >> (unsigned)log2(64);
    int set_Number = block_add % 2048;
    uint64_t tag = block_add >> (unsigned)log2(2048);
    int index_s = ((int)set_Number) * nway;
    int nwayTemp = nway;
    int index_l = index_s;

    while (nwayTemp > 0)
    {
        if (l2->valid_field[index_l] && l2->tag_field[index_l] == tag)
        {
            return 1;
        }
        index_l += 1;
        nwayTemp--;
    }
    return 0;
}

void data_to_l1(uint64_t address, int nway, struct L1Cache *l1)
{
    uint64_t block_add = address >> (unsigned)log2(64);
    int set_Number = block_add % 512;
    uint64_t tag = block_add >> (unsigned)log2(512);
    int index_s = ((int)set_Number) * nway;
    int nwayTemp = nway;
    int loop_index = index_s;
    int any_space = 0;
    int index_end = index_s + nway - 1;
 
    while (nwayTemp > 0)
    {
        if (l1->valid_field[loop_index] == 0)
        {
            any_space = 1;
        }
        loop_index++;
        nwayTemp--;
    }
    if (any_space > 0)
    {
        nwayTemp = nway;
        loop_index = index_s;
        while (nwayTemp > 0)
        {
            if (l1->valid_field[loop_index] == 0)
            {
                l1->valid_field[loop_index] = 1;
                l1->tag_field[loop_index] = tag;
                break;
            }

            loop_index += 1;
            nwayTemp--;
        }
    }
    else
    {
        
        int index_rand = (rand() % (index_end - index_s + 1)) + index_s;
        
        l1->valid_field[index_rand] = 1;
        l1->tag_field[index_rand] = tag;
    }
}

void data_to_l2(uint64_t address, int nway, struct L2Cache *l2)
{

    uint64_t block_add = address >> (unsigned)log2(64);
    int set_Number = block_add % 2048;
    uint64_t tag = block_add >> (unsigned)log2(2048);
    int index_s = ((int)set_Number) * nway;
    int nwayTemp = nway;
    int loop_index = index_s;
    int any_space = 0;
    int index_end = index_s + nway - 1;
    
    while (nwayTemp > 0)
    {
        if (l2->valid_field[loop_index] == 0)
        {
            any_space = 1;
        }
        loop_index++;
        nwayTemp--;
    }
    if (any_space > 0)
    {
        nwayTemp = nway;
        loop_index = index_s;
        while (nwayTemp > 0)
        {
            if (l2->valid_field[loop_index] == 0)
            {
                l2->valid_field[loop_index] = 1;
                l2->tag_field[loop_index] = tag;
                break;
            }

            loop_index += 1;
            nwayTemp--;
        }
    }
    else
    {
        //pick a rand value and replace
        int index_rand = (rand() % (index_end - index_s + 1)) + index_s;
        //   printf("Picking a rand variable %d",randomIndex);
        l2->valid_field[index_rand] = 1;
        l2->tag_field[index_rand] = tag;
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    trace_file_name = argv[2];
    char mem_request[20];
    uint64_t address;
    struct L1Cache l1;
    struct L2Cache l2;
    int numberOfBlocksinl1 = 1024;
    int numberOfBocksinl2 = 16384;
    int l1nway = 2;
    int l2nway = 8;
    int numberOfSetsl1 = 512;
    int numberOfSetsl2 = 2048;
    for (int i = 0; i < numberOfBlocksinl1; i++)
    {
        l1.valid_field[i] = 0;
        l1.dirty_field[i] = 0;
        l1.tag_field[i] = 0;
    }
    for (int i = 0; i < numberOfBocksinl2; i++)
    {
        l2.valid_field[i] = 0;
        l2.dirty_field[i] = 0;
        l2.tag_field[i] = 0;
    }

    l1.hits = 0;
    l1.misses = 0;
    l2.hits = 0;
    l2.misses = 0;

    fp = fopen(trace_file_name, "r");

    if (strncmp(argv[1], "direct", 6) == 0)
    {
        while (fgets(mem_request, 20, fp) != NULL)
        {
            address = convert_address(mem_request);
            int dataInL1 = is_data_in_l1(address, l1nway, &l1);
            if (dataInL1 == 1)
            {
                l1.hits++;
                l2.hits++;
            }
            else
            {
                l1.misses++;
                int dataInL2 = is_data_in_l2(address, l2nway, &l2);
                if (dataInL2)
                {
                    l2.hits += 1;
                }
                else
                {
                    l2.misses++;
                    data_to_l2(address, l2nway, &l2);
                }
                data_to_l1(address, l1nway, &l1);
            }
        }
        printf("\n==================================\n");
        printf("Cache type:     l1\n");
        printf("==================================\n");
        printf("Cache Hits:    %d\n", l1.hits);
        printf("Cache Misses:  %d\n", l1.misses);
        printf("Cache Hit Rate : %0.9f%%\n", ((float)l1.hits / (float)(l1.hits + l1.misses)) * 100);
        printf("Cache Miss Rate : %0.9f%%\n", ((float)l1.misses / (float)(l1.hits + l1.misses)) * 100);
        printf("\n");
        printf("\n==================================\n");
        printf("Cache type:     l2\n");
        printf("==================================\n");
        printf("Cache Hits:    %d\n", l2.hits);
        printf("Cache Misses:  %d\n", l2.misses);
        printf("Cache Hit Rate : %0.9f%%\n", ((float)l2.hits / (float)(l2.hits + l2.misses)) * 100);
        printf("Cache Miss Rate : %0.9f%%\n", ((float)l2.misses / (float)(l2.hits + l2.misses)) * 100);
        printf("\n");
    }

    fclose(fp);

    return 0;
}