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
#include <map>

using namespace ultralight;

class MyApp : public LoadListener,
              public ViewListener,
              public Logger {
private:
  RefPtr<Renderer> renderer_;
  RefPtr<View> view_;
  bool done_ = false;
  std::map<std::string, std::string> imagePaths_;
  bool useLocalImages_ = false;

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
    view_->set_view_listener(this);
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
    
    useLocalImages_ = false;
    view_->Resize(width, height);
    
    view_->LoadHTML(html_string);
    LogMessage(LogLevel::Info, "Html String loaded into the View.");

    done_ = false;
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

  ultralight::RefPtr<ultralight::Buffer> RunWithImages(const String& html_string, 
                                                    const std::map<std::string, std::string>& imagePaths, 
                                                    uint32_t width = 1600, 
                                                    uint32_t height = 800) {
    LogMessage(LogLevel::Info, "Starting RunWithImages(), waiting for page to load...");
    
    imagePaths_ = imagePaths;
    useLocalImages_ = true;
    
    view_->Resize(width, height);
    
    String modified_html = PreprocessHtml(html_string);
    
    view_->LoadHTML(modified_html);
    LogMessage(LogLevel::Info, "Html String with embedded images loaded into the View.");

    done_ = false;
    do {
      renderer_->Update();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (!done_);

    renderer_->RefreshDisplay(0);
    renderer_->Render();

    BitmapSurface* bitmap_surface = (BitmapSurface*)view_->surface();
    RefPtr<Bitmap> bitmap = bitmap_surface->bitmap();
    ultralight::RefPtr<ultralight::Buffer> buffer = bitmap->EncodePNG();
    
    LogMessage(LogLevel::Info, "Saved a render of our page with images to result.png.");
    
    return buffer;
  }

  String PreprocessHtml(const String& html) {
    std::string htmlStr = html.utf8().data();
    
    for (const auto& pair : imagePaths_) {
      const std::string& imageName = pair.first;
      const std::string& imagePath = pair.second;
      
      std::string dataUrl = GetImageDataUrl(imagePath);
      if (!dataUrl.empty()) {
        std::string localRef = "url('local://" + imageName + "')";
        std::string dataUrlRef = "url('" + dataUrl + "')";
        
        size_t pos = 0;
        while ((pos = htmlStr.find(localRef, pos)) != std::string::npos) {
          htmlStr.replace(pos, localRef.length(), dataUrlRef);
          pos += dataUrlRef.length();
        }
        
        localRef = "url(\"local://" + imageName + "\")";
        while ((pos = htmlStr.find(localRef, pos)) != std::string::npos) {
          htmlStr.replace(pos, localRef.length(), dataUrlRef);
          pos += dataUrlRef.length();
        }
        
        localRef = "url(local://" + imageName + ")";
        while ((pos = htmlStr.find(localRef, pos)) != std::string::npos) {
          htmlStr.replace(pos, localRef.length(), dataUrlRef);
          pos += dataUrlRef.length();
        }
      }
    }
    
    return String(htmlStr.c_str());
  }
  
  std::string GetImageDataUrl(const std::string& imagePath) {
    std::ifstream file(imagePath, std::ios::binary);
    if (!file) {
      LogMessage(LogLevel::Error, "Failed to open image file: " + String(imagePath.c_str()));
      return "";
    }
    
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
      LogMessage(LogLevel::Error, "Failed to read image file: " + String(imagePath.c_str()));
      return "";
    }
    
    std::string mimeType = "image/png"; 
    std::string ext = imagePath.substr(imagePath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    
    if (ext == "jpg" || ext == "jpeg") {
      mimeType = "image/jpeg";
    } else if (ext == "gif") {
      mimeType = "image/gif";
    } else if (ext == "webp") {
      mimeType = "image/webp";
    } else if (ext == "svg") {
      mimeType = "image/svg+xml";
    }
    
    std::string base64Data = Base64Encode(buffer.data(), buffer.size());
    std::string dataUrl = "data:" + mimeType + ";base64," + base64Data;
    
    return dataUrl;
  }
  
  std::string Base64Encode(const uint8_t* data, size_t length) {
    static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((length + 2) / 3) * 4);
    
    uint32_t value;
    for (size_t i = 0; i < length; i += 3) {
      value = data[i] << 16;
      if (i + 1 < length) value |= data[i + 1] << 8;
      if (i + 2 < length) value |= data[i + 2];
      
      out.push_back(encoding[(value >> 18) & 0x3F]);
      out.push_back(encoding[(value >> 12) & 0x3F]);
      out.push_back((i + 1 < length) ? encoding[(value >> 6) & 0x3F] : '=');
      out.push_back((i + 2 < length) ? encoding[value & 0x3F] : '=');
    }
    
    return out;
  }

  virtual void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame,
                               const String& url) override {
    if (is_main_frame) {
      LogMessage(LogLevel::Info, "Our page has loaded!");
      done_ = true;
    }
  }

  virtual void OnDOMReady(ultralight::View* caller,
                         uint64_t frame_id,
                         bool is_main_frame,
                         const String& url) override {
    if (is_main_frame && useLocalImages_) {
      LogMessage(LogLevel::Info, "DOM is ready, processing any dynamic content...");
      
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

Napi::Value renderHtmlToPNGWithImages(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  uint32_t width = 1600;
  uint32_t height = 800;
  
  if (info.Length() >= 3 && info[1].IsNumber() && info[2].IsNumber()) {
    width = info[1].As<Napi::Number>().Uint32Value();
    height = info[2].As<Napi::Number>().Uint32Value();
  }

  std::map<std::string, std::string> imagePaths;
  if (info.Length() >= 4 && info[3].IsObject()) {
    Napi::Object pathsObj = info[3].As<Napi::Object>();
    Napi::Array propertyNames = pathsObj.GetPropertyNames();
    
    for (uint32_t i = 0; i < propertyNames.Length(); i++) {
      Napi::Value key = propertyNames[i];
      Napi::Value value = pathsObj.Get(key);
      
      if (key.IsString() && value.IsString()) {
        std::string keyStr = key.As<Napi::String>().Utf8Value();
        std::string valueStr = value.As<Napi::String>().Utf8Value();
        imagePaths[keyStr] = valueStr;
      }
    }
  }

  std::string html_string = info[0].As<Napi::String>().Utf8Value();
  ultralight::String html_string_ul = ultralight::String(html_string.c_str());

  ultralight::RefPtr<ultralight::Buffer> buffer = MyApp::instance().RunWithImages(html_string_ul, imagePaths, width, height);

  if (!buffer) {
    Napi::Error::New(env, "Failed to render HTML with images").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<char> napiBuffer = Napi::Buffer<char>::Copy(env, (char*)buffer->data(), buffer->size());

  return napiBuffer;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "renderHtmlToPNG"), Napi::Function::New(env, renderHtmlToPNG));
  exports.Set(Napi::String::New(env, "renderHtmlToPNGWithImages"), Napi::Function::New(env, renderHtmlToPNGWithImages));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)