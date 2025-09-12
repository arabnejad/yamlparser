# Limitation Examples

This folder contains self-contained executables that illustrate known limitations or simplified behaviors of **YamlParserLib**.

## Build & Run (from repo root)

```bash
cmake -S . -B build
cmake --build build --target sample_limitation
cmake --build build --target run_sample_limitation
```

- Executables are emitted into `limitation/bin/`.
- When run via the `run_sample_limitation` target, the working directory is set to `limitation/`, so relative paths like `sample_yaml/...` resolve correctly.

## Included samples
- `merge_comment_test`
- `nested_seq_test`
- `escape_test`
- `boolean_test`
- `null_test`

Each one demonstrates a corner case or intentional simplification in the parser.
