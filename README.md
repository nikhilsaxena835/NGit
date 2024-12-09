# NGit  
**Git Version Control: Linux**  

A version control system that mimics fundamental operations of Git.  

## How to compile this project?  
```bash
g++ main.cpp utils.cpp -Wno-deprecated-declarations -lssl -lcrypto -lz -o mygit
```

## How to run this project?

    On the terminal, use ./mygit as the executable, followed by the command and parameters (optionally).
    The following commands are available:
        - init
        - hash-object
        - write-tree
        - cat-file
        - ls-tree
        - commit
        - add
        - checkout
        - log

## How is it implemented?

All features are implemented in the utils.cpp file. The main.cpp file serves as the starting point, which calls the respective functions based on the command passed as a command-line argument.

    * zlib is used for compression.
    * OpenSSL is used for calculating the SHA of the file.
