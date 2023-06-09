#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
	nPrefix = 2,
	hashTblSize = 4093,
	maxOutput = 200
};

char* NONWORD = "\n";

typedef struct Suffix {
	char* word;
	struct Suffix* next;
} Suffix;

typedef struct State {
	char* prefixes[nPrefix];
	Suffix* suffix;
	struct State* next;
} State;

State* stateTbl[hashTblSize];

// Deallocate memory within the state-table. States, Suffixes, and
// suffix-strings need to be freed.
void cleanup() {
	for (int i = 0; i < hashTblSize; ++i) {
		State* sp = stateTbl[i];

		// Free the state list.
		while (sp) {
			State* stateNext = sp->next;
			Suffix* suffp = sp->suffix;

			// Free the suffix list.
			while (suffp) {
				Suffix* suffNext = suffp->next;

				// Free the suffix string.
				free(suffp->word);
				free(suffp);

				suffp = suffNext;
			}

			free(sp);

			sp = stateNext;
		}
	}
}

// Graceful exit helper function.
void gExit(const char* msg, int retCode) {
	cleanup();
	fprintf(stderr, "%s", msg);
	exit(retCode);
}

// Generate hash of prefixes.
unsigned int hash(char* prefixes[nPrefix]) {
	unsigned int hashVal = 0;

	// Constant empirically found to be good for ascii data.
	const int hashMagic = 31;

	for (int i = 0; i < nPrefix; ++i) {
		unsigned char* ch = (unsigned char*) prefixes[i];

		while (*ch != '\0') {
			hashVal = hashMagic * hashVal + *ch;

			++ch;
		}
	}

	return hashVal % hashTblSize;
}

// Lookup the prefix array in the hashtable; create it if requested.
// If the state is found or created, return a pointer to it; else return NULL.
//
// Warning: creation does not strdup so the caller must ensure the memory isn't
// changed later.
State* lookup(char* prefixes[nPrefix], _Bool create) {
	State* sp = NULL;
	unsigned int hashVal = hash(prefixes);

	// Return match if found.
	for (sp = stateTbl[hashVal]; sp != NULL; sp = sp->next) {
		int i;

		for (i = 0; i < nPrefix; ++i) {
			if (strcmp(prefixes[i], sp->prefixes[i]) != 0) {
				break;
			}
		}

		// All prefix strings matched. Return this state.
		if (i == nPrefix) {
			return sp;
		}
	}

	// No match was found. Create if requested.
	if (create) {
		sp = malloc(sizeof(State));

		if (!sp) {
			gExit("Error: Memory allocation failed!\n", 1);
		}

		for (int i = 0; i < nPrefix; ++i) {
			sp->prefixes[i] = prefixes[i];
		}

		sp->suffix = NULL;
		sp->next = stateTbl[hashVal];
		stateTbl[hashVal] = sp;
	}

	return sp;
}

// Make duplicate of a string on the heap and return a pointer to it.
char* strDuplicate(const char* str) {
	char* dup = malloc(strlen(str) + 1);
	if (!dup) {
		gExit("Error: Failed to allocate memory for string!\n", 1);
	}

	strcpy(dup, str);

	return dup;
}

void add(char* prefixes[nPrefix], char* suffix) {
	State* sp = lookup(prefixes, true);

	// Create suffix list entry.
	Suffix* suff = malloc(sizeof(Suffix));

	if (!suff) {
		gExit("Error: Memory allocation failed!\n", 1);
	}

	char* suffstr = strDuplicate(suffix);

	if (!suffstr) {
		gExit("Error: String duplication failed!\n", 1);
	}

	suff->word = suffstr;
	suff->next = sp->suffix;
	sp->suffix = suff;

	// Move prefix window.
	memmove(prefixes, prefixes + 1, (nPrefix - 1) * sizeof(prefixes[0]));
	prefixes[nPrefix - 1] = suffstr;
}

void build(char* prefixes[nPrefix], FILE* input) {
	char suffix[128], fmt[16];
	sprintf(fmt, "%%%lus", sizeof(suffix) - 1);

	while (fscanf(input, fmt, suffix) != EOF) {
		add(prefixes, suffix);
	}

	add(prefixes, NONWORD);
}

void generate(void) {
	char* prefixes[nPrefix];

	for (int i = 0; i < nPrefix; ++i) {
		prefixes[i] = NONWORD;
	}

	for (int i = 0; i < maxOutput; ++i) {
		State* sp = lookup(prefixes, false);
		int nSuffix = 0;
		char* word;

		for (Suffix* suffix = sp->suffix; suffix != NULL; suffix = suffix->next) {
			if (rand() % ++nSuffix == 0) {
				word = suffix->word;
			}
		}

		if (strcmp(word, NONWORD) == 0) {
			return;
		}

		printf("%s ", word);

		memmove(prefixes, prefixes + 1, (nPrefix - 1) * sizeof(prefixes[0]));
		prefixes[nPrefix - 1] = word;
	}
}

int main(void) {
	srand((unsigned int) time(NULL));

	char* prefixes[nPrefix];

	for (int i = 0; i < nPrefix; ++i) {
		prefixes[i] = NONWORD;
	}

	build(prefixes, stdin);
	generate();
	printf("\n");

	cleanup();
	return 0;
}
