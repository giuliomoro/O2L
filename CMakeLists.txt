set(pixelbone_src  pixel.cpp matrix.cpp gfx.cpp glcdfont.c pru.c util.c Adafruit_NeoPixel.cpp)
set(pixelbone_headers pixel.hpp matrix.hpp gfx.hpp pru.h util.h Adafruit_NeoPixel.h gamma.h ws281x_bin.h)

add_library(pixelbone ${pixelbone_src} ${pixelbone_headers})
#add_library(pixelbone::pixelbone ALIAS pixelbone)

target_include_directories(pixelbone PRIVATE 
    ${BELA_ROOT} ${BELA_ROOT}/include 
    ${CMAKE_SYSROOT}/usr/local/include
)