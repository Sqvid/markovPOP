import random
import sys

def main():
    N_PREFIX = 2
    MAX_OUTPUT = 200
    NON_WORD = "\n"

    # This is the state-table. It associates prefix phrases with suffixes
    # they've appeared with in the input text.
    st = {}

    # Initialise the prefix window with a sentinal value that cannot naturally
    # occur in the state-table. This will help during text generation to handle
    # the beginning of the document correctly.
    prefixes = [NON_WORD] * N_PREFIX

    # Build the state-table.
    for line in sys.stdin:
        for suffix in line.split():
            key = "".join(prefixes)

            if key in st:
                st[key].append(suffix)
            else:
                st[key] = [suffix]

            # Move the prefix window and add the suffix.
            prefixes = prefixes[1:]
            prefixes.append(suffix)

    prefixes = [NON_WORD] * N_PREFIX

    for _ in range(MAX_OUTPUT):
        key = "".join(prefixes)

        if key in st:
            suffixList = st[key]
            suffix = random.choice(suffixList)

            prefixes = prefixes[1:]
            prefixes.append(suffix)

            print(suffix, end=" ")

        else:
            break

    print("\n")

if __name__ == "__main__":
    main()
