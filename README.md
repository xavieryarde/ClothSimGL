# ClothSimGL
A real-time cloth simulation application built with OpenGL, featuring multiple simulation modes including cloth tearing, collision physics, and flag animation.



https://github.com/user-attachments/assets/d74b0321-9c9e-4b48-84fc-2cad755cbc96



## Features

### Simulation Modes
- **Tear Mode**: Interactive cloth tearing
- **Collision Mode**: Cloth physics with sphere and cube collision objects  
- **Flag Mode**: Realistic flag animation with wind effects

### Physics System
- Mass-spring particle system with Verlet integration
- Multiple spring types: structural, shear, and bend springs
- Constraint satisfaction for stable simulation
- Realistic collision response with friction and damping

### Rendering
- Modern OpenGL 4.6 with PBR-style lighting
- Skybox environments for each simulation mode
- Textured cloth and flag materials
- ImGui interface for real-time parameter control

## Requirements

### Dependencies
- **SDL3** - Windowing and input handling
- **OpenGL 4.6** - Graphics rendering
- **GLM** - Mathematics library
- **SDL3_image** - Texture loading
- **Dear ImGui** - GUI interface
- **GLAD** - OpenGL loader
- **CMake 3.21+** - Build system
- **vcpkg** - Package manager

### System Requirements
- OpenGL 4.6 compatible graphics card
- Windows
- C++23 compatible compiler

## Building

### Prerequisites
1. Install [vcpkg](https://github.com/Microsoft/vcpkg)
2. Set the `VCPKG_ROOT` environment variable

### Windows (Visual Studio)

**Available Configure Presets:**
- `win64-dbg` - x64 Debug build
- `win64-rel` - x64 Release build (recommended)
- `win32-dbg` - x86 Debug build
- `win32-rel` - x86 Release build

**Available Build Presets:**
- `win64-dbg` - x64 Debug build
- `win64-rel` - x64 Release build (recommended)
- `win32-dbg` - x32 Debug build
- `win32-rel` - x32 Release build

**Option 1: Using Developer Command Prompt / Developer Powershell (Recommended)**
```bash
# Configure
cmake --preset win64-rel

# Build
cmake --build build/win64-rel
# Alternative: cmake --build --preset win64-rel

# Install
cmake --install build/win64-rel
```

**Option 2: Regular Command Prompt / Powershell**
```bash
# Modify CMakePresets.json to use full compiler paths
# Edit "CMAKE_C_COMPILER" and "CMAKE_CXX_COMPILER" to full paths like:
# "CMAKE_C_COMPILER": "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.xx.xxxxx/bin/Hostx64/x64/cl.exe"

# Configure
cmake --preset win64-rel

# Build
cmake --build build/win64-rel
# Alternative: cmake --build --preset win64-rel

# Install
cmake --install build/win64-rel
```

## Controls

### General
- **E** - Switch simulation mode (Tear → Collision → Flag)
- **R** - Reset current simulation
- **F** - Toggle fullscreen
- **ESC** - Exit application

### Camera (Collision/Flag modes)
- **SPACE** - Toggle free camera mode
- **W/A/S/D** - Move camera (when active)
- **Mouse** - Look around (when camera active)

### Mode-Specific
- **P** - Change pinning mode (Tear mode)
- **C** - Switch collision shape (Collision mode)
- **Left Click + Drag** - Tear cloth (Tear mode)

### Pinning Modes
- **Top Row** - Pin top edge particles
- **All** - Pin all particles (frozen)
- **Corners** - Pin top corner particles only
- **Flag** - Pin left edge (flag pole)
- **None** - No pinning (free fall)

## License
This project is free to use for any purpose and provided as-is.

## Todos
- Add support for linux/MacOS
