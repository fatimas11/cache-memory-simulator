/* fatima swelem  */
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uchar;

typedef struct cache_line_s {
    uchar valid;
    uchar frequency;
    long int tag;
    uchar* block;
} cache_line_t;

typedef struct cache_s {
    uchar s;
    uchar t;
    uchar b;
    uchar E;
    cache_line_t** cache;
} cache_t;

cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E) {

    cache_t my_cache;
    my_cache.s = s;  // set bits
    my_cache.t = t;  // tag bits
    my_cache.b = b;  // block bits
    my_cache.E = E;  // lines per set

    int S = 1 << s;  // S = number of sets = 2^s
    int B = 1 << b;  // B = bytes per cache block = 2^b

    // allocate array of pointers for sets (each set holds E lines)
    my_cache.cache = (cache_line_t**)malloc(S * sizeof(cache_line_t*));
    // check if malloc found place
    if (my_cache.cache == NULL) {
        printf("malloc failed for sets");
        exit(1);
    }

    // initialize each set
    for (int i = 0; i < S; i++) {
        // allocate array of E cache lines in each set
        my_cache.cache[i] = (cache_line_t*)malloc(E * sizeof(cache_line_t));
        // check if malloc found place
        if (my_cache.cache[i] == NULL) {
            printf("malloc failed for lines");
            exit(1);
        }

        // initialize each cache line in the set
        for (int j = 0; j < E; j++) {
            my_cache.cache[i][j].valid = 0;
            my_cache.cache[i][j].frequency = 0;
            my_cache.cache[i][j].tag = 0;

            // allocate memory for the block in this line
            my_cache.cache[i][j].block = (uchar*)malloc(B * sizeof(uchar));
            // check if malloc found place
            if (my_cache.cache[i][j].block == NULL) {
                printf("malloc failed for block");
                exit(1);
            }
            
            // initialize each block
            for (int k = 0; k < B; k++) {
                my_cache.cache[i][j].block[k] = 0;
            }
        } 
    }

    return my_cache;
}

void print_cache(cache_t cache) {

    int S = 1 << cache.s;
    int B = 1 << cache.b;

    for (int i = 0; i < S; i++) {
        printf("Set %d\n", i);
        for (int j = 0; j < cache.E; j++) {
            printf("%1d %d 0x%0*lx ", cache.cache[i][j].valid,
            cache.cache[i][j].frequency, cache.t, cache.cache[i][j].tag);
            for (int k = 0; k < B; k++) {
                printf("%02x ", cache.cache[i][j].block[k]);
            }
            puts("");
        }
    }
}

uchar read_byte(cache_t cache, uchar* start, long int off) {
    
    int s = cache.s;
    int t = cache.t;
    int b = cache.b;

    int S = 1 << s;
    int B = 1 << b;

    long int block_offset = off & (B - 1);            // last b bits 
    long int set_index = (off >> b) & (S - 1);        // next s bits
    long int tag = (off >> (b + s));                  // remaining t bits

    // try to find the line in the correct set
    for (int i = 0; i < cache.E; i++) {

        cache_line_t* line = &cache.cache[set_index][i];

        if (line->valid != 0 && line->tag == tag) {
            // Cache hit -> return the byte at the offset
            line->frequency++;
            return line->block[block_offset];
        }

    }// the line not found = cache miss -> so we need to load from memory into cache

    // we look for an invalid line
    for (int i = 0; i < cache.E; i++) {

        cache_line_t* line = &cache.cache[set_index][i];

        if (!line->valid) {

            // fill this line with new block from memory
            line->valid = 1;
            line->tag = tag;
            line->frequency = 1;

            // copy block from memory to cache
            long int block_start = off - block_offset;
            for (int j = 0; j < B; j++) {

                line->block[j] = start[block_start + j];
            }

            return line->block[block_offset];
        }
    }

    // no invalid line found -> use LFU
    int min_frequency = cache.cache[set_index][0].frequency;
    int min_index = 0;
    // find the index to the line with the min frequency
    for (int i = 1; i < cache.E; i++) {

        if (cache.cache[set_index][i].frequency < min_frequency) {

            min_frequency = cache.cache[set_index][i].frequency;
            min_index = i;
        }
    }

    // replace the LFU line
    cache_line_t* line_to_replace = &cache.cache[set_index][min_index];
    line_to_replace->valid = 1;
    line_to_replace->tag = tag;
    line_to_replace->frequency = 1;

    // copy block from memory to cache
    long int block_start = off - block_offset;
    for (int j = 0; j < B; j++) {

        line_to_replace->block[j] = start[block_start + j];
    }

    return line_to_replace->block[block_offset];
}

void write_byte(cache_t cache, uchar* start, long int off, uchar new) {

    int s = cache.s;
    int t = cache.t;
    int b = cache.b;

    int S = 1 << s;
    int B = 1 << b;

    long int block_offset = off & (B - 1);
    long int set_index = (off >> b) & (S - 1);
    long int tag = (off >> (b + s));

    // write-through -> update memory directly
    start[off] = new;

    // find a matching cache line
    for (int i = 0; i < cache.E; i++) {

        cache_line_t* line = &cache.cache[set_index][i];
        if (line->valid != 0 && line->tag == tag) {

            // cache hit -> update cache
            line->block[block_offset] = new;
            return;
        }
    }

    // cache miss -> copy to cache from memory
    for (int i = 0; i < cache.E; i++) {

        cache_line_t* line = &cache.cache[set_index][i];
        if (line->valid != 0) {

            line->valid = 1;
            line->tag = tag;
            line->frequency = 1;

            // copy block from memory
            long int block_start = off -block_offset;
            for (int j = 0; j < B; j++) {

                line->block[j] = start[block_start + j];
            }

            // update the cache
            line->block[block_offset] = new;
            return;
        }
    }

    // no invalid lines -> use LFU
    int min_frequency = cache.cache[set_index][0].frequency;
    int min_index = 0;
    // find the index to the line with the min frequency
    for (int i = 1; i < cache.E; i++) {

        if (cache.cache[set_index][i].frequency < min_frequency) {

            min_frequency = cache.cache[set_index][i].frequency;
            min_index = i;
        }
    }

    // replace the LFU line
    cache_line_t* line_to_replace = &cache.cache[set_index][min_index];
    line_to_replace->valid = 1;
    line_to_replace->tag = tag;
    line_to_replace->frequency = 1;

    long int block_start = off - block_offset;
    for (int j = 0; j < B; j++) {

        line_to_replace->block[j] = start[block_start + j];
    }

    line_to_replace->block[block_offset] = new;
    return;
}

void free_cache(cache_t* cache) {

    int S = 1 << cache->s;
    int E = cache->E;

    for (int i = 0; i < S; i++) {

        for (int j = 0; j < E; j++) {

            free(cache->cache[i][j].block);  // free each block
        }

        free(cache->cache[i]);  // free each set
    }

    free(cache->cache);  // free the array of sets
    return;

}

int main() {
    int n;
    printf("Size of data: ");
    scanf("%d", &n);
    uchar* mem = malloc(n);
    printf("Input data >> ");
    for (int i = 0; i < n; i++)
        scanf("%hhd", mem + i);

    int s, t, b, E;
    printf("s t b E: ");
    scanf("%d %d %d %d", &s, &t, &b, &E);
    cache_t cache = initialize_cache(s, t, b, E);

    while (1) {
        scanf("%d", &n);
        if (n < 0) break;
        read_byte(cache, mem, n);
    }

    puts("");
    print_cache(cache);

    free(mem);
    free_cache(&cache);

}

