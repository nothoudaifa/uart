# uart
simple uart configuration

## build instructions
run the following command
```bash
gcc uart.c -o uart
```

## run instructions
you could either use a real uart device or use socat, i will be using socat in this example
```
socat -d -d pty,raw,echo=0 pty,raw,echo=0
2026/05/15 19:11:59 socat[893204] N PTY is /dev/pts/2
2026/05/15 19:11:59 socat[893204] N PTY is /dev/pts/3
2026/05/15 19:11:59 socat[893204] N starting data transfer loop with FDs [5,5] and [7,7]
```
then run your executable using the first /dev/pts/2 and read and write using your terminal from /dev/pts/3
> Note: these will be different on your machine
```
./uart /dev/pts/2
```

and in another terminal
```
cat /dev/pts/3
```
or
```
echo -n 'my cool data' > /dev/pts/3
```
