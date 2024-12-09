#ifndef UTILS_H
#define UTILS_H

#include <openssl/sha.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>    
#include <unistd.h>        
#include <vector>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>
#include <string.h>
#include <algorithm>
#include <unordered_set>

using namespace std;

void hash_object(string &file_path, bool write);
void cat_file(string &sha1_hash, string &flag);
string write_tree(string &dir_path, bool write);
void print_tree(string& file_hash, bool flag);
void add_files(vector<string> &files);
void commit(string &message);
void checkout(string &commit_sha);
void log_command();

#endif 
