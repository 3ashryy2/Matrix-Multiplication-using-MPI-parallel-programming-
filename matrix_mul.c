#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

MPI_Status status;
int *a, *b, *c;
int main(int argc, char **argv)
{
  int numtasks, taskid, numworkers, source, dest, rows, start, i, j, k, _1stRows, _2ndRows, _1stCols, _2ndCols, choice, tmp = 0;
  char tmp1[2];
  FILE *myFile;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  if (taskid == 0)
  {
    printf("Welcome to vector matrix multiplication program!\nTo read dimensions and values from file press 1\nTo read dimensions and values from console press 2\n");
    scanf("%d", &choice);

    // Reading input from console
    if (choice == 2)
    {
      printf("Please enter dimensions of the first matrix: ");
      scanf("%d %d", &_1stRows, &_1stCols);
      printf("Please enter values of the first matrix:\n");

      a = (int *)malloc(_1stRows * _1stCols * sizeof(int));

      for (i = 0; i < _1stRows; i++)
        for (j = 0; j < _1stCols; j++)
        {
          scanf("%d", &tmp);
          a[i * _1stCols + j] = tmp;
        }

      printf("Please enter dimensions of the second matrix: ");
      scanf("%d %d", &_2ndRows, &_2ndCols);

      printf("Please enter values of the second matrix:\n");

     b = (int *)malloc(_2ndRows * _2ndCols * sizeof(int));

      for (i = 0; i < _2ndRows; i++)
        for (j = 0; j < _2ndCols; j++)
        {
          scanf("%d", &tmp);
          b[i * _2ndCols + j] = tmp;
        }
    }

    // Reading input from file
    else if (choice == 1)
    {
      myFile = fopen("numbers.txt", "r");
      fscanf(myFile, "%s", &tmp1[0]);
      fscanf(myFile, "%s", &tmp1[1]);

      // casting characters int integers
      _1stRows = tmp1[0] - '0';
      _1stCols = tmp1[1] - '0';

      a = (int *)malloc(_1stRows * _1stCols * sizeof(int));

      for (i = 0; i < _1stRows; i++)
        for (j = 0; j < _1stCols; j++)
        {
          fscanf(myFile, "%s", &tmp1[0]);
          a[i * _1stCols + j] = tmp1[0] - '0';
        }
      fscanf(myFile, "%s", &tmp1[0]);
      fscanf(myFile, "%s", &tmp1[1]);

      _2ndRows = tmp1[0] - '0';
      _2ndCols = tmp1[1] - '0';

      b = (int *)malloc(_2ndRows * _2ndCols * sizeof(int));

      for (i = 0; i < _2ndRows; i++)
        for (j = 0; j < _2ndCols; j++)
        {
          fscanf(myFile, "%s", &tmp1[0]);
          b[i * _2ndCols + j] = tmp1[0] - '0';
        }

      fclose(myFile);
    }

    // Checking if matrices can be multiplied
    if (_1stCols != _2ndRows)
    {
      printf("Matrices cannot be multiplied.\n");
      return 0;
    }

    c = (int *)malloc(_1stRows * _2ndCols * sizeof(int));
    rows = _1stRows;
    int chunk = rows / (numtasks - 1);
    int extra = rows % (numtasks - 1);
    start = 0;
    dest = 0;

    // Sending data to worker tasks
    for (dest = 1; dest < numtasks; dest++)
    {
      rows = (dest <= extra) ? chunk + 1 : chunk;
      MPI_Send(&start, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&_1stCols, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&_2ndCols, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(a + start * _1stCols, rows * _1stCols, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(b, _1stCols * _2ndCols, MPI_INT, dest, 1, MPI_COMM_WORLD);
      start += rows;
    }

    // Receiving results from worker tasks
    for (i = 1; i < numtasks; i++)
    {
      source = i;
      MPI_Recv(&start, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(c + start * _2ndCols, rows * _2ndCols, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
    }

    // Printing the result matrix
    printf("Result matrix:\n");
    for (i = 0; i < _1stRows; i++)
    {
      for (j = 0; j < _2ndCols; j++)
        printf("%d ", c[i * _2ndCols + j]);
      printf("\n");
    }

    free(a);
    free(b);
    free(c);
  }

  // Worker tasks
  if (taskid > 0)
  {
    MPI_Recv(&start, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&_1stCols, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&_2ndCols, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    int *a = (int *)malloc(rows * _1stCols * sizeof(int));
    int *b = (int *)malloc(_1stCols * _2ndCols * sizeof(int));
    int *c = (int *)malloc(rows * _2ndCols * sizeof(int));
    MPI_Recv(a, rows * _1stCols, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(b, _1stCols * _2ndCols, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    // Multiplying matrices
    for (i = 0; i < rows; i++)
    {
      for (j = 0; j < _2ndCols; j++)
      {
        c[i * _2ndCols + j] = 0;
        for (k = 0; k < _1stCols; k++)
          c[i * _2ndCols + j] += a[i * _1stCols + k] * b[k * _2ndCols + j];
      }
    }

    MPI_Send(&start, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(c, rows * _2ndCols, MPI_INT, 0, 2, MPI_COMM_WORLD);

    free(a);
    free(b);
    free(c);
  }

  MPI_Finalize();
  return 0;
}