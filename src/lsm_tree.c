#include "lsm_tree.h"
#include "functions.h"

int allocate(lsmtree **lsmt, int bfr_size, int depth, int fanout, double error)
{
    buffer *bfr = NULL;

    // Check if the pointer is NULL
    if ((*lsmt) != NULL)
    {
        return -1;
    }

    // Allocate the memory for the hashtable
    (*lsmt) = malloc(sizeof(lsmtree));
    // Check if malloc failed
    if ((*lsmt) == NULL)
    {
        return -1;
    }

    (**lsmt).max_depth = depth;
    (**lsmt).fanout = fanout;
    (**lsmt).error = error;

    if (allocate_buffer(&bfr, bfr_size) != 0)
    {
        return -1;
    }
    (**lsmt).bfr = bfr;

    (**lsmt).levels = malloc((**lsmt).max_depth * sizeof(level));
    // Check if malloc failed
    if ((**lsmt).levels == NULL)
    {
        return -1;
    }
    return 0;
}

int put(lsmtree *lsmt, keyType key, valType value)
{
    // Check if the LSM tree is allocated
    if (lsmt == NULL)
    {
        return -1;
    }

    //printf(" inserting  %d, value %d to buffer \n", key,  value);

    // Try to fit it in the buffer
    if (put_buffer((*lsmt).bfr, key, value) == 0)
    {
        return 0;
    }
    else if (put_buffer((*lsmt).bfr, key, value) == -2)
    {
        // The buffer is full; put everything in the next level
        int out, res;

        out = merge_sort((*lsmt).bfr->bfr_array, 0, (*lsmt).bfr->current_index - 1);
        if (out)
            (*lsmt).bfr->current_index -= out;

        //printf("flush to log after inserting  %d, value %d to buffer \n", key, value);
        res = flush(lsmt, 0);
        if (res)
            return -1;

        // Put the new value
        if (put_buffer((*lsmt).bfr, key, value) == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

int flush(lsmtree *lsmt, int index)
{
    FILE *fptr;
    char file_bfr[CHAR_SIZE + EXTRA_SIZE];
    int idx, jdx, kdx, level_size, run_size;
    size_t pagesize = getpagesize();

    // create the level and run sizes, we'll use them frequently
    level_size = pow((*lsmt).fanout, index + 1) * (*lsmt).bfr->max_size; // power of

    // printf("level  %d\n", index);
    // printf("level size %d\n", level_size);

    // if the level doesn't exist, create it
    if ((*lsmt).levels[index] == NULL)
    {
        bloom *blm_f = NULL;

        if (allocate_filter(&blm_f, level_size, (*lsmt).error) != 0)
        {
            return -1;
        }
        (*lsmt).levels[index] = malloc(sizeof(level));
        if ((*lsmt).levels[index] == NULL)
        {
            return -1;
        }
        (*lsmt).levels[index]->curr_runs = 0;
        (*lsmt).levels[index]->curr_size = 0;
        (*lsmt).levels[index]->blm_f = blm_f;
        (*lsmt).levels[index]->runs = malloc(sizeof(run) * (*lsmt).fanout);
        if ((*lsmt).levels[index]->runs == NULL)
        {
            return -1;
        }
    }

    if (index == 0)
    {
        if (((*lsmt).levels[index]->curr_size + (*lsmt).bfr->current_index) > level_size)
        {
            // clean up the level by pushing current contents down
            // and then put stuff in
            flush(lsmt, index + 1);
        }

        // the same check should not fail again
        idx = 0;
        if (((*lsmt).levels[index]->curr_size + (*lsmt).bfr->current_index) <= level_size)
        {
            // generate our runs stuff
            run_size = (*lsmt).bfr->current_index;
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_start = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            //(*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_new = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) * 2/ pagesize);
            //printf("fence point size %d\n", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[0]);
            //printf("fence pointer %d\n", ptr_lvl->runs[idx].fence_p);

            //int size = sizeof((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p);
            // printf("fence size %d\n", size);
            // printf("page size %d\n", (int)pagesize);
            // printf("chunck size %d\n", (int)(run_size * sizeof(data_chunk)));
            // for (int j = 0; j < size; j++)
            // {
            //      printf("fence pointer value %d\n", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[j]);
            // }

            if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p == NULL)
            {
                return -1;
            }
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files = (char **)malloc(sizeof(char *) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files == NULL)
            {
                return -1;
            }
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].run_size = 0;
            // for each file create the file name array
            for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
            {
                (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx] = (char *)malloc(sizeof(char) * CHAR_SIZE);
                if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx] == NULL)
                {
                    return -1;
                }
            }

            // divide the files to pages
            for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
            {
                snprintf((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx], CHAR_SIZE, "l%dr%df%d", index, (*lsmt).levels[index]->curr_runs, jdx);

                // find max for fence pointer
                // it makes a difference if the buffer has more elements than
                // can fit in a page size or not

                (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_start[jdx] = (*lsmt).bfr->bfr_array[jdx * pagesize / sizeof(data_chunk)].key;

                if ((*lsmt).bfr->current_index - jdx * pagesize / sizeof(data_chunk) >= pagesize / sizeof(data_chunk))
                {
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[jdx] = (*lsmt).bfr->bfr_array[(jdx + 1) * pagesize / sizeof(data_chunk) - 1].key;
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end[jdx] = (*lsmt).bfr->bfr_array[(jdx + 1) * pagesize / sizeof(data_chunk) - 1].key;
                }
                else
                {
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[jdx] = (*lsmt).bfr->bfr_array[(*lsmt).bfr->current_index - jdx * pagesize / sizeof(data_chunk) - 1].key;
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end[jdx] = (*lsmt).bfr->bfr_array[(*lsmt).bfr->current_index - jdx * pagesize / sizeof(data_chunk) - 1].key;
                }

                //printf("level %d\n", index);
                //printf("fence pointer value %d\n", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[jdx]) ;
                // write data to files
                snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx]);

                fptr = fopen(file_bfr, "wb");
                if ((*lsmt).bfr->current_index - jdx * pagesize / sizeof(data_chunk) >= pagesize / sizeof(data_chunk))
                {
                    for (; idx < (int)((jdx + 1) * pagesize / sizeof(data_chunk)); idx++)
                    {
                        fwrite(&(*lsmt).bfr->bfr_array[idx].key, sizeof(int), 1, fptr);
                        fwrite(&(*lsmt).bfr->bfr_array[idx].value, sizeof(int), 1, fptr);
                        hash((*lsmt).levels[index]->blm_f, (*lsmt).bfr->bfr_array[idx].key);
                    }
                }
                else
                {
                    for (; idx < (*lsmt).bfr->current_index - (int)(jdx * pagesize / sizeof(data_chunk)); idx++)
                    {
                        fwrite(&(*lsmt).bfr->bfr_array[idx].key, sizeof(int), 1, fptr);
                        fwrite(&(*lsmt).bfr->bfr_array[idx].value, sizeof(int), 1, fptr);
                        hash((*lsmt).levels[index]->blm_f, (*lsmt).bfr->bfr_array[idx].key);
                    }
                    //printf("\n");
                }
                fclose(fptr);
            }

            //int size = sizeof((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p);
            // printf("fence size %d\n", size);
            // for (int j = 0; j < size; j++)
            // {
            //      printf("fence pointer value %d\n", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[j]);
            // }

            // update the current size and runs
            (*lsmt).levels[index]->curr_size += (*lsmt).bfr->current_index;
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].run_size = (*lsmt).bfr->current_index;
            (*lsmt).levels[index]->curr_runs++;
            (*lsmt).levels[index]->sorted = 0;

            // soft reset the buffer
            (*lsmt).bfr->current_index = 0;
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (((*lsmt).levels[index]->curr_size + (*lsmt).levels[index - 1]->curr_size) > level_size)
        {
            flush(lsmt, index + 1);
        }
        if (((*lsmt).levels[index]->curr_size + (*lsmt).levels[index - 1]->curr_size) <= level_size)
        {
            // need to load all the data of the previous level into memory,
            // sort it, and store it into this level
            int help[2];
            data_chunk *data_bfr;
            data_bfr = (data_chunk *)malloc((*lsmt).levels[index - 1]->curr_size * sizeof(data_chunk));

            jdx = 0;
            for (idx = 0; idx < (*lsmt).levels[index - 1]->curr_runs; idx++)
            {
                // we have multiple files, we need to open all of them
                // we need the number of files of the previous level
                run_size = (*lsmt).levels[index - 1]->runs[idx].run_size;
                for (kdx = 0; kdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); kdx++)
                {
                    snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", (*lsmt).levels[index - 1]->runs[idx].files[kdx]);
                    fptr = fopen(file_bfr, "rb");

                    while (fread(help, sizeof(int), 2, fptr) == 2)
                    {
                        data_bfr[jdx].key = help[0];
                        data_bfr[jdx].value = help[1];
                        jdx++;
                    }
                    fclose(fptr);

                    // read all of the file, delete it
                    remove(file_bfr);
                }
            }
            int out;
            // we now need to sort the data_bfr and proceed with everything else
            out = merge_sort(data_bfr, 0, (*lsmt).levels[index - 1]->curr_size - 1);

            if (out)
                (*lsmt).levels[index - 1]->curr_size -= out;

            // create the run stuff
            run_size = (*lsmt).levels[index - 1]->curr_size;
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_start = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p == NULL)
            {
                return -1;
            }
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files = (char **)malloc(sizeof(char *) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
            if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files == NULL)
            {
                return -1;
            }
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].run_size = 0;

            // for each file create the file name array
            for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
            {
                (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx] = (char *)malloc(sizeof(char) * CHAR_SIZE);
                if ((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx] == NULL)
                {
                    return -1;
                }
            }

            // divide the files to pages
            idx = 0;
            for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
            {
                snprintf((*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx], CHAR_SIZE, "l%dr%df%d", index, (*lsmt).levels[index]->curr_runs, jdx);

                (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_start[jdx] = data_bfr[jdx * pagesize / sizeof(data_chunk)].key;
                // find max for fence pointer
                if ((*lsmt).levels[index - 1]->curr_size - jdx * pagesize / sizeof(data_chunk) >= pagesize / sizeof(data_chunk))
                {
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[jdx] = data_bfr[(jdx + 1) * pagesize / sizeof(data_chunk) - 1].key;
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end[jdx] = data_bfr[(jdx + 1) * pagesize / sizeof(data_chunk) - 1].key;
                }
                else
                {
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_p[jdx] = data_bfr[(*lsmt).levels[index - 1]->curr_size - jdx * pagesize / sizeof(data_chunk) - 1].key;
                    (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].fence_end[jdx] = data_bfr[(*lsmt).levels[index - 1]->curr_size - jdx * pagesize / sizeof(data_chunk) - 1].key;
                    ;
                }

                // write data to files
                snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].files[jdx]);

                fptr = fopen(file_bfr, "wb");
                if ((*lsmt).levels[index - 1]->curr_size - jdx * pagesize / sizeof(data_chunk) >= pagesize / sizeof(data_chunk))
                {
                    for (; idx < (int)((jdx + 1) * pagesize / sizeof(data_chunk)); idx++)
                    {
                        fwrite(&data_bfr[idx].key, sizeof(int), 1, fptr);
                        fwrite(&data_bfr[idx].value, sizeof(int), 1, fptr);
                        hash((*lsmt).levels[index]->blm_f, data_bfr[idx].key);
                    }
                }
                else
                {
                    for (; idx < (*lsmt).levels[index - 1]->curr_size - (int)(jdx * pagesize / sizeof(data_chunk)); idx++)
                    {
                        fwrite(&data_bfr[idx].key, sizeof(int), 1, fptr);
                        fwrite(&data_bfr[idx].value, sizeof(int), 1, fptr);
                        hash((*lsmt).levels[index]->blm_f, data_bfr[idx].key);
                    }
                }
                fclose(fptr);
            }
            free(data_bfr);

            // update the current size and runs
            (*lsmt).levels[index]->curr_size += (*lsmt).levels[index - 1]->curr_size;
            (*lsmt).levels[index]->runs[(*lsmt).levels[index]->curr_runs].run_size = (*lsmt).levels[index - 1]->curr_size;
            (*lsmt).levels[index]->curr_runs++;
            (*lsmt).levels[index]->sorted = 0;

            // soft reset the level
            for (idx = 0; idx < (*lsmt).levels[index - 1]->curr_runs; idx++)
            {
                run_size = (*lsmt).levels[index - 1]->runs[idx].run_size;
                for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
                {
                    free((*lsmt).levels[index - 1]->runs[idx].files[jdx]);
                }
                free((*lsmt).levels[index - 1]->runs[idx].fence_p);
                free((*lsmt).levels[index - 1]->runs[idx].fence_start);
                free((*lsmt).levels[index - 1]->runs[idx].fence_end);
                free((*lsmt).levels[index - 1]->runs[idx].files);
                (*lsmt).levels[index - 1]->runs[idx].run_size = 0;
            }
            (*lsmt).levels[index - 1]->curr_runs = 0;
            (*lsmt).levels[index - 1]->curr_size = 0;
            memset((*lsmt).levels[index - 1]->blm_f->blm_array, 0, (*lsmt).levels[index - 1]->blm_f->m_bits);

            return 0;
        }
        else
        {
            return -1;
        }
    }
}

int getRange(lsmtree *lsmt, keyType startKey, keyType endKey, valType *value, data_chunk rangeRes[], int range)
{
    int found, i = 0, idx, jdx, current_size, bin_index, run_size, num_files, help[2];
    char file_bfr[CHAR_SIZE + EXTRA_SIZE];
    FILE *fptr;
    level *ptr_lvl;
    size_t pagesize = getpagesize();

    if (lsmt == NULL)
    {
        return -1;
    }

    // check the buffer first
    found = get_bufferRange((*lsmt).bfr, startKey, endKey, value, rangeRes, range);
    int curInd = found;

    if (found == -1)
    {
        // we get all data from buffer
        return 0;
    }
    else
    {
        // start checking the levels
        ptr_lvl = (*lsmt).levels[i];

        while (ptr_lvl != NULL &&  curInd <range)
        {
            current_size = pagesize / sizeof(data_chunk);
            for (idx = ptr_lvl->curr_runs - 1; idx >= 0; idx--)
            {
                run_size = ptr_lvl->runs[idx].run_size;
                num_files = (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize;
                bin_index = 0;

                for (int k = 0; k < num_files; k++)
                {
                    if (endKey < ptr_lvl->runs[idx].fence_start[k] || startKey > ptr_lvl->runs[idx].fence_end[k])
                    {
                        continue;
                    }
                    
                    snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[k]);
                    fptr = fopen(file_bfr, "rb");
                    
                    int dataKey;
                    int dataValue;

                    jdx = 0;
                    while (fread(help, sizeof(int), 2, fptr) == 2)
                    {                        
                        dataKey = help[0];
                        dataValue = help[1];
                        if (dataKey >= startKey && dataKey <= endKey && checkIfExist(rangeRes, dataKey, range) == 0)
                        {
                            if (dataValue != GRAVEYARD)
                            {
                                (*value) = dataValue;
                                rangeRes[curInd].key = dataKey;
                                rangeRes[curInd].value = dataValue;
                                curInd = curInd + 1;
                            }
                            if (curInd == range)
                            {
                                fclose(fptr);
                                return 0;
                            }
                        }
                        //jdx++;
                    }
                    fclose(fptr);
                    
                }
            }
            ptr_lvl = (*lsmt).levels[++i];
        }
    }

    // we didn't find it
    return -1;
}

int getRangeWithCompaction(lsmtree *lsmt, keyType startKey, keyType endKey, valType *value, data_chunk rangeRes[], int range, float time[])
{
    int found, i = 0, idx, jdx, current_size, bin_index, run_size, num_files, help[2];
    char file_bfr[CHAR_SIZE + EXTRA_SIZE];
    FILE *fptr;
    level *ptr_lvl;
    size_t pagesize = getpagesize();
    clock_t start1;
    clock_t end1;
    float seconds1;

    if (lsmt == NULL)
    {
        return -1;
    }

    //searchTime = 0;
    //compactionTime = 0;


    // check the buffer first
    start1 = clock();
    found = get_bufferRange((*lsmt).bfr, startKey, endKey, value, rangeRes, range);
    end1 = clock();
    seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
    time[0] = time[0] + seconds1;
    
    
    int curInd = found;

    //printf("search from %d, to %d\n", startKey, endKey);

    if (found == -1)
    {
        return 0; // get all from buffer
    }
    else
    { 
        // start checking the levels
        ptr_lvl = (*lsmt).levels[i];
        
        while (ptr_lvl != NULL && curInd < range)
        {
            // printf("search level %d, level size %d sorted %d\n", i, ptr_lvl->curr_size, ptr_lvl->sorted);
            int numofRuns = 0;
            numofRuns = ptr_lvl->curr_runs;

            current_size = pagesize / sizeof(data_chunk);
            int tobeMerged = 0;
            
            int run_size0 = 0;
            run_size0 = ptr_lvl->runs[0].run_size;

            start1 = clock();

             // check based on the smallest and the highest point ponter on that level
            run_size = ptr_lvl->runs[ptr_lvl->curr_runs - 1].run_size;
            num_files = (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize;
            if (ptr_lvl->sorted == 1 && (startKey > ptr_lvl->runs[ptr_lvl->curr_runs - 1].fence_end[num_files -1] || endKey < ptr_lvl->runs[0].fence_start[0]))
            {
                end1 = clock();
                seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
                time[1] = time[1] + seconds1;

                printf("do not check the level\n");
                ptr_lvl = (*lsmt).levels[++i];

                continue;
            }

            // Get the data 
            for (idx = ptr_lvl->curr_runs - 1; idx >= 0; idx--)
            {
                run_size = ptr_lvl->runs[idx].run_size;
                num_files = (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize;
                

                // read each file in the run
                for (int k = 0; k < num_files; k++)
                {
                    // check  based on pointer
                    if (endKey < ptr_lvl->runs[idx].fence_start[k] || startKey > ptr_lvl->runs[idx].fence_end[k])
                    {
                        continue;
                    }
                    tobeMerged = tobeMerged+1;
                    snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[k]);
                    fptr = fopen(file_bfr, "rb");
                    
                    int dataKey;
                    int dataValue;

                    
                    while (fread(help, sizeof(int), 2, fptr) == 2)
                    {                        
                        dataKey = help[0];
                        dataValue = help[1];

                       
                        if (dataKey >= startKey && dataKey <= endKey && checkIfExist(rangeRes, dataKey, range) == 0)
                        {
                            if (dataValue != GRAVEYARD)
                            {
                                (*value) = dataValue;
                                rangeRes[curInd].key = dataKey;
                                rangeRes[curInd].value = dataValue;
                                curInd = curInd + 1;
                            }
                            if (curInd == range)
                            {
                                fclose(fptr);
                                break;
                                //return 0;
                            }
                        }
                        //jdx++;
                    }
                    fclose(fptr);
                }
            }
            end1 = clock();
            seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
            time[0] = time[0] + seconds1;

            if (tobeMerged <= 1 || ptr_lvl->sorted == 1)
            {
                //printf("No Compaction\n");
                ptr_lvl = (*lsmt).levels[++i];
                continue;
            }
            
            // do compaction in that level (sort merge)

            start1 = clock();

            // Adding the data into an array
            data_chunk *data_bfr;
            data_bfr = (data_chunk *)malloc(ptr_lvl->curr_size * sizeof(data_chunk));
            jdx = 0;      
            for (idx = ptr_lvl->curr_runs - 1; idx >= 0; idx--)
            {
                run_size = ptr_lvl->runs[idx].run_size;
                num_files = (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize;
                bin_index = 0;

                for (int k = 0; k < num_files; k++)
                {
                    snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[k]);
                    fptr = fopen(file_bfr, "rb");                  
                    while (fread(help, sizeof(int), 2, fptr) == 2)
                    {                        
                        data_bfr[jdx].key = help[0];
                        data_bfr[jdx].value = help[1];
                        jdx++;

                    }
                     fclose(fptr);
                     remove(file_bfr);

                }
            }

        

            int out;
            int newSize = 0;
            // we now need to sort the data_bfr and proceed with everything else
            out = merge_sort(data_bfr, 0, ptr_lvl->curr_size - 1);

            if (out)
            {
                ptr_lvl->curr_size -= out;
            }

            newSize = ptr_lvl->curr_size;
            run_size = run_size0;

            //replacing the level with the sorted data 
            for (idx = 0; idx < numofRuns; idx++)
            {
                ptr_lvl->curr_runs = idx + 1;
                //run_size = (*lsmt).bfr->current_index;
                ptr_lvl->runs[idx].fence_p = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
                ptr_lvl->runs[idx].fence_start = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
                ptr_lvl->runs[idx].fence_end = malloc(sizeof(int) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);

                if (ptr_lvl->runs[idx].fence_p == NULL)
                {
                    return -1;
                }
                ptr_lvl->runs[idx].files = (char **)malloc(sizeof(char *) * (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
                if (ptr_lvl->runs[idx].files == NULL)
                {
                    return -1;
                }
                ptr_lvl->runs[idx].run_size = 0;
                // for each file create the file name array
                for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
                {
                    ptr_lvl->runs[idx].files[jdx] = (char *)malloc(sizeof(char) * CHAR_SIZE);
                    if (ptr_lvl->runs[idx].files[jdx] == NULL)
                    {
                        return -1;
                    }
                }

                // divide the files to pages
                //int start =0;

                //for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
                int numberOfPage = (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize);
                int start = idx * run_size;
                for (int jdx = 0; jdx < numberOfPage; jdx++)
                {

                    //printf("run %d and file %d \n", idx, jdx);

                    //printf("run %d \n", ptr_lvl->curr_runs);
                    snprintf(ptr_lvl->runs[idx].files[jdx], CHAR_SIZE, "l%dr%df%d", i, idx, jdx);

                    int lengthRun = 0;
                    lengthRun = newSize / numofRuns;
                    //step = pagesize / sizeof(data_chunk)
                    int arrayIndex = (int)(start + (jdx * run_size / numberOfPage));
                    //
                    int endArrayIndex = arrayIndex + (run_size / numberOfPage) - 1;
                    // printf(" array index file %d  end %d\n",  arrayIndex, endArrayIndex);
                    // printf("run %d and file %d \n", idx, jdx);

                    if (&data_bfr[arrayIndex] == NULL)
                    {
                        continue;
                    }

                    ptr_lvl->runs[idx].fence_start[jdx] = data_bfr[arrayIndex].key;

                    if (newSize >= endArrayIndex + 1)
                    {
                        ptr_lvl->runs[idx].fence_p[jdx] = data_bfr[endArrayIndex].key;
                        ptr_lvl->runs[idx].fence_end[jdx] = data_bfr[endArrayIndex].key;
                    }
                    else
                    {
                        ptr_lvl->runs[idx].fence_p[jdx] = data_bfr[newSize - 1].key;
                        ptr_lvl->runs[idx].fence_end[jdx] = data_bfr[newSize - 1].key;
                    }
                    
                    // write data to files
                    snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[jdx]);

                    fptr = fopen(file_bfr, "wb");
                    //printf("file name %s\n", ptr_lvl->runs[idx].files[jdx]);
                    // //start = jdx *run_size0 * pagesize / sizeof(data_chunk);

                    if (newSize >= endArrayIndex)
                    {
                        for (int arrIdx = arrayIndex; arrIdx < endArrayIndex; arrIdx++)
                        {
                            fwrite(&data_bfr[arrIdx].key, sizeof(int), 1, fptr);
                            fwrite(&data_bfr[arrIdx].value, sizeof(int), 1, fptr);
                            hash(ptr_lvl->blm_f, data_bfr[arrIdx].key);
                        }
                    }
                    else
                    {
                        for (int arrIdx = arrayIndex; arrIdx < newSize; arrIdx++)
                        {
                            fwrite(&data_bfr[arrIdx].key, sizeof(int), 1, fptr);
                            fwrite(&data_bfr[arrIdx].value, sizeof(int), 1, fptr);
                            hash(ptr_lvl->blm_f, data_bfr[arrIdx].key);
                          
                        }
                        //printf("\n");
                    }
                    fclose(fptr);
                    ptr_lvl->runs[idx].run_size = run_size0;
                }
            }

            end1 = clock();
            seconds1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
            time[1] = time[1] + seconds1;

            //printf("error here \n");
            ptr_lvl->curr_size = newSize;
            ptr_lvl->sorted = 1;

            //printf("no error here \n");

            ptr_lvl = (*lsmt).levels[++i];
            //printf("===== done one level ==== \n");
        }
    }

    //printf("time -  %0.08f  -- %0.08f \n", compactionTime, searchTime);

    return 1;
}

int checkIfExist(data_chunk rangeRes[], keyType key, int size)
{
    int i = 0;
    while (i < size)
    {
        if (rangeRes[i].key == key)
        {
            return 1;
        }
        i = i + 1;
    }
    //printf("checking key exist in array \n");
    return 0;
}

int get(lsmtree *lsmt, keyType key, valType *value)
{
    int found, i = 0, idx, jdx, current_size, bin_index, run_size, num_files, help[2];
    char file_bfr[CHAR_SIZE + EXTRA_SIZE];
    FILE *fptr;
    level *ptr_lvl;
    size_t pagesize = getpagesize();

    // check the lsmt tree is allocated
    if (lsmt == NULL)
    {
        return -1;
    }

    // check the buffer first
    found = get_buffer((*lsmt).bfr, key, value);
    if (found == 0)
    {
        return 0;
    }
    else if (found == -5)
    {
        // propagate -5 (deleted file)
        return -5;
    }
    else
    {
        // start checking the levels
        ptr_lvl = (*lsmt).levels[i];

        while (ptr_lvl != NULL)
        {

            //printf("level  %d\n", i);

            // check the bloom filter, and move to
            // the next level if it's not here
            if (!check_hash(ptr_lvl->blm_f, key))
            {
                ptr_lvl = (*lsmt).levels[++i];
                continue;
            }
            // check the runs in reverse order to make sure
            // we're checking the most recent one
            current_size = pagesize / sizeof(data_chunk);
            for (idx = ptr_lvl->curr_runs - 1; idx >= 0; idx--)
            {
                // we need to define the number of files
                run_size = ptr_lvl->runs[idx].run_size;
                num_files = (run_size * sizeof(data_chunk) + pagesize - 1) / pagesize;

                bin_index = binary_fences(ptr_lvl->runs[idx].fence_p, 0, num_files - 1, key);

                if (bin_index == -1)
                {
                    // the key is bigger than all the fence pointers
                    continue;
                }

                // if the file is the final file we know its probably not full
                if (bin_index == num_files - 1)
                {
                    current_size = ptr_lvl->runs[idx].run_size - (num_files - 1) * pagesize / sizeof(data_chunk);
                }

                data_chunk *data_bfr;
                //int key;
                //int val;
                data_bfr = (data_chunk *)malloc(current_size * sizeof(data_chunk));
                // bring the particular file in memory and binary search
                // to find the element we're looking for
                snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[bin_index]);
                fptr = fopen(file_bfr, "rb");

                jdx = 0;
                while (fread(help, sizeof(int), 2, fptr) == 2)
                {
                    data_bfr[jdx].key = help[0];
                    ;
                    data_bfr[jdx].value = help[1];
                    //key = help[0];
                    //val = help[1];
                    if (data_bfr[jdx].key == key)
                    {
                        if (data_bfr[jdx].value != GRAVEYARD)
                        {
                            (*value) = data_bfr[jdx].value;

                            fclose(fptr);
                            free(data_bfr);
                            return 0;
                        }
                        else
                        {
                            // -5 denotes deleted value
                            fclose(fptr);
                            free(data_bfr);
                            return -5;
                        }
                    }
                    jdx++;
                }
                fclose(fptr);
                free(data_bfr);

                // used to have a binary search for values
                // but since we're reading from the files
                // we can check it on the go
            }
            ptr_lvl = (*lsmt).levels[++i];
        }
    }

    // we didn't find it
    return -1;
}

int erase(lsmtree *lsmt, keyType key)
{
    int idx;

    //Check the lsmt tree is allocated
    if (lsmt == NULL)
    {
        return -1;
    }

    // We will check first if we can delete it from the buffer, so that we don't fill our tree with garbage
    if ((*lsmt).bfr->bfr_array == NULL)
    {
        // -3 to differentiate from the -1 of core structure missalocation
        return -3;
    }
    else
    {
        for (idx = 0; idx < (*lsmt).bfr->current_index; idx++)
        {
            if ((*lsmt).bfr->bfr_array[idx].key == key)
            {
                (*lsmt).bfr->bfr_array[idx].value = GRAVEYARD;
            }
        }
    }

    if (put(lsmt, key, GRAVEYARD) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int print_statistics(lsmtree *lsmt)
{
    int g_count = 0, l_count = 0, i = 0, idx, jdx, kdx, run_size;
    size_t pagesize = getpagesize();
    char file_bfr[CHAR_SIZE + EXTRA_SIZE];
    level *ptr_lvl;
    FILE *fptr;

    if (lsmt == NULL)
    {
        return -1;
    }

    if ((*lsmt).bfr == NULL)
    {
        return -1;
    }

    // Add the buffer stuff
    g_count += (*lsmt).bfr->current_index;
    l_count += (*lsmt).bfr->current_index;
    printf("LVL0: %d", l_count);

    l_count = 0;
    ptr_lvl = (*lsmt).levels[i];
    while (ptr_lvl != NULL)
    {
        g_count += (*lsmt).levels[i]->curr_size;
        l_count += (*lsmt).levels[i]->curr_size;

        printf(", LVL%d: %d", i + 1, l_count);
        ptr_lvl = (*lsmt).levels[++i];
        l_count = 0;
    }
    printf("\nLogical Pairs: %d\n", g_count);

    for (idx = 0; idx < (*lsmt).bfr->current_index; idx++)
    {
        if ((*lsmt).bfr->bfr_array[idx].value == GRAVEYARD)
        {
            printf("%d:DEL:L0 ", (*lsmt).bfr->bfr_array[idx].key);
        }
        else
        {
            printf("%d:%d:L0 ", (*lsmt).bfr->bfr_array[idx].key, (*lsmt).bfr->bfr_array[idx].value);
        }
    }
    printf("\n");

    i = 0;
    ptr_lvl = (*lsmt).levels[i];
    while (ptr_lvl != NULL)
    {
        int help[2];
        data_chunk *data_bfr;
        data_bfr = (data_chunk *)malloc(ptr_lvl->curr_size * sizeof(data_chunk));

        jdx = 0;
        for (idx = 0; idx < ptr_lvl->curr_runs; idx++)
        {
            // we have multiple files, we need to open all of them
            // we need the number of files of the previous level
            run_size = ptr_lvl->runs[idx].run_size;
            for (kdx = 0; kdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); kdx++)
            {
                snprintf(file_bfr, CHAR_SIZE + EXTRA_SIZE, "./data/%s.dat", ptr_lvl->runs[idx].files[kdx]);

                fptr = fopen(file_bfr, "rb");
                while (fread(help, sizeof(int), 2, fptr) == 2)
                {
                    data_bfr[jdx].key = help[0];
                    data_bfr[jdx].value = help[1];
                    jdx++;
                }
                fclose(fptr);
            }
        }

        for (idx = 0; idx < ptr_lvl->curr_size; idx++)
        {
            if (data_bfr[idx].value == GRAVEYARD)
            {
                printf("%d:DEL:L%d ", data_bfr[idx].key, i + 1);
            }
            else
            {
                printf("%d:%d:L%d ", data_bfr[idx].key, data_bfr[idx].value, i + 1);
            }
        }
        free(data_bfr);
        ptr_lvl = (*lsmt).levels[++i];
    }

    printf("\n----------------------------------------------------------------\n");
    return 0;
}

int deallocate(lsmtree *lsmt)
{
    // need to empty the LSM tree
    int i = 0, idx, jdx, run_size;
    level *ptr_lvl;
    size_t pagesize = getpagesize();

    ptr_lvl = (*lsmt).levels[i];
    while (ptr_lvl != NULL)
    {
        for (idx = 0; idx < ptr_lvl->curr_runs; idx++)
        {
            run_size = ptr_lvl->runs[idx].run_size;
            for (jdx = 0; jdx < (int)((run_size * sizeof(data_chunk) + pagesize - 1) / pagesize); jdx++)
            {
                free(ptr_lvl->runs[idx].files[jdx]);
            }
            free(ptr_lvl->runs[idx].fence_p);
            free(ptr_lvl->runs[idx].files);
        }
        deallocate_filter(ptr_lvl->blm_f);
        free(ptr_lvl->runs);
        free(ptr_lvl);
        ptr_lvl = (*lsmt).levels[++i];
    }

    deallocate_buffer((*lsmt).bfr);
    free((*lsmt).levels);
    free(lsmt);
    return 0;
}
