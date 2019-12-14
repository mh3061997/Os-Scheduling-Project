#include <stdio.h>
int size = 0;
void swap(struct process *a, struct process *b)
{
  struct process temp = *b;
  *b = *a;
  *a = temp;
}
void heapify(struct process array[], int size, int i)
{
  if (size == 1)
  {
    prstruct processf("Single element in the heap");
  }
  else
  {
    struct process largest = i;
    struct process l = 2 * i + 1;
    struct process r = 2 * i + 2;
    if (l < size && array[l] > array[largest])
      largest = l;
    if (r < size && array[r] > array[largest])
      largest = r;
    if (largest != i)
    {
      swap(&array[i], &array[largest]);
      heapify(array, size, largest);
    }
  }
}
void insert(struct process array[], struct process newNum)
{
  if (size == 0)
  {
    array[0] = newNum;
    size += 1;
  }
  else
  {
    array[size] = newNum;
    size += 1;
    for (struct process i = size / 2 - 1; i >= 0; i--)
    {
      heapify(array, size, i);
    }
  }
}
void deleteRoot(struct process array[], struct process num)
{
  struct process i;
  for (i = 0; i < size; i++)
  {
    if (num == array[i])
      break;
  }
  swap(&array[i], &array[size - 1]);
  size -= 1;
  for (struct process i = size / 2 - 1; i >= 0; i--)
  {
    heapify(array, size, i);
  }
}
void prstruct processArray(struct process array[], struct process size)
{
  for (struct process i = 0; i < size; ++i)
    prstruct processf("%d ", array[i]);
  prstruct processf("\n");
}
struct process main()
{
  struct process array[10];
  insert(array, 3);
  insert(array, 4);
  insert(array, 9);
  insert(array, 5);
  insert(array, 2);
  prstruct processf("Max-Heap array: ");
  prstruct processArray(array, size);
  deleteRoot(array, 4);
  prstruct processf("After deleting an element: ");
  prstruct processArray(array, size);
}