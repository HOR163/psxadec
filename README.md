# PSX / PS2 / (PS3?) ADPCM audio decoder

Convert PlayStation 4-bit ADPCM audio into .wav files

## Sources

* [VGMToolBox](https://github.com/Manicsteiner/VGMToolbox) and [VGMStream](https://github.com/vgmstream/vgmstream)  
* [argparse](https://github.com/cofyc/argparse)  
* [PlayStation: The SPU Parts 1-4](https://jsgroth.dev/blog/posts/ps1-spu-part-1/) by jsgort  

All third party components are in `/libs/` folder with their respective licenses.

## Download

Windows and Linux binaries are under releases (linux one has been verified to work under WSL).

## Usage

```
Usage: psxadec -i file -f frequency [-s skip] [-c channels] [-l interleave] [-n chunks] [-o file]
   or: psxadec -i ./file.vag -f 44100
   or: psxadec -i ./file.vag -c 2 -s 0x4000 -l 0x8000 -f 48000
   or: psxadec -i ./file.vag -c 1 -s 32 -f 22500 -o ./file.wav

    -h, --help                show this help message and exit
    -i, --input=<str>         [REQUIRED] Input file path
    -f, --frequency=<int>     [REQUIRED] Audio frequency (in hz)
    -s, --skip=<int>          Header skip / Data beginning offset (default 0)
    -c, --channels=<int>      Number of channels (default 1)
    -l, --interleave=<int>    Interleave between channels (default 16)
    -n, --chunks=<int>        Number of chunks to be read (default 0 ie all)
    -o, --output=<str>        Output file path (default is input file path with replaced wav extension)
```

## Building

### Windows

1. Run `cmake --preset windows` or if you are using MSVC, then `cmake --preset default`
2. Then build it `cmake --build --preset windows` or `cmake --build --preset default --config Release` if using MSVC

### Linux

1. Run `cmake --preset linux`
2. Then build it `cmake --build --preset linux`
