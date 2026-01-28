# Disk Usage Analyzer

A command-line tool written in C for analyzing disk space usage. Quickly identify which files and directories are consuming the most space on your system.

## Features

- **Fast Performance** - Written in C for optimal speed
- **Human-Readable Output** - Automatic formatting in B, KB, MB, GB, TB
- **Sorted Results** - Displays entries sorted by size (largest first)
- **Percentage Breakdown** - Shows what percentage each entry occupies
- **Flexible Options** - Limit results to top N entries
- **Safe & Robust** - Proper error handling and permission checks
- **Recursive Scanning** - Analyzes entire directory trees

## Installation

### Prerequisites

- GCC or any C compiler
- Linux/Unix-based system (macOS, Linux, BSD, etc.)

### Build from Source

```bash
# Clone the repository
git clone https://github.com/perridplatypus/disk-usage-analyzer.git
cd disk-usage-analyzer

# Compile the program
gcc -Wall -o diskusage diskusage.c

# Optional: Install system-wide
sudo cp diskusage /usr/local/bin/
```

## Usage

### Basic Usage

```bash
# Analyze current directory
./diskusage

# Analyze specific directory
./diskusage /path/to/directory

# Show only top 10 largest entries
./diskusage -n 10 /path/to/directory

# Display help
./diskusage -h
```

### Command-Line Options

| Option        | Description                       |
| ------------- | --------------------------------- |
| `-n <number>` | Show top N entries (default: all) |
| `-h`          | Display help message              |

### Examples

**Analyze your home directory:**

```bash
./diskusage ~
```

**Find the top 5 space consumers in /var:**

```bash
./diskusage -n 5 /var
```

**Check what's using space in the current project:**

```bash
./diskusage -n 20 .
```

## Output Format

```
Analyzing directory: /home/user/projects
Scanning...

Total size: 2.45 GB

Top entries by size:
Name                                               Size        Percent
----                                               ----        -------
node_modules                                       1.82 GB      74.29%
build                                              445.23 MB    18.19%
.git                                               156.78 MB     6.40%
src                                                28.45 MB      1.16%
docs                                               12.34 MB      0.50%
```

## How It Works

1. **Scanning**: The tool recursively traverses the specified directory
2. **Calculation**: Computes the total size of each subdirectory and file
3. **Sorting**: Sorts all entries by size in descending order
4. **Display**: Shows formatted results with human-readable sizes and percentages

## Performance Considerations

- Large directories (millions of files) may take several seconds to analyze
- Symbolic links are not followed to prevent infinite loops
- Inaccessible directories are skipped with warnings
- Memory usage scales with the number of top-level entries in the target directory

## Limitations

- Currently supports Linux/Unix-based systems only
- Requires read permissions for all directories being analyzed
- Does not count sparse file holes (reports allocated space)
- Does not account for filesystem overhead or block size alignment

## Contributing

Contributions are welcome! Here are some ways you can help:

- Report bugs and issues
- Suggest new features
- Submit pull requests
- Improve documentation

### Development Setup

```bash
# Clone the repository
git clone https://github.com/perridplatypus/disk-usage-analyzer.git
cd disk-usage-analyzer

# Compile with debug symbols
gcc -Wall -g -o diskusage diskusage.c

# Run with valgrind to check for memory leaks
valgrind --leak-check=full ./diskusage
```

## Troubleshooting

**Permission Denied Errors:**

**Note**: Read the source code before running as root.

```bash
# Run with appropriate permissions
sudo ./diskusage /root
```

**Path Too Long Warnings:**

- The tool has a maximum path length of 4096 characters
- Very deep directory structures may be truncated

**Memory Issues:**

- If analyzing directories with millions of entries, the tool may consume significant memory
- Use the `-n` option to limit results

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Abhay - [@perridplatypus](https://github.com/perridplatypus)

Project Link: [https://github.com/perridplatypus/disk-usage-analyzer](https://github.com/perridplatypus/disk-usage-analyzer)
