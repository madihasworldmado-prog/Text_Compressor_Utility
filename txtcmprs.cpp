#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cstdio>

using namespace std;

// Check whether a file exists
bool fileExists(const string &path) {
    ifstream file(path.c_str(), ios::binary);
    return file.good();
}

// Convert extension to lowercase and validate
bool hasExtension(const string &path, string extension) {

    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');

    if (dot == string::npos)
        return false;

    if (slash != string::npos && dot < slash)
        return false;

    string ext = path.substr(dot);

    for (int i = 0; i < ext.length(); i++)
        ext[i] = tolower((unsigned char)ext[i]);

    return ext == extension;
}

// Remove extension and generate new filename
string generateOutputName(const string &path,
                          string newExtension) {

    size_t dot = path.find_last_of('.');

    return path.substr(0, dot) + newExtension;
}

// Remove surrounding quotes
string cleanPath(string path) {

    if (path.length() >= 2 &&
        path[0] == '"' &&
        path[path.length() - 1] == '"') {

        path = path.substr(1, path.length() - 2);
    }

    return path;
}

// --------------------------------------
// COMPRESSION
// --------------------------------------

void compressFile() {

    string inputPath, outputPath;

    cout << "\nEnter TXT file path: ";
    getline(cin, inputPath);

    inputPath = cleanPath(inputPath);

    if (!hasExtension(inputPath, ".txt")) {
        cout << "\nError: Only TXT files are supported!\n";
        return;
    }

    ifstream in(inputPath.c_str(), ios::binary);

    if (!in) {
        cout << "\nError: Cannot open input file!\n";
        return;
    }

    outputPath = generateOutputName(inputPath, ".apm");

    if (fileExists(outputPath)) {
        cout << "\nError: Output file already exists!\n";
        in.close();
        return;
    }

    ofstream out(outputPath.c_str(), ios::binary);

    if (!out) {
        cout << "\nError: Cannot create output file!\n";
        in.close();
        return;
    }

    unsigned long originalSize = 0;
    unsigned long compressedSize = 4;

    int current, count;

    // APM file signature
    out.write("APM1", 4);

    current = in.get();

    while (current != EOF) {

        count = 1;
        originalSize++;

        // Count consecutive repeated bytes
        while (count < 255 && in.peek() == current) {

            in.get();
            count++;
            originalSize++;
        }

        // Write count and character
        out.put((char)count);
        out.put((char)current);

        compressedSize += 2;

        current = in.get();
    }

    in.close();
    out.close();

    if (!out) {
        remove(outputPath.c_str());
        cout << "\nError: Compression failed!\n";
        return;
    }

    cout << "\n========== COMPRESSION REPORT ==========\n";

    cout << "Input File  : " << inputPath << endl;
    cout << "Output File : " << outputPath << endl;

    cout << "Original Size   : " << originalSize << " bytes\n";
    cout << "Compressed Size : " << compressedSize << " bytes\n";

    if (originalSize > 0) {

        double ratio =
            (double)compressedSize / originalSize * 100;

        cout << "Compression Ratio: " << ratio << "%\n";
        cout << "Space Saved      : " << 100.0 - ratio << "%\n";
    }

    cout << "\nCompression successful!\n";
}

// --------------------------------------
// DECOMPRESSION
// --------------------------------------

void extractFile() {

    string inputPath, outputPath;

    cout << "\nEnter APM file path: ";
    getline(cin, inputPath);

    inputPath = cleanPath(inputPath);

    if (!hasExtension(inputPath, ".apm")) {
        cout << "\nError: Only APM files are supported!\n";
        return;
    }

    ifstream in(inputPath.c_str(), ios::binary);

    if (!in) {
        cout << "\nError: Cannot open APM file!\n";
        return;
    }

    // Verify APM signature
    char header[4];

    in.read(header, 4);

    if (in.gcount() != 4 ||
        header[0] != 'A' ||
        header[1] != 'P' ||
        header[2] != 'M' ||
        header[3] != '1') {

        cout << "\nError: Invalid APM file!\n";
        in.close();
        return;
    }

    outputPath = generateOutputName(inputPath, ".txt");

    if (fileExists(outputPath)) {
        cout << "\nError: Output TXT file already exists!\n";
        in.close();
        return;
    }

    ofstream out(outputPath.c_str(), ios::binary);

    if (!out) {
        cout << "\nError: Cannot create output file!\n";
        in.close();
        return;
    }

    unsigned long extractedSize = 0;

    int count, ch;

    while (true) {

        count = in.get();

        if (count == EOF)
            break;

        ch = in.get();

        // Check incomplete data
        if (ch == EOF || count == 0) {

            cout << "\nError: Corrupted APM file!\n";

            in.close();
            out.close();

            remove(outputPath.c_str());

            return;
        }

        // Restore repeated characters
        for (int i = 0; i < count; i++) {

            out.put((char)ch);
            extractedSize++;
        }

        if (!out) {

            cout << "\nError: Extraction failed!\n";

            in.close();
            out.close();

            remove(outputPath.c_str());

            return;
        }
    }

    in.close();
    out.close();

    cout << "\n========== EXTRACTION REPORT ==========\n";

    cout << "Input File : " << inputPath << endl;
    cout << "Output File: " << outputPath << endl;

    cout << "Restored Size: "
         << extractedSize << " bytes\n";

    cout << "\nExtraction successful!\n";
}

// --------------------------------------
// MAIN MENU
// --------------------------------------

int main() {

    int choice;

    do {

        cout << "\n====================================\n";
        cout << "          APM FILE UTILITY\n";
        cout << "====================================\n";

        cout << "1. Compress TXT File\n";
        cout << "2. Extract APM File\n";
        cout << "3. Exit\n";

        cout << "====================================\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');

            cout << "\nInvalid input!\n";
            continue;
        }

        cin.ignore(10000, '\n');

        switch (choice) {

            case 1:
                compressFile();
                break;

            case 2:
                extractFile();
                break;

            case 3:
                cout << "\nThank you for using APM!\n";
                break;

            default:
                cout << "\nInvalid choice!\n";
        }

    } while (choice != 3);

    return 0;
}

