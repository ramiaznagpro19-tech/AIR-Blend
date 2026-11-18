"""
This function is for advanced use only, misuse can crash AIR Blend since the user
count is used to prevent data being removed when it is used.
"""

# This example shows what _not_ to do, and will crash AIR Blend.
import bpy

# Object which is in the scene.
obj = bpy.data.objects["Cube"]

# Without this, removal would raise an error.
obj.user_clear()

# Runs without an exception but will crash on redraw.
bpy.data.objects.remove(obj)
