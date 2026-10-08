#include<stdio.h>

struct Student{
    int roll;
    char name[50];
    int marks1;
    int marks2;
    int marks3;
};

int calculateTotal(struct Student s){
    return s.marks1 + s.marks2 + s.marks3;
}

float calculateAverage(int total){
    return total/3.0;
}

char calculateGrade(float average){
    if(average >= 85) return 'A';
    else if(average >= 70) return 'B';
    else if(average >= 50) return 'C';
    else if(average >= 35) return 'D';
    else return 'F'; 
}

void printStars(char grade){
    int stars = 0;
    int i = 0;

    if(grade == 'A') stars = 5;
    else if(grade == 'B') stars = 4;
    else if(grade == 'C') stars = 3;
    else if(grade == 'D') stars = 2;

    for(i=0;i<stars;i++){
        printf("*");
    }
}

void printRollNumbers(int current, int n){
    if(current > n) return;

    printf("%d", current);
    if(current < n){
        printf(" ");
    }

    printRollNumbers(current+1, n);
}

int main(){
    int n;
    int i;
    int total;
    float average;
    char grade;

    printf("Enter number of students ");
    scanf("%d", &n);

    struct Student students[n];

    for(i=0;i<n;i++){
        scanf("%d %s %d %d %d", &students[i].roll, students[i].name, &students[i].marks1,
                                &students[i].marks2, &students[i].marks3);
    }

    printf("\n");

    for(i=0;i<n;i++){
        total = calculateTotal(students[i]);
        average = calculateAverage(total);
        grade = calculateGrade(average);

        printf("Roll: %d\n", students[i].roll);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);

        if(average < 35) continue;

        printf("Performance: ");
        printStars(grade);
        printf("\n");
    }

    printf("\n List of roll numbers ");
    printRollNumbers(1, n);
    printf("\n");

    return 0;
}