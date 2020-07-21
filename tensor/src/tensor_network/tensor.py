class Tensor:
    def __init__(self, base, variables):
        self.base = base
        self.variables = variables

    def duplicate_variable(self, tensor_library, var_index):
        """
        Duplicate the provided variable in the tensor.

        Note that the duplicated variable is inserted as the first index,
        so all variables shift forwards by 1.
        
        :param tensor_library: The underlying tensor library
        :param var_index: The index of the variable to duplicate
        :return: The location of the new variable (0)
        """
        if len(self.variables) == 30:
            raise RuntimeError("Requires tensor rank above 30")

        self.base = tensor_library.stack([self.base, self.base], axis=0)
        self.variables.insert(0, self.variables[var_index])

        lookup = [slice(0, 2) for _ in self.base.shape]
        lookup[0] = 0
        lookup[var_index + 1] = 1
        self.base[tuple(lookup)] = 0

        lookup[0] = 1
        lookup[var_index + 1] = 0
        self.base[tuple(lookup)] = 0

        return 0

    def join_with(self, tensor_library, other, projected_weights):
        """
        Take the product of this tensor with the provided tensor,
        then project out all specified variables.

        The result is stored in this tensor.

        :param tensor_library: The underlying tensor library
        :param other: The tensor to multiply by (may be modified during execution)
        :param projected_weights: The variables to project out, mapped to their weights
        :return: None
        """

        # Duplicate the variables that appear in both (but are not projected out)
        var_both = [var for var in self.variables if var in other.variables]
        for var in reversed(var_both):
            if var in projected_weights:
                # Include the weights of the projected variables
                var_index = self.variables.index(var)
                lookup = [slice(0, 2) for _ in self.base.shape]
                lookup[var_index] = 0
                self.base[tuple(lookup)] *= projected_weights[var][0]
                lookup[var_index] = 1
                self.base[tuple(lookup)] *= projected_weights[var][1]

                del projected_weights[var]  # Projection will be done by tensordot
                continue  # no need to duplicate
            elif len(self.variables) <= len(other.variables):
                self.duplicate_variable(tensor_library, self.variables.index(var))
            else:
                other.duplicate_variable(tensor_library, other.variables.index(var))

        # Compute the resulting tensor
        if len(self.base.shape) + len(other.base.shape) - 2 * len(var_both) > 30:
            raise RuntimeError("Requires tensor rank above 30")

        left_summed = [self.variables.index(var) for var in var_both]
        right_summed = [other.variables.index(var) for var in var_both]
        self.base = tensor_library.tensordot(
            self.base, other.base, (left_summed, right_summed)
        )

        # Determine the variables of the new tensor
        self.variables = [
            var for j, var in enumerate(self.variables) if j not in left_summed
        ] + [var for j, var in enumerate(other.variables) if j not in right_summed]

        # Project remaining variables
        self.project_out(tensor_library, projected_weights)

    def project_out(self, tensor_library, projected_weights):
        """
        Project out the provided variables.
        
        :param tensor_library: The underlying tensor library
        :param projected_weights: The variables to project out, mapped to their weights
        :return: None
        """
        if len(projected_weights) == 0:
            return

        for var in projected_weights:
            self.base = tensor_library.tensordot(
                self.base, projected_weights[var], ([self.variables.index(var), 0])
            )
            self.variables.remove(var)

    @staticmethod
    def from_clause(tensor_library, clause):
        # Compute the resulting tensor
        if len(clause) > 30:
            raise RuntimeError("Requires tensor rank above 30")

        # Record the variable that corresponds to each tensor index, with no repeated variables
        variables = list({abs(lit) for lit in clause})

        # Build the tensor for this clause
        base = tensor_library.create_tensor([2 for _ in variables], 1)

        # If a variable and its negation appear in a clause, the clause is trivial
        if len(variables) != len(clause):
            for var in variables:
                if var in clause and -var in clause:
                    return Tensor(base, variables)

        base[tuple(1 if var in clause else 0 for var in variables)] = 0

        return Tensor(base, variables)
