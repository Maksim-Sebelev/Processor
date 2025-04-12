# Examples

If you want to check, how examples work - you can do it!
Just type in terminal (you must be in roor directory of project):
```bash
make example1
```
```bash
make example2
```
```bash
make example3
```
```bash
make example4
```

If you want to see it without make use:
```bash
./build/processor -compile examples/example1/HelloWorld.asm examples/example1/HelloWorld.bin -run examples/example1/HelloWorld.bin 
```
```bash
./build/processor -compile examples/example2/Math.asm examples/example2/Math.bin -run examples/example2/Math.bin 
```
```bash
./build/processor -compile examples/example3/Cycle.asm examples/example3/Cycle.bin -run examples/example3/Cycle.bin 
```
```bash
./build/processor -compile examples/example4/Factorial.asm examples/example4/Factorial.bin -run examples/example4/Factorial.bin 
```
