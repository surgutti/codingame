for file in brain_log.cpp; do
  filename=$(basename "$file" .cpp)
  
  echo "$file $filename.exe"
  g++ -DPSYLEAGUE -std=c++17 $file -o $filename.exe
done
