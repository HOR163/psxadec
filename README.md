# PSX / PS2 / (PS3?) ADPCM audio decoder
Convert PlayStation 4-bit ADPCM audio into .wav files


## Sources
[VGMToolBox](https://github.com/Manicsteiner/VGMToolbox) and [VGMStream](https://github.com/vgmstream/vgmstream)  
[PlayStation: The SPU Parts 1-4](https://jsgroth.dev/blog/posts/ps1-spu-part-1/) by jsgort


## Download
Windows and Linux binaries are under releases (linux one has been verified to work under WSL).

## Usage
```
psxadec -i <in> [options] -f <frequency>

Options:
  Required:
    -i          Input file path
    -f          Audio frequency (in hz)
  Optional:
    -s          Header skip / Data beginning offset (default 0)
    -c          Number of channels (default 1)
    -l          Interleave between channels (default 16)
    -n          Number of chunks to be read (default 0 ie all)
    -o          Output file path (default is input file path with replaced wav
                extension)

Example usages:
    psxadec -h                       Print this help dialog
    psxadec -i ./file.vag -f 44100
    psxadec -i ./file.vag -c 2 -s 0x4000 -l 0x8000 -f 48000
    psxadec -i ./file.vag -c 1 -s 32 -f 22500 -o ./file.wav
```

## Building
### Windows
1. Run `cmake -G "MinGW Makefiles" .`
2. Then use the CMake extension in VSCode to run it (if you don't use VSCode, then just use `cmake --build .` and run built executable in `build\psxadec.exe`)

### Linux
1. Run `cmake .`
2. Use the CMake extension in VSCode or `cmake --build .` and run the executable in `build/psxadec`

## ⚠️ Disclaimer
As this is my first project in C, some functionality (like memory allocation) might be unsafe and unstable. If you want an alternative, then you can look into vgmtoolbox and vgmstream. In vgmtoolbox you can create .genh files, which later can be converted to wav files using vgmstream (vgmstream is also available in the browser). Also as this is one of the first times using CMake without any extras then the project configuration, building steps and compilation may be wrong. Moreover some C libraries might not be available on Windows if not using MinGW.