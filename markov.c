/*
 * Markov chain random text generator.
 *
 * This implementation is derived from Kernighan and Pike's PoP book.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
    NPREFIXES = 2,
    NSTATES = 4096,
    MAX_OUTPUT = 10000
};

typedef struct State State;
typedef struct Suffix Suffix;

struct State {
    char *prefix[NPREFIXES];
    Suffix *suffix;
    State *next;
};

struct Suffix {
    char *word;
    Suffix *next;
};

State *statetable[NSTATES];

const unsigned int MULTIPLIER = 32;

char NONWORD[] = "\n";

/*
 * hash: computes a hash value.
 */
unsigned int hash(char *prefix[NPREFIXES])
{
    unsigned int h = 0;
    for (int i = 0; i < NPREFIXES; i++) {
        for (unsigned char *p = (unsigned char *) prefix[i]; *p != '\0'; p++) {
            h = MULTIPLIER * h + *p;
        }
    }

    return h % NSTATES;
}

/*
 * lookup: search for a prefix and create it if requested.
 */
State* lookup(char *prefix[NPREFIXES], bool create)
{
    unsigned int h = hash(prefix);

    /*
     * Search for the matching prefix in the State table and subsequent State
     *     lists.
     */
    State *sp;
    for (sp = statetable[h]; sp != NULL; sp = sp->next) {

        /*
         * Match each of the prefix words.
         */
        int i;
        for (i = 0; i < NPREFIXES; i++) {
            if (strcmp(prefix[i], sp->prefix[i]) != 0) {
                break;
            }
        }

        /*
         * Confirm a match was found.
         */
        if (i == NPREFIXES) {
            return sp;
        }
    }

    /*
     * A State for the prefix was not found, so we will create it.
     */
    if (create) {
        sp = malloc(sizeof(State));

        /*
         * Fill in the prefix words.
         */
        for (int j = 0; j < NPREFIXES; j++) {
            sp->prefix[j] = prefix[j];
        }

        /*
         * Leave the suffix NULL for the caller to set.
         */
        sp->suffix = NULL;

        /*
         * Insert the new State at the beginning of the State list.
         */
        sp->next = statetable[h];
        statetable[h] = sp;
    }

    return sp;
}

/*
 * addsuffix: create a new Suffix and add it to the State.
 */
void addsuffix(State *sp, char *word)
{
    Suffix *suffix = malloc(sizeof(Suffix));

    suffix->word = word;

    /*
     * Insert the new Suffix at the beginning of the State's Suffix list.
     */
    suffix->next = sp->suffix;
    sp->suffix = suffix;
}

/*
 * add: add word to a Suffix list.
 */
void add(char *prefix[NPREFIXES], char *word)
{
    State *sp = lookup(prefix, true);
    addsuffix(sp, word);

    /*
     * Shift the prefix phrase forward.
     *
     *     prefix[0] becomes prefix[1] and prefix[1] becomes word
     */
    memmove(prefix, prefix+1, (NPREFIXES-1) * sizeof(prefix[0]));
    prefix[NPREFIXES-1] = word;
}

/*
 * build: read input and build table.
 *
 * Note, this doesn't safely handle words > 100 characters!
 */
void build(char *prefix[NPREFIXES], FILE *file)
{
    char buffer[100];
    while (fscanf(file, "%s", buffer) != EOF) {
        add(prefix, strdup(buffer));
    }
}

/*
 * generate: generate and print the output one word at a time.
 */
void generate()
{
    char *prefix[NPREFIXES];

    /*
     * Setup initial prefix.
     */
    for (int i = 0; i < NPREFIXES; i++) {
        prefix[i] = NONWORD;
    }

    /*
     * Seed rand function to pick Suffixes later.
     */
    srand(time(NULL));

    char *word;
    for (int i = 0; i < MAX_OUTPUT; i++) {

        /*
         * Pick a random Suffix for prefix.
         */
        int nmatch = 0;
        for (Suffix *suf = lookup(prefix, false)->suffix; suf != NULL; suf = suf->next) {
            if (rand() % ++nmatch == 0) {
                word = suf->word;
            }
        }

        if (strcmp(word, NONWORD) == 0) {
            break;
        }

        printf("%s\n", word);

        /*
         * Shift the prefix phrase forward.
         *
         *     prefix[0] becomes prefix[1] and prefix[1] becomes word
         */
        memmove(prefix, prefix+1, (NPREFIXES-1) * sizeof(prefix[0]));
        prefix[NPREFIXES-1] = word;
    }
}

int main()
{
    char *prefix[NPREFIXES];

    /*
     * Setup initial prefix.
     */
    for (int i = 0; i < NPREFIXES; i++) {
        prefix[i] = NONWORD;
    }

    build(prefix, stdin);
    add(prefix, NONWORD);

    generate();

    return 0;
}

