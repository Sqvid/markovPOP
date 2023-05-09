package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"strings"
)

const (
	nPrefix   = 2
	maxOutput = 200
	// Sentinel value that cannot organically appear in the state-table
	nonWord = "\n"
)

func main() {
	// The state-table that holds prefix phrases and their associated suffixes.
	st := make(map[string][]string)

	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(bufio.ScanWords)

	// Initialise the prefix slice with a non-word sentinel value to mark the
	// start of the document; useful during text generation.
	prefixes := make([]string, 0, nPrefix)
	for i := 0; i < nPrefix; i++ {
		prefixes = append(prefixes, nonWord)
	}

	// Build the state-table.
	for scanner.Scan() {
		suffix := scanner.Text()

		// The prefix key in the state-table is just the concatination of the
		// prefix strings.
		key := strings.Join(prefixes, "")
		st[key] = append(st[key], suffix)

		// Move the prefix window, and add the suffix.
		prefixes = append(prefixes[1:], suffix)
	}

	// Reset prefixes to sentinel values to indicate the beginning of the
	// document.
	prefixes = []string{}
	for i := 0; i < nPrefix; i++ {
		prefixes = append(prefixes, nonWord)
	}

	// Generate output
	for n := 0; n < maxOutput; n++ {
		key := strings.Join(prefixes, "")

		if suffixes, ok := st[key]; ok {
			suffix := suffixes[rand.Intn(len(suffixes))]
			fmt.Printf("%s ", suffix)

			prefixes = append(prefixes[1:], suffix)

		} else {
			break
		}
	}

	fmt.Printf("\n")
}
