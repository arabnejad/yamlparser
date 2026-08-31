# Usage examples

The examples cover scalar values, mappings, sequences, nested structures, block strings, anchors, aliases, and merge keys.

Build or run every example from the repository root:

```bash
make examples
make run-examples
```

Each program reads its corresponding file from `sample_usage/yaml_files` and demonstrates direct navigation through `YamlValue::at()`.

| Example | Demonstrates |
| --- | --- |
| `basic_config` | Basic scalar configuration |
| `nested_structures` | Nested mappings |
| `arrays_sequences` | Sequences and mappings inside sequences |
| `anchors_merge` | Anchors, aliases, and merge keys |
| `nested_maps` | Recursive mapping traversal |
| `multiline_strings` | Literal and folded strings |
| `data_types` | Scalar type detection |
| `app_config` | Application configuration |
| `complex_data` | Larger nested documents |
| `nested_arrays` | Inline and nested sequences |
