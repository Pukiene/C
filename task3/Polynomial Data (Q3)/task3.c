#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define MAX_TERMS 5
#define MAX_POLYNOMIALS 100
#define MAX_LINE_LENGTH 256

// Structure to store polynomial data
typedef struct {
    int coefficients[MAX_TERMS];
    int powers[MAX_TERMS];
    int term_count;
    int x_value;
} Polynomial;

// Structure to pass thread data
typedef struct {
    Polynomial *polynomials;
    int start;
    int end;
    FILE *output_file;
    pthread_mutex_t *file_mutex;
} ThreadData;

// Function to parse a single polynomial from a line of input
void parse_polynomial(char *line, Polynomial *poly) {
    char *y_part = strtok(line, "=");
    char *polynomial_part = strtok(NULL, "(");
    char *x_part = strtok(NULL, ")");

    poly->term_count = 0;
    poly->x_value = atoi(x_part + 2); // Skip "x="

    char *term = strtok(polynomial_part, "+");
    while (term != NULL) {
        int coef = 0, power = 0;
        if (sscanf(term, "%dx^%d", &coef, &power) == 2) {
            // Term with coefficient and power
        } else if (sscanf(term, "%dx", &coef) == 1) {
            // Term with coefficient and x (power = 1)
            power = 1;
        } else if (sscanf(term, "%d", &coef) == 1) {
            // Constant term (power = 0)
            power = 0;
        }
        poly->coefficients[poly->term_count] = coef;
        poly->powers[poly->term_count] = power;
        poly->term_count++;
        term = strtok(NULL, "+");
    }
}

// Function to calculate the derivative and evaluate gradient
void calculate_gradient(Polynomial *poly, char *derivative, int *gradient) {
    *gradient = 0;
    derivative[0] = '\0';

    for (int i = 0; i < poly->term_count; i++) {
        int coef = poly->coefficients[i];
        int power = poly->powers[i];

        if (power > 0) {
            int new_coef = coef * power;
            int new_power = power - 1;
            char term[50];

            if (new_power > 0) {
                sprintf(term, "%dx^%d", new_coef, new_power);
            } else {
                sprintf(term, "%d", new_coef);
            }

            if (strlen(derivative) > 0) {
                strcat(derivative, " + ");
            }
            strcat(derivative, term);

            *gradient += new_coef * pow(poly->x_value, new_power);
        }
    }
}

// Thread function
void *process_polynomials(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start; i < data->end; i++) {
        Polynomial *poly = &data->polynomials[i];
        char derivative[MAX_LINE_LENGTH];
        int gradient;

        calculate_gradient(poly, derivative, &gradient);

        pthread_mutex_lock(data->file_mutex);
        fprintf(data->output_file, "%s = %d\n", derivative, gradient);
        pthread_mutex_unlock(data->file_mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <input_file>\n", argv[0]);
        return 1;
    }

    int thread_count = atoi(argv[1]);
    const char *input_file = argv[2];

    FILE *file = fopen(input_file, "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    Polynomial polynomials[MAX_POLYNOMIALS];
    int poly_count = 0;
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) && poly_count < MAX_POLYNOMIALS) {
        parse_polynomial(line, &polynomials[poly_count]);
        poly_count++;
    }
    fclose(file);

    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Failed to open output file");
        return 1;
    }

    pthread_t threads[thread_count];
    ThreadData thread_data[thread_count];
    pthread_mutex_t file_mutex;
    pthread_mutex_init(&file_mutex, NULL);

    int slice = poly_count / thread_count;
    int remainder = poly_count % thread_count;
    int start = 0;

    for (int i = 0; i < thread_count; i++) {
        thread_data[i].polynomials = polynomials;
        thread_data[i].start = start;
        thread_data[i].end = start + slice + (i < remainder ? 1 : 0);
        thread_data[i].output_file = output_file;
        thread_data[i].file_mutex = &file_mutex;

        pthread_create(&threads[i], NULL, process_polynomials, &thread_data[i]);
        start = thread_data[i].end;
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&file_mutex);
    fclose(output_file);

    return 0;
}

