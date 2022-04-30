#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/stdc++.h>

using namespace std;


struct ProcessData {
    double **A;
    double **B;
    double **C;
    int veclen, i, j, t;
};

void* mult(void *arg){
    ProcessData* pd = *(ProcessData**)arg;
    double *A = *(pd->A); double *B = *(pd->B); double *C = *(pd->C);
    int i = pd->i; int j = pd->j; int c2 = pd->t; int veclen = pd->veclen;
    C[i*c2 + j] = 0;
    for(int k=0;k<veclen;k++){
        C[i*c2+j] += A[i*veclen+k]*B[k*c2+j];
    }
}

int main()
{
    srand(time(0));
    int r1, c1, r2, c2;
    cout << "Enter the rows and columns of the first matrix: ";
    cin >> r1 >> c1;
    cout << "Enter the rows and columns of the second matrix: ";
    cin >> r2 >> c2;
    key_t key = ftok("shmfile",65);
    
    int size = r1 * c1 + r2 * c2 + r1 * c2;
    int shmid = shmget(100,sizeof(double)*size,0777|IPC_CREAT);
    double *A=(double *) shmat(shmid, (void*)0,0);
    double *B = A + r1*c1;
    double *C = B + r2*c2;
    
    if(shmid < 0){
        cout<<"Error in creating shared memory\n";
        exit(1);
    }

    cout << "Enter the values of the first matrix: \n";
    for(int i=0;i<r1;i++){
        for(int j=0;j<c1;j++){
           cin >> A[i*c1+j];
            // A[i*c1+j] = 1;
        }   
    }

    cout << "\nEnter the values of the second matrix: \n";

    for(int i=0;i<r2;i++){
        for(int j=0;j<c2;j++){
           cin >> B[i*c2+j];
            // B[i*c2+j] = 1;
        }   
    }
    for(int i=0;i<r1;i++){
        for(int j=0;j<c2;j++){
            pid_t pid = fork();
            if(pid == 0){
                ProcessData* pd = (ProcessData *)malloc(sizeof(ProcessData));
                pd->A = &A; pd->B = &B; pd->C = &C;
                pd->veclen = c1; pd->i = i; pd->j = j; pd->t = c2;
                mult(&pd);
                exit(0);
            }
            else if(pid < 0){
                shmdt((void *)A);
                shmctl(shmid,IPC_RMID,NULL);
                cout<<"Forking Error\n";
                exit(1);
            }
        }   
    }
    for(int i=0;i<r1*c2;i++){
        wait(NULL);
    }
    cout << fixed << setprecision(6);
    cout << "\nFinal Matrix: \n";
    for(int i=0;i<r1;i++){
        for(int j=0;j<c2;j++){
            cout<<C[i*c2+j]<<" ";
        }   
        cout<<"\n";
    } 
    shmdt((void *)A);
    shmctl(shmid,IPC_RMID,NULL);
    return 0;
}