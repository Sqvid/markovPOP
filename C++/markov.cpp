#include <iostream>
#include <istream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

class StateTable {
	public:
		// Build the state-table from an input stream.
		explicit StateTable(std::istream& in, int nPrefix = 2);

		// Generate the next word.
		std::string nextWord();

	private:
		// Size of the prefix window.
		int nPrefix_;

		// The state-table.
		std::unordered_map<std::string, std::vector<std::string>> st_;
		// The prefix window.
		std::vector<std::string> prefixes_;
		// Random number generation.
		std::random_device rd_;
		std::mt19937 rng_;

		// Member functions.
		// Add a suffix to the state-table.
		void addSuffix_(std::string key, std::string suffix);
};

// Add a suffix to the state-table.
void StateTable::addSuffix_(std::string key, std::string suffix) {
	auto it = st_.find(key);

	if (it != st_.end()) {
		auto& suffixVec = it->second;
		suffixVec.push_back(suffix);

	} else {
		st_.emplace(key, std::vector<std::string>{suffix});
	}
}

// Concatenate a vector of strings into a single string.
std::string joinStringVec(std::vector<std::string> vec) {
	std::string joined;

	for (auto& str : vec) {
		joined.append(str);
	}

	return joined;
}

// Build the state-table from an input stream.
StateTable::StateTable(std::istream& in, int nPrefix) : nPrefix_(nPrefix), rng_(rd_()) {
	std::string word;

	for (int i = 0; i < nPrefix_; ++i) {
		prefixes_.push_back("\n");
	}

	while (in >> word) {
		auto key = joinStringVec(prefixes_);

		addSuffix_(key, word);

		prefixes_.erase(prefixes_.begin());
		prefixes_.push_back(word);
	}

	prefixes_.clear();
	for (int i = 0; i < nPrefix_; ++i) {
		prefixes_.push_back("\n");
	}
}

// Return the next word in the Markov chain.
std::string StateTable::nextWord() {
	auto key = joinStringVec(prefixes_);
	auto search = st_.find(key);

	if (search != st_.end()) {
		auto suffixes = search->second;

		// Pick a random suffix from the list.
		std::uniform_int_distribution<size_t> dist(0, suffixes.size() - 1);
		auto suffix = suffixes[dist(rng_)];

		// Update the prefix window.
		prefixes_.erase(prefixes_.begin());
		prefixes_.push_back(suffix);

		return suffix;

	} else {
		return "\n";
	}
}

int main() {
	StateTable st(std::cin);
	const int maxOutput = 200;

	for (int i = 0; i < maxOutput; ++i) {
		auto word = st.nextWord();

		if (word == "\n") {
			break;
		}

		std::cout << word << " ";
	}

	std::cout << "\n";

	return 0;
}
