#!/bin/bash
producer_py3="python3 producer-py/producer.py"
producer_py2="python2 producer-py/producer.py"
producer=${producer_py3}
consumer_dir="consumer-c/"
consumer="./consumer"
consumer_make="./make_all.sh"
cleanup="producer-py/cleanup.py"
images_dir="../images"

killall consumer &>/dev/null
echo "Compiling consumer.."
(cd $consumer_dir && $consumer_make)

echo "Cleanup.."
$cleanup

echo "Starting consumer.."
(cd $consumer_dir && $consumer &)

echo "Starting producer loop.."
echo "TIME:"
time for file in `ls $images_dir`; do
	$producer $images_dir/$file;
done;
killall consumer
