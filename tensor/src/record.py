import click
import os
import signal
import traceback

from join_tree import JoinTree
import util


@click.command()
@click.option(
    "--formula", required=True, type=click.File(mode="r"), help="Formula to use",
)
@click.option(
    "--join_tree",
    required=True,
    type=click.File(mode="r"),
    default="-",
    help="Join tree to use",
)
@click.option(
    "--store",
    required=False,
    type=click.Path(writable=True),
    help="Folder to store all discovered join trees",
    default=None,
)
@click.option(
    "--output",
    type=util.KeyValueOutput("a", lazy=False),
    default="-",
    help="File to write output to",
)
@click.option(
    "--timeout",
    required=False,
    type=float,
    default=0,
    help="Timeout for the decomposition search (s).",
)
def run(formula, join_tree, store, output, timeout):
    if store is not None and not os.path.exists(store):
        os.makedirs(store)

    log = []
    log_trees = []
    pid = None
    with util.TimeoutTimer(timeout) as _:
        try:
            formula = util.Formula.parse_DIMACS(formula)
            while True:
                jt, gen_time, new_pid = JoinTree.parse_jt(join_tree)
                if new_pid is not None:
                    pid = new_pid
                if jt is None:
                    break
                log.append(
                    (
                        gen_time,
                        jt.add_width(formula),
                        jt.tensor_width(formula),
                        jt.tensor_flops(formula),
                    )
                )
                if store is not None:
                    log_trees.append((store + "/" + str(len(log)) + ".jt", jt))
        except TimeoutError:
            pass
        except:
            util.log("Error during execution", flush=True)
            util.log(traceback.format_exc())
            output.output_pair("Error", "execution unknown error")

    if pid is not None:
        try:
            os.kill(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass

    output.output_pair("Log", repr(str(log)))
    for filename, jt in log_trees:
        with open(filename, "w") as f:
            jt.write(f)


if __name__ == "__main__":
    run(prog_name=os.getenv("TENSORORDER_CALLER", None))
