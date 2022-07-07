# LogMore

Exploring new language features and overengineering solutions, not because I should, but because I can
Currently needs gcc-12 or later to compile (will not compile with clang-trunk)

## Building

```bash
cmake --preset debug
cmake --build --preset debug --parallel <cpus>
ctest --preset debug
```

## Comparison to `less`

*nb these are the end goal and do not necessarily reflect the current state*

- Will never get behind when tailing a file (ie: will skip printing if it needs to)
- Filtering by loglevel
- Jumping to timestamps (binary search)

# Development TODO

- Write python script to generate a dummy logfile
- Finish the log level filtering + tests
- (g/G) skip to ends of file
- Add forward mode to tail a file
- Add line wrapping (possibly custom range adaptor)
- Add search functionality + highlighting
- Ensure search functionality is asynchronous