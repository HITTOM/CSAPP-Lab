#include "cachelab.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void print_help() {
    puts("fuck");
}

struct Op {
    char command;
    unsigned long long addr;
    int size;
};

int main(int argc, char *argv[])
{
    int opt;
    char *trace_file_name;
    int s, E, b, verbose = 0;
    while (-1 != (opt = getopt(argc, argv, "vhs:E:b:t:"))) {
        switch (opt) {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file_name = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            print_help();
            break;
        default:
            print_help();
            exit(-1);
        }
    }
    printf("s = %d, E = %d, b = %d, trace_file_name = %s, verbose = %d\n", s, E, b, trace_file_name, verbose);
    FILE *fp = fopen(trace_file_name, "r");
    char op;
    unsigned long long addr;
    int field_size;
    int hit = 0, miss = 0, evict = 0;
    int S = 1 << s;
    int cache_line_num = S * E;
    int belong[cache_line_num], recent_time[cache_line_num];
    for (int i = 0; i < cache_line_num; ++i) {
        belong[i] = -1;
        recent_time[i] = -1;
    }
    int input_id = 0;
    while (-1 != fscanf(fp, " %c %llx,%d", &op, &addr, &field_size)) {
        ++input_id;
        if ('I' == op) continue;
        int cache_set_id = (addr >> b) % S;
        int addr_line_id = (addr >> b) / S;
        int selected_line_id = cache_set_id * E, is_hit = 0;
        // lru
        for (int i = 0; i < E; ++i) {
            int cache_line_id = cache_set_id * E + i;
            if (belong[cache_line_id] == addr_line_id) {
                is_hit = 1;
                selected_line_id = cache_line_id;
                break;
            }
            if (recent_time[cache_line_id] < recent_time[selected_line_id]) {
                selected_line_id = cache_line_id;
            }
        }
        if (!is_hit) {
            ++miss;
            if (-1 != belong[selected_line_id]) ++evict;
            belong[selected_line_id] = addr_line_id;
        } else ++hit;
        if ('M' == op) ++hit;
        recent_time[selected_line_id] = input_id;
    }
    printSummary(hit, miss, evict);
    return 0;
}
