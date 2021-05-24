/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace util {
/**
 * Parses DIMACS-like text file, i.e. where all non-comment lines match:
 *   [^\d.-]*( -?\d*.?\d*)*
 * (i.e. have a string prefix, followed by 0+ space-separated doubles).
 *
 * A line is considered a comment (and ignored) if it begins with 'c'.
 */
class DimacsParser {
 public:
  /**
   * Constructs a parser to parse the provided input stream.
   * If given, output comment lines to the provided output stream.
   * Note output comment lines will not be parsed in this case.
   * 
   * If no comment stream is given, comment lines will be parsed as usual.
   * 
   * Provided streams should outlive the DimacsParser.
   */
  explicit DimacsParser(std::istream *stream);
  explicit DimacsParser(std::istream *stream, std::ostream *comment_stream);

  /**
   * Returns true if we have parsed all of the stream, and false otherwise.
   */
  bool finished();

  /**
   * Consumes and parses the next (non-empty, non-comment) line of the stream.
   * 
   * The string prefix is returned and each double is added to out.
   */
  std::string parseLine(std::vector<double> *out);

  /**
   * Parses the next (non-empty, non-comment) line of the stream, expecting the
   * given prefix.
   * 
   * Returns false (and consumes nothing) if the given prefix is missing.
   * Otherwise returns true, consumes the next line, and each double is added
   * to out.
   */
  bool parseExpectedLine(const std::string_view &prefix,
                         std::vector<double> *out);

 private:
  /**
   * Consumes characters from the input stream until the start of the next
   * non-empty non-comment line.
   */
  void skipToContent();

  // The underlying input stream being parsed.
  std::istream *input_stream_;
  std::ostream *comment_stream_;
};
}  // namespace util
