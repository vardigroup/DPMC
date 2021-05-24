/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include "util/dimacs_parser.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>
#include <utility>

namespace util {
  DimacsParser::DimacsParser(std::istream *stream)
  : input_stream_(stream), comment_stream_(nullptr)
  {}

  DimacsParser::DimacsParser(std::istream *stream, std::ostream *comment_stream)
  : input_stream_(stream), comment_stream_(comment_stream)
  {}

  std::string DimacsParser::parseLine(std::vector<double> *out) {
    if (finished()) return "";

    // Find the prefix of the first line
    std::string line;
    std::getline(*input_stream_, line);
    std::size_t split = line.find_first_of("-.0123456789");
    if (split == std::string::npos) {
      split = line.size();
    } else {
      // Copy all doubles from the line (beyond the prefix) into out
      std::stringstream line_stream(line);
      line_stream.seekg(static_cast<std::streamoff>(split));
      std::copy(std::istream_iterator<double>(line_stream),
                std::istream_iterator<double>(),
                std::back_inserter(*out));
    }


    // Remove trailing whitespace from the prefix, and return it
    if (split == 0) return "";
    while (split > 0 && (line.at(split-1) == ' '
                         || line.at(split-1) == '\t'
                         || line.at(split-1) == '\n'
                         || line.at(split-1) == '\r')) {
      split--;
    }
    return line.substr(0, split);
  }

  bool DimacsParser::parseExpectedLine(const std::string_view &prefix,
                     std::vector<double> *out) {
    if (finished()) return false;

    // Peek at the start of the next line. Consume prefix if it matches.
    std::streampos oldPos = input_stream_->tellg();
    auto match = std::mismatch(prefix.begin(), prefix.end(),
                               std::istreambuf_iterator<char>(*input_stream_));
    if (match.first != prefix.end()) {
      input_stream_->seekg(oldPos);  // Reset to the beginning of the line
      return false;
    }

    // Copy all remaining doubles from the line into out
    std::string line;
    std::getline(*input_stream_, line);
    std::stringstream line_stream(line);
    std::copy(std::istream_iterator<double>(line_stream),
              std::istream_iterator<double>(),
              std::back_inserter(*out));
    return true;
  }

  void DimacsParser::skipToContent() {
    char next = input_stream_->peek();
    // Ignore comment lines (starting with 'c') and empty lines
    while ((next == 'c' && comment_stream_ != nullptr)
           || next == '\n' || next == '\r') {
      if (next == 'c' && comment_stream_ != nullptr) {
        std::string line;
        std::getline(*input_stream_, line);
        (*comment_stream_) << line << std::endl;
      } else {
        input_stream_->ignore(std::numeric_limits<std::streamsize>::max(),
                              '\n');
      }
      next = input_stream_->peek();
    }
  }

  bool DimacsParser::finished() {
    if ( input_stream_->eof())
      return true;
    skipToContent();
    return input_stream_->eof();
  }
}  // namespace util
