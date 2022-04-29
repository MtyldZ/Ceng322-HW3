/*
 * UMUT YILDIZ 260201028
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// thread states
#define IDLE 0
#define WORKING 1
#define FINISHED -1
int thread_state = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

/* Struct for list nodes */
struct DataNode {
    int data;
    struct DataNode *next;
};

struct DataNode *dataList; // global data list

// search a specific value
// return 0 if success else 1
int searchData(int value) {
    struct DataNode *node = dataList;
    while (node != NULL && node->data <= value) { // while node exists and node.data smaller than search value
        if (node->data == value) { return 0; } // value found
        node = node->next;
    }
    return 1;
}

// insert a specific value
// return 0 if success else 1
int insertData(int value) {
    if (dataList == NULL) { // if there is no element
        dataList = (struct DataNode *) calloc(1, sizeof(struct DataNode));
        dataList->data = value;
        return 0;
    }

    struct DataNode *prevNode = dataList;
    // find the biggest, smaller than value node
    while (prevNode->data <= value) {
        if (prevNode->data == value) { return 1; } // there is exists failure
        if (prevNode->next == NULL) { break; }
        prevNode = prevNode->next;
    }

    // create new node
    struct DataNode *newNode = (struct DataNode *) calloc(1, sizeof(struct DataNode));
    newNode->data = value;
    newNode->next = NULL;

    if (prevNode->next == NULL) { // if prev node is the last node
        prevNode->next = newNode;
    } else { // if new node should be between nodes
        newNode->next = prevNode->next;
        prevNode->next = newNode;
        newNode->data = prevNode->data;
        prevNode->data = value;
    }
    return 0;
}

// delete a specific value
// return 0 if success else 1
int deleteData(int value) {
    struct DataNode *node = dataList;
    struct DataNode *prev = NULL;
    while (node != NULL && node->data <= value) {
        if (node->data == value) {
            if (prev == NULL) { // if first entry is the value
                struct DataNode *nextNode = dataList->next;
                free(dataList);
                dataList = nextNode;
            } else { // if value is between nodes
                prev->next = node->next;
                free(node);
            }
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return 1;
}

// print data list
void printData(struct DataNode *firstNode) {
    printf("Final Data List: ");
    struct DataNode *node = firstNode;
    while (node != NULL) {
        printf("%d ", node->data);
        node = node->next;
    }
    printf("\n");
};

/* Struct for task nodes */
struct TaskNode {
    int task_num;
    int task_type; // insert:0, delete:1, search:2
    int value;
    struct TaskNode *next;
};

// since it is a queue I hold first and last nodes for fast insert and remove
struct TaskNode *taskFirst;
struct TaskNode *taskLast;

// insert a task
void Task_enqueue(int task_num, int task_type, int value) {
    // create task
    struct TaskNode *newNode = (struct TaskNode *) calloc(1, sizeof(struct TaskNode));
    newNode->task_num = task_num;
    newNode->task_type = task_type;
    newNode->value = value;

    if (taskFirst == NULL) { // if queue is empty
        taskLast = newNode;
        taskFirst = newNode;
    } else { // attach to last node
        taskLast->next = newNode;
        taskLast = newNode;
    }
}

// remove first task
// return 0 if success else 1
int Task_dequeue(long my_rank, int *task_num_p, int *task_type_p, int *value_p) {
    if (taskFirst == NULL) { // no task to dequeue
        return 1;
    }
    // assign values of node
    *task_num_p = taskFirst->task_num;
    *task_type_p = taskFirst->task_type;
    *value_p = taskFirst->value;

    // remove the first node from task queue
    struct TaskNode *nextNode = taskFirst->next;
    free(taskFirst);
    taskFirst = nextNode;
    return 0;
}

// create many tasks
void Task_queue(int n) {
    thread_state = WORKING;
    srand(time(NULL)); // randomizer
    for (int i = 0; i < n; ++i) {
        int value = rand() % 500;
        int type = rand() % 3;

        // lock the task enqueue since there can be worker working on taskFirst
        pthread_mutex_lock(&mutex);
        Task_enqueue(i, type, value);
        pthread_mutex_unlock(&mutex);
        // allow one worker to continue
        pthread_cond_signal(&cond);
    }
}

// every worker threads job
void *worker(void *arg) {
    int *taskNum = malloc(sizeof(int));
    int *taskType = malloc(sizeof(int));
    int *taskValue = malloc(sizeof(int));

    int tID = (int) (long) arg;
    while (thread_state == IDLE) {} // idle wait

    while (thread_state != FINISHED) { // continue if not task are not finished
        // mutex lock on task dequeue
        pthread_mutex_lock(&mutex);
        while (Task_dequeue(0, taskNum, taskType, taskValue)) {
            if (thread_state == FINISHED) {
                // freeing dynamic allocated memory spaces
                free(taskNum);
                free(taskType);
                free(taskValue);
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            // if could not dequeue then cond lock
            pthread_cond_wait(&cond, &mutex);
        }

        // execute the task end print the result
        char buffer[64];
        switch (*taskType) {
            case 0:
                sprintf(buffer, "%d %s inserted", *taskValue, insertData(*taskValue) ? "cannot be" : "is");
                break;
            case 1:
                sprintf(buffer, "%d %s deleted", *taskValue, deleteData(*taskValue) ? "cannot be" : "is");
                break;
            case 2:
                sprintf(buffer, "%d %s found", *taskValue, searchData(*taskValue) ? "cannot be" : "is");
                break;
        }
        printf("Tread %d: Task %d: %s\n", tID, *taskNum, buffer);
        pthread_mutex_unlock(&mutex);
        // allow next worker to work
        pthread_cond_signal(&cond);
    }
    // freeing dynamic allocated memory spaces
    free(taskNum);
    free(taskType);
    free(taskValue);
    return NULL;
}

int main(int argc, char *argv[]) {
    // take arguments
    int threadAmount = (int) strtol(argv[1], NULL, 10);
    int taskAmount = (int) strtol(argv[2], NULL, 10);
    FILE *fileOut = fopen(argv[3], "a");

    // init locks
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    int size = threadAmount;
    pthread_t tIDs[size]; // thread id list

    // bulk thread create
    // all thread will stay idle until thread_state changed
    for (int i = 0; i < size; i++) {
        pthread_create(&tIDs[i], NULL, worker, (void *) (long) i);
    }

    clock_t begin = clock();
    thread_state = WORKING;
    Task_queue(taskAmount);
    pthread_cond_broadcast(&cond);

    while (taskFirst != NULL) {} // wait until every task is finished
    thread_state = FINISHED;
    pthread_cond_broadcast(&cond);

    // bulk thread join
    for (int i = 0; i < size; i++) {
        pthread_join(tIDs[i], NULL);
    }

    // print final data list
    printf("Main: ");
    printData(dataList);

    clock_t end = clock();
    // calculate time past
    double timeSpent = (double) (end - begin) / CLOCKS_PER_SEC;
    // print time results to fileOut (csv format)
    fprintf(fileOut, "%d,%d,%f\n", threadAmount, taskAmount, timeSpent);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    if (dataList != NULL) { free(dataList); }
    if (taskFirst != NULL) { free(taskFirst); }

    // close file
    fclose(fileOut);
}


