# PSX / PS2 / (PS3?) ADPCM audio decoder
Convert PlayStation 4-bit ADPCM audio into .wav files  


## Sources
[VGMToolBox](https://github.com/Manicsteiner/VGMToolbox) and [VGMStream](https://github.com/vgmstream/vgmstream)  
[PlayStation: The SPU Parts 1-4](https://jsgroth.dev/blog/posts/ps1-spu-part-1/) by jsgort


## Usage
```
-i PATH                  Input file path
-s OFFSET                (optional) Offset from where the ADPCM data starts, default is 0. Can be in decimal or hex (in that case has to start with 0x)
-c CHANNELS              Amount of channels (default 1)
-l INTERLEAVE            Interleave between channels (only needed when there is more than one channel)
-n CHUNKS                (optional) How many interleaved chunks will be converted. One chunk contains the data of all channels
-o PATH                  (optional) Output file path

Example usages:
  psxadec -i "C:\Users\[Username]\Desktop\adpcm_file"
  psxadec -i "C:\Users\[Username]\Desktop\adpcm_file" -s 64 -c 2 -l 0x4000 -o "C:\Users\[Username]\Desktop\wav_file.wav"
```

## Building
### Windows
1. Run `cmake -G "MinGW Makefiles" .`
2. Then use the CMake extension in VSCode to run it (if you don't use VSCode, then just use `cmake --build .` and then open the application in `build\psxadec.exe`)

### Linux
1. Run `cmake .`
2. Use the CMake extension in VSCode or `cmake --build .` and run the executable in `build/psxadec`