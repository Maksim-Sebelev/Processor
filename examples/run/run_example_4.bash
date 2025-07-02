example_asm=example4/Factorial.asm
example_bin=example4/Factorial.bin

./build/asm --src ${example_asm} --exe ${example_bin}
./build/exe ${example_bin}
