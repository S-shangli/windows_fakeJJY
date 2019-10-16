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
 
```
$ ./fakeJJY.exe  COM3 80000
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
  
