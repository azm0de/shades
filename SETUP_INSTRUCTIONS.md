# Setup Instructions

## Required Folder Structure

Your project should be organized as follows:

```
C:\Users\Justin\DEV\shades\
├── /libs/
│   └── /detours/
│       ├── /include/
│       │   └── detours.h       <-- Copy from compiled Detours
│       └── /lib.x64/
│           └── detours.lib     <-- Copy from compiled Detours
├── /ThemeEngine/
│   ├── ThemeEngine.cpp
│   └── CMakeLists.txt
├── /Injector/
│   ├── Injector.cpp
│   └── CMakeLists.txt
└── PROJECT_PLAN.md
```

## Step 1: Compile Microsoft Detours

1. Clone the Detours repository:
   ```
   git clone https://github.com/microsoft/Detours.git
   ```

2. Open "Developer Command Prompt for VS" (or x64 Native Tools Command Prompt)

3. Navigate to the Detours directory and compile:
   ```
   cd Detours
   nmake
   ```

4. This will create:
   - `include/detours.h`
   - `lib.X64/detours.lib` (for 64-bit)
   - `lib.X86/detours.lib` (for 32-bit)

## Step 2: Copy Detours Files

1. Create the libs folder structure:
   ```
   mkdir libs\detours\include
   mkdir libs\detours\lib.x64
   ```

2. Copy the compiled files:
   - Copy `Detours/include/detours.h` to `libs/detours/include/`
   - Copy `Detours/lib.X64/detours.lib` to `libs/detours/lib.x64/`

## Step 3: Build ThemeEngine.dll

1. Navigate to the ThemeEngine folder:
   ```
   cd ThemeEngine
   ```

2. Create build directory and run CMake:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. The compiled `ThemeEngine.dll` will be in the build folder

## Step 4: Build Injector.exe

1. Navigate to the Injector folder:
   ```
   cd ../../Injector
   ```

2. Create build directory and run CMake:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. The compiled `Injector.exe` will be in the build folder

## Step 5: Test the Theme

1. Copy `ThemeEngine.dll` and `Injector.exe` to the same folder

2. Open Event Viewer (Windows Key + search for "Event Viewer")

3. Run `Injector.exe` as Administrator:
   ```
   Right-click Injector.exe → Run as administrator
   ```

4. The Event Viewer should immediately apply the dark theme!

## Troubleshooting

- **"Could not open process"**: Run Injector.exe as Administrator
- **"No mmc.exe process found"**: Make sure Event Viewer is running first
- **"Could not find LoadLibraryA"**: Ensure you're running on Windows with kernel32.dll
- **CMake can't find Detours**: Verify the folder structure matches exactly as shown above
