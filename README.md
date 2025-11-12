# LGE - Lightweight Game Engine

A modern, cross-platform game engine supporting multiple rendering backends: OpenGL, DirectX 12, and Vulkan.

## Features

- **Multi-Backend Rendering**: Support for OpenGL, DirectX 12, and Vulkan
- **Cross-Platform**: Windows, Linux, and macOS support
- **Modern C++**: Built with C++17
- **Modular Architecture**: Clean separation of concerns

## Project Structure

```
LGE/
├── include/              # Public headers
│   └── LGE/
│       ├── core/         # Core engine systems
│       ├── rendering/    # Rendering abstraction
│       │   └── opengl/   # OpenGL implementation
│       ├── platform/     # Platform-specific code
│       └── math/         # Math utilities
├── src/                  # Source files
│   ├── core/
│   ├── rendering/
│   │   └── opengl/
│   ├── platform/
│   ├── math/
│   └── main.cpp          # Example application
├── third_party/          # External dependencies
│   ├── glfw/             # GLFW windowing library
│   └── glad/             # GLAD OpenGL loader
├── assets/               # Game assets
├── build/                # Build output directory
└── CMakeLists.txt        # Build configuration
```

## Building

### Prerequisites

- CMake 3.20 or higher
- C++17 compatible compiler
- GLFW and GLAD (included in `third_party/`)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run example
./bin/LGE_Example  # Linux/macOS
# or
.\bin\Release\LGE_Example.exe  # Windows
```

## Current Status

- ✅ OpenGL rendering backend (implemented)
- ⏳ DirectX 12 rendering backend (planned)
- ⏳ Vulkan rendering backend (planned)

## Usage Example

```cpp
#include "LGE/core/Application.h"

class MyApp : public LGE::Application {
public:
    MyApp() : LGE::Application("My Game") {}

    void OnUpdate(float deltaTime) override {
        // Update game logic
    }

    void OnRender() override {
        // Render game objects
    }
};

int main() {
    MyApp app;
    app.Initialize();
    app.Run();
    app.Shutdown();
    return 0;
}
```

## License

[Add your license here]

