#include <stdio.h>
#include <stdlib.h>

struct node
{
    int data;
    struct node *next;
};

struct Queue
{
    struct node *front;
    struct node *rear;
};

void display(struct Queue *queue);
void enqueue(struct Queue *queue, int item);
struct Queue *dequeue(struct Queue *queue);
int main()
{

    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->front = NULL;
    queue->rear = NULL;

    enqueue(queue, 1);
    enqueue(queue, 2);
    enqueue(queue, 3);
    struct node* temp =dequeue(queue);
    enqueue(queue,temp->data);
    display(queue);
}

void enqueue(struct Queue *queue, int item)
{

    struct node *nptr = (struct node *)malloc(sizeof(struct node));
    nptr->data = item;
    nptr->next = NULL;
    if (queue->rear == NULL)
    {
        printf("REAR NULL\n");
        queue->front = nptr;
        queue->rear = nptr;
    }
    else
    {
        queue->rear->next = nptr;
        queue->rear = queue->rear->next;
    }
}

void display(struct Queue *queue)
{

    struct node *temp;
    temp = queue->front;
    printf("\n");
    while (temp != NULL)
    {
        printf("%d\t", temp->data);
        temp = temp->next;
    }
}

struct Queue *dequeue(struct Queue *queue)
{

    if (queue->front == NULL)
    {
        printf("\n\nqueue is empty \n");
        return 0;
    }
    else
    {
        struct node *temp;
        temp = queue->front;
        queue->front = queue->front->next;
        printf("\n\n%d deleted", temp->data);
        //free(temp);
        return temp;
    }
}
/*
#include <stdio.h>
#include <stdlib.h>

struct node
{
    int data;
    struct node *next;
};

struct node *front = NULL;
struct node *rear = NULL;

void display();
void enqueue(int);
void dequeue();

int main()
{
    int n, ch;
    do
    {
        printf("\n\nQueue Menu\n1. Add \n2. Remove\n3. Display\n0. Exit");
        printf("\nEnter Choice 0-3? : ");
        scanf("%d", &ch);
        switch (ch)
        {
            case 1:
                printf("\nEnter number ");
                scanf("%d", &n);
                enqueue(n);
                break;
            case 2:
                dequeue();
                break;
            case 3:
                display();
                break;
        }
    }while (ch != 0);
}

void enqueue(int item)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->data = item;
    nptr->next = NULL;
    if (rear == NULL)
    {
        front = nptr;
        rear = nptr;
    }
    else
    {
        rear->next = nptr;
        rear = rear->next;
    }
}

void display()
{
    struct node *temp;
    temp = front;
    printf("\n");
    while (temp != NULL)
    {
        printf("%d\t", temp->data);
        temp = temp->next;
    }
}

void dequeue()
{
    if (front == NULL)
    {
        printf("\n\nqueue is empty \n");
    }
    else
    {
        struct node *temp;
        temp = front;
        front = front->next;
        printf("\n\n%d deleted", temp->data);
        free(temp);
    }
}
*/