# SPDX-FileCopyrightText: 2026 AIR Blend
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
AIR Blend: preferences adapted to phones and tablets (Android only).

Applied on first launch (no saved preferences yet) and when factory preferences are loaded.
Everything stays editable in the Preferences afterwards.

- Larger interface, sized from the screen, so buttons are easier to hit with a finger.
- Zoom towards the fingers and orbit around the selection, which suits touch gestures.
- Lighter viewport settings for smoother drawing, less heat and less memory use.
"""

import os
import sys

import bpy
from bpy.app.handlers import persistent

IS_ANDROID = sys.platform == "android"

# Interface scale range on touch screens.
_UI_SCALE_MIN = 1.0
_UI_SCALE_MAX = 1.4
# Smallest usable interface height, in unscaled interface pixels.
_UI_MIN_HEIGHT = 330.0
# Android draws the interface at a fixed 240 DPI, 2.5x the desktop reference of 96 DPI.
_ANDROID_DPI_FAC = 240.0 / 96.0


def _set(data, attr, value):
    # Tolerate properties that differ between versions.
    try:
        setattr(data, attr, value)
    except Exception as ex:
        print("AIR Blend: unable to set {:s}: {!s}".format(attr, ex))


def _ui_scale_for_screen():
    try:
        win = bpy.context.window_manager.windows[0]
        short_side = float(min(win.width, win.height))
    except Exception:
        return 1.2
    if short_side <= 0.0:
        return 1.2
    scale = short_side / (_ANDROID_DPI_FAC * _UI_MIN_HEIGHT)
    return round(max(_UI_SCALE_MIN, min(_UI_SCALE_MAX, scale)), 2)


def _mobile_preferences_apply():
    prefs = bpy.context.preferences

    view = prefs.view
    _set(view, "ui_scale", _ui_scale_for_screen())
    _set(view, "show_navigate_ui", True)

    inputs = prefs.inputs
    _set(inputs, "use_zoom_to_mouse", True)
    _set(inputs, "use_rotate_around_active", True)

    system = prefs.system
    _set(system, "viewport_aa", 'FXAA')
    _set(system, "anisotropic_filter", 'FILTER_0')
    _set(system, "gl_texture_limit", 'CLAMP_2048')

    _set(prefs.edit, "undo_steps", 16)


def _has_saved_preferences():
    config_dir = bpy.utils.user_resource('CONFIG')
    return bool(config_dir) and os.path.exists(os.path.join(config_dir, "userpref.blend"))


def _apply_first_run():
    if not _has_saved_preferences():
        _mobile_preferences_apply()
    return None  # Run once.


@persistent
def _on_factory_preferences(*_args):
    _mobile_preferences_apply()


def register():
    if not IS_ANDROID:
        return
    bpy.app.handlers.load_factory_preferences_post.append(_on_factory_preferences)
    if not bpy.app.background:
        # Delay so the window exists and its size is known.
        bpy.app.timers.register(_apply_first_run, first_interval=0.5)


def unregister():
    if not IS_ANDROID:
        return
    bpy.app.handlers.load_factory_preferences_post.remove(_on_factory_preferences)
