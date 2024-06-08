#!/usr/bin/env python3

import argparse
import functools
import math
import os
import subprocess
import time

print = functools.partial(print, flush=True)

MC = 'mc'
WMC = 'wmc'
PMC = 'pmc'
PWMC = 'pwmc'

UI = 'ui'
TU = 'tu'

toolIndex = 0

def cat(cmd):
    assert isinstance(cmd, list), cmd
    return ' '.join(cmd)

def printCallLine(cmd):
    global toolIndex
    toolIndex += 1
    print(f'c o CALLS({toolIndex}) {cat(cmd)}')

def getBinPath(*paths):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), *paths)

def addArgs(argParser):
    argParser.add_argument(
        'cnf',
        help='path to benchmark file (stdin unsupported)',
    )
    argParser.add_argument(
        '--task',
        choices=(MC, WMC, PMC, PWMC),
        help='track',
        default=MC,
    )
    argParser.add_argument(
        '--pre',
        choices=(0, 1),
        help='whether to call preprocessor pmc',
        default=1,
        type=int,
    )
    argParser.add_argument(
        '--width',
        help='max width of tree decomposition',
        default=100,
        type=int,
    )
    argParser.add_argument(
        '--mp',
        choices=(0, 1),
        help='multiple precision',
        default=0,
        type=int,
    )
    argParser.add_argument(
        '--vs',
        choices=(0, 1, 2, 3),
        help='verborse solving',
        default=0,
        type=int,
    )
    argParser.add_argument(
        '--cluster',
        choices=[UI, TU],
        help='U of Iowa [StarExec], or TU Dresden [Taurus]',
        default=UI,
    )

def addUiArgs(argParser):
    argParser.add_argument(
        'outdir',
        help='[StarExec only] output dir',
        nargs='?',
        default=getBinPath(),
    )

def addTuArgs(argParser):
    argParser.add_argument(
        '--tmpdir',
        help='[Taurus only] temporary dir',
        default=getBinPath(),
    )
    argParser.add_argument(
        '--maxrss',
        help='[Taurus only] RAM cap in GB',
        default=4.0,
        type=float,
    )

def printPreprocessorSolution(count, task, mp): # UNSAT or MC only
    print(f's {"" if count else "UN"}SATISFIABLE')
    print(f'c s type {task}')
    print(f'c s log10-estimate {math.log10(count) if count else -math.inf}')
    print(f'c s exact {"arb int" if mp else "double prec-sci"} {count}')

def preprocessCnf(megs, cnf, outDirPath, task, mp, vs):
    startTime = time.time()
    def printTime():
        print(f'c preprocessor seconds: {time.time() - startTime}\n')

    cmd = [
        getBinPath('pmc'),
        cnf,
        # f'-mem-lim={megs}', # segfault if megs in range(8192, 8199)
        # '-verb=2', # would print line '|  Garbage collection:' (default: 1)
        # '-solve', # with SAT solving (default: -no-solve)
    ]
    cmd += [ # equivalent
        '-vivification',
        '-eliminateLit',
        '-litImplied',
        '-iterate=10',
    ]

    cpLines = [] # weight lines and show lines
    if task == MC:
        cmd += [ # equinumerous
            '-equiv',
            '-orGate',
            '-affine',
        ]
    else: # WMC, PMC, PWMC
        with open(cnf) as inFile:
            for line in open(cnf):
                if line.startswith('c p'):
                    cpLines.append(line)

    outFilePath = os.path.join(outDirPath, 'preprocessed_' + os.path.basename(cnf))
    cmd.append(f'>{outFilePath}')

    if not vs:
        cmd.append('2>/dev/null')

    printCallLine(cmd)

    subprocess.run(
        cat(cmd),
        shell=True,
    )

    with open(outFilePath) as outFile: # preprocessor may have solved benchmark
        count = None
        for line in outFile:
            words = line.split()
            if len(words) > 1:
                if words[0] in {'c', 's'} and words[1] == 'UNSATISFIABLE':
                    count = 0
                elif words[0] == 's':
                    count = int(words[1])

                if count != None:
                    printTime()
                    if count == 0 or task == MC:
                        printPreprocessorSolution(count, task, mp)
                        print('\nc exiting after preprocessor solved benchmark')
                        exit(0)
                    else: # SAT and (WMC or PMC or PWMC)
                        print(f'c ignored preprocessed solution {count} due to task {task}')
                        return cnf

    with open(outFilePath, 'a') as outFile: # appends weight and show lines
        outFile.write(''.join(cpLines))

    printTime()
    return outFilePath

def planJt(cnfPath, width, task, vs):
    lgArg = f'{getBinPath("flow_cutter_pace17")} -p {width}'
    cmd = [
        getBinPath('lg'),
        f'"{lgArg}"',
        f'<{cnfPath}',
    ]

    if not vs:
        cmd.append('2>/dev/null')

    printCallLine(cmd + ['\\'])

    return subprocess.Popen(
        cat(cmd),
        shell=True,
        stdout=subprocess.PIPE,
    )

def main():
    formatter = lambda prog: argparse.ArgumentDefaultsHelpFormatter(prog, max_help_position=30)
    parser = argparse.ArgumentParser(formatter_class=formatter)
    addArgs(parser)
    addUiArgs(parser)
    addTuArgs(parser)

    (args, unknowns) = parser.parse_known_args()
    print(f'c args: {args}')
    if unknowns:
        print(f'c ignored args: {unknowns}')

    if args.cluster == UI:
        outDirPath = os.path.realpath(args.outdir)

        MAX_MEM = 'STAREXEC_MAX_MEM'
        megs = os.environ[MAX_MEM]
        print(f'c env var: ${MAX_MEM}={megs}')
    else:
        outDirPath = os.path.realpath(args.tmpdir)

        megs = args.maxrss * 1e3

    megs = int(megs)
    print()

    if args.pre:
        os.makedirs(outDirPath, exist_ok=True)
        cnfPath = preprocessCnf(megs, args.cnf, outDirPath, args.task, args.mp, args.vs)
    else:
        cnfPath = args.cnf

    plannerProcess = planJt(cnfPath, args.width, args.task, args.vs)

    dmcCmd = [
        getBinPath('dmc'),
        f'--cf={cnfPath}',
        f'--wc={int(args.task in {WMC, PWMC})}',
        f'--pc={int(args.task in {PMC, PWMC})}',
        f'--dp={"s" if args.mp else "c"}',
        f'--mm={megs}',
        f'--mp={args.mp}',
        f'--lc={int(not args.mp)}',
        f'--vc={args.vs}',
        f'--vj={args.vs}',
        f'--vs={args.vs}',
    ]
    dmcProcess = subprocess.Popen(
        dmcCmd,
        stdin=plannerProcess.stdout,
    )

    plannerProcess.stdout.close() # allows plannerProcess to receive SIGPIPE if dmcProcess exits

    printCallLine(['|'] + dmcCmd)
    print()

    dmcProcess.communicate()

    dmcExit = dmcProcess.returncode
    print(f'\nc exiting with dmc return code: {dmcExit}')
    exit(dmcExit)

if __name__ == '__main__':
    main()
