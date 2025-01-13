# ALT: A C Library for Interpretable Machine Learning Models

[![Status](https://img.shields.io/badge/Status-Under_Construction-red)](https://github.com/teleprint-me/alt.c)

ALT is an open-source C library for building interpretable machine learning models, specifically transformer models for natural language processing tasks.

## Features

* Transformer models for natural language processing tasks such as completion, chat completion, and infill operations.
* Built from scratch in C for portability and efficiency.
* Support for various features of transformer models, such as byte-pair encoding, continuous bag of words, skip-gram modeling, multi-layer perceptrons, sliding window attention, grouped query attention, forward, upward, and downward projections, single precision and half-precision, 8-bit and 4-bit quantization for digital signal processing, and more.

## Requirements

* Any Linux distribution should work, but officially supports Arch Linux.
* POSIX for CPU multi-threading.
* Vulkan for GPU multi-threading.

## Installation

### Dependencies

* `cmake`: Tool for managing source code building
* `libc`: Standard C library
* `pthread`: POSIX multi-threading library for portable CPUs
* `vulkan`: Portable multi-threading library for GPUs
* `libpcre2`: Implements Perl 5-style regular expressions
* `libuuid`: DCE compatible Universally Unique Identifier library
* `libutf8proc`: C library for Unicode handling
* `stb`: Image loading/decoding library

### Installation (Arch Linux)

```sh
sudo pacman -S gcc gdb cmake util-linux-libs pcre2 libutf8proc stb vulkan-headers vulkan-tools
```

### Python Dependencies

```sh
sudo pacman -S python-pip python-virtualenv python-isort python-black flake8
```

### Clone the Repository

Clone the repository to your local machine and navigate into it:

```sh
git clone https://github.com/teleprint-me/alt.c.git alt
cd alt
```

### Python Virtual Environment

Set up a Python virtual environment to manage project dependencies:

```sh
virtualenv .venv
source .venv/bin/activate
```

**Important Note:** Arch Linux has recently upgraded Python from **3.12.x** to **3.13.x**. Some critical libraries, such as SentencePiece and PyTorch, are not fully compatible with Python 3.13.x. This causes issues with the development environment.

To resolve this, I have created a Knowledge Base article that provides a detailed guide on building and using a local Python 3.12.x environment without interfering with the system-wide Python installation. Refer to [docs/kb/python.md](docs/kb/python.md) for step-by-step instructions.

### Install CPU Torch

Install the CPU-only version of PyTorch, to avoid GPU dependency issues (GPU support is not required):

```sh
pip install torch --index-url https://download.pytorch.org/whl/cpu --upgrade
```

### Install Python Requirements

Install the additional Python dependencies listed in `requirements.txt`. **It is recommended to install these after PyTorch to avoid potential conflicts:**

```sh
pip install -r requirements.txt
```

## Building the Project

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j $(nproc)
```

## Converting Mistral 7B v0.1

### Convert tokenizer model to alt

```sh
python -m alt.convert.spm models/mistralai/Mistral-7B-Instruct-v0.1
```

### Convert tensors to alt

**TODO:** Tensor conversion coming soon.

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

This project is licensed under the AGPL License. See the [LICENSE](LICENSE) file for details.

---

*Note:* Everything is still a work in progress—expect changes and refinements as development continues.
