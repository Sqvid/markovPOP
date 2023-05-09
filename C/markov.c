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
			fprintf(stderr, "Error: Memory allocation failed!\n");
			exit(1);
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

char* strDuplicate(const char* str) {
	char* dup = malloc(strlen(str) + 1);
	if (!dup) {
		fprintf(stderr, "Error: Failed to allocate memory for string!\n");
		exit(1);
	}

	strcpy(dup, str);

	return dup;
}

void add(char* prefixes[nPrefix], char* suffix) {
	State* sp = lookup(prefixes, true);

	// Create suffix list entry.
	Suffix* suff = malloc(sizeof(Suffix));

	if (!suff) {
		fprintf(stderr, "Error: Memory allocation failed!\n");
		exit(1);
	}

	char* suffstr = strDuplicate(suffix);

	if (!suffstr) {
		fprintf(stderr, "Error: String duplication failed!\n");
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
	char* prefixes[nPrefix];

	for (int i = 0; i < nPrefix; ++i) {
		prefixes[i] = NONWORD;
	}

	srand((unsigned int) time(NULL));
	build(prefixes, stdin);
	generate();

	printf("\n");

	return 0;
}
