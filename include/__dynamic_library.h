#pragma once
#include <__audio_base.h>

#if defined(_WIN32)
#define __STDAUDIO_HAS_DYLIB 1
#include <windows.h>
#elif __has_include(<dlfcn.h>)
#define __STDAUDIO_HAS_DYLIB 1
#include <dlfcn.h>
#endif

_LIBSTDAUDIO_NAMESPACE_BEGIN
#if __STDAUDIO_HAS_DYLIB
class __dynamic_library
{
public:
  explicit __dynamic_library(const char* const so) /* not noexcept - some DLL constructors can throw */
  {
#ifdef _WIN32
    impl = (void*)LoadLibraryA(so);
#else
    impl = dlopen(so, RTLD_LAZY | RTLD_LOCAL | RTLD_NODELETE);
#endif
  }

  __dynamic_library(const __dynamic_library&) noexcept = delete;
  __dynamic_library& operator=(const __dynamic_library&) noexcept = delete;
  __dynamic_library(__dynamic_library&& other)
  {
    impl = other.impl;
    other.impl = nullptr;
  }

  __dynamic_library& operator=(__dynamic_library&& other) noexcept
  {
    impl = other.impl;
    other.impl = nullptr;
    return *this;
  }

  ~__dynamic_library()
  {
    if (impl)
    {
#ifdef _WIN32
      FreeLibrary((HMODULE)impl);
#else
      dlclose(impl);
#endif
    }
  }

  template <typename T>
  T symbol(const char* const sym) const noexcept
  {
#ifdef _WIN32
    return (T)GetProcAddress((HMODULE)impl, sym);
#else
    return (T)dlsym(impl, sym);
#endif
  }

  operator bool() const { return bool(impl); }

private:
  void* impl{};
};
#endif

_LIBSTDAUDIO_NAMESPACE_END
