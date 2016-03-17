# Lewei_lib
Lewei长连接库，ESP8266专用


库中使用到了aJson库 链接:https://github.com/interactive-matter/aJson 
还使用到C++标准库的map，如果编译出现std::map linking error的错误，请尝试将libstdc++添加到platforms.txt中，具体方法如下：

> ESP8266 for Arduino IDE (xtensa-lx106-elf-gcc) and std::map linking error
> 
>  For some reason the standard ESP build does not link against libstdc++.
>  You'll need to edit platforms.txt in $ARDUINO_IDE/hardware/esp8266com/esp8266, and add -lstdc++ to the following line:
>    compiler.c.elf.libs=-lm -lgcc -lhal -lphy -lnet80211 -llwip -lwpa -lmain -lpp -lsmartconfig -lwps -lcrypto -laxtls
>  Arduino IDE only reads this file once at start, so, make sure to restart the IDE before compiling.
> 
>  From: http://stackoverflow.com/questions/33450946/esp8266-for-arduino-ide-xtensa-lx106-elf-gcc-and-stdmap-linking-error

