#pragma once 
#include <windows.h>
#include "camera.h"
#include "linalg.h"
#include "draw_device.h"
#include "vertex_shader.h"
#include "misc.h"
#include "matrix.h"
#include <windowsx.h>
#include <stdint.h>
#include <gdiplus.h>
#include <math.h>
#include <new>
#include <vector>
#include <functional>
#include <math.h>
#include <map>

#define PI 3.14159265358979323846  /* pi */
#define u32 uint32_t


template <class DERIVED_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE* pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow() : m_hwnd(NULL) { }

    BOOL Create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
    )
    {
        WNDCLASS wc = { 0 };

        wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();

        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(
            dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
            nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
        );

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const { return m_hwnd; }

protected:

    virtual PCWSTR  ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;
};

class RenderWindow : public BaseWindow<RenderWindow> {
public:
    RenderWindow() : BaseWindow(){}

    PCWSTR ClassName() const { return L"Renderer"; }

    LRESULT draw_screen();

    LRESULT update_cam();
    LRESULT update_fields();

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT handle_create();
    LRESULT handle_mouse_move(WPARAM wParam, LPARAM lParam);
    LRESULT handle_scroll(WPARAM wParam, LPARAM lParam);
    LRESULT handle_keydown(WPARAM wParam, LPARAM lParam);

    //pixel data for screen
    u32* pixel_mem;
    BITMAPINFO bitmap_info;
    HDC hdc;

    //screen
    draw_device screen;
    vertex_shader rframe;
    bool show_alert;
    
    //camera 
    camera cam;
    double FOV = 90;
    vec vcam_real = { 0,0,0 };
    

    //objectss
    vector<obj_3d*> objects;
    vector<cube*> random_cubes;

    //other fields
    int CLIENT_WIDTH, CLIENT_HEIGHT;
    pt CLIENT_CENTER;

    //some cursor info
    RECT c_clip_old;

    //time
    double framerate;
    long long unsigned int TIME_TICKER = 0;    
    double time() { return static_cast<double>(TIME_TICKER) / framerate; }

    //input fields
    pt MOUSE_POS = pt(0, 0);
    BOOL MOVMOUSE = false;
    int SCROLL_DIST = 0;
    int SCROLL_DIST_ABS = 0;
    std::map<u32, bool> key_presses;
    BOOL SHOWMENU = false;

    //useful things for calculations
    const vec zero = vec::zero(3, 1);
    const vec e[3] = { vec::std_basis(3,0) , vec::std_basis(3,1), vec::std_basis(3,2) };
    const mat proj_xy = {
            {1,0,0},
            {0,1,0},
            {0,0,0} };
    const mat proj_xz = {
            {1,0,0},
            {0,0,0},
            {0,0,1} };
    const mat proj_yz = {
            {0,0,0},
            {0,1,0},
            {0,0,1} };

    //fucking stupid dumb bad stuff
    vector<vec> point_field;
    vector<u32> point_colors;
    double field_size = 0.3;
    
    

    
    
};  
