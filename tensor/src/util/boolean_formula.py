from collections import defaultdict


class Formula:
    def __init__(self):
        self._variables = defaultdict(lambda: (0.5, 0.5))
        self._clauses = []

    def add_clause(self, literals):
        """
        Add a new CNF clause representing the disjunction of the provided literals.

        The elements of literals should be variable id (representing the corresponding
        positive literal) or the negative of a variable id (for the negative literal).

        :param literals: An iterable of variable ids and negations of variable ids.
        :return: None
        """
        self._clauses.append(list(literals))

    def set_weight(self, var_id, neg_weight, pos_weight):
        """
        Set the weight of the variable in the formula.

        :param var_id: Variable to set weight of
        :param neg_weight: Multiplicative weight on an assignment when variable is false
        :param pos_weight: Multiplicative weight on an assignment when variable is true
        :return: None
        """
        self._variables[var_id] = (neg_weight, pos_weight)

    def clause(self, clause_id):
        return self._clauses[clause_id]

    @property
    def clauses(self):
        return list(self._clauses)

    @property
    def variables(self):
        return list(self._variables.keys())

    def literal_weight(self, lit):
        """
        Returns the multiplicative weight of the provided DIMACS literal.
        """
        return self._variables[abs(lit)][1 if lit > 0 else 0]

    @staticmethod
    def parse_DIMACS(file):
        """
        Parse a DIMACS file into a formula.

        The file may optionally contain weights, in the cachet style i.e. lines of the form
        w [var id] [prob]
        that each indicate that the variable [var id] should have positive literal weight [prob]
        and negative literal weight 1-[prob].

        If [prob] is -1, the variable is unweighted

        :param file: A handler to the file to read
        :param include_missing_vars: If true, variables indicated by the DIMACS header are assigned a weight 1 1
        :return: the resulting formula
        """
        result = Formula()

        for line in file:
            if line.startswith("c weights"):  # MiniC2D weights
                weights = line.split(" ")[2:]
                for i in range(len(weights) // 2):
                    result.set_weight(
                        i + 1, float(weights[2 * i + 1]), float(weights[2 * i])
                    )
            elif len(line) == 0 or line[0] == "c":
                continue
            elif line[0] == "p":
                num_vars = int(line.split()[2])
            elif line[0] == "w":  # Cachet weights
                args = line.split()
                if float(args[2]) == -1:
                    result.set_weight(int(args[1]), 1, 1)
                else:
                    prob = float(args[2])
                    result.set_weight(int(args[1]), 1 - prob, prob)
            else:
                args = line.split()
                literals = [int(lit) for lit in args if lit != "0"]
                if len(literals) == 0:
                    continue
                result.add_clause(literals)

        return result
