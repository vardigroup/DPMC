#!/bin/bash

DIR=`dirname $0`

find $DIR -name __pycache__ -exec rm -r {} +

find $DIR/data -name _inputs.zip -delete
find $DIR/data -name _outputs.zip -delete
find $DIR/data -name _worker_logs.zip -delete

find $DIR/data -name _results.db $@ | sort
