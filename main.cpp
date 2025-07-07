#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

using namespace std;

// =================== Utility Functions ===================

void clearScreen() {
    std::system(CLEAR_COMMAND);
}

bool checkForMenuOrQuit(const std::string& input) {
    if (input == "menu") return true;
    if (input == "quit") {
        std::cout << "\033[37mExiting...\n";  // white
        exit(0);
    }
    return false;
}

// =================== Program 1: Similarity Quiz ===================

double similarityPercentage(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size();
    const size_t len2 = s2.size();

    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (size_t i = 0; i <= len1; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= len2; ++j) dp[0][j] = j;

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)
                });
        }
    }

    int maxLen = std::max(len1, len2);
    return 100.0 - (static_cast<double>(dp[len1][len2]) / maxLen * 100.0);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

int findNearestSection(const vector<string>& answers, int currentLine, const string& searchTerm) {
    int n = answers.size();
    int bestLine = -1;
    int bestDistance = n + 1;

    string searchTermLower = toLower(searchTerm);

    for (int i = 0; i < n; ++i) {
        if (!answers[i].empty() && answers[i][0] == '#') {
            string lineLower = toLower(answers[i]);
            if (lineLower.find(searchTermLower) != string::npos) {
                int distance = abs(i - currentLine);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestLine = i;
                }
            }
        }
    }

    return bestLine;
}

bool runSimilarityQuiz(const std::string& answersFileName) {
    ifstream answersFile(answersFileName);
    if (!answersFile) {
        cerr << "\033[37mError opening answers file.\n"; // white error message
        return false;
    }

    vector<string> answers;
    string line;
    while (getline(answersFile, line)) {
        answers.push_back(line);
    }
    answersFile.close();

    vector<int> questionIndices;
    vector<int> questionDisplayNumbers;
    int displayLineNumber = 1;

    for (int i = 0; i < answers.size(); ++i) {
        if (answers[i].empty() || answers[i][0] == '#') continue;
        questionIndices.push_back(i);
        questionDisplayNumbers.push_back(displayLineNumber++);
    }

    if (questionIndices.empty()) {
        cerr << "\033[37mNo valid questions found in the file.\n";
        return false;
    }

    int fileLineIndex = 0;
    int questionNumber = 0;
    int planetNumber = 0;

    while (fileLineIndex < answers.size() && questionNumber < questionIndices.size()) {
        if (!answers[fileLineIndex].empty() && answers[fileLineIndex][0] == '#') {
            std::cout << "\n\033[36m" << answers[fileLineIndex] << "\033[37m\n\n"; // cyan header, then back to white
            fileLineIndex++;
            continue;
        }

        if (fileLineIndex != questionIndices[questionNumber]) {
            fileLineIndex++;
            continue;
        }

        int displayLineIndex = questionDisplayNumbers[questionNumber];
        if (planetNumber > 25) planetNumber = 0;

        std::cout << "\033[37mLine " << displayLineIndex << "\n\n> "; // white prompt
        std::string userAnswer;
        std::getline(std::cin, userAnswer);

        if (checkForMenuOrQuit(userAnswer)) return true;

        double similarity = similarityPercentage(userAnswer, answers[fileLineIndex]);

        if (similarity > 75) {
            std::cout << "\n\033[32m" << answers[fileLineIndex] << "\n\nCorrect!\n\n\033[37m"; // green correct answer, then back to white
            questionNumber++;
            planetNumber++;
            fileLineIndex++;
        }
        else {
            std::cout << "\033[31m\nThe answer was: \n\n" << answers[fileLineIndex] << "\n" // red wrong answer
                << "\033[37m\nJump to: \n\n> "; // back to white prompt
            std::string searchTerm;
            std::getline(std::cin, searchTerm);
            std::cout << "\n";
            if (checkForMenuOrQuit(searchTerm)) return true;

            if (searchTerm.empty()) {
                questionNumber++;
                if (questionNumber < questionIndices.size())
                    fileLineIndex = questionIndices[questionNumber];
            }
            else {
                int foundLine = findNearestSection(answers, fileLineIndex, searchTerm);
                if (foundLine != -1) {
                    for (int i = 0; i < questionIndices.size(); ++i) {
                        if (questionIndices[i] > foundLine) {
                            questionNumber = i;
                            fileLineIndex = questionIndices[i];
                            break;
                        }
                    }
                    std::cout << "\n\033[36m" << answers[foundLine] << "\033[37m\n\n"; // cyan header, then back to white
                    planetNumber = 0;
                }
                else {
                    std::cout << "\n\033[37mNo matching section found. Restarting from beginning.\n\n"; // white message
                    questionNumber = 0;
                    fileLineIndex = questionIndices[0];
                    planetNumber = 0;
                }
            }
        }
    }

    std::cout << "\033[37mYou remembered everything!\n"; // white
    return false;
}

// =================== Program 2: Card Memory Quiz ===================

const int NUM_CARDS = 52;

bool loadCardsFromFile(const std::string& filename, std::vector<std::string>& cards) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "\033[37mError: Could not open file '" << filename << "'\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) continue;
        if (line[firstNonSpace] == '#') continue;

        size_t lastNonSpace = line.find_last_not_of(" \t");
        std::string trimmed = line.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
        if (!trimmed.empty()) cards.push_back(trimmed);
    }

    return cards.size() == NUM_CARDS;
}

bool runCardMemoryQuiz(const std::string& filename) {
    std::vector<std::string> cards;
    if (!loadCardsFromFile(filename, cards)) {
        std::cerr << "\033[37mError: Invalid card file.\n";
        return false;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::vector<int> startPositions(NUM_CARDS);
    for (int i = 0; i < NUM_CARDS; ++i) startPositions[i] = i;
    std::random_shuffle(startPositions.begin(), startPositions.end());

    int currentRound = 0;
    int previewCount = 0;
    bool askPreviewCount = true;

    while (true) {
        if (askPreviewCount) {
            std::cout << "\033[37mType shuffle to mix the cards\n"; // white
            std::cout << "\nNumber of Hints:\n\n>> ";
            std::string input;
            std::getline(std::cin, input);

            if (checkForMenuOrQuit(input)) return true;

            if (input == "shuffle") {
                cards.clear();
                if (!loadCardsFromFile(filename, cards)) {
                    std::cerr << "\033[37mError: Invalid card file.\n";
                    return false;
                }
                for (int i = 0; i < NUM_CARDS; ++i) startPositions[i] = i;
                std::random_shuffle(startPositions.begin(), startPositions.end());

                currentRound = 0;
                askPreviewCount = true;
                clearScreen();
                std::cout << "\033[37mCards shuffled!\n\n"; // white
                continue;
            }

            try {
                previewCount = std::stoi(input);
            }
            catch (...) {
                previewCount = 1;
            }

            if (previewCount < 1) previewCount = 1;
            if (previewCount > NUM_CARDS) previewCount = NUM_CARDS;
            clearScreen();
            askPreviewCount = false;
        }

        if (currentRound >= NUM_CARDS) {
            std::cout << "\033[37mCongratulations! You have completed all 52 rounds.\n"; // white
            return false;
        }

        int start = startPositions[currentRound];

        std::cout << "\n\033[37mCard " << (currentRound + 1) << " of " << NUM_CARDS << ":\n\n";

        for (int i = 0; i < previewCount; ++i) {
            int cardIndex = (start - previewCount + i + NUM_CARDS) % NUM_CARDS;
            std::cout << cards[cardIndex];
            if (i < previewCount - 1) std::cout << ", ";
        }

        std::cout << "\n\n>> ";
        std::string userAnswer;
        std::getline(std::cin, userAnswer);

        if (checkForMenuOrQuit(userAnswer)) return true;

        if (userAnswer == "shuffle") {
            cards.clear();
            if (!loadCardsFromFile(filename, cards)) {
                std::cerr << "\033[37mError: Invalid card file.\n";
                return false;
            }
            for (int i = 0; i < NUM_CARDS; ++i) startPositions[i] = i;
            std::random_shuffle(startPositions.begin(), startPositions.end());

            currentRound = 0;
            askPreviewCount = true;
            clearScreen();
            std::cout << "\033[37mCards shuffled!\n"; // white
            continue;
        }

        if (userAnswer == cards[start % NUM_CARDS]) {
            std::cout << "\n\033[32mCorrect!\n\n\033[37m"; // green, then white
            currentRound++;
        }
        else {
            std::cout << "\n\033[31mIncorrect. Correct answer: " << cards[start % NUM_CARDS] << "\n\n\033[37m"; // red message, then white
            std::cout << "\nPress enter to continue...";
            std::getline(std::cin, userAnswer);
            if (checkForMenuOrQuit(userAnswer)) return true;
            clearScreen();
            currentRound = 0;
            askPreviewCount = true;
        }
    }

    return false;
}

// =================== Main Menu ===================

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "\033[37mUsage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    std::string filename = argv[1];

    while (true) {
        clearScreen();

        std::cout << "\033[37mChoose a program:\n\n";
        std::cout << "1. Memory Houses\n";
        std::cout << "2. Memory Hints\n";
        std::cout << "3. Quit\n\n> ";

        int choice = 0;
        std::string input;
        std::getline(std::cin, input);
        if (input == ":quit" || input == "3") break;
        choice = std::stoi(input);

        clearScreen();

        std::cout << "\033[37mType \"menu\" or \"quit\" anytime to return or exit.\n\n";

        if (choice == 1) {
            if (!runSimilarityQuiz(filename)) {
                std::cout << "\n\033[37mProgram complete.\n";
            }
        }
        else if (choice == 2) {
            if (!runCardMemoryQuiz(filename)) {
                std::cout << "\n\033[37mProgram complete.\n";
            }
        }
        else {
            std::cout << "\n\033[37mInvalid choice.\n";
        }
    }

    std::cout << "\n\033[37mGoodbye!\n";
    return 0;
}
