# LogMore

Exploring new language features and overengineering solutions, not because I should, but because I can

## Building

```bash
cmake --preset debug
cmake --build --preset debug --parallel <cpus>
ctest --preset debug
```

# Development TODO

 - Write python script to generate a dummy logfile
 - Finish the log level filtering + tests
 - (g/G) skip to ends of file
 - Add forward mode to tail a file
 - Add line wrapping (possibly custom range adaptor)
 - Add search functionality + highlighting
 - Ensure search functionality is asynchronous