#!/usr/bin/env python3


import click
import os
import signal
import sys
import traceback

import join_tree as jt
import tensor_network
import util


@click.command(context_settings={
    "max_content_width": 105,
    "help_option_names": ["-h", "--help"],
    "show_default": True,
})
@click.option("--formula",
    type=click.File(mode="r"),
    required=True,
    help="Formula to use.",
)
@click.option("--join_tree",
    type=click.File(mode="r"),
    default="-",
    help="Join tree to use (default is stdin).",
)
@click.option("--output",
    type=util.KeyValueOutput("a", lazy=False),
    default="-",
    help="File to write output to (default is stdout).",
)
@click.option("--max_width",
    type=int,
    default=30,
    help="Join trees of larger tensor width are not used.",
)
@click.option("--timeout",
    type=float,
    default=0.0,
    help="Timeout for the contraction (s).",
)
@click.option("--performance_factor",
    type=float,
    default=1e-11,
    help="Float to scale estimated computation time by.",
)
@click.option("--thread_limit",
    type=int,
    default=1,
    help="Number of threads to limit tensor manipulations.",
)
@click.option("--entry_type",
    type=click.Choice(
        ["uint", "int", "bigint", "float16", "float32", "float64"], case_sensitive=False
    ),
    default="float64",
    help="Data type to use for all tensor computations.",
)
@click.option("--tensor_library",
    type=util.TaggedChoice(tensor_network.ALL_APIS, case_sensitive=False),
    default="numpy",
    help="Tensor library to use.",
)
def run(
    formula,
    join_tree,
    output,
    max_width,
    timeout,
    performance_factor,
    thread_limit,
    entry_type,
    tensor_library,
):
    sys.setrecursionlimit(100000)
    tensor_library = tensor_library(entry_type, thread_limit=thread_limit)

    stopwatch = util.Stopwatch()
    with util.TimeoutTimer(timeout) as timer:
        try:
            formula = util.Formula.parse_DIMACS(formula)
            stopwatch.record_interval("Parse Formula")

            tree = get_join_tree(
                formula, join_tree, timer, output, max_width, performance_factor
            )
            if tree is not None:
                timer.reset_timeout(timeout)
                stopwatch.record_interval("Parse Join Tree")

                count = execute_join_tree(formula, tree, tensor_library, output)
                if count is not None:
                    output.output_pair("Count", count)
                    stopwatch.record_interval("Execution")
            stopwatch.record_total("Total")
        except TimeoutError:
            util.log("Parsing timed out", flush=True)
            output.output_pair("Error", "execution timeout")
        except:
            util.log("Unknown error", flush=True)
            util.log(traceback.format_exc())
            output.output_pair("Error", "unknown error")

    for name, record in stopwatch.records.items():
        output.output_pair(name + " Time", record)


def get_join_tree(
    formula, join_tree_stream, timer, output, max_width, performance_factor
):
    best_join_tree = None
    best_time = None
    best_width = None
    pid = None
    timed_out = False

    try:
        while True:
            next_join_tree, gen_time, new_pid = jt.JoinTree.parse_jt(
                join_tree_stream, log=util.log
            )
            if next_join_tree is None:
                break
            width = next_join_tree.tensor_width(formula)
            util.log("Parsed join tree with tensor width " + str(width), flush=True)

            if new_pid is not None:
                pid = new_pid

            if best_width is None or width < best_width:
                best_join_tree = next_join_tree
                best_time = gen_time
                best_width = width

                if width <= max_width:
                    flops = next_join_tree.tensor_flops(formula)
                    timer.recap_timeout(flops * performance_factor)
    except TimeoutError:
        timed_out = True
    except:
        util.log("Error during join tree parsing", flush=True)
        util.log(traceback.format_exc())
        output.output_pair("Error", "tree unknown error")

    # Kill the planning process when done
    if pid is not None:
        try:
            os.kill(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass

    if best_join_tree is None:
        if timed_out:
            util.log("Timed out waiting for join tree", flush=True)
            output.output_pair("Error", "tree timeout")
        else:
            util.log("Unable to parse join tree", flush=True)
            output.output_pair("Error", "tree parse")
        return None
    elif best_width > max_width:
        util.log("Join tree has width higher than " + str(max_width), flush=True)
        output.output_pair("Error", "tree large")
        return None
    else:
        if best_time is not None:
            output.output_pair("Join Tree Time", best_time)
        return best_join_tree


def execute_join_tree(formula, join_tree, tensor_library, output):
    def at_leaf(node_id):
        return tensor_network.Tensor.from_clause(
            tensor_library, formula.clause(node_id - 1)
        )

    def at_internal_node(children, projected_vars):
        projected_weights = {
            var: (formula.literal_weight(-var), formula.literal_weight(var))
            for var in projected_vars
        }

        # Handle join tree internal nodes that have no children
        children = [c for c in children if c is not None]
        if len(children) == 0:
            return None

        result = children[0]
        if len(children) == 1:
            # Nothing to join with, just do the projection
            result.project_out(tensor_library, projected_weights)
            return result

        # Perform all joins, including the projection for the last
        for other in children[1:-1]:
            result.join_with(tensor_library, other, [])
        result.join_with(tensor_library, children[-1], projected_weights)

        return result

    try:
        count = join_tree.visit(at_internal_node, at_leaf)
        return count.base[tuple()]
    except TimeoutError:
        util.log("Execution timed out", flush=True)
        output.output_pair("Error", "execution timeout")
        return None
    except:
        util.log("Error during execution", flush=True)
        util.log(traceback.format_exc())
        output.output_pair("Error", "execution unknown error")
        return None


if __name__ == "__main__":
    run(prog_name=os.getenv("TENSORORDER_CALLER", None))
