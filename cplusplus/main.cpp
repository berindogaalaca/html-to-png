#include <napi.h>
#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

using namespace ultralight;


class MyApp : public LoadListener,
              public Logger {
private:
  RefPtr<Renderer> renderer_;
  RefPtr<View> view_;
  bool done_ = false;

  MyApp() {
    std::string app_path = std::filesystem::current_path().string();
    LogMessage(LogLevel::Info, "App Path: " + ultralight::String(app_path.c_str()));


    Config config;


    Platform::instance().set_config(config);
    
  

    Platform::instance().set_font_loader(GetPlatformFontLoader());

 
    Platform::instance().set_file_system(GetPlatformFileSystem("./assets/"));

    Platform::instance().set_logger(this);

    renderer_ = Renderer::Create();


    ViewConfig view_config;
    view_config.initial_device_scale = 1.0;
    view_config.is_accelerated = false;

    view_ = renderer_->CreateView(1600, 800, view_config, nullptr);


    view_->set_load_listener(this);


  }

  ~MyApp() {
    view_ = nullptr;
    renderer_ = nullptr;
  }

public:
  static MyApp& instance() {
    static MyApp app;
    return app;
  }

  ultralight::RefPtr<ultralight::Buffer> Run(const String& html_string, uint32_t width = 1600, uint32_t height = 800) {
    LogMessage(LogLevel::Info, "Starting Run(), waiting for page to load...");
    
    view_->Resize(width, height);
    
    view_->LoadHTML(html_string);
    LogMessage(LogLevel::Info, "Html String loaded into the View.");

    do {
      renderer_->Update();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (!done_);


    renderer_->RefreshDisplay(0);
    renderer_->Render();


    BitmapSurface* bitmap_surface = (BitmapSurface*)view_->surface();

    RefPtr<Bitmap> bitmap = bitmap_surface->bitmap();

    ultralight::RefPtr<ultralight::Buffer> buffer = bitmap->EncodePNG();
    
    LogMessage(LogLevel::Info, "Saved a render of our page to result.png.");

    return buffer;
  }

  virtual void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame,
                               const String& url) override {

    if (is_main_frame) {
      LogMessage(LogLevel::Info, "Our page has loaded!");
      done_ = true;
    }
  }

  virtual void LogMessage(LogLevel log_level, const String& message) override {
    std::cout << "> " << message.utf8().data() << std::endl << std::endl;
  }
};

Napi::Value renderHtmlToPNG(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Argument must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  uint32_t width = 1600;
  uint32_t height = 800;
  
  if (info.Length() >= 3 && info[1].IsNumber() && info[2].IsNumber()) {
    width = info[1].As<Napi::Number>().Uint32Value();
    height = info[2].As<Napi::Number>().Uint32Value();
  }

  std::string html_string = info[0].As<Napi::String>().Utf8Value();
  ultralight::String html_string_ul = ultralight::String(html_string.c_str());

  ultralight::RefPtr<ultralight::Buffer> buffer = MyApp::instance().Run(html_string_ul, width, height);

  if (!buffer) {
    Napi::Error::New(env, "Failed to render HTML").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<char> napiBuffer = Napi::Buffer<char>::Copy(env, (char*)buffer->data(), buffer->size());

  return napiBuffer;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "renderHtmlToPNG"), Napi::Function::New(env, renderHtmlToPNG));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)