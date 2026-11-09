cmake_minimum_required(VERSION 3.20)
project(ps2_xmb_combo C)

# Needs the toolchain file (-DCMAKE_TOOLCHAIN_FILE=.../ps3-ppu-toolchain.cmake)
# and -DCMAKE_MODULE_PATH=<PS3DK cmake dir>; the CI workflow passes both.
include(ps3-self)

add_executable(ps2_xmb_combo src/main.c)
target_link_libraries(ps2_xmb_combo PRIVATE io sysutil sysfs lv2)

# SIGN emits ps2_xmb_combo.sprx (make_sprx) + ps2_xmb_combo.fake.sprx (fself)
ps3_add_prx(ps2_xmb_combo NAME ps2_xmb_combo VERSION 1.0 SIGN)
