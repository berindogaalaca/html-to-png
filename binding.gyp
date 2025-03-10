{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "cplusplus/main.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "/app/cplusplus/lib/include"
      ],
      "libraries": [
        "/app/cplusplus/lib/bin/libAppCore.so",
        "/app/cplusplus/lib/bin/libUltralight.so",
        "/app/cplusplus/lib/bin/libUltralightCore.so",
        "/app/cplusplus/lib/bin/libWebCore.so"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      "cflags": [
        "-std=c++17"
      ],
      "cflags_cc": [
        "-std=c++17"
      ],
      "ldflags": [
        "-Wl,-rpath=./"
      ]
    }
  ]
}
