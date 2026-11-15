
#include <stdio.h>
#include <string.h>

#define MAX 100

struct Employee {
    int id;
    char name[50];
    char designation[50];
    int managerId;
    struct Employee *manager;
};

struct Employee employees[MAX];
int employeeCount = 0;

int findEmployee(int id){
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].id == id){
            return i;
        }
    }
    return -1;
}

int findCEO(){
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].managerId == -1){
            return i;
        }
    }
    return -1;
}

int wouldCreateCycle(int employeeId, int managerId){
    int currentId = managerId;

    while (currentId != -1) {
        if (currentId == employeeId)
            return 1;

        int index = findEmployee(currentId);

        if (index == -1)
            return 1;

        currentId = employees[index].managerId;
    }

    return 0;
}

void rebuildManagerPointers()
{
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].managerId == -1) {
            employees[i].manager = NULL;
        } else {
            int index = findEmployee(employees[i].managerId);

            if (index != -1)
                employees[i].manager = &employees[index];
            else
                employees[i].manager = NULL;
        }
    }
}

int countDirectReports(struct Employee *employee)
{
    int count = 0;

    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].manager == employee)
            count++;
    }

    return count;
}

int countUnder(struct Employee *employee)
{
    if (employee == NULL)
        return 0;

    int total = 0;

    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].manager == employee)
            total += 1 + countUnder(&employees[i]);
    }

    return total;
}

void displayHierarchy(struct Employee *employee, int level)
{
    if (employee == NULL)
        return;

    for (int i = 0; i < level; i++)
        printf("    ");

    printf("%d - %s - %s\n",
           employee->id,
           employee->name,
           employee->designation);

    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].manager == employee)
            displayHierarchy(&employees[i], level + 1);
    }
}

void addEmployee()
{
    if (employeeCount >= MAX) {
        printf("Employee limit reached.\n");
        return;
    }

    int id, managerId;

    printf("Enter Employee ID: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }

    if (id <= 0 || findEmployee(id) != -1) {
        printf("Invalid or duplicate Employee ID.\n");
        return;
    }

    printf("Enter Reporting Manager ID (-1 for CEO): ");
    if (scanf("%d", &managerId) != 1) {
        printf("Invalid manager ID.\n");
        return;
    }

    if (managerId == -1) {
        if (findCEO() != -1) {
            printf("A CEO already exists. Only one CEO is allowed.\n");
            return;
        }
    } else {
        if (findEmployee(managerId) == -1) {
            printf("Reporting manager does not exist.\n");
            return;
        }

        if (wouldCreateCycle(id, managerId)) {
            printf("Invalid reporting relationship: cycle detected.\n");
            return;
        }
    }

    int index = employeeCount;

    employees[index].id = id;
    employees[index].managerId = managerId;
    employees[index].manager = NULL;

    printf("Enter Name: ");
    if (scanf(" %49[^\n]", employees[index].name) != 1) {
        printf("Invalid name.\n");
        return;
    }

    printf("Enter Designation: ");
    if (scanf(" %49[^\n]", employees[index].designation) != 1) {
        printf("Invalid designation.\n");
        return;
    }

    employeeCount++;
    rebuildManagerPointers();

    printf("Employee added successfully.\n");
}

void removeEmployee()
{
    int id;

    printf("Enter Employee ID to remove: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }

    int index = findEmployee(id);

    if (index == -1) {
        printf("Employee not found.\n");
        return;
    }

    if (employees[index].managerId == -1) {
        printf("CEO cannot be removed.\n");
        return;
    }

    int superiorId = employees[index].managerId;
    int siblingIndex = -1;
    int minimumReports = MAX + 1;

    for (int i = 0; i < employeeCount; i++) {
        if (i != index &&
            employees[i].managerId == superiorId) {

            int reports = countDirectReports(&employees[i]);

            if (reports < minimumReports) {
                minimumReports = reports;
                siblingIndex = i;
            }
        }
    }

    int destinationId;

    if (siblingIndex != -1)
        destinationId = employees[siblingIndex].id;
    else
        destinationId = superiorId;

    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].managerId == id)
            employees[i].managerId = destinationId;
    }

    for (int i = index; i < employeeCount - 1; i++)
        employees[i] = employees[i + 1];

    employeeCount--;
    rebuildManagerPointers();

    printf("Employee removed successfully.\n");
    printf("Direct reports transferred to employee ID %d.\n",
           destinationId);
}

void displayEmployeeHierarchy()
{
    int id;

    printf("Enter Employee ID: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }

    int index = findEmployee(id);

    if (index == -1) {
        printf("Employee not found.\n");
        return;
    }

    printf("\n===== EMPLOYEE HIERARCHY =====\n");

    displayHierarchy(&employees[index], 0);

    printf("Total Employees Under %d: %d\n",
           id, countUnder(&employees[index]));
}

void displayCompleteHierarchy()
{
    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    int ceoIndex = findCEO();

    if (ceoIndex == -1) {
        printf("No CEO found.\n");
        return;
    }

    displayHierarchy(&employees[ceoIndex], 0);
    printf("Total Employees: %d\n", employeeCount);
}

int main()
{
    int choice;

    do {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                addEmployee();
                break;

            case 2:
                removeEmployee();
                break;

            case 3:
                displayEmployeeHierarchy();
                break;

            case 4:
                displayCompleteHierarchy();
                break;

            case 5:
                printf("Exiting program.\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }

    } while (choice != 5);

    return 0;
}
