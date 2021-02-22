cnfFile=$1
treeDecomposer=$2
diagramVarOrder=$3

if [[ $treeDecomposer == flow ]]; then
  lgArg="/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100"
elif [[ $treeDecomposer == htd ]]; then
  lgArg="/solvers/htd-master/bin/htd_main -s 1234567 --opt width --iterations 0 --strategy challenge --print-progress --preprocessing full"
else # tcs
  lgArg="java -classpath /solvers/TCS-Meiji -Xmx25g -Xms25g -Xss1g tw.heuristic.MainDecomposer -s 1234567 -p 100"
fi

lg.sif "$lgArg" < $cnfFile | dmc --jf=- --cf=$cnfFile --dv=$diagramVarOrder
