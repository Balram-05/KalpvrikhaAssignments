#include<stdio.h>

struct User{
    int id;
    char name[50];
    int age;
};

void createFile(){

    FILE *fp;
    fp = fopen("users.txt", "a");
    if(fp == NULL){
        printf("Error creating file. \n");
        return;
    }

    fclose(fp);
}

void createUser(){
    FILE *fp;
    struct User u;

    fp = fopen("users.txt", "a");

    if(fp == NULL){
        printf("Error opening file. \n");
        return;
    }

    printf("Enter ID:\n");
    scanf(" %d", &u.id);

    printf("Enter name \n");
    scanf(" %[^\n]", u.name);

    printf("Enter age: \n");
    scanf(" %d", &u.age);

    fprintf(fp, " %d|%s|%d\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added successfully. \n");
}

void readUsers(){
    FILE *fp;
    struct User u;

    fp = fopen("users.txt", "r");

    if(fp == NULL){
        printf("Error opening file. \n");
        return;
    }

    printf("\nID\tName\tAge\t\n");

    while(fscanf(fp, " %d|%[^|]|%d", &u.id, u.name, &u.age) == 3){
        printf(" %d\t%s\t\t%d\n", u.id, u.name, u.age);
    }
    
    fclose(fp);
}

void updateUser(){
    FILE *fp;
    struct User users[100];

    int count = 0;
    int id;
    int i;
    int found = 0;

    fp = fopen("users.txt", "r");

    if(fp == NULL){
        printf("error opening file. \n");
        return;
    }

    while(fscanf(fp, " %d|%[^|]|%d", &users[count].id, users[count].name, &users[count].age) == 3){
        count++;
    }

    fclose(fp);

    printf("Enter ID to update: ");
    scanf("%d", &id);

    for(i=0; i<count; i++){
        if(users[i].id == id){
            found = 1;

            printf("Enter new name ");
            scanf(" %[^\n]", users[i].name);
            printf("Enter new age ");
            scanf(" %d", &users[i].age);

            break;
        }
    }

    fp = fopen("users.txt", "w");
    if(fp == NULL){
        printf("Error opening file. \n");
        return;
    }

    for(i=0;i<count;i++){
        fprintf(fp, " %d|%s|%d\n", users[i].id, users[i].name, users[i].age);
    }

    fclose(fp);
    if(found){
        printf("user updated successfully \n");
    } else {
        printf("User not found. \n");
    }
}

void deleteUser(){
    FILE *fp;
    struct User users[100];

    int count = 0;
    int id; 
    int i;
    int found = 0;

    fp = fopen("users.txt", "r");

    if(fp == NULL){
        printf("Error opening file");
        return;
    }

    while(fscanf(fp, " %d|%[^|]|%d", &users[count].id, users[count].name, &users[count].age) == 3){
        count++;
    }

    fclose(fp);

    printf("Enter ID to delete ");
    scanf(" %d", &id);

    for(i=0;i<count;i++){
        if(users[i].id == id){
            found = 1;
            int j;

            for(j=i;j<count-1;j++){
                users[j] = users[j+1];
            }
            count--;
            break;
        }
    }

    fp = fopen("users.txt", "w");
    if(fp == NULL){
        printf("error opening file");
        return;
    }

    for(i=0;i<count;i++){
        fprintf(fp, " %d|%s|%d\n", users[i].id, users[i].name, users[i].age);
    }

    fclose(fp);
    if(found){
        printf("User deleted successfully \n");
    } else {
        printf("User not found \n");
    }
}

int main(){
    int choice;
    createFile();

    while(1){
        printf("\n USER MANAGEMENT SYSTEM \n");
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");

        printf("Enter your choice ");
        scanf(" %d", &choice);

        if(choice == 1){
            createUser();
        } else if (choice == 2){
            readUsers();
        } else if (choice == 3){
            updateUser();
        } else if (choice == 4){
            deleteUser();
        } else if(choice == 5){
            printf("Quit. \n");
            break;
        } else{
            printf("Invalid choice. \n");
        }
    }

    return 0;
}

