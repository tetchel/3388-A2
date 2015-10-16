#include "vertex_processor.h"

#define SOURCEFILE "vase.txt"
#define OUTFILE "polys.txt"
//NUM_VERTEX could also be determined dynamically by counting lines, but this is sufficient
#define NUM_VERTEX 26

/**
*   This file performs all the necessary operations to store the polygons that form the wiremesh as 3-tuples in OUTFILE.
*   vertexProcessor is the part of this file that interfaces with the main program, and returns the name of the file as well as
*   setting the number of polygons in an integer whose pointer is passed.
**/

vertex** vertices;
char axis;
int rotation, num_rotations;

//calls the other methods to prepare polygons and store them in a file
//returns the name of the polygon file
//also sets *num_polys to the number of polygons, so the caller knows how long the file is.
char* vertexProcessor(int* num_polys) {
    readVertices();
    rotateVertices();
    storePolygons();

    //set the number of polygons
    *num_polys = 2*num_rotations*(NUM_VERTEX-1);

    //return the name of the polygon file
    char* ret;
    asprintf(&ret, "%s", OUTFILE);
    return ret;
}

//reads through the vertices in SOURCEFILE, storing them in 2D array vertices[][]
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
    //divide 360/ the rotation increment to get the number of rotations
    num_rotations = 360/rotation;

    //reserve memory for vertices[0]
    vertices = malloc((NUM_VERTEX+1)*sizeof(vertex));
    //make sure it succeeded
    if(!vertices)
        printf("Malloc error! 1\n");

    int i;
    //reserve memory for the rest of the vertices 2D array
    for(i = 0; i < num_rotations+1; i++) {
        vertices[i] = malloc(num_rotations*sizeof(vertex));
        if(!vertices[i])
            printf("Malloc error! 2\n");
    }

    //loop through the rest of the file and assign x/y/z/w values
    for(i = 0; i < NUM_VERTEX; i++) {
        //reserve memory for current item
        vertex* vt = malloc(sizeof(vertex));
        if(!vt) {
            printf("Malloc error! 3\n");
        }
        //get x,y,z values
        //%*f skips over w value, since we know it's 1.0
        fscanf(fp, "%lf %lf %lf %*f", &vt->x, &vt->y, &vt->z);
        //store the vertex in the first part of the array (vertices[0] represents the non-rotated vertices)
        vertices[0][i] = *vt;
    }
    fclose(fp);
}

//performs the rotation step on all vertices, filling the rest of vertices[][] with the rotated values.
void rotateVertices() {
    int i, j;
    //convert to radians
    double rotation_rads = rotation * (M_PI / 180);

    for(i = 0; i < num_rotations; i++) {
        //compute sin, cos of the current angle, which increases by rotation_rads each time
        double  sinr = sin(rotation_rads*i),
                cosr = cos(rotation_rads*i);
        //why did I have NUM_VERTEX - 1 previously?
        for(j = 0; j < NUM_VERTEX; j++) {
            //check for different rotation axes (even though we know it's z in this case)
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

//loops through all vertices, groups them into 3-tuples, and prints them to OUTFILE.
//each line in OUTFILE represents the 3 points that describe a polygon.
void storePolygons() {
    FILE* fp = fopen(OUTFILE, "w");
    if(!fp) {
        printf("Couldn't open %s\n", OUTFILE);
        exit(EXIT_FAILURE);
    }

    int i, j;
//  vertices 515-519 are WRONG!!!
    //debug only
    int count = 0;
    for(i = 0; i < num_rotations; i++) {
        for(j = 0; j < NUM_VERTEX; j++) {
            printf("%d: %lf %lf %lf\n", count++, vertices[i][j].x, vertices[i][j].y, vertices[i][j].z);
        }
    }

    //loop through 2d array and print each vertex
//    for(i = 0; i < num_rotations; i++) {
        i = 0;
        for(j = 0; j < NUM_VERTEX-1; j++) {
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
//    }

    fclose(fp);
}

//print the given vertex to the given file in the form x, y, z, w
//since w = 1.0, 1.0 is printed each time.
void printVertex(FILE* fp, vertex* v) {
    fprintf(fp, "%lf %lf %lf 1.0 ", v->x, v->y, v->z);
}
