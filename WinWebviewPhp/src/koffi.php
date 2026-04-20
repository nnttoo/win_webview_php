<?php

// 1. Definisikan signature fungsi dan struct sesuai C++ (Gaya C)
$c_definitions = "
    // Definisikan tipe callback: fungsi yang menerima char* dan tidak mengembalikan apa-apa
    
    
        typedef struct 
        {
            char *uri;
            char *method;
            unsigned char *body;
            size_t body_len;
            
            void* myweb;
            void* resouceReq;
        } ResourceRequest;

        typedef struct 
        {
            unsigned char *body;
            size_t body_len;
            char *contentType;
            int status;
        } ResourceResponse;

        typedef void (*SendResponse)(ResourceResponse *, void*,void* );

    typedef struct {
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
    } WebConfig;

    char* callWeb(WebConfig* mywebconfig);
";

function createCBuffer($string) {
    $len = strlen($string); 
    $buffer = FFI::new("char[" . ($len + 1) . "]", false); 
    FFI::memcpy($buffer, $string, $len);
    
    return $buffer;
}

function createCbyteArray($bodystring){
    $length = strlen($bodystring);
    $cBody = FFI::new("unsigned char[$length]", false);   
    FFI::memcpy($cBody, $bodystring, $length);

    return (object) [
        "data" => $cBody,
        "len" => $length
    ];
}

try {
    $dll_path = "../WinWebviewDll/build/Release/x64/Release/MyProject.dll";
    if (!file_exists($dll_path)) {
        die("File DLL tidak ditemukan di: " . $dll_path);
    } else {
        echo "file ditemukan";
    }
        
    $ffi = FFI::cdef($c_definitions, $dll_path);
 

    // 3. Buat instance struct
    $config = $ffi->new("WebConfig");
    
     
    $config->wclassname = createCBuffer( "wclassajadeh");
    $config->url = createCBuffer( "https://myappphp.local");
    $config->title = createCBuffer( "Just title");

    $config->width = 900;
    $config->height = 600; 
 
    $config->isKiosk = false; 
    $config->isDebug = true; 
    $config->virtualHostName = createCBuffer("https://myappphp.local");

    echo "Memanggil DLL..." . PHP_EOL;

    $callback = function($request, $sendResponse) use ($ffi) {
        
        $methodStr = FFI::string($request->method);
        $uri =  FFI::string($request->uri);
        echo "test dulu aja" . $uri;

        $response = $ffi->new("ResourceResponse");

        $body = createCbyteArray("test aja dulu");
        $response->body = $body->data;
        $response->body_len = $body->len;


        $sendResponse(FFI::addr($response),  $request->myweb, $request->resouceReq);

    };

    $config->onVirtualHostRequested = $callback;

    // 6. Panggil fungsi di DLL
    $result = $ffi->callWeb(FFI::addr($config));

    echo "Respon return dari DLL: " . FFI::string($result) . PHP_EOL;

} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . PHP_EOL;
} 