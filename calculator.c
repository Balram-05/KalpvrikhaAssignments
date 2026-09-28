#include<stdio.h>

int main(){
    char exp[100];
    int n=0;
    char oper[100];
    int num[100];
    int i=0, o=0;
    int number = 0;
    int hasNumber = 0;

    printf("Enter expression");
    scanf("%[^\n], exp");

    while(exp[i] != '\0'){
        if(exp[i]==" "){
            i++;
            continue;
        }

        if(exp[i]>'0' && exp[i]<'9'){
            number = 0;

            while(exp[i]>'0' && exp[i]<'9'){
                number = number*10+(exp[i]-'0');
                i++;
            }

            num[n]=number;
            n++;
            hasNumber = 1;
        }

        else if(exp[i]=='+' || exp[i]=='-' || exp[i]=='*' || exp[i]=='/'){
            if(hasNumber == 0){
                printf("Error: Invalid expression");
                return 0;
            }

            oper[0]=exp[i];
            o++;
            hasNumber = 0;
            i++;
        }
        else{
            printf("Error: Invalid Expression");
            return 0;
        }
    }

    if(hasNumber == 0 || n==0){
        printf("Error: Invalid Expression");
            return 0;
    }

    i=0;

    while(i<o){
        if(oper[i] == '*' || oper[i]=='/'){
            if(oper[i] == '/' && num[i+1]==0){
                printf("Error: Division by Zero");
                return 0;
            }

            if(oper[i]=='*'){
                num[i]=num[i]*num[i+1];
            }else{
                num[i]=num[i]/num[i+1];
            }

            int j;
            for(j=i+1;j<n-1;j++){
                num[j]=num[j+1];
            }

            for(j=i;j<o-1;j++){
                oper[j]=oper[j+1];
            }

            o--;
            n--;
        } else{
            i++;
        }
    }

    int result = num[0];
    for(i=0;i<o;i++){
        if(oper[i]=='+'){
            result = result+num[i+1];
        } else{
            result = result-num[i+1];
        }
    }

    print("%d", result);
    return 0;
}