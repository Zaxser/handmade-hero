#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64; 

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Win32_Offscreen_Buffer {
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int bytesPerPixel;
  int pitch;
};

struct Win32_Window_Dimension {
  int width;
  int height;
};

global_variable BOOL running;
global_variable Win32_Offscreen_Buffer globalBackbuffer;

Win32_Window_Dimension
win32GetWindowDimension(HWND window){
  Win32_Window_Dimension result;
  RECT clientRect;
  GetClientRect(window, &clientRect);
  result.width = clientRect.right - clientRect.left;
  result.height = clientRect.bottom - clientRect.top;
  return result;
}

internal void 
renderWeirdGradient(Win32_Offscreen_Buffer buffer, 
                    int xOffset, 
                    int yOffset){
  uint8 *row = (uint8 *)buffer.memory;
  for(int y = 0; y < buffer.height; y++){
    uint32 *pixel = (uint32 *)row;
    for(int x = 0; x < buffer.width; x++){
      // Pixel in Memory :
      uint8 blue = (x + xOffset);
      uint8 green = (y + yOffset);
      
      *pixel++ = (green << 8 | blue);
    }
    row += buffer.pitch;
  }
}

internal void 
win32ResizeDIBSection(Win32_Offscreen_Buffer *buffer, int width, int height){
  if(buffer->memory){
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;

  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = buffer->width;
  buffer->info.bmiHeader.biHeight = buffer->height;
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  buffer->bytesPerPixel = 4;
  int bitMapSize = buffer->width * buffer->height * buffer->bytesPerPixel;
  buffer->memory = VirtualAlloc(0, bitMapSize, MEM_COMMIT, PAGE_READWRITE);
  
  buffer->pitch = buffer->width * buffer->bytesPerPixel;
  //renderWeirdGradient(1, 1);
}

internal void 
win32DisplayBufferInWindow(HDC deviceContext, 
                           int windowWidth,
                           int windowHeight,
                           Win32_Offscreen_Buffer buffer,
                           int x, 
                           int y, 
                           int width, 
                           int height){
  StretchDIBits(deviceContext,
                0, 0, windowWidth, windowHeight,
                0, 0, buffer.width, buffer.height,
                buffer.memory,
                &buffer.info,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32MainWindowCallback(HWND window,
                                         UINT message,
                                         WPARAM wParam,
                                         LPARAM lParam){
  LRESULT result = 0;
  switch(message){
    case WM_SIZE:{
      OutputDebugString("WM_SIZE\n");
    }break;
    case WM_DESTROY:{
      running = false;
    }break;
    case WM_CLOSE:{
      running = false;
    }break;
    case WM_ACTIVATEAPP:{
      OutputDebugString("WM_ACTIVATEAPP\n");
    }break;
    case WM_PAINT:{
      PAINTSTRUCT paint;
      HDC deviceContext = BeginPaint(window, &paint);
      int x = paint.rcPaint.left;
      int y = paint.rcPaint.top;
      int width = paint.rcPaint.right - paint.rcPaint.left;
      int height = paint.rcPaint.bottom - paint.rcPaint.top;

      Win32_Window_Dimension dimension = win32GetWindowDimension(window);
      win32DisplayBufferInWindow(deviceContext, 
                                 dimension.width,
                                 dimension.height,
                                 globalBackbuffer,
                                 x, 
                                 y, 
                                 width, 
                                 height);
      EndPaint(window, &paint);
    }
    default:{
      OutputDebugString("default\n");
      result = DefWindowProc(window, message, wParam, lParam);
    }break;
  }

  return result;
}

int CALLBACK WinMain(HINSTANCE instance,
                     HINSTANCE prevInstance,
                     LPSTR commandLine,
                     int showCode){
  WNDCLASS WindowClass = {};

  win32ResizeDIBSection(&globalBackbuffer, 1280, 720);

  WindowClass.style = CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = win32MainWindowCallback;
  WindowClass.hInstance = instance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(RegisterClass(&WindowClass)){
    HWND window = CreateWindowEx(
      0,
      WindowClass.lpszClassName,
      "Handmade Hero",
      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      instance,
      0
    );

    if(window){
      running = true;
      int x = 0;
      int y = 0;
      while(running){
        MSG message;

        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
          if(message.message == WM_QUIT || message.message == WM_CLOSE){
            running = false;
          }
          TranslateMessage(&message);
          DispatchMessageA(&message);
        }

        HDC deviceContext = GetDC(window);
        renderWeirdGradient(globalBackbuffer, x, y);
        Win32_Window_Dimension dimension = win32GetWindowDimension(window);
        win32DisplayBufferInWindow(deviceContext, 
                                   dimension.width,
                                   dimension.height,
                                   globalBackbuffer, 
                                   0, 
                                   0, 
                                   dimension.width,
                                   dimension.height);
        x++;
        y++;
      }
    }else{

    }
  }
  return 0;
}