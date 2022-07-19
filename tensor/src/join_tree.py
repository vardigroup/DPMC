import itertools


class JoinTreeNode:
    """
    A simple class for an internal join tree node.
    """

    def __init__(self, children, projected):
        self._children = children
        self._projected = projected

    @property
    def children(self):
        return self._children

    @property
    def projected(self):
        return self._projected

    def __str__(self):
        return " ".join(
            str(c)
            for c in itertools.chain(reversed(self._children), ["e"], self._projected)
        )


class JoinTree:
    """
    A class representing a join tree.
    """

    def __init__(self, num_leaves, root):
        """
        Create a join tree.

        :param num_leaves: The number of leaves to allow in the join tree.
        :param root: The root of the join tree.
        """
        self._num_clauses = num_leaves
        self._nodes = {}
        self._root = root

    def add_node(self, node_id, children, projected):
        """
        Add an internal node to the join tree.

        :param node_id: The id of the node to add.
        :param children: The list of children at this node.
        :param projected: The list of variables to project.
        :return: None
        """
        self._nodes[node_id] = JoinTreeNode(children, projected)

    def visit(self, at_internal_node, at_leaf):
        """
        Visit all nodes of the join tree in a postorder traversal.

        :param at_internal_node: Function to compute the result for an internal node.
                                 Takes [list of child results] and [projected variables] as arguments.
        :param at_leaf: Function to compute the result for a child node.
                         Takes [node id] as argument.
        :return: The result at the root node.
        """
        processed = [(self._root, False)]
        result_stack = []

        while len(processed) > 0:
            node, expanded = processed.pop()
            if node <= self._num_clauses:
                result_stack.append(at_leaf(node))
            elif expanded:
                # We have visited all children of this internal node

                # Gather the child results
                num_children = len(self._nodes[node].children)
                children_results = result_stack[-num_children:]
                del result_stack[-num_children:]

                # Compute the result at this node
                result_stack.append(
                    at_internal_node(children_results, self._nodes[node].projected)
                )
                pass
            else:
                processed.append((node, True))
                processed.extend((child, False) for child in self._nodes[node].children)
        return result_stack.pop()

    def _get_costs(self, formula, join_cost):
        costs = []

        def at_leaf_node(node_id):
            variables = {abs(lit) for lit in formula.clause(node_id - 1)}
            return variables

        def at_internal_node(children, projected_vars):
            if len(children) == 1:
                total_variables = children[0]
                for variable in projected_vars:
                    total_variables.discard(variable)
                return total_variables

            total_variables = children[0]
            for i, variables in enumerate(children[1:]):
                costs.append(
                    join_cost(
                        total_variables,
                        variables,
                        projected_vars if i == len(children) - 1 else [],
                    )
                )
                total_variables.update(variables)

            for variable in projected_vars:
                total_variables.discard(variable)
            return total_variables

        _ = self.visit(at_internal_node, at_leaf_node)
        return costs

    def _compute_width(self, formula, score_join):
        def at_leaf_node(node_id):
            variables = {abs(lit) for lit in formula.clause(node_id - 1)}
            return variables, len(variables)

        def at_internal_node(children, projected_vars):
            if len(children) == 1:
                total_variables = children[0]
                for variable in projected_vars:
                    total_variables.discard(variable)
                return total_variables, children[0]

            total_variables, highest_width = children[0]
            for i in enumerate(children[1:]):
                variables, width = children[i]
                intermediate_width = score_join(
                    total_variables,
                    variables,
                    projected_vars if i == len(children) - 1 else [],
                )
                total_variables.update(variables)
                highest_width = max(highest_width, width, intermediate_width)

            for variable in projected_vars:
                total_variables.discard(variable)
            return total_variables, highest_width

        _, result = self.visit(at_internal_node, at_leaf_node)
        return result

    def add_width(self, formula):
        """
        Compute the width when ADDs are used to execute this join tree.
        """

        def score_add_join(left, right, _):
            return len(left | right)

        highest_clause_length = max(len({abs(v) for v in c}) for c in formula.clauses)
        highest_join_length = max(self._get_costs(formula, score_add_join))
        return max(highest_clause_length, highest_join_length)

    def tensor_width(self, formula):
        """
        Compute the width when tensors are used to execute this join tree.
        """

        def score_tensor_join(left, right, projected_vars):
            var_both = [var for var in left if var in right]
            left_size = len(left)
            right_size = len(right)
            for var in reversed(var_both):
                if var not in projected_vars:
                    # Non-projected variable that appears in both must be duplicated
                    if left_size <= right_size:
                        left_size += 1
                    else:
                        right_size += 1
            result_size = left_size + right_size - 2 * len(var_both)
            return max(left_size, right_size, result_size)

        highest_clause_length = max(len({abs(v) for v in c}) for c in formula.clauses)
        highest_join_length = max(self._get_costs(formula, score_tensor_join))
        return max(highest_clause_length, highest_join_length)

    def tensor_flops(self, formula):
        def flops_tensor_join(left, right, projected_vars):
            var_both = [var for var in left if var in right]
            total_axes = (
                len(left)
                + len(right)
                + sum(1 for var in var_both if var not in projected_vars)
            )

            log_num_sums = total_axes - 2 * len(var_both)
            log_terms_in_sum = len(var_both)
            if log_num_sums + log_terms_in_sum > 100:
                return 2 ** 100  # Cap
            else:
                return 2 ** (log_num_sums + log_terms_in_sum)

        return sum(self._get_costs(formula, flops_tensor_join))

    @staticmethod
    def parse_jt(file, log=lambda _: None):
        """
        Parse a .jt file into a Join Tree.

        :param file: A handler to the file to read
        :param log: Function to print error message
        :return: the resulting join tree, and the time taken to generate it
        """
        gen_time = None
        result = None
        pid = None
        for line in file:
            if len(line) <= 1 or line.startswith("c"):
                if line.startswith("c seconds"):
                    gen_time = float(line.split()[-1])
                elif line.startswith("c pid"):
                    pid = int(line.split()[-1])
                continue
            elif line.startswith("p jt"):
                header = line.split()
                result = JoinTree(int(header[3]), int(header[4]))
                break
            else:
                log("Unknown line in .jt before header: " + line)
                return None, gen_time, pid
        if result is None:
            log("No header found in .jt")
            return None, gen_time, pid

        for line in file:
            if len(line) <= 1 or line.startswith("c"):
                if line.startswith("c seconds"):
                    gen_time = float(line.split()[-1])
                elif line.startswith("c pid"):
                    pid = int(line.split()[-1])
                continue
            elif line.startswith("="):
                break
            else:
                internal_node = line.split()
                try:
                    projected_split = internal_node.index("e")
                    result.add_node(
                        int(internal_node[0]),
                        [int(i) for i in reversed(internal_node[1:projected_split])],
                        [int(i) for i in internal_node[projected_split + 1 :]],
                    )
                except ValueError:
                    log("Line does not contain 'e': " + line)
                    return None, gen_time, pid
        return result, gen_time, pid

    def write(self, file):
        max_var = max(
            max(self._nodes[node].projected)
            for node in self._nodes
            if len(self._nodes[node].projected) > 0
        )
        file.write(
            " ".join(["p jt", str(max_var), str(self._num_clauses), str(self._root)])
        )
        file.write("\n")
        for node in self._nodes:
            file.write(str(node) + " " + str(self._nodes[node]) + "\n")
