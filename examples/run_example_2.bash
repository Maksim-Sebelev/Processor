example_asm=example2/Math.asm
example_bin=example2/Math.bin

./build/asm --src ${example_asm} --exe ${example_bin}
./build/exe ${example_bin}
