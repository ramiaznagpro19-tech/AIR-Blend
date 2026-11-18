@echo off
echo Starting AIR Blend with the Vulkan backend

"%~dp0\blender" --gpu-backend vulkan
