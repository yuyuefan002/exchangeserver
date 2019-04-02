for ((i=1;i<=100;i++))
do
    echo $i
cat basic.xml|nc localhost 8080 &
done
