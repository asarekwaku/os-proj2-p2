#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Menu definitions (provided) */
MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Helper function prototypes */
bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int r = rand() % BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[r];
}

/* Allocate memory for the Restaurant and initialize synchronization objects */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb = (BENSCHILLIBOWL*)malloc(sizeof(BENSCHILLIBOWL));
    if (bcb == NULL) {
        perror("Failed to allocate restaurant");
        return NULL;
    }

    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->expected_num_orders = expected_num_orders;

    pthread_mutex_init(&bcb->mutex, NULL);
    pthread_cond_init(&bcb->can_add_orders, NULL);
    pthread_cond_init(&bcb->can_get_orders, NULL);

    printf("Restaurant is open!\n");
    return bcb;
}

/* Close restaurant: verify all orders are fulfilled and cleanup */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&bcb->mutex);
    assert(bcb->orders_handled == bcb->expected_num_orders);
    pthread_mutex_unlock(&bcb->mutex);

    pthread_mutex_destroy(&bcb->mutex);
    pthread_cond_destroy(&bcb->can_add_orders);
    pthread_cond_destroy(&bcb->can_get_orders);

    free(bcb);
    printf("Restaurant is closed!\n");
}

/* Add an order to the back of queue (thread-safe) */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&bcb->mutex);

    /* Wait if restaurant is full */
    while (IsFull(bcb)) {
        pthread_cond_wait(&bcb->can_add_orders, &bcb->mutex);
    }

    /* Assign order number and enqueue */
    order->order_number = bcb->next_order_number++;
    order->next = NULL;

    AddOrderToBack(&bcb->orders, order);
    bcb->current_size++;

    /* Signal that an order is available */
    pthread_cond_signal(&bcb->can_get_orders);
    pthread_mutex_unlock(&bcb->mutex);

    return order->order_number;
}

/* Remove an order from the front of queue (thread-safe) */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&bcb->mutex);

    /* Wait until not empty OR until all expected orders have been processed */
    while (IsEmpty(bcb) && bcb->orders_handled < bcb->expected_num_orders) {
        pthread_cond_wait(&bcb->can_get_orders, &bcb->mutex);
    }

    /* No more orders to process */
    if (bcb->orders_handled >= bcb->expected_num_orders) {
        pthread_mutex_unlock(&bcb->mutex);
        pthread_cond_broadcast(&bcb->can_get_orders);
        return NULL;
    }

    /* Pop order from front */
    Order* order = bcb->orders;
    if (order != NULL) {
        bcb->orders = order->next;
        bcb->current_size--;
        bcb->orders_handled++;
    }

    /* Signal that space is now available */
    pthread_cond_signal(&bcb->can_add_orders);
    pthread_mutex_unlock(&bcb->mutex);

    return order;
}

/* Helper: add order to end of queue */
void AddOrderToBack(Order **orders, Order *order) {
    if (*orders == NULL) {
        *orders = order;
    } else {
        Order *temp = *orders;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = order;
    }
}

/* Check if restaurant queue is empty */
bool IsEmpty(BENSCHILLIBOWL* bcb) {
    return bcb->current_size == 0;
}

/* Check if restaurant queue is full */
bool IsFull(BENSCHILLIBOWL* bcb) {
    return bcb->current_size >= bcb->max_size;
}
