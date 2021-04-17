#include "lsm_tree.h"
#include "buffer.h"
#include "typedefs.h"

// This is where you can implement your own tests for the hash table
// implementation.
int main()
{

  printf("-------------------------------Start---------------------------------\n");

  lsmtree *lsmt = NULL;
  //char c;
  valType int1, int2, *value;
  int bfr_size = 1024, depth = 2000, fanout = 2, count, found;
  double error = 0.01;
  //char file_bfr[CHAR_SIZE + EXTRA_SIZE];
  //FILE *fptr;

  // statistics defs
  // size_t pagesize = getpagesize();
  // int idx, jdx, kdx, i=0, run_size, help[2];
  // level *ptr_lvl;
  // FILE *fptr;
  // char file_bfr[CHAR_SIZE + EXTRA_SIZE];

  if (allocate(&lsmt, bfr_size, depth, fanout, error) == -1)
  {
    return -1;
  }

  //-------- read Data

  //FILE *dtptr;
  //char fileName[] = "generated-data/100K.data";
  char fileName[] = "generated-data/100K.random.data";
  //char const* const fileName = argv[1]; /* should check that argc > 1 */
  FILE *file = fopen(fileName, "r"); /* should check the result */
  char line[256];
  char *ind;
  char *val;
  int index;
  int valu;

  while (fgets(line, sizeof(line), file))
  {
  
    ind = strtok(line, " ");
    val = strtok(NULL, " ");

    index = atoi(ind);
    valu = atoi(val);
    put(lsmt, index, valu);

  }


  fclose(file);

  //----- my test with inserting  -----

  // count = 0;
  // value = (valType *)malloc(sizeof(valType));
  // int range= 10000;
  // int epocs = 1000;
  // float timeKeep[epocs];
  // float insertingTime[epocs];
  // float totalTime[epocs];

  // data_chunk* arrayRes = (data_chunk *)malloc((range+1) * sizeof(data_chunk));
  // for (int j = 0; j < epocs; j++)
  // {
  //   count = j;
  //   int1 = getRandoms(0, 90000);
  //   int2 = int1 + range;
    
  //   for (int i = 0; i < range; i++)
  //   {
  //     arrayRes[i].key = 0;
  //     arrayRes[i].value = 0;
  //   }

  //   printf("get range \n");
  //   clock_t start = clock();
  //   found = getRange(lsmt, int1, int2, value, arrayRes, range);
  //   clock_t end = clock();
  //   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
  //   timeKeep[j] = seconds;


  //   printf("merge sort \n");
  //   clock_t start1 = clock();
  //   found = merge_sort( arrayRes, 0, range);
  //   printf("put \n");
  //   for (int kk =range-1 ; kk >= 0; kk--)
  //   {
  //     //printf("iteration %d \n", kk);
  //     if (arrayRes[kk].key != 0)
  //     {
  //       put(lsmt, arrayRes[kk].key, arrayRes[kk].value);
  //     }
  //   }
  //   clock_t end1 = clock();
  //   float seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
  //   insertingTime[j] = seconds1;

  //   float total = seconds+ seconds1;
  //   totalTime[j] = total;
    

    

  //   // for (int i = 0; i < range; i++)
  //   // {
  //   //   printf("data before sort  index %d, value %d\n", arrayRes[i].key, arrayRes[i].value);
  //   // }

 
  //   printf("----------------------------------------------------------------\n");
  //   printf(" search from %d, to %d\n", int1, int2);
  //   printf("operation %d took %.04f seconds.\n", count, seconds);
    
  // }

  // FILE *filePtr;
  // filePtr = fopen("result/test-withUpdate-1000epocs-10000range.data","w");
  //  for (int i = 0; i < epocs; i++) {
  //     fprintf(filePtr, "%.04f\t%.04f\t%.04f\n", timeKeep[i], insertingTime[i], totalTime[i]);
  //  };


  //---- end with inserting -----

  //----- my test without inserting  -----

  count = 0;
  value = (valType *)malloc(sizeof(valType));
  int range= 10000;
  int epocs = 1000;
  float timeKeep[epocs];
  //float insertingTime[epocs];
  //float totalTime[epocs];

  data_chunk* arrayRes = (data_chunk *)malloc((range+1) * sizeof(data_chunk));
  for (int j = 0; j < epocs; j++)
  {
    count = j;
    int1 = getRandoms(0, 90000);
    printf("random number %d\n", int1);
    int2 = int1 + range;
    
    for (int i = 0; i < range; i++)
    {
      arrayRes[i].key = 0;
      arrayRes[i].value = 0;
    }

    printf("get range \n");
    clock_t start = clock();
    //found = 0;
    found =getRange(lsmt, int1, int2, value, arrayRes, range);
    printf("done \n");
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    timeKeep[j] = seconds;


    // printf("merge sort \n");
    // clock_t start1 = clock();
    // found = merge_sort( arrayRes, 0, range);
    // printf("put \n");
    // for (int kk =range-1 ; kk >= 0; kk--)
    // {
    //   //printf("iteration %d \n", kk);
    //   if (arrayRes[kk].key != 0)
    //   {
    //     put(lsmt, arrayRes[kk].key, arrayRes[kk].value);
    //   }
    // }
    // clock_t end1 = clock();
    // float seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
    // insertingTime[j] = seconds1;

    // float total = seconds+ seconds1;
    // totalTime[j] = total;
    

    
    // if ( j == epocs -1){
    // for (int i = 0; i < range; i++)
    // {
    //   printf("data before sort  index %d, value %d\n", arrayRes[i].key, arrayRes[i].value);
    // }
    // }

    printf("operation %d took %.04f seconds.\n", count, seconds);
    printf("----------------------------------------------------------------\n");
    //printf(" search from %d, to %d\n", int1, int2);
  
    
  }

  FILE *filePtr;
  filePtr = fopen("result/test-100Kdata-1000epocs-10000range.data","w");
   for (int i = 0; i < epocs; i++) {
      fprintf(filePtr, "%.04f\n", timeKeep[i]);
   };

  // while (scanf("%c", &c) == 1) {
  //     if (c == 'p') {
  //         if (scanf("%d %d\n", &int1, &int2) == 2){
  //             printf("values %d %d .\n", int1, int2);
  //             put(lsmt, int1, int2);
  //         };
  //     } else if (c == 'g') {
  //         if (scanf("%d\n", &int1) == 1) {
  //             found = get(lsmt, int1, value);
  //             if (found == 0) {
  //                 printf("%d\n", *value);
  //             } else {
  //                 printf("\n");
  //             }
  //         };
  //     } else if (c == 'd') {
  //         scan_out = scanf("%d\n", &int1);
  //         if (scan_out == 1)
  //             erase(lsmt, int1);
  //     } else if (c == 's'){
  //         // Read the line change char
  //         scan_out = scanf("%c", &c);
  //         if (scan_out == 1)
  //             print_statistics(lsmt);
  //     } else if (c == 'l'){
  //         // Read the line change char
  //         scan_out = scanf("%s", file_bfr);
  //         if (scan_out == 1) {
  //             fptr = fopen(file_bfr, "rb");
  //             while (fread(help, sizeof(int), 2, fptr) == 2) {
  //                 put(lsmt, help[0], help[1]);
  //             }
  //             fclose(fptr);
  //         }
  //         // Read the line change char
  //         scan_out = scanf("%c", &c);
  //     } else if (c == 'r'){
  //         if (scanf("%d %d\n", &int1, &int2) == 2){
  //             for (idx=int1; idx<int2; idx++) {
  //                 found = get(lsmt, idx, value);
  //                 if (found == 0) {
  //                     printf("%d:%d ", idx, *value);
  //                 }
  //             }
  //             printf("\n");
  //         }
  //     } else {
  //         return -1;
  //     }
  //     count++;
  // }

  // Test to make sure binary files read and write correctly

  // ptr_lvl = (*lsmt).levels[i];
  // while (ptr_lvl != NULL) {
  //     jdx = 0;
  //     data_chunk *data_bfr;
  //     data_bfr = (data_chunk *) malloc(ptr_lvl->curr_size * sizeof(data_chunk));
  //
  //     for (idx=0; idx < ptr_lvl->curr_runs; idx++) {
  //         run_size = pow((*lsmt).fanout, i) * (*lsmt).bfr->max_size;
  //
  //         for (kdx=0; kdx<(int) ((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); kdx++) {
  //             snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[kdx]);
  //             fptr = fopen(file_bfr, "rb");
  //
  //             while (fread(help, sizeof(int), 2, fptr) == 2) {
  //                 data_bfr[jdx].key = help[0];
  //                 data_bfr[jdx].value = help[1];
  //                 jdx++;
  //             }
  //             fclose(fptr);
  //         }
  //     }
  //     free(data_bfr);
  //     printf("Level %d pairs: %d\n", i + 1, ptr_lvl->curr_size);
  //     printf("Actual: %d\n", jdx);
  //     printf("\n");
  //     ptr_lvl = (*lsmt).levels[++i];
  // }
  //
  // // Test for bloom filters and fence pointers
  // printf("Let's print a dump of all the fence pointers:\n");
  // i=0;
  // ptr_lvl = (*lsmt).levels[i];
  // while (ptr_lvl != NULL) {
  //     if (i == 0) {
  //         run_size = (*lsmt).bfr->max_size;
  //     } else {
  //         run_size = pow((*lsmt).fanout, i) * (*lsmt).bfr->max_size;
  //     }
  //     printf("Level %d fp:\n", i + 1);
  //     for (idx=0; idx<ptr_lvl->curr_runs; idx++) {
  //         printf("--R%d: ", idx);
  //         for (jdx=0; jdx<(int) ceil((float) run_size * sizeof(data_chunk) / pagesize); jdx++) {
  //             printf("%d ", ptr_lvl->runs[idx].fence_p[jdx]);
  //         }
  //         printf("\n");
  //     }
  //     printf("\n");
  //     ptr_lvl = (*lsmt).levels[++i];
  // }
  // printf("----------------------------------------------------------------\n");
  // printf("Short hardcoded test for bloom:\n");
  // i = 0;
  // ptr_lvl = (*lsmt).levels[i];
  // while (ptr_lvl != NULL) {
  //     printf("LVL%d:\n", i+1);
  //     for (idx=0; idx<16; idx++) {
  //         printf("(%d:", idx);
  //         if (check_hash(ptr_lvl->blm_f, idx)) {
  //             printf("Y) ");
  //         } else {
  //             printf("N) ");
  //         }
  //     }
  //     printf("\n");
  //     ptr_lvl = (*lsmt).levels[++i];
  // }
  // printf("----------------------------------------------------------------\n");
  // System diagnostics
  // printf("System page size: %zu bytes\n", pagesize);
  // printf("Data chunk size: %zu bytes\n", sizeof(data_chunk));
  // printf("Int size: %zu bytes\n", sizeof(int));
  // printf("Current buffer size: %zu bytes\n", ((*lsmt).bfr->current_index) * sizeof(data_chunk));
  // printf("Max buffer size: %zu bytes\n", ((*lsmt).bfr->max_size) * sizeof(data_chunk));

  //free(value);
  //deallocate(lsmt);

  return 0;
}

/*
CODE FOR MMAP FROM JAMESFISHER.COM

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
  size_t pagesize = getpagesize();

  printf("System page size: %zu bytes\n", pagesize);

  char * region = mmap(
    (void*) (pagesize * (1 << 20)),   // Map from the start of the 2^20th page
    pagesize,                         // for one page length
    PROT_READ|PROT_WRITE|PROT_EXEC,
    MAP_ANON|MAP_PRIVATE,             // to a private block of hardware memory
    0,
    0
  );
  if (region == MAP_FAILED) {
    perror("Could not mmap");
    return 1;
  }

  strcpy(region, "Hello, world!");

  printf("Contents of region: %s\n", region);

  int unmap_result = munmap(region, 1 << 10);
  if (unmap_result != 0) {
    perror("Could not munmap");
    return 1;
  }
  // getpagesize
  return 0;
}
*/


