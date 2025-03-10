add_console_app(addon main.cpp)

target_include_directories(addon PUBLIC
  /app/node_modules/node-addon-api
)

target_link_libraries(addon
  AppCore
  Ultralight
  stdc++fs
)
