#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define MAX_LINE_LENGTH 256

typedef struct {
    char equation[MAX_LINE_LENGTH];
    int  x_value;
    double result;
} Polynomial;

/*
A dynamic array of polynomials  yra tarsi tamprus konteineris, kuriame gali buti daug polynomial  lygcių. sis konteineris gali padideti, jei jums reikia daugiau vietos, ir susitraukti, kai jums nereikia tiek daug.

*/
static Polynomial *polynomials = NULL;
static int count = 0;      // numeris of polynomials dabar saugoma 0
static int capacity = 0;   //  kiek yra paskirta vietos'polynomials'

// Shared index for threads to process next polynomial 
static int current_index = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * chaks 'polynomials' has enough spase for 'needed' items.
 * If not , it doubles the capacity until it's sufficient, using realloc.
 */
static void ensure_capacity(int needed) {
    if (needed <= capacity) {
        return; // enough capacity
    }
    if (capacity == 0) {
        capacity = 10; // some small capacity
    }
    while (capacity < needed) {
        capacity *= 2;
    }

    Polynomial *temp = realloc(polynomials, capacity * sizeof(Polynomial));
    if (!temp) {
        fprintf(stderr, "Failed to realloc polynomials array.\n");
        free(polynomials);
        exit(1);
    }
    polynomials = temp;
}

/**
 * Removes trailing newline, trailing spaces, and an optional "y = " 
 */
static void clean_equation_for_output(char *equation) {
    //  Remove any trailing '\r' tab enter or '\n tarpas'
    equation[strcspn(equation, "\r\n")] = '\0';

    //  Remove spaces
    size_t len = strlen(equation);
    while (len > 0 && equation[len - 1] == ' ') {
        equation[--len] = '\0';
    }

    //Remove  y = in front
    const char *prefix = "y = ";
    size_t prefix_len = strlen(prefix);
    if (strncmp(equation, prefix, prefix_len) == 0) {
        memmove(equation, equation + prefix_len, strlen(equation) - prefix_len + 1);
    }
}

//fom end takes x-value from "(x=N)" 
 
static int extract_x_value(const char *line) {
    const char *x_start = strstr(line, "(x=");
    if (x_start) {
        return atoi(x_start + 3); // ignores the beginn (x="
    }
    return 0;
}

/**
  Evaluates a polynomial at x=x_value.
  Splits on ' ' and '+' 
 */
static double evaluate_polynomial(char *equation, int x_value) {
    double result = 0.0;
    char *token = strtok(equation, " +");

    while (token) {
        int coef = 0, power = 0;

        // pvz "2x^3"
        if (sscanf(token, "%dx^%d", &coef, &power) == 2) {
            result += coef * pow(x_value, power);
        }
        // pvz "2x"
        else if (sscanf(token, "%dx", &coef) == 1) {
            char *x_pos = strstr(token, "x");
            if (x_pos && *(x_pos + 1) == '\0') {
                result += coef * x_value;
            } else {
                sscanf(token, "%d", &coef);
                result += coef;
            }
        }
        // pvz "2"
        else if (sscanf(token, "%d", &coef) == 1) {
            result += coef;
        }

        token = strtok(NULL, " +");
    }
    return result;
}

//Thread function. Repeatedly picks the next polynomial from current_index.

static void* thread_func(void *arg) {
    (void)arg; // nenaudojamas

    while (1) {
        // uzrakina ir issaugo ir increment current_index
        pthread_mutex_lock(&mutex);
        if (current_index >= count) {
            pthread_mutex_unlock(&mutex);
            break; // No more polynomials
        }
        int idx = current_index++;
        pthread_mutex_unlock(&mutex);

        // ivertina polynomials[idx]
        char local_equation[MAX_LINE_LENGTH];
        strcpy(local_equation, polynomials[idx].equation);

        clean_equation_for_output(local_equation);

        double val = evaluate_polynomial(local_equation, polynomials[idx].x_value);

        // isaugo resultata lykties kvieno fredo
        polynomials[idx].result = val;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_threads> <input_file>\n", argv[0]);
        return 1;
    }

    // Number of threads from argv[1] number of threads small tasks that can run in paralle
    int num_threads = atoi(argv[1]);
    const char *input_file = argv[2];

    // Open the input file
    FILE *file = fopen(input_file, "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    // Read all line seperetli, expand (padidinti) polynomials array if needed
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove  \r or \n
        line[strcspn(line, "\r\n")] = '\0';

        // Ensure we have enough space for 1 more polynomial
        ensure_capacity(count + 1);

        // Copy the lykties line and parse x_value
        strcpy(polynomials[count].equation, line);
        polynomials[count].x_value = extract_x_value(line);
        polynomials[count].result  = 0.0;

        count++;
    }
    fclose(file);

    // sukurti tam tikra skaiciu fredu
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    if (!threads) {
        fprintf(stderr, "Failed to allocate thread array.\n");
        free(polynomials); 
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free(threads);
            free(polynomials);
            return 1;
        }
    }

    // sujunge visus threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);

    // sunaikina  mutex (mutrex one thread can access a resource at a timeone)
    pthread_mutex_destroy(&mutex);

    // print results to output.txt
    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Failed to open output file");
        free(polynomials);
        return 1;
    }

    for (int i = 0; i < count; i++) {
        // Re-clean for printing
        char eq_print[MAX_LINE_LENGTH];
        strcpy(eq_print, polynomials[i].equation);
        clean_equation_for_output(eq_print);

        fprintf(output_file, "%s m=%.2f\n", eq_print, polynomials[i].result);
    }
    fclose(output_file);

    // Free the polynomials array
    free(polynomials);

    return 0;
}




//cd /home/2334768/Desktop/task3
//gcc -o task3 task3.c -lpthread -lm
//./task3 4 Polynomials1.txt

