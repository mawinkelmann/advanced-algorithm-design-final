/*
 * final.c
 *
 *  Created on: Dec 6, 2017
 *      Author: eric_wagner
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define ADDED 1
#define OPEN_SPACE ' '
#define START_1 'S'
#define END_1 'E'
#define START_2 'F'
#define END_2 'L'

typedef struct{
	int row;
	int col;
}Location;

typedef struct{
	int rows;
	int cols;
	int **matrix;
	Location source1;
	Location source2;
	Location dest1;
	Location dest2;
}Matrix;

/*
 * Helper Funcitons:
 * initMatrix - Creates the maze within a matrix
 * mallMatrix - Creates space to hold a matrix the size of a maze
 * freeMatrix - Frees all memory allocated by the matrix
 * printMatrix - prints an easy to read version of the maze
 * revArray - Flips the order of a given array
*/
void initMatrix(Matrix *mat){
	int i =0, j = 0;

	if(mat == NULL){
		return;
	}

	for(i = 0; i < mat->rows; i++){
		for(j = 0; j < mat->cols; j++){
			mat->matrix[i][j] = 0;
		}
	}
}

Matrix *mallMatrix(int rows, int cols){
	int i = 0;
	Matrix *matrix = (Matrix *)malloc(sizeof(Matrix));

	matrix->rows = rows;
	matrix->cols = cols;

	matrix->matrix = (int **)malloc(rows * sizeof(int *));

	for (i = 0; i < rows; i++){
		matrix->matrix[i] = (int *)malloc(cols * sizeof(int));
	}

	initMatrix(matrix);
	return matrix;
}

void freeMatrix(Matrix *matrix){

	int i = 0;

	if(matrix == NULL){
		return;
	}

	for (i = 0; i < matrix->rows; i++){
		free(matrix->matrix[i]);
	}

	free(matrix->matrix);
	free(matrix);

}

void printMatrix(Matrix *mat){
	int i =0, j = 0;
	if(mat == NULL){
		return;
	}

	for(i = 0; i < mat->rows; i++){
		for(j = 0; j < mat->cols; j++){
			printf("%d", mat->matrix[i][j]);
		}
		printf("\n");
	}


}

void revArray(int *arr, int start, int end){
	int temp = 0;

	while (start < end)
	{
		temp = arr[start];
		arr[start] = arr[end];
		arr[end] = temp;
		start++;
		end--;
	}
}

int getRowMajor(int currRow, int currCol, int maxRow, int maxCol){
	return (currRow * maxCol)  + currCol;
}

int getRowIndex(int rowMajor, int maxRow, int maxCol){
	return rowMajor/maxCol;
}

int getColIndex(int rowMajor, int maxRow, int maxCol){
	int rowIndex = getRowIndex(rowMajor, maxRow, maxCol);
	return (rowMajor - (rowIndex * maxCol));
}

/*
 * File Handling:
 * countRow - Counts the number of rows in the file
 * countCol - Counts the number of columns in the file
 * readFile - Creates matrix of proper size and then fills it with the contents of the maze
 */
int countRow(char *fileName){
	FILE *fp = fopen(fileName, "r");
	char * line = NULL;
	size_t size = 0;
	int rowCount = 0;
	int read = 0;

	if (fp == NULL){
		perror("file:");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &size, fp)) != -1) {
		rowCount++;
	}

	fclose(fp);
	free(line);

	return rowCount;
}

int countCol(char *fileName){
	FILE *fp = fopen(fileName, "r");
	char * line = NULL;
	size_t size = 0;
	int colCount = 0, currColCount = 0;
	int read = 0;

	if (fp == NULL){
		perror("file:");
		exit(EXIT_FAILURE);
	}

	while((read = getline(&line, &size, fp)) != -1) {
		currColCount = strlen(line);

		if(currColCount > colCount){
			colCount = currColCount;
		}
	}

	fclose(fp);
	free(line);

	return colCount;
}

Matrix *readFile(char *fileName){
	int rows = countRow(fileName);
	int cols = countCol(fileName);
	int i = 0;
	int read = 0;
	FILE *fp = fopen(fileName, "r");
	char * line = NULL;
	size_t size = 0;
	int currRow = 0, currCol = 0;

	Matrix *matrix = NULL;

	if (fp == NULL){
		perror("file:");
		exit(EXIT_FAILURE);
	}

	matrix = mallMatrix(rows, cols);

	currRow = 0;

	while ((read = getline(&line, &size, fp)) != -1){
		currCol = 0;

		for(i = 0; i < strlen(line); i++){
			if(line[i] == OPEN_SPACE){
				matrix->matrix[currRow][currCol] = 1;
			}else if(line[i] == START_1){
				matrix->matrix[currRow][currCol] = 1;
				matrix->source1.row = currRow;
				matrix->source1.col = currCol;
			}else if(line[i] == START_2){
				matrix->matrix[currRow][currCol] = 1;
				matrix->source2.row = currRow;
				matrix->source2.col = currCol;
			}else if(line[i] == END_1){
				matrix->matrix[currRow][currCol] = 1;
				matrix->dest1.row = currRow;
				matrix->dest1.col = currCol;
			}else if(line[i] == END_2){
				matrix->matrix[currRow][currCol] = 1;
				matrix->dest2.row = currRow;
				matrix->dest2.col = currCol;
			}else{
				matrix->matrix[currRow][currCol] = 0;
			}
			currCol++;
		}
		currRow++;
	}

	fclose(fp);
	free(line);

	return matrix;
}
/*Graph Functions:
 * genGraph - generates a graph using a maze held in a matrix
 * getPath - finds all paths for the robots to exit the maze
 * minDist - finds the smallest path for each robot
 * dijkstra - the search algorithm used in for parsing the graph
 * delEdge - removes an edge from the graph
 * addEdge - adds an edge t0 the graph
 * delNode - removes a node from the graph
 * addNode - adds a node to the graph
 */
Matrix *generateGraph(Matrix *room){
	int dimension = (room->rows) * (room->cols);
	int i = 0, j = 0;
	Matrix *matrix = mallMatrix(dimension, dimension);
	int src = 0, dest = 0;

	matrix->source1.row = room->source1.row;
	matrix->source1.col = room->source1.col;
	matrix->source2.row = room->source2.row;
	matrix->source2.col = room->source2.col;
	matrix->dest1.row = room->dest1.row;
	matrix->dest1.col = room->dest1.col;
	matrix->dest2.col = room->dest2.col;

	for(i = 0; i < room->rows; i++){
		for(j = 0; j < room->cols; j++){
			src = getRowMajor(i, j, room->rows, room->cols);
			//check South
			if((i+1) < room->rows && room->matrix[i][j] == 1 && room->matrix[i+1][j] == 1){
				dest = getRowMajor(i+1, j, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check North
			if((i-1) >= 0 && room->matrix[i][j] == 1 && room->matrix[i-1][j] == 1){
				dest = getRowMajor(i-1, j, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check West
			if(j-1 >= 0 && room->matrix[i][j] == 1 && room->matrix[i][j-1] == 1){
				dest = getRowMajor(i, j-1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check East
			if(j+1 < room->cols && room->matrix[i][j] == 1 && room->matrix[i][j+1] == 1){
				dest = getRowMajor(i, j+1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//Check north-east
			if(i-1 >= 0 && j+1 < room->cols && room->matrix[i][j] == 1 && room->matrix[i-1][j+1] == 1){
				dest = getRowMajor(i-1, j+1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check north-west
			if((i-1) >= 0 && j-1 >= 0  && room->matrix[i][j] == 1 && room->matrix[i-1][j-1] == 1){
				dest = getRowMajor(i-1, j-1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check south-east
			if((i+1) < room->rows && j+1 < room->cols && room->matrix[i][j] == 1 && room->matrix[i+1][j+1] == 1){
				dest = getRowMajor(i+1, j+1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}
			//check south-west
			if((i+1) < room->rows && j-1 >= 0 && room->matrix[i][j] == 1 && room->matrix[i+1][j-1] == 1){
				dest = getRowMajor(i+1, j-1, room->rows, room->cols);
				matrix->matrix[src][dest] = 1;
			}

		}
	}
	return matrix;
}

int *getPath(int dist[], int dimension, int parent[], int src, int dest){
	int j = dest, i = 0;
	int *path = (int *)malloc(sizeof(int) * dimension);

	if(path == NULL){
		perror("Malloc:");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < dimension; i++){
		path[i] = -1;
	}

	i = 0;
	while(parent[j] != -1){
		path[i] = j;
		j = parent[j];
		i++;
	}

	if(path[i] != -1){
		path[i] = src;
		i++;
	}

	revArray(path, 0, i-1);
	return path;
}

int minDistance(int dist[], bool queue[], int dimension)
{
	int min = INT_MAX, min_index;
    int v;

	for (v = 0; v < dimension; v++){
		if (queue[v] == false && dist[v] < min){
			min = dist[v], min_index = v;
		}
	}

	return min_index;
}


int *dijkstra(Matrix *graph, int src, int dest){
	int *dist = NULL;
	bool *queue = NULL;
	int *parent = NULL;
	int dimension = 0;

	if(graph == NULL){
		return NULL;
	}

	if(src < 0 || src > graph->rows){
		fprintf(stderr, "Source not set: %d\n", src);
		exit(EXIT_FAILURE);
	}


	dimension = graph->rows;

	dist = (int *)malloc(sizeof(int) * dimension);
	if(dist == NULL){
		perror("malloc:");
		exit(EXIT_FAILURE);
	}

	parent = (int *)malloc(sizeof(int) * dimension);
	if(parent == NULL){
		perror("malloc:");
		exit(EXIT_FAILURE);
	}

	queue = (bool *)malloc(sizeof(bool) * dimension);
	if(queue == NULL){
		perror("malloc:");
		exit(EXIT_FAILURE);
	}

    int i;
	for (i = 0; i < dimension; i++)
	{
		parent[i] = -1;
		dist[i] = INT_MAX;
		queue[i] = false;
	}

	dist[src] = 0;

    int count;
	for (count = 0; count < dimension-1; count++)
	{
		int u = minDistance(dist, queue, dimension);
		queue[u] = true;

        int v;
		for (v = 0; v < dimension; v++){
			if (!queue[v] && graph->matrix[u][v] && (dist[u] + graph->matrix[u][v] < dist[v]))
			{
				parent[v]  = u;
				dist[v] = dist[u] + graph->matrix[u][v];
			}
		}
	}

	int *path = getPath(dist, dimension, parent, src, dest);

	free(dist);
	free(queue);
	free(parent);

	return path;
}

void deleteEdge(Matrix *graph, int src, int dest){
	graph->matrix[src][dest] = 0;
	graph->matrix[dest][src] = 0;
}

void addEdge(Matrix *graph, int src, int dest){
	graph->matrix[src][dest] = 1;
	graph->matrix[dest][src] = 1;
}

void deleteNodeAndNeighbour(Matrix *graph, int src, int *deletedEdge){
	int i = 0, j = 0;

	for(i = 0; i < graph->rows; i++){
		if(graph->matrix[src][i] == 1){
			deleteEdge(graph, src, i);
			deletedEdge[j] = i;
			j++;
		}
	}
}

void addNodeAndNeighbour(Matrix *graph, int src, int *deletedEdge){
	int i = 0, j = 0;

	//Search neighbour
	while(deletedEdge[j] != -1){
		i = deletedEdge[j];
		addEdge(graph, src, i);
		j++;
	}
}
/*
 * Robot Functions:
 * getDir - finds the directions for the robot to move
 * findPath - creates the paths for each robot to follow
 */
char *getDir(int src, int dest, int rows, int cols)
{
	char *sol = (char *)malloc(sizeof(char) * 16);
	memset(sol, '\0', 16);

	int srcRow = getRowIndex(src, rows, cols), srcCol = getColIndex(src, rows, cols), destRow = getRowIndex(dest, rows, cols), destCol = getColIndex(dest, rows, cols);

	int rowDiff = srcRow - destRow;
	int colDiff = srcCol - destCol;

	if(rowDiff < 0 && colDiff == 0)
	{
		strcpy(sol, "Move North");
	}
	else if(rowDiff < 0 && colDiff < 0)
	{
		strcpy(sol, "Move Northwest");
	}
	else if(rowDiff < 0 && colDiff > 0)
	{
		strcpy(sol, "Move Northeast");
	}
	else if(rowDiff > 0 && colDiff == 0)
	{
		strcpy(sol, "Move South");
	}
	else if(rowDiff > 0 && colDiff > 0)
	{
		strcpy(sol, "Move Southwest");
	}
	else if(rowDiff == 0 && colDiff > 0)
	{
		strcpy(sol, "Move East");
	}
	else if(rowDiff == 0 && colDiff < 0)
	{
		strcpy(sol, "Move West");
	}
	else if(rowDiff > 0 && colDiff < 0)
	{
		strcpy(sol, "Move Southeast");
	}
	else
	{
		strcpy(sol, "Wait");
	}
	return sol;
}

void findPath(Matrix *graph, Matrix *room)
{
	int *path1 = NULL, *path2 = NULL, i = 0;
	int source1 = 0, dest1 = 0, source2 = 0, dest2 = 0;
	int deletedEdge[8];
	char *direction = NULL;
	int firstMove = 0;

	for(i = 0; i < 8; i++)
	{
		deletedEdge[i] = -1;
	}

	source1 = getRowMajor(room->source1.row, room->source1.col, room->rows, room->cols);
	dest1 = getRowMajor(room->dest1.row, room->dest1.col, room->rows, room->cols);
	source2 = getRowMajor(room->source2.row, room->source2.col, room->rows, room->cols);
	dest2 = getRowMajor(room->dest2.row, room->dest2.col, room->rows, room->cols);


	deleteNodeAndNeighbour(graph, source2, &deletedEdge[0]);
	path1 = dijkstra(graph, source1, dest1);

	addNodeAndNeighbour(graph, source2, &deletedEdge[0]);

	for(i = 0; i < 8; i++)
	{
		deletedEdge[i] = -1;
	}

	if(path1[0] != -1)
	{
		deleteNodeAndNeighbour(graph, dest1, &deletedEdge[0]);
		firstMove = 1;
	}
	else
	{
		deleteNodeAndNeighbour(graph, source1, &deletedEdge[0]);
	}

	path2 = dijkstra(graph, source2, dest2);

	if(path1[0] != -1)
	{
		addNodeAndNeighbour(graph, dest1, &deletedEdge[0]);
	}
	else
	{
		addNodeAndNeighbour(graph, source1, &deletedEdge[0]);

		if(path2[0] != -1)
		{
			for(i = 0; i < 8; i++)
			{
				deletedEdge[i] = -1;
			}

			firstMove = 2;
			deleteNodeAndNeighbour(graph, dest2, &deletedEdge[0]);
			path1 = dijkstra(graph, source1, dest1);
			addNodeAndNeighbour(graph, dest2, &deletedEdge[0]);
		}
	}

	for(i = 0; i < 8; i++)
	{
		deletedEdge[i] = -1;
	}


	if(firstMove == 0)
	{
		printf("Unable to move any robot\n");
	}
	else if(path1[0] == -1)
	{
		printf("Unable to first robot\n");
	}
	else if(path2[0] == -1)
	{
		printf("Unable to second robot\n");
	}

	int count1 = 0, count2 = 0, repeat = 0;
	char *prevDir = (char*)malloc(sizeof(char)*16);

	if(firstMove == 1)
	{
		printf("First the Robot from %c will travel to %c by following these instructions:\n", START_1, END_1);

		i = 1;
		while(path1[i] != -1){
			if(direction != NULL)
				strcpy(prevDir, direction);
			direction = getDir(path1[i], path1[i-1], room->rows, room->cols);
			if(strcmp(direction, prevDir) == 0)
			{
				repeat ++;
			}
			else
			{
				if(repeat == 0)
					printf("\n%s", direction);
				else
				{
					printf(" %d times\n%s", repeat+1, direction);
					repeat = 0;
				}
			}
			i++;
			free(direction);
		}
		count1 = i-1;

		printf("\n\nThen the Robot from %c will travel to %c by following these instructions:\n", START_2, END_2);
		i = 1;
		repeat = 0;
		while(path2[i] != -1)
		{
			if(direction != NULL)
				strcpy(prevDir, direction);
			direction = getDir(path2[i], path2[i-1], room->rows, room->cols);
			if(strcmp(direction, prevDir) == 0)
			{
				repeat ++;
			}
			else
			{
				if(repeat == 0)
					printf("\n%s", direction);
				else
				{
					printf(" %d times\n%s", repeat+1, direction);
					repeat = 0;
				}
			}
			i++;
			free(direction);
		}
		count2 = i-1;

	}
	else if(firstMove == 2)
	{
		printf("First the Robot from %c will travel to %c by following these instructions\n", START_2, END_2);

                i = 1;
                repeat = 0;
                while(path2[i] != -1)
                {
        				direction = getDir(path2[i], path2[i-1], room->rows, room->cols);
        				if(strcmp(direction, prevDir) == 0)
        				{
        					repeat ++;
        				}
        				else
        				{
        					if(repeat == 0)
        						printf("%s\n", direction);
        					else
        					{
        						printf("%s %d times\n%s\n", prevDir, repeat+1, direction);
        						repeat = 0;
        					}
        				}
        				i++;
        				strcpy(prevDir, direction);
        				free(direction);
                }
                count1 = i-1;

                printf("\n\nThen the Robot from %c will travel to %c by following these instructions\n", START_1, END_1);
                i = 1;
                repeat = 0;
                while(path1[i] != -1)
                {
						direction = getDir(path1[i], path1[i-1], room->rows, room->cols);
						if(strcmp(direction, prevDir) == 0)
						{
							repeat ++;
						}
						else
						{
							if(repeat == 0)
								printf("%s\n", direction);
							else
							{
								printf("%s %d times\n%s\n", prevDir, repeat+1, direction);
								repeat = 0;
							}
						}
						i++;
						strcpy(prevDir, direction);
						free(direction);
                }
                count2 = i-1;

	}
    printf("\n\nRobot 1 reached the destination in %d moves\n", count1);
    printf("Robot 2 reached the destination in %d moves\n", count2);

    free(prevDir);
	free(path1);
	free(path2);
}





int main(int args, char **argv)
{
	char *fileName = NULL;
	Matrix *matrix = NULL;
	Matrix *graph = NULL;

	if(args < 2){
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fileName = argv[1];
	printf("Rows: %d\n", countRow(fileName));
	printf("Cols: %d\n", countCol(fileName));
	matrix = readFile(fileName);
	graph = generateGraph(matrix);

	findPath(graph, matrix);

	freeMatrix(matrix);
	freeMatrix(graph);
	return 0;
}
