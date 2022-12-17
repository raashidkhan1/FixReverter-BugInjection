#include <stdint.h>
#include <stddef.h>

unsigned int * __afl_fuzz_len;
unsigned char *__afl_fuzz_ptr;

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);

int main(int argc, char **argv) {
    LLVMFuzzerTestOneInput(__afl_fuzz_ptr, *__afl_fuzz_len);
}
