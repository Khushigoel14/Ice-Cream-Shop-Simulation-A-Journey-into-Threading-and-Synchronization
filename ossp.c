#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/syscall.h>
#include <linux/kernel.h>

// Prices of flavors and toppings in rupees
#define FLAVOR_PRICE_1 80.0
#define FLAVOR_PRICE_2 150.0
#define FLAVOR_PRICE_3 120.0
#define TOPPING_PRICE_1 48.0
#define TOPPING_PRICE_2 30.0

// Resources
int availableTickets = 30;
int flavorStock[3] = {29, 34, 18};
int toppingStock[2] = {20, 34};
double totalRevenue = 0.0;

// Semaphores declaration
sem_t semTicket, semFlavor, semF1, semF2, semF3, semTopping, semT1, semT2, semPayment;

// Function prototype
void *customerVisit(void *customerId);

// Main function
int main() {
    int numCustomers;

    printf("\n\nEnter the number of customers [1-%d]: ", availableTickets);
    scanf("%d", &numCustomers);
    printf("\n\n");

    if (numCustomers > availableTickets || numCustomers <= 0) {
        printf("\n\nInvalid Input! Please enter a number between 1 and %d.\n\n", availableTickets);
        return 0;
    }

    int customerIds[numCustomers];

    // Assign unique IDs to each customer
    for (int i = 0; i < numCustomers; i++) {
        customerIds[i] = i + 100;
    }

    // Initialize semaphores
    sem_init(&semTicket, 0, 1);
    sem_init(&semFlavor, 0, 3);
    sem_init(&semF1, 0, 1);
    sem_init(&semF2, 0, 1);
    sem_init(&semF3, 0, 1);
    sem_init(&semTopping, 0, 2);
    sem_init(&semT1, 0, 1);
    sem_init(&semT2, 0, 1);
    sem_init(&semPayment, 0, 1);

    // Create threads for each customer
    pthread_t customerThreads[numCustomers];

    for (int i = 0; i < numCustomers; i++) {
        pthread_create(&customerThreads[i], NULL, &customerVisit, (void *)&customerIds[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < numCustomers; i++) {
        pthread_join(customerThreads[i], NULL);
    }

    // Display final business journal
    printf("\n\nBusiness Journal - At Closing\n\n");
    printf("Number Of Customers Served: %d", numCustomers);
    printf("\nTotal Revenue Generated: ₹ %.2f", totalRevenue);
    printf("\nTickets Remaining: %d\n\n", availableTickets);

    // Destroy all semaphores
    sem_destroy(&semTicket);
    sem_destroy(&semFlavor);
    sem_destroy(&semF1);
    sem_destroy(&semF2);
    sem_destroy(&semF3);
    sem_destroy(&semTopping);
    sem_destroy(&semT1);
    sem_destroy(&semT2);
    sem_destroy(&semPayment);

    return 0;
}

// Function executed by each customer thread
void *customerVisit(void *customerId) {
    int id = *(int *)customerId;
    double billAmount = 0.0;
    int flavorSelected = 0;

    // Ticket counter
    sem_wait(&semTicket);

    if (availableTickets <= 0) {
        syscall(333, "Leaving Shop. [REASON]: No Tickets Available\n", id);
        sleep(1);
        pthread_exit(NULL);
    }

    availableTickets--;
    syscall(333, "Acquired Ticket.\n", id);
    sem_post(&semTicket);

    sleep(2);

    // Flavor selection
    sem_wait(&semFlavor);

    if (flavorStock[0] <= 0 && flavorStock[1] <= 0 && flavorStock[2] <= 0) {
        syscall(333, "Leaving Shop. [REASON]: Flavors Out of Stock\n", id);
        sleep(1);
        pthread_exit(NULL);
    } else {
        // Flavor 1 selection
        sem_wait(&semF1);

        if (flavorStock[0] > 0) {
            flavorStock[0]--;
            flavorSelected++;
            billAmount += FLAVOR_PRICE_1;
            syscall(333, "Selected Flavor 1.\n", id);
            sleep(1);
        }

        sem_post(&semF1);

        // Flavor 2 selection
        sem_wait(&semF2);

        if (flavorStock[1] > 0) {
            flavorStock[1]--;
            flavorSelected++;
            billAmount += FLAVOR_PRICE_2;
            syscall(333, "Selected Flavor 2.\n", id);
            sleep(1);
        }

        sem_post(&semF2);

        // Flavor 3 selection
        sem_wait(&semF3);

        if (flavorStock[2] > 0) {
            flavorStock[2]--;
            flavorSelected++;
            billAmount += FLAVOR_PRICE_3;
            syscall(333, "Selected Flavor 3.\n", id);
            sleep(1);
        }

        sem_post(&semF3);

        // Exit if no flavor could be selected
        if (flavorSelected == 0) {
            syscall(333, "Leaving Shop. [REASON]: No Flavors Available\n", id);
            sleep(1);
            pthread_exit(NULL);
        }
    }

    syscall(333, "Flavor Selection Complete. Moving On.\n", id);
    sem_post(&semFlavor);

    // Topping selection
    sem_wait(&semTopping);

    // Topping 1
    sem_wait(&semT1);

    if (toppingStock[0] > 0) {
        toppingStock[0]--;
        billAmount += TOPPING_PRICE_1;
    }

    sem_post(&semT1);

    // Topping 2
    sem_wait(&semT2);

    if (toppingStock[1] > 0) {
        toppingStock[1]--;
        billAmount += TOPPING_PRICE_2;
    }

    sem_post(&semT2);

    syscall(333, "Topping Selection Complete.\n", id);
    sem_post(&semTopping);

    sleep(2);

    // Payment section
    sem_wait(&semPayment);

    totalRevenue += billAmount;

    char paymentMessage[100];
    sprintf(paymentMessage, "Total Billed Amount: ₹ %.2f.\n", billAmount);
    syscall(333, paymentMessage, id);

    sem_post(&semPayment);

    sleep(2);

    syscall(333, "Exiting Ice-Cream Shop.\n\n", id);

    return NULL;
}
