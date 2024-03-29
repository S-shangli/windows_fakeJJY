# fakeJJY
fake JJY generator via Serial Port

## References
* JJY format
  * http://jjy.nict.go.jp/jjy/trans/index.html
  * http://jjy.nict.go.jp/jjy/trans/timecode1.html
* Windows API
  * http://wisdom.sakura.ne.jp/system/winapi/win32/index.html

## Environment
+ Windows OS
+ mingw32
+ gcc version 8.2.0 (MinGW.org GCC-8.2.0-3)

## Compile
```
$ gcc ./main.c -o fakeJJY
```
 
## Usage - Error message in without any argument
```
$ ./fakeJJY.exe
Error:  Can't assume <COMPORT> from "fakeJJY.exe"

Usage:
  fakeJJY.exe <COMPORT> [BAUDRATE]
  <COMPORT> can specify in filename (COMPORT assume mode)

Example:
  fakeJJY.exe   COM3    80000  (full specified)
  fakeJJY.exe   COM3           (use default BAUDRATE)
  fakeJJY_COM3.exe             (COMPORT assume mode)
  fakeJJY.exe   COM3    80000t (Test mode)
  fakeJJY.exe   COM3   120000  (60kHz mode)
```
## Usage
```
$ ./fakeJJY.exe COM3 80000
Opening:        COM3            done
Setting:        80000N81        done
Waiting:        clock 00sec     done(10h25m00s)
TIMECODE:       M01000101M000100000M001001001M010100110M000011001M010000000M
Sending:        M01000101M000100000M001001001M010100110M000011001M010000000M
```

## Usage - 60kHz mode
+ into 60kHz mode if you specified over 10000 BAUDRATE
```
$ ./fakeJJY.exe COM3 120000
Warning:        60kHz mode enabled
Opening:        COM3            done
Setting:        120000N81       done
Waiting:        clock 00sec     done(10h14m00s)
TIMECODE:       M00100100M000100000M001001001M010100100M000011001M010000000M
Sending:        M00100100M000100000M001001001M010100100M000011001M010000000M
```

## Usage - Default Baudrate mode
+ Default Baudrate is 80000 (40kHz carrier wave)
```
$ ./fakeJJY.exe  COM3
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
## Usage - COMPORT assume mode
+ COMn (n=0..) keyword in file name
+ for easy to use
```
$ cp fakeJJY.exe fakeJJY_COM3.exe

$ fakeJJY_COM3.exe
Opening:        COM3            done
Setting:        80000N81        done
Waiting:        clock 00sec     done(23h45m00s)
TIMECODE:       M10000101M001000011M001001001M000100110M000011001M101000000M
Sending:        M10000101M001000
```
## Usage - Test mode
+ put t or T charactor after [BAUDRATE]
+ you can see/check/calibrate 40kHz carrier wave in 800ms, if you have oscilloscope.
```
$ fakeJJY.exe COM3 80000t
Warning:        Testmode enabled
Opening:        COM3            done
Setting:        80000N81        done
TESTING:        send_800ms      done
```

## External Connection
```
                     (coil)   (put the near by that coil)
 COM_PORT.TX ----------@@@    [JJY receiver] 
```

## Known limitations
+ not support JJY call sign
+ not support LS1,2 bit (leap second)
+ not support SU1,2 bit (reserved bit)
+ rough synchronise with windows date (using Sleep WinAPI)

## How it working
+ Send data via Serial Port
+ Send 0x55 data
+ Using 80000 baudrate, StopBit 1, Non-Parity

---> Tx line output the 40kHz JJY carrier wave

```
    start  LSB                               MSB  stop
      bit  d1   d2   d3   d4   d5   d6   d7   d8   bit
****+    +----+    +----+    +----+    +----+    +----+****
****|    |    |    |    |    |    |    |    |    |    |****
****+----+    +----+    +----+    +----+    +----+    +****
    <====>      1tick   = 12.5us @ 80000baud
    <=========> 2tick   = 1cycle = 25us = 40kHz
```

+ JJY modulation
  + data0 : 0.8s carrier wave + 0.2s blank
  + data1 : 0.5s carrier wave + 0.5s blank
  + Marker: 0.2s carrier wave + 0.8s blank
  + note: in strictly, blank is not blank that is carrier with 10% power, but not supported :)
  
