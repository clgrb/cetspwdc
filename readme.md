# CETSP-WDC instances generator
This repository contains instances and two instance generators for the Close-Enough Traveling Salesman Problem with Wake-up and Data Collection (CETSP-WDC).

## Overview
This repository contains:

- _dataset/_ - benchmark instances folder used in the paper
- _config.h_ - directory configuration file
- _readme.md_ - instruction file
- _lib/_ - libraries source code directory
- _src/_ - execution files source code directory
- _CMakelists.txt_ - cmake configuration file

## Requirement
- CMake >= 3.21

### Instances source
The CETSP-WDC benchmark instances are generated from the Mennell's instances, available [here](http://hdl.handle.net/1903/9822).
Please place the instances into the folder _dataset/mennell_

## Build instructions
Then comile the code using the following commands:

```bash
cmake -Bbuild
cmake --build build
```

After compilation, the `build` folder will contain two executable files:

- **behdani** - CETSP-WDC instance generator based on the Behdani procedure
- **mennell** - CETSP-WDC instance generator based on Mennell's instances

### Behdani-based instances generator
The program generates CETSP-WDC instances using the Behdani procedure and stores them in `BEHDANI_WDC_DIR`.

The `BEHDANI_WDC_DIR` parameter can be adjusted in the `config.h` file.
The program does not require any additional parameters and can be executed as follows:

```bash
build/behdani
```

### Mennell's instances generator
The program generates CETSP-WDC instances using Mennell's instances and stores them in `MENNELL_WDC_DIR`.

The `MENNELL_WDC_DIR` and `MENNELL_CETSP_DIR` parameters can be adjusted in the `config.h` file.
The program does not require any additional parameters and can be executed as follows:

```bash
build/mennell
```

