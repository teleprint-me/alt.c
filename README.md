# Alt

The transformer model completely from scratch in C.

## Dependencies

- cmake
- POSIX Threads
- Vulkan
- libc

## Build

```sh
git clone https://github.com/teleprint-me/alt.c.git alt
cd alt
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j $(nproc)
```

## Examples

- **Run an example:**

```sh
./build/examples/flex_array
```

- **Sampled Output:**

```sh
FlexArray successfully created and populated.
Current length: 3
Current capacity: 10
Element 0: 3.140000
Element 1: 2.710000
Element 2: 1.610000
Popped: 1.610000
Popped: 2.710000
Popped: 3.140000
FlexArray successfully destroyed.
```
