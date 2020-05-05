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

    installEventFilter(this);
    setMouseTracking(true);
}

Canvas::~Canvas(){}

void Canvas::initializeGL(){
    initializeOpenGLFunctions();

    //if(argc < 2 || !parse_configuration(argv[1])){
    //    std::cout << "Usage: gdsiiview example.gdsiiview" << std::endl;
    //    return 0;
    //}

    // initialize all parts/meshes (e.g., open files, tesselate...)
    // after OpenGL context is available
    /*
    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->initialize();
    }
    */

    // change screen size once for initial frame
    //int width, height;
    //glfwGetWindowSize(window, &width, &height);
    //glViewport(0, 0, width, height);
    //screen_size = glm::vec2(width, height);
}

void Canvas::resizeGL(int width, int height){
    screen_size = glm::vec2(width, height);
}

void Canvas::paintGL(){
    glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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
        }
        cursor_position = glm::vec2((float)temppos.x(), (float)temppos.y());}
    if(event->type() == QEvent::Wheel){
        if(((QWheelEvent*)event)->angleDelta().y()>0){
            zoom *= 1.05;
        }else{
            zoom /= 1.05;
        }
    }
    return false;
}

bool Canvas::parse_configuration(std::string filepath){
    /*
    if(!boost::filesystem::exists(filepath) || boost::filesystem::is_directory(filepath)){
        std::cout << "Error: filepath \"" << filepath << "\" not valid" << std::endl; return false;
    }
    config_filepath = filepath;
    config_write_time = boost::filesystem::last_write_time(filepath);
    boost::filesystem::path relativepath(filepath);
    relativepath = relativepath.parent_path();
    std::ifstream infile(filepath);
    std::string line;
    std::shared_ptr<Part>temppart = std::shared_ptr<Part>(new Part());
    std::shared_ptr<Mesh>tempmesh = std::shared_ptr<Mesh>(new Mesh());
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
            temppart->filepath = (relativepath / commands[1]).c_str();
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
            tempmesh->stlfilepath = (relativepath / commands[1]).c_str();
        }else{
            std::cout << "Error: could not parse configuration file line " << linenumber << std::endl;
        }
    }
    // store last remaining part and/or mesh
    if(temppart->created){
        if(tempmesh->created){ temppart->meshes.push_back(tempmesh); }
        parts.push_back(temppart);
    }
    */
    return true;
}
