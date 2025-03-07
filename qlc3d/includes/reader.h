#ifndef READER2_H
#define READER2_H
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <optional>

struct ReaderError : public std::runtime_error
{
    std::string errorMessage;
    std::string fileName;
    size_t lineNumber;
    std::string lineText;

    ReaderError(const std::string &errorMessage,
                const std::string &fileName = "",
                const size_t &lineNumber = 0,
                const std::string &lineText = "") : std::runtime_error(errorMessage + ", file: " + fileName + ", line: " + std::to_string(lineNumber) + ", line text: " + lineText),
                                                    errorMessage(errorMessage),
                                                    fileName(fileName),
                                                    lineNumber(lineNumber),
                                                    lineText(lineText) {}

    void printError()
    {
        /*!
            Prints error info to stderr.
        */
        if ((fileName.size()) && (lineNumber == 0))
        { // only filename
            std::cerr << "Error in file :" << fileName << std::endl;
        }
        else if ((fileName.size()) && (this->lineNumber > 0))
        { // filename AND line number
            std::cerr << "Error in file " << fileName << " on line " << this->lineNumber << std::endl;
            std::cerr << this->lineText << std::endl;
        }
        std::cerr << this->errorMessage << std::endl;
    }
};

class Reader
{
    struct lineData
    {                         // a record for storing single key/value definition line data
        size_t lineNumber_;   // line number in source file
        std::string rawText_; // raw un formatted text of the line
        std::string val_;     // formatted value stored as a string
    };
    bool isCaseSensitive_;                   // if not case sensitive, all text will be converted to lowercase
    bool isLowerCaseStringValues_;           // whether to convert all returned string values to lowercase
    bool isEnvironmentVariableSubstitution_; // whether to perform string substitution from environment variables
    // ERROR MESSAGES
    const std::string _R_FILE_OPEN_ERROR_MESSAGE = "Could not open file: ";
    const std::string _R_VECTOR_FORMAT_ERROR_MSG = "Bad vector format";
    const std::string _R_KEY_NOT_FOUND_ERROR_MSG = "Key not found: ";
    const std::string _R_NUMBER_FORMAT_ERROR_MSG = "Bad number format: ";
    const std::string _R_BAD_VALUE_ERROR_MSG = "Bad value format: ";
    const std::string _R_BAD_KEY_VALUE_ERROR_MSG = "Bad key/value assignment";
    // CHARACTERS USED FOR VALIDATING KEYS AND VALUES
    const std::string _R_WHITE_SPACE = " \t\r"; // white space characters
    const std::string _R_COMMENT_CHARS = "#";
    const std::string _R_DIGIT_CHARS = "1234567890-.";

    // VECTOR DELIMITERS
    const char _R_OBRACE = '[';
    const char _R_CBRACE = ']';
    const char _R_VDELIM = ',';

    std::filesystem::path _fileName;            // holds current file name
    std::map<std::string, lineData> _keyValues; // "database" of all read key/value pairs
    std::vector<std::string> _validKeys;        // optional list of valid keys. read in from a separte file
    // STRING FORMATTING FUNCTIONS
    inline void removeComments(std::string &line) const; // removes everything in a line after comment character
    inline void cleanLineEnds(std::string &line) const;  // removes white space from both ends of a string
    inline bool splitByChar(const std::string &line, std::string &key, std::string &value, const char &split = '=') const;
    inline bool isValidNumber(const std::string &strVal) const;     // returns false if not a valid number
    inline bool isValidString(const std::string &strVal) const;     // returns false if not a valid string
    inline bool isValidBoolean(const std::string &strVal) const;    // returns true if the string is a boolean
    inline size_t getLineNumberByKey(const std::string &key) const; // find line number in file where a given key is defined
    inline void toLower(std::string &s) const;                      // converts string to all lower case

    inline std::string envVarSubstitution(std::string &s) const; // performs string substitution on detected environment variables

    // SPECIALISATIONS FOR HANDLING DIFFERENT VALUE TYPES
    inline void parseValue(const std::string &strVal, std::string &val) const;
    inline void parseValue(const std::string &strVal, std::vector<std::string> &val) const; // make array of strings
    inline void parseValue(const std::string &strVal, size_t &val) const;                   // size_t conversion
    inline void parseValue(const std::string &strVal, unsigned int &val) const;             // 32 bit unsigned int conversion
    inline void parseValue(const std::string &strVal, bool &val) const;                     // bool conversion

    [[nodiscard]] inline static bool validateKey(const std::string &key);

    template <class T>
    inline void parseValue(const std::string &strVal, std::vector<T> &val) const
    {
        /*!
         * Converts to a vector of numbers
         */
        // first convert to a vector of strings
        std::vector<std::string> svals;
        parseValue(strVal, svals);
        // then convert each item to a number
        for (auto &s : svals)
        {
            if (!isValidNumber(s))
            {
                std::string errMSG = _R_NUMBER_FORMAT_ERROR_MSG + s;
                throw std::runtime_error(errMSG);
            }
            std::stringstream ss(s);
            double tval(0);
            ss >> tval;
            val.push_back(static_cast<T>(tval));
        }
    }
    template <class T>
    inline void parseValue(const std::string &strVal, T &val) const
    {
        /*!
          Converts to numebers
          */
        if (!isValidNumber(strVal))
        {
            std::string errMSG = _R_NUMBER_FORMAT_ERROR_MSG + strVal;
            throw std::runtime_error(errMSG);
        }
        std::string sval;
        parseValue(strVal, sval);
        // first convert to double, then to other if needed
        std::stringstream ss(sval);
        double tval(0);
        ss >> tval;
        val = static_cast<T>(tval);
    }

public:
    inline Reader() : isCaseSensitive_(true), isLowerCaseStringValues_(false), isEnvironmentVariableSubstitution_(false) {}
    inline void readSettingsFile(const std::filesystem::path &fileName);
    inline void readValidKeysFile(const std::filesystem::path &fileName);
    [[nodiscard]] inline bool containsKey(const std::string &key) const;

    /** check whether a key that starts with the given prefix exists */
    [[nodiscard]] inline bool containsKeyWithPrefix(const std::string &keyPrefix);
    template <class T>
    inline T getValueByKey(const std::string &key) const;
    template <class T>
    inline std::optional<T> getOptional(const std::string &key);

    template <class T>
    inline T get(const std::string &key, const T &defaultValue);
    [[nodiscard]] inline bool isCaseSensitive() const { return isCaseSensitive_; }
    [[nodiscard]] inline bool isLowerCaseStringValues() const { return isLowerCaseStringValues_; }
    [[nodiscard]] inline bool isEnvironmentVariableSubstitution() const { return isEnvironmentVariableSubstitution_; }
    inline void setCaseSensitivity(bool isCS) { isCaseSensitive_ = isCS; }
    inline void setLowerCaseStringValues(bool isLowerCase) { isLowerCaseStringValues_ = isLowerCase; }
    inline void setEnvironmentVariableSubstitution(bool isEnvironmentVariableSubstitution)
    {
        isEnvironmentVariableSubstitution_ = isEnvironmentVariableSubstitution;
    }
    /** Checks whether key is in list of valid keys. Keys may contain wildcards, marked with '*' */
    [[nodiscard]] inline bool isValidKey(std::string key) const;
    /** Return std::vector<string> that contains all keys in the file */
    [[nodiscard]] inline std::vector<std::string> getAllKeys() const;
    /** Check whether the value of key exists and is an array of some types */
    [[nodiscard]] inline bool isValueArray(const std::string &key) const;
    /**
     * Return whether all values in the array are valid strings,
     * NOTE: currently all numbers are strings too, but all strings are not numbers
     */
    [[nodiscard]] inline bool isValueArrayOfStrings(const std::string &key) const;
    /** Return whether all values in the array are valid numbers */
    [[nodiscard]] inline bool isValueArrayOfNumbers(const std::string &key) const;
};

//****************************************************************************************
//
//                      METHOD DEFINITIONS
//
//****************************************************************************************
inline void Reader::toLower(std::string &s) const
{
    std::locale loc;
    for (size_t i = 0; i < s.size(); i++)
    {
        s[i] = std::tolower(s[i], loc);
    }
}

/**
 * Makes a string substitution from environment variables. For example in the string
 * "someKey = ${MY_VALUE}", the substring ${MY_VALUE} is replaced with the value of
 * the environment variable MY_VALUE.
 * NOTE: this modifies the input string.
 *
 * If an environment variable is not defined, an std::runtime_error is thrown.
 *
 * @param target string
 * @return the target string after substitutions
 */
inline std::string Reader::envVarSubstitution(std::string &stringIn) const
{
    using namespace std;
    string s(stringIn);
    while (s.find('$') != string::npos)
    {
        size_t start = s.find_first_of("${");
        size_t end = s.find_first_of('}') + 1;

        if ((start == string::npos) || (end == string::npos) || (end < start))
        {
            throw runtime_error("Invalid environment variable substitution format");
        }

        // find the corresponding environment variable
        string key = s.substr(start + 2, (end - start) - 3);
        cleanLineEnds(key);
        char *value = getenv(key.c_str());
        if (value == nullptr)
        {
            throw runtime_error("No such environment variable " + key);
        }

        s = s.substr(0, start) + value + s.substr(end);
    }
    return s;
}

inline void Reader::cleanLineEnds(std::string &line) const
{
    /*! Removes white space from both ends of input string*/
    if (line.size() == 0)
    { // empty line
        return;
    }
    // REMOVE WHITE SPACE CHARACTERS AT START OF LINE
    size_t ind = line.find_first_not_of(_R_WHITE_SPACE);
    if (ind == std::string::npos)
    { // if white space only line
        line = "";
        return;
    }
    else if (ind > 0)
    { // white space at start of line
        line = line.substr(ind);
    }
    // REMOVE WHITE SPACE CHARACTERS AT END OF LINE
    ind = line.find_last_not_of(_R_WHITE_SPACE);
    if (ind < line.size() - 1)
    {
        line = line.substr(0, ind + 1);
    }

    // remove new line characters
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
}

inline void Reader::removeComments(std::string &line) const
{
    /*! Removes everything after comment character*/
    size_t ind = line.find_first_of(_R_COMMENT_CHARS.c_str());
    if (ind != std::string::npos)
    {
        line = line.substr(0, ind);
    }
}
inline bool Reader::splitByChar(const std::string &line,
                                std::string &key,
                                std::string &value,
                                const char &split) const
{
    /*!
     * Breaks line into two parts by specified split character. Returns true/false if success/fail.
     * If split chracter is not defined, default '=' is used
     */
    // MAKE SURE EXACTLY ONE '=' CHARACTER ON LINE
    size_t ind = line.find_first_of(split);
    size_t ind2 = line.find_last_of(split);
    if ((ind == std::string::npos) || // if no equals sign
        (ind != ind2))
    { // or more than 1
        return false;
    }
    // GET NON-EMPTY KEY/VALUE STRINGS
    key = line.substr(0, ind);
    value = line.substr(ind + 1);
    cleanLineEnds(key);
    cleanLineEnds(value);
    if ((key.empty()) || (value.empty()))
    {
        return false;
    }
    return true;
}

void Reader::readSettingsFile(const std::filesystem::path &fileName)
{
    /*!
     * Reads text file and stores found key/value pair definitions in a
     * map "database". Does initial error checking for validity of format.
     */
    _keyValues.clear();
    _fileName = fileName;
    std::ifstream fin(_fileName.c_str());
    if (!fin.is_open())
    {
        throw ReaderError(_R_FILE_OPEN_ERROR_MESSAGE + _fileName.string());
    }
    std::string line;
    size_t lineNumber = 1;
    // READ IN RAW TEXT LINE BY LINE
    do
    {
        std::string rawLine;
        std::getline(fin, rawLine);
        std::string line(rawLine);
        removeComments(line);
        cleanLineEnds(line);
        if (line.empty())
        { // skip rest of test for empty lines
            lineNumber++;
            continue;
        }
        if (isEnvironmentVariableSubstitution())
        {
            try
            {
                line = envVarSubstitution(line);
            }
            catch (std::exception &error)
            {
                throw ReaderError(error.what(), fileName.string(), lineNumber, line);
            }
        }

        // EXTRACT KEY AND VALUES AS STRINGS
        std::string key;
        std::string value;
        if (!splitByChar(line, key, value))
        {
            fin.close();
            throw ReaderError(_R_BAD_VALUE_ERROR_MSG, _fileName.string(), lineNumber, rawLine);
        }

        if (!validateKey(key))
        {
            fin.close();
            throw ReaderError("Key contains invalid character(s)", _fileName.string(), lineNumber, rawLine);
        }

        // HANDLE CASE SENSITIVITY
        std::string rawKey = key;
        std::string rawValue = value;
        if (!isCaseSensitive())
        {
            toLower(key);
            toLower(value);
        }
        if (isLowerCaseStringValues())
        { // really only needed if case sensitivity is false
            toLower(value);
        }

        // IF VALID KEYS HAVE BEEN DEFINED, CHECK THIS IS ONE
        if ((_validKeys.size() > 0) && !isValidKey(key))
        {
            throw ReaderError(rawKey + " is not a valid key.", _fileName.string(), lineNumber, rawLine);
        }
        // DISALLOW REDEFINITION OF EXISTING KEYS
        if (containsKey(key))
        {
            size_t prevDefLine = this->getLineNumberByKey(key);
            std::stringstream line1;
            line1 << prevDefLine;
            std::string msg = "Multiple definitions of \"" + key + "\", which is already defined on line " +
                              line1.str();
            throw ReaderError(msg, _fileName.string(), lineNumber, rawLine);
        }
        // IF OK SO FAR - SAVE LINE INFO TO "DATABASE"
        lineData valueData;
        valueData.lineNumber_ = lineNumber;
        valueData.rawText_ = rawLine;
        valueData.val_ = value;
        _keyValues[key] = valueData;
        lineNumber++;
    } while (!fin.eof());
    fin.close();
}

void Reader::readValidKeysFile(const std::filesystem::path &fileName)
{
    /*! Reads a text file containing a list of all allowed keys.
        This should be called before reading in the actual settings file*/
    // Make sure this is called before reading the actual settings file
    if (_keyValues.size() > 0)
    {
        throw ReaderError("Valid keys file should be read first.", fileName.string());
    }
    std::ifstream fin(fileName.c_str());
    if (!fin.is_open())
    {
        throw ReaderError(_R_FILE_OPEN_ERROR_MESSAGE + fileName.string());
    }
    std::string line;
    size_t lineNumber = 1;
    // READ IN RAW TEXT LINE BY LINE
    do
    {
        std::string rawLine;
        std::getline(fin, rawLine);
        std::string line(rawLine);
        removeComments(line);
        cleanLineEnds(line);
        if (line.empty())
        { // skip rest of test for empty lines
            lineNumber++;
            continue;
        }
        // EXTRACT KEY AND VALUES AS STRINGS
        std::string key;
        std::string value;
        if (!splitByChar(line, key, value))
        {
            fin.close();
            throw ReaderError(_R_BAD_VALUE_ERROR_MSG, fileName.string(), lineNumber, rawLine);
        }
        // HANDLE CASE SENSITIVITY
        if (!isCaseSensitive())
        {
            toLower(key);
            toLower(value);
        }
        // need to replace wildcard "*" with regex wildcard ".*"
        size_t ind = 0;
        while (ind < std::string::npos)
        { // loop to cover multiple wildcards in same key
            ind = key.find("*", ind);
            if (ind < std::string::npos)
            {
                key.replace(ind, 1, ".*");
                ind += 2; // increment by 2 to avoid infinite loop over same *->.*
            }
        }
        // IF OK SO FAR - SAVE LINE INFO TO "DATABASE"
        _validKeys.push_back(key);
        lineNumber++;
    } while (!fin.eof());
    fin.close();
}

bool Reader::isValidKey(std::string key) const
{
    // compare each valid key to current key
    for (auto &k : _validKeys)
    {
        std::regex rx(k);
        if (std::regex_match(key.begin(), key.end(), rx))
        {
            return true;
        }
    }
    return false; // key not found
}

bool Reader::isValueArray(const std::string &key) const
{
    if (!containsKey(key))
    {
        return false;
    }

    std::string tempKey(key);
    toLower(tempKey);
    auto val = _keyValues.at(tempKey).val_;
    cleanLineEnds(val);

    return !val.empty() && val.front() == '[' && val.back() == ']';
}

bool Reader::isValueArrayOfStrings(const std::string &key) const
{
    if (!isValueArray(key))
    {
        return false;
    }
    // it's an array of something, but is it an array of strings?
    try
    {
        std::vector<std::string> valArray;
        std::string keyLower(key);
        toLower(keyLower);
        auto val = _keyValues.at(keyLower).val_;

        parseValue(val, valArray); // throws if not a valid array of strings
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool Reader::isValueArrayOfNumbers(const std::string &key) const
{
    if (!isValidNumber(key))
    {
        return false;
    }

    try
    {
        // brute force method to check if all values are readable as numbers without errors
        auto array = getValueByKey<std::vector<double>>(key);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

size_t Reader::getLineNumberByKey(const std::string &key) const
{
    /*!
     * Returns line number in file where a given key/value definition occurs.
     */
    auto itr = _keyValues.find(key);
    if (itr == _keyValues.end())
    { // if not found
        throw std::runtime_error("could not find key: " + key);
    }
    return itr->second.lineNumber_;
}

bool Reader::containsKey(const std::string &key) const
{
    // validate key format
    std::string tkey(key);
    this->cleanLineEnds(tkey);
    if (!isCaseSensitive())
        toLower(tkey); // if case insensitive
    auto itr = _keyValues.find(tkey);
    //
    if (itr != _keyValues.end())
    { // if contains given key
        return true;
    }
    else
    {
        return false;
    }
}

bool Reader::containsKeyWithPrefix(const std::string &keyPrefix)
{
    std::string prefix(keyPrefix);
    if (!isCaseSensitive())
    { // keys are already converted to lowercase
        toLower(prefix);
    }
    return std::any_of(_keyValues.begin(), _keyValues.end(),
                       [&prefix](const auto &keyVal)
                       { return keyVal.first.find(prefix) == 0; });
}

bool Reader::isValidNumber(const std::string &strVal) const
{
    /*!
      Tries to validate that the input string can be converted to a valid number.
      Does not currently catch many special cases. (repeated '-', '.' and 'e' characters)
      */
    // IF NUMBER USES SCIENTIFIC NOTATION, e.g.: 1e-2 , SPLIT IT BY e AND CHECK EACH SEPARATELY
    std::string num1, num2;
    if (splitByChar(strVal, num1, num2, 'e'))
    {                                                      // IF 'scientific' notation fractional: 2e-5
        return isValidNumber(num1) && isValidNumber(num2); // both parts must be valid
    }
    else
    {
        // if contains non-numeric chars
        if (strVal.find_first_not_of(_R_DIGIT_CHARS) < std::string::npos)
        {
            return false;
        }
        else
            // if contains multiple dots or minuses
            if ((std::count(strVal.begin(), strVal.end(), '.') > 1) ||
                (std::count(strVal.begin(), strVal.end(), '-') > 1))
            {
                return false;
            }
        // if minus sign found, but not in first position
        size_t minPos = strVal.find_first_of('-');
        if ((minPos > 0) && (minPos < std::string::npos))
        {
            return false;
        }
        // found no errors
        return true;
    }
}

bool Reader::isValidBoolean(const std::string &strVal) const
{
    return strVal == "true" || strVal == "false";
}

bool Reader::isValidString(const std::string &strVal) const
{
    // It's a string if it starts and ends with double quotes
    // count number of occurrences of " in the string
    size_t quotesCount = std::count(strVal.begin(), strVal.end(), '"');
    // check whether first and last characters are "
    if (quotesCount == 2 && strVal.front() == '"' && strVal.back() == '"')
    {
        return true;
    }

    // if there are odd number of quotes, it's not a valid string
    if (quotesCount % 2 != 0)
    {
        return false;
    }

    // If there is no whitespace then it must be a string (or boolean or number, but they count as strings)
    return strVal.find_first_of(_R_WHITE_SPACE) == std::string::npos;
}

void Reader::parseValue(const std::string &strVal, std::string &val) const
{
    /*!
     * Parses a string to a string-value, checking for whitespaces. This method is called
     * first in numeric conversions too.
     */
    if (!isValidString(strVal))
    {
        throw ReaderError(_R_BAD_VALUE_ERROR_MSG + strVal, _fileName.string());
    }
    val = strVal;
    // Remove white space at start and end ov current value
    cleanLineEnds(val);

    // If val starts and ends with double quotes, remove them and return whatever is left as-is without further checks
    if (val.front() == '"' && val.back() == '"')
    {
        val = val.substr(1, val.size() - 2);
    }
    else
    { // check that no whitespace is present in the value
        size_t ind = val.find_first_of(_R_WHITE_SPACE);
        if (ind < std::string::npos)
        {
            std::string errMsg = _R_BAD_VALUE_ERROR_MSG + strVal + ", found unexpected whitespace";
            throw ReaderError(errMsg, _fileName.string());
        }
    }
}

void Reader::parseValue(const std::string &strVal, std::vector<std::string> &val) const
{
    std::string str(strVal); // make working copy
    size_t i1, i2;

    // CHECK FOR OPEN/CLOSE BRACES
    if ((i1 = str.find_first_of(_R_OBRACE)) != str.find_last_of(_R_OBRACE))
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); // multiple '['
    }
    if ((i2 = str.find_first_of(_R_CBRACE)) != str.find_last_of(_R_CBRACE))
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); // multiple ']'
    }
    if (i1 > 0)
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); //  '[' not first
    }
    if (i2 < str.length() - 1)
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); //']' not last
    }
    if (i2 == std::string::npos)
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); // no closing ']'
    }
    if (i1 == i2 - 1)
    {
        throw std::runtime_error(_R_VECTOR_FORMAT_ERROR_MSG); // empty vector
    }
    str = str.substr(i1 + 1, i2 - i1 - 1); // remove braces from both ends

    // check that vector didn't reduce to all whitespace characters
    if (str.size() == 0) // if all whitespace, return with 0 elements added
        return;

    // Parse the string manually, considering commas inside quoted strings
    std::string temp;
    bool insideQuote = false;
    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i];

        // Toggle the insideQuote flag if we encounter a quote
        if (c == '"' && (i == 0 || str[i - 1] != '\\'))
        {
            insideQuote = !insideQuote;
        }

        // If we encounter a comma and we're not inside quotes, this marks the end of an item
        if (c == ',' && !insideQuote)
        {
            cleanLineEnds(temp);
            parseValue(temp, temp); // Validate that item is a valid string-value
            val.push_back(temp);
            temp.clear();
        }
        else
        {
            // Append character to the current value
            temp += c;
        }
    }

    // Add the last item after parsing the entire string
    if (!temp.empty())
    {
        cleanLineEnds(temp);
        parseValue(temp, temp); // Validate the last item
        val.push_back(temp);
    }
}

/*!
 * Returns value corresponding to specified key.
 * @throws ReaderError if the specified key does not exist or can not be converted to the specified type.
 */
template <class T>
T Reader::getValueByKey(const std::string &key) const
{
    // create temporary working key-string
    std::string tkey(key);
    cleanLineEnds(tkey);
    if (!isCaseSensitive())
        toLower(tkey);
    // First make sure key exists
    auto const &itr = _keyValues.find(tkey);
    if (itr == _keyValues.end())
    {
        throw ReaderError(_R_KEY_NOT_FOUND_ERROR_MSG + key, _fileName.string());
    }
    // Get value stored as a string and convert to
    // specified type
    const std::string &strVal = itr->second.val_;
    try
    {
        T retVal;
        parseValue(strVal, retVal);
        return retVal;
    }
    catch (std::runtime_error &exception)
    { // error converting to required type
        lineData badLine = (_keyValues.find(tkey)->second);
        throw ReaderError(exception.what(), _fileName.string(), badLine.lineNumber_, badLine.rawText_);
    }
    catch (std::exception &exception)
    { // error converting to required type
        lineData badLine = (_keyValues.find(tkey)->second);
        throw ReaderError("Error getting value by key: " + key, _fileName.string(), badLine.lineNumber_, badLine.rawText_);
    }
}

template <class T>
std::optional<T> Reader::getOptional(const std::string &key)
{
    if (containsKey(key))
    {
        return std::optional<T>(getValueByKey<T>(key));
    }
    else
    {
        return std::nullopt;
    }
}

/*! Returns value by key. If key does not exit, returns given default value*/
template <class T>
T Reader::get(const std::string &key, const T &defaultValue)
{
    if (containsKey(key))
    {
        return getValueByKey<T>(key);
    }
    else
    {
        return defaultValue;
    }
}

std::vector<std::string> Reader::getAllKeys() const
{
    /*! Returns vector containing all keys in input file */
    std::vector<std::string> returnKeys;
    for (auto &kv : _keyValues)
        returnKeys.push_back(kv.first);
    return returnKeys;
}

inline void Reader::parseValue(const std::string &strVal, size_t &val) const
{
    /*!
     * Parses to size_t type (may be 64 bit). Checks that value is positive.
     */
    if (!isValidNumber(strVal))
    {
        throw std::runtime_error(_R_NUMBER_FORMAT_ERROR_MSG);
    }
    double dval(0);
    parseValue(strVal, dval);
    if (dval >= 0)
    {
        val = static_cast<size_t>(dval);
    }
    else
    {
        std::string errMsg = _R_BAD_VALUE_ERROR_MSG + strVal + " , expected positive number.";
        throw std::runtime_error(errMsg);
    }
}

inline void Reader::parseValue(const std::string &strVal, unsigned int &val) const
{
    /*!
     * Parses to 32 bit unsigned int. Checks that value is positive.
     */
    // First convert to size_t type, then to unisgned.
    size_t tval(0);
    parseValue(strVal, tval);
    val = static_cast<unsigned int>(tval);
}

inline void Reader::parseValue(const std::string &strVal, bool &val) const
{
    if (strVal == "true" || strVal == "1")
    {
        val = true;
    }
    else if (strVal == "false" || strVal == "0")
    {
        val = false;
    }
    else
    {
        std::string errMsg = _R_BAD_VALUE_ERROR_MSG + strVal + " , expected true or false.";
        throw std::runtime_error(errMsg);
    }
}

bool Reader::validateKey(const std::string &key)
{
    if (key.find_first_of('"') != std::string::npos)
    {
        return false;
    }
    return true;
}

#endif // READER2_H
