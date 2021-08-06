

#include <iostream>
#include <string>
#include <windows.h>
#include <gdiplus.h>

const int ENCODER_GET_FAILURE = -1;

int GetEncoderClsid(WCHAR const *format, CLSID *pClsid) {
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

const size_t ENCODER_MSTRLEN = 16;

std::wstring encoderfmt_w = L"image/";
std::wstring filename_w = L"capture.";

int main(int argc, char const **argv) {
    char const *fmt = argc > 1 ? argv[1] : "bmp";
    std::wstring fmtbuf_w(ENCODER_MSTRLEN, L'#');
    mbstowcs(&fmtbuf_w[0], fmt, ENCODER_MSTRLEN);

    encoderfmt_w.append(fmtbuf_w);
    filename_w.append(fmtbuf_w);


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
    int retVal = GetEncoderClsid(&encoderfmt_w[0], &nClsId);
    
    if (retVal == -1) {
        std::cout << "Could not get bmp encoder ID." << '\n';
        return EXIT_FAILURE;
    }

    capture->Save(&filename_w[0], &nClsId, NULL);
    delete capture;


    DeleteObject(hCaptureBitmap);
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return EXIT_SUCCESS;
}
