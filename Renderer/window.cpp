#ifndef UNICODE
#define UNICODE
#endif 

#include "window.h"

using mat = matrix<realnum>;
using vec = R3::elem;
using vertex = linked_node<vec>;

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
        1280,
        720,
        NULL,
        NULL)
        ) 
    {
        return 0;
    }

    RenderWindow* pwin = &win;
    HWND hwnd = win.Window();

    ShowWindow(win.Window(), nCmdShow);
    GetClipCursor(&win.c_clip_old);

    //get dimensions of client window
    CLIENT_WIDTH = win.CLIENT_WIDTH;
    CLIENT_HEIGHT = win.CLIENT_HEIGHT;
    pt CLIENT_CENTER = win.CLIENT_CENTER;

    //object queue
    vector<obj_3d*> objects;

    //Initializing a bunch of stuff for the screen
    vec center = win.screen.get_center();
    
    //vector stuff
    const vec zero = vec::zero(3, 1);
    const vec e[3] = { vec::std_basis(3,0) , vec::std_basis(3,1), vec::std_basis(3,2) };
    const mat proj_xy = {
            {1,0,0},
            {0,1,0},
            {0,0,0} };

    //shapes

    //cubes
    double cube_size = 20;

    cube cube1(cube_size,{0,0,0});

    int s_size = 2;
    //surface
    surface s1(s_size, 10); s1.color = 0x7060C0; s1.set_pos({ 0,0,0 });
    surface s2(10, 10); s2.color = 0xFFFF00; s2.set_pos({ 0,0,0 });

    //sphere sph = sphere(10,3,{ 0,0,0 });
    
    objects.push_back(&cube1);
    objects.push_back(&s2);
    //objects.push_back(&s2);
    //objects.push_back(&sph);

    win.cam_speed = 2;

    auto f = [&](double x, double y) {
        return 20*exp(-pow(sin(x),2)-pow(y,2));
    };


    /**************** main run loop *****************/
    bool running = true;
    while (running) {
        //set the program state
        win.MOVMOUSE = false;
        win.SCROLL_DIST = 0;

        // Run the message loop.
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //update shit
        win.update_fields();
        win.update_cam();

        //cube1.transform(win.cam_rotation);

        if (win.TIME_TICKER % 15 == 0) {
        }

        //set up screen with backround 
        win.screen.set_color(0x222222);

        //actual objects
        for (auto obj : objects) {
            win.rframe.draw(*obj);
        }
        double t = win.time();

       win.screen.draw_triangle(
           win.cam.proj(cube1.mesh[0]->data),
           win.cam.proj(cube1.mesh[1]->data),
           win.cam.proj(cube1.mesh[2]->data),
           0x0503F7C
       );
            
        win.draw_screen();

        for (int i = 0; i < 0xFE; i++) {
            pwin->key_presses.at(i) = false;
        }
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
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

/*
* Initializes all fields in the window object.  Basically a constructor.
*/
LRESULT RenderWindow::handle_create() {
    //Initialize screen dimensions
    RECT rect;
    GetClientRect(this->m_hwnd, &rect);
    this->CLIENT_WIDTH = rect.right - rect.left;
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

    this->screen = draw_device(pixel_mem, CLIENT_WIDTH, CLIENT_HEIGHT,10);
    this->cam = camera({ 1,1,-0.8 }, { -50,-50,100 });
    this->rframe = render_frame(this->screen, this->cam);

    this->framerate = 60;
    this->FOV = 90;

    //set up key presses
    for (int i = 0; i < 0xFE; i++) {
        key_presses.insert(pair<u32, bool>(i, false));
    }
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
    return TRUE;
}

inline LRESULT RenderWindow::draw_screen()
{
    //little xy axes
    this->rframe.draw_line(zero, e[0] * 5, RED);
    this->rframe.draw_line(zero, e[1] * 5, GREEN);
    this->rframe.draw_line(zero, e[2] * 5, BLUE);


    //crosshair
    this->screen.draw_line_raw(pt(CLIENT_WIDTH / 2 - 20, CLIENT_HEIGHT / 2), pt(CLIENT_WIDTH / 2 + 20, CLIENT_HEIGHT / 2), 0x33FFFF);
    this->screen.draw_line_raw(pt(CLIENT_WIDTH / 2, CLIENT_HEIGHT / 2 - 20), pt(CLIENT_WIDTH / 2, CLIENT_HEIGHT / 2 + 20), 0x33FFFF);

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

LRESULT RenderWindow::update_cam() {
    
    //rotate camera with mouse
    if (MOVMOUSE) {
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

        double mag_rot = sqrt(dx*dx + dy*dy);

        double horz = dx * 0.001;
        double vert = dy * 0.001;

        cam.rotate(horz, vert);

        if (mag_rot > 1) {
            this->v_horz = 5*horz; this->v_vert = 5*vert;   
        }
        else {
            this->v_horz *= 0.95; this->v_vert *= 0.95;
        }

        this->cam_rotation = cam.cam_rotation(v_horz*0.2, v_vert*0.2);
       
        SetCursorPos(win_rect.left + CLIENT_CENTER.x + 8, win_rect.top + CLIENT_CENTER.y + 32);
        ClipCursor(&c_clip_old);
    }

    // camera movement with wasd + shift and space
    vec cam_pos_new = cam.get_pos();
    vec cam_normal = cam.get_normal();
    vec cam_plane_horz = cam.get_plane()[0];

    double speed = this->cam_speed;

    if (key_presses[VK_SPACE]) {
        cam_pos_new = cam_pos_new + e[2] * speed;
    }
    if (key_presses[VK_SHIFT]) {
        cam_pos_new = cam_pos_new - e[2] * speed;
    }
    if (key_presses[0x57]) {
        cam_pos_new = cam_pos_new + R3::unitize(proj_xy * cam_normal) * speed;
    }
    if (key_presses[0x53]) {
        cam_pos_new = cam_pos_new - R3::unitize(proj_xy * cam_normal) * speed;
    }
    if (key_presses[0x41]) {
        cam_pos_new = cam_pos_new - R3::unitize(proj_xy * cam_plane_horz) * speed;
    }
    if (key_presses[0x44]) {
        cam_pos_new = cam_pos_new + R3::unitize(proj_xy * cam_plane_horz) * speed;
    }
    cam.set_pos(cam_pos_new);

    return FALSE;
}

LRESULT RenderWindow::update_fields()
{
    this->TIME_TICKER += 1;
    this->FOV -= this->SCROLL_DIST / 20;

    //handle FOV changes
    if (this->FOV > 0 && this->FOV < 180) {
        double FOV_rad = (FOV * PI) / 360.0;
        double f_dist_new = static_cast<double>(this->CLIENT_WIDTH)/ (2 * tan(FOV_rad));
        this->cam.set_focus(f_dist_new);
    }
    else {
        this->FOV = 1*(FOV < 0) + 179*(FOV > 0);
    }
    return TRUE;
}
