#include "canvas.h"

Canvas::Canvas() : VBO(QOpenGLBuffer::VertexBuffer) {
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
    //triangulate((char*)"pzV", &in, &out, NULL);

    installEventFilter(this);
    setMouseTracking(true);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &Canvas::update_file);
}

Canvas::~Canvas(){
    delete watcher;
}

void Canvas::initializeGL(){
    initializeOpenGLFunctions();

        VAO.create();
        VBO.create();
        VAO.bind();
        VBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
        VBO.bind();
        float points[]{
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };
        VBO.allocate(points, sizeof(float)*9);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        VAO.release();

        /*
    QOpenGLShader vertex_shader(QOpenGLShader::Vertex);
    vertex_shader.compileSourceCode(vertex_source);
    qDebug() << vertex_shader.log();
    QOpenGLShader fragment_shader(QOpenGLShader::Fragment);
    fragment_shader.compileSourceCode(fragment_source);
    qDebug() << fragment_shader.log();
    shader.addShader(&vertex_shader);
    shader.addShader(&fragment_shader);
    *
    */
    shader.addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
    shader.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
    shader.link();
    qDebug() << shader.log();
        //VAO.release();

}

void Canvas::resizeGL(int width, int height){
    screen_size = glm::vec2(width, height);
    qDebug() << width << " " << height;
}

void Canvas::paintGL(){
    glClearColor(background_color.x, background_color.y, background_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //transform XYZ device space to screen XY
    glm::mat4 view;
    view = glm::ortho(-0.5f*screen_size.x/screen_size.y, 0.5f*screen_size.x/screen_size.y, -0.5f, 0.5f, -100.0f, 100.0f);
    view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // camera rotate and zoom
    view = glm::scale(view, glm::vec3(zoom, zoom, zoom));
    view = glm::rotate(view, glm::radians(phi), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));

    for(unsigned int i=0; i<parts.size(); i++){
        qDebug() << "rendering part " << i;
        parts[i]->render(view);
    }

    shader.bind();
    VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    VAO.release();

}

// Handle mouse events
bool Canvas::eventFilter(QObject*, QEvent* event){
    if(event->type() == QEvent::Enter){}
    if(event->type() == QEvent::Leave){ orbiting = false; }
    if(event->type() == QEvent::MouseButtonPress){
        if(((QMouseEvent*)event)->button()==Qt::LeftButton){ orbiting = true; }
    }
    if(event->type() == QEvent::MouseButtonRelease){
        if(((QMouseEvent*)event)->button()==Qt::LeftButton){ orbiting = false; }
    }
    if(event->type() == QEvent::MouseMove){
        QPoint temppos = ((QMouseEvent*)event)->pos();
        if(orbiting){
            theta += (((float)temppos.x()) - cursor_position.x);
            phi += (((float)temppos.y()) - cursor_position.y);
            update();
        }
        cursor_position = glm::vec2((float)temppos.x(), (float)temppos.y());
    }
    if(event->type() == QEvent::Wheel){
        if(((QWheelEvent*)event)->angleDelta().y()>0){
            zoom *= 1.05;
        }else{
            zoom /= 1.05;
        }
        update();
    }
    return false;
}

bool Canvas::initialize_from_file(QString filepath){
    if(filepath == ""){ return false; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){ return false; }

    qDebug() << "open: " << filepath;
    this->filepath = filepath;
    watcher->addPath(filepath);
    watcher->files().removeDuplicates();

    // TODO: need to delete previous parts

    std::shared_ptr<Part>temppart = std::shared_ptr<Part>(new Part());
    std::shared_ptr<Mesh>tempmesh = std::shared_ptr<Mesh>(new Mesh());

    QString relativepath = QFileInfo(filepath).absolutePath();
    std::ifstream infile(filepath.toStdString());
    std::string line;

    int linenumber = 0;
    // parse configuration file line by line
    while(std::getline(infile, line)){
        // break each line into whitespace-separated words
        linenumber += 1;
        line = line + " "; // add whitespace to flush last word in line...
        std::vector<std::string>commands;
        std::stringstream ss;
        bool quoted = false;
        for(unsigned int i=0; i<line.size(); i++){
            switch(line[i]){
                case '"': quoted = !quoted; break;
                case '\n':
                case '\r':
                case ' ':
                case '\t': // ...here.
                    if(!quoted){
                        if(ss.str().size()>0){
                            commands.push_back(ss.str());
                            ss.str("");
                        }
                    }else{ ss << line[i]; } // quoted strings keep whitespace
                    break;
                default:
                    ss << line[i];
            }
        }
        if(commands.size() == 0){continue;} // tolerate blank lines
        if(commands[0][0] == '#'){continue;} // comments
        // apply commands
        if(commands[0] == "gdsii:"){
            if(temppart->created){
                if(tempmesh->created){ temppart->meshes.push_back(tempmesh);
                                       tempmesh = std::shared_ptr<Mesh>(new Mesh()); }
                parts.push_back(temppart);
            }
            temppart = std::shared_ptr<Part>(new Part());
            temppart->filepath = QDir(relativepath).filePath(QString(commands[1].c_str()));
            temppart->created = true;
        }else if(commands[0] == "comment:"){
        }else if(commands[0] == "transform:"){
        }else if(commands[0] == "layer:"){
            if(tempmesh->created){ temppart->meshes.push_back(tempmesh); }
            tempmesh = std::shared_ptr<Mesh>(new Mesh());
            tempmesh->gdslayer = std::stoi(commands[1]);
            tempmesh->created = true;
        }else if(commands[0] == "rotate:"){
            glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f);
            switch(commands[1][0]){
                case 'x': axis = glm::vec3(1.0f, 0.0f, 0.0f); break;
                case 'y': axis = glm::vec3(0.0f, 1.0f, 0.0f); break;
                case 'z': axis = glm::vec3(0.0f, 0.0f, 1.0f); break;
                default: std::cout << "Error: unknown axis in configuration file in line " << linenumber << std::endl;
            }
            temppart->transform = glm::rotate(glm::mat4(1.0f), glm::radians(std::stof(commands[2])), axis)*temppart->transform;
        }else if(commands[0] == "translate:"){
            temppart->transform = glm::translate(glm::mat4(1.0f), glm::vec3(std::stof(commands[1]), std::stof(commands[2]), std::stof(commands[3]))) * temppart->transform;
        }else if(commands[0] == "color:"){
            tempmesh->color = glm::vec3(std::stoi(commands[1])/255.0f, std::stoi(commands[2])/255.0f, std::stoi(commands[3])/255.0f);
        }else if(commands[0] == "zbounds:"){
            tempmesh->zbounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
        }else if(commands[0] == "stl:"){
            tempmesh->export_stl = true;
            temppart->stlfilepath = QDir(relativepath).filePath(QString(commands[1].c_str()));
        }else{
            std::cout << "Error: could not parse configuration file line " << linenumber << std::endl;
        }
    }
    // store last remaining part and/or mesh
    if(temppart->created){
        if(tempmesh->created){ temppart->meshes.push_back(tempmesh); }
        parts.push_back(temppart);
    }

    makeCurrent();
    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->initialize();
    }
    update();
    return true;
}

void Canvas::update_file(QString filepath){
    if(filepath == this->filepath){
        initialize_from_file(filepath);
    }
}
void Canvas::file_open(){
    QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "", "*.gdsiiview");
    if(filepath != ""){ initialize_from_file(filepath); }}
void Canvas::file_save(){
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", "", "*.png");
    // need to check whether path is empty and whether ".png" is at end of string
    qDebug() << "save image: " << filepath; }
void Canvas::view_fit(){ qDebug() << "fit"; }
void Canvas::view_perspective(){ qDebug() << "perspective"; }
void Canvas::view_orthographic(){ qDebug() << "ortho"; }
void Canvas::view_orient(glm::vec3 direction, glm::vec3 up){ qDebug() << "orient"; }
