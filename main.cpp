#include <iostream>
#include <omp.h>
#include "dataset.h"


void openMP() {
    int i;
    int threadID = 0;
    #pragma omp parallel for private(i, threadID)
    for(i = 0; i < 20; i++ )
    {
        threadID = omp_get_thread_num();

        #pragma omp critical
        {
            printf("Thread %d reporting\n", threadID);
        }
    }
}

int main() {
    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");
    std::cout << dataset.open[0] << dataset.open[1] <<endl;
    std::string hello = "Hello World!";
    openMP();
    return 0;
}
