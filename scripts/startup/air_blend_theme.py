# SPDX-License-Identifier: GPL-2.0-or-later

"""
AIR Blend: use the white "AIR Blend White" interface theme by default.

The theme is applied when there are no saved preferences yet (first launch)
and whenever factory preferences are loaded. Once the user saves their own
preferences (including another theme), their choice is respected.
"""

import os

import bpy
from bpy.app.handlers import persistent

_THEME_PRESET = "AIR_Blend_White"
_XML_MAP = (
    ("preferences.themes[0]", "Theme"),
    ("preferences.ui_styles[0]", "ThemeStyle"),
)


def _theme_apply():
    try:
        import rna_xml
        filepath = bpy.utils.preset_find(_THEME_PRESET, "interface_theme", ext=".xml")
        if filepath:
            rna_xml.xml_file_run(bpy.context, filepath, _XML_MAP)
    except Exception as ex:
        print("AIR Blend: unable to apply the default theme:", ex)


def _has_saved_preferences():
    config_dir = bpy.utils.user_resource('CONFIG')
    return bool(config_dir) and os.path.exists(os.path.join(config_dir, "userpref.blend"))


def _theme_apply_first_run():
    if not _has_saved_preferences():
        _theme_apply()
    return None  # Run once.


@persistent
def _on_factory_preferences(*_args):
    _theme_apply()


def register():
    bpy.app.handlers.load_factory_preferences_post.append(_on_factory_preferences)
    if not bpy.app.background:
        bpy.app.timers.register(_theme_apply_first_run, first_interval=0.0)


def unregister():
    bpy.app.handlers.load_factory_preferences_post.remove(_on_factory_preferences)
