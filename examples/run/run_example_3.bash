example_asm=example3/Cycle.asm
example_bin=example3/Cycle.bin

./build/asm --src ${example_asm} --exe ${example_bin}
./build/exe ${example_bin}
