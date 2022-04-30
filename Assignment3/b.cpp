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
#include <pthread.h>
#include <chrono>

using namespace std;

#define MAX 550

struct job {
	int time;
	int process_id;
	int producer_num;
	int status;  // 0 -> producer create , 1 -> ongoing multiplication , -1-> Treated as NULL. 
	int matrix_id;
	long long matrix[MAX][MAX];
	int sets[8];  // -1 -> taken by some worker
	int fill[4];  
	int destination_matrix_id;
};

struct node {
	int job_created;
	int job_completed;
	int max_size;
	int cur_size;
	int index;
	job jq[50];
	pthread_mutex_t lock;
};

job createJob(int process_id, int producer_num) {
	job j;
	j.time = rand()%4 + 1;
	j.matrix_id = 1 + rand()%100000;
	j.process_id = process_id;
	j.producer_num = producer_num;
	j.status = 0;
	j.destination_matrix_id = 0;
	for(int i = 0; i < MAX; i++) {
		for(int k = 0; k < MAX; k++) {
			j.matrix[i][k] = -9 + rand()%19;
		}
	}
	for(int i = 0; i < 8; i++) {
		j.sets[i] = i;
	}
	for(int i = 0; i< 4; i++) {
		j.fill[i] = 0;
	}
	return j;
}

void insert(node *total, job *j) {
	(total->jq)[total->index] = *j;
	total->cur_size = total->cur_size + 1;
	total->index = total->index + 1;
	return;
}

string helper(int width, const string& str) {
    int len = str.length();
    if(width < len) { return str; }

    int diff = width - len;
    int pad1 = diff/2;
    int pad2 = diff - pad1;
    return string(pad1, ' ') + str + string(pad2, ' ');
}

void print_producer_table(job *j){
	cout << "==============================================\n";
	cout << "Job Process ID | "<<"Job Prod No. | "<<"Mat ID | "<<"Time\n";
	cout << helper(15, to_string((*j).process_id)) <<"|";
	cout << helper(14, to_string((*j).producer_num)) <<"|";
	cout << helper(8, to_string((*j).matrix_id)) <<"|";
	cout << helper(6, to_string((*j).time));
	cout << endl;
	cout << "==============================================\n\n";
}

void producer(node *total, int producer_num, int process_id, int max_jobs) {
	while(1) {
		sleep((int)rand()%4);
		if(total->job_created >= max_jobs) {
			break;
		}
		while(1) {
			pthread_mutex_lock(&total->lock);
			if(total->job_created>=max_jobs) {
				pthread_mutex_unlock(&total->lock);
				break;
			}
			job j = createJob(process_id,producer_num);
			if((total->cur_size)<(total->max_size)) {
				total->job_created++;
				
				insert(total,&j);
				print_producer_table(&j);
				
				pthread_mutex_unlock(&total->lock);
				break;
			}
			pthread_mutex_unlock(&total->lock);
		}
	}
}

void remove_two_matrices(node *total, int first, int second) {
	(total->jq)[first].status = -1;
	(total->jq)[second].status = -1;
	total->cur_size = total->cur_size - 2;
}

long long **multiply(job *j1, job *j2, int i, int j, int k) {
	long long **result = (long long **) malloc(sizeof(long long*) * MAX/2);
	for(int i = 0; i< MAX/2; i++) {
		result[i] = (long long *) malloc(sizeof(long long) * MAX/2);
		for(int j = 0; j < MAX/2; j++) {
			result[i][j] = 0;
		}
	}
	int **a = (int **) malloc(sizeof(int*) * MAX/2);
	for(int i = 0; i< MAX/2; i++) {
		a[i] = (int *) malloc(sizeof(int) * MAX/2);
	}
	int **b = (int **) malloc(sizeof(int*) * MAX/2);
	for(int i = 0; i< MAX/2; i++) {
		b[i] = (int *) malloc(sizeof(int) * MAX/2);
	}
	// int a[MAX/2][MAX/2], b[MAX/2][MAX/2];
	// j1.matrix[i][k]  j2.matrix[k][j]
	int row1 = 0, row2 = MAX/2, col1 = 0, col2 = MAX/2;
	if(i == 1) {
		row1 = MAX/2;
		row2 = MAX;
	}
	if(k == 1) {
		col1 = MAX/2;
		col2 = MAX;
	}
	for(; row1 < row2; row1++) {
		for(; col1 < col2; col1++) {
			a[(row1>= MAX/2)? row1-MAX/2: row1][(col1 >= MAX/2) ? col1-MAX/2: col1] = (*j1).matrix[row1][col1];
		}
	}

	row1 = 0, row2 = MAX/2, col1 = 0, col2 = MAX/2;
	if(k == 1) {
		row1 = MAX/2;
		row2 = MAX;
	}
	if(j == 1) {
		col1 = MAX/2;
		col2 = MAX;
	}
	for(; row1 < row2; row1++) {
		for(; col1 < col2; col1++) {
			b[(row1>= MAX/2)? row1-MAX/2: row1][(col1 >= MAX/2) ? col1-MAX/2: col1] = (*j2).matrix[row1][col1];
		}
	}
	for(int i = 0; i< MAX/2; i++) {
		for(int j = 0; j < MAX/2; j++) {
			for(int k = 0; k < MAX/2; k++) {
				result[i][j] += a[i][k]*b[k][j];
			}
		}
	}
	free(a);
	free(b);
	return result;
}

int get(node *total, int id) {
	for(int i = 0; i < 50; i++) {
		if((total->jq)[i].status == -1) {
			continue;
		}
		if((total->jq)[i].matrix_id == id) {
			return i;
		}
	}
	return -1;
}

void copy(node *total, int index, long long **result, int i, int j) {
	int row1 = 0, row2 = MAX/2, col1 = 0, col2 = MAX/2;
	if(i == 1) {
		row1 = MAX/2;
		row2 = MAX;
	}
	if(j == 1) {
		col1 = MAX/2;
		col2 = MAX;
	}
	for(; row1 < row2; row1++) {
		for(; col1 < col2; col1++) {
			((total->jq)[index].matrix)[row1][col1] = result[(row1>= MAX/2)? row1-MAX/2: row1][(col1 >= MAX/2) ? col1-MAX/2: col1];
		}
	}
}

void add(node *total, int index, long long **result, int i, int j) {
	int row1 = 0, row2 = MAX/2, col1 = 0, col2 = MAX/2;
	if(i == 1) {
		row1 = MAX/2;
		row2 = MAX;
	}
	if(j == 1) {
		col1 = MAX/2;
		col2 = MAX;
	}
	for(; row1 < row2; row1++) {
		for(; col1 < col2; col1++) {
			((total->jq)[index].matrix)[row1][col1] += result[(row1>= MAX/2)? row1-MAX/2: row1][(col1 >= MAX/2) ? col1-MAX/2: col1];
		}
	}
}

void print_comsumer_table(int producer_num, int p1, int p2, int id1, int id2, int to, string op){
	cout << "=============================================================================================\n";
	cout << "Worker No | "<<"Prod No. [Mat 1] | "<<"Prod No. [Mat 2] | "<<"Mat ID 1 | "<<"Mat ID 2 | "<<"Block No. | "<<"Operation"<<"\n";
	cout << helper(9, to_string(producer_num)) <<" |";
	cout << helper(17, to_string(p1)) <<" |";
	cout << helper(17, to_string(p2)) <<" |";
	cout << helper(9, to_string(id1)) <<" |";
	cout << helper(9, to_string(id2)) <<" |";
	cout << helper(10, to_string(to)) <<" |";
	cout << helper(9, op);
    cout << endl;
    cout << "=============================================================================================\n\n";
}

void print_read(int producer_num, int p1, int p2, int id1, int id2, int to) {
	
	print_comsumer_table(producer_num, p1, p2, id1, id2, to, "Reading");
}

void print_copy(int producer_num, int p1, int p2, int id1, int id2, int to) {
	print_comsumer_table(producer_num, p1, p2, id1, id2, to, "Copy");
}

void print_add(int producer_num, int p1, int p2, int id1, int id2, int to) {
	print_comsumer_table(producer_num, p1, p2, id1, id2, to, "Add");
}

void consumer(node *total, int producer_num, int process_id, int max_jobs) {
	
	while(1) {
		sleep(rand()%4);
		if(total->job_completed >= max_jobs-1) {
			break;
		}
		while(1) {
			pthread_mutex_lock(&total->lock);
			if((total->job_completed)>=max_jobs-1) {
				pthread_mutex_unlock(&total->lock);
				break;
			}
			if(total->cur_size > 1) {
				int first = -1, second = -1;
				for(int i = 0; i < 50; i++) {
					if((total->jq)[i].status != -1) {
						if(first == -1) {
							first = i;
						}
						else {
							second = i;
							break;
						}
					}
				}
				if(first == -1 || second == -1 || first == second) {
					pthread_mutex_unlock(&total->lock);
					break;
				}
				assert(first != second);
				job j1 = (total->jq)[first], j2 = (total->jq)[second];
				if(j1.status != 0 || j2.status != 0) {
					pthread_mutex_unlock(&total->lock);
					break;
				}
				for(int i = 0; i< 8; i++) {
					if(j1.sets[i] != -1) {
						(total->jq)[first].sets[i] = -1;
						(total->jq)[second].sets[i] = -1;
						if(i == 0) {
							if(total->cur_size >= total->max_size){
								(total->jq)[first].sets[i] = 0;
								(total->jq)[second].sets[i] = 0;
								break;
							}
							// insert job
							job j_new = createJob(process_id, producer_num);
							j_new.status = 1;
							(total->jq)[first].destination_matrix_id = j_new.matrix_id;
							(total->jq)[second].destination_matrix_id = j_new.matrix_id;
							
							int f = (i & 4), j = (i & 2), k = (i & 1);
							// multiply A[f][k]*B[k][j]
							print_read(producer_num, j1.producer_num, j2.producer_num, j1.matrix_id, j2.matrix_id, 0);
							long long **result = multiply(&j1, &j2, f, j, k);
							print_copy(producer_num, j1.producer_num, j2.producer_num, j1.matrix_id, j2.matrix_id, 0);
							for(int row = 0; row < MAX/2; row++) {
								for(int col = 0; col < MAX/2; col++) {
									j_new.matrix[row][col] = result[row][col];
								}
							}
							(total->jq)[first].fill[0] = 1;
							(total->jq)[second].fill[0] = 1;
							free(result);
							insert(total, &j_new);

						}
						else {
							int index = get(total, j1.destination_matrix_id);
							assert(index != -1);
							int f = (i & 4)>>2, j = (i & 2)>>1, k = (i & 1);
							int to = 2*f + j;
							// multiply A[f][k]*B[k][j]
							print_read(producer_num, j1.producer_num, j2.producer_num, j1.matrix_id, j2.matrix_id, to);
							long long **result = multiply(&j1, &j2, f, j, k);
							
							if((total->jq)[first].fill[to] == 0) {
								// copy
								copy(total, index, result, f, j);
								print_copy(producer_num, j1.producer_num, j2.producer_num, j1.matrix_id, j2.matrix_id, to);
							}
							else {
								// add
								add(total, index, result, f, j);
								print_add(producer_num, j1.producer_num, j2.producer_num, j1.matrix_id, j2.matrix_id, to);
							}
							(total->jq)[first].fill[to] += 1;
							(total->jq)[second].fill[to] += 1;
							free(result);
						}

						if(i == 7) {
							int index = get(total, j1.destination_matrix_id);
							(total->jq)[index].status = 0;
							remove_two_matrices(total, first, second);
							total->job_completed += 1;
						}
						break;
					}
				}
				pthread_mutex_unlock(&total->lock);
				break;
			}
			pthread_mutex_unlock(&total->lock);
			break;
		}
	}
}

int main() {
	auto begin = std::chrono::high_resolution_clock::now();
	srand(time(0));
	key_t key = ftok("shmfile", 69);
	int NP, NW, num_jobs;
	cout << "Enter the number of producers: ";
	cin >> NP;
	cout << "Enter the number of workers: ";
	cin >> NW;
	cout << "Enter the number of matrices to multiply: ";
	cin >> num_jobs;
	cout << "\n";
	int shmid = shmget(key,sizeof(node),0777|IPC_CREAT);
	if(shmid < 0) {
		cout << "Error in creating shared memory";
		exit(1);
	}


	node *total = (node *) shmat(shmid, 0, 0);
	total->job_created = 0;
	total->job_completed = 0;
	total->max_size = 8;
	total->cur_size = 0;
	total->index = 0;
	pthread_mutexattr_t lock_attr;
	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&total->lock, &lock_attr);

	for(int i = 0; i < NP; i++) {
		pid_t pid;
		pid = fork();
		if(pid == 0) {
			srand(time(NULL)^i*9 + 10);
			int process_id = getpid();
			producer(total, i, process_id, num_jobs);
			exit(0);
		}
		else if(pid < 0) {
			cout << "Error in creating producer process\n";
		}
	}

	for(int i = 0; i < NW; i++) {
		pid_t pid;
		pid = fork();
		if(pid == 0) {
			srand(time(NULL)^i*6 + 69);
			int process_id = getpid();
			consumer(total, i, process_id, num_jobs);
			exit(0);
		}
		else if(pid < 0) {
			cout << "Error in creating worker process\n";
		}
	}
	bool flag = 0;
	while(1) {
		sleep(rand()%4);
		pthread_mutex_lock(&total->lock);
		if(!flag && (total->job_created) >= 10) {
			auto end = std::chrono::high_resolution_clock::now();
			auto consumed = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
			cout<<"We have created (at least 10) and consumed "<<(total->job_created)<<" jobs, within "<< (float)consumed.count()/1000000 <<" seconds."<<endl;
			flag = 1;
		}
		if((total->job_created)>=num_jobs && (total->job_completed) >= num_jobs-1) {
			auto end = std::chrono::high_resolution_clock::now();
			auto consumed = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
			cout<<"We have created (all jobs) and consumed "<<num_jobs<<" jobs, within "<< (float)consumed.count()/1000000 <<" seconds."<<endl;
			pthread_mutex_unlock(&total->lock);
			break;
		}
		pthread_mutex_unlock(&total->lock);
	}

	long long trace = 0;
	cout << "\nSum of principal diagonal elements: ";
	for(int i = 0; i < MAX; i++) {
		trace += (total->jq)[total->index - 1].matrix[i][i];
	}
	cout << trace << endl;

	shmdt((void *)total);
    shmctl(shmid,IPC_RMID,NULL);
	return 0;
}
