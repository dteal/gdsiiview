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
}

void Canvas::initializeGL(){
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    axes = new Axes();
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
    view = glm::scale(view, glm::vec3(1/camera_zoom, 1/camera_zoom, 1/camera_zoom));
    view = glm::rotate(view, glm::radians(240.0f), glm::vec3(0.5773503f, 0.5773503f, 0.5773503f));
    view = glm::rotate(view, glm::radians(90-camera_phi), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-camera_theta), glm::vec3(0.0f, 0.0f, 1.0f));
    axes->render(view);

    view = glm::translate(view, camera_position);

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
        }
        if(camera_panning){
            glm::vec3 camera_vector = glm::vec3(sin(glm::radians(camera_phi))*cos(glm::radians(camera_theta)),
                                                sin(glm::radians(camera_phi))*sin(glm::radians(camera_theta)),
                                                cos(glm::radians(camera_phi)));
            glm::vec3 up_vector = glm::vec3(-cos(glm::radians(camera_phi))*cos(glm::radians(camera_theta)),
                                            -cos(glm::radians(camera_phi))*sin(glm::radians(camera_theta)),
                                            sin(glm::radians(camera_phi)));
            glm::vec3 right_vector = glm::cross(camera_vector, up_vector);
            camera_position -= up_vector * (((float)temppos.y()) - cursor_position.y)/screen_size.y*camera_zoom;
            camera_position -= right_vector * (((float)temppos.x()) - cursor_position.x)/screen_size.y*camera_zoom;
            update();
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
    // TODO: expect version number in file
    if(filepath == ""){ return false; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){ return false; }

    // reset view when new file opened to avoid surprising behavior
    // (but keep view if only updating current file)
    if(filepath != this->filepath){
        camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
        camera_theta = 45.0f;
        camera_phi = 54.73561f;
    }

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
    if(filepath == this->filepath){ // this function is called when any watched file is changed; make sure it's the right one
        initialize_from_file(filepath);
    }
}
void Canvas::file_open(){
    //QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "", "*.gdsiiview");
    QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "../example", "*.gdsiiview");
    if(filepath != ""){ initialize_from_file(filepath); }}
void Canvas::file_save(){
    QFileDialog save_dialog(this, "Save Image");
    save_dialog.setAcceptMode(QFileDialog::AcceptSave);
    save_dialog.setNameFilter("Images (*.png)"); // display only *.png files
    save_dialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0]); // open in documents folder
    save_dialog.setDefaultSuffix("png"); // force any file extension, and use "*.png" by default
    if(!save_dialog.exec()){ return; } // file dialog cancelled
    QString filepath = save_dialog.selectedFiles().first();
    uint8_t* data = new uint8_t[4*(int)(screen_size.x*screen_size.y)];
    glReadPixels(0, 0, (int)screen_size.x, (int)screen_size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
    QImage image(data, (int)screen_size.x, (int)screen_size.y, QImage::Format_RGBA8888); // might cause a little vs. big endian bug on some architectures?
    image = image.mirrored(false, true); // original OpenGL image was mirrored
    image.save(filepath);
}
void Canvas::view_fit(){ qDebug() << "fit";
    // position object to fit centered in camera view
    // make object fit in 90% of camera view or center object in perpendicular direction
}
void Canvas::view_perspective(){ qDebug() << "perspective"; }
void Canvas::view_orthographic(){ qDebug() << "ortho"; }
void Canvas::view_orient(float theta, float phi){
    camera_theta = theta;
    camera_phi = phi;
    update();
}
