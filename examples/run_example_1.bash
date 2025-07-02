example_asm=example1/HelloWorld.asm
example_bin=example1/HelloWorld.bin

./build/asm --src ${example_asm} --exe ${example_bin}
./build/exe ${example_bin}
