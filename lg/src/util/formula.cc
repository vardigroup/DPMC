/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include "util/formula.h"

#include <algorithm>
#include <string_view>

#include "util/dimacs_parser.h"

namespace util {
  bool Formula::add_clause(std::vector<int> literals) {
    for (int literal : literals) {
      if (literal == 0 || static_cast<size_t>(abs(literal)) > num_variables_) {
        return false;
      }
    }

    clauses_.push_back(literals);
    clause_variables_.push_back(std::vector<size_t>());
    clause_variables_.back().reserve(literals.size());
    for (int literal : literals) {
      clause_variables_.back().push_back(abs(literal));
    }

    std::sort(clause_variables_.back().begin(), clause_variables_.back().end());
    clause_variables_.back().erase(std::unique(clause_variables_.back().begin(),
                                               clause_variables_.back().end()),
                                   clause_variables_.back().end());
    return true;
  }

  std::optional<Formula> Formula::parse_DIMACS(std::istream *stream) {
    util::DimacsParser parser(stream);

    // Parse the header
    std::vector<double> entries;
    std::string prefix;
    do {
      entries.clear();
      prefix = parser.parseLine(&entries);
    } while (prefix != "" && prefix.at(0) == 'c');  // skip comments
    if ((prefix != "p cnf" && prefix != "p wpcnf" &&
        prefix != "p wcnf" && prefix != "p pcnf") || entries.size() < 2) {
      std::cerr << "Parse error: Unexpected header " << prefix << std::endl;
      return std::nullopt;
    }

    Formula result(static_cast<int>(entries[0]));
    int num_clauses_to_parse = entries[1];
    int expected_additive_vars = -1;
    if (entries.size() > 2) {
      expected_additive_vars = entries[2];
    }

    // Parse the remaining clauses
    while (!parser.finished()) {
      entries.clear();
      prefix = parser.parseLine(&entries);

      if (prefix == "w") {
        continue;  // Ignore weight lines
      } else if (prefix == "vp" || prefix == "c p show") {
        // vp [x] [y] ... [z] 0 indicates {x, ..., z} are additive vars
        for (double entry : entries) {
          if (entry != 0) {   // ignore trailing 0s, if they exist
            result.relevant_vars_.push_back(entry);
          }
        }
      } else if (prefix == "") {
        // [x] [y] ... [z] 0 indicates a clause with literals (x, y, ..., z)
        if (entries.size() == 0 || entries.back() != 0) {
          std::cerr << "Parse error: Empty clause detected" << std::endl;
          return std::nullopt;
        }

        // Clause lines end in a trailing 0, so remove it
        std::vector<int> clause(entries.begin(),
                                std::prev(entries.end()));
        if (!result.add_clause(clause)) {
          std::cerr << "Parse error: Invalid literal" << std::endl;
          return std::nullopt;
        }
        num_clauses_to_parse--;
      } else if (prefix == "c t mc" || prefix == "c t wmc") {
        // Headers from MCC21 indicating (unprojected) model counting
        // and (unprojected) weighted model counting
        expected_additive_vars = 0;
      } else if (prefix.at(0) == 'c') {
        // Comment; Do nothing
      } else {
        // Unknown line
        std::cerr << "Parse error: Unknown prefix " << prefix << std::endl;
        return std::nullopt;
      }
    }

    // Verify that we have parsed the correct number of clauses
    if (num_clauses_to_parse != 0) {
      std::cerr << "Parse warning: Expected " << num_clauses_to_parse;
      std::cerr << " more clauses" << std::endl;
      // return std::nullopt; // warning, not error
    }

    // Verify that we have parsed the correct number of additive vars
    if (expected_additive_vars >= 0
        && result.relevant_vars_.size() != expected_additive_vars) {
      std::cerr << "Parse error: Expected " << expected_additive_vars;
      std::cerr << " additve variables, got " << result.relevant_vars_.size();
      std::cerr << std::endl;
      return std::nullopt;
    }

    return result;
  }

  GradedClauses Formula::graded_clauses() {
    util::GradedClauses result(clause_variables_);
    // If relevant variables were specified, group using them
    if (relevant_vars_.size() > 0) {
      result.group_by(relevant_vars_, num_variables_);
    }
    return result;
  }
}  // namespace util
