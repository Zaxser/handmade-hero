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

global_variable BOOL running;

global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;

// internal void renderWeirdGradient(int blueOffset, int greenOffset){
//   int width = bitmapWidth;
//   int height = bitmapHeight;

//   int pitch = width * bytesPerPixel;
//   uint8 *row = (uint8 *)bitmapMemory;
//   for(int y = 0; y < bitmapHeight; y++)
// }
internal void win32ResizeDIBSection(int width, int height){
  if(bitmapMemory){
    VirtualFree(bitmapMemory, 0, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;

  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = bitmapWidth;
  bitmapInfo.bmiHeader.biHeight = bitmapHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  int bytesPerPixel = 4;
  int bitMapMemorySize = width * height * bytesPerPixel;
  bitmapMemory = VirtualAlloc(0, bitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  
  int pitch = width * bytesPerPixel;
  uint8 *row = (uint8 *)bitmapMemory;
  for(int y = 0; y < bitmapHeight; y++){
    uint8 *pixel = (uint8 *)row;
    for(int x = 0; x < bitmapWidth; x++){
      // Pixel in Memory :
      *pixel = 255;
      *pixel++;

      *pixel = 0;
      *pixel++;

      *pixel = 0;
      *pixel++;

      *pixel = 0;
      *pixel++;
    }
    row += pitch;
  }
}

internal void 
win32UpdateWindow(HDC deviceContext, 
                  RECT *windowRect, 
                  int x, 
                  int y, 
                  int width, 
                  int height){
  int windowWidth = windowRect->right - windowRect->left;
  int windowHeight = windowRect->bottom - windowRect->top;
  StretchDIBits(deviceContext,
                0, 0, bitmapWidth, bitmapHeight,
                0, 0, windowWidth, windowHeight,
                bitmapMemory,
                &bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}
LRESULT CALLBACK win32MainWindowCallback(HWND window,
                                         UINT message,
                                         WPARAM wParam,
                                         LPARAM lParam){
  LRESULT result = 0;
  switch(message){
    case WM_SIZE:{
      RECT clientRect;
      GetClientRect(window, &clientRect);
      int width = clientRect.right - clientRect.left;
      int height = clientRect.bottom - clientRect.top;
      win32ResizeDIBSection(width, height);
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

      RECT clientRect;
      GetClientRect(window, &clientRect);

      win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
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

  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = win32MainWindowCallback;
  WindowClass.hInstance = instance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  if(RegisterClass(&WindowClass)){
    HWND WindowHandle = CreateWindowEx(
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
    if(WindowHandle){
      MSG message;
      running = true;
      while(running){
        BOOL messageResult = GetMessage(&message, 0, 0, 0);
        if(messageResult > 0){
          TranslateMessage(&message);
          DispatchMessage(&message);
        }else{
          break;
        }
      }
    }else{

    }
  }
  return 0;
}