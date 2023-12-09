/* Alan Khalili 10/12/2023
 * I wrote prog4.cpp to take in a set of usernames/passwords and either create an encripted set of passwords or check if another set matches up.
 */

//Here I delcared the libraries I would be using.
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sstream>
#include <iomanip>


using namespace std;

typedef unsigned long ulong;
//Here is the credentials sctruct provided. I did not touch it.
struct credentials {
    void set_salt(string &);
    void set_hash(string &);

    void operator=(const credentials &);
    bool operator==(const credentials &);

    string salt;
    ulong password_hash;
};
//Here is my set_salt function.
void credentials::set_salt(string &username) {
    salt = "g0b1g0rAnge";
    //I looped through the entire string and garbles it using 0x07
    for (ulong i = 0; i < salt.size(); i++) {
        char newChar = salt[i] + (username[i%username.size()] & 0x07);
        if (std::isalnum(newChar)) { //checked for alphanumerical
            salt[i] = newChar;
        } else { //if not set it to *
            salt[i] = '*';
        }
    }
}
//Here is my set_hash function.
void credentials::set_hash(string &password) {
    password += salt; //I added salt to the end of password.
    password_hash = 0; //Initialized hash to 0 to prevent an error I was reciving
    //I copied this bit of code from the handout on canvas, adjusting variable names to match my use case.
    const char *c = password.c_str();
    while (*c)
        password_hash = ((password_hash << 5) | (password_hash >> 27)) + *c++; //called the bitwise operation to turn it into an int.
}
//Here is my = operator overload.
void credentials::operator=(const credentials &cred) {
    salt = cred.salt;
    password_hash = cred.password_hash;
}
//Here is my == overload.
bool credentials::operator==(const credentials &cred) {
    return (salt == cred.salt) && (password_hash == cred.password_hash);
}
//here is my input overload
istream &operator>>(istream &in, credentials &login) {
    string hex_password; //I first declared a string since psswd.txt contained the hex version
    in >> login.salt >> hex_password;
    login.password_hash = stoul(hex_password, nullptr, 16); //I then took in the hex and converted it back to a ulong
    return in;
}
//Here is my output overload.
ostream &operator<<(ostream &out, const credentials &login) {
    out << left << login.salt << " " << hex << login.password_hash;
    return out;
}

typedef std::unordered_map<std::string, credentials> hashtable;
//Here is my write_hashtable function.
void write_hashtable(hashtable &H, bool verbose) {
    string username, password;
    int slot = 0;//Slot is a counter for verbose.
    if(verbose){ //Here is where I start to handle verbose. I noticed sprog was printing out an extra slot, so I added one before to match.
        //I just print out my slot, followed by the number of buckets, and the ratio.
        std::cout << "** S = " << setw(4) << slot;
        std::cout << " N = " << setw(4) << H.bucket_count();
        std::cout << " : load = " << fixed << setprecision(2)
                  << static_cast<double>(slot) / static_cast<double>(H.bucket_count()) << "\n";
    }
    slot++;
    while(cin >> username >> password) { //Here I am taking in the data from stdin
        //I call my funcitons and set my variables
        H[username].set_salt(username);
        H[username].set_hash(password);

        if(verbose){
            std::cout << "** S = " << setw(4) << slot;
            std::cout << " N = " << setw(4) << H.bucket_count();
            std::cout << " : load = " << fixed << setprecision(2)
                      << static_cast<double>(slot) / static_cast<double>(H.bucket_count()) << "\n";
        }
        slot++;
    }

    //here I start writing to passwd.txt
    ofstream fout;
    fout.open("passwd.txt");
    if(!fout.is_open()) {
        cerr << "Can't open file" << endl;
    }
    hashtable::iterator it = H.begin(); //I make an iterator to go through H
    while(it != H.end()) {
        fout << left << setw(10) <<  it->first << " " << it->second << endl; //Here i use my << operator overload to print out credentials.
        ++it;
    }
    fout.close();

    if (verbose) { //here is verbose again.
        std::cout << std::endl;
        //I just loop through the map and print out the bucket, their number of values, and the values themselves
        for (size_t i = 0; i < H.bucket_count(); ++i) {
            std::cout << std::setw(6) << i << std::setw(5) << H.bucket_size(i);

            for (hashtable::local_iterator it = H.begin(i); it != H.end(i); ++it) {
                std::cout << " " << it->first;
            }
            cout << endl;
        }
        std::cout << endl;
    }
}
//Here is my read hashtable function
void read_hashtable(hashtable &H, bool verbose) {
    ifstream fin("passwd.txt");
    if (!fin) {
        cerr << "Unable to open passwd.txt for reading." << endl;
        return;
    }
    //doing the same stuff for verbose as the last
    int slot = 0;
    if(verbose){
        std::cout << "** S = " << setw(4) << slot;
        std::cout << " N = " << setw(4) << H.bucket_count();
        std::cout << " : load = " << fixed << setprecision(2)
                  << static_cast<double>(slot) / static_cast<double>(H.bucket_count()) << "\n";
    }
    slot++;
    string username, line;
    while (getline(fin, line)) { //Here I am taking in the line, parsing with string stream, then putting it in H
        stringstream iss(line);
        iss >> username >> H[username];
        if(verbose){
            std::cout << "** S = " << setw(4) << slot;
            std::cout << " N = " << setw(4) << H.bucket_count();
            std::cout << " : load = " << fixed << setprecision(2)
                      << static_cast<double>(slot) / static_cast<double>(H.bucket_count()) << "\n";
        }
        slot++;
    }
    fin.close();
    //Here I just print verbose again
    if (verbose) {
        std::cout << std::endl;

        for (size_t i = 0; i < H.bucket_count(); ++i) {
            std::cout << std::setw(6) << i << std::setw(5) << H.bucket_size(i);

            // Iterate over elements in each bucket
            for (hashtable::local_iterator it = H.begin(i); it != H.end(i); ++it) {
                std::cout << " " << it->first;
            }
            cout << endl;
        }
        std::cout << endl;
    }
}
//Here is my main
int main(int argc, char *argv[]) {
    //Here I parse the command line
    bool create, check, verbose;
    float load = 1.0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-create") {
            create = true;
            check = false;
        } else if (arg == "-check") {
            check = true;
            create = false;
        } else if (arg == "-load") {
            load = std::stof(argv[i + 1]);
            i += 1;
        } else if (arg == "-verbose") {
            verbose = true;
        } else {
            //Here I printed to cerr instead of cout, so if the user was piping the file it would show up in the terminal.
			cerr << "usage: ./Prog4 -create|check [-load Z] [-verbose] < logins.txt" << std::endl;
            return 1;
        }
    }
    if (!create && !check) {

        std::cerr << "usage: ./Prog4 -create|check [-load Z] [-verbose] < logins.txt" << std::endl;
        return 1;
    }
    //here I declare my instance of hashtable, H.
    hashtable H;
    H.max_load_factor(load); //set max load factor
    //split it up into create and check.
    if (create) {
        write_hashtable(H, verbose);
    } else if (check) {
        read_hashtable(H, verbose);
        string username, password;
        //parse stdin
        while (cin >> username >> password) {
            if (H.find(username) == H.end()) { //if the key DNE return this
                cout << left << setw(11) << username << "bad username" << endl;
            } else {
                //make an instance of credentials and do the proper functions.
                credentials copy = H[username];
                copy.set_salt(username);
                copy.set_hash(password);

                if (H[username] == copy) { //if it matches up, pass
                    cout << left << setw(11) << username << "access granted" << endl;
                } else { //otherwise fail
                    cout << left << setw(11) << username << "bad password" << endl;
                }
            }
        }
    }
    return 0;
}
