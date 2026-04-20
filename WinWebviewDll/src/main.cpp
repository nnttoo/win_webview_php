#include <iostream>
#include <functional>
#include "openWebview2.h"
#include <thread>
#include <objbase.h>
#include "tools.h"


struct ResourceRequest
{
    char *uri;
    char *method;
    unsigned char *body;
    size_t body_len;

    void* myweb;
    void* resouceReq;
};

struct ResourceResponse
{
    unsigned char *body;
    size_t body_len;
    char *contentType;
    int status = 200;
};



typedef void (*SendResponse)(ResourceResponse *, void*,void* );

struct WebConfig
{
    char *wclassname;
    char *url;
    char *title;

    int width;
    int height;

    bool isKiosk;
    bool isMaximize;
    bool isDebug;

    void (*onVirtualHostRequested)(ResourceRequest *, SendResponse);
    char *virtualHostName;
};

void callWebView(WebConfig *param)
{
    MyWebView::WebViewConfig webconfig;

    webconfig.wclassname = ConvertToWideString(param->wclassname).c_str();
    webconfig.url = ConvertToWideString(param->url).c_str();
    webconfig.title = ConvertToWideString(param->title).c_str();
    webconfig.virtualHostName = ConvertToWideString(param->virtualHostName).c_str();

    webconfig.height = param->height;
    webconfig.width = param->width;

    webconfig.modewindow = WS_OVERLAPPEDWINDOW;
    webconfig.maximized = SW_NORMAL;

    if (param->isKiosk)
    {
        webconfig.modewindow = WS_POPUP;
    }
    if (param->isMaximize)
    {
        webconfig.maximized = SW_MAXIMIZE;
    }

    MyWebView* my = new MyWebView();
    if (param->onVirtualHostRequested != nullptr)
    { 

        webconfig.onVirtualHostRequested = [param,my](MyWebView::ResourceRequest *request)
        {
           SendResponse callback = [](ResourceResponse * response, void* webPointer, void * reqPointer)
           { 
                MyWebView * myweb = static_cast<MyWebView*>(webPointer);
                MyWebView::ResourceRequest *request = static_cast<MyWebView::ResourceRequest *>(reqPointer);

                auto res = new MyWebView::ResourceResponse();
                res->body.assign(response->body, response->body + response->body_len);
                res->contentType = L"text/html";
                res->status = 200;

                printf("\n\n\n ini sudah dipanggil loh \n\n"); 

                PostMessageW(myweb->hWnd, WM_SEND_WEB_MESSAGE, (WPARAM)request, (LPARAM)res);
                
           };

           ResourceRequest phpReq; 
           phpReq.method = const_cast<char*>(ConvertFromWideString(request->method).c_str());
           phpReq.body = request->body.data();
           phpReq.body_len = request->body.size();
           phpReq.uri = const_cast<char*>(ConvertFromWideString(request->uri).c_str());

           phpReq.resouceReq = request;
           phpReq.myweb = (void*) my;



           param->onVirtualHostRequested(&phpReq,callback);
        };
    } 

    my->openWebview2(NULL, webconfig);

    delete my;
    printf("myweb has deleted\n\n");
}

extern "C"
{
    __declspec(dllexport) const wchar_t *callWeb(WebConfig *mywebconfig)
    {
        callWebView(mywebconfig);

        return L"success";
    }
}