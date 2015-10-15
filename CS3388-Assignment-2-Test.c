/*            PURPOSE : Synthetic camera framework.

 PREREQUISITES : matrix.h

 */

#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

#define Ex 10.0
#define Ey 10.0
#define Ez 10.0

#define Gx 0.0
#define Gy 0.0
#define Gz 0.0

#define UPx 0.0
#define UPy 0.0
#define UPz 1.0

#define Near 1.0
#define Far 50.0

#define THETA 45.0
#define ASPECT 1.5

#define H 512

typedef struct {
    int width, height ;
} window_t ;

typedef struct {
    dmatrix_t UP ;
    dmatrix_t E ;
    dmatrix_t G ;
    dmatrix_t u, v, n ;
    dmatrix_t Mv, Mp ;
    dmatrix_t T1, T2, S1, S2, W ;
    dmatrix_t M ;
} camera_t ;

Display *InitX(Display *d, Window *w, int *s, window_t *Window) {

    d = XOpenDisplay(NULL) ;
    if(d == NULL) {
        printf("Cannot open display\n") ;
        exit(1) ;
    }
    *s = DefaultScreen(d) ;
    *w = XCreateSimpleWindow(d, RootWindow(d, *s), 0, 0, Window->width, Window->height, 1, BlackPixel(d, *s), WhitePixel(d, *s)) ;
    Atom delWindow = XInternAtom(d, "WM_DELETE_WINDOW", 0) ;
    XSetWMProtocols(d, *w, &delWindow, 1) ;
    XSelectInput(d, *w, ExposureMask | KeyPressMask) ;
    XMapWindow(d, *w) ;
    return(d) ;
}


void SetCurrentColorX(Display *d, GC *gc, unsigned int r, unsigned int g, unsigned int b) {

    XSetForeground(d, *gc, r << 16 | g << 8 | b) ;
}


void SetPixelX(Display *d, Window w, int s, int i, int j) {

    XDrawPoint(d, w, DefaultGC(d, s), i, j) ;
}


void QuitX(Display *d, Window w) {

    XDestroyWindow(d,w) ;
    XCloseDisplay(d) ;
}

double dot_product(dmatrix_t *a, dmatrix_t *b) {

    double s  ;

    s = a->m[1][1]*b->m[1][1] + a->m[2][1]*b->m[2][1] + a->m[3][1]*b->m[3][1] ;

    return(s) ;
}

dmatrix_t *projection_transform(dmatrix_t *p) {

    dmatrix_t *q ;

    q = (dmatrix_t *)malloc(sizeof(dmatrix_t)) ;
    dmat_alloc(q,4,1) ;

    if ((p->l == 4) && (p->c == 1)) {
        for (int i = 1 ; i <= 4 ; i++) {
            q->m[i][1] = p->m[i][1]/p->m[4][1] ;
        }
    }
    return q ;
}

window_t *build_window(window_t *Window, int height, float aspect) {

    Window->height = height ;
    Window->width =  aspect*height ;

    return(Window) ;
}

camera_t *build_camera(camera_t *Camera, window_t *Window) {

    dmat_alloc(&Camera->E,4,1) ;

    Camera->E.m[1][1] = Ex ;
    Camera->E.m[2][1] = Ey ;
    Camera->E.m[3][1] = Ez ;
    Camera->E.m[4][1] = 1.0 ;

    dmat_alloc(&Camera->G,4,1) ;

    Camera->G.m[1][1] = Gx ;
    Camera->G.m[2][1] = Gy ;
    Camera->G.m[3][1] = Gz ;
    Camera->G.m[4][1] = 1.0 ;

    dmat_alloc(&Camera->n,4,1) ;
    Camera->n = *dmat_normalize(dmat_sub(&Camera->E,&Camera->G)) ;
    Camera->n.l = 3 ;

    dmat_alloc(&Camera->UP,4,1) ;

    Camera->UP.l = 3 ;

    Camera->UP.m[1][1] = UPx ;
    Camera->UP.m[2][1] = UPy ;
    Camera->UP.m[3][1] = UPz ;
    Camera->UP.m[4][1] = 1.0 ;

    dmat_alloc(&Camera->u,4,1) ;

    Camera->u = *dmat_normalize(dcross_product(&Camera->UP,&Camera->n)) ;
    Camera->v = *dmat_normalize(dcross_product(&Camera->n,&Camera->u)) ;

    dmat_alloc(&Camera->Mv,4,4) ;

    Camera->Mv.m[1][1] = Camera->u.m[1][1] ;
    Camera->Mv.m[1][2] = Camera->u.m[2][1] ;
    Camera->Mv.m[1][3] = Camera->u.m[3][1] ;
    Camera->Mv.m[1][4] = -1.0*dot_product(&Camera->E,&Camera->u) ;

    Camera->Mv.m[2][1] = Camera->v.m[1][1] ;
    Camera->Mv.m[2][2] = Camera->v.m[2][1] ;
    Camera->Mv.m[2][3] = Camera->v.m[3][1] ;
    Camera->Mv.m[2][4] = -1.0*dot_product(&Camera->E,&Camera->v) ;

    Camera->Mv.m[3][1] = Camera->n.m[1][1] ;
    Camera->Mv.m[3][2] = Camera->n.m[2][1] ;
    Camera->Mv.m[3][3] = Camera->n.m[3][1] ;
    Camera->Mv.m[3][4] = -1.0*dot_product(&Camera->E,&Camera->n) ;

    Camera->Mv.m[4][1] = 0.0 ;
    Camera->Mv.m[4][2] = 0.0 ;
    Camera->Mv.m[4][3] = 0.0 ;
    Camera->Mv.m[4][4] = 1.0 ;

    dmat_alloc(&Camera->Mp,4,4) ;
    dmat_identity(&Camera->Mp) ;

    float a = -1.0*(Far + Near)/(Far - Near) ;
    float b = -2.0*(Far*Near)/(Far - Near) ;

    Camera->Mp.m[1][1] = Near ;
    Camera->Mp.m[2][2] = Near ;
    Camera->Mp.m[3][3] = a ;
    Camera->Mp.m[3][4] = b ;
    Camera->Mp.m[4][3] = -1.0 ;
    Camera->Mp.m[4][4] = 0.0 ;

    float top = Near*tan(M_PI/180.0*THETA/2.0) ;
    float right = ASPECT*top ;
    float bottom = -top ;
    float left = -right ;

    dmat_alloc(&Camera->T1,4,4) ;

    Camera->T1 = *dmat_identity(&Camera->T1) ;

    Camera->T1.m[1][4] = -(right + left)/2.0 ;
    Camera->T1.m[2][4] = -(top + bottom)/2.0 ;

    dmat_alloc(&Camera->S1,4,4) ;

    Camera->S1 = *dmat_identity(&Camera->S1) ;

    Camera->S1.m[1][1] = 2.0/(right - left) ;
    Camera->S1.m[2][2] = 2.0/(top - bottom) ;

    dmat_alloc(&Camera->T2,4,4) ;
    dmat_alloc(&Camera->S2,4,4) ;
    dmat_alloc(&Camera->W,4,4) ;

    Camera->T2 = *dmat_identity(&Camera->T2) ;
    Camera->S2 = *dmat_identity(&Camera->S2) ;
    Camera->W = *dmat_identity(&Camera->W) ;

    Camera->T2.m[1][4] = 1.0 ;
    Camera->T2.m[2][4] = 1.0 ;

    Camera->S2.m[1][1] = Window->width/2.0 ;
    Camera->S2.m[2][2] = Window->height/2.0 ;

    Camera->W.m[2][2] = -1.0 ;
    Camera->W.m[2][4] = (double)Window->height ;

    dmat_alloc(&Camera->M,4,4) ;

    Camera->M = *dmat_mult(&Camera->W,dmat_mult(&Camera->S2,dmat_mult(&Camera->T2,dmat_mult(&Camera->S1,dmat_mult(&Camera->T1,dmat_mult(&Camera->Mp,&Camera->Mv)))))) ;

    return(Camera) ;
}

int main() {

    Display *d ;
    Window w ;
    XEvent e ;
    int s ;

    dmatrix_t p1, p2, p3, p4 ;

    camera_t Camera ;
    window_t Window ;

    /* Building display window and synthetic camera */

    Window = *build_window(&Window,H,ASPECT) ;
    Camera = *build_camera(&Camera,&Window) ;

    dmat_alloc(&p1,4,1) ;
    dmat_alloc(&p2,4,1) ;
    dmat_alloc(&p3,4,1) ;
    dmat_alloc(&p4,4,1) ;

    /* Draw the three coordinate axes of the world reference frame */

    p1.m[1][1] = 0.0 ;
    p1.m[2][1] = 0.0 ;
    p1.m[3][1] = 0.0 ;
    p1.m[4][1] = 1.0 ;

    p2.m[1][1] = 0.0 ;
    p2.m[2][1] = 0.0 ;
    p2.m[3][1] = 5.0 ;
    p2.m[4][1] = 1.0 ;

    p3.m[1][1] = 5.0 ;
    p3.m[2][1] = 0.0 ;
    p3.m[3][1] = 0.0 ;
    p3.m[4][1] = 1.0 ;

    p4.m[1][1] = 0.0 ;
    p4.m[2][1] = 5.0 ;
    p4.m[3][1] = 0.0 ;
    p4.m[4][1] = 1.0 ;

    d = InitX(d,&w,&s,&Window) ;
    XNextEvent(d,&e) ;

    while (1) {
        XNextEvent(d,&e) ;
        if (e.type == Expose) {

            p1 = *projection_transform(dmat_mult(&(Camera.M),&p1)) ;
            p2 = *projection_transform(dmat_mult(&(Camera.M),&p2)) ;
            p3 = *projection_transform(dmat_mult(&(Camera.M),&p3)) ;
            p4 = *projection_transform(dmat_mult(&(Camera.M),&p4)) ;

            XDrawLine(d,w,DefaultGC(d,s),(int)p1.m[1][1],(int)p1.m[2][1],(int)p2.m[1][1],p2.m[2][1]) ;

            XDrawLine(d,w,DefaultGC(d,s),(int)p1.m[1][1],(int)p1.m[2][1],(int)p3.m[1][1],p3.m[2][1]) ;

            XDrawLine(d,w,DefaultGC(d,s),(int)p1.m[1][1],(int)p1.m[2][1],(int)p4.m[1][1],p4.m[2][1]) ;
        }
        if (e.type == KeyPress)
            break ;
        if (e.type == ClientMessage)
            break ;
    }
    QuitX(d,w) ;
}
