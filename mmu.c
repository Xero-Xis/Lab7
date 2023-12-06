#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

void TOUPPER(char * arr){
  
    for(int i=0;i<strlen(arr);i++){
        arr[i] = toupper(arr[i]);
    }
}

void get_input(char *args[], int input[][2], int *n, int *size, int *policy) 
{
    FILE *input_file = fopen(args[1], "r");
      if (!input_file) {
            fprintf(stderr, "Error: Invalid filepath\n");
            fflush(stdout);
            exit(0);
      }

    parse_file(input_file, input, n, size);
  
    fclose(input_file);
  
    TOUPPER(args[2]);
  
    if((strcmp(args[2],"-F") == 0) || (strcmp(args[2],"-FIFO") == 0))
        *policy = 1;
    else if((strcmp(args[2],"-B") == 0) || (strcmp(args[2],"-BESTFIT") == 0))
        *policy = 2;
    else if((strcmp(args[2],"-W") == 0) || (strcmp(args[2],"-WORSTFIT") == 0))
        *policy = 3;
    else {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
       exit(1);
    }
        
}

void allocate_memory(list_t * freelist, list_t * alloclist, int pid, int blocksize, int policy) {
  
    // If no block is found, print error message and return
    node_t *temp = NULL;
    block_t *blk;
    
    switch (policy) {
        case 1: // First Fit
            temp = freelist->head;
            while (temp) {
                blk = temp->blk;
                if (blk->end - blk->start >= blocksize) {
                    break;
                }
                temp = temp->next;
            }
            break;
        case 2: // Best Fit
            // Create temporary list sorted by block size
            list_t *temp_list = list_alloc();
            while ((blk = list_remove_from_front(freelist)) != NULL) {
                list_add_ascending_by_blocksize(temp_list, blk);
            }
            temp = temp_list->head;
            while (temp) {
                blk = temp->blk;
                if (blk->end - blk->start >= blocksize) {
                    break;
                }
                temp = temp->next;
            }
            free(temp_list);
            break;
        case 3: // Worst Fit
            // Create temporary list sorted by block size in descending order
            list_t *temp_list = list_alloc();
            while ((blk = list_remove_from_front(freelist)) != NULL) {
                list_add_descending_by_blocksize(temp_list, blk);
            }
            temp = temp_list->head;
            while (temp) {
                blk = temp->blk;
                if (blk->end - blk->start >= blocksize) {
                    break;
                }
                temp = temp->next;
            }
            free(temp_list);
            break;
        default:
            break;
    }

    if (!temp) {
        printf("Error: Not Enough Memory\n");
        return;
    }

    // Remove block from Free Block list
    blk = temp->blk;
    list_remove(freelist, temp);

    // Update block information and add to Allocated Block list
    blk->pid = pid;
    blk->end = blk->start + blocksize - 1;
    list_add_ascending_by_address(alloclist, blk);

    // Handle remaining fragment
    if (blk->end - blk->start > blocksize) {
        block_t *fragment = malloc(sizeof(block_t
