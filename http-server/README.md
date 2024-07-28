## HTTP-Server
Simple HTTP server written in C++
### Features
- serves static files
  - file exists? -> 200 OK
  - file not found? -> send 404 page
  - directory? -> send directory listing (todo)
- supports only GET requests currently
- async io implemented using C++20 coroutines
- hopefully good code documentation (I'll try my best!)

### How to run
- clone repo & cd into this directory
- install vcpkg or the boost libraries manually
- install cmake, add it to PATH on Windows
- install toolchain with C++20 support (I'm using msvc, but llvm or mingw should work as well)
- run ``cmake ..`` in build directory
- run ``cmake --build .`` in build directory
- run ``.\http-server.exe <port>``
- open browser and navigate to ``localhost:<port>``

### Good to know
- When using vcpkg, make sure to update the paths in CMakeLists.txt