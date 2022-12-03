#ifndef UNICODE
#define UNICODE
#endif 

#include "window.h"

using mat = matrix<realnum>;
using vec = R3::elem;
using vertex = linked_node<vec>;

//root two over two
#define ROOT2 1.41421356237309504880168872420969807856967187537694

static u32 darken(u32 color, double factor) {
    u32 R = 0, G = 0, B = 0;
    for (int i = 0; i < 8; i++) {
        u32 R_mask = (0x00010000 << i) & color;
        u32 G_mask = (0x00000100 << i) & color;
        u32 B_mask = (0x00000001 << i) & color;

        R = R | R_mask >> 16;
        G = G | G_mask >> 8;
        B = B | B_mask;
    }
    R = (int)(static_cast<double>(R) * factor) << 16;
    G = (int)(static_cast<double>(G) * factor) << 8;
    B = (int)(static_cast<double>(B) * factor);
    return R | G | B;
}

int CLIENT_WIDTH;
int CLIENT_HEIGHT;

//Main window
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
   
    RenderWindow win;
    if (!win.Create(
        L"Draw",
        WS_OVERLAPPEDWINDOW,
        0,
        128,
        72,
        1440,
        810,
        NULL,
        NULL)
        ) 
    {
        return 0;
    }

    HWND hwnd = win.Window();
    ShowWindow(hwnd, nCmdShow);
    GetClipCursor(&win.c_clip_old);

    //surface s1(9,20);
    //s1.set_pos({ 0,0,0 });
    //win.rframe.add_mesh(&s1.mesh);

    //sphere sph1(20, 4);
    //win.rframe.add_mesh(&sph1.mesh);

    for (obj_3d* ob : win.objects) {
        win.rframe.add_mesh(&(ob->mesh));
    }
     
    /**************** main run loop *****************/
    bool running = true;
    while (running) {
        win.screen.set_color(0);
        win.MOVMOUSE = false;

        // Run the message loop.
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto f = [&](double x, double y) {
            return 15*exp(-pow(x+sin(win.time()), 2) - pow(y-cos(win.time()), 2));
        };

        for (cube* C : win.random_cubes) {
            C->affine_transform(R3::rotate_intr(PI / 128, 0, 0));
        }

        //sph1.affine_transform(R3::rotate_intr(PI / 128, 0, 0));

        win.LIGHT.pos = { 100*cos(3*win.time()),100*sin(3*win.time()), 200};
        
        //update shit
        win.update_cam();
        win.draw_screen();
        win.update_fields();
        
    }
    
    return 0;
}

//Member functions of RenderWindow class

LRESULT RenderWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
    {
        DestroyWindow(m_hwnd);
        return 0;
    }
    case WM_CREATE:
    {
        this->handle_create();
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        this->handle_mouse_move(wParam,lParam);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        this->handle_scroll(wParam, lParam);
        return 0;
    }
    case WM_KEYDOWN:
    {
        this->handle_keydown(wParam,lParam);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        this->field_size *= 0.94;
    }
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

LRESULT RenderWindow::handle_create() {
    //Initialize screen dimensions
    RECT rect;
    GetClientRect(this->m_hwnd, &rect);
    this->CLIENT_WIDTH = rect.right - rect.left - (rect.right - rect.left)/6;
    this->CLIENT_HEIGHT = rect.bottom - rect.top;
    this->CLIENT_CENTER.x = CLIENT_WIDTH / 2;
    this->CLIENT_CENTER.y = CLIENT_HEIGHT / 2;

    //initialize BITMAPINFO structure
    this->bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    this->bitmap_info.bmiHeader.biWidth = CLIENT_WIDTH;
    this->bitmap_info.bmiHeader.biHeight = CLIENT_HEIGHT;
    this->bitmap_info.bmiHeader.biPlanes = 1;
    this->bitmap_info.bmiHeader.biBitCount = 32;
    this->bitmap_info.bmiHeader.biCompression = BI_RGB;

    //get device context
    this->hdc = GetDC(m_hwnd);

    //set up screen
    void* memory = VirtualAlloc(
        0,
        CLIENT_WIDTH * CLIENT_HEIGHT * sizeof(u32),
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );

    this->pixel_mem = (u32*)memory;

    this->screen = draw_device(pixel_mem, CLIENT_WIDTH, CLIENT_HEIGHT,100);
    this->cam = camera({ 0.5,0.5,-1 }, { -300,-300,50 });
    this->rframe = vertex_shader(this->screen, this->cam);

    this->framerate = 120;
    this->FOV = 90;

    this->LIGHT = light({ 0, 0, 100 }, 20000);
    this->rframe.add_light(&LIGHT);

    //a bunch of shapes because it's cool
    {
        vector<int> bounds = {5,5, 1};
        int cube_size = 30;
    
        for (int i = -bounds[0]; i < bounds[0]; i++) {
            for (int j = -bounds[1]; j < bounds[1]; j++) {
                for (int k = 0; k < bounds[2]; k++) {
                   vec pos = vec({ (double)i, (double)j, (double)k }) * cube_size;
                   double yes = (double)(rand() % 100) / 100;
    
                   if (yes > 0.5) {
                       cube* C = new cube(cube_size,pos);
                       random_cubes.push_back(C);
                       this->objects.push_back(C);
                   }
                }        
            }
        }
    }

    //stupid electron stuff
    {
        int size = 10;
        double d = 0.7;
        int res = size / d;

        auto cosine = [&](vec v, vec w) {
            return R3::ip(v, w) / (R3::norm(v) * R3::norm(w));
        };
        auto elec_dist = [&](vec v) {
            auto vmag = R3::norm(v);

            auto a1 = cosine({ 0,1,0 }, v);
            auto a2 = cosine({ 1,0,0 }, v);
            auto a3 = cosine({ 0,0,1 }, v);
            //auto a3 = cosine({ 1,1,0 }, v);
            auto a4 = cosine({ 1,-1,0 }, v);

            return 3 * (
                //exp(-(vmag)*field_size) +
                exp(-(vmag)*field_size) * a1 +
                exp(-(vmag)*field_size) * a2 +
                exp(-(vmag)*field_size) * a3);
        };

        for (int n = -res; n < res; n++) {
            for (int i = -res; i < res; i++) {
                for (int j = -res; j < res; j++) {
                    double x = rand();
                    double y = rand();
                    double z = rand();

                    vec point = { x,y,z };
                    point_field.push_back(point);
                    auto brightness = elec_dist(point);
                    bool neg = brightness < 0;
                    u32 color = darken(0x0000FF * neg + 0xFF0000 * !neg, abs(brightness));
                    point_colors.push_back(color);
                }
            }
        }
    }
    
    //set up key presses
    for (int i = 0; i < 0xFE; i++) {
        key_presses.insert(pair<u32, bool>(i, false));
    }
    return TRUE;
}

LRESULT RenderWindow::draw_screen()
{
    //set up screen with backround 
    
    this->rframe.process_meshes();

    //for (int i = 0; i < point_field.size(); i++) {
    //    screen.draw_circ(cam.proj(point_field[i]), 2, point_colors[i]);
    //}

    //little xy axes
    this->rframe.draw_line(zero, e[0], RED);
    this->rframe.draw_line(zero, e[1], GREEN);
    this->rframe.draw_line(zero, e[2], BLUE);

    //crosshair
    this->screen.draw_line_raw(pt(CLIENT_WIDTH / 2 - 20, CLIENT_HEIGHT / 2), pt(CLIENT_WIDTH / 2 + 20, CLIENT_HEIGHT / 2), 0x33FFFF);
    this->screen.draw_line_raw(pt(CLIENT_WIDTH / 2, CLIENT_HEIGHT / 2 - 20), pt(CLIENT_WIDTH / 2, CLIENT_HEIGHT / 2 + 20), 0x33FFFF);

    //alert 
    if (show_alert) {
        this->screen.draw_circ_raw(100, 100, 50, 0xFF0000);
    }

    StretchDIBits(
        this->hdc,
        0,
        0,
        CLIENT_WIDTH,
        CLIENT_HEIGHT,
        0,
        0,
        CLIENT_WIDTH,
        CLIENT_HEIGHT,
        (const void*)(this->pixel_mem),
        &(this->bitmap_info),
        DIB_RGB_COLORS,
        SRCCOPY
    );

    return TRUE;
}

LRESULT RenderWindow::handle_mouse_move(WPARAM wParam, LPARAM lParam){
    this->MOUSE_POS.x = GET_X_LPARAM(lParam);
    this->MOUSE_POS.y = abs(GET_Y_LPARAM(lParam) - CLIENT_HEIGHT);
    this->MOVMOUSE = true;
    return TRUE;
}

LRESULT RenderWindow::handle_scroll(WPARAM wParam, LPARAM lParam){
    int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    this->SCROLL_DIST += zDelta;
    return TRUE;
}

LRESULT RenderWindow::handle_keydown(WPARAM wParam, LPARAM lParam){
    this->key_presses.at((int)wParam) = true;

    switch ((int)wParam) {
         case 27:
         {
             this->SHOWMENU = ~this->SHOWMENU;
         } 
    }

    return TRUE;
}

/*
* This mostly handles the motion of the camera.  Eventually I will separate the 
* motion from this.
*/
LRESULT RenderWindow::update_cam() {
    //rotate camera with mouse
    if (SHOWMENU) {
        HCURSOR c = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
        SetCursor(c);
        ShowCursor(true);
    }
    if (MOVMOUSE && !this->SHOWMENU) {
        ShowCursor(false);
        RECT win_rect;
        GetWindowRect(m_hwnd, &win_rect);

        RECT c_clip;
        {
            c_clip.top = win_rect.top + 16;
            c_clip.bottom = win_rect.bottom - 16;
            c_clip.left = win_rect.left + 16;
            c_clip.right = win_rect.right - 16;
        }

        ClipCursor(&c_clip);

        int dx = MOUSE_POS.x - CLIENT_CENTER.x;
        int dy = MOUSE_POS.y - CLIENT_CENTER.y;

        double mag_rot = sqrt(dx * dx + dy * dy);

        vec normal = cam.get_normal();

        double cam_limit = 0.1;

        double cosine = R3::ip(normal, e[2])/R3::norm(normal);

        double horz = dx * 0.001;
        double vert = dy * 0.001;

        if (cosine < -cos(cam_limit)) {
            vec n = R3::unitize(proj_xy * normal) - e[2] * tan(PI / 2 - cam_limit);
            cam.set_facing(n);
        }
        else if (cosine > cos(cam_limit)) {
            vec n = R3::unitize(proj_xy * normal) + e[2] * tan(PI / 2 - cam_limit);
            cam.set_facing(n);
        }
        else {
            cam.rotate(horz, vert);
        }

       
        /* This bit basically allows you to spin shapes in the direction of mouse movement
        * cam_rotation is the matrix which spins the shapes.
        *
        if (mag_rot > 1) {
            this->v_horz = 5 * horz; this->v_vert = 5 * vert;
        }
        else {
            this->v_horz *= 0.95; this->v_vert *= 0.95;
        }
        this->cam_rotation = cam.cam_rotation(v_horz * 0.2, v_vert * 0.2);

        PUT THESE IN window.h: 
        double v_horz = 0;
        double v_vert = 0; 
        mat cam_rotation;
        
        */

        SetCursorPos(win_rect.left + CLIENT_CENTER.x + 8, win_rect.top + CLIENT_CENTER.y + 32);
        ClipCursor(&c_clip_old);
    }

    // camera movement with wasd + shift and space
    double speed = 0.4;
    double cam_height = 10;

    vec vcam_relative = { 0,0,0 };
    vec pos = cam.get_focal_point() -e[2] * cam_height;
    vec new_pos = pos + vcam_real;
    vcam_real = { vcam_real[0][0], vcam_real[1][0],0 };

    //change of basis matrix from camera-relative coordinates to standard basis coordinates
    mat CoB = mat(vector<mat>({ R3::unitize(proj_xy * cam.get_normal()), R3::unitize(proj_xy * cam.get_plane()[0]), {0,0,1} }));

    //colliding if new_pos is inside the cube
    bool colliding = false;
    auto handle_collision = [&](cube* C,vec P) {
            vec O = C->get_pos();
            vec OP = P - O;

            double l_xy = R3::norm(proj_xy * OP);
            double l_xz = R3::norm(proj_xy * OP);
            double l_yz = R3::norm(proj_yz * OP);   

            double cosx = OP[0][0] / l_xy;
            double cosy = OP[1][0] / l_xy;
            bool in_x_quadrant = (cosx >= ROOT2 / 2) + (cosx <= -ROOT2 / 2);
            bool in_y_quadrant = (cosy >= ROOT2 / 2) + (cosy <= -ROOT2 / 2);

            double cosz = (OP[2][0] / l_xz) * in_x_quadrant + (OP[2][0] / l_yz) * in_y_quadrant;
            bool is_vertical = (cosz >= ROOT2 / 2) + (cosz <= -ROOT2 / 2);

            vec direction = {
                (double)in_x_quadrant * !is_vertical,
                (double)in_y_quadrant * !is_vertical,
                (double)is_vertical
            };

            mat signs = {
                {(double)sign(cosx),0,0},
                {0,(double)sign(cosy),0},
                {0,0,(double)sign(cosz)}
            };
            
            vec plane_normal = signs * direction;
            if (direction == vec({ 0, 0, 0 })) {
                plane_normal = { 0,0,1 };
            }
            vec plane_pos = O + plane_normal * (C->get_side_length() / 2);

            vcam_real = vcam_real - plane_normal * R3::ip(vcam_real, plane_normal)*1.7;
            vec temp = new_pos - plane_pos;

            this->rframe.draw_line(O, plane_pos,0x0000FF);

            return plane_pos + temp - plane_normal * R3::ip(temp, plane_normal);
        };
    


    //NEED TO FIX THIS AND MAKE COLLISION WORK PROPERLY FOR NON-POINT OBJECTS
    //check collision for each cube.
    for (cube* c : this->random_cubes) {        
        if (c->is_inside(new_pos)) {
            new_pos = handle_collision(c, new_pos);
            colliding = true;
        }      
    }

    //use wasd to control motion in xy-plane
    if (key_presses[0x57]) {
        vcam_relative = vcam_relative + e[0] * speed;
    }
    if (key_presses[0x53]) {
        vcam_relative = vcam_relative + e[0] * -speed;
    }
    if (key_presses[0x41]) {
        vcam_relative = vcam_relative + e[1] * -speed;
    }
    if (key_presses[0x44]) {
        vcam_relative = vcam_relative + e[1] * speed;
    }

    vcam_real = vcam_real + CoB * vcam_relative;
    if (key_presses[VK_SPACE]) {
        vcam_real = { vcam_real[0][0], vcam_real[1][0],1 };
    }
    //handle camera motion upon collision
    if (colliding) { 
        show_alert = true; 
        vcam_real = mat({ {0.9,0,0}, {0,0.9,0}, {0,0,1} }) * vcam_real;
    }
    else if (vcam_real[2][0] > -2) {
        //vcam_real = vcam_real - vec({ 0, 0, 0.01 });
    }
    vcam_real = mat({ {0.9,0,0}, {0,0.9,0}, {0,0,1} }) * vcam_real;
    
    //resets camera position to origin if user falls too far
    if (pos[2][0] < -100) {
        cam.set_pos({0,0,40});
        vcam_real = { 0,0,0 };
    }
    else {
        cam.set_pos(new_pos + e[2] * cam_height);
    }

    
    
    return FALSE;
}

LRESULT RenderWindow::update_fields()
{
    this->TIME_TICKER += 1;
    this->FOV -= this->SCROLL_DIST / 100;

    //handle FOV changes
    if (this->FOV > 0 && this->FOV < 180) {
        double FOV_rad = (FOV * PI) / 360.0;
        double f_dist_new = static_cast<double>(this->CLIENT_WIDTH)/ (2 * tan(FOV_rad));
        this->cam.set_focus(f_dist_new);
    }
    else {
        this->FOV = 1*(FOV < 0) + 179*(FOV > 0);
    }

    if (TIME_TICKER % 120 == 0) {
        show_alert = false;
    }

    this->SCROLL_DIST = 0;
    for (int i = 0; i < 0xFE; i++) {
        key_presses.at(i) = false;
    }
    return TRUE;
}
