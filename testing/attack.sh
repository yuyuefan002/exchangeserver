for ((i=1;i<=100;i++))
do
    echo $i
cat basic.xml|nc localhost 12345 &
done
