DIR=`dirname $0`

find $DIR/data -name _inputs.zip -delete
find $DIR/data -name _outputs.zip -delete
find $DIR/data -name _worker_logs.zip -delete

find $DIR/data -name _results.db -delete

rm -rf figures
