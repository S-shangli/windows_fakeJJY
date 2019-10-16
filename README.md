# fakeJJY
fake JJY generator via Serial Port

## Environment
+ Windows OS
+ MSYS + gcc

## Compile
```
$ gcc ./main.c -o fakeJJY
```
 
## Usage
```
$ ./fakeJJY.exe
 Error: invalid args
 
 Usage: fakeJJY.exe <COMPORT> <BAUDRATE>
   ex.: fakeJJY.exe COM3      80000
```
 
``` $ ./fakeJJY.exe  COM3 80000
Opening:        COM3            done
Setting:        80000N81        done
Waiting:        clock 00sec     done(19h47m00s)
TIMECODE:       M10000111M000101001M001001000M100100100M000011001M011000000M
Sending:        M10000111M000101001M001001000M100100100M000011001M011000000M
Waiting:        clock 00sec     done(19h48m00s)
TIMECODE:       M10001000M000101001M001001000M100100100M000011001M011000000M
Sending:        M10001000M000101001M001001000M100100100M000011001M011000000M
Waiting:        clock 00sec     done(19h49m00s)
```

## External Connection
```
                     (coil)   (put the near by that coil)
 COM_PORT.TX ----------@@@    [JJY receiver] 
```

## Known limitations
+ not support JJY call sign
+ rough synchronise with windows date (using Sleep WinAPI)
