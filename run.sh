for((d=1;d<=40;d++)); do
	#printf "$d\n0 0 0\n0 0 0\n0 0 0\n" > in
	#./bfs < in > out$d
	#echo "$d: done"
	cat out$d
	echo -n ","
done
