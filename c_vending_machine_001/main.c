//
//  main.c
//  c_vending_machine_001
//
//  Created by Junhyuek Kim on 2022/02/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define ITEM_SIZE 10
#define WHOLESALE 0.9 // Wholesale price when restock
#define MAX_NAME_LENGTH 10
#define TIMESTAMP_LENGTH 30
//For record of buy and sell history
typedef enum Type{
    SELL = 0,
    BUY
}historyType_t;

// For timestamp
time_t now;
struct tm *t;

//MARK: - Structure

// Item info
typedef struct ITEM{
    char name[MAX_NAME_LENGTH];
    int number;
    int price;
    int amount;
}item_t;

// User shopping list info
typedef struct LIST{
    item_t items[ITEM_SIZE];
    int total;
}list_t;

// User info
typedef struct USER{
    int cash;
    list_t list;
}user_t;

// History info
typedef struct HISTORY{
    char time[TIMESTAMP_LENGTH]; // Timestamp
    int type; // Buy or sell
    item_t item;
    struct HISTORY *next; // Link for linked list(history)
}history_t;

// Machine info
typedef struct MACHINE{
    int cash;
    item_t items[ITEM_SIZE];
    history_t *history;
}machine_t;


void generateItems(machine_t *machine);//Fixed item contents
void startMachine(machine_t *machine, user_t *user, int *selectMenu);


int chooseItem(void);
int findIndex(item_t *item, user_t *user);

void addItem(user_t *user, item_t *item, int index);
void addCash(user_t *user);
void itemToList(machine_t *machine, user_t *user);
void supplyItems(machine_t *machine);

void pay(user_t *user, machine_t *machine);
void payment(machine_t *machine, user_t *user);

void arrangeList(user_t *user, int index, int last);
void editList(machine_t *machine, user_t *user);
void clearList(machine_t *machine, user_t *user);
void deleteFromList(user_t *user, int index);

void addHistory(history_t *history, item_t *item, historyType_t type);
void freeMalloc(history_t *history);

void printItems(machine_t *machine);
void printList(user_t *user);
void printUserBalance(user_t *user);
void printMachineBalance(machine_t *machine);
void printHistory(history_t *history);

void pressEnter(void);
int calTotal(user_t *user);
int listLength(user_t *user);



int main(int argc, const char * argv[]) {
    
    user_t user = {0};
    machine_t machine = {0,};
    
    int flag = 0;
    int selectMenu = 0;
    
    // Create head
    machine.history = (history_t*)malloc(sizeof(history_t));
    machine.history -> next = NULL;
    
    
    system("clear");
    
    generateItems(&machine);
    printItems(&machine);
    printUserBalance(&user);
    
    printf("\nPlease insert cash : ");
    scanf("%d", &user.cash);
    
    while (!flag) {
        startMachine(&machine, &user, &selectMenu);
        switch (selectMenu) {
            case 1:
                itemToList(&machine, &user);
                break;
            case 2:
                payment(&machine, &user);
                break;
            case 3:
                addCash(&user);
                break;
            case 4:
                supplyItems(&machine);
                break;
            case 5:
                printHistory(machine.history);
                pressEnter();
                break;
            case 6:
                flag = 1;
                printf("\nProgram terminated\n");
                freeMalloc(machine.history);
                machine.history = NULL;
                break;
        }
    }
    
    return 0;
}


//MARK: - Start machine

void generateItems(machine_t *machine){
    int i = 0;
    char itemNames[ITEM_SIZE][10] = {"Lays", "Cheetos", "Doritos", "m&m", "Skittles", "Snickers", "Mars", "Oreo", "Twix", "Hershey\'s"};
    srand((uint)time(NULL));
    for (i = 0; i < sizeof(*machine -> items) * ITEM_SIZE / sizeof(item_t); i++) {
        strcpy(machine -> items[i].name, itemNames[i]);
        machine -> items[i].number = i + 1;
        machine -> items[i].price = (rand() % 11 + 10) * 100; // 1000 ~ 2000
        machine -> items[i].amount = ITEM_SIZE; // Initial item amount
    }
    return;
}

void startMachine(machine_t *machine, user_t *user, int *selectMenu){
    system("clear");
    printItems(machine);
    printUserBalance(user);
    
    printf("\n%-30s%-30s\n%-30s%-30s\n%-30s%-30s\n", "1. Add item to your list", "2. Payment", "3. Add cash", "4. Restock", "5. History","6. Finish program");
    printf("\nWhat do you want to do ? ");
    scanf("%d", selectMenu);
    return;
}


//MARK: - Choose item

int chooseItem(){
    int itemNumber = 0;
    //User enter item number
    printf("\nEnter item number (enter \'0\' to go back) : ");
    
    scanf("%d", &itemNumber);
    
    return itemNumber;
}

int findIndex(item_t *item, user_t *user){
    int index = -1; // If there's the same item in both user shopping list and machine, it'll return item's index.
    int last = listLength(user);
    
    for (int i = 0; i < last; i++) {
        if (user -> list.items[i].number == item -> number){
            index = i;
            break;
        }
    }
    return index;
}


//MARK: - Item and cash increase, decrease


void addItem(user_t *user, item_t *item, int index){
    strcpy(user -> list.items[index].name, item -> name);
    user -> list.items[index].number = item -> number;
    return;
}

void addCash(user_t *user){
    int add = 0;
    
    printf("\nHow much do you want to add ? ");
    
    scanf("%d", &add);
    user -> cash += add;
    printf("\n[%d] successfully added.\n", add);
    pressEnter();
    system("clear");
    return;
}

void supplyItems(machine_t *machine){
    int flag = 0;
    int itemNumber = 0;
    int amount = 0;
    item_t *selectedItem = {0,};
    item_t itemTemp = {0,}; // For history
    
    while (!flag) {
        system("clear");
        printItems(machine);
        printMachineBalance(machine);
        printf("\n*Wholesale price is 90%% of retail price.\n");
        printf("\nEnter item number to supply (enter \'0\' to go back) : ");
        scanf("%d", &itemNumber);
        switch(itemNumber){
            case 0 :
                system("clear");
                flag = 1;
                break;
            default:
                if (itemNumber > 0 && itemNumber <= 10) {
                    selectedItem = &machine -> items[itemNumber - 1];
                    printf("\nHow many [%s] do you want to refill ? : ", selectedItem -> name);
                    scanf("%d", &amount);
                    if (amount > 0){
                        if (machine -> cash >= (selectedItem -> price * amount) * WHOLESALE) {
                            machine -> cash -= (selectedItem -> price * amount) * WHOLESALE;
                            selectedItem -> amount += amount;
                            //Save into itemTemp first due to the total price of selectedItem user bought
                            strcpy(itemTemp.name, selectedItem -> name);
                            itemTemp.number = selectedItem -> number;
                            itemTemp.amount = amount;
                            itemTemp.price = (selectedItem -> price * itemTemp.amount) * WHOLESALE;
                            
                            addHistory(machine -> history, &itemTemp, BUY);
                            printf("\n[%s] successfully supplied.\n", selectedItem -> name);
                        } else {
                            printf("\nNot enough money\n");
                        }
                    }
                } else {
                    printf("\nInvalid item number. Please enter again.\n");
                }
                pressEnter();
        }
    }
    return;
}


//MARK: - Item to list

void itemToList(machine_t *machine, user_t *user){
    int itemNumber = 0;
    int amount = 0;
    int available = 0;
    int last = 0;
    int index = 0;
    int flag = 0;
    item_t *selectedItem = {0,};
    
    while (!flag) {
        system("clear");
        printItems(machine);
        printUserBalance(user);
        itemNumber = chooseItem();
        if (itemNumber >= 0 && itemNumber < 11) {
            
            switch (itemNumber) {
                case 0:
                    system("clear");
                    flag = 1;
                    break;
                default:
                    selectedItem = &machine -> items[itemNumber - 1];
                    if (selectedItem -> amount > 0) {
                        
                        printf("\nHow many [%s] do you want to buy ? ", selectedItem -> name);
                        scanf("%d", &amount);
                        //Check machine has enough amount of selectedItem
                        available = selectedItem -> amount - amount >= 0 ? 1 : 0;
                        
                        if (available == 1) {
                            //Find user shopping list length
                            last = listLength(user);
                            //Check if selectedItem already in shopping list
                            index = findIndex(selectedItem, user);
                            
                            if (index >= 0){
                                //If yes, add amount to the item at the index
                                addItem(user, selectedItem, index);
                                user -> list.items[index].amount += amount;
                                user -> list.items[index].price = selectedItem -> price * amount;
                            } else {
                                //If no, add item to a new index
                                addItem(user, selectedItem, last);
                                user -> list.items[last].amount += amount;
                                user -> list.items[last].price = selectedItem -> price * amount;
                            }
                            //Reduce amount in machine
                            selectedItem -> amount -= amount;
                            
                            user -> list.total = calTotal(user);
                            
                            printf("\nItem(s) added to you shopping list.\n\n");
                            printList(user);
                            pressEnter();
                        } else {
                            printf("\nNot enough [%s]\n", selectedItem -> name);
                            pressEnter();
                        }
                    } else {
                        printf("\n[%s] has been sold out\n", selectedItem -> name);
                        pressEnter();
                    }
                    break;
            }
        } else {
            printf("\nInvalid item number.\n");
            pressEnter();
        }
    }
}


//MARK: - Edit list

void deleteFromList(user_t *user, int index){
    
    strcpy(user -> list.items[index].name, "");
    user -> list.items[index].amount = 0;
    user -> list.items[index].price = 0;
    user -> list.items[index].number = 0;
    
    return;
}

void editList(machine_t *machine, user_t *user){
    char name[10] = "";
    int amount = 0;
    int i = 0;
    int j = 0;
    int last = listLength(user);
    
    printf("\nEnter item name to edit : ");
    scanf("%s", name);
    // Find item in user shopping list
    for (i = 0; i < last; i++) {
        if (!strcmp(user -> list.items[i].name, name)){
            // Find item in machine
            for (j = 0; j < ITEM_SIZE; j++) {
                if (!strcmp(machine -> items[j].name, name)) {
                    
                    printf("\nChange current amount of [%s] to : ", user -> list.items[i].name);
                    scanf("%d", &amount);
                    if (amount > 0) {
                        if (amount <= machine -> items[j].amount + user -> list.items[i].amount) {
                            machine -> items[j].amount += user ->  list.items[i].amount;
                            user -> list.items[i].amount = 0;
                            user -> list.items[i].amount += amount;
                            machine -> items[j].amount -= amount;
                            user -> list.items[i].price = machine -> items[j].price * amount;
                            printf("\nList edited.\n");
                        } else {
                            printf("\nNot enough [%s] in the vending machine.", machine -> items[j].name);
                        }
                    } else {
                        //If amount <= 0, return all amount(selected item) in shopping list to machine
                        machine -> items[j].amount += user -> list.items[i].amount;
                        deleteFromList(user, i);
                        arrangeList(user, i, last);
                        printf("\nList edited.\n");
                    }
                    break;
                }
            }
            break;
        }
    }
    user -> list.total = calTotal(user);
    
    return;
}

void clearList(machine_t *machine, user_t *user){
    char confirm = 0;
    int i = 0;
    int j = 0;
    int last = listLength(user);
    
    printf("\nDo you want to clear your shopping list ? (y/n) : ");
    getchar();
    scanf("%c", &confirm);
    switch(confirm){
        case 'y':
            for (i = 0; i < last; i++) {
                for (j = 0; j < ITEM_SIZE; j++) {
                    //Compare and find common item and return the amount of items in shopping list to machine
                    if (user -> list.items[i].number == machine -> items[j].number) {
                        machine -> items[j].amount += user -> list.items[i].amount;
                    }
                }
                //Delete item info in shopping list
                deleteFromList(user, i);
            }
            
            user -> list.total = calTotal(user);
            
            printf("\nList cleared.\n");
            break;
        case 'n':
            break;
        default:
            printf("\nFailed clearing\n");
            break;
    }
    
    return;
}


//MARK: - Payment

void pay(user_t *user, machine_t *machine){
    int last = listLength(user);
    
    if (user -> cash >= user -> list.total) {
        user -> cash -= user -> list.total;
        machine -> cash += user -> list.total;
        for (int i = 0; i < last; i++) {
            //Add history when user buy items
            addHistory(machine -> history, &user -> list.items[i], SELL);
            deleteFromList(user, i);
        }
        user -> list.total = calTotal(user);
        printf("\nSuccessfully purchased.\n");
    } else {
        printf("\nNot enough money.\n");
    }
    
}

void payment(machine_t *machine, user_t *user){
    int menu = 0;
    int flag = 0;
    
    while(!flag){
        system("clear");
        printList(user);
        printf("%-12s%12s%12s%12d\n", "Balance", "", "", user -> cash);
        printf("------------------------------------------------\n");
        printf("\n%-30s%-30s\n%-30s%-30s\n", "1. Purchase items", "2. Edit list", "3. Clear List", "4. Back");
        printf("\nSelect menu : ");
        scanf("%d", &menu);
        
        switch (menu) {
            case 1:
                if (user -> list.total != 0){
                    pay(user, machine);
                } else {
                    printf("\nShopping list is empty.\n");
                }
                pressEnter();
                break;
            case 2:
                if (user -> list.total != 0) {
                    editList(machine, user);
                } else {
                    printf("\nShopping list is empty.\n");
                }
                pressEnter();
                break;
            case 3:
                if (user -> list.total != 0) {
                    clearList(machine, user);
                } else {
                    printf("\nShopping list is empty.\n");
                }
                pressEnter();
                break;
            case 4:
                system("clear");
                flag = 1;
                break;
        }
    }
    return;
}


//MARK: - History

void addHistory(history_t *history, item_t *item, historyType_t type){
    time_t now = time(NULL);
    t = localtime(&now);
    history_t *current = history;//head
    
    while (current -> next != NULL) {
        current = current -> next;
    }
    
    history_t *newHistory = (history_t*)malloc(sizeof(history_t));
    memcpy(&newHistory -> item, item, sizeof(item_t));
    strcpy(newHistory -> time, asctime(t));
    newHistory -> type = type;
    newHistory -> next = NULL;
    //Add new history next to the last history
    current -> next = newHistory;
    
    return;
}

void freeMalloc(history_t *history){
    history_t *current = history;//head
    history_t *temp;
    
    while (current != NULL) {
        temp = current -> next; // Because current from head will be deleted
        free(current);
        current = temp;
    }
    return;
}


//MARK: - Print

void printItems(machine_t *machine){
    printf("------------------------------------------------\n");
    printf("                  Grab A Bite !                 \n");
    printf("------------------------------------------------\n");
    printf("%-12s%12s%12s%12s\n", "Item no.","Item", "Amount", "Price");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < ITEM_SIZE; i ++) {
        if (machine -> items[i].amount != 0) {
            printf("%-12d%12s%12d%12d\n", machine -> items[i].number, machine -> items[i].name, machine -> items[i].price, machine -> items[i].amount);
            printf("------------------------------------------------\n");
            
        } else if (machine -> items[i].amount <= 0){
            printf("%-12d%12s%12d%12s\n", machine -> items[i].number, machine -> items[i].name, machine -> items[i].price, "*Sold out");
            printf("------------------------------------------------\n");
            
        }
    }
    return;
}

void printUserBalance(user_t *user){
    printf("%-12s%12s%12s%12d\n", "Balance", "User", "", user -> cash);
    printf("------------------------------------------------\n");
    return;
}

void printMachineBalance(machine_t *machine){
    printf("%-12s%12s%12s%12d\n", "Balance", "Machine", "", machine -> cash);
    printf("------------------------------------------------\n");
    return;
}

void printList(user_t *user){
    int last = listLength(user);
    
    printf("------------------------------------------------\n");
    printf("                 Order summary                  \n");
    printf("------------------------------------------------\n");
    printf("%-12s%12s%12s%12s\n", "Item no.","Item", "Amount", "Price");
    printf("------------------------------------------------\n");
    for (int i = 0; i < last; i++) {
        printf("%-12d%12s%12d%12d\n", user -> list.items[i].number, user -> list.items[i].name, user -> list.items[i].amount, user -> list.items[i].price);
        printf("------------------------------------------------\n");
    }
    printf("%-12s%12s%12s%12d\n", "Total", "", "", user -> list.total);
    printf("------------------------------------------------\n");
    return;
}

void printHistory(history_t *history){
    int saleTotal = 0;
    int buyTotal = 0;
    
    system("clear");
    history_t *current = history -> next;
    printf("\n------------------------------------------------\n");
    
    printf("%-12s%12s%12s%12s", "Item no.","Item", "Amount", "Price");
    
    printf("\n------------------------------------------------\n");
    
    while (current != NULL) {
        printf("%-30s\n", current -> time);
        printf("%48s\n", current -> type == 0 ? "SELL" : "BUY");
        printf("%-12d%12s%12d%12d\n", current -> item.number, current -> item.name, current -> item.amount, current -> item.price);
        printf("------------------------------------------------\n");
        if (current -> type == 0){
            saleTotal += current -> item.price;
        } else {
            buyTotal += current -> item.price;
        }
        current = current -> next;
    }
    
    printf("%-12s%12s%12s%12d\n", "", "", "SELL", saleTotal);
    printf("%-12s%12s%12s%12d\n", "", "", "BUY", buyTotal);
    printf("%-12s%12s%12s%12d", "", "", "PROFIT", saleTotal - buyTotal);
    printf("\n------------------------------------------------\n");
    
    return;
}


//MARK: - Utility

void pressEnter(){
    printf("\nPress enter to continue...\n");
    getchar();
    getchar();
    return;
}

int calTotal(user_t *user){
    int total = 0;
    int last = listLength(user);
    
    for (int i = 0; i < last; i++) {
        total += user -> list.items[i].price;
    }
    return total;
}

//Move deleted item info in shopping list to the end of list
void arrangeList(user_t *user, int index, int last){
    item_t itemTemp = {0,};
    for (int i = index; i < last; i++) {
        memcpy(&itemTemp, &user -> list.items[i], sizeof(item_t));
        memcpy(&user -> list.items[i], &user -> list.items[i + 1], sizeof(item_t));
        memcpy(&user -> list.items[i + 1], &itemTemp, sizeof(item_t));
    }
    return;
}

int listLength(user_t *user){
    int last = 0;
    while (user -> list.items[last].amount != 0) {
        last++;
    }
    return last;
}
