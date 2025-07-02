example_asm=example5/circle.asm
example_bin=example5/circle.bin

./build/asm --src ${example_asm} --exe ${example_bin}
./build/exe ${example_bin}
