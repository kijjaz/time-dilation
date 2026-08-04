# Time Dilation DAW Workstation — Installation & Compilation Manual

> **Target Platform Support**: macOS (11+), Linux (Ubuntu, Debian, Fedora, Arch), Windows (10/11 64-bit)  
> **Toolchain**: CMake 3.22+, C++20 Compiler (Clang 13+, GCC 11+, or MSVC 2022)  
> **Frameworks**: JUCE 7 & Tracktion Engine (*Automatically fetched via CMake FetchContent*)  

---

## 📋 System Requirements & Dependencies

Before compiling **Time Dilation DAW**, ensure your system meets the minimum requirements:

* **C++20 Compiler**: Clang 13.0+, GCC 11.0+, or MSVC 2022 (v19.30+).
* **Build Generator**: `CMake` (version 3.22 or higher) and `Git`.
* **Optional Build Acceleration**: `Ninja` (recommended for faster parallel builds).

---

## 🍏 1. macOS Compilation & Installation

### Step 1: Install Prerequisites & Tools

Open **Terminal** and install the Xcode Command Line Tools:
```bash
xcode-select --install
```

If you use **Homebrew**, install CMake, Git, and Ninja:
```bash
brew install cmake git ninja
```

### Step 2: Clone the Repository
```bash
git clone https://github.com/kijjaz/time-dilation-daw.git
cd "time-dilation-daw"
```

### Step 3: Configure and Build
```bash
# Create build directory
mkdir -p build && cd build

# Configure CMake with Release optimization and Ninja generator
cmake -B . -S .. -DCMAKE_BUILD_TYPE=Release -GNinja

# Compile the standalone application
cmake --build . --config Release -j$(sysctl -n hw.logicalcpu)
```

*(Note: If you do not have Ninja installed, omit `-GNinja` to use standard Apple Unix Makefiles).*

### Step 4: Launch the Application
Upon completion, the compiled macOS application bundle is located at:
```bash
open "TimeDilationDAW_App_artefacts/Release/Standalone/Time Dilation DAW.app"
```

> [!NOTE]
> **macOS Security & Gatekeeper**: If macOS prevents launching due to unsigned binary security policies, remove the quarantine flag using:
> ```bash
> xattr -cr "TimeDilationDAW_App_artefacts/Release/Standalone/Time Dilation DAW.app"
> ```

---

## 🐧 2. Linux Compilation & Installation

### Step 1: Install Build Tools & JUCE System Dependencies

Linux requires X11/Wayland headers, FreeType, FontConfig, ALSA, and JACK development packages. Install them according to your Linux distribution:

#### Ubuntu / Debian / Linux Mint:
```bash
sudo apt update
sudo apt install -y build-essential cmake git ninja-build pkg-config \
    libfreetype6-dev libfontconfig1-dev libx11-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxcursor-dev libgl1-mesa-dev libglu1-mesa-dev \
    libasound2-dev libjack-jackd2-dev curl libcurl4-openssl-dev
```

#### Fedora / RHEL:
```bash
sudo dnf install -y @development-tools cmake git ninja-build \
    freetype-devel fontconfig-devel libX11-devel libXext-devel libXinerama-devel \
    libXrandr-devel libXcursor-devel mesa-libGL-devel alsa-lib-devel jack-audio-connection-kit-devel libcurl-devel
```

#### Arch Linux / Manjaro:
```bash
sudo pacman -S --needed base-devel cmake git ninja freetype2 fontconfig \
    libx11 libxext libxinerama libxrandr libxcursor mesa alsa-lib jack curl
```

### Step 2: Clone the Repository
```bash
git clone https://github.com/kijjaz/time-dilation-daw.git
cd "time-dilation-daw"
```

### Step 3: Configure and Build
```bash
# Create build directory
mkdir -p build && cd build

# Configure CMake with Release optimization (-O3 -ffast-math)
cmake -B . -S .. -DCMAKE_BUILD_TYPE=Release -GNinja

# Compile using all available CPU cores
cmake --build . --config Release -j$(nproc)
```

### Step 4: Launch the Binary
```bash
./TimeDilationDAW_App_artefacts/Release/Standalone/Time\ Dilation\ DAW
```

> [!TIP]
> **Linux Audio Drivers**: For best low-latency performance on Linux, ensure ALSA or JACK / PipeWire-Pulse is running.

---

## 🪟 3. Windows Compilation & Installation

### Step 1: Install Visual Studio 2022

1. Download and install **[Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/)**.
2. In the **Visual Studio Installer**, select the workload **Desktop development with C++**.
3. Ensure the following optional components are checked:
   - **MSVC v143 - VS 2022 C++ x64/x86 build tools**
   - **C++ CMake tools for Windows**
   - **Git for Windows**

---

### Step 2: Clone the Repository

Open **Developer PowerShell for VS 2022** or **Command Prompt**:
```powershell
git clone https://github.com/kijjaz/time-dilation-daw.git
cd "time-dilation-daw"
```

---

### Step 3: Configure & Build via MSVC

```powershell
# Create build directory
mkdir build
cd build

# Generate Visual Studio 2022 solution target (64-bit)
cmake -B . -S .. -G "Visual Studio 17 2022" -A x64

# Build executable in Release mode
cmake --build . --config Release --parallel
```

*(Alternative using Ninja generator)*:
```powershell
cmake -B . -S .. -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

### Step 4: Launch the Executable

```powershell
.\TimeDilationDAW_App_artefacts\Release\Standalone\"Time Dilation DAW.exe"
```

> [!TIP]
> **Windows Audio Drivers**: The standalone application supports **WASAPI** (Exclusive/Shared) and **ASIO** drivers out of the box.

---

## 🛠️ Troubleshooting & Clean Rebuilds

### Cleaning the Build Cache
If you update CMake dependencies or source definitions and experience compilation warnings, perform a full clean rebuild:

* **macOS / Linux**:
  ```bash
  rm -rf build
  mkdir build && cd build
  cmake -B . -S .. -DCMAKE_BUILD_TYPE=Release -GNinja
  cmake --build . --config Release
  ```

* **Windows (PowerShell)**:
  ```powershell
  Remove-Item -Recurse -Force build
  mkdir build
  cd build
  cmake -B . -S .. -G "Visual Studio 17 2022" -A x64
  cmake --build . --config Release --parallel
  ```

---

## 📊 Summary Table of Build Commands Across Platforms

| Platform | Dependencies / Tools | CMake Generator | Build Command | Output Path |
| :--- | :--- | :--- | :--- | :--- |
| **macOS** | Xcode CLI, Homebrew CMake/Ninja | `-GNinja` | `cmake --build . --config Release` | `build/.../Standalone/Time Dilation DAW.app` |
| **Linux** | `build-essential`, ALSA, JACK, X11 | `-GNinja` | `cmake --build . --config Release -j$(nproc)` | `build/.../Standalone/Time Dilation DAW` |
| **Windows** | Visual Studio 2022 C++ Workload | `-G "Visual Studio 17 2022"` | `cmake --build . --config Release --parallel` | `build\... \Standalone\Time Dilation DAW.exe` |
