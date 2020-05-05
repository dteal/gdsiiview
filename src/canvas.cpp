#include "canvas.h"

Canvas::Canvas(){
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    glm::vec3 test = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::scale(trans, glm::vec3(2.0f, 1.0f, 4.0f));
    qDebug() << (trans*glm::vec4(test, 1.0f)).x;

    struct triangulateio in, out;
    in.numberofpoints = 4;
    in.numberofpointattributes = 0;
    in.pointmarkerlist = NULL;
    in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
    in.pointlist[0] = 0.0;
    in.pointlist[1] = 0.0;
    in.pointlist[2] = 1.0;
    in.pointlist[3] = 0.0;
    in.pointlist[4] = 1.0;
    in.pointlist[5] = 10.0;
    in.pointlist[6] = 0.0;
    in.pointlist[7] = 10.0;
    in.numberofsegments = 4;
    in.segmentmarkerlist = NULL;
    in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
    in.segmentlist[0] = 0;
    in.segmentlist[1] = 1;
    in.segmentlist[2] = 1;
    in.segmentlist[3] = 2;
    in.segmentlist[4] = 2;
    in.segmentlist[5] = 3;
    in.segmentlist[6] = 3;
    in.segmentlist[7] = 0;
    in.numberofholes = 0;
    in.holelist = NULL;
    in.numberofregions = 0;
    in.regionlist = NULL;
    // need set of vertices, segments
    // eventually, see which triangles border edge, on which side, etc...
    // -p = planar straight line graph
    // -z = number from zero
    // -V = verbose
    out.pointlist = NULL;
    out.pointmarkerlist = NULL;
    out.trianglelist = NULL;
    out.segmentlist = NULL;
    out.segmentmarkerlist = NULL;
    triangulate((char*)"pzV", &in, &out, NULL);
}

Canvas::~Canvas(){}

void Canvas::initializeGL(){
    initializeOpenGLFunctions();
}

void Canvas::resizeGL(int, int){}

void Canvas::paintGL(){
    glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
