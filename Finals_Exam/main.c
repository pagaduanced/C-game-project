#include <stdio.h>
#include <stdlib.h>
#include <string.h>
    void addStudent(char name[10][15], int score[10], int *count);
    void deleteStudent(char name[10][15], int score[10], int *count);
    void studentgrade(char name[10][15], int score[10], int count, FILE* SummaryFile);
    void studentaverage(int score[], int count, FILE* SummaryFile);

int main()
{
    FILE* studentsFile;
    FILE* SummaryFile;
    char name[20][15];
    int score[10],i,j,count;
    char tempname[15];
    int tempscore;
    int choice;

        studentsFile = fopen("finals\\students.txt","r");
    if(studentsFile!=NULL){
        printf("The File Exist!");
        }
    else{
        printf("The File Does Not Exist!");
        return 1;
        }
    i=0;
    while(fscanf(studentsFile, "%s %d", name[i], &score[i]) == 2)
    {
        i++;
    }
    count=i;
    for (i=0; i < count-1; i++){
        for (j= i+1; j < count; j++){
            if (strcmp(name[i], name[j]) > 0)
            {
                strcpy(tempname, name[i]);
                strcpy(name[i], name[j]);
                strcpy(name[j], tempname);
                tempscore = score[i];
                score[i] = score[j];
                score[j] = tempscore;
            }
        }
    }
    SummaryFile = fopen("finals\\quiz_summary.txt", "w");
    if(!SummaryFile) {
        printf("Cannot create summary file!\n");
        return 1;
    }
        studentgrade(name, score, count, SummaryFile);
        studentaverage(score, count, SummaryFile);

        fclose(SummaryFile);
    do {
    printf("\nChoose an action: 1 = Add, 2 = Delete, 0 = Exit: ");
    scanf("%d", &choice);

    switch(choice) {
        case 1:
            addStudent(name, score, &count);
            printf("\nUpdated List of Students:\n");
            SummaryFile = fopen("finals\\quiz_summary.txt", "w");
        if (SummaryFile != NULL){
            studentgrade(name, score, count, SummaryFile);
            studentaverage(score, count, SummaryFile);
            fclose(SummaryFile);}
        else {
            printf("Cannot open summary file!\n");}
            break;
        case 2:
            deleteStudent(name, score, &count);
            printf("\nUpdated List of Students:\n");
            SummaryFile = fopen("finals\\quiz_summary.txt", "w");
        if (SummaryFile != NULL) {
            studentgrade(name, score, count, SummaryFile);
            studentaverage(score, count, SummaryFile);
            fclose(SummaryFile);}
        else {
            printf("Cannot open summary file!\n");}
            break;
        case 0:
            printf("Exiting program...\n");
            break;
        default:
            printf("Invalid choice! Try again.\n");
    }

} while(choice != 0);

    fclose(studentsFile);
    return 0;
}
void addStudent(char name[50][15], int score[10], int *count) {
    if (*count >= 50) {
        printf("Cannot add more students. Maximum reached.\n");
        return;
    }
    printf("Enter student name: ");
    scanf("%s", name[*count]);
    printf("Enter student score: ");
    scanf("%d", &score[*count]);
    (*count)++;
    printf("Student added successfully!\n");
    }

    void deleteStudent(char name[50][15], int score[10], int *count) {
    char delName[55];
    printf("Enter the name of the student to delete: ");
    scanf("%s", delName);

    int found = 0;
    for (int i = 0; i < *count; i++) {
        if (strcmp(name[i], delName) == 0) {
            found = 1;
            for (int j = i; j < *count - 1; j++) {
                strcpy(name[j], name[j+1]);
                score[j] = score[j+1];
            }
            (*count)--;
            printf("Student deleted successfully!\n");
            break;
        }
    }

    if (!found) {
        printf("Student not found.\n");
    }
}
void studentgrade(char name[50][15], int score[10], int count, FILE* SummaryFile) {
    char grade[3];
    char remark[10];

        printf("\n\n\t\t\t\tLIST OF STUDENTS\n");
        printf("\tStudent Name\t\tScore\t\tGrade\t\tRemarks\n");
         if (SummaryFile != NULL) {
        fprintf(SummaryFile, "\n\n\t\t\t\tLIST OF STUDENTS\n");
        fprintf(SummaryFile, "\tStudent Name\t\tScore\t\tGrade\t\tRemarks\n");}
    for(int i = 0; i < count; i++) {
       if(score[i] >= 90) strcpy(grade, "A");
        else if(score[i] >= 80) strcpy(grade, "B");
        else if(score[i] >= 75) strcpy(grade, "C");
        else if(score[i] >= 70) strcpy(grade, "D");
        else strcpy(grade, "F");

         if(score[i] >= 75)
            strcpy(remark, "Passed");
        else
            strcpy(remark, "Failed");
        printf("\t%s\t\t\t%d\t\t%s\t\t%s\n",name[i],score[i],grade,remark);
        fprintf(SummaryFile, "\t%s\t\t\t%d\t\t%s\t\t%s\n", name[i], score[i], grade, remark);
    }
}
 void studentaverage(int score[], int count, FILE* SummaryFile){
 int highest = score[0], lowest = score[0];
 float total = 0;
 int i;
 for(i=0;i<count;i++)
        {
        if(score[i]>highest){
            highest = score[i];
        }
        if(score[i]<lowest){
            lowest = score[i];
        }
        total += score[i];
        }
 float average = total / count;
        printf("\nHighest: %d\n", highest);
        fprintf(SummaryFile,"\nHighest: %d\n", highest);
        printf("Lowest: %d\n", lowest);
        fprintf(SummaryFile,"Lowest: %d\n", lowest);
        printf("Average: %.2f\n", average);
        fprintf(SummaryFile,"Average: %.2f\n", average);
 }

