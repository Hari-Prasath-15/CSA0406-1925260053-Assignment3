# CSA0406 – Operating Systems Assignment 3

## Student Details

- **Name:** Hari Prasath V G
- **Register Number:** [1925260053]
- **Course:** B.Tech – Artificial Intelligence and Machine Learning
- **Subject Code:** CSA0406
- **Assignment:** Operating Systems – Assignment 3

## Project Description

This repository contains the source code, experiment scripts, execution results, screenshots, and documentation for the seven questions in Operating Systems Assignment 3.

The programs are primarily implemented in C and executed using GCC in Ubuntu Linux through Windows Subsystem for Linux (WSL).

## Assignment Questions

| Question | Topic |
|---|---|
| Question 1 | Linking, Loading and Address Binding |
| Question 2 | Paging |
| Question 3 | Segmentation |
| Question 4 | Virtual Memory and Demand Paging |
| Question 5 | Page-Replacement Techniques |
| Question 6 | Disk Management |
| Question 7 | Memory-Management Plan from First Principles |

## Repository Structure

```text
CSA0406-19125260053-Assignment3/
│
├── Question - 1/
│   ├── Library/
│   ├── Screenshots/
│   ├── Scripts/
│   └── Source/
│
├── Question - 2/
│   ├── Screenshots/
│   ├── Scripts/
│   └── Source/
│
├── Question - 3/
│   ├── Screenshots/
│   ├── Scripts/
│   └── Source/
│
├── Question - 4/
│   ├── Scripts/
│   └── Source/
│
├── Question - 5/
│   ├── Result/
│   ├── Screenshots/
│   ├── Scripts/
│   └── Source/
│
├── Question - 6/
│   ├── Result/
│   ├── Screenshots/
│   ├── Scripts/
│   └── Source/
│
├── Question - 7/
│   ├── Screenshots/
│   ├── results/
│   ├── scripts/
│   └── source/
│
├── Assignment 3 Report.docx
└── README.md
```

## Technologies and Tools Used

- **Programming Language:** C
- **Compiler:** GCC
- **Operating System:** Ubuntu Linux (WSL)
- **Code Editor:** Visual Studio Code
- **Version Control:** Git and GitHub
- **Scripting:** Bash
- **Additional Tools:** Python and Linux system utilities

## Compilation and Execution

C programs can generally be compiled using:

```bash
gcc -Wall -Wextra -std=c11 program.c -o program
```

Execute the compiled program:

```bash
./program
```

For questions containing experiment scripts:

```bash
chmod +x Scripts/*.sh
./Scripts/run_experiments.sh
```

For Question 7, the script directory uses lowercase:

```bash
chmod +x scripts/*.sh
./scripts/run_experiments.sh
```

Refer to the individual question folders for their specific compilation and execution instructions.

## Folder Description

- **Library:** Contains libraries required for Question 1.
- **Source / source:** Contains C source code.
- **Scripts / scripts:** Contains scripts used to compile and execute experiments.
- **Result / results:** Contains generated experiment outputs.
- **Screenshots:** Contains screenshots documenting program execution and results.

## Report

The consolidated assignment report is available as:

`Assignment 3 Report.docx`

## Conclusion

This repository documents the practical implementation and execution of seven Operating Systems assignment questions. It includes source code, experiment scripts, execution results, screenshots, and the consolidated report.
