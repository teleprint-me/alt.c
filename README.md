# Alt

[![Status](https://img.shields.io/badge/Status-Under_Construction-red)](https://teleprint.me/)

A transformer model built completely from scratch in C.

## Dependencies

### System Dependencies

- `libc`
- `libuuid`
- `stb`
- `pthread`
- `vulkan`
- `cmake`

#### Install System Dependencies (Arch Linux)

```sh
sudo pacman -S gcc gdb cmake stb vulkan-headers vulkan-tools
```

### Python Dependencies

```sh
sudo pacman -S python-pip python-virtualenv python-isort python-black flake8
```

## Setup

### Clone the Repository

```sh
git clone https://github.com/teleprint-me/alt.c.git alt
cd alt
```

### Python Virtual Environment

```sh
virtualenv .venv
source .venv/bin/activate
```

### Install CPU Torch

```sh
pip install torch torchtext --index-url https://download.pytorch.org/whl/cpu --upgrade
```

### Install Additional Python Requirements

**Warning:** Install these last to avoid potential conflicts.

```sh
pip install -r requirements.txt
```

## Build the Project

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j $(nproc)
```

## Convert Mistral 7B v0.1

**TODO:** Conversion steps coming soon.

## Examples

### Run an Example

```sh
./build/examples/models/perceptron
```

### Sample Output

```text
Epoch 0: Average Error: 0.50862
Epoch 1000: Average Error: 0.15645
Epoch 2000: Average Error: 0.10852
Epoch 3000: Average Error: 0.08668
Epoch 4000: Average Error: 0.07379
Epoch 5000: Average Error: 0.06513
Epoch 6000: Average Error: 0.05883
Epoch 7000: Average Error: 0.05400
Epoch 8000: Average Error: 0.05015
Epoch 9000: Average Error: 0.04699
Trained Weights: 5.48, 5.48 | Bias: -8.32
Input: 0, 0 -> Prediction: 0.000
Input: 0, 1 -> Prediction: 0.056
Input: 1, 0 -> Prediction: 0.056
Input: 1, 1 -> Prediction: 0.934
```

## License

**TBD**

---

*Note:* Everything is still a work in progress—expect changes and refinements as development continues.
