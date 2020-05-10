#include "canvas.h"

Canvas::Canvas() {
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    installEventFilter(this);
    setMouseTracking(true);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &Canvas::update_file);
}

Canvas::~Canvas(){
    makeCurrent(); // reinitialize OpenGL to correctly free GPU memory in destructors
    delete watcher;
    delete axes;
    delete axes2;
}

void Canvas::initializeGL(){
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    axes = new Axes();
    axes2 = new Axes();
    axes2->location = glm::vec3(0, 0, 100);
}

void Canvas::resizeGL(int width, int height){
    screen_size = glm::vec2(width, height);
}

void Canvas::paintGL(){
    glClearColor(background_color.x, background_color.y, background_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //transform XYZ device space to screen XY
    glm::mat4 view;
    view = glm::ortho(-0.5f*screen_size.x/screen_size.y, 0.5f*screen_size.x/screen_size.y, -0.5f, 0.5f, -100.0f, 100.0f);
    view = glm::rotate(view, glm::radians(240.0f), glm::vec3(0.5773503f, 0.5773503f, 0.5773503f));
    view = glm::rotate(view, glm::radians(90-camera_phi), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-camera_theta), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::scale(view, glm::vec3(camera_zoom, camera_zoom, camera_zoom));
    //view = glm::translate(view, camera_position);

    axes->render(view);
    axes2->render(view);

    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->render(view);
    }


}

// Handle mouse events
bool Canvas::eventFilter(QObject*, QEvent* event){
    if(event->type() == QEvent::Enter){}
    if(event->type() == QEvent::Leave){ camera_orbiting = false; camera_panning = false; }
    if(event->type() == QEvent::MouseButtonPress){
        if(((QMouseEvent*)event)->button()==Qt::LeftButton){ camera_orbiting = true; }
        if(((QMouseEvent*)event)->button()==Qt::RightButton){ camera_panning = true; }
    }
    if(event->type() == QEvent::MouseButtonRelease){
        if(((QMouseEvent*)event)->button()==Qt::LeftButton){ camera_orbiting = false; }
        if(((QMouseEvent*)event)->button()==Qt::RightButton){ camera_panning = false; }
    }
    if(event->type() == QEvent::MouseMove){
        QPoint temppos = ((QMouseEvent*)event)->pos();
        if(camera_orbiting){
            camera_theta -= (((float)temppos.x()) - cursor_position.x);
            camera_phi -= (((float)temppos.y()) - cursor_position.y);
            if(camera_phi < 0) camera_phi = 0;
            if(camera_phi > 180) camera_phi = 180;
            update();
            qDebug() << "Phi:" << camera_phi << "Theta:" << camera_theta;
        }
        if(camera_panning){
            /*
            glm::vec3 camera_vector = glm::vec3(sin(camera_phi)*cos(camera_theta), sin(camera_phi)*sin(camera_theta), cos(camera_phi));
            glm::vec3 up_vector = glm::vec3(-cos(camera_phi)*cos(camera_theta), -cos(camera_phi)*sin(camera_theta), sin(camera_phi));
            glm::vec3 right_vector = glm::cross(camera_vector, up_vector);
            camera_position -= up_vector * (((float)temppos.y()) - cursor_position.y);
            camera_position += right_vector * (((float)temppos.x()) - cursor_position.x);
            update();
            */
        }
        cursor_position = glm::vec2((float)temppos.x(), (float)temppos.y());
    }
    if(event->type() == QEvent::Wheel){
        if(((QWheelEvent*)event)->angleDelta().y()>0){
            camera_zoom *= 1.05;
        }else{
            camera_zoom /= 1.05;
        }
        update();
    }
    return false;
}

bool Canvas::initialize_from_file(QString filepath){
    if(filepath == ""){ return false; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){ return false; }

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
    // TODO: why do is this required?
    if(filepath == this->filepath){
        initialize_from_file(filepath);
    }
}
void Canvas::file_open(){
    //QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "", "*.gdsiiview");
    QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "../example", "*.gdsiiview");
    if(filepath != ""){ initialize_from_file(filepath); }}
void Canvas::file_save(){
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", "", "*.png");
    // need to check whether path is empty and whether ".png" is at end of string
    qDebug() << "save image: " << filepath; }
void Canvas::view_fit(){ qDebug() << "fit"; }
void Canvas::view_perspective(){ qDebug() << "perspective"; }
void Canvas::view_orthographic(){ qDebug() << "ortho"; }
void Canvas::view_orient(glm::vec3 direction, glm::vec3 up){ qDebug() << "orient"; }
