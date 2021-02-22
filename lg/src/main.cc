/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include <sys/types.h>
#include <unistd.h>

#include "util/dimacs_parser.h"
#include "util/formula.h"
#include "util/graded_clauses.h"
#include "decomposition/tree_decomposition.h"
#include "decomposition/join_tree.h"

#include <boost/process.hpp>

int main(int argc, char *argv[]) {
  // Print help message
  if (argc == 2 &&
      (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
      std::cout << argv[0] << " [TREE DECOMPOSER]" << std::endl;
      std::cout << "    Use [TREE DECOMPOSER] to make join trees." << std::endl;
      std::cout << "    Input formula is parsed from STDIN." << std::endl;
      std::cout << "    Join trees are written to STDOUT." << std::endl;
      return 0;
  }

  if (argc != 2) {
    std::cerr << "Error: Exactly 1 argument required." << std::endl;
    return -1;
  }

  try {
    // Start the tree decomposition solver.
    boost::process::opstream solver_input;
    boost::process::ipstream solver_output;
    boost::process::child solver(argv[1],
                                boost::process::std_out > solver_output,
                                boost::process::std_in < solver_input);

    pid_t pid = solver.id();
    std::cout << "c pid " << pid << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    // Parse the input formula
    std::optional<util::Formula> f = util::Formula::parse_DIMACS(&std::cin);
    if (!f.has_value()) {
      solver.terminate();
      std::cerr << "Error: Unable to process formula." << std::endl;
      return -1;
    }

    // Provide the line graph of the input formula to the solver.
    util::GradedClauses clauses = f->graded_clauses();
    clauses.write_line_graph(&solver_input, f->num_variables());
    solver_input.flush();
    solver_input.pipe().close();

    while (true) {
      // Read a single tree decomposition from the solver.
      auto td = decomposition::TreeDecomposition::parse_one(&solver_output);
      if (!td.has_value()) {
        std::cerr << "Tree decomposition stream ended." << std::endl;
        solver.terminate();
        return -1;
      }

      // Convert the tree decomposition into a join tree.
      auto jt = decomposition::JoinTree::graded_from_tree_decomposition(
        clauses, *f, *td);
      if (!jt.has_value()) {
        std::cerr << "Error: Unable to build join tree." << std::endl;
        solver.terminate();
        return -1;
      }

      // Output the join tree to stdout.
      jt->write(&std::cout);

      auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::steady_clock::now() - start_time).count();
      std::cout << "c seconds " << elapsed << "\n";
      std::cout << "=" << std::endl;
    }

    solver.terminate();
    return 0;
  } catch (boost::process::process_error& e) {
    std::cerr << "Error: Unable to run tree decomposition solver." << std::endl;
    return -1;
  }
}
