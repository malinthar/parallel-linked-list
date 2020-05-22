/* serial program for linked list operations */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_RANDOM_VALUE 65535 /*2^16-1: threshold for random value*/

/*Global variables */
int n;
int m ;
int m_insert, m_delete, m_member;
float m_insert_frac, m_delete_frac , m_member_frac;
int *operations;

struct list_node_s {
    int data;
    struct list_node_s *next;
};

int thread_count;
struct list_node_s *head = NULL;
pthread_rwlock_t list_rw_lock;

/* likedlist operations*/
int Insert(int value, struct list_node_s **head_pp);
int Member(int value, struct list_node_s **head_pp);
int Delete(int value, struct list_node_s **head_pp);

/*helpers*/
double CalculateTime(struct timeval time_begin, struct timeval time_end); /*Calculate time to exectue */
void Input(int argc,char *argv[]); /*Validate input and populate values*/
void BuildOperationsArray();
void shuffle(int *array, size_t n);

/*Thread execution*/
void *Operate(void *rank);

int main(int argc, char *argv[]) {
    struct timeval t_begin, t_end;
    pthread_t *thread_handles;
    Input(argc,argv);

    m_insert = (int) (m_insert_frac * m);
    m_delete = (int) (m_delete_frac * m);
    m_member = (int) (m_member_frac * m);

    /*create randomized operations array*/
    BuildOperationsArray();
    
    thread_handles = malloc(sizeof(pthread_t)*thread_count);
    pthread_rwlock_init(&list_rw_lock,NULL);
    
    /*Seed the linkedlist with values */
    int i = 0;
    while( i < n) {
        if (Insert(rand() % MAX_RANDOM_VALUE,&head) == 1)
            i++;
    }

    /*start time */
    gettimeofday(&t_begin, NULL);
    for(int thread= 0; thread< thread_count;thread++) {
        pthread_create(&thread_handles[thread], NULL, Operate, (void*)thread);
    } 
    for(int thread=0; thread < thread_count;thread++) {
        pthread_join(thread_handles[thread],NULL);
    }
    gettimeofday(&t_end, NULL); /*end time */

    pthread_rwlock_destroy(&list_rw_lock);
    free(thread_handles);
    printf("Time taken for execution : %.6f secs\n", CalculateTime(t_begin, t_end));
    return 0;
}

void *Operate(void *rank) {   

    int my_rank = (long) rank;

    /* Mapping threads to operations*/
    int m_local;
    int local_m_start;
    int local_m_end;
    if(m % thread_count == 0 || m % thread_count <= my_rank) {
            m_local = m /thread_count;
            local_m_start = m_local * my_rank + m % thread_count;
            local_m_end = local_m_start + m_local;
    }
    else {
        m_local = m /thread_count + 1;
        local_m_start = m_local * my_rank;
        local_m_end = local_m_start + m_local;
    }

    for(int i=local_m_start;i<local_m_end;i++) {
        // Variable to randomly generate values for operations
        int random_value = rand() % MAX_RANDOM_VALUE;

        // Variable to randomly select one of the three operations
        int operation = operations[i];

        // Member operation
        if (operation == 0) {
            pthread_rwlock_rdlock(&list_rw_lock);
            Member(random_value, &head);
            pthread_rwlock_unlock(&list_rw_lock);
        }

        // Insert Operation
        if (operation == 1) {
            pthread_rwlock_wrlock(&list_rw_lock);
            Insert(random_value, &head);
            pthread_rwlock_unlock(&list_rw_lock);
        }

        // Delete Operation
        if(operation == 2) {
            pthread_rwlock_wrlock(&list_rw_lock);
            Delete(random_value, &head);
            pthread_rwlock_unlock(&list_rw_lock);
        }
    }
    return NULL;
}

int Insert(int value, struct list_node_s **head_pp) {
   struct list_node_s *curr_p = *head_pp;
   struct list_node_s *pred_p = NULL;
   struct list_node_s *temp_p;

   while (curr_p != NULL && curr_p->data < value) {
       pred_p = curr_p;
       curr_p = curr_p->next;
   }
   if(curr_p == NULL || curr_p->data > value) {
       temp_p = malloc(sizeof(struct list_node_s));
       temp_p->data = value;
       temp_p->next = curr_p;
       if(pred_p == NULL)
           head_pp = &temp_p;
       else
           pred_p->next = temp_p;
       return 1;
   }
   else {
       return 0; /*The list already have the given value*/
   }
}

int Member(int value, struct list_node_s **head_pp) {
    struct list_node_s *curr_p = *head_pp;
    while (curr_p != NULL && curr_p->data < value)
        curr_p = curr_p -> next;
    if (curr_p == NULL || curr_p->data > value)
        return 0;
    else 
        return 1;
} /*Member*/

int Delete(int value, struct list_node_s **head_pp) {
    struct list_node_s *curr_p = *head_pp;
    struct list_node_s *pred_p = NULL;
    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }
    if(curr_p != NULL && curr_p -> data == value) {
        if(pred_p == NULL) { /*Delete the first node of the list*/
            *head_pp = curr_p-> next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    } else { /*Value not in the list */
        return 0;
    }
}

void Input(int argc, char *argv[]) {
   
    if(argc!=7) {
        printf("Invalid number of arguments!");
        exit(0);
    }
    n = (int) strtol(argv[1], (char **) NULL, 10);
    m = (int) strtol(argv[2], (char **) NULL, 10);
    thread_count = (int) strtol(argv[3], (char **) NULL, 10);
    m_member_frac = (float) atof(argv[4]);
    m_insert_frac = (float) atof(argv[5]);
    m_delete_frac = (float) atof(argv[6]); 
    if (n <= 0) {
        printf("Invalid n !");
        exit(0);
    }
    if (m <= 0) {
        printf("invalid m !");
        exit(0);
    }
    if (m_member_frac + m_insert_frac + m_delete_frac != 1.0) {
        printf("Invalid combination of fractions!");
        exit(0);
    }
} /*Input*/

void BuildOperationsArray() {
    operations = malloc(sizeof(int)*m);
    int total = 0;
    int insert = 0;
    int member = 0;
    int delete = 0;
    int i = 0;
    while(total < m) {
        int rand_op = rand() % 3;
        if(rand_op == 0 && member < m_member) {
            operations[i] = 0;
            member++;
            i++;
        } 
        if(rand_op == 1 && insert < m_insert) {
            operations[i] = 1;
            insert++;
            i++;
        }
        if(rand_op == 2 && delete < m_delete) {
            operations[i] = 2;
            delete++;
            i++;
        }
        total = insert + member + delete;
    }
    shuffle(operations,m);
}

void shuffle(int *array, size_t nn)
{
    if (nn > 1) 
    {
        size_t i;
        for (i = 0; i < nn - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (nn - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}


double CalculateTime(struct timeval time_begin, struct timeval time_end) {
    return (double) (time_end.tv_usec - time_begin.tv_usec) / 1000000 + (double) (time_end.tv_sec - time_begin.tv_sec);
} /*CalculateTime*/
