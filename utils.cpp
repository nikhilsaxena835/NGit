#include "utils.h"

#define ll long long

using namespace std;


string compute_sha(string &data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)data.c_str(), data.size(), hash);
    
    stringstream ss;
    for (ll i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << (ll)hash[i];
    }
    return ss.str();
}

void write_blob(string &file_path, string &sha1_hash) {
    string content;

    ifstream file(file_path, ios::binary);
    ostringstream content_stream;
    content_stream << file.rdbuf();
    content = content_stream.str();
    file.close();


    string object_dir = ".mygit/objects";
    string object_file_path = object_dir + "/" + sha1_hash;
    gzFile gz_file = gzopen(object_file_path.c_str(), "wb");

    gzwrite(gz_file, content.c_str(), content.size());
    gzclose(gz_file);
}

void hash_object(string &file_path, bool write) {

    ifstream file(file_path, ios::binary);
    if (!file) {
    perror(file_path.c_str());   
    return; 
    }
    
    ostringstream content_stream;
    content_stream << file.rdbuf();
    string content = content_stream.str();
    file.close();

    string sha1_hash = compute_sha(content);

    cout << "Hash of " << file_path << ": " << sha1_hash << endl;
        if (write) {
            write_blob(file_path, sha1_hash);
            cout << "Blob " << sha1_hash << " stored in .mygit/objects/" << endl;
        }
}


void cat_file(string &sha1_hash, string &flag) {
    string object_file_path = ".mygit/objects/" + sha1_hash;
    string tree_file_path = ".mygit/objects/trees/" + sha1_hash;

    string object_file_to_read;
    bool is_tree = false;
    gzFile gz_file;
    string object_content;
    ll filesize;

    struct stat buffer;

    // Blob file
    if (stat(object_file_path.c_str(), &buffer) == 0) {
        object_file_to_read = object_file_path;

        gz_file = gzopen(object_file_to_read.c_str(), "rb");

        char buffer_arr[4096];
        int bytes_read;
        while ((bytes_read = gzread(gz_file, buffer_arr, sizeof(buffer_arr))) > 0) {
            object_content.append(buffer_arr, bytes_read);
        }
        gzclose(gz_file);
    } 
    // Tree file
    else if (stat(tree_file_path.c_str(), &buffer) == 0) {
        object_file_to_read = tree_file_path;
        is_tree = true;

        ifstream file(tree_file_path, ios::binary);
        ostringstream content_stream;
        content_stream << file.rdbuf();
        object_content = content_stream.str();
        file.close();
    } 
    else {
        cout << "Error: Object file not found." << endl;
        return;
    }
    std::filesystem::path p{object_file_to_read};
    filesize = std::filesystem::file_size(p);
    istringstream content_stream(object_content);

    if(flag == "-p")
    {
        if (is_tree) {        
        while (content_stream) {
            string mode, type, hash, name;
            getline(content_stream, mode, ' ');
            if (mode.empty()) break;

            getline(content_stream, type, ' ');
            getline(content_stream, hash, ' '); 
            getline(content_stream, name);

            cout << "Mode: " << mode << ", Type: " << type << ", Hash: " << hash << ", Name: " << name << "\n";
        }
    } else {
        cout << content_stream.rdbuf();
    }

    }

    if(flag == "-t")
    {
        if(is_tree)
        cout << "Object is tree object" << endl;
        else{
            cout << "Object is blob object" << endl;
        }
    }

    if(flag == "-s")
    {
        cout << "File size is : " << filesize << " bytes" << endl;
    }
}

void print_tree(string& file_hash, bool flag) {
    string tree_file_path = ".mygit/objects/trees/" + file_hash;

    ifstream file(tree_file_path, ios::binary);
    ostringstream content_stream;
    content_stream << file.rdbuf();
    file.close();

    istringstream stream(content_stream.str());
    while (stream) {
        string mode, type, hash, name;

        if (!getline(stream, mode, ' ') || mode.empty()) break;
        if (!getline(stream, type, ' ')) break;
        if (!getline(stream, hash, ' ')) break;
        if (!getline(stream, name)) break;

        if (!flag) {
            cout << "Mode: " << mode << ", Type: " << type 
                 << ", Hash: " << hash << ", Name: " << name << "\n";
        } else {
            cout << name << endl;
        }
    }
}


string write_tree(string &dir_path, bool write_flag) {
    vector<string> tree_entries;
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        perror("opendir");
        return "";
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        string entry_name = entry->d_name;
        if (entry_name == "." || entry_name == ".." || entry_name == ".mygit") {
            continue;
        }

        string full_path = dir_path + "/" + entry_name;
        struct stat entry_stat;
        if (stat(full_path.c_str(), &entry_stat) == -1) {
            perror("stat");
            closedir(dir);
            return "";
        }

        string mode = to_string(entry_stat.st_mode & 0777);

        if (S_ISDIR(entry_stat.st_mode)) {
            string subtree_hash = write_tree(full_path, write_flag); 
            tree_entries.push_back(mode + " tree " + subtree_hash + " " + entry_name);
        } 
        else if (S_ISREG(entry_stat.st_mode)) {
            ifstream file(full_path, ios::binary);
            stringstream buffer;
            buffer << file.rdbuf();
            string file_content = buffer.str();
            string blob_hash = compute_sha(file_content);
            if(write_flag)
            write_blob(full_path, blob_hash);

            tree_entries.push_back(mode + " blob " + blob_hash + " " + entry_name);
        }
    }
    closedir(dir);

    stringstream tree_content;
    for (const string &entry : tree_entries) {
        tree_content << entry << '\n';
    }

    string temp = tree_content.str();
    string tree_hash = compute_sha(temp);

    string tree_path = ".mygit/objects/trees/" + tree_hash;
    ofstream tree_file(tree_path, ios::binary | ios::trunc);
    if (!tree_file) {
        perror("ofstream at write-tree");
        return "";
    }
    tree_file << tree_content.str();

    return tree_hash;
}

void restore_tree(const string &tree_hash, const string &dir_path) {
    string tree_path = ".mygit/objects/trees/" + tree_hash;
    ifstream tree_file(tree_path);

    //string type, size;
    //getline(tree_file, type); 
    //getline(tree_file, size);
    //cout << "reading tree" << tree_path << endl;
    string line;
    while (getline(tree_file, line)) {
        stringstream ss(line);
        string mode, hash, name, type;
        ss >> mode >> type >> hash >> name;

        string full_path = dir_path + "/" + name;
        //cout << type << name << full_path << endl;
        if (type == "tree") {
            mkdir(full_path.c_str(), 0755);  
            restore_tree(hash, full_path);   
        } else if (type == "blob") {
            string blob_path = ".mygit/objects/" + hash;
            ifstream blob_file(blob_path);
            if (!blob_file) {
                cout << "Error opening blob object: " << hash << endl;
                return;
            }

            string blob_type, blob_size;
            getline(blob_file, blob_type);
            getline(blob_file, blob_size);   
            gzFile gz_file = gzopen(blob_path.c_str(), "rb");
            //cout << "creating file " << full_path << endl;
            char buffer_arr[4096];
            int bytes_read;
            string object_content;
            ofstream output_file(full_path, ios::binary);

            while ((bytes_read = gzread(gz_file, buffer_arr, sizeof(buffer_arr))) > 0) {
            output_file.write(buffer_arr, bytes_read);
        }

            gzclose(gz_file);   
            //output_file << blob_file.rdbuf(); 
        }
    }
}

string read_file(string &file_path) {
    ifstream file(file_path, ios::binary);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void stage_file(string &file_path, ofstream &index_file) {
    string file_content = read_file(file_path);
    if (file_content.empty()) {
        cout << "Check file-name : " << file_path << endl;
        return;
        }

    string file_sha = compute_sha(file_content);
    
    index_file << file_path << " " << file_sha << endl;
    write_blob(file_path, file_sha);
    //string blob_path = ".mygit/objects/" + file_sha;
    //ofstream blob_file(blob_path, ios::binary);

    //blob_file << "blob\n" << file_content.size() << "\n" << file_content;
    //blob_file.close();
}


void stage_directory(const string &dir_path, ofstream &index_file) {
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        string entry_name = entry->d_name;
        if (entry_name == "." || entry_name == ".." || entry_name == ".mygit") continue;

        string full_path = dir_path + "/" + entry_name;
        struct stat entry_stat;
        if (stat(full_path.c_str(), &entry_stat) == -1) {
            perror("stat");
            closedir(dir);
            return;
        }

        if (S_ISDIR(entry_stat.st_mode)) {
            stage_directory(full_path, index_file);
        } else if (S_ISREG(entry_stat.st_mode)) {
            stage_file(full_path, index_file);
        }
    }
    closedir(dir);
}


void add_files(vector<string> &files) {
    ofstream index_file(".mygit/index", ios::trunc);
    if (files.size() == 1 && files[0] == ".") {
        stage_directory(".", index_file);  
    } 
    else {
        for (string &file : files) {
            struct stat file_stat;
            if (stat(file.c_str(), &file_stat) == -1) {
                cout << "File does not exist: " << file << endl;
                continue;
            }
            if (S_ISDIR(file_stat.st_mode)) {
                stage_directory(file, index_file);  
            } 
            else {                          //if (S_ISREG(file_stat.st_mode))
                stage_file(file, index_file);  
            } 
    }
    }
    cout << "Add files completed" << endl;
    index_file.close();
}


string get_current_timestamp() {
    time_t now = time(0);
    char buf[80];
    struct tm tstruct;
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return string(buf);
}

string get_parent_commit_sha() {
    ifstream head_file(".mygit/HEAD");
    string parent_sha = "";
    if (head_file.is_open()) {
        getline(head_file, parent_sha);
        head_file.close();
    }
    return parent_sha;
}

void update_head(const string &commit_sha) {
    ofstream head_file(".mygit/HEAD", ios::trunc);
    if (head_file.is_open()) {
        head_file << commit_sha;
        head_file.close();
    }
}


void commit(string &message) {
    string parent_sha = get_parent_commit_sha();

    string curr = ".";
    string tree_sha = write_tree(curr, true);

    stringstream commit_content;
    commit_content << tree_sha << "\n";
    commit_content << parent_sha << "\n";
    
    commit_content << "Nikhil nikhil.saxena@students.iiit.ac.in " << get_current_timestamp() << "\n";
    commit_content << "message " << message << "\n";

    string commit_content_str = commit_content.str();
    string commit_sha = compute_sha(commit_content_str);

    string commit_path = ".mygit/refs/" + commit_sha;
    ofstream commit_file(commit_path);

    commit_file << "commit\n" << commit_content_str;
    commit_file.close();

    update_head(commit_sha);

    string log_file_path = ".mygit/log";
    ofstream log_file(log_file_path, ios::app);
  
    log_file << commit_sha << "\n";
    log_file.close();

    cout << "New commit SHA: " << commit_sha << endl;
}

unordered_set<string> parse_tree(const string &tree_sha) {
    unordered_set<string> files_in_tree;
    string tree_path = ".mygit/objects/trees/" + tree_sha;

    ifstream tree_file(tree_path);
    if (!tree_file.is_open()) {
        //cerr << "Error opening tree file: " << tree_path << endl;
        return files_in_tree;
    }

    string line;
    while (getline(tree_file, line)) {
        stringstream ss(line);
        string mode, type, sha, name;
        ss >> mode >> type >> sha >> name;
        
        if (!name.empty()) {
            files_in_tree.insert(name);
        }
    }
    tree_file.close();
    return files_in_tree;
}

void checkout(string &commit_sha) {
    string commit_path = ".mygit/refs/" + commit_sha;
    ifstream commit_file(commit_path);
    if (!commit_file.is_open()) {
        cout << "Error: Check SHA or presence of commit file." << endl;
        return;
    }

    string line;
    string tree_sha;
    bool is_first_line = true;

    while (getline(commit_file, line)) {
    if (is_first_line) {
        is_first_line = false;
        continue;
    }

    stringstream ss(line);
    string key;

    if (tree_sha.empty()) {
        ss >> tree_sha;
        continue;
    }

    ss >> key;
    if (key == "parent") {
        string parent_sha;
        ss >> parent_sha;
        //cout << "Parent SHA: " << parent_sha << endl;
    }
}
    if (tree_sha.empty()) {
        return;
    }
    
    unordered_set<string> files_in_commit_tree = parse_tree(tree_sha);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string name = ent->d_name;
            if (name != "." && name != ".." && name != ".mygit") {
                if (files_in_commit_tree.count(name)) {
                    if (ent->d_type == DT_DIR) {
                        string command = "rm -rf " + name;
                        system(command.c_str());
                    } else {
                        unlink(name.c_str());
                    }
                }
            }
        }
        closedir(dir);
    }

    restore_tree(tree_sha, ".");
    cout << "Checked out commit : " << commit_sha << endl;
}

vector<string> read_commit_shas(string &log_file_path) {
    vector<string> commit_shas;
    ifstream log_file(log_file_path);
    
    if (!log_file) {
        //cerr << "Error opening log file: " << log_file_path << endl;
        return commit_shas;
    }

    string sha;
    while (getline(log_file, sha)) {
        if (!sha.empty()) {
            commit_shas.push_back(sha);
        }
    }
    log_file.close();
    reverse(commit_shas.begin(), commit_shas.end());
    return commit_shas;
}

void display_commit_details(string &commit_sha) {
    string commit_file_path = ".mygit/refs/" + commit_sha;
    ifstream commit_file(commit_file_path);


    string line;
    string parent_sha, author_name, author_email, timestamp, commit_message;

    getline(commit_file, line);
    getline(commit_file, line);  

    if (getline(commit_file, line) ) {
        istringstream parent_stream(line);
        //string parent_keyword;
        parent_stream >> parent_sha;
    }

    if (getline(commit_file, line)) {
        istringstream committer_stream(line);
        committer_stream >> author_name >> author_email >> timestamp;

        string additional_token;
        while (committer_stream >> additional_token) {
            timestamp += " " + additional_token;
        }
        
        if (!author_email.empty() && author_email.front() == '<' && author_email.back() == '>') {
            author_email = author_email.substr(1, author_email.size() - 2);
        }
    }

    if (getline(commit_file, line) && line.substr(0, 7) == "message") {
        commit_message = line.substr(8);  // Remove the "message " prefix
    }

    cout << "Commit SHA: " << commit_sha << endl;
    if (!parent_sha.empty()) {
        cout << "Parent SHA: " << parent_sha << endl;
    }
    cout << "Author: " << author_name << endl;
    cout << "Email: " << author_email << endl;
    cout << "Timestamp: " << timestamp << endl;
    cout << "Message: " << commit_message << endl;
    cout << "<----------------------------------->" << endl;

    commit_file.close();
}

void log_command() {
    string log_file_path = ".mygit/log";
    
    vector<string> commit_shas = read_commit_shas(log_file_path);
    
    for (string &commit_sha : commit_shas) {
        display_commit_details(commit_sha);
    }
}