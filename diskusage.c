#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define MAX_PATH 4096

typedef struct {
	char path[MAX_PATH];
	long long size;
} DirEntry;


// Function to format bytes into human-readable format
void format_size(long long bytes, char *output) {
	const char *units[] = {"B", "KB", "MB", "GB", "TB"};
	int unit_index = 0;
	double size = (double)bytes;

	while (size >= 1024 && unit_index < 4) {
		size /= 1024;
		unit_index++;
	}

	if (unit_index == 0) {
		sprintf(output, "%lld %s", bytes, units[unit_index]);
	} else {
		sprintf(output, "%.2f %s", size, units[unit_index]);
	}
}


// Recursive function to calculate directory size
long long calculate_dir_size(const char *path, int show_details) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	long long total_size = 0;
	char full_path[MAX_PATH];

	if (path == NULL) {
		return 0;
	}

	dir = opendir(path);
	if (!dir) {
		if (show_details) {
			fprintf(stderr, "Warning: Cannot open directory: %s (%s)\n", path, strerror(errno));
		}
		return 0;
	}

	while ((entry = readdir(dir)) != NULL) {
		// Skip . and ..
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// Build full path - check for buffer overflow
		int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
		if (path_len >= sizeof(full_path)) {
			if (show_details) {
				fprintf(stderr, "Warning: Path too long, skipping: %s/%s\n", path, entry->d_name);
			}
			continue;
		}

		if (lstat(full_path, &statbuf) == -1) {
			if (show_details) {
				fprintf(stderr, "Warning: Cannot stat: %s (%s)\n", full_path, strerror(errno));
			}
			continue;
		}

		if (S_ISDIR(statbuf.st_mode)) {
			// Recursively calculate subdirectory size
			total_size += calculate_dir_size(full_path, show_details);
		} else if (S_ISREG(statbuf.st_mode)) {
			// Add file size
			total_size += statbuf.st_size;
		}
	}

	closedir(dir);
	return total_size;
}


// Comparison function for qsort
int compare_entries(const void *a, const void *b) {
	DirEntry *entry_a = (DirEntry *)a;
	DirEntry *entry_b = (DirEntry *)b;

	if (entry_b->size > entry_a->size)
		return 1;
	if (entry_b->size < entry_a->size)
		return -1;
	return 0;
}


// Analyze directory and show top-level breakdown
void analyze_directory(const char *path, int limit) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	char full_path[MAX_PATH];
	DirEntry *entries = NULL;
	int entry_count = 0;
	int entry_capacity = 16;
	long long total_size = 0;

	if (path == NULL) {
		fprintf(stderr, "Error: NULL path provided\n");
		return;
	}

	entries = malloc(sizeof(DirEntry) * entry_capacity);
	if (!entries) {
		fprintf(stderr, "Memory allocation failed\n");
		return;
	}

	dir = opendir(path);
	if (!dir) {
		fprintf(stderr, "Error: Cannot open directory: %s (%s)\n", path, strerror(errno));
		free(entries);
		return;
	}

	printf("Analyzing directory: %s\n", path);
	printf("Scanning...\n\n");

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
		if (path_len >= sizeof(full_path)) {
			fprintf(stderr, "Warning: Path too long, skipping: %s/%s\n", path, entry->d_name);
			continue;
		}

		if (lstat(full_path, &statbuf) == -1) {
			continue;
		}

		// Expand array if needed
		if (entry_count >= entry_capacity) {
			entry_capacity *= 2;
			DirEntry *new_entries = realloc(entries, sizeof(DirEntry) * entry_capacity);
			if (!new_entries) {
				fprintf(stderr, "Memory reallocation failed\n");
				free(entries);
				closedir(dir);
				return;
			}
			entries = new_entries;
		}

		// Calculate size
		long long size;
		if (S_ISDIR(statbuf.st_mode)) {
			size = calculate_dir_size(full_path, 0);
		} else {
			size = statbuf.st_size;
		}

		strncpy(entries[entry_count].path, entry->d_name, MAX_PATH - 1);
		entries[entry_count].path[MAX_PATH - 1] = '\0'; // Ensure null termination
		entries[entry_count].size = size;
		total_size += size;
		entry_count++;
	}

	closedir(dir);

	// Sort by size
	if (entry_count > 0) {
		qsort(entries, entry_count, sizeof(DirEntry), compare_entries);
	}

	// Display results
	printf("Total size: ");
	char size_str[64];
	format_size(total_size, size_str);
	printf("%s\n\n", size_str);

	if (entry_count == 0) {
		printf("No entries found.\n");
		free(entries);
		return;
	}

	printf("Top entries by size:\n");
	printf("%-50s %15s %10s\n", "Name", "Size", "Percent");
	printf("%-50s %15s %10s\n", "----", "----", "-------");

	int display_count = (limit > 0 && limit < entry_count) ? limit : entry_count;

	for (int i = 0; i < display_count; i++) {
		format_size(entries[i].size, size_str);
		double percent = total_size > 0 ? (double)entries[i].size / total_size * 100 : 0;

		// Truncate long names
		char display_name[51];
		if (strlen(entries[i].path) > 47) {
			strncpy(display_name, entries[i].path, 44);
			display_name[44] = '.';
			display_name[45] = '.';
			display_name[46] = '.';
			display_name[47] = '\0';
		} else {
			strncpy(display_name, entries[i].path, 50);
			display_name[50] = '\0';
		}

		printf("%-50s %15s %9.2f%%\n", display_name, size_str, percent);
	}

	free(entries);
}


void print_usage(const char *program_name) {
	printf("Usage: %s [options] [directory]\n", program_name);
	printf("Options:\n");
	printf("  -n <number>  Show top N entries (default: all)\n");
	printf("  -h           Show this help message\n");
	printf("\nExample:\n");
	printf("  %s -n 10 /home/user\n", program_name);
}


int main(int argc, char *argv[]) {
	char *target_dir = ".";
	int limit = 0;
	int opt;

	// Parse command line options
	while ((opt = getopt(argc, argv, "n:h")) != -1) {
		switch (opt) {
			case 'n':
				limit = atoi(optarg);
				if (limit <= 0) {
					fprintf(stderr, "Error: -n must be a positive number\n");
					return 1;
				}
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			default:
				print_usage(argv[0]);
				return 1;
		}
	}

	// Get directory from remaining arguments
	if (optind < argc) {
		target_dir = argv[optind];
	}

	// Verify directory exists and is accessible
	struct stat st;
	if (stat(target_dir, &st) != 0) {
		fprintf(stderr, "Error: Cannot access '%s': %s\n", target_dir, strerror(errno));
		return 1;
	}

	if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr, "Error: '%s' is not a directory\n", target_dir);
		return 1;
	}

	// Get absolute path (but don't fail if realpath fails)
	char abs_path[MAX_PATH];
	if (realpath(target_dir, abs_path) != NULL) {
		analyze_directory(abs_path, limit);
	} else {
		// If realpath fails, use the path as-is
		analyze_directory(target_dir, limit);
	}

	return 0;
}
