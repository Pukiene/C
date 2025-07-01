#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct structVariables {
    long long start;
    long long end;
    double partialSum;
};

//funkcija Leibnizo  dalies apskacevimas
void *calculatePi(void *sqvars) {
    struct structVariables *thread_struct = (struct structVariables *)sqvars;
    long long startVal = thread_struct->start;// Get starting index for this thread
    long long endVal = thread_struct->end;//ending index fredui
    double localSum = 0.0;


// Calculate the partial sum of the Leibniz
    for (long long num = startVal; num <= endVal; num++) {
        double term = (num % 2 == 0 ? 1.0 : -1.0) / (2.0 * num + 1.0);// Alternate between +1 and -1
        localSum += term;
    }

    thread_struct->partialSum = localSum; // Store sum in the structure
    pthread_exit(0);// exsed fred
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <number_of_iterations> <number_of_threads>\n", argv[0]);
        return -1;
    }

    long long compCount = atoll(argv[1]); // Number of iterations
    int threadCount = atoi(argv[2]);     // skaicius kiek fredu

    if (threadCount <= 0 || compCount <= 0) {
        printf("Both terations and iterations  should be positive integers.\n");
        return -1;
    }

    int sliceList[threadCount]; // lakyti vienodas dalis is initially
    int remainder = compCount % threadCount;// pasiema papildomai freda jai yra liginis kaicius


// kiekvenam fredo prskeremas dydis 
    for (int i = 0; i < threadCount; i++) {
        sliceList[i] = compCount / threadCount;
    }
    for (int j = 0; j < remainder; j++) {
        sliceList[j] += 1;
    }


 // Calculate start and end indices for each thread
    int startList[threadCount];
    int endList[threadCount];

    for (int k = 0; k < threadCount; k++) {
        if (k == 0) {
            startList[k] = 0;
            endList[k] = sliceList[k] - 1;
        } else {
            startList[k] = endList[k - 1] + 1;
            endList[k] = startList[k] + sliceList[k] - 1;
        }
    }

    struct structVariables mainStruct[threadCount];
    pthread_t threadIDs[threadCount];

    for (int n = 0; n < threadCount; n++) {
        mainStruct[n].start = startList[n];
        mainStruct[n].end = endList[n];
        mainStruct[n].partialSum = 0.0;//// Initialize partial sum to 0

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&threadIDs[n], &attr, calculatePi, &mainStruct[n]); // Create fred
    }

    double totalSum = 0.0; //total sum from all fred

    for (int n = 0; n < threadCount; n++) {
        pthread_join(threadIDs[n], NULL);
        totalSum += mainStruct[n].partialSum;
    }
// Multiply total sum by 4 to get the approximation of Pi
    double pi = totalSum * 4.0;

    printf("Calculated value of Pi: %.15f\n", pi);
    return 0;//print approximation pi
}


// cd
//gcc -o task2 task2.c -pthread
//./task2 1000000 4
