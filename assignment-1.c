#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>

// Global variables
int *numbers;
int count;
double average;
bool is_avg_calculated = false;
int minimum;
int maximum;
double stddev;
pthread_mutex_t lock;
pthread_cond_t cv;

// Function to calculate the average
void *calculate_average(void *arg) {
    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += numbers[i];
    }

    pthread_mutex_lock(&lock);

    average = sum / count;
    is_avg_calculated = true;

    // Notify average has been calculated
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lock);
}

// Finds the minimum value
void *calculate_min(void *arg) {
    minimum = numbers[0];
    for (int i = 1; i < count; i++) {
        if (numbers[i] < minimum) {
            minimum = numbers[i];
        }
    }
}

// Finds the maximum value
void *calculate_max(void *arg) {
    maximum = numbers[0];
    for (int i = 1; i < count; i++) {
        if (numbers[i] > maximum) {
            maximum = numbers[i];
        }
    }
}

// Calculates the standard deviation
void *calculate_stddev(void *arg) {
    pthread_mutex_lock(&lock);
    // Wait for average to be calculated
    while (!is_avg_calculated) {
        pthread_cond_wait(&cv, &lock);
    }
    pthread_mutex_unlock(&lock);

    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += pow(numbers[i] - average, 2);
    }
    stddev = sqrt(sum / count);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num1> <num2> ...\n", argv[0]);
        return 1;
    }
    
    // Create array of numbers
    count = argc - 1;
    numbers = malloc(count * sizeof(int));
    if (numbers == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Convert CLI arguments to int and append to array
    for (int i = 0; i < count; i++) {
        numbers[i] = atoi(argv[i]+1);
    }
    
    // Thread pool
    pthread_t threads[4];
    
    // Thread creations
    pthread_create(&threads[0], NULL, calculate_average, NULL);
    pthread_create(&threads[1], NULL, calculate_min, NULL);
    pthread_create(&threads[2], NULL, calculate_max, NULL);
    pthread_create(&threads[3], NULL, calculate_stddev, NULL);

    // Thread Join
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("The average value is %.2f\n", average);
    printf("The minimum value is %d\n", minimum);
    printf("The maximum value is %d\n", maximum);
    printf("The Standard Deviation is %.2f\n", stddev);
    
    free(numbers);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&lock);

    return 0;
}
