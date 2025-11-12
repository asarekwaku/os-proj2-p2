#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "BENSCHILLIBOWL.h"

#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS (NUM_CUSTOMERS * ORDERS_PER_CUSTOMER)

// Global variable for the restaurant
BENSCHILLIBOWL *bcb;

/**
 * Thread function that represents a customer.
 * A customer:
 *  - allocates space for an order
 *  - selects a random menu item
 *  - fills in customer ID
 *  - adds the order to the restaurant
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long)tid;
    int i;
    for (i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = (Order*)malloc(sizeof(Order));
        if (!order) {
            perror("malloc failed");
            pthread_exit(NULL);
        }
        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;
        order->next = NULL;

        int order_number = AddOrder(bcb, order);
        printf("Customer #%d placed order #%d (%s)\n",
               customer_id, order_number, order->menu_item);
    }
    pthread_exit(NULL);
}

/**
 * Thread function that represents a cook.
 * A cook:
 *  - repeatedly gets an order
 *  - if the order is valid, fulfills it and frees memory
 *  - stops when no more orders are available
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long)tid;
    int orders_fulfilled = 0;
    Order* order;

    while ((order = GetOrder(bcb)) != NULL) {
        printf("Cook #%d fulfilled order #%d (%s for Customer #%d)\n",
               cook_id, order->order_number, order->menu_item, order->customer_id);
        free(order);
        orders_fulfilled++;
    }

    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    pthread_exit(NULL);
}

/**
 * main()
 *  - Opens the restaurant
 *  - Creates customer and cook threads
 *  - Waits for all threads to finish
 *  - Closes the restaurant
 */
int main() {
    srand((unsigned int)time(NULL));

    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    int i;
    // Create customer threads
    for (i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)(long)(i + 1));
    }

    // Create cook threads
    for (i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)(long)(i + 1));
    }

    // Wait for customers to finish
    for (i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // Wait for cooks to finish
    for (i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }

    CloseRestaurant(bcb);
    return 0;
}
