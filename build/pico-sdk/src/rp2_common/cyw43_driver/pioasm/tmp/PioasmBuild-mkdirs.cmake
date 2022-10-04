# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/remon/pico/pico-sdk/tools/pioasm"
  "/Users/remon/pico/1bit_sdcard_out/build/pioasm"
  "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm"
  "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/tmp"
  "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src"
  "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/remon/pico/1bit_sdcard_out/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
