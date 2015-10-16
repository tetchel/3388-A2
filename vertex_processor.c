#include "vertex_processor.h"

/**
*   Vertex_Processor
*   Author Tim Etchells
*   CS3388 Assignment 2 - Professor Beauchemin
*   For October 16, 2015
*   This file performs all the necessary operations to store the polygons that form the wiremesh as 3-tuples in OUTFILE.
*   vertexProcessor interfaces with the main program, the other functions are simply separate for readability.
**/

vertex** vertices;      //2D array to hold all verticies.
char axis;              //axis around which to rotate the object
int rotation,           //rotation increment in degrees
    num_rotations,      //number of times the object will be rotated (equal to 360/rotation)
    num_vertices_;      //number of verticies to expect in the file
char *infile_,          //file to get vertex input from
     *outfile_;         //file to print polygon data to

//calls the other methods to prepare polygons and store them in a file
//accepts the source file (vase.txt), the file in which to store the polygon data (polys.txt), and the number of vertices to expect in infile (equal to # of lines - 2)
//returns the number of polygons (lines) in the polygon file
int vertexProcessor(char* infile, char* outfile, int num_verticies) {
    infile_ = infile;
    outfile_ = outfile;
    num_vertices_ = num_verticies;
    readVertices();
    rotateVertices();
    storePolygons();

    //return the number of polygons
    return 2*num_rotations*(num_vertices_+1);
}

//reads through the vertices in infile_, storing them in 2D array vertices[][]
void readVertices() {
    FILE* fp = fopen(infile_, "r");

    //make sure it opened
    if(!fp) {
        printf("Couldn't open %s\n", infile_);
        exit(EXIT_FAILURE);
    }

    //get the first two values, axis and angle of rotation
    fscanf(fp, "%c", &axis);
    fscanf(fp, "%d", &rotation);
    //divide 360/ the rotation increment to get the number of rotations
    num_rotations = 360/rotation;

    //reserve memory for vertices[0]
    vertices = malloc(num_rotations*sizeof(vertex));
    //make sure it succeeded
    if(!vertices)
        printf("Malloc error! 1\n");

    int i;
    //reserve memory for the subarrays
    for(i = 0; i < num_vertices_; i++) {
        vertices[i] = malloc(num_vertices_*sizeof(vertex));
        if(!vertices[i])
            printf("Malloc error! 2\n");
    }

    //loop through the rest of the file and assign x/y/z/w values
    for(i = 0; i < num_vertices_; i++) {
        //get x,y,z values
        //%*f skips over w value, since we know it's 1.0
        //store the vertex in the first part of the array
        //(vertices[0] represents the non-rotated vertices)
        fscanf(fp, "%lf %lf %lf %*f", &vertices[0][i].x, &vertices[0][i].y, &vertices[0][i].z);
    }
    fclose(fp);
}

//performs the rotation step on all vertices, filling the rest of vertices[][] with the rotated values.
void rotateVertices() {
    int i, j;
    //convert the angle of rotation to radians
    double angle_rads = rotation * (M_PI / 180);
    //perform all the rotations and store the results in vertices
    for(i = 0; i < num_rotations; i++) {
        //compute sin, cos of the current angle in radians
        double  sinr = sin(angle_rads*i),
                cosr = cos(angle_rads*i);

        for(j = 0; j < num_vertices_; j++) {
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

//loops through all vertices, groups them into 3-tuples, and prints them to outfile_.
//each line in outfile_ represents a tuple that describes a polygon.
void storePolygons() {
    FILE* fp = fopen(outfile_, "w");
    if(!fp) {
        printf("Couldn't open %s\n", outfile_);
        exit(EXIT_FAILURE);
    }

    int i, j;
    //debug only
//    FILE* vertfile = fopen("verts.txt", "w");
//    int count = 0;
//    for(i = 0; i < num_rotations; i++) {
//        for(j = 0; j < NUM_VERTEX; j++) {
//            fprintf(vertfile,"%d %lf %lf %lf %lf\n", count++, vertices[i][j].x, vertices[i][j].y, vertices[i][j].z, 1.0);
//        }
//    }
//    fclose(vertfile);
    //loop through 2d array and print each vertex
    for(i = 0; i < num_rotations; i++) {
        //-1 from NUM_VERTEX to stop out-of-bounds when [j+1] is requested
        for(j = 0; j < num_vertices_-1; j++) {
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
    //release memory alloced for vertices array
    for(i = 0; i < num_rotations; i++) {
           free(vertices[i]);
    }
    free(vertices);
}

//print the given vertex to the given file in the form x, y, z, w
//since w = 1.0, 1.0 is printed each time.
void printVertex(FILE* fp, vertex* v) {
    fprintf(fp, "%lf %lf %lf 1.0 ", v->x, v->y, v->z);
}
