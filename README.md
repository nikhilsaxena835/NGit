
Git Version Control : Linux
# NGit
A version control system that mimics fundamental operations of Git.

How to compile this project ?
	g++ main.cpp utils.cpp -Wno-deprecated-declarations -lssl -lcrypto -lz -o mygit

How to run this project ?
	1) On the terminal use ./mygit as the executable, followed by the command and parameters (optionally).
	2) Following commands are available : init, hash-object, write-tree, cat-file, ls-tree, commit, add, checkout and log. 

How is it implemented ?
	All of the features are implemented in the utils.cpp file. The main.cpp file serves as the starting point which calls the respective functions based on the command passed as command line argument. zlib is used for compression and OpenSSL is used for calculating the SHA of the file. 
