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
    // TODO: this stops registering watches after ~5-20 file changes on Windows?!
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
    glm::mat4 rotate = glm::mat4(1.0f);
    rotate = glm::rotate(rotate, glm::radians(240.0f), glm::vec3(0.5773503f, 0.5773503f, 0.5773503f));
    rotate = glm::rotate(rotate, glm::radians(90-camera_phi), glm::vec3(0.0f, 1.0f, 0.0f));
    rotate = glm::rotate(rotate, glm::radians(-camera_theta), glm::vec3(0.0f, 0.0f, 1.0f));
    view = view*rotate;
    if(show_axes){
        axes->render(view);
    }
    view = glm::scale(view, glm::vec3(1/camera_zoom, 1/camera_zoom, 1/camera_zoom));
    view = glm::translate(view, camera_position);

    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->render(view, rotate);
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

void Canvas::emit_initialization_error(QString error){
    QMessageBox::warning(this, "File Reading Error", error);
}

bool Canvas::initialize_from_file(QString filepath){
    if(filepath == ""){ return false; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){
        // gets confused when reloading file on Windows?
        //emit_initialization_error("File not found: \""+filepath+"\".");
        return false;
    }
    // reset view when new file opened to avoid surprising behavior
    // (but keep view if only updating current file)
    bool reset_view = (filepath != this->filepath);

    this->filepath = filepath;
    watcher->files().clear();
    watcher->addPath(filepath);

    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->deinitialize();
    }
    parts.clear();

    std::shared_ptr<Part>temppart = std::shared_ptr<Part>(new Part());
    std::shared_ptr<Mesh>tempmesh = std::shared_ptr<Mesh>(new Mesh());
    std::shared_ptr<Image>tempimage = std::shared_ptr<Image>(new Image());

    QString relativepath = QFileInfo(filepath).absolutePath();
    std::ifstream infile(filepath.toStdString());
    std::string line;

    int linenumber = 0;
    // parse configuration file line by line
    while(std::getline(infile, line)){
        // break each line into whitespace-separated words
        linenumber += 1;
        try{
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
        if(commands[0] == "gdsii:" || commands[0] == "image:"){
            if(temppart->created){
                if(temppart->type == Part::PART_GDSII && tempmesh->created){
                    temppart->meshes.push_back(tempmesh);
                    tempmesh = std::shared_ptr<Mesh>(new Mesh());
                }
                if(temppart->type == Part::PART_IMAGE){
                    temppart->image = tempimage;
                    tempimage = std::shared_ptr<Image>(new Image());
                }
                parts.push_back(temppart);
            }
            temppart = std::shared_ptr<Part>(new Part());
            temppart->filepath = QDir(relativepath).filePath(QString(commands[1].c_str()));
            watcher->addPath(temppart->filepath);
            if(commands[0] == "gdsii:"){ temppart->type = Part::PART_GDSII; }
            if(commands[0] == "image:"){ temppart->type = Part::PART_IMAGE; }
            temppart->created = true;
        }else if(commands[0] == "comment:"){ // ignore comments
        }else if(commands[0][0] == '#'){ // these are also comments
        }else if(commands[0] == "transform:"){ // really just a placeholder; ignore this line too
        }else if(commands[0] == "geometry:"){ // same
        }else if(commands[0] == "hidden:"){
            if(commands[1]=="true"){ temppart->hidden = true; }
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
                default: emit_initialization_error(QString("Unknown axis in configuration file at line %1.").arg(linenumber)); return false;
            }
            temppart->transform = glm::rotate(glm::mat4(1.0f), glm::radians(std::stof(commands[2])), axis)*temppart->transform;
            temppart->rotate = glm::rotate(glm::mat4(1.0f), glm::radians(std::stof(commands[2])), axis)*temppart->rotate;
        }else if(commands[0] == "translate:"){
            temppart->transform = glm::translate(glm::mat4(1.0f), glm::vec3(std::stof(commands[1]), std::stof(commands[2]), std::stof(commands[3]))) * temppart->transform;
        }else if(commands[0] == "background:"){
            background_color = glm::vec3(std::stoi(commands[1])/255.0f, std::stoi(commands[2])/255.0f, std::stoi(commands[3])/255.0f);
        }else if(commands[0] == "color:"){
            if(temppart->type == Part::PART_GDSII){
                tempmesh->color = glm::vec3(std::stoi(commands[1])/255.0f, std::stoi(commands[2])/255.0f, std::stoi(commands[3])/255.0f);
            }else if(temppart->type == Part::PART_IMAGE){
                tempimage->color = glm::vec3(std::stoi(commands[1])/255.0f, std::stoi(commands[2])/255.0f, std::stoi(commands[3])/255.0f);
            }
        }else if(commands[0] == "zbounds:"){
            if(temppart->type == Part::PART_GDSII){
                tempmesh->zbounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
            }else if(temppart->type == Part::PART_IMAGE){
                tempimage->zbounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
            }
        }else if(commands[0] == "xbounds:"){
            tempimage->xbounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
        }else if(commands[0] == "ybounds:"){
            tempimage->ybounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
        }else if(commands[0] == "mirror:"){
            if(commands[1] == "x"){ tempimage->mirror_horizontal = true; }
            if(commands[1] == "y"){ tempimage->mirror_vertical = true; }
        }else if(commands[0] == "stl:"){
            tempmesh->export_stl = true;
            temppart->stlfilepath = QDir(relativepath).filePath(QString(commands[1].c_str()));
        }else{
            emit_initialization_error(QString("Could not parse configuration file at line %1.").arg(linenumber));
            return false;
        }
        }catch(QException e){
            emit_initialization_error(QString("Unknown error on line %1.").arg(linenumber));
            return false;
        }
    }
    // store last remaining part and/or mesh
    if(temppart->created){
        if(temppart->type == Part::PART_GDSII && tempmesh->created){
            temppart->meshes.push_back(tempmesh);
        }
        if(temppart->type == Part::PART_IMAGE){
            temppart->image = tempimage;
        }
        parts.push_back(temppart);
    }

    // initialize with OpenGL context
    makeCurrent();
    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->initialize();
    }

    // redraw


    if(reset_view){
        camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
        camera_theta = 45.0f;
        camera_phi = 54.73561f;
        view_fit(); // includes update
    }else{
        update();
    }

    return true;
}

void Canvas::update_file(QString filepath){
    // reload when the entire *.gdsiiview file or any *.gds or image file is changed
    // TODO: add fine-grained reloading
    //if(filepath == this->filepath){ // this function is called when any watched file is changed; make sure it's the right one
        initialize_from_file(this->filepath);
    //}
}
void Canvas::file_open(){
    //QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File", "", "*.gdsiiview");
    QString filepath = QFileDialog::getOpenFileName(this, "Open *.gdsiiview File",QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0], "*.gdsiiview");
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

void Canvas::center_model_origin(){
    this->camera_position = glm::vec3(0,0,0);
    update();
}

void Canvas::toggle_axes(){
    show_axes = !show_axes;
    update();
}

void Canvas::view_fit(){
    // first, get current model bounds
    glm::mat4 view;
    view = glm::ortho(-0.5f*screen_size.x/screen_size.y, 0.5f*screen_size.x/screen_size.y, -0.5f, 0.5f, -100.0f, 100.0f);
    view = glm::scale(view, glm::vec3(1/camera_zoom, 1/camera_zoom, 1/camera_zoom));
    view = glm::rotate(view, glm::radians(240.0f), glm::vec3(0.5773503f, 0.5773503f, 0.5773503f));
    view = glm::rotate(view, glm::radians(90-camera_phi), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-camera_theta), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::translate(view, camera_position);
    glm::vec4 bounds = glm::vec4(std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest());
    for(unsigned int i=0; i<parts.size(); i++){
        if(!parts[i]->hidden){
        glm::vec4 partbounds = parts[i]->get_bounds(view);
        if(partbounds[0] < bounds[0]) bounds[0] = partbounds[0];
        if(partbounds[1] > bounds[1]) bounds[1] = partbounds[1];
        if(partbounds[2] < bounds[2]) bounds[2] = partbounds[2];
        if(partbounds[3] > bounds[3]) bounds[3] = partbounds[3];
        }
    }

    // first, center the camera
    float x_pan_delta = (bounds[1]+bounds[0])/2; // amount to move vs [-1,1] window size
    float y_pan_delta = (bounds[3]+bounds[2])/2; // amount to move vs [-1,1] window size
    glm::vec3 camera_vector = glm::vec3(sin(glm::radians(camera_phi))*cos(glm::radians(camera_theta)),
                                        sin(glm::radians(camera_phi))*sin(glm::radians(camera_theta)),
                                        cos(glm::radians(camera_phi)));
    glm::vec3 up_vector = glm::vec3(-cos(glm::radians(camera_phi))*cos(glm::radians(camera_theta)),
                                    -cos(glm::radians(camera_phi))*sin(glm::radians(camera_theta)),
                                    sin(glm::radians(camera_phi)));
    glm::vec3 right_vector = glm::cross(camera_vector, up_vector);
    //camera_position -= up_vector * (((float)temppos.y()) - cursor_position.y)/screen_size.y*camera_zoom;
    //camera_position -= right_vector * (((float)temppos.x()) - cursor_position.x)/screen_size.y*camera_zoom;
    camera_position += right_vector * x_pan_delta*0.5f*screen_size.x/screen_size.y*camera_zoom;
    camera_position -= up_vector * y_pan_delta*0.5f*camera_zoom;

    // next, figure out how much to adjust the camera zoom by
    float fit_size = 0.9f; // fill this much of the window
    //float camera_zoom = 500.0f; // model display size (model units per window height)
    float x_zoom_delta = (bounds[1]-bounds[0])/(2*fit_size);
    float y_zoom_delta = (bounds[3]-bounds[2])/(2*fit_size);
    camera_zoom *= std::max(x_zoom_delta, y_zoom_delta);

    // and update render
    update();
}
void Canvas::view_orient(float theta, float phi){
    camera_theta = theta;
    camera_phi = phi;
    update();
}
