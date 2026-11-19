/* SPDX-FileCopyrightText: 2026 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup creator
 *
 * Android-specific creator entry point, used when AIR Blend runs inside an APK.
 *
 * AIR Blend is built as a shared library (libblender.so) which SDLActivity loads at startup,
 * calling the main entry point defined below.
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <thread>

#include <android/log.h>
#include <unistd.h>

#include <SDL3/SDL_main.h>

#include "creator_intern.h"

#define BLENDER_LOG_TAG "blender"

/* -------------------------------------------------------------------- */
/** \name Standard Output Redirection to the System Log
 * \{ */

static void stdio_logcat_pump_fn(int read_fd)
{
  char buffer[1024];
  ssize_t size;

  while ((size = read(read_fd, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[size] = '\0';
    __android_log_write(ANDROID_LOG_INFO, BLENDER_LOG_TAG, buffer);
  }

  close(read_fd);
}

static bool redirect_stdio_to_logcat()
{
  int fds[2];
  if (pipe(fds) == -1) {
    return false;
  }

  setvbuf(stdout, nullptr, _IOLBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  dup2(fds[1], STDOUT_FILENO);
  dup2(fds[1], STDERR_FILENO);
  close(fds[1]);

  std::thread(stdio_logcat_pump_fn, fds[0]).detach();
  return true;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name App Directories Fallback (AIR Blend)
 *
 * `BlenderActivity` passes the app directories via environment variables. When they don't
 * reach native code (seen with ARM binary translation in emulators), rebuild them from
 * the package name, the process name of the app.
 * \{ */

static std::string android_package_name()
{
  std::ifstream cmdline("/proc/self/cmdline");
  std::string name;
  std::getline(cmdline, name, '\0');
  /* Remove a ":process" suffix, if any. */
  const size_t colon = name.find(':');
  if (colon != std::string::npos) {
    name.resize(colon);
  }
  return name;
}

static void android_env_fallbacks()
{
  if (getenv("BLENDER_ANDROID_EXTRACT_DIR") && getenv("HOME") && getenv("TMPDIR")) {
    return;
  }
  const std::string package = android_package_name();
  if (package.empty() || package.find('/') != std::string::npos) {
    return;
  }
  const std::string data_dir = "/data/user/0/" + package;
  const std::string extract_dir = data_dir + "/files/extract";
  const std::string home_dir = data_dir + "/files/home";
  const std::string tmp_dir = data_dir + "/cache";

  if (!getenv("BLENDER_ANDROID_EXTRACT_DIR") && access(extract_dir.c_str(), R_OK) == 0) {
    setenv("BLENDER_ANDROID_EXTRACT_DIR", extract_dir.c_str(), 1);
    printf("creator: BLENDER_ANDROID_EXTRACT_DIR not passed, using %s\n", extract_dir.c_str());
  }
  if (!getenv("HOME") && access(home_dir.c_str(), W_OK) == 0) {
    setenv("HOME", home_dir.c_str(), 1);
  }
  if (!getenv("TMPDIR") && access(tmp_dir.c_str(), W_OK) == 0) {
    setenv("TMPDIR", tmp_dir.c_str(), 1);
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Entry Point
 * \{ */

/* Renamed to SDL_main by <SDL3/SDL_main.h>, called by SDLActivity. */
int main(int argc, char *argv[])
{
  if (!redirect_stdio_to_logcat()) {
    __android_log_write(ANDROID_LOG_WARN,
                        BLENDER_LOG_TAG,
                        "creator: Failed to redirect stdio to Android system log.");
  }

  android_env_fallbacks();

  /* Root of the extracted install tree, set by BlenderActivity. */
  const char *extract_dir = getenv("BLENDER_ANDROID_EXTRACT_DIR");
  if (extract_dir == nullptr) {
    printf("creator: BLENDER_ANDROID_EXTRACT_DIR is not set.\n");
    return EXIT_FAILURE;
  }

  /* Override argv[0] (the theorical program name) with a fake <install>/blender for
   * appdir.cc to be able to infer the program_dirname and find the extracted install. */
  const std::string argv0 = std::string(extract_dir) + "/blender";
  argv[0] = const_cast<char *>(argv0.c_str());

  printf("creator: entering AIR Blend main().\n");

  return creator_main(argc, const_cast<const char **>(argv));
}

/** \} */
