#pragma once

#if __cplusplus >= 201103L
   #include <functional>
   #include <memory>
#else
   #error "this plugin needs a compiler with C++11 support, ie. gcc-4.8.1 or higher"
#endif
