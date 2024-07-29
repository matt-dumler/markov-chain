/*
 * Markov chain random text generator.
 *
 * This implementation is derived from Kernighan and Pike's PoP book.
 */

#include <time.h>

#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

const unsigned int NPREFIXES = 2;
const unsigned int MAX_OUTPUT = 10000;

typedef deque<string> Prefix;

map<Prefix, vector<string>> statetable;

const char NONWORD[] = "\n";

void add(Prefix &prefix, const string &suffix)
{
    if (prefix.size() == NPREFIXES) {
        statetable[prefix].push_back(suffix);
        prefix.pop_front();
    }

    prefix.push_back(suffix);
}

void build(Prefix &prefix, istream &in)
{
    string buffer;

    while (in >> buffer) {
        add(prefix, buffer);
    }
}

void generate()
{
    Prefix prefix;

    for (int i = 0; i < NPREFIXES; i++) {
        add(prefix, NONWORD);
    }

    srand(time(NULL));

    for (int i = 0; i < MAX_OUTPUT; i++) {
        vector<string> &suffix = statetable[prefix];

        const string &word = suffix[rand() % suffix.size()];

        if (word == NONWORD) {
            break;
        }

        cout << word << endl;

        prefix.pop_front();
        prefix.push_back(word);
    }
}

int main()
{
    Prefix prefix;

    for (int i = 0; i < NPREFIXES; i++) {
        add(prefix, NONWORD);
    }

    build(prefix, cin);
    add(prefix, NONWORD);

    generate();

    return 0;
}

