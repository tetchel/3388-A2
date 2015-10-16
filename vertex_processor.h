#ifndef FILEOPS_H
#define FILEOPS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SOURCEFILE "vase.txt"

typedef struct {
    double x, y, z, w;
} vertex;

int vertexProcessor(char*, char*, int);
void readVertices();
void rotateVertices();
void storePolygons();
void printVertex(FILE*, vertex*);

#endif // FILEOPS_H
