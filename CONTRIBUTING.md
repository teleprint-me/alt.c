# Contributing to ALT

Thank you for your interest in contributing to ALT! Contributions are always welcome and
appreciated. Here are some guidelines to help you get started.

## Reporting Bugs

If you encounter a bug, please open an issue in the
[Issues section](https://github.com/teleprint-me/alt.c/issues) of this repository. Be sure to
provide clear and concise details about the issue, including the steps to reproduce it, any error
messages, and the environment you are using (e.g. operating system, Python version, etc.).

## Reporting CVEs

All CVEs should be reported in the Issues section. These will be publicly maintained.

## Submitting Pull Requests

If you want to contribute code or features, please fork the repository, create a new branch, and
submit a pull request. Before submitting a pull request, please make sure to:

1. Write clear and concise commit messages that describe the changes made.
2. Write unit tests to test the functionality you are adding or changing.
3. Make sure the code follows the existing code style and adheres to the guidelines in the
   [Code Style](#code-style) section.
4. Make sure the code is properly documented, including comments and documentation strings.
5. Make sure all tests pass and the code compiles without warnings or errors.

## Code Style

I use the following code style guidelines for ALT:

1. Use Yoda-style conditionals for literal comparisons to avoid the need for NOT operators, e.g.
   `if (literal == value)` instead of `if (value == literal)`.
2. Use clear and concise names for functions, variables, and types that are self-explanatory, e.g.
   `flex_string_compare` should compare two strings against one another.
3. Use snake_case for function names, e.g. `flex_string_compare`.
4. Use camelCase for function pointers, enums, and structs, e.g. `FlexString`.
5. Use `vk_` as a prefix for Vulkan functions to avoid conflicts with the library's conventions.

## Pull Request Process

Once your pull request is submitted, it will be reviewed by one of the maintainers. If changes are
needed, we will provide feedback and suggest revisions. I may also ask you to update the
documentation or unit tests to reflect the changes you made.

Thank you again for your contribution to ALT! I appreciate your help in making this project better.
