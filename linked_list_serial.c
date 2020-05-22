/* serial program for linked list operations */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define MAX_RANDOM_VALUE 65535 /* 2^16-1; threshold for value*/
/* Global variables */
int n; /* initial number of nodes in the linked list */
int m; /* number of random operations */
int m_insert, m_delete, m_member;
float m_insert_fraction, m_delete_fraction, m_member_fraction; /* fractions of insert, delete and member operations */
int *operations;

struct list_node_s {
    int data;
    struct list_node_s *next;
};

/* likedlist operations*/
int Insert(int value, struct list_node_s **head_pp);
int Member(int value, struct list_node_s **head_pp);
int Delete(int value, struct list_node_s **head_pp);
double CalculateTime(struct timeval time_begin, struct timeval time_end);
/*validate input and populate values */
void ValidateInput(int argc,char *argv[]);
void BuildOperationsArray();
void shuffle(int *array, size_t n);


int main(int argc, char *argv[]) {
    struct list_node_s *head = NULL;
    ValidateInput(argc,argv);
    struct timeval t_begin, t_end;

    m_insert = (int) (m * m_insert_fraction);
    m_member = (int) (m * m_member_fraction);
    m_delete = (int) (m * m_delete_fraction);
    
    BuildOperationsArray();
    /*seed the linkedlist with values */
    int i = 0;
    while( i < n) {
        if (Insert(rand() % MAX_RANDOM_VALUE,&head) == 1)
            i++;
    }
    /* operations on the linekd list */

    gettimeofday(&t_begin, NULL); /* start_time */
    for(int i=0;i<m;i++) {
        int operation = operations[i];
        int value = rand() % MAX_RANDOM_VALUE;
        if (operation == 0) {
            Member(value,&head);
        }
        if (operation == 1) {
            Insert(value,&head);
        }  
        if (operation == 2) {
            Delete(value,&head);
        }
    }
    gettimeofday(&t_end, NULL); /* end time */
    printf("Time taken for execution : %.6f secs\n", CalculateTime(t_begin, t_end));
    return 0;
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
       return 0; /* the list already have the given value */
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
        if(pred_p == NULL) {/* Delete the first node of the list*/
            *head_pp = curr_p-> next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    } else { /*value not in the list */
        return 0;
    }
}

void ValidateInput(int argc, char *argv[]) {
    if(argc!=6) {
        printf("Invalid number of arguments!");
        exit(0);
    }
    n = (int) strtol(argv[1], (char **) NULL, 10);
    m = (int) strtol(argv[2], (char **) NULL, 10);
    m_member_fraction = (float) atof(argv[3]);
    m_insert_fraction = (float) atof(argv[4]);
    m_delete_fraction = (float) atof(argv[5]);
    if (n <= 0) {
        printf("Invalid n !");
        exit(0);
    }
    if (m <= 0) {
        printf("invalid m !");
        exit(0);
    }
    if (m_member_fraction + m_insert_fraction + m_delete_fraction != 1.0) {
        printf("Invalid combination of fractions!");
        exit(0);
    }
} /*ValidateInput*/


double CalculateTime(struct timeval time_begin, struct timeval time_end) {
    return (double) (time_end.tv_usec - time_begin.tv_usec) / 1000000 + (double) (time_end.tv_sec - time_begin.tv_sec);
}/* Calculate Time */

void BuildOperationsArray() {
    operations = (int*)malloc(sizeof(int) * m);
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

void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}
