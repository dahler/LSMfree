#include <fstream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

void generate_one_file(unsigned long long pTOTAL_NUMBERS, unsigned int pdomain, unsigned long long pL, short ppercent_outRange, short plpercentage, int pseed);
std::string generate_partitions_stream(unsigned long long TOTAL_NUMBERS, unsigned int domain, unsigned long long L, short percent_outRange, short l_percentage, int seed, std::string folder);

void generate_one_file(unsigned long long pTOTAL_NUMBERS, unsigned int pdomain, unsigned long long pL, short ppercent_outRange, short plpercentage, int pseed = 1)
{
    try
    {

        std::ofstream outfile;

        srand(time(NULL));
        outfile.open("./generated-data/dataledger.txt", std::ios_base::app);

        std::string folder_name = "generated-data";
        outfile << generate_partitions_stream(pTOTAL_NUMBERS, pdomain, pL, ppercent_outRange, plpercentage, pseed, folder_name) << std::endl;

        outfile.close();
    }
    catch (int e)
    {
        cout << "An exception occurred. Exception Nr. " << e << '\n';
    }
}

unsigned int generate_random_in_range(unsigned long long position, unsigned long long Total_Numbers, double l_percentage)
{
    unsigned int l = Total_Numbers * l_percentage;

    if ((int)(position - l) <= 0)
    {

        return (rand() % (position + l));
    }
    else if ((int)(position + l) >= Total_Numbers)
    {

        return (rand() % (Total_Numbers - (position - l))) + (position - l);
    }
    else
    {

        return (rand() % ((position + l) - (position - l))) + (position - l);
    }
}

/*
    Function which generates uniform data over some domain, and write it in binary format.
    Each partition of L elements is shuffled, and has some noise (randomness) linked to the
    percent_outRange parameter.
    */
std::string generate_partitions_stream(unsigned long long TOTAL_NUMBERS, unsigned int domain, unsigned long long L, short percent_outRange, short l_percentage, int seed, std::string folder = "./generated-data")
{
    float p_outOfRange = percent_outRange / 100.0;

    std::srand(seed);

    //unsigned long long array[TOTAL_NUMBERS];
    std::vector<unsigned long long> array(TOTAL_NUMBERS, 0);

    std::string fname = folder;
    fname += "/data_";
    fname += std::to_string(TOTAL_NUMBERS);
    fname += "-elems_";
    fname += std::to_string(percent_outRange);
    fname += "-kperct_";
    fname += std::to_string(l_percentage);
    fname += "-lperct_";
    fname += std::to_string(seed);
    fname += "seed";
    fname += std::to_string(std::time(nullptr));
    fname += ".dat";

    cout << folder << endl;
    cout << fname << endl;

    std::ofstream myfile1(fname, std::ios::binary);

    unsigned long long noise_limit = TOTAL_NUMBERS * p_outOfRange;
    unsigned long long noise_counter = 0;

    for (unsigned long long i = 0; i < TOTAL_NUMBERS; i++)
    {
        array[i] = i;

        float ran = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        //randomize generation
        if (ran < p_outOfRange && noise_counter < noise_limit)
        {
            //generate position of shuffle
            //unsigned long long r = (rand() % TOTAL_NUMBERS);
            unsigned long long r = generate_random_in_range(i, TOTAL_NUMBERS, l_percentage / 100.0);

            if (array[r] == 0)
            {
                if (r != 0)
                {
                    array[r] = r;
                }
            }

            unsigned long long temp = array[i];
            array[i] = array[r];
            array[r] = temp;

            noise_counter++;
        }
    }

    for (unsigned long long j = 0; j < TOTAL_NUMBERS; ++j)
    {
        myfile1.write(reinterpret_cast<char *>(&array[j]), sizeof(int));
    }

    myfile1.close();

    return fname;
}

//arguments to program:
//unsigned long long pTOTAL_NUMBERS, unsigned int pdomain, unsigned long long pL, short ppercent_outRange, int pseed

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: ./execs/workload_generator.exe totalNumbers noisePercentage windowThreshold" << std::endl;
        return 0;
    }

    unsigned long long totalNumbers = atoi(argv[1]);
    short noisePercentage = atoi(argv[2]);
    short lPercentage = atoi(argv[3]);

    unsigned long long domain = totalNumbers;
    unsigned long long windowSize = 1;
    int seedValue = 1;

    // simple checks
    if (totalNumbers < 0)
    {
        std::cout << "No. of Elements cannot be < 0" << std::endl;
        exit(1);
    }
    if (noisePercentage < 0 || noisePercentage > 50)
    {
        std::cout << "Please ensure 0 <= noisePercentage <= 50" << std::endl;
        exit(1);
    }
    if (lPercentage < 0 || lPercentage > 50)
    {
        std::cout << "Please ensure 0 <= lPercentage <= 50" << std::endl;
        exit(1);
    }

    generate_one_file(totalNumbers, domain, windowSize, noisePercentage, lPercentage, seedValue);
}