<<<<<<< HEAD
# AES_on_AMP
=======
# Asymetric-Multi-Processing

### External Library - MbedTLS 

- Building

  1. Copy mbedtls_config.h file to ext_lib/include/mbedtls directory.

  2. Run the command 

      `CC=riscv64-unknown-elf-gcc CFLAGS='-std=gnu99 -Wall -Wextra -march=rv32im -mabi=ilp32 -D__vexriscv__ -MMD' make lib`
>>>>>>> 3e6b413431dd86fc642ba8271c0fc3493ae09324
