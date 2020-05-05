# Symple File System

Symple File System project for the "operating system" course at La Sapienza University of Rome.
For the final report (in Italian) click here:
- **[Final report](./project_report.pdf)**

To play with the interactive shell, go to the code directory, and run:
```
make shell
./shell
```
The instructions will appear o top

---
## Professor instructions
Implement a file system interface using binary files
  - The file system reserves the first part of the file to store:
    - a linked list of free blocks
    - linked lists of file blocks
    - a single global directory

  - Blocks are of two types
    - data blocks: "random" information
    - directory blocks: contain a sequence of structs of type "directory_entry", containing the blocks where the files in that folder start and if they are directory themselves

---
## Authors
* **[Manuel Ivagnes](https://www.linkedin.com/in/manuel-ivagnes-4a5ba018b)**
* **[Valerio Coretti](https://www.linkedin.com/in/valerio-coretti-2913721a3)**
