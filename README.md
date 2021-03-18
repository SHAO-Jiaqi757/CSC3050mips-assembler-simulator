# CSC3050mips-assembler-simulator

## Usage: 
```
$ make
g++-10 -g -c ./src/simulator.cpp
g++-10 -g -o simulator assembler.o simulator.o 
```

```
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- TEST 1 < a plus b > +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
$ ./simulator ./test/simulator-samples/a-plus-b.asm ./test/simulator-samples/a-plus-b.in ./test/simulator-samples/a-plus-b.out
$ cat ./test/simulator-samples/a-plus-b.in
114
900
$ cat ./test/simulator-samples/a-plus-b.out
1014

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- END TEST 1 < a plus b >+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
```

```

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- TEST 2 < fib > +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
$ ./simulator ./test/simulator-samples/fib.asm ./test/simulator-samples/fib.in ./test/simulator-samples/fib.out
$ cat ./test/simulator-samples/fib.in
16
$ cat ./test/simulator-samples/fib.out
fib(16) = 987
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- END TEST 2 < fib > +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
```

```
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- TEST 3 < memcpy-hello-world > +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
$ ./simulator ./test/simulator-samples/memcpy-hello-world.asm ./test/simulator-samples/memcpy-hello-world.in ./test/simulator-samples/memcpy-hello-world.out
$ cat ./test/simulator-samples/memcpy-hello-world.in
$ cat ./test/simulator-samples/memcpy-hello-world.out
hello, world

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- END TEST 2 < memcpy-hello-world > +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

```
