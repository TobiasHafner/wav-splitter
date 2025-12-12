# wav-splitter
**wav-splitter** is a command-line tool that splits multichannel WAV files as known from **Midas M32** or **Behringer X32** multitrack recordings into individual mono tracks.

## Features
- Split multichannel WAV files into separate mono tracks.
- Automatically merges sequential input files to create one WAV file per channel.
- Creates an organized output directory for all extracted tracks.

## Usage
```bash
wav-splitter <session_path>
```

- <session_path>: Path to the directory containing your multitrack WAV files.

The session directory contains audio files representing chunks of an input sequence. Each file is named using an eight digit uppercase hexadecimal string that indicates its order in the input sequence. The first file is thus called `00000001.WAV`, the second one `00000002.WAV` while the last one might be `00000A3F.wav`.

The tool will create an output directory called `out` inside the specified session directory. Each channel of the multitrack WAV files will be saved as a separate mono WAV file. The Multichannel WAV files from the session are automatically merged per channel, resulting in one WAV file per channel.

## Build Instructions (Linux)
This section explains how to build the **wav-splitter** tool from source on Linux (Debian 12+), using CMake and Ninja.

### 1. Install dependencies

Make sure you have a C compiler, CMake, and Ninja installed:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build
```

### 2. Clone the repository

```bash
git clone https://github.com/TobiasHafner/wav-splitter wav-splitter
cd wav-splitter
```

### 3. Create a build directory
It is recommended to build in a separate directory.

```bash
mkdir build
cd build
```

### 4. Configure the build with CMake
This generates the build files using the Ninja build system.

```bash
cmake .. -G Ninja
```

### 5. Build the project
```bash
ninja
```

The wav-splitter executable will be created inside the build directory.

### Cleaning and rebuilding from scratch
To clean and rebuild from scratch, delete the build directory. This already includes steps 3-5.
```bash
rm -rf build
mkdir build
cd build
cmake .. -G Ninja
ninja
```

## Build Instructions (Windows)
This section explains how to build the **wav-splitter** tool from source on Windows, using CMake and Ninja.

### 1. Install dependencies

Make sure you have a C compiler, CMake, and Ninja installed:
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the "Desktop development with C++" workload (needed for compiler and tools).
- [CMake](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)

### 2. Clone the repository
Using PowerShell or Command Prompt.
```powershell
git clone https://github.com/TobiasHafner/wav-splitter wav-splitter
cd wav-splitter
```

### 3. Create a build directory
It is recommended to build in a separate directory.

```powershell
mkdir build
cd build
```

### 4. Configure the build with CMake
This generates the build files using the Ninja build system.

```powershell
cmake .. -G Ninja
```

### 5. Build the project
```bash
ninja
```

The wav-splitter executable will be created inside the build directory.

### Cleaning and rebuilding from scratch
To clean and rebuild from scratch, delete the build directory. This already includes steps 3-5.
```powershell
rd /s /q build
mkdir build
cd build
cmake .. -G Ninja
ninja
```

## Contributing
Contributions, issues, and feature requests are welcome! Please open an issue or submit a pull request.

## License
Distributed under the Apache 2.0 License. See LICENSE for more information.
