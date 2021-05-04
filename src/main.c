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
  

  if (allocate(&lsmt, bfr_size, depth, fanout, error) == -1)
  {
    return -1;
  }

  //-------- INserting data to LSM TREE

  //FILE *dtptr;
  //char fileName[] = "generated-data/100K.data";
  char fileName[] = "generated-data/1M.random.data";  // input data
  //char const* const fileName = argv[1]; /* should check that argc > 1 */
  FILE *file = fopen(fileName, "r"); /* should check the result */
  char line[256];
  char *ind;
  char *val;
  int index;
  int valu;


  // Reading input data to be inserted into LSM Tree
  while (fgets(line, sizeof(line), file))
  {

    ind = strtok(line, " ");
    val = strtok(NULL, " ");

    index = atoi(ind);
    valu = atoi(val);
    // Inserting data into LSM tree
    put(lsmt, index, valu);
  }

  fclose(file);

  // //----- my test with compaction  -----

  // -------- only for normal ditributed input search
  //FILE *dtptr;
  char fileName1[] = "generated-data/new100K.random.data";
  //char const* const fileName = argv[1]; /* should check that argc > 1 */
  FILE *file1 = fopen(fileName1, "r"); /* should check the result */
  char line1[256];
  //char *ind1;
  //int index1;
  int listData[100000];
  int listDataVal[100000];

  int ff = 0; 


  //reading input for query
  while (fgets(line1, sizeof(line1), file1))
  {
    ind = strtok(line1, " ");
    val = strtok(NULL, " ");

    index = atoi(ind);
    valu = atoi(val);

    listData[ff] = index;
    listDataVal[ff] = valu;
    ff = ff+1;
  }

  fclose(file);

  // -------- end only for normal ditributed input search

  // count = 0;
  // value = (valType *)malloc(sizeof(valType));
  // int range = 50000;
  // int epocs = 200;
  // float timeKeep[2];
  // float searchTimeArray[epocs];
  // float compactionTimeArray[epocs];
  // float totalTime[epocs];
  // int epocs1 = 1;
  

  // data_chunk *arrayRes = (data_chunk *)malloc((range + 1) * sizeof(data_chunk));
  // for (int j = 0; j < epocs; j++)
  // {
  //   count = j;
  //   //int1 = listData[j]-(range/2); // for normally ditrubuted input serach
  //   int1 = getRandoms(0, 90000);  // for ramdonly distributed inout search
  //   int2 = int1 + range;

  //   for (int i = 0; i < range; i++)
  //   {
  //     arrayRes[i].key = 0;
  //     arrayRes[i].value = 0;
  //   }

  //   printf("get range \n");

  //   //searchTime = 0;
  //   //compactionTime = 0;

  //   timeKeep[0] = 0.0;
  //   timeKeep[1] = 0.0;


  //   // Get range query
  //   found = getRangeWithCompaction(lsmt, int1, int2, value, arrayRes, range, timeKeep);
  //   if (j%50 == 0 )
  //   {
  //     printf("reinserting data \n");
  //     for (int kkl = 0; kkl < 10000; kkl++)
  //     {
  //       //printf("reinserting data %d \n", listData[kkl+epocs1]);
  //       put(lsmt, listData[kkl+epocs1], listDataVal[kkl+epocs1]);
  //     }
  //     epocs1 = 10000 + epocs1;
  //   }

  //   searchTimeArray[j] = timeKeep[0];
  //   compactionTimeArray[j] = timeKeep[1];
  //   totalTime[j] = timeKeep[0]+timeKeep[1];

    
  //   //found = merge_sort(arrayRes, 0, range);
  //   // for (int i = 0; i < range; i++)
  //   // {
  //   //   printf("data before sort  index %d, value %d\n", arrayRes[i].key, arrayRes[i].value);
  //   // }
    

    
  //   printf(" search from %d, to %d\n", int1, int2);
  //   printf("operation %d took search  %.08f -- compaction %.08f seconds.\n", count, timeKeep[0], timeKeep[1]);
  //   printf("----------------------------------------------------------------\n");
  // }

  // //Saving the time into file
  // FILE *filePtr;
  // filePtr = fopen("result/test-withCompaction-Uniform-2FO-100epocs-withInsert-50000range-1M.random.data","w");
  //  for (int i = 0; i < epocs; i++) {
  //     fprintf(filePtr, "%.04f\t%.04f\t%.04f\n", searchTimeArray[i], compactionTimeArray[i], totalTime[i]);
  //  };

  //---- end with compaction -----


  //----- my test with inserting  -----

  // count = 0;
  // value = (valType *)malloc(sizeof(valType));
  // int range = 5000;
  // int epocs = 100;

  // float timeKeep[epocs];
  // float insertingTime[epocs];
  // float totalTime[epocs];

  // //searchTime = 0;

  // data_chunk *arrayRes = (data_chunk *)malloc((range + 1) * sizeof(data_chunk));
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
  //   found = merge_sort(arrayRes, 0, range);
  //   printf("put \n");
  //   for (int kk = range - 1; kk >= 0; kk--)
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

  //   float total = seconds + seconds1;
  //   totalTime[j] = total;

  //   for (int i = 0; i < range; i++)
  //   {
  //     printf("data before sort  index %d, value %d\n", arrayRes[i].key, arrayRes[i].value);
  //   }

  //   printf("----------------------------------------------------------------\n");
  //   printf(" search from %d, to %d\n", int1, int2);
  //   printf("operation %d took %.04f seconds.\n", count, seconds);
  // }

  // FILE *filePtr;
  // filePtr = fopen("result/test-withUpdate-100epocs-5000range-1M.random.data","w");
  //  for (int i = 0; i < epocs; i++) {
  //     fprintf(filePtr, "%.04f\t%.04f\t%.04f\n", timeKeep[i], insertingTime[i], totalTime[i]);
  //  };

  //---- end with inserting -----

  //----- my test without inserting  -----


  // char fileName1[] = "generated-data/1000.normal.data";
  // //char const* const fileName = argv[1]; /* should check that argc > 1 */
  // FILE *file1 = fopen(fileName1, "r"); /* should check the result */
  // char line1[256];
  // char *ind1;
  // int index1;
  // int listData[1000];

  // int ff = 0; 
  // while (fgets(line1, sizeof(line1), file1))
  // {
  //   ind1 = strtok(line1, " ");
  //   index1 = atoi(ind1);
  //   listData[ff] = index1;
  //   ff = ff+1;
  // }

  // fclose(file1);

  count = 0;
  value = (valType *)malloc(sizeof(valType));

  int range= 50000;
  int epocs = 200;
  float timeKeep[epocs];
  int epocs1 = 1;

  data_chunk* arrayRes = (data_chunk *)malloc((range+1) * sizeof(data_chunk));
  for (int j = 0; j < epocs; j++)
  {
    count = j;
    int1 = getRandoms(0, 90000);
    //printf("random number %d\n", int1);
    //int1 =listData[j]- (range/2);
    int2 = int1 + range;

    for (int i = 0; i < range; i++)
    {
      arrayRes[i].key = 0;
      arrayRes[i].value = 0;
    }

    printf("get range \n");
    clock_t start = clock();
    found = 0;
    found =getRange(lsmt, int1, int2, value, arrayRes, range);
    printf("done \n");
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    timeKeep[j] = seconds;

    if (j%50 == 0 )
    {
      printf("reinserting data \n");
      for (int kkl = 0; kkl < 10000; kkl++)
      {
        //printf("reinserting data %d \n", listData[kkl+epocs1]);
        put(lsmt, listData[kkl+epocs1], listDataVal[kkl+epocs1]);
      }
      epocs1 = 10000 + epocs1;
    }


    // if ( j == epocs -1){
    // for (int i = 0; i < range; i++)
    // {
    //   printf("data before sort  index %d, value %d\n", arrayRes[i].key, arrayRes[i].value);
    // }

    printf("operation %d took %.04f seconds.\n", count, seconds);
    printf("----------------------------------------------------------------\n");
    //printf(" search from %d, to %d\n", int1, int2);
  }

  FILE *filePtr;
  filePtr = fopen("result/testRegularSearch-Uniform-2FO-1Mdata-100epocs-withInsert-50000range.data","w");
   for (int i = 0; i < epocs; i++) {
      fprintf(filePtr, "%.04f\n", timeKeep[i]);
   };
 
//----------------- search range without reinserting


  

  return 0;
}


