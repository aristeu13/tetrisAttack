#include "tetris.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*, GLuint tela_2d[], Selector selector, Pecas pecas);
void DisableOpenGL(HWND, HDC, HGLRC);
void reiniciar();

tela = 0;
movLaterais = 0;
movVerticais = 0;
selectd = 0;

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    GLuint tela_2d[3];
    Selector selector;
    selector = init_selector();
    Pecas pecas;
    pecas = init_pecas();
    float vel = 0.0002;
    srand(time(NULL));

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1024,
                          800,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC, tela_2d, selector, pecas);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if(tela == 1){
                trocar(pecas,selector);
                desenhaPecas(pecas, vel);
                gravidade(pecas);
                verifica(pecas);
                movimentaSelector(selector, pecas, vel);
                desenhaSelector(selector);
            }
            renderiza(1,-1,1,-1,tela_2d[tela]);
            if(tela == 1) desenhaPontos(pecas,&vel);

            if(tela == -1){
                movLaterais = 0;
                movVerticais = 0;
                selectd = 0;
                
                selector->posX = -0.068;
                selector->posY = 0.2;
                selector->height = 0.2;
                selector->width = 0.13;

                int i,j;
                for(i=0;i<10;i++){
                    for(j=0;j<6;j++){
                        if(i > 6) pecas->tipo[i][j] = -1;
                        else pecas->tipo[i][j] = (rand() % 4);
                        while ((j > 1 && pecas->tipo[i][j] == pecas->tipo[i][j-1] && pecas->tipo[i][j] == pecas->tipo[i][j-2] && pecas->tipo[i][j] != -1) || (i >= 2 && pecas->tipo[i][j] == pecas->tipo[i-1][j] && pecas->tipo[i][j] == pecas->tipo[i-2][j] && pecas->tipo[i][j] != -1)){
                            pecas->tipo[i][j] = (rand() % 4);
                        }
                    }
                }
                pecas->vel = 0;
                pecas->pontos = 0;
                vel = 0.0002;
                tela = 1;
                srand(time(NULL));
            }

            SwapBuffers(hDC);

            Sleep (10);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            if(tela == 0) tela = 1;
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
                case VK_RIGHT:
                    movLaterais = 1;
                break;
                case VK_LEFT:
                    movLaterais = -1;
                break;
                case VK_UP:
                    movVerticais = 1;
                break;
                case VK_DOWN:
                    movVerticais = -1;
                break;
                case VK_SPACE:
                    selectd = 1;
                break;
                case VK_RETURN:
                    if(tela == 2) tela = -1;
                break;
            }
        }
        break;

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_RIGHT:
                    movLaterais = 0;
                break;
                case VK_LEFT:
                    movLaterais = 0;
                break;
                case VK_UP:
                    movVerticais = 0;
                break;
                case VK_DOWN:
                    movVerticais = 0;
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC, GLuint tela_2d[], Selector selector, Pecas pecas)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    carregaTexturas(tela_2d,selector,pecas);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void reiniciar(){

}