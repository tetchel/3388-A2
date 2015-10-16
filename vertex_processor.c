#include "vertex_processor.h"

#define SOURCEFILE "vase.txt"
#define OUTFILE "polys.txt"
//FILELEN could also be determined dynamically by counting lines, but this is easier for this assignment
#define FILELEN 26

vertex** vertices;
char axis;
int rotation, num_rotations;

char* vertexProcessor(int* num_polys) {
    readVertices();
    rotateVertices();
    storePolygons();
    //return the name of the polygon file
    char* ret;
    asprintf(&ret, "%s", OUTFILE);
    *num_polys = 2*num_rotations*(FILELEN-1);
    return ret;
}

void readVertices() {
    FILE* fp = fopen(SOURCEFILE, "r");

    //make sure it opened
    if(!fp) {
        printf("Couldn't open %s\n", SOURCEFILE);
        exit(EXIT_FAILURE);
    }

    //get the first two values, axis and angle
    fscanf(fp, "%c", &axis);
    fscanf(fp, "%d", &rotation);
    num_rotations = 360/rotation;

    //reserve memory for 2D array of vertices
    vertices = calloc(FILELEN, sizeof(vertex));
    int i;
    for(i = 0; i < num_rotations; i++) {
        vertices[i] = calloc(num_rotations, sizeof(vertex));
    }

    //loop through the rest of the file and assign x/y/z/w values
    for(i = 0; i < FILELEN; i++) {
        vertex* vt = calloc(1, sizeof(vertex));
        //get x,y,z values
        //%*f skips over w value, since we know it's 1.0
        fscanf(fp, "%lf %lf %lf %*f", &vt->x, &vt->y, &vt->z);
        //store the vertex in the first part of the array
        vertices[0][i] = *vt;
    }
    fclose(fp);
}

void rotateVertices() {
    int i, j;
    double rotation_rads = rotation * (M_PI / 180);

    for(i = 0; i < num_rotations; i++) {
        //compute sin, cos of the current angle - increases by i each time
        double  sinr = sin(rotation_rads*i),
                cosr = cos(rotation_rads*i);
        for(j = 0; j < FILELEN; j++) {
            //check for different rotation axes (even though it's z in this case)
            if(axis == 'z') {
                vertices[i][j].x = vertices[0][j].x * cosr;
                vertices[i][j].y = vertices[0][j].x * sinr;
                vertices[i][j].z = vertices[0][j].z;
            }
            else if(axis == 'y') {
                vertices[i][j].x = vertices[0][j].x * sinr;
                vertices[i][j].y = vertices[0][j].y;
                vertices[i][j].z = vertices[0][j].y * cosr;
            }
            else if(axis == 'x') {
                vertices[i][j].x = vertices[0][j].x;
                vertices[i][j].y = vertices[0][j].z * cosr;
                vertices[i][j].z = vertices[0][j].z * sinr;
            }
            else {
                printf("Axis of rotation not valid! Program will exit.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void storePolygons() {
    FILE* fp = fopen(OUTFILE, "w");
    int i, j;
//  vertices 515-519 are WRONG!!!
    int count = 0;
    for(i = 0; i < num_rotations; i++) {
        for(j = 0; j < FILELEN; j++) {
            printf("%d: %lf %lf %lf\n", count++, vertices[i][j].x, vertices[i][j].y, vertices[i][j].z);
        }
    }

    //loop through 2d array and print each vertex
    for(i = 0; i < num_rotations; i++) {
        for(j = 0; j < FILELEN-1; j++) {
            //print each vertex, counterclockwise
            printVertex(fp, &vertices[i][j]);
            printVertex(fp, &vertices[i][j+1]);
            //%num_rotations for the case when i+1 'wraps around', so i+1 goes from num_rotations+1 to 0
            printVertex(fp, &vertices[(i+1)%num_rotations][j+1]);
            //new polygon
            fprintf(fp, "\n");
            printVertex(fp, &vertices[i][j]);
            printVertex(fp, &vertices[(i+1)%num_rotations][j+1]);
            printVertex(fp, &vertices[(i+1)%num_rotations][j]);
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}

void printVertex(FILE* fp, vertex* v) {
    fprintf(fp, "%lf %lf %lf 1.0 ", v->x, v->y, v->z);
}
