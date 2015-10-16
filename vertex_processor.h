#ifndef FILEOPS_H
#define FILEOPS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double x, y, z, w;
} vertex;

char* vertexProcessor();
void readVertices();
void rotateVertices();
void storePolygons();
void printVertex(FILE*, vertex*);

#endif // FILEOPS_H
