# Smart Backup Utility - Code Refactoring & Software Engineering Standards Guide

**Course:** Advanced Programming Lab - 2nd Year CSE  
**Project:** Smart Backup Utility  
**Purpose:** Establish consistent, professional, and modular coding standards  
**Date:** February 2026

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [New Naming Conventions](#new-naming-conventions)
3. [New Naming Style](#new-naming-style)
4. [New Folder Structure](#new-folder-structure)
5. [Programming Style Guidelines](#programming-style-guidelines)
6. [SOLID Principles Implementation](#solid-principles-implementation)
7. [Design Patterns to Apply](#design-patterns-to-apply)
8. [Migration Roadmap](#migration-roadmap)
9. [Before & After Examples](#before--after-examples)

---

## Executive Summary

### Current State Analysis

The Smart Backup Utility is a functional C-based GTK+ 3.0 desktop application with the following characteristics:

**Strengths:**
- Fully working backup workflow (select → backup → log)
- Real-time progress tracking via GTK progress bar
- Persistent configuration saved to `backup_config.txt`
- Auto-backup timer with configurable interval
- Hidden-file and subfolder filtering

**Areas for Improvement:**
- Single monolithic `main.c` (~500 lines) mixing GUI, logic, and I/O
- 8 global GTK widget pointers with no clear ownership
- No header files — all declarations implicit
- Tight coupling between UI creation and business logic
- Hard-coded Windows path separators (`\\`) throughout
- Name collision bug in backup engine (flat copy)
- No formal design patterns despite clear opportunities

### Refactoring Philosophy

> "We're not rewriting from scratch — we're systematically improving what works, making it cleaner, more maintainable, and easier to extend."

This guide focuses on **evolutionary refactoring**: improving the codebase incrementally while maintaining full backup functionality at each step.

---

## New Naming Conventions

### General Principles

1. **Be Descriptive, Not Cryptic**: Names should reveal intent
2. **Consistency Over Brevity**: Prefer `sourceFilePath` over `src`
3. **Avoid Abbreviations**: Unless universally understood (e.g., `id`, `max`, `min`)
4. **Use Domain Language**: Backup-specific terms (`destination`, `interval`, `recursive`) should be clear

### Variable Naming

#### Current Problems
```c
FILE *src;          // Source file? Source config? Unclear.
FILE *dst;          // Destination file? Unclear.
int returned;       // What was returned?
char *path;         // Path to what?
size_t bytes;       // Bytes of what?
```

#### New Standards

**Local Variables:**
- Use `camelCase` for local variables
- Descriptive names that reveal purpose
- Single letters only for trivial loop counters (`i`, `j`)

```c
// Before
FILE *src, *dst;
size_t bytes;
char *path;

// After
FILE *sourceFile, *destFile;
size_t bytesRead;
char *sourceFilePath;
```

**Constants:**
- Use `UPPER_SNAKE_CASE` for constants
- Prefix with module name if scope is global

```c
// Before
#define MAX_PATH 512
#define MAX_ITEMS 1000
#define CONFIG_FILE "backup_config.txt"

// After
#define BACKUP_MAX_PATH_LENGTH   512
#define BACKUP_MAX_QUEUE_ITEMS   1000
#define BACKUP_CONFIG_FILENAME   "backup_config.txt"
#define BACKUP_LOG_FILENAME      "backup_log.txt"
#define BACKUP_COPY_BUFFER_SIZE  8192
```

**Global Variables:**
- Prefix with `g_` to indicate global scope
- Use `camelCase` after prefix

```c
// Before
GtkWidget *window;
GtkWidget *progress_bar;
gboolean backup_running;
guint timer_id;

// After
GtkWidget *g_mainWindow;
GtkWidget *g_progressBar;
gboolean   g_backupInProgress;
guint      g_autoBackupTimerId;
```

### Function Naming

#### Current Problems
```c
void load_settings();           // No module prefix — load from where?
void add_file_to_list();        // Which list? For what?
void update_status();           // Update what kind of status?
gboolean perform_backup();      // Too vague — what kind of backup?
void show_notification_msg();   // Redundant "msg" suffix
```

#### New Standards

**Naming Pattern:**
- Use `Module_VerbNoun` for public functions
- GTK signal callbacks keep the `on_action_name` convention (GTK standard)
- Static/internal helpers use `moduleName_verbNoun` (camelCase)

```c
// Before
void load_settings();
void save_settings();
void add_file_to_list(const char *path);
void add_folder_to_list(const char *path, int recursive);
gboolean perform_backup(gpointer data);
void update_status(const char *message, double progress);
void show_notification_msg(const char *title, const char *msg);
void create_main_window();

// After
void Settings_Load(void);
void Settings_Save(void);
void FileQueue_AddFile(const char *filePath);
void FileQueue_AddFolder(const char *folderPath, int recursive);
gboolean Backup_Perform(gpointer data);
void UI_UpdateStatus(const char *message, double progressFraction);
void UI_ShowNotification(const char *title, const char *message);
void UI_CreateMainWindow(void);
```

**Function Name Components:**
1. **Module prefix**: `Settings_`, `Backup_`, `FileQueue_`, `UI_`
2. **Action verb**: `Load`, `Save`, `Add`, `Perform`, `Update`, `Show`
3. **Object**: What is being acted upon
4. **Qualifier**: Additional context if needed

### Structure and Type Naming

#### Current Problems
```c
typedef struct {
    char backup_destination[MAX_PATH];
    int auto_backup;
    int backup_interval;
    int max_copies;
    int backup_subfolders;
    int include_hidden;
    int show_notifications;
} BackupSettings;
// All concerns bundled — destination, timer, filter, UI prefs mixed together
```

#### New Standards

**Type Definitions:**
- Use `PascalCase` without prefixes
- Descriptive names that reveal purpose
- Split large structs by concern

```c
// Before — one god struct
typedef struct {
    char backup_destination[MAX_PATH];
    int  auto_backup;
    int  backup_interval;
    int  max_copies;
    int  backup_subfolders;
    int  include_hidden;
    int  show_notifications;
} BackupSettings;

// After — separated by concern
typedef struct {
    char destinationPath[BACKUP_MAX_PATH_LENGTH];
    int  maxSnapshotCopies;
} BackupDestinationConfig;

typedef struct {
    int autoBackupEnabled;
    int intervalSeconds;
} BackupTimerConfig;

typedef struct {
    int includeSubfolders;
    int includeHiddenFiles;
} BackupFilterConfig;

typedef struct {
    int showNotifications;
} BackupUIPreferences;

// Composed config — backward compatible
typedef struct {
    BackupDestinationConfig destination;
    BackupTimerConfig       timer;
    BackupFilterConfig      filter;
    BackupUIPreferences     ui;
} BackupSettings;

// Common pointer types
typedef BackupSettings* BackupSettingsPtr;
```

### Enumeration Naming

#### Current Problems
```c
// No enums used — magic integers used instead:
settings.auto_backup = 1;      // What does 1 mean?
settings.include_hidden = 0;   // Is 0 "yes" or "no"?
```

#### New Standards

**Enum Type Names:**
- Use `PascalCase` with descriptive name
- Use `typedef` for cleaner usage

**Enum Values:**
- Prefix with abbreviated type name
- Use `UPPER_SNAKE_CASE`

```c
// Before — magic integers
settings.auto_backup = 1;
settings.backup_subfolders = 0;

// After — self-documenting enums
typedef enum {
    BACKUP_TIMER_DISABLED = 0,
    BACKUP_TIMER_ENABLED  = 1
} BackupTimerState;

typedef enum {
    BACKUP_FILTER_EXCLUDE = 0,
    BACKUP_FILTER_INCLUDE = 1
} BackupFilterState;

typedef enum {
    BACKUP_STATUS_IDLE      = 0,
    BACKUP_STATUS_RUNNING   = 1,
    BACKUP_STATUS_COMPLETE  = 2,
    BACKUP_STATUS_FAILED    = 3
} BackupStatus;

// Usage becomes self-documenting:
settings.timer.autoBackupEnabled = BACKUP_TIMER_ENABLED;
settings.filter.includeSubfolders = BACKUP_FILTER_INCLUDE;
```

### Macro Naming

#### Current Problems
```c
#define MAX_PATH 512       // Conflicts with Windows SDK macro
#define MAX_ITEMS 1000     // Vague — max items of what?
#define CONFIG_FILE "..."  // No module prefix
```

#### New Standards

**Macro Names:**
- Use `UPPER_SNAKE_CASE`
- Always add module prefix to avoid conflicts
- Descriptive action-oriented names

```c
// Before
#define MAX_PATH    512
#define MAX_ITEMS   1000
#define CONFIG_FILE "backup_config.txt"
#define LOG_FILE    "backup_log.txt"

// After
#define BACKUP_MAX_PATH_LENGTH    512
#define BACKUP_MAX_QUEUE_ITEMS    1000
#define BACKUP_CONFIG_FILENAME    "backup_config.txt"
#define BACKUP_SESSION_LOG_FILE   "backup_log.txt"
#define BACKUP_COPY_BUFFER_SIZE   8192
#define BACKUP_TIMESTAMP_FORMAT   "%04d%02d%02d_%02d%02d"
#define BACKUP_DIR_PREFIX         "Backup_"
```

---

## New Naming Style

### File Naming Conventions

#### Current State
```
main.c         (everything — 500 lines)
               (no headers)
               (no build system)
```

#### New Standards

**Pattern:** `responsibility.c` / `module_component.h`

```
src/main/main.c              (entry point only)
src/core/settings.c          (config persistence)
src/core/backup.c            (backup engine)
src/core/fileops.c           (file queue management)
src/rendering/rendering.c    (GTK window & dialogs)
src/utils/utils.c            (shared helpers)

include/settings.h           (BackupSettings + load/save prototypes)
include/ui.h                 (GTK global widget externs)
include/backup.h             (Backup_Perform + timer prototypes)
include/fileops.h            (FileQueue_* prototypes)
include/utils.h              (UI_UpdateStatus + UI_ShowNotification)
```

**File Naming Rules:**
- Lowercase with underscores for `.c` files
- Lowercase with underscores for `.h` files
- Name reflects single responsibility of the file
- No generic names like `helpers.c` or `misc.c`

### Comment Style

#### Current State
```c
// initialize score
int score = 0;

// fclose
fclose(src);
```

#### New Standards

**File Header:**
```c
/**
 * @file   backup.c
 * @brief  Backup execution engine — file copy, log writing, auto-timer.
 *
 * Implements perform_backup() which copies each file from the GTK list
 * to a timestamped directory under settings.destination.path.
 * Writes a per-session log file inside the backup directory.
 *
 * Dependencies: backup.h, settings.h, ui.h, utils.h
 * Platform: Windows (MSYS2/MinGW-w64) with GTK+ 3.0
 */
```

**Function Header:**
```c
/**
 * @brief  Copy all queued files to a timestamped backup directory.
 *
 * Creates a directory named Backup_YYYYMMDD_HHMM under the configured
 * destination, copies each file from the GTK list, and writes
 * backup_log.txt with per-file success/failure status.
 *
 * Must be called from the GTK main thread (updates progress bar directly).
 *
 * @param data  Unused GLib idle callback parameter (pass NULL).
 * @return      G_SOURCE_REMOVE always (one-shot idle callback).
 */
gboolean Backup_Perform(gpointer data);
```

**Inline Comments:**
```c
// Before — states the obvious
fclose(src);  // fclose

// After — explains WHY
fclose(sourceFile);  // Release handle before GTK event loop regains control
```

---

## New Folder Structure

### Before Refactoring
```
smart-backup-utility/
└── main.c           (~500 lines — everything in one file)
```

**Problems:**
- No separation of concerns
- Impossible to navigate for new developers
- Any change risks breaking unrelated functionality
- Cannot reuse individual modules

### Recommended Structure (After Refactoring)

```
smart-backup-utility/
├── src/
│   ├── main/
│   │   └── main.c              # Entry point ONLY (20 lines)
│   ├── core/
│   │   ├── settings.c          # Config read/write — backup_config.txt
│   │   ├── backup.c            # Backup engine, timer, button callback
│   │   └── fileops.c           # File queue: add/remove/clear
│   ├── rendering/
│   │   └── rendering.c         # GTK window, Settings dialog, Log Viewer
│   └── utils/
│       └── utils.c             # UI_UpdateStatus, UI_ShowNotification
├── include/
│   ├── settings.h              # BackupSettings struct, constants, prototypes
│   ├── ui.h                    # GTK global widget extern declarations
│   ├── backup.h                # Backup_Perform, auto_backup_timer prototypes
│   ├── fileops.h               # FileQueue_* prototypes
│   └── utils.h                 # UI_UpdateStatus, UI_ShowNotification prototypes
├── docs/                       # Documentation (this guide, README)
├── assets/                     # Icons, resources
├── build/                      # Compiled objects (auto-generated, gitignored)
├── Makefile                    # Auto-discovery build
├── .gitignore                  # Excludes build/, *.exe, *.o, config/log files
└── README.md                   # Project documentation
```

**Benefits:**
- **Separation of concerns**: `rendering.c` knows nothing about file copying
- **Easier navigation**: File purpose is obvious from name and location
- **Safer changes**: Modifying `backup.c` cannot accidentally break the GTK window layout
- **Reusability**: `settings.c` could be reused in a CLI backup tool without GTK

### Include Dependency Graph

```
main.c        ──► settings.h, ui.h, backup.h
rendering.c   ──► ui.h, settings.h, utils.h, backup.h, fileops.h
backup.c      ──► backup.h, settings.h, ui.h, utils.h
fileops.c     ──► fileops.h, settings.h, ui.h, utils.h
settings.c    ──► settings.h  (foundation layer — no project includes)
utils.c       ──► utils.h, ui.h, settings.h
```

**No circular dependencies.** `settings.h` and `utils.h` form the stable foundation.

---

## Programming Style Guidelines

### Indentation

- **4 spaces** — no tabs
- Brace on same line as control statement (K&R style)
- Always use braces even for single-line `if` bodies

```c
// Before — inconsistent bracing
if(backup_running) return G_SOURCE_REMOVE;

if (has_items) {
    total++;
}

// After — consistent K&R with always-braces
if (g_backupInProgress) {
    return G_SOURCE_REMOVE;
}

if (hasQueuedItems) {
    totalFiles++;
}
```

### Line Length

- Maximum **100 characters** per line
- Break long GTK function calls at parameter boundaries

```c
// Before — too long
GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Files and Folders", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Add", GTK_RESPONSE_ACCEPT, NULL);

// After — readable
GtkWidget *dialog = gtk_file_chooser_dialog_new(
    "Select Files and Folders",
    GTK_WINDOW(g_mainWindow),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Add",    GTK_RESPONSE_ACCEPT,
    NULL);
```

### Function Length

- Maximum **50 lines** per function
- If a function needs more, extract helper functions

```c
// Before — create_main_window() is 100+ lines

// After — broken into focused helpers
void UI_CreateMainWindow(void) {
    ui_initWindow();
    ui_attachProgressBar();
    ui_attachFileListView();
    ui_attachFileActionButtons();
    ui_attachBackupActionButtons();
    gtk_widget_show_all(g_mainWindow);
}
```

### Magic Number Elimination

```c
// Before — magic numbers everywhere
sprintf(backup_dir, "%s\\Backup_%04d%02d%02d_%02d%02d", ...);
char buffer[8192];
if (st.st_size < 1024)
    sprintf(size_str, "%ld B", st.st_size);

// After — named constants
char buffer[BACKUP_COPY_BUFFER_SIZE];
sprintf(backupDirPath, "%s\\" BACKUP_DIR_PREFIX BACKUP_TIMESTAMP_FORMAT, ...);
if (fileSizeBytes < BYTES_PER_KILOBYTE) {
    sprintf(sizeDisplay, "%ld B", fileSizeBytes);
}
```

### Error Handling

```c
// Before — silent failures
FILE *src = fopen(source_path, "rb");
if (src) {
    FILE *dst = fopen(dest_path, "wb");
    if (dst) { /* copy */ }
    fclose(src);
}

// After — logged failures
FILE *sourceFile = fopen(sourceFilePath, "rb");
if (!sourceFile) {
    fprintf(sessionLog, "FAIL (cannot open source): %s\n", sourceFilePath);
    failedCount++;
    g_free(sourceFilePath);
    continue;
}

FILE *destFile = fopen(destFilePath, "wb");
if (!destFile) {
    fprintf(sessionLog, "FAIL (cannot create dest): %s\n", destFilePath);
    fclose(sourceFile);
    failedCount++;
    g_free(sourceFilePath);
    continue;
}
```

---

## SOLID Principles Implementation

### S — Single Responsibility Principle

**Violation:**
`main.c` has at least 5 distinct responsibilities: GTK window construction, file selection dialogs, backup copying engine, configuration persistence, and display utilities.

**Resolution:**

| File | Single Responsibility |
|------|-----------------------|
| `main.c` | Application startup sequence only |
| `settings.c` | Read and write `backup_config.txt` |
| `fileops.c` | Manage the backup file queue (add/remove/clear) |
| `backup.c` | Execute backup copies and manage the auto-timer |
| `rendering.c` | Construct and manage all GTK widgets and dialogs |
| `utils.c` | Provide shared display utilities (`UpdateStatus`, `ShowNotification`) |

```c
// Before — main() does everything
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    load_settings();          // config I/O
    create_main_window();     // GTK construction
    // starts timer...
    gtk_main();
    return 0;
}

// After — main() only orchestrates startup
int main(int argc, char *argv[]) {
    Env_ConfigureGtkPaths();
    gtk_init(&argc, &argv);
    Settings_Load();
    UI_CreateMainWindow();
    Timer_StartIfEnabled();
    gtk_main();
    return 0;
}
```

### O — Open/Closed Principle

**Violation:**
Adding a new backup type (incremental, compressed) requires modifying `perform_backup()`. Adding a new export format requires modifying the log writer.

**Resolution — BackupStrategy interface:**

```c
// Define an open interface
typedef struct {
    const char *strategyName;
    int (*execute)(const char *sourcePath, const char *destDir, BackupSettings *settings);
} BackupStrategy;

// Existing implementation (closed — unchanged)
static int fullCopy_Execute(const char *src, const char *dest, BackupSettings *cfg) {
    /* current copy logic */
}

BackupStrategy FullCopyStrategy = {
    .strategyName = "Full Copy",
    .execute      = fullCopy_Execute
};

// New implementation (open for extension — no modification of existing code)
static int incremental_Execute(const char *src, const char *dest, BackupSettings *cfg) {
    /* compare modification times, skip unchanged files */
}

BackupStrategy IncrementalStrategy = {
    .strategyName = "Incremental",
    .execute      = incremental_Execute
};
```

### L — Liskov Substitution Principle

**Applicability:**  
LSP concerns substitutable implementations. In C without OOP, this maps to function pointer interfaces. The `BackupStrategy` struct above satisfies LSP — any strategy implementation can substitute another as long as it follows the `execute()` contract: given a source path and destination directory, copy the file and return 0 on success.

```c
// Any strategy can be substituted — caller does not change
void Backup_PerformWithStrategy(BackupStrategy *strategy) {
    // ... iterate queue ...
    int result = strategy->execute(sourceFilePath, backupDir, &settings);
    // works identically regardless of which strategy is passed
}
```

### I — Interface Segregation Principle

**Violation:**
`BackupSettings` forces every consumer to see timer settings, filter settings, destination config, and UI preferences — even if it only needs one.

**Resolution:**

```c
// Before — fat interface
#include "settings.h"   // forces all of BackupSettings on every includer

// After — segregated headers
#include "settings_destination.h"   // only BackupDestinationConfig
#include "settings_timer.h"          // only BackupTimerConfig
#include "settings_filter.h"         // only BackupFilterConfig
```

```c
// backup.c only needs destination — include only what it needs
#include "settings_destination.h"

// rendering.c needs everything for the settings dialog
#include "settings.h"
```

### D — Dependency Inversion Principle

**Violation:**
`create_main_window()` directly calls `perform_backup()`, `load_settings()`, `add_file_to_list()`. High-level GUI depends on low-level implementations.

**Resolution — GTK signals as inversion mechanism:**

```c
// Before — direct coupling in GUI code
void create_main_window() {
    // ...
    button = gtk_button_new_with_label("Start Backup");
    g_signal_connect(button, "clicked", G_CALLBACK(perform_backup), NULL);
    //                                              ^ direct dependency on implementation
}

// After — depends on interface (header), not implementation (.c file)
// rendering.c includes backup.h (interface) — NOT backup.c (implementation)
#include "backup.h"   // knows only: gboolean Backup_Perform(gpointer data);

void UI_CreateMainWindow(void) {
    // ...
    GtkWidget *backupButton = gtk_button_new_with_label("Start Backup");
    g_signal_connect(backupButton, "clicked", G_CALLBACK(Backup_Perform), NULL);
    //                                                   ^ depends on header contract only
}
```

---

## Design Patterns to Apply

### 1. Facade Pattern ✅ (Already Present — Formalize It)

**Where:** `UI_CreateMainWindow()` in `rendering.c`

`UI_CreateMainWindow()` already hides ~100 lines of GTK widget construction behind a single call. Formalize by breaking it into private helper functions:

```c
// Public facade (rendering.h)
void UI_CreateMainWindow(void);

// Private helpers inside rendering.c (not in header)
static void ui_buildWindowFrame(void);
static void ui_attachProgressArea(void);
static void ui_attachFileListView(void);
static void ui_attachFileActionButtons(void);
static void ui_attachBackupActionButtons(void);
```

**Benefit:** `main.c` calls one function. Complexity of widget construction is hidden.

### 2. Observer Pattern ✅ (Via GTK Signals — Formalize It)

**Where:** Button → callback connections in `rendering.c`

GTK's signal/callback system IS the Observer pattern. `rendering.c` (Subject) notifies observers (callbacks in `backup.c`, `fileops.c`) without knowing their implementation.

```c
// Observer registration (rendering.c)
g_signal_connect(backupButton, "clicked", G_CALLBACK(Backup_Perform),       NULL);
g_signal_connect(addButton,    "clicked", G_CALLBACK(FileQueue_OpenChooser), NULL);
g_signal_connect(clearButton,  "clicked", G_CALLBACK(FileQueue_ClearAll),    NULL);
// rendering.c knows ONLY the function signatures — not the implementations
```

**Extend with custom progress observer:**
```c
// Define progress observer interface
typedef void (*BackupProgressCallback)(double fraction, const char *statusMessage, gpointer userData);

// Register observer before starting backup
void Backup_SetProgressObserver(BackupProgressCallback callback, gpointer userData);

// Inside perform_backup — notify observer instead of calling update_status directly
g_backupProgressCallback(progressFraction, statusMessage, g_callbackUserData);
```

### 3. Repository Pattern (Planned)

**Where:** `settings.c` — config persistence

`load_settings()` and `save_settings()` are an informal Repository. Formalize with a function-pointer interface to allow swapping storage backends:

```c
typedef struct {
    int (*load)(BackupSettings *outSettings);
    int (*save)(const BackupSettings *settings);
} SettingsRepository;

// File-based implementation (current)
static int fileRepo_Load(BackupSettings *out) { /* parse backup_config.txt */ }
static int fileRepo_Save(const BackupSettings *cfg) { /* write backup_config.txt */ }

SettingsRepository FileSettingsRepo = {
    .load = fileRepo_Load,
    .save = fileRepo_Save
};

// Registry-based (future Windows integration)
SettingsRepository RegistrySettingsRepo = {
    .load = registryRepo_Load,
    .save = registryRepo_Save
};
```

### 4. Strategy Pattern (Planned)

**Where:** `backup.c` — backup copy logic

Different backup strategies can be hot-swapped without modifying the engine:

```c
typedef struct {
    const char *name;
    int (*execute)(const char *src, const char *destDir, const BackupFilterConfig *filter);
} BackupStrategy;

// Available strategies
extern BackupStrategy FullCopyStrategy;       // copy everything
extern BackupStrategy IncrementalStrategy;    // skip unchanged files
extern BackupStrategy CompressedStrategy;     // zip before copying

// Engine uses whichever strategy is active
void Backup_SetStrategy(BackupStrategy *strategy);
gboolean Backup_Perform(gpointer data);  // uses g_activeStrategy internally
```

---

## Migration Roadmap

### Phase 1 — Preparation (Day 1)

- [ ] Create git branch `refactor/modular-structure`
- [ ] Document every function in `main.c` with its responsibility
- [ ] Identify which module each function belongs to
- [ ] Design include dependency graph (no circular deps)
- [ ] Commit: "docs: add refactoring plan"

### Phase 2 — Structural Split (Day 2)

- [ ] Create folder structure (`src/`, `include/`, `docs/`, `build/`)
- [ ] Create all 5 header files with `#ifndef` guards
- [ ] Move functions to their target `.c` files one module at a time
- [ ] Add `extern` declarations to headers
- [ ] Verify project still compiles after each module extraction
- [ ] Commit after each module: "refactor: extract settings module"

### Phase 3 — Naming Convention (Day 3)

- [ ] Apply `Module_VerbNoun` naming to all public functions
- [ ] Add `g_` prefix to all global GTK widget pointers
- [ ] Replace magic numbers with named constants
- [ ] Replace magic `0`/`1` integers with enums
- [ ] Update all call sites after renaming
- [ ] Commit: "refactor: apply naming conventions"

### Phase 4 — Code Quality (Day 4)

- [ ] Replace `strcpy()` with `strncpy()` in `settings.c`
- [ ] Add bounds checking to config parser
- [ ] Fix name collision bug in `Backup_Perform()` (flat copy)
- [ ] Ensure all `gtk_file_chooser_get_filename()` results are freed
- [ ] Ensure all `gtk_tree_model_get()` string results are freed
- [ ] Add Doxygen function headers to all public functions
- [ ] Commit: "fix: buffer safety + memory leaks"

### Phase 5 — Design Patterns (Day 5)

- [ ] Formalize Facade in `UI_CreateMainWindow()` with private helpers
- [ ] Add `Backup_SetProgressObserver()` for decoupled progress updates
- [ ] Design `BackupStrategy` struct for OCP compliance
- [ ] Document patterns applied in `README.md`
- [ ] Commit: "feat: apply Facade and Observer patterns"

### Phase 6 — Documentation (Day 6)

- [ ] Write full `README.md`
- [ ] Write refactoring report
- [ ] Write this design model guide
- [ ] Tag release: `v2.0-refactored`

---

## Before & After Examples

### Example 1: Settings Loading

#### Before
```c
// Buried inside main.c — no module boundary
void load_settings() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        save_settings();
        return;
    }

    char line[MAX_PATH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *key   = strtok(line, "=");
        char *value = strtok(NULL, "=");
        if (!key || !value) continue;

        if (strcmp(key, "destination") == 0)
            strcpy(settings.backup_destination, value);  // unsafe!
        else if (strcmp(key, "auto_backup") == 0)
            settings.auto_backup = atoi(value);
        else if (strcmp(key, "interval") == 0)
            settings.backup_interval = atoi(value);
    }
    fclose(file);
    mkdir(settings.backup_destination);
}
```

#### After
```c
/**
 * @file  settings.c
 * @brief Configuration persistence — reads and writes backup_config.txt.
 */

#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SETTINGS_MIN_INTERVAL_SECONDS  60
#define SETTINGS_MAX_INTERVAL_SECONDS  86400

BackupSettings g_settings = {
    .destination.destinationPath  = "C:\\Backups",
    .destination.maxSnapshotCopies = 10,
    .timer.autoBackupEnabled       = BACKUP_TIMER_ENABLED,
    .timer.intervalSeconds         = 300,
    .filter.includeSubfolders      = BACKUP_FILTER_INCLUDE,
    .filter.includeHiddenFiles     = BACKUP_FILTER_EXCLUDE,
    .ui.showNotifications          = 1
};

/**
 * @brief Load settings from backup_config.txt.
 *        Creates the file with defaults if it does not exist.
 *        Validates interval range — clamps to [60, 86400] seconds.
 */
void Settings_Load(void) {
    FILE *configFile = fopen(BACKUP_CONFIG_FILENAME, "r");
    if (!configFile) {
        Settings_Save();   /* write defaults on first run */
        return;
    }

    char lineBuffer[BACKUP_MAX_PATH_LENGTH];
    while (fgets(lineBuffer, sizeof(lineBuffer), configFile)) {
        lineBuffer[strcspn(lineBuffer, "\n")] = '\0';

        char *key   = strtok(lineBuffer, "=");
        char *value = strtok(NULL, "=");
        if (!key || !value) { continue; }

        if (strcmp(key, "destination") == 0) {
            strncpy(g_settings.destination.destinationPath, value,
                    BACKUP_MAX_PATH_LENGTH - 1);
            g_settings.destination.destinationPath[BACKUP_MAX_PATH_LENGTH - 1] = '\0';

        } else if (strcmp(key, "auto_backup") == 0) {
            g_settings.timer.autoBackupEnabled = atoi(value);

        } else if (strcmp(key, "interval") == 0) {
            int parsedInterval = atoi(value);
            /* Clamp to valid range — prevents infinite-loop timer */
            if (parsedInterval < SETTINGS_MIN_INTERVAL_SECONDS) {
                parsedInterval = SETTINGS_MIN_INTERVAL_SECONDS;
            }
            if (parsedInterval > SETTINGS_MAX_INTERVAL_SECONDS) {
                parsedInterval = SETTINGS_MAX_INTERVAL_SECONDS;
            }
            g_settings.timer.intervalSeconds = parsedInterval;

        } else if (strcmp(key, "max_copies") == 0) {
            g_settings.destination.maxSnapshotCopies = atoi(value);
        }
    }
    fclose(configFile);

    /* Ensure destination directory exists */
    _mkdir(g_settings.destination.destinationPath);
}
```

**Key improvements:**
- Module boundary enforced by dedicated file
- `strncpy` + null terminator: buffer overflow prevented
- Interval validation: clamps to `[60, 86400]`
- Named constants instead of magic numbers
- Doxygen function header
- Descriptive variable names (`configFile`, `lineBuffer`, `parsedInterval`)

---

### Example 2: File Addition to Queue

#### Before
```c
// Mixed with all other logic in main.c
void add_file_to_list(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return;

    if (!settings.include_hidden) {
        const char *name = strrchr(path, '\\');
        name = name ? name + 1 : path;
        if (name[0] == '.') return;
    }

    GtkTreeIter iter;
    gboolean exists = FALSE;

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(items_list), &iter)) {
        do {
            char *existing;
            gtk_tree_model_get(GTK_TREE_MODEL(items_list), &iter, 0, &existing, -1);
            if (strcmp(existing, path) == 0) {
                exists = TRUE;
                g_free(existing);
                break;
            }
            g_free(existing);
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(items_list), &iter));
    }

    if (!exists) {
        char size_str[32];
        if (st.st_size < 1024)
            sprintf(size_str, "%ld B", st.st_size);
        // ... etc

        GtkTreeIter newIter;
        gtk_list_store_append(items_list, &newIter);
        gtk_list_store_set(items_list, &newIter, 0, path, 1, size_str, 2, time_str, -1);
    }
}
```

#### After
```c
/**
 * @file  fileops.c
 * @brief Backup queue management — add, remove, and clear the GTK file list.
 */

#include "fileops.h"
#include "settings.h"
#include "ui.h"

#define BYTES_PER_KILOBYTE  1024L
#define BYTES_PER_MEGABYTE  (1024L * 1024L)
#define BYTES_PER_GIGABYTE  (1024L * 1024L * 1024L)

/* ── Internal helpers ─────────────────────────────────────────────── */

static void fileops_formatFileSize(long sizeBytes, char *outBuffer, size_t bufferSize) {
    if (sizeBytes < BYTES_PER_KILOBYTE)
        snprintf(outBuffer, bufferSize, "%ld B",   sizeBytes);
    else if (sizeBytes < BYTES_PER_MEGABYTE)
        snprintf(outBuffer, bufferSize, "%.1f KB", sizeBytes / (double)BYTES_PER_KILOBYTE);
    else if (sizeBytes < BYTES_PER_GIGABYTE)
        snprintf(outBuffer, bufferSize, "%.1f MB", sizeBytes / (double)BYTES_PER_MEGABYTE);
    else
        snprintf(outBuffer, bufferSize, "%.1f GB", sizeBytes / (double)BYTES_PER_GIGABYTE);
}

static int fileops_isDuplicate(const char *filePath) {
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(g_fileQueueList), &iter)) {
        return 0;
    }
    do {
        char *existingPath;
        gtk_tree_model_get(GTK_TREE_MODEL(g_fileQueueList), &iter, 0, &existingPath, -1);
        int isDup = (strcmp(existingPath, filePath) == 0);
        g_free(existingPath);   /* always free GTK-allocated strings */
        if (isDup) { return 1; }
    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(g_fileQueueList), &iter));
    return 0;
}

/* ── Public API ───────────────────────────────────────────────────── */

/**
 * @brief  Add a single file to the backup queue.
 *
 * Skips the file if: stat() fails, it is hidden and include_hidden is off,
 * or it is already present in the queue (duplicate check).
 *
 * @param filePath  Absolute path to the file to add.
 */
void FileQueue_AddFile(const char *filePath) {
    struct stat fileStat;
    if (stat(filePath, &fileStat) != 0) { return; }

    /* Apply hidden-file filter */
    if (!g_settings.filter.includeHiddenFiles) {
        const char *fileName = strrchr(filePath, '\\');
        fileName = fileName ? fileName + 1 : filePath;
        if (fileName[0] == '.') { return; }
    }

    if (fileops_isDuplicate(filePath)) { return; }

    char sizeDisplay[32];
    fileops_formatFileSize(fileStat.st_size, sizeDisplay, sizeof(sizeDisplay));

    char modifiedDisplay[64];
    struct tm *modifiedTime = localtime(&fileStat.st_mtime);
    strftime(modifiedDisplay, sizeof(modifiedDisplay), "%Y-%m-%d %H:%M", modifiedTime);

    GtkTreeIter newRow;
    gtk_list_store_append(g_fileQueueList, &newRow);
    gtk_list_store_set(g_fileQueueList, &newRow,
                       0, filePath,
                       1, sizeDisplay,
                       2, modifiedDisplay,
                       -1);
}
```

**Key improvements:**
- Extracted to dedicated file with single responsibility
- `fileops_formatFileSize()` extracted — no repetition, easy to test
- `fileops_isDuplicate()` extracted — hidden complexity made explicit
- `snprintf` instead of `sprintf` — buffer-safe
- `g_free()` called immediately after use — no memory leak
- All global names use `g_` prefix

---

### Example 3: Backup Execution

#### Before
```c
gboolean perform_backup(gpointer data) {
    if (backup_running) return G_SOURCE_REMOVE;

    GtkTreeIter iter;
    gboolean has_items = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(items_list), &iter);
    if (!has_items) {
        update_status("No items to backup", 0.0);
        return G_SOURCE_REMOVE;
    }

    backup_running = TRUE;

    char backup_dir[MAX_PATH];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    sprintf(backup_dir, "%s\\Backup_%04d%02d%02d_%02d%02d",
            settings.backup_destination,
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
            tm->tm_hour, tm->tm_min);
    mkdir(backup_dir);

    // ... 60 more lines mixing copy logic, progress, and logging
}
```

#### After
```c
/**
 * @file  backup.c
 * @brief Backup execution engine — orchestrates file copying and session logging.
 */

#include "backup.h"
#include "settings.h"
#include "ui.h"
#include "utils.h"

/* ── Internal helpers ─────────────────────────────────────────────── */

static void backup_buildTimestampedDirPath(char *outPath, size_t pathSize) {
    time_t now = time(NULL);
    struct tm *localNow = localtime(&now);
    snprintf(outPath, pathSize,
             "%s\\" BACKUP_DIR_PREFIX BACKUP_TIMESTAMP_FORMAT,
             g_settings.destination.destinationPath,
             localNow->tm_year + 1900, localNow->tm_mon + 1, localNow->tm_mday,
             localNow->tm_hour, localNow->tm_min);
}

static int backup_copyFile(const char *sourcePath, const char *destPath, FILE *sessionLog) {
    FILE *sourceFile = fopen(sourcePath, "rb");
    if (!sourceFile) {
        if (sessionLog) {
            fprintf(sessionLog, "FAIL (cannot open source): %s\n", sourcePath);
        }
        return 0;   /* failure */
    }

    FILE *destFile = fopen(destPath, "wb");
    if (!destFile) {
        if (sessionLog) {
            fprintf(sessionLog, "FAIL (cannot create dest): %s\n", destPath);
        }
        fclose(sourceFile);
        return 0;   /* failure */
    }

    char transferBuffer[BACKUP_COPY_BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(transferBuffer, 1, sizeof(transferBuffer), sourceFile)) > 0) {
        fwrite(transferBuffer, 1, bytesRead, destFile);
    }

    fclose(destFile);
    fclose(sourceFile);

    if (sessionLog) {
        fprintf(sessionLog, "OK  %s -> %s\n", sourcePath, destPath);
    }
    return 1;   /* success */
}

/* ── Public API ───────────────────────────────────────────────────── */

/**
 * @brief  Execute a full backup of all queued files.
 *
 * Creates a timestamped directory, copies each file from the queue,
 * and writes backup_log.txt. Updates the GTK progress bar throughout.
 * Sets g_backupInProgress flag to prevent concurrent runs.
 *
 * @param data  Unused (GLib idle callback parameter). Pass NULL.
 * @return      G_SOURCE_REMOVE always (one-shot idle callback).
 */
gboolean Backup_Perform(gpointer data) {
    if (g_backupInProgress) { return G_SOURCE_REMOVE; }

    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(g_fileQueueList), &iter)) {
        UI_UpdateStatus("No items to backup", 0.0);
        return G_SOURCE_REMOVE;
    }

    g_backupInProgress = TRUE;

    /* Count total items for progress calculation */
    int totalFiles = 0;
    GtkTreeIter countIter = iter;
    do { totalFiles++; } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(g_fileQueueList), &countIter));

    /* Create timestamped backup directory */
    char backupDirPath[BACKUP_MAX_PATH_LENGTH];
    backup_buildTimestampedDirPath(backupDirPath, sizeof(backupDirPath));
    _mkdir(backupDirPath);

    /* Open session log */
    char sessionLogPath[BACKUP_MAX_PATH_LENGTH];
    snprintf(sessionLogPath, sizeof(sessionLogPath), "%s\\backup_log.txt", backupDirPath);
    FILE *sessionLog = fopen(sessionLogPath, "w");
    if (sessionLog) {
        time_t startTime = time(NULL);
        fprintf(sessionLog, "Backup started: %s", ctime(&startTime));
    }

    int currentFile = 0, successCount = 0;

    do {
        char *sourceFilePath;
        gtk_tree_model_get(GTK_TREE_MODEL(g_fileQueueList), &iter, 0, &sourceFilePath, -1);

        currentFile++;
        const char *fileName = strrchr(sourceFilePath, '\\');
        fileName = fileName ? fileName + 1 : sourceFilePath;

        char progressMessage[200];
        snprintf(progressMessage, sizeof(progressMessage),
                 "Backing up (%d/%d): %s", currentFile, totalFiles, fileName);
        UI_UpdateStatus(progressMessage, (double)currentFile / totalFiles);

        char destFilePath[BACKUP_MAX_PATH_LENGTH];
        snprintf(destFilePath, sizeof(destFilePath), "%s\\%s", backupDirPath, fileName);

        if (backup_copyFile(sourceFilePath, destFilePath, sessionLog)) {
            successCount++;
        }

        g_free(sourceFilePath);   /* free GTK-allocated string */

    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(g_fileQueueList), &iter));

    if (sessionLog) {
        fprintf(sessionLog, "\nCompleted: %d/%d files successful\n", successCount, totalFiles);
        fclose(sessionLog);
    }

    char completionMessage[200];
    snprintf(completionMessage, sizeof(completionMessage),
             "Backup complete! %d/%d files backed up successfully", successCount, totalFiles);
    UI_UpdateStatus(completionMessage, 1.0);
    UI_ShowNotification("Backup Complete", completionMessage);

    g_backupInProgress = FALSE;
    g_timeout_add_seconds(3, (GSourceFunc)gtk_progress_bar_set_fraction, GINT_TO_POINTER(0));

    return G_SOURCE_REMOVE;
}
```

**Key improvements:**
- Extracted `backup_buildTimestampedDirPath()` — testable in isolation
- Extracted `backup_copyFile()` — single responsibility, returns success/failure
- Explicit error logging for both `fopen` failures
- `snprintf` throughout — buffer safe
- `g_free(sourceFilePath)` — memory leak fixed
- Named constants replace magic numbers
- `g_` prefix on all globals

---

## Conclusion

### Key Takeaways

1. **Consistency is King**: Apply naming and style conventions uniformly across all files
2. **Gradual Migration**: Refactor one module at a time, verify compile after each step
3. **Documentation Matters**: Well-documented code is maintainable code — future-you will thank present-you
4. **SOLID Principles**: Guide every architectural decision — SRP alone eliminates most maintenance pain
5. **Design Patterns**: Use proven solutions for common problems — don't reinvent Observer when GTK already provides it

### Expected Benefits

**Code Quality:**
- Improved readability — any function can be understood in under 2 minutes
- Easier debugging — bugs are isolated to one module's file
- Better testability — `settings.c` can be tested without GTK

**Development Velocity:**
- Faster onboarding — folder structure communicates architecture instantly
- Safer changes — modifying `backup.c` cannot break the window layout
- Easier extension — adding incremental backup does not modify existing code

**Software Engineering Skills:**
- Practical experience with industry-standard modular C architecture
- Understanding of SOLID principles in a real GTK+ application
- Experience applying design patterns in C without OOP

### Next Steps

1. **Review this guide** with your instructor
2. **Create a refactoring branch**: `git checkout -b refactor/modular-structure`
3. **Start with Phase 1** (preparation and planning)
4. **Commit after each phase** with clear messages
5. **Document lessons learned** as you progress

### Resources for Further Learning

- **Books:**
  - *Clean Code* by Robert C. Martin
  - *Design Patterns* by Gang of Four
  - *Code Complete* by Steve McConnell

- **Online:**
  - [GNU C Coding Standards](https://www.gnu.org/prep/standards/standards.html)
  - [GTK+ 3.0 Documentation](https://docs.gtk.org/gtk3/)
  - [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html)

---

**Remember:** The goal isn't perfection on the first try. The goal is continuous improvement and learning to write maintainable, professional code. Each refactoring step makes the codebase a little better, and each lesson learned makes you a better developer.

Good luck with your refactoring journey! 🚀
