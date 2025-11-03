#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define NUM_ORDERS 1000000 

typedef struct {
    char order_id[20];
    time_t timestamp;
} Order;

void generate_sample_orders(Order *orders, int n) {
    struct tm base_time = {0};
    base_time.tm_year = 2025 - 1900;
    base_time.tm_mon = 5; // June (0-based)
    base_time.tm_mday = 24;
    base_time.tm_hour = 12;
    time_t base = mktime(&base_time);
    
    for (int i = 0; i<n; i++) {
        int random_minutes = rand() % 100000; // up to ~70 days
        orders[i].timestamp = base + (random_minutes * 60);
        sprintf(orders[i].order_id, "ORD%d", i + 1);
    }
}
void merge(Order *orders, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    Order *L = malloc(n1 * sizeof(Order));
    Order *R = malloc(n2 * sizeof(Order));
    
    for (int i = 0; i<n1; i++) L[i] = orders[left + i];
    for (int j = 0; j<n2; j++) R[j] = orders[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i<n1 && j<n2) {
        if (L[i].timestamp<= R[j].timestamp)
            orders[k++] = L[i++];
        else
            orders[k++] = R[j++];
    }
    while (i<n1) orders[k++] = L[i++];
    while (j<n2) orders[k++] = R[j++];
    
    free(L);
    free(R);
}
void merge_sort(Order *orders, int left, int right) {
    if (left< right) {
        int mid = left + (right - left) / 2;
        merge_sort(orders, left, mid);
        
        merge_sort(orders, mid + 1, right);
        
        merge(orders, left, mid, right);
    
    }
}

void print_first_n_orders(Order *orders, int n) {
    char time_str[30];
    for (int i = 0; i<n; i++) {
        struct tm *tm_info = gmtime(&orders[i].timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%SZ", tm_info);
        
        printf("Order ID: %s, Timestamp: %s\n", orders[i].order_id, time_str);
    }
}
int main() {
    srand(time(NULL));
    Order *orders = malloc(NUM_ORDERS * sizeof(Order));
    
    printf("Generating orders...\n");
    generate_sample_orders(orders, NUM_ORDERS);
    
    printf("Sorting orders by timestamp...\n");
    clock_t start = clock();
    merge_sort(orders, 0, NUM_ORDERS - 1);
    clock_t end = clock();
    
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Done! Sorted %d orders in %.2f seconds.\n", NUM_ORDERS, time_taken);
    
    printf("\n First 5 Sorted Orders:\n");
    print_first_n_orders(orders, 5);
    free(orders);
    
    return 0;
}
