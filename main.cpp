//g++ main.cpp utils.cpp -Wno-deprecated-declarations -lssl -lcrypto -lz -o mygit


#include <cstring>
#include "utils.h"

using namespace std;


void init()
{
    mkdir(".mygit", 0755);
    mkdir(".mygit/objects", 0755);
    mkdir(".mygit/objects/trees", 0755);
    mkdir(".mygit/refs", 0755);
}



bool checkCommand(int argc, char *argv[])
{
    if(argc == 2 && strcmp(argv[1], "init") == 0)
    {
        init();
        return true;
    }
    //  hash-object [-w] filepath
    if((argc == 3 || argc == 4) && strcmp(argv[1], "hash-object") == 0)
    {
        bool write;
        if(argc == 3)
        {
            write = false;
            string filepath = argv[2];
            hash_object(filepath, write);
        }
        else{
            write = true;
            string filepath = argv[3];
            hash_object(filepath, write);
        }        
        return true;
    }
    // cat-file [-p/-t/-s] sha
    if((argc == 4) && strcmp(argv[1], "cat-file") == 0)
    {
        string flag = argv[2];
        string sha = argv[3];
        cat_file(sha, flag);
        return true;
    }

    //  write-tree 
    if(argc == 2 && strcmp(argv[1], "write-tree") == 0)
    {
        string curr = ".";
        cout << write_tree(curr, false) << endl;
        return true;
    }
    //ls-tree [--name-only] sha
    if((argc == 3 || argc == 4) && strcmp(argv[1], "ls-tree") == 0)
    {
        if(argc == 4)
        {
            string sha = argv[3];
            print_tree(sha, true);
            return true;
        }
        else{
            string sha = argv[2];
            print_tree(sha, false);
            return true;
        }
        
    }
    // add [. / *{filenames}]
    if(strcmp(argv[1], "add") == 0)
    {
        int count = argc - 2;
        vector<string> params;

        for(int i = 0; i < count; i++)
        params.push_back(argv[i+2]);

        add_files(params);
        return true;
    }

    // commit [-m "message"]
    if((argc == 4 || argc == 2) && strcmp(argv[1], "commit") == 0)
    {
        string message = "";
        if(argc == 4)
        message = argv[3];
        
        commit(message);
        return true;
    }

    // checkout sha
    if(argc == 3 && strcmp(argv[1], "checkout") == 0)
    {
        string sha = argv[2];
        checkout(sha);
        return true;
    }

    // log
    if(argc == 2 && strcmp(argv[1], "log") == 0 )
    {
        log_command();
        return true;
    }
    return false;
}


int main(int argc, char * argv[])
{
    //cout << argc << endl;
    //cout << argv[1] << endl;
    if(!checkCommand(argc, argv));
    {
        //cout << "Check the command syntax" << endl;
        return -1;
    }

    return 0;
}