# ALT: A C Library for Interpretable Machine Learning Models

ALT is an open-source C library designed for educational purposes, primarily aimed at enthusiasts,
developers, researchers, and security experts. The library's primary focus is on mechanistic
interpretability of machine learning models, specifically transformer models for natural language
processing tasks such as completion, chat completion, and infill operations.

## Inspiration

The ALT project was inspired by the cautionary tale of the DataKrash, a catastrophic event in the
Cyberpunk universe that resulted in the creation of rogue AI entities. The project aims to promote
the development of safe and secure AI technologies by learning from this cautionary tale. The
project is named after Altiera Cunningham, a brilliant netrunner who was used to create a powerful
AI program in the Cyberpunk universe.

## Goals

The primary goal of the ALT project is to develop safe and secure AI technologies that can prevent
the creation of rogue AI entities that could threaten the stability of the Net. To achieve this, ALT
focuses on providing interpretable machine learning models that can be easily understood and audited
by humans.

## Planned Features

- Support for modern state-of-the-art transformer models for natural language processing tasks.
- Implemented in Pure C for portability and efficiency.
- Support for various features of transformer models, such as:
  - Byte-pair encoding
  - Continuous bag of words
  - Skip-gram modeling
  - Multi-layer perceptrons
  - Sliding window attention
  - Grouped query attention
  - Forward, upward, and downward projections
  - Single precision and half-precision
  - 8-bit and 4-bit quantization for digital signal processing

## Platform Support

ALT currently supports Arch Linux and any Linux distribution should work. The library uses POSIX
Threads for CPU support and Vulkan for GPU support, with a focus on portability over efficiency.

## User Interfaces

In addition to the core library, ALT will provide command line interfaces (CLI) for pre-processing,
post-processing, training, fine-tuning, and inference. A graphical user interface (GUI) will also be
developed using Simple DirectMedia Layer (SDL) with a Vulkan backend for studying pre-trained and
fine-tuned transformer models. The details of these implementations and use cases is to be decided.

## Contributing

Contributions to the ALT project are welcome! If you are interested in contributing, please see the
[CONTRIBUTING.md](CONTRIBUTING.md) file for more information.

## License

ALT is released under the AGPL License. See the [LICENSE](LICENSE) file for more information.
