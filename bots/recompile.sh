for file in *.cpp; do
  filename=$(basename "$file" .cpp)
  
  echo "$file $filename.exe"
  g++ -DPSYLEAGUE -std=c++17 $file -o $filename.exe
done