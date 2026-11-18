/* SPDX-FileCopyrightText: 2011-2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup GHOST
 * Declaration of GHOST_SystemSDL class.
 */

#pragma once

#include "../GHOST_Types.hh"
#include "GHOST_Event.hh"
#include "GHOST_System.hh"
#include "GHOST_TimerManager.hh"
#include "GHOST_WindowSDL.hh"

#include <SDL3/SDL.h>

class GHOST_WindowSDL;

class GHOST_SystemSDL : public GHOST_System {
 public:
  void addDirtyWindow(GHOST_WindowSDL *bad_wind);

  GHOST_SystemSDL();
  ~GHOST_SystemSDL();

  bool processEvents(bool waitForEvent) override;

  bool setConsoleWindowState(GHOST_TConsoleWindowState /*action*/) override
  {
    return false;
  }

  GHOST_TSuccess getModifierKeys(GHOST_ModifierKeys &keys) const override;

  GHOST_TSuccess getButtons(GHOST_Buttons &buttons) const override;

  GHOST_TCapabilityFlag getCapabilities() const override;

  char *getClipboard(bool selection) const override;

  void putClipboard(const char *buffer, bool selection) const override;

  uint64_t getMilliSeconds() const override;

  uint8_t getNumDisplays() const override;

  GHOST_TSuccess getCursorPosition(int32_t &x, int32_t &y) const override;

  GHOST_TSuccess setCursorPosition(int32_t x, int32_t y) override;

  void getAllDisplayDimensions(uint32_t &width, uint32_t &height) const override;

  void getMainDisplayDimensions(uint32_t &width, uint32_t &height) const override;

  GHOST_IContext *createOffscreenContext(GHOST_GPUSettings gpu_settings) override;

  GHOST_TSuccess disposeContext(GHOST_IContext *context) override;

 private:
#ifdef WITH_VULKAN_BACKEND
  static bool SDLCALL vulkanLifecycleEventWatch(void *userdata, SDL_Event *event);

  Uint32 vulkan_surface_recreation_event_ = 0;
#endif

  GHOST_TSuccess init() override;

  GHOST_IWindow *createWindow(const char *title,
                              int32_t left,
                              int32_t top,
                              uint32_t width,
                              uint32_t height,
                              GHOST_TWindowState state,
                              GHOST_GPUSettings gpu_settings,
                              const bool exclusive = false,
                              const bool is_dialog = false,
                              const GHOST_IWindow *parent_window = nullptr) override;

  /* SDL specific */
  GHOST_WindowSDL *findGhostWindow(SDL_Window *sdl_win);

  bool generateWindowExposeEvents();

  void processEvent(SDL_Event *sdl_event);
  void processPenEvent(const SDL_Event &sdl_event);

  GHOST_TabletData pen_tablet_data_ = {};

#ifdef __ANDROID__
  /* AIR Blend: touch-screen input.
   * - One finger: tap = left click, drag = left drag, long press = right click.
   * - Two fingers: pinch = zoom, move = pan/orbit (sent as trackpad gestures). */
  enum class TouchState { Idle, Pending, Drag, Gesture, WaitAllUp };
  struct TouchFinger {
    SDL_FingerID id;
    float x, y; /* Window coordinates. */
  };
  TouchState touch_state_ = TouchState::Idle;
  std::vector<TouchFinger> touch_fingers_;
  SDL_WindowID touch_window_id_ = 0;
  float touch_start_x_ = 0.0f, touch_start_y_ = 0.0f;
  uint64_t touch_start_ms_ = 0;
  float touch_gesture_cx_ = 0.0f, touch_gesture_cy_ = 0.0f, touch_gesture_dist_ = 0.0f;
  float touch_accum_x_ = 0.0f, touch_accum_y_ = 0.0f, touch_accum_zoom_ = 0.0f;

  void processTouchEvent(const SDL_Event &sdl_event);
  void processTouchLongPress();
  /** Milliseconds until a pending long press fires, -1 when none is pending. */
  int64_t touchLongPressTimeout() const;
  void touchPushCursor(GHOST_WindowSDL *window, SDL_Window *sdl_win, float x, float y);
  void touchPushButton(GHOST_WindowSDL *window, GHOST_TButton button, bool down);
  void touchGestureReset();
#endif

  /** The vector of windows that need to be updated. */
  std::vector<GHOST_WindowSDL *> dirty_windows_;
};
