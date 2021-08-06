#include <iostream>
#include <windows.h>
#include <gdiplus.h>

const int ENCODER_GET_FAILURE = -1;

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
    UINT num = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo *pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return ENCODER_GET_FAILURE;
    
    pImageCodecInfo = (Gdiplus::ImageCodecInfo *) malloc(size);
    if (! pImageCodecInfo)
        return ENCODER_GET_FAILURE;
    
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return ENCODER_GET_FAILURE;
}

int main() {
    HWND hDesktopWindow = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWindow);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    
    size_t nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    size_t nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
                                nScreenWidth, nScreenHeight);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight,
            hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);

    ReleaseDC(hDesktopWindow, hDesktopDC);
    DeleteDC(hCaptureDC);


    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Gdiplus::Bitmap *capture = new Gdiplus::Bitmap(hCaptureBitmap, NULL);
    CLSID nClsId;
    int retVal = GetEncoderClsid(L"image/bmp", &nClsId);
    
    if (retVal == -1) {
        std::cout << "Could not get bmp encoder ID." << '\n';
        return EXIT_FAILURE;
    }

    capture->Save(L"capture.bmp", &nClsId, NULL);
    delete capture;

    DeleteObject(hCaptureBitmap);
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return EXIT_SUCCESS;
}
