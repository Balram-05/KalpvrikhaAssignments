#include <stdio.h>
#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400

int main(){
    char name[50];
    int tickets, choice;
    int price;
    float subtotal, discount, finalBill;

    printf("Enter name ");
    scanf("%49s", name);
    printf("Enter number of tickets ");
    scanf("%d", &tickets);
    printf("Enter choice ");
    scanf("%d", &choice);

    if(tickets < 1 || tickets > 20){
        printf("Error: Invalid number of tickets");
        return 0;
    }

    switch (choice){
        case 1: price = STANDARD_PRICE;
        break;

        case 2: price = PREMIUM_PRICE;
        break;

        case 3: price = RECLINER_PRICE;
        break;

        default: printf("Error: Invalid ticket category \n");
        return 0;
    }

    subtotal = price * tickets;

    if(tickets >= 5){
        discount = subtotal * 0.10;
    } else {
        discount = 0;
    }

    finalBill = subtotal - discount;
    printf("Customer: %s\n", name);
    printf("Ticket Category: ");

    switch (choice){
        case 1: printf("Standard\n");
        break;

        case 2: printf("Premium\n");
        break;

        case 3: printf("Recliner\n");
        break;
    }

    printf("Tickets: %d\n", tickets);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", finalBill);

    return 0;
}