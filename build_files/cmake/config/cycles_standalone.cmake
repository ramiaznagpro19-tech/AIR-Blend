# SPDX-License-Identifier: GPL-2.0-or-later

# only compile Cycles standalone, without AIR Blend
#
# Example usage:
#   cmake -C../blender/build_files/cmake/config/cycles_standalone.cmake  ../blender
#

# disable AIR Blend
set(WITH_BLENDER             OFF  CACHE BOOL "" FORCE)
set(WITH_CYCLES_BLENDER      OFF  CACHE BOOL "" FORCE)

# build Cycles
set(WITH_CYCLES_STANDALONE        ON CACHE BOOL "" FORCE)
set(WITH_CYCLES_STANDALONE_GUI    ON CACHE BOOL "" FORCE)
