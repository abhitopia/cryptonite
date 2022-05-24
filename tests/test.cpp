//
// Created by Abhishek Aggarwal on 23/05/2022.
//

#include "test.h"

void test_genetic_algorithm() {
    cryptonite::seed(0);
    auto dna = TestDNA::getTestDNA(10);
    GeneticAlgorithm<TestDNA> ga{dna, 1000, 0.4, 0.9 };
    double start_time = omp_get_wtime();
    ga.optimize(10, true);
    std::cout << "Time taken:" << omp_get_wtime() - start_time << std::endl;
}
