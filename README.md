
##  BensChilliBowl — Thread Synchronization Project

This project simulates a restaurant system where **customers** place orders and **cooks** fulfill them using **POSIX threads** (`pthread`). It demonstrates synchronization with **mutexes** and **condition variables** following the producer–consumer model.

###  Key Points

* Customers (producers) add orders to a shared queue.
* Cooks (consumers) remove and fulfill orders.
* Uses thread-safe operations with proper locking and signaling.
* Clean startup and shutdown with all orders processed.

###  Compile & Run

```bash
gcc -pthread -o bens main.c BENSCHILLIBOWL.c
./bens
```

###  Concepts

* Multithreading
* Mutual exclusion (mutex)
* Condition variables
* Producer–consumer synchronization
